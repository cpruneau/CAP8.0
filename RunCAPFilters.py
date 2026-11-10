# **********************************************************************
# Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
# All rights reserved.
#
# For the licensing terms see LICENSE.
# **********************************************************************
"""
RunCAPFilters -- the filters of a CAP workflow and the histogram folders of its analyzers (model of RunCAP.py, no GUI).

A workflow has a library of named objects, shared by its tasks, of eight kinds:
  * event filters and particle filters: each a list of conditions (a range, two ranges, a value or a flag of one
    quantity: multiplicity, transverse momentum, pseudorapidity, PDG code, ...);
  * event and particle efficiencies, event and particle smearers: each its binning parameters;
  * events (filled by the generators or readers, used by the analyzers) and particle databases (imported).
Each task chooses, for each kind it supports, how many objects it uses and which ones (by name). Analyzers fill one
folder of histograms per combination of their filters: (event filter, particle filter) for single-particle analyzers,
(event filter, particle filter 1, particle filter 2) for pair analyzers (with their single-particle folders too). The
folders hold the histograms of each combination, written by the job to its output file in the folder
<task>/<event filter>_<particle filter>[_<particle filter 2>] (with the derived histograms), and
their binning is either the same for all (the HISTOGRAM group of the task) or set folder by folder.

CAP reads all this from flat keys of the configuration of each task (see ManagedObjects::configureObjects() and the
configure() functions of the filters, efficiencies, smearers and histogram folders):
    <task>:<KIND>:N, <task>:<KIND>:USE
    <task>:<KIND>_<k>:NAME, TITLE, OWNER
    <task>:<KIND>_<k>:CONDITION:N
    <task>:<KIND>_<k>:CONDITION_<i>:TYPE, SUBTYPE, MINIMUM, MAXIMUM[, MINIMUM2, MAXIMUM2]
    <task>:<KIND>_<k>:<parameter>                              (efficiencies, smearers)
    <task>:HISTOGRAM_<l>:N, USE, CREATE, IMPORT, <task>:HISTOGRAM_<l>_<k>:NAME, TITLE, OWNER[, <binning parameter>]
    <main task>:JOB:OUTPUT_FILE, INPUT_FILE   (files of the job, see CAP.cxx)
apply_choices() writes these keys from the library and the choices of the tasks; extract_choices() reads them back
(library, choices and histogram options), so workflows written by hand or by earlier versions load too. The object of
a name is defined (OWNER true, with its conditions or parameters) by the task FilterCreator, written first in the
workflow file (so configured first by CAP), with one subtask per kind (EventFilterCreator, ParticleFilterCreator, ...)
creating all the objects of the library; the other tasks take them by name from CAP's stores (OWNER false). Event
filters on Glauber quantities (impact parameter, participants, ...) are objects of the Glauber tasks, which have a
store of their own: the first Glauber task using one defines it.

The library itself, including the objects no task uses, and the histogram options of the tasks are also written in a
RUNCAP element of the workflow file, next to its TASK element (CAP reads only the TASK elements).

Selection files. The objects of the library are written in XML, one element per object named after its kind
(EventFilter, ParticleFilter, EventEfficiency, ...; see TAG_OF_KIND):
    <ParticleFilter name="PiP" title="positive pions">
      <Condition quantity="PDG" form="value" value="211"/>
      <Condition quantity="PT" form="range" minimum="0.2" maximum="2"/>
    </ParticleFilter>
    <EventEfficiency name="Eff"><Parameter name="nBins_mult">100</Parameter> ...</EventEfficiency>
The same elements are written in the RUNCAP element of a workflow, or in a file of their own, a selection file (root
CapSelections, see write_selections(), read_selections()), which several workflows can share: a workflow either
copies the objects of a selection file (import_selections()), or links it (FilterLibrary.link()), writing
<Include file="..."/> (relative to the workflow) instead of the objects, which are read from the file each time the
workflow is loaded and written back to it when the workflow is saved (save_linked()).

No tkinter here: usable and testable without a display.
"""

import copy
import math
import os
import re
import subprocess
import xml.etree.ElementTree as ET

#: Kinds of objects of the library, in the order of the tabs of the Filters pane.
KINDS = ["EVENT_FILTER", "PARTICLE_FILTER", "EVENT_EFFICIENCY", "PARTICLE_EFFICIENCY", "EVENT_SMEARER", "PARTICLE_SMEARER",
         "EVENT", "PARTICLE_DB"]
#: Kinds of corrections (efficiencies and smearers).
CORRECTION_KINDS = ("EVENT_EFFICIENCY", "PARTICLE_EFFICIENCY", "EVENT_SMEARER", "PARTICLE_SMEARER")
#: Kinds whose lists have settings in FilterCreator (creation or import of the objects, export).
SETTINGS_KINDS = CORRECTION_KINDS + ("PARTICLE_DB",)
#: Kinds chosen by default by a new task (the first object of the library).
DEFAULT_CHOICE_KINDS = ("EVENT", "PARTICLE_DB")
KIND_LABELS = {"EVENT_FILTER": "Event filters", "PARTICLE_FILTER": "Particle filters",
               "EVENT_EFFICIENCY": "Event efficiencies", "PARTICLE_EFFICIENCY": "Particle efficiencies",
               "EVENT_SMEARER": "Event smearers", "PARTICLE_SMEARER": "Particle smearers", "EVENT": "Events",
               "PARTICLE_DB": "Particle databases"}
#: Element of an object of each kind in XML (workflows and selection files), and the root of a selection file.
TAG_OF_KIND = {"EVENT_FILTER": "EventFilter", "PARTICLE_FILTER": "ParticleFilter", "EVENT_EFFICIENCY": "EventEfficiency",
               "PARTICLE_EFFICIENCY": "ParticleEfficiency", "EVENT_SMEARER": "EventSmearer",
               "PARTICLE_SMEARER": "ParticleSmearer", "EVENT": "Event", "PARTICLE_DB": "ParticleDb"}
KIND_OF_TAG = {tag: kind for kind, tag in TAG_OF_KIND.items()}
SELECTIONS_ROOT = "CapSelections"
#: Kinds defined by conditions (the others by parameters).
CONDITION_KINDS = ("EVENT_FILTER", "PARTICLE_FILTER")

#: Forms of a condition and the TYPE written for them.
RANGE, DUAL_RANGE, VALUE, FLAG = "range", "two ranges", "value", "flag"
FORMS = (RANGE, DUAL_RANGE, VALUE, FLAG)
TYPE_OF_FORM = {RANGE: "DOUBLE_RANGE", DUAL_RANGE: "DOUBLE_DUAL_RANGE", VALUE: "INTEGER", FLAG: "Bool"}
FORM_OF_TYPE = {"DOUBLE_RANGE": RANGE, "DOUBLE_DUAL_RANGE": DUAL_RANGE, "INTEGER": VALUE, "BOOL": FLAG}

#: Quantities of the conditions: {SUBTYPE: (description, forms accepted)}, as read by the configure() functions.
#: Event filters: the global quantities of the selection of a particle filter, named by the condition (SELECTION),
#: computed once per event and shared by the tasks (CAP::ParticleSelection, CAP::EventFilter).
SELECTION_SUBTYPES = {"MULT": ("multiplicity of a selection", (RANGE,)),
                      "ENERGY": ("energy of a selection", (RANGE,)),
                      "CHARGE": ("net charge of a selection", (RANGE,)),
                      "STRANGENESS": ("net strangeness of a selection", (RANGE,)),
                      "BARYON": ("net baryon number of a selection", (RANGE,)),
                      "PTSUM": ("pT sum of a selection", (RANGE,)),
                      "SPHERO": ("spherocity of a selection", (RANGE,))}
EVENT_SUBTYPES = dict(SELECTION_SUBTYPES)
#: Earlier versions: the quantities set by GlobalAnalyzer for its particle filter k (it must run first).
LEGACY_EVENT_SUBTYPES = set()
for _k in range(6):
    EVENT_SUBTYPES["MULT_%d" % _k] = ("multiplicity %d" % _k, (RANGE,))
    EVENT_SUBTYPES["ENERGY_%d" % _k] = ("energy %d" % _k, (RANGE,))
    EVENT_SUBTYPES["SPHERO_%d" % _k] = ("spherocity %d" % _k, (RANGE,))
    LEGACY_EVENT_SUBTYPES |= {"MULT_%d" % _k, "ENERGY_%d" % _k, "SPHERO_%d" % _k}
#: Event filters of the Glauber tasks (GlauberEventFilter).
GLAUBER_SUBTYPES = {"IMPACT": ("impact parameter b", (RANGE, DUAL_RANGE)),
                    "NPARTS": ("participants N_part", (RANGE, DUAL_RANGE)),
                    "NCOLLS": ("binary collisions N_coll", (RANGE, DUAL_RANGE)),
                    "NSPECS": ("spectators N_spec", (RANGE, DUAL_RANGE))}
PARTICLE_SUBTYPES = {"PT": ("transverse momentum pT", (RANGE, DUAL_RANGE)),
                     "PX": ("momentum px", (RANGE, DUAL_RANGE)),
                     "PY": ("momentum py", (RANGE, DUAL_RANGE)),
                     "PZ": ("momentum pz", (RANGE, DUAL_RANGE)),
                     "ENERGY": ("energy", (RANGE, DUAL_RANGE)),
                     "ETA": ("pseudorapidity eta", (RANGE, DUAL_RANGE)),
                     "RAPIDITY": ("rapidity y", (RANGE, DUAL_RANGE)),
                     "PHI": ("azimuth phi", (RANGE, DUAL_RANGE)),
                     "PDG": ("PDG code", (VALUE,)),
                     "CHARGE": ("charge", (VALUE,)),
                     "BARYON": ("baryon number", (VALUE,)),
                     "STRANGE": ("strangeness", (VALUE,)),
                     "CHARM": ("charm", (VALUE,)),
                     "LIVE": ("live (not decayed)", (FLAG,)),
                     "ENABLED": ("enabled", (FLAG,))}

#: Parameters of the efficiencies and smearers: {kind: [(name, type, default)]}, as read by their configure(). The
#: particle efficiencies and smearers have none here: they are read from a maps file (see MAPS_KINDS).
KIND_PARAMETERS = {
    "EVENT_EFFICIENCY": [("BASE_NAME", "string", ""), ("nBins_mult", "int", "100"), ("Min_mult", "double", "0"),
                         ("Max_mult", "double", "100")],
    "EVENT_SMEARER": [("BASE_NAME", "string", "")],
    # switch read by ParticleDb::configure(): true turns off the weak decays (K0S, K0L, Lambda, Sigma, Xi, Omega)
    "PARTICLE_DB": [("_dbDisableWeakDecays", "bool", "false")],
}

#: Task creating the objects of the library, run first (CAP::FilterCreator), and its subtasks, one per kind.
CREATOR_NAME, CREATOR_CLASS = "FilterCreator", "CAP::FilterCreator"
CREATORS = {"EVENT_FILTER": ("EventFilters", "CAP::EventFilterCreator"),
            "PARTICLE_FILTER": ("ParticleFilters", "CAP::ParticleFilterCreator"),
            "EVENT_EFFICIENCY": ("EventEfficiencies", "CAP::EventEfficiencyCreator"),
            "PARTICLE_EFFICIENCY": ("ParticleEfficiencies", "CAP::ParticleEfficiencyCreator"),
            "EVENT_SMEARER": ("EventSmearers", "CAP::EventSmearerCreator"),
            "PARTICLE_SMEARER": ("ParticleSmearers", "CAP::ParticleSmearerCreator"),
            "EVENT": ("Events", "CAP::EventCreator"),
            "PARTICLE_DB": ("ParticleDbs", "CAP::ParticleDbCreator")}
#: Settings of the list of objects of a kind in its creator (import or creation of their histograms, export).
LIST_SETTINGS = [("CREATE", "bool", "false"), ("IMPORT", "bool", "false"), ("IMPORT:PATH", "string", "DEFAULT"),
                 ("IMPORT:FILE_NAME", "string", "NONE"), ("EXPORT", "bool", "false"),
                 ("EXPORT:PATH", "string", "DEFAULT"), ("EXPORT:FILE_NAME", "string", "NONE")]
#: Settings of the lists of a kind that differ from LIST_SETTINGS: particle databases are imported from
#: <CAP_DATABASE_PATH>/ParticleData/ParticleDb.xml (one XML file: the particles and their decay modes).
DATABASE_FILE = "ParticleDb.xml"
KIND_SETTINGS = {"PARTICLE_DB": {"IMPORT": "true", "IMPORT:PATH": "ParticleData", "IMPORT:FILE_NAME": DATABASE_FILE}}
#: Settings shown for a kind in the Filters pane (all of LIST_SETTINGS by default): databases are only imported.
KIND_SETTING_NAMES = {"PARTICLE_DB": ("IMPORT:PATH", "IMPORT:FILE_NAME")}


def is_database_file(name):
    """Whether a particle database can be imported from a file: XML (particles and decay modes in one file)."""
    return name.lower().endswith(".xml")


def update_database_files(library):
    """Particle databases imported from the former text files (particles.data and decays.data) are imported from
    DATABASE_FILE instead. @return warnings"""
    settings = library.settings["PARTICLE_DB"]
    name = settings.get("IMPORT:FILE_NAME", "")
    if settings.get("IMPORT") == "true" and not is_database_file(name):
        settings["IMPORT:FILE_NAME"] = DATABASE_FILE
        return ["Particle databases are read from XML files (particles and decay modes in one file): %s replaced by %s."
                % (name, DATABASE_FILE)]
    return []


def default_settings(kind):
    """Default settings of the list of a kind in FilterCreator."""
    settings = {name: value for name, _, value in LIST_SETTINGS}
    settings.update(KIND_SETTINGS.get(kind, {}))
    return settings
#: Suffix of the names of the folders of derived histograms.
DERIVED_SUFFIX = "_D"
#: Names of objects: as task names, without spaces or colons (they become parts of keys and histogram names).
NAME_FULL = re.compile(r"[A-Za-z_][A-Za-z0-9_.-]*")
#: Keys of the parameters of a task managed by this model (rewritten by apply_choices()).
#: Kinds whose list settings (USE, CREATE, IMPORT, EXPORT and files) belong to each task: what the task does with its
#: events (a generator or reader creates them, an analyzer uses them). Those of the other kinds are FilterCreator's.
TASK_SETTING_KINDS = ("EVENT",)
MANAGED_KEY = re.compile(r"^(?:(?:%s)(?::N|_\d+:.*)|(?:%s):USE|HISTOGRAM_[1-4](?::N|_\d+:.*)"
                         r"|HISTOGRAM:N|HISTOGRAM_\d+:(?!(?:USE|CREATE|IMPORT|EXPORT|N)(?::|$)).*)$"
                         % ("|".join(KINDS), "|".join(k for k in KINDS if k not in TASK_SETTING_KINDS)))
#: Settings of the events of a task shown in its window.
EVENT_SETTINGS = ("USE", "CREATE", "IMPORT", "IMPORT:PATH", "IMPORT:FILE_NAME", "EXPORT", "EXPORT:PATH",
                  "EXPORT:FILE_NAME")
#: Tasks making events (generators, readers, models): they create them; the analyzers use them.
EVENT_SOURCES = ("BasicEventGen", "PythiaEventGenerator", "PythiaEventReader", "EposEventReader", "PHSDEventReader",
                 "Therminator3", "ThermalModel", "GlauberGenerator", "EventMerger")


#: Tasks writing an event of their own from the event they read: {class: default name of the written event}. Their
#: events are [read, written]; the written event is an object of the library like the others, used (and reset before
#: each event) by the event iterator, and chosen by the analyzers of the detector-level events.
EVENT_WRITERS = {"MeasurementSimulator": "FastSimEvent"}


#: Analyzers of the events written by those tasks: they read the written event first and the event it was made from
#: second (PerformanceAnalyzer: the detector-level event and, for the efficiencies, the generator-level event).
WRITTEN_EVENT_READERS = ("PerformanceAnalyzer",)


def default_reader_events(task, library):
    """Events of a new analyzer of written events (WRITTEN_EVENT_READERS): [written, read] when the library has a written
    event (FastSimEvent)."""
    if task.cls.replace("CAP::", "") not in WRITTEN_EVENT_READERS:
        return
    names = library.names("EVENT")
    written = [n for n in EVENT_WRITERS.values() if n in names]
    if not written:
        return
    others = [n for n in names if n not in EVENT_WRITERS.values()]
    choices(task)["EVENT"] = written[:1] + others[:1]


def writes_events(task):
    """Whether a task writes an event of its own (see EVENT_WRITERS)."""
    return task.cls.replace("CAP::", "") in EVENT_WRITERS


def written_event(task):
    """Name of the event a task writes (its second event), or None."""
    names = choices(task).get("EVENT", [])
    return names[1] if writes_events(task) and len(names) >= 2 else None


def default_writer_events(task, library):
    """Events of a task writing one: [the first other event of the library, its written event], the written event
    (EVENT_WRITERS) added to the library if needed. USE on, CREATE off: it reads the first and fills the second
    itself. @return True if changed"""
    if not writes_events(task):
        return False
    written = EVENT_WRITERS[task.cls.replace("CAP::", "")]
    names = choices(task).setdefault("EVENT", [])
    changed = False
    if len(names) < 2:
        if library.get("EVENT", written) is None:
            library.add(FilterDef("EVENT", written))
        if not names:
            names.extend(n for n in library.names("EVENT")[:1] if n != written)
        if names and names[0] != written:
            names[1:] = [written]
            changed = True
    for name, value in (("USE", "true"), ("CREATE", "false")):
        p = task.param("EVENT:" + name)
        if p is not None and p.value != value:
            p.value = value
            changed = True
    return changed


def update_writer_events(tasks, library):
    """Tasks writing events (MeasurementSimulator) of workflows written before they did: their written event is set
    (FastSimEvent). @return warnings"""
    warnings = []
    for task in _preorder(tasks):
        if default_writer_events(task, library):
            warnings.append("Task %s reads %s and writes %s (Events)." % (
                task.name, choices(task)["EVENT"][0] if choices(task).get("EVENT") else "?",
                written_event(task) or EVENT_WRITERS[task.cls.replace("CAP::", "")]))
    return warnings


def makes_events(task):
    """Whether a task makes events (a generator, a reader or a model)."""
    name = task.cls.replace("CAP::", "")
    return name in EVENT_SOURCES or name.endswith(("Gen", "Generator", "Reader"))


def update_event_settings(tasks, templates):
    """Generators and readers that neither create nor import their events (workflows written before the events
    settings were the tasks', when RunCAP wrote USE true and CREATE false for every task): they create them, and do
    not use them. @return warnings"""
    warnings = []
    for task in _preorder(tasks):
        if "EVENT" not in task_kinds(task, templates) or not makes_events(task):
            continue
        if any(_param_value(task, "EVENT:" + flag, "false") == "true" for flag in ("CREATE", "IMPORT")):
            continue
        for name, value in (("CREATE", "true"), ("USE", "false")):
            p = task.param("EVENT:" + name)
            if p is not None:
                p.value = value
        warnings.append("Task %s makes events: set to create them (Create) and not to use them." % task.name)
    return warnings


def events_made_below(iterator, templates):
    """Events created or imported by the tasks an EventIterator runs (its subtasks, theirs, ...), in order: not those of
    a nested EventIterator (it resets them itself), nor those of the Glauber tasks (stores of their own)."""
    names = []

    def visit(task):
        for child in task.children:
            if child.cls == ITERATOR_OF_MODE[FILL]:
                continue
            if not is_glauber(child) and "EVENT" in task_kinds(child, templates) and any(
                    _param_value(child, "EVENT:" + flag, "false") == "true" for flag in ("CREATE", "IMPORT")):
                names.extend(n for n in choices(child).get("EVENT", []) if n not in names)
            written = written_event(child)
            if written and written not in names:
                names.append(written)
            visit(child)
    visit(iterator)
    return names


def update_iterator_events(tasks, templates):
    """Each EventIterator uses the events made by its tasks: it resets them (and counts them) before each event, so
    that several generators can fill the same event (embedding). A generator does not reset its events. Applied when a
    workflow is loaded and after each change in RunCAP.py. @return a warning for each iterator changed"""
    warnings = []
    for task in _preorder(tasks):
        if task.cls != ITERATOR_OF_MODE[FILL]:
            continue
        made = events_made_below(task, templates)
        if not made:
            continue
        used = choices(task).setdefault("EVENT", [])
        missing = [name for name in made if name not in used]
        use = task.param("EVENT:USE")
        if not missing and (use is None or use.value == "true"):
            continue
        used.extend(missing)
        if use is not None:
            use.value = "true"
        warnings.append("Event iterator %s set to use the events of its tasks (%s): it resets them before each event."
                        % (task.name, ", ".join(made)))
    return warnings


def default_event_settings(task, templates):
    """Events of a new task: created by a generator or reader, used by an analyzer, neither by the other tasks."""
    if "EVENT" not in task_kinds(task, templates):
        return
    if writes_events(task):
        return                                          # see default_writer_events()
    source = makes_events(task)
    for name, value in (("CREATE", source), ("USE", not source and histogram_layout(task, templates) is not None)):
        p = task.param("EVENT:" + name)
        if p is not None:
            p.value = "true" if value else "false"
#: Settings of the lists of objects of a task (<KIND>:USE, IMPORT, CREATE, EXPORT and their files): FilterCreator
#: creates, imports and exports the objects, so the tasks only choose them (they are hidden in RunCAP.py's windows).
LIST_SETTING_KEY = re.compile(r"^(?:%s):(?:USE|IMPORT|CREATE|EXPORT)(?::.*)?$" % "|".join(KINDS))
_OBJECT_KEY = re.compile(r"^(%s)_(\d+):(.+)$" % "|".join(KINDS))
_CONDITION_KEY = re.compile(r"^CONDITION_(\d+):(.+)$")
_FOLDER_KEY = re.compile(r"^HISTOGRAM_([1-4])_(\d+):(.+)$")
_FOLDER_ATTRIBUTES = ("NAME", "TITLE", "OWNER", "BASE_NAME")
_LIST_ATTRIBUTES = ("USE", "N", "IMPORT", "CREATE", "EXPORT")
#: Facts about the folders stated by the templates of the tasks of the folders design (not settings): the number of
#: particle filters per folder (FOLDER_RANK: 1, one folder per event and particle filter; 2, per event filter and pair of
#: particle filters, e.g. a pair analysis, which also has a single-particle folder per event and particle filter).
_FOLDER_FACTS = ("FOLDER_RANK",)


def number_text(value):
    """Shortest text of a number that reads back exactly (integers without a decimal point)."""
    x = float(value)
    if x == int(x) and abs(x) < 1e15:
        return str(int(x))
    return repr(x)


# ------------------------------------------------------------------------------------------------------------------
# Library
# ------------------------------------------------------------------------------------------------------------------

class Condition:
    """A condition of a filter on one quantity (SUBTYPE): a range [minimum, maximum), two ranges, a value or a flag.
    For the quantities of a selection (SELECTION_SUBTYPES), selection is the name of the particle filter."""

    def __init__(self, subtype, form=RANGE, minimum=0.0, maximum=0.0, minimum2=0.0, maximum2=0.0, selection=""):
        self.subtype = subtype
        self.selection = selection or ""
        self.form = form
        self.minimum = float(minimum)
        self.maximum = float(maximum)
        self.minimum2 = float(minimum2)
        self.maximum2 = float(maximum2)

    @property
    def type(self):
        """TYPE written for CAP."""
        return TYPE_OF_FORM.get(self.form, self.form)

    def significant(self):
        """What the condition tests: the values its form uses (a value or a flag: the minimum only)."""
        values = {VALUE: (self.minimum,), FLAG: (bool(self.minimum),), RANGE: (self.minimum, self.maximum)}.get(
            self.form, (self.minimum, self.maximum, self.minimum2, self.maximum2))
        return self.subtype, self.selection, self.form, values

    def __eq__(self, other):
        return isinstance(other, Condition) and self.significant() == other.significant()

    def __repr__(self):
        return "Condition(%s%s, %s, %g, %g)" % (self.subtype, "(%s)" % self.selection if self.selection else "",
                                                 self.form, self.minimum, self.maximum)


class FilterDef:
    """An object of the library: kind, name, title, and its conditions (filters) or parameters (the others). source:
    the selection file it comes from when the library links that file ("": an object of the workflow itself)."""

    def __init__(self, kind, name, title="", conditions=None, parameters=None, source=""):
        self.kind = kind
        self.name = name
        self.title = title or ""
        self.conditions = list(conditions or [])
        self.parameters = dict(default_parameters(kind))
        self.source = source or ""
        if kind not in MAPS_KINDS:                      # those of the maps file: read from it, none here
            self.parameters.update(parameters or {})

    def same_definition(self, other):
        """Whether two objects define the same thing (wherever they come from)."""
        return (isinstance(other, FilterDef) and (self.kind, self.name, self.title, self.conditions, self.parameters)
                == (other.kind, other.name, other.title, other.conditions, other.parameters))

    def copy(self, source=""):
        """A copy of the object, from another source."""
        return FilterDef(self.kind, self.name, self.title, [copy.copy(c) for c in self.conditions],
                         dict(self.parameters), source)

    def __eq__(self, other):
        return isinstance(other, FilterDef) and vars(self) == vars(other)

    def __repr__(self):
        return "FilterDef(%s, %s, %d conditions)" % (self.kind, self.name, len(self.conditions))


def default_parameters(kind):
    """Default parameters of an object of a kind ({} for filters)."""
    return {name: value for name, _, value in KIND_PARAMETERS.get(kind, [])}


def parameter_type(kind, name):
    for n, t, _ in KIND_PARAMETERS.get(kind, []):
        if n == name:
            return t
    return "string"


def subtypes_for(kind, glauber=False):
    """Quantities a filter of a kind can test ({} for the kinds without conditions). @param glauber for the event
    filters of the Glauber tasks"""
    if kind == "EVENT_FILTER":
        return GLAUBER_SUBTYPES if glauber else EVENT_SUBTYPES
    if kind == "PARTICLE_FILTER":
        return PARTICLE_SUBTYPES
    return {}


class FilterLibrary:
    """The objects of a workflow, by kind, in order."""

    def __init__(self):
        self.objects = {kind: [] for kind in KINDS}
        self.settings = {kind: default_settings(kind) for kind in KINDS}
        self.job = {name: "" for name in JOB_SETTINGS}      # files of the job (see JOB_SETTINGS)
        self.links = []                                     # selection files linked (absolute paths, in order)
        self.link_extras = {}                               # {path: elements of the file not read, written back}
        self.link_order = {}                                # {path: [(kind, name)] of the objects, in the file}
        self.link_shadowed = {}                             # {path: objects of the file the workflow also defines}

    @staticmethod
    def with_defaults():
        """Library of a new workflow: one event (Event) and one particle database (ParticleDb)."""
        library = FilterLibrary()
        library.add(FilterDef("EVENT", "Event"))
        library.add(FilterDef("PARTICLE_DB", "ParticleDb"))
        return library

    def names(self, kind):
        return [o.name for o in self.objects[kind]]

    def get(self, kind, name):
        for o in self.objects[kind]:
            if o.name == name:
                return o
        return None

    def add(self, definition):
        """Add an object, or replace the one of the same kind and name."""
        existing = self.get(definition.kind, definition.name)
        if existing is not None:
            self.objects[definition.kind][self.objects[definition.kind].index(existing)] = definition
        else:
            self.objects[definition.kind].append(definition)

    def resize(self, kind, n):
        """Keep the first n objects of a kind, or add new ones (named after the kind) up to n."""
        items = self.objects[kind]
        del items[n:]
        base = {"EVENT_FILTER": "EventFilter", "PARTICLE_FILTER": "ParticleFilter", "EVENT_EFFICIENCY": "EventEfficiency",
                "PARTICLE_EFFICIENCY": "ParticleEfficiency", "EVENT_SMEARER": "EventSmearer",
                "PARTICLE_SMEARER": "ParticleSmearer", "EVENT": "Event", "PARTICLE_DB": "ParticleDb"}[kind]
        k = 0
        while len(items) < n:
            name = "%s%d" % (base, k)
            if self.get(kind, name) is None:
                items.append(FilterDef(kind, name))
            k += 1

    def all_objects(self):
        """Every object, kind by kind."""
        return [o for kind in KINDS for o in self.objects[kind]]

    def linked(self, path):
        """Objects that come from a linked selection file, kind by kind."""
        path = os.path.abspath(path)
        return [o for o in self.all_objects() if o.source and os.path.abspath(o.source) == path]

    def link(self, path):
        """Link a selection file: its objects are added to the library (an object of the same kind and name already
        there is kept, with a warning), coming from the file, which they are written back to (save_linked()) and
        read from again when the workflow is loaded. @return warnings @throws OSError, ValueError if the file cannot
        be read"""
        path = os.path.abspath(path)
        objects, extras, warnings = read_selections(path)
        if path not in self.links:
            self.links.append(path)
        self.link_extras[path] = extras
        self.link_order[path] = [(o.kind, o.name) for o in objects]
        shadowed = self.link_shadowed[path] = []
        for o in objects:
            existing = self.get(o.kind, o.name)
            if existing is not None and existing.source != path:
                warnings.append("%s: %s %s is already in the workflow (the one of the workflow is kept)."
                                % (os.path.basename(path), KIND_LABELS[o.kind].lower()[:-1], o.name))
                shadowed.append(o)                         # left in the file as it is
                continue
            o.source = path
            self.add(o)
        return warnings

    def unlink(self, path, keep=True):
        """Stop linking a selection file: its objects become objects of the workflow (keep) or are removed.
        @return names removed {kind: [name]}"""
        path = os.path.abspath(path)
        removed = {}
        for o in self.linked(path):
            if keep:
                o.source = ""
            else:
                self.objects[o.kind].remove(o)
                removed.setdefault(o.kind, []).append(o.name)
        if path in self.links:
            self.links.remove(path)
        self.link_extras.pop(path, None)
        self.link_shadowed.pop(path, None)
        self.link_order.pop(path, None)
        return removed

    def import_objects(self, objects, names=None):
        """Copy objects into the library (those of the given (kind, name), or all), as objects of the workflow; an
        object of the same kind and name is replaced. @return the (kind, name) replaced"""
        replaced = []
        for o in objects:
            if names is not None and (o.kind, o.name) not in names:
                continue
            if self.get(o.kind, o.name) is not None:
                replaced.append((o.kind, o.name))
            self.add(o.copy(""))
        return replaced

    def __eq__(self, other):
        return (isinstance(other, FilterLibrary) and self.objects == other.objects and self.settings == other.settings
                and self.job == other.job and self.links == other.links)


# ------------------------------------------------------------------------------------------------------------------
# Choices of the tasks and their histogram folders
# ------------------------------------------------------------------------------------------------------------------

class HistogramOptions:
    """Binning of the histogram folders of an analyzer: the same for all (the HISTOGRAM group of the task), or per
    combination of filters ({combination (tuple of filter names): {parameter: value}}, values missing from a
    combination being those of the HISTOGRAM group)."""

    def __init__(self, same_binning=True, binning=None):
        self.same_binning = same_binning
        self.binning = dict(binning or {})

    def __eq__(self, other):
        return isinstance(other, HistogramOptions) and vars(self) == vars(other)

    def __repr__(self):
        return "HistogramOptions(same=%r, %d combinations)" % (self.same_binning, len(self.binning))


class Folder:
    """A folder of histograms of an analyzer: list (1 to 4), index in the list, name, combination of filters, and
    whether it holds derived histograms."""

    def __init__(self, list_index, index, name, combination, derived):
        self.list = list_index
        self.index = index
        self.name = name                        # name of the object in CAP's store
        self.combination = combination
        self.derived = derived
        self.folder = "_".join(combination)     # folder in the folder of the task, in the files of the job

    def __repr__(self):
        return "Folder(%s %s)" % (folder_key(self.list, self.index), self.name)


def _template_keys(task, templates):
    template = templates.get(task.cls)
    return set(p.key for p in template.params) if template else set(p.key for p in task.params)


def task_kinds(task, templates):
    """Kinds of objects a task can use: those whose list its class configures (its template has <KIND>:N)."""
    keys = _template_keys(task, templates)
    return [kind for kind in KINDS if kind + ":N" in keys]


def is_glauber(task):
    """Whether the event filters of a task are those of Glauber (GlauberEventFilter)."""
    return task.cls.replace("CAP::", "").startswith("Glauber")


def histogram_layout(task, templates):
    """Folders of histograms of a task: "folders" (one list, HISTOGRAM: the tasks of an analysis in the design where
    each folder holds all its histograms, see EventProcessorHistos), "pair" (lists 1 to 4), "single" (lists 1 and 2),
    or None."""
    keys = _template_keys(task, templates)
    if "HISTOGRAM_3:N" in keys and "HISTOGRAM_1:N" in keys:
        return "pair"
    if "HISTOGRAM_1:N" in keys:
        return "single"
    if "HISTOGRAM:N" in keys:
        return "folders"
    return None


def _template_value(task, templates, key, default=None):
    template = templates.get(task.cls)
    p = template.param(key) if template is not None else task.param(key)
    return p.value if p is not None else default


def folder_rank(task, templates):
    """Number of particle filters per folder of a task of the folders design (HISTOGRAM:FOLDER_RANK of its template):
    1 (default), one folder per event and particle filter; 2, per event filter and ordered pair of particle filters (a
    pair analysis: n x m x m pair folders, listed here, and n x m single-particle folders, made by the task)."""
    try:
        return int(_template_value(task, templates, "HISTOGRAM:FOLDER_RANK", "1"))
    except ValueError:
        return 1


def list_key(list_index):
    """Key of a list of folders: HISTOGRAM_<l> for a numbered list, HISTOGRAM for the single list (0)."""
    return "HISTOGRAM" if list_index == 0 else "HISTOGRAM_%d" % list_index


def folder_key(list_index, index):
    """Key prefix of a folder: HISTOGRAM_<l>_<k>: in a numbered list, HISTOGRAM_<k>: in the single list (0)."""
    return "HISTOGRAM_%d:" % index if list_index == 0 else "HISTOGRAM_%d_%d:" % (list_index, index)


def binning_keys(task, templates):
    """Names of the binning parameters of the histograms of a task (the parameters of its HISTOGRAM group)."""
    template = templates.get(task.cls)
    params = template.params if template else task.params
    names = []
    for p in params:
        parts = p.key.split(":")
        if len(parts) == 2 and parts[0] == "HISTOGRAM" and parts[1] not in _LIST_ATTRIBUTES + _FOLDER_FACTS + ("BASE_NAME",):
            names.append(parts[1])
    return names


def choices(task):
    """Names of the objects a task uses, by kind (created empty if needed)."""
    if not hasattr(task, "filters") or task.filters is None:
        task.filters = {}
    return task.filters


def histogram_options(task):
    """Histogram options of a task (created if needed)."""
    if not hasattr(task, "histograms") or task.histograms is None:
        task.histograms = HistogramOptions()
    return task.histograms


def combinations(task, templates):
    """Combinations of filters of the base folders of a task: [(event, particle)] or, for a pair analyzer,
    [(event, particle 1, particle 2)] then [(event, particle)], in the order of the folders."""
    layout = histogram_layout(task, templates)
    events = choices(task).get("EVENT_FILTER", [])
    particles = choices(task).get("PARTICLE_FILTER", [])
    singles = [(e, p) for e in events for p in particles]
    if layout == "folders":
        return _rank_combinations(events, particles, folder_rank(task, templates))
    if layout == "pair":
        return [(e, p1, p2) for e in events for p1 in particles for p2 in particles] + singles
    if layout == "single":
        return singles
    return []


def _rank_combinations(events, particles, rank):
    """Combinations of an event filter and rank particle filters, in the order of the C++ folders."""
    combos = [(e,) for e in events]
    for _ in range(rank):
        combos = [c + (p,) for c in combos for p in particles]
    return combos


def folder_lists(task, templates):
    """Lists of folders of a task: [0] (the single list HISTOGRAM), [1, 2] (histograms, derived histograms) or, for a
    pair analyzer, [1, 2, 3, 4] (pairs, derived pairs, singles, derived singles); [] if it has no histograms."""
    layout = histogram_layout(task, templates)
    return [1, 2, 3, 4] if layout == "pair" else [1, 2] if layout == "single" else [0] if layout == "folders" else []


def list_label(task, templates, list_index):
    """Name of a list of folders, e.g. "Derived histograms"."""
    if list_index == 0:
        return "Histograms"
    if histogram_layout(task, templates) == "pair":
        return {1: "Pair histograms", 2: "Derived pair histograms", 3: "Single histograms",
                4: "Derived single histograms"}[list_index]
    return {1: "Histograms", 2: "Derived histograms"}[list_index]


def list_used(task, list_index):
    """Whether a task uses a list of folders (HISTOGRAM_<l>:USE): it then has one folder per combination of its
    filters, none otherwise."""
    return _param_value(task, list_key(list_index) + ":USE", "false") == "true"


#: Settings of a list of folders shown in the task windows, and those set for the lists of a new analyzer.
#: A list of folders has no files of its own: the job writes the histograms of all its tasks to one output file and
#: reads those imported (IMPORT) from one input file (see JOB_SETTINGS).
FOLDER_LIST_SETTINGS = ("USE", "CREATE", "IMPORT")
#: Keys of earlier versions, dropped when a workflow is read: files of the lists of folders, base names of the folders,
#: directory levels of FileIterator.
OBSOLETE_KEY = re.compile(r"^(?:HISTOGRAM_[1-4]:(?:EXPORT(?::.*)?|IMPORT:.*|SCALE|OWNER)|nLevels)$")
#: Files of the job, keys of the task CAP runs (<task>:JOB:<name>, see CAP.cxx): the output file, where the histograms of
#: all the tasks are written (a folder per task holding a folder per combination of filters; by default <task>.cap),
#: and the input file, read for the histograms imported (a derived run reads the output file of its fill run).
JOB_SETTINGS = ("OUTPUT_FILE", "INPUT_FILE")
JOB_KEY = re.compile(r"^JOB:(%s)$" % "|".join(JOB_SETTINGS))


#: Runs of an analyzer: filling its histograms event by event (in an EventIterator), or computing its derived
#: histograms from histograms filled by an earlier run (in a FileIterator: postProcess() calls calculateDerived()).
FILL, DERIVED, CUSTOM = "fill", "derived", "custom"
ITERATOR_OF_MODE = {FILL: "CAP::EventIterator", DERIVED: "CAP::FileIterator"}


def _set_list(task, list_index, name, value):
    p = task.param("%s:%s" % (list_key(list_index), name))
    if p is not None:
        p.value = value


def _flag(task, list_index, name):
    return _param_value(task, "%s:%s" % (list_key(list_index), name), "false") == "true"


def set_histogram_mode(task, templates, mode):
    """Set the folder lists of an analyzer for a run: FILL (its histograms used and created; derived histograms not
    used) or DERIVED (its histograms used and imported from the input file of the job, the output file of the fill
    run; derived histograms used and created). The job writes them all to its output file."""
    for l in folder_lists(task, templates):
        base = l in (0, 1, 3)
        if mode == FILL:
            settings = {"USE": base, "CREATE": base, "IMPORT": False}
        else:
            settings = {"USE": True, "CREATE": not base, "IMPORT": base}
        for name, value in settings.items():
            _set_list(task, l, name, "true" if value else "false")


def histogram_mode(task, templates):
    """Run the folder lists of an analyzer are set for: FILL, DERIVED, or CUSTOM (set by hand); None without lists."""
    lists = folder_lists(task, templates)
    if not lists:
        return None
    base = [l for l in lists if l in (0, 1, 3)]
    derived = [l for l in lists if l in (2, 4)]
    if all(list_used(task, l) and _flag(task, l, "CREATE") and not _flag(task, l, "IMPORT") for l in base) \
            and not any(list_used(task, l) for l in derived):
        return FILL
    if all(list_used(task, l) and _flag(task, l, "IMPORT") and not _flag(task, l, "CREATE") for l in base) \
            and all(list_used(task, l) and _flag(task, l, "CREATE") for l in derived):
        return DERIVED
    return CUSTOM


def _copy_task(task):
    """Copy of a task without its subtasks: own parameters (without the widgets of the GUI), choices and options."""
    c = copy.copy(task)
    c.params = []
    for p in task.params:
        q = copy.copy(p)
        q.__dict__.pop("widget_var", None)
        c.params.append(q)
    c.children = []
    c.filters = {k: list(v) for k, v in choices(task).items()}
    if getattr(task, "histograms", None) is not None:
        c.histograms = copy.deepcopy(task.histograms)
    return c


def derived_calculator_class(cls):
    """Class of the task computing the derived histograms of an analyzer (CAP::XAnalyzer -> CAP::XDerivedCalculator)."""
    return cls[:-len("Analyzer")] + "DerivedCalculator" if cls.endswith("Analyzer") else None


def derived_calculator(analyzer, templates):
    """Task computing the derived histograms of an analyzer whose folders hold all its histograms (layout "folders"):
    its calculator class (derived_calculator_class()), named <analyzer>Derived, with the filters and histogram settings
    of the analyzer, SOURCE_TASK the analyzer (it reads and writes the folders of the analyzer in the files of the jobs)
    and its folders imported; no events (a calculator, a CAP::AnalysisTask, has the event and particle filters only).
    @return the calculator, or None if the class has no calculator"""
    template = templates.get(derived_calculator_class(analyzer.cls) or "")
    if template is None:
        return None
    calculator = template.clone()
    calculator.name = analyzer.name + "Derived"
    values = {p.key: p.value for p in analyzer.params}
    for p in calculator.params:
        if p.key in values and p.key != "SOURCE_TASK":
            p.value = values[p.key]
    source = calculator.param("SOURCE_TASK")
    if source is not None:
        source.value = analyzer.name
    kinds = task_kinds(calculator, templates)   # a calculator has the filters of the analyzer, and no events
    calculator.filters = {k: list(v) for k, v in choices(analyzer).items() if k in kinds}
    if getattr(analyzer, "histograms", None) is not None:
        calculator.histograms = copy.deepcopy(analyzer.histograms)
    set_histogram_mode(calculator, templates, DERIVED)
    if calculator.param("EVENT:USE") is not None:
        calculator.param("EVENT:USE").value = "false"
    return calculator


def is_calculator(task, templates):
    """Whether a task computes histograms from those of an analyzer (a calculator: SOURCE_TASK, folders imported)."""
    return histogram_layout(task, templates) == "folders" and task.param("SOURCE_TASK") is not None


def derived_workflow(tasks, templates, TaskConfig):
    """Workflow computing the derived histograms of the analyzers of a workflow from the histograms it exports.
    An analyzer whose folders hold all its histograms (layout "folders") gets its calculator (derived_calculator()),
    a top-level task: no event loop. The other analyzers: each task holding them becomes a FileIterator of the same
    name holding them (set to DERIVED, same names and filters, so their folders and files match). The other tasks
    (generators, ...) are left out. @return top-level tasks (copies)"""
    def convert(task):
        layout = histogram_layout(task, templates)
        if layout == "folders":
            calculator = derived_calculator(task, templates)
            return [calculator] if calculator is not None else []
        calculators = [derived_calculator(c, templates) for c in task.children
                       if histogram_layout(c, templates) == "folders"]
        calculators = [c for c in calculators if c is not None]
        analyzers = [_copy_task(c) for c in task.children if histogram_layout(c, templates) in ("single", "pair")]
        if layout is not None:
            analyzers = [_copy_task(task)]
            set_histogram_mode(analyzers[0], templates, DERIVED)
            if analyzers[0].param("EVENT:USE") is not None:
                analyzers[0].param("EVENT:USE").value = "false"
            return analyzers
        if not analyzers and not calculators:
            return [t for c in task.children for t in convert(c)]
        if not analyzers:
            return calculators
        template = templates.get(ITERATOR_OF_MODE[DERIVED])
        iterator = template.clone() if template is not None else TaskConfig(task.name, ITERATOR_OF_MODE[DERIVED])
        iterator.name = task.name
        for a in analyzers:
            set_histogram_mode(a, templates, DERIVED)
            use = a.param("EVENT:USE")
            if use is not None:
                use.value = "false"         # no event loop: histograms read from the input file of the job
        iterator.children = analyzers
        return calculators + [iterator]
    return [t for task in tasks for t in convert(task)]


#: Keys of the numbered lists of folders in workflows written before an analysis moved to one list (HISTOGRAM).
_NUMBERED_LIST_KEY = re.compile(r"^HISTOGRAM_[1-4]:(?:USE|CREATE|IMPORT)$")


def update_histogram_lists(tasks, templates):
    """Analyzers whose folders now hold all their histograms (one list, HISTOGRAM) read from workflows written for the
    numbered lists: the settings of list 1 (their histograms) become those of HISTOGRAM; those of the derived lists
    are dropped (the derived histograms are now computed by the calculator of the analysis, see derived_workflow()).
    @return warnings"""
    warnings = []
    for task in _preorder(tasks):
        if histogram_layout(task, templates) != "folders":
            continue
        old = [p for p in task.params if _NUMBERED_LIST_KEY.match(p.key)]
        if not old:
            continue
        for name in FOLDER_LIST_SETTINGS:
            value = _param_value(task, "HISTOGRAM_1:" + name)
            if value is not None:
                _set_list(task, 0, name, value)
        task.params = [p for p in task.params if p not in old]
        warnings.append("Task %s: its histograms are now in one list (HISTOGRAM), set as its list HISTOGRAM_1; the "
                        "derived histograms are computed by its calculator (derived run)." % task.name)
    return warnings


def is_numbered_list_warning(warning):
    """Whether a warning of RunCAP.load_workflow() is about a key of the numbered lists (see update_histogram_lists())."""
    return re.search(r"parameter HISTOGRAM_[1-4]:(?:USE|CREATE|IMPORT) is not defined by class", warning) is not None


def default_histogram_settings(task, templates):
    """Settings of the folder lists of a new analyzer: a fill run (see set_histogram_mode())."""
    set_histogram_mode(task, templates, FILL)


def folders(task, templates):
    """Folders of histograms of a task, in the order of their lists and indices (as the C++ code indexes them:
    e*nP + p for single folders, e*nP*nP + p1*nP + p2 for pair folders)."""
    layout = histogram_layout(task, templates)
    if layout is None:
        return []
    prefix = task.name                  # objects shared by name in CAP's stores: unique across the tasks
    events = choices(task).get("EVENT_FILTER", [])
    particles = choices(task).get("PARTICLE_FILTER", [])
    result = []

    def add(list_base, combos):
        for k, combo in enumerate(combos):
            name = "_".join([prefix] + list(combo))
            result.append(Folder(list_base, k, name, combo, False))
        for k, combo in enumerate(combos):
            name = "_".join([prefix] + list(combo)) + DERIVED_SUFFIX
            result.append(Folder(list_base + 1, k, name, combo, True))

    singles = [(e, p) for e in events for p in particles]
    if layout == "folders":
        for k, combo in enumerate(_rank_combinations(events, particles, folder_rank(task, templates))):
            result.append(Folder(0, k, "_".join([prefix] + list(combo)), combo, False))
    elif layout == "pair":
        add(1, [(e, p1, p2) for e in events for p1 in particles for p2 in particles])
        add(3, singles)
    else:
        add(1, singles)
    return result


# ------------------------------------------------------------------------------------------------------------------
# Keys of the configuration
# ------------------------------------------------------------------------------------------------------------------

def _param_value(task, key, default=None):
    p = task.param(key)
    return p.value if p is not None else default


def _set(task, key, ptype, value, Param):
    task.params.append(Param(key, ptype, value))


def strip_managed(task):
    """Remove the parameters managed by this model from a task."""
    task.params = [p for p in task.params if not MANAGED_KEY.match(p.key)]


def _preorder(tasks):
    for t in tasks:
        yield t
        yield from _preorder(t.children)


#: Kinds whose objects the Glauber tasks keep in stores of their own (GlauberEventFilter, GlauberEvent): the first
#: Glauber task using one defines it.
GLAUBER_KINDS = ("EVENT_FILTER", "EVENT")


def default_choices(task, library, templates):
    """Choose for a new task the first event and particle database of the library (if it uses them)."""
    for kind in DEFAULT_CHOICE_KINDS:
        if kind in task_kinds(task, templates) and library.objects[kind] and not choices(task).get(kind):
            choices(task)[kind] = [library.objects[kind][0].name]
    default_writer_events(task, library)
    default_reader_events(task, library)


def glauber_only(definition):
    """Whether an event filter tests Glauber quantities: it is then an object of the Glauber tasks (GlauberEventFilter),
    not created by FilterCreator."""
    return definition.kind == "EVENT_FILTER" and any(c.subtype in GLAUBER_SUBTYPES for c in definition.conditions)


def _write_object(task, kind, k, name, definition, owner, Param):
    """Keys of object k of a list: NAME, TITLE, OWNER and, for an owner, its conditions or parameters."""
    prefix = "%s_%d:" % (kind, k)
    _set(task, prefix + "NAME", "string", name, Param)
    _set(task, prefix + "TITLE", "string", definition.title if definition else "", Param)
    _set(task, prefix + "OWNER", "bool", "true" if owner else "false", Param)
    if not owner:
        return
    if kind in CONDITION_KINDS:
        _set(task, prefix + "CONDITION:N", "int", str(len(definition.conditions)), Param)
        for i, c in enumerate(definition.conditions):
            cp = prefix + "CONDITION_%d:" % i
            _set(task, cp + "TYPE", "string", c.type, Param)
            _set(task, cp + "SUBTYPE", "string", c.subtype, Param)
            if c.subtype in SELECTION_SUBTYPES and kind == "EVENT_FILTER":
                _set(task, cp + "SELECTION", "string", c.selection, Param)
            _set(task, cp + "MINIMUM", "double", number_text(c.minimum), Param)
            _set(task, cp + "MAXIMUM", "double", number_text(c.minimum if c.form in (VALUE, FLAG) else c.maximum),
                 Param)                                         # a value or a flag: CAP reads MINIMUM only
            if c.form == DUAL_RANGE:
                _set(task, cp + "MINIMUM2", "double", number_text(c.minimum2), Param)
                _set(task, cp + "MAXIMUM2", "double", number_text(c.maximum2), Param)
    else:
        for pname, ptype, _ in KIND_PARAMETERS.get(kind, []):
            _set(task, prefix + pname, ptype, definition.parameters.get(pname, ""), Param)


def creator_task(library, templates, TaskConfig, Param):
    """The task FilterCreator: one subtask per kind, creating all the objects of the library of that kind (OWNER
    true), but the event filters on Glauber quantities. @param TaskConfig, Param classes of RunCAP.py"""
    def new(name, cls):
        template = templates.get(cls)
        t = template.clone() if template is not None else TaskConfig(name, cls)
        t.name = name
        return t
    creator = new(CREATOR_NAME, CREATOR_CLASS)
    for kind in KINDS:
        sub = new(*CREATORS[kind])
        strip_managed(sub)
        objects = [o for o in library.objects[kind] if not glauber_only(o)]
        _set(sub, kind + ":N", "int", str(len(objects)), Param)
        use = sub.param(kind + ":USE")
        if use is not None:
            use.value = "true" if objects else "false"
        else:
            _set(sub, kind + ":USE", "bool", "true" if objects else "false", Param)
        for name, ptype, _ in LIST_SETTINGS:
            value = library.settings[kind].get(name, default_settings(kind)[name])
            if kind in MAPS_KINDS and name == "IMPORT" and not objects:
                value = "false"                         # nothing to read: the file is not opened
            p = sub.param(kind + ":" + name)
            if p is not None:
                p.value = value
            else:
                _set(sub, kind + ":" + name, ptype, value, Param)
        for k, o in enumerate(objects):
            _write_object(sub, kind, k, o.name, o, True, Param)
        creator.children.append(sub)
    return creator


def apply_choices(tasks, library, templates, Param):
    """Write in the parameters of the tasks the keys of their objects (by name, OWNER false: FilterCreator creates
    them) and of their histogram folders (the parameters managed by this model are replaced). The event filters on
    Glauber quantities are defined by the first Glauber task using them. The tasks are changed in place: apply it to
    copies to keep the model. @param Param class of the parameters (RunCAP.Param)"""
    glauber_owned = set()                               # (kind, name) defined by an earlier Glauber task
    for task in _preorder(tasks):
        strip_managed(task)
        kinds = task_kinds(task, templates)
        for kind in kinds:
            names = choices(task).get(kind, [])
            for flag in ("IMPORT", "CREATE", "EXPORT") if kind not in TASK_SETTING_KINDS else ():   # FilterCreator's
                p = task.param(kind + ":" + flag)
                if p is not None and p.value != "false":
                    off = copy.copy(p)                          # the parameter of the model is kept
                    off.value = "false"
                    task.params[task.params.index(p)] = off
            _set(task, kind + ":N", "int", str(len(names)), Param)
            if kind not in TASK_SETTING_KINDS:
                _set(task, kind + ":USE", "bool", "true" if names else "false", Param)
            for k, name in enumerate(names):
                definition = library.get(kind, name)
                owner = (kind in GLAUBER_KINDS and is_glauber(task) and definition is not None
                         and (kind, name) not in glauber_owned)
                if owner:
                    glauber_owned.add((kind, name))
                _write_object(task, kind, k, name, definition, owner, Param)
        layout = histogram_layout(task, templates)
        if layout is None:
            continue
        options = histogram_options(task)
        names = binning_keys(task, templates)
        types = {}
        template = templates.get(task.cls)
        for p in (template.params if template else task.params):
            if p.key.startswith("HISTOGRAM:"):
                types[p.key[len("HISTOGRAM:"):]] = p.type
        all_folders = [f for f in folders(task, templates) if list_used(task, f.list)]      # none for unused lists
        counts = {}
        for f in all_folders:
            counts[f.list] = counts.get(f.list, 0) + 1
        for list_index in folder_lists(task, templates):
            _set(task, list_key(list_index) + ":N", "int", str(counts.get(list_index, 0)), Param)
        for f in all_folders:
            prefix = folder_key(f.list, f.index)
            _set(task, prefix + "NAME", "string", f.name, Param)
            _set(task, prefix + "TITLE", "string", f.name, Param)
            _set(task, prefix + "OWNER", "bool", "true", Param)
            if options.same_binning:
                continue
            overrides = options.binning.get(f.combination, {})
            for name in names:
                value = overrides.get(name, _param_value(task, "HISTOGRAM:" + name, ""))
                _set(task, prefix + name, types.get(name, "string"), value, Param)


def extract_choices(tasks, templates, library=None):
    """Read from the parameters of the tasks their objects and histogram folders: fill the library (objects defined
    by owners, unless already in it), set the choices and histogram options of the tasks, and remove the parameters
    managed by this model. @return library, warnings"""
    library = library if library is not None else FilterLibrary()
    warnings = []
    for task in _preorder(tasks):
        values = {p.key: p.value for p in task.params}
        kinds = task_kinds(task, templates) or [k for k in KINDS if k + ":N" in values]
        task.filters = {}
        for kind in kinds:
            try:
                n = int(values.get(kind + ":N", "0") or 0)
            except ValueError:
                n = 0
            names = []
            for k in range(n):
                prefix = "%s_%d:" % (kind, k)
                name = values.get(prefix + "NAME", "")
                if not name:
                    warnings.append("Task %s: %s_%d has no name (ignored)." % (task.name, kind, k))
                    continue
                names.append(name)
                if values.get(prefix + "OWNER", "false").lower() not in ("true", "1", "yes", "on"):
                    continue
                definition = _definition_from_keys(task, kind, k, name, values, warnings)
                if kind in MAPS_KINDS and any(key.startswith(prefix) and key[len(prefix):] not in ("NAME", "TITLE", "OWNER")
                                              for key in values):
                    warnings.append("%s %s: its maps are read from the maps file now (Filters pane, Detector maps); "
                                    "the parameters of this file are ignored." % (MAPS_LABELS[kind], name))
                existing = library.get(kind, name)
                if existing is None:
                    library.add(definition)
                    if task.cls != CREATOR_CLASS and kind in SETTINGS_KINDS \
                            and library.settings[kind] == default_settings(kind):
                        for setting, _, _ in LIST_SETTINGS:       # settings of the owner (now FilterCreator's)
                            if kind + ":" + setting in values:
                                library.settings[kind][setting] = values[kind + ":" + setting]
                elif not existing.source and not existing.same_definition(definition):
                    warnings.append("Task %s: %s %s is defined differently by an earlier task or the library (the first "
                                    "definition is kept)." % (task.name, KIND_LABELS[kind].lower()[:-1], name))
            if names:
                task.filters[kind] = names
        layout = histogram_layout(task, templates)
        for l in folder_lists(task, templates):           # folders without USE (USE was not read by CAP): used
            n = values.get(list_key(l) + ":N", "0") or "0"
            p = task.param(list_key(l) + ":USE")
            if n.isdigit() and int(n) > 0 and p is not None and p.value != "true":
                p.value = "true"
        if layout is not None:
            previous = getattr(task, "histograms", None)
            task.histograms = previous if previous is not None else HistogramOptions()
            _histograms_from_keys(task, templates, values, warnings)
        strip_managed(task)
    for task in _preorder(tasks):
        for kind, names in choices(task).items():
            for name in names:
                if library.get(kind, name) is None:
                    warnings.append("Task %s: %s %s is not defined by any task (added to the library, empty)."
                                    % (task.name, KIND_LABELS[kind].lower()[:-1], name))
                    library.add(FilterDef(kind, name))
    return library, warnings


def remove_creator(tasks, library):
    """Top-level tasks without FilterCreator (its objects being in the library, see extract_choices()), whose
    settings of the lists of objects go to the library."""
    kept = []
    for task in tasks:
        if task.cls != CREATOR_CLASS:
            kept.append(task)
            continue
        for sub in task.children:
            kind = next((k for k, (_, cls) in CREATORS.items() if cls == sub.cls), None)
            if kind is None:
                continue
            for name, _, _ in LIST_SETTINGS:
                p = sub.param(kind + ":" + name)
                if p is not None:
                    library.settings[kind][name] = p.value
    share_maps_file(library)
    return kept


def _definition_from_keys(task, kind, k, name, values, warnings):
    prefix = "%s_%d:" % (kind, k)
    title = values.get(prefix + "TITLE", "")
    if kind not in CONDITION_KINDS:
        parameters = {pname: values[prefix + pname] for pname, _, _ in KIND_PARAMETERS.get(kind, [])
                      if prefix + pname in values}
        return FilterDef(kind, name, title, parameters=parameters)
    conditions = []
    try:
        n = int(values.get(prefix + "CONDITION:N", "0") or 0)
    except ValueError:
        n = 0
    for i in range(n):
        cp = prefix + "CONDITION_%d:" % i

        def number(key):
            try:
                return float(values.get(cp + key, "0") or 0)
            except ValueError:
                warnings.append("Task %s: %s is not a number (0 used)." % (task.name, cp + key))
                return 0.0
        ctype = values.get(cp + "TYPE", "")
        form = FORM_OF_TYPE.get(ctype.upper(), ctype)
        conditions.append(Condition(values.get(cp + "SUBTYPE", ""), form, number("MINIMUM"), number("MAXIMUM"),
                                    number("MINIMUM2"), number("MAXIMUM2"), values.get(cp + "SELECTION", "")))
    return FilterDef(kind, name, title, conditions)


def _histograms_from_keys(task, templates, values, warnings):
    """Binning of the folders of a task from its keys (one folder per combination of its filters)."""
    options = task.histograms
    found = [f for f in folders(task, templates) if list_used(task, f.list)]
    stated = 0
    for l in folder_lists(task, templates):
        n = values.get(list_key(l) + ":N", "0") or "0"
        stated += int(n) if n.isdigit() else 0
    if stated and stated != len(found):
        warnings.append("Task %s: its histogram folders (%d) are not those of its filters (%d, one per combination): "
                        "they will be written as such." % (task.name, stated, len(found)))
    names = binning_keys(task, templates)
    per_folder = {}
    for f in found:
        if f.derived:
            continue
        prefix = folder_key(f.list, f.index)
        overrides = {n: values[prefix + n] for n in names if prefix + n in values}
        if overrides:
            per_folder[f.combination] = overrides
    if per_folder:
        options.same_binning = False
        defaults = {n: values.get("HISTOGRAM:" + n) for n in names}
        options.binning = {combo: {n: v for n, v in o.items() if v != defaults.get(n)} for combo, o in per_folder.items()}
        options.binning = {combo: o for combo, o in options.binning.items() if o}


# ------------------------------------------------------------------------------------------------------------------
# Checks
# ------------------------------------------------------------------------------------------------------------------

def rename_selection(library, old, new):
    """A particle filter renamed: the conditions of the event filters on its selection follow."""
    for o in library.objects["EVENT_FILTER"]:
        for c in o.conditions:
            if c.subtype in SELECTION_SUBTYPES and c.selection == old:
                c.selection = new


def selection_users(library, name):
    """Names of the event filters testing the selection of a particle filter."""
    return [o.name for o in library.objects["EVENT_FILTER"]
            if any(c.subtype in SELECTION_SUBTYPES and c.selection == name for c in o.conditions)]


def check_library(library):
    """Problems of the objects of the library (names, conditions, parameters, file of the particle databases)."""
    errors = []
    settings = library.settings["PARTICLE_DB"]
    if library.objects["PARTICLE_DB"] and settings.get("IMPORT") == "true" \
            and not is_database_file(settings.get("IMPORT:FILE_NAME", "")):
        errors.append("Particle databases are imported from an XML file (particles and decay modes): %s is not one."
                      % settings.get("IMPORT:FILE_NAME", ""))
    for kind in KINDS:
        names = library.names(kind)
        label = KIND_LABELS[kind].lower()[:-1]
        for name in sorted(set(n for n in names if names.count(n) > 1)):
            errors.append("Several %ss are named %s." % (label, name))
        for o in library.objects[kind]:
            if not NAME_FULL.fullmatch(o.name or ""):
                errors.append("Invalid name '%s' of a %s (letters, digits, _ . -; no spaces or colons)." % (o.name, label))
            for i, c in enumerate(o.conditions):
                accepted = dict(EVENT_SUBTYPES, **GLAUBER_SUBTYPES) if kind == "EVENT_FILTER" else subtypes_for(kind)
                if c.subtype not in accepted:
                    errors.append("%s %s, condition %d: unknown quantity '%s'." % (label.capitalize(), o.name, i + 1, c.subtype))
                elif c.form not in accepted[c.subtype][1]:
                    errors.append("%s %s, condition %d: %s cannot be tested as a %s." % (label.capitalize(), o.name, i + 1,
                                                                                       accepted[c.subtype][0], c.form))
                elif c.form in (RANGE, DUAL_RANGE) and not c.minimum < c.maximum:
                    errors.append("%s %s, condition %d: the minimum of %s is not below its maximum."
                                  % (label.capitalize(), o.name, i + 1, accepted[c.subtype][0]))
                elif c.form == DUAL_RANGE and not c.minimum2 < c.maximum2:
                    errors.append("%s %s, condition %d: the second minimum is not below the second maximum."
                                  % (label.capitalize(), o.name, i + 1))
                elif kind == "EVENT_FILTER" and c.subtype in SELECTION_SUBTYPES \
                        and c.selection not in library.names("PARTICLE_FILTER"):
                    errors.append("%s %s, condition %d: %s needs a particle filter of the library (%s)."
                                  % (label.capitalize(), o.name, i + 1, accepted[c.subtype][0],
                                     "'%s' is not one" % c.selection if c.selection else "none chosen"))
            for pname, ptype, _ in KIND_PARAMETERS.get(kind, []):
                value = o.parameters.get(pname, "")
                if not _valid(ptype, value):
                    errors.append("%s %s: invalid %s value '%s' for %s." % (label.capitalize(), o.name, ptype, value, pname))
    for path in library.links:
        if not os.path.isfile(path):
            errors.append("The linked selection file %s is missing: its objects are not in the workflow." % path)
    if any(library.objects[k] for k in MAPS_KINDS) and not maps_file(library):
        errors.append("The particle efficiencies and smearers are read from a maps file: choose it (Filters pane, "
                      "Detector maps).")
    return errors


def _valid(ptype, value):
    if ptype == "bool":
        return value in ("true", "false")
    if ptype in ("int", "long"):
        return re.fullmatch(r"[+-]?\d+", value or "") is not None
    if ptype == "double":
        try:
            float(value)
            return True
        except (TypeError, ValueError):
            return False
    return True


def check_job(tasks, library, templates):
    """Problems of the files of the job: histograms imported without input file, input file written over."""
    errors = []
    importing = [t.name for t in _preorder(tasks)
                 if any(list_used(t, l) and _flag(t, l, "IMPORT") for l in folder_lists(t, templates))]
    input_file = library.job.get("INPUT_FILE", "")
    if importing and not input_file:
        errors.append("%s import%s histograms: name the input file of the job (the output file of the fill run)."
                      % (", ".join(importing), "s" if len(importing) == 1 else ""))
    if input_file and input_file == library.job.get("OUTPUT_FILE", ""):
        errors.append("The input and output files of the job are the same file (%s): the output would replace it."
                      % input_file)
    return errors


def short_class(cls):
    return cls.replace("CAP::", "")


def _parents(tasks, parent=None):
    for t in tasks:
        yield t, parent
        yield from _parents(t.children, t)


def check_choices(tasks, library, templates):
    """Problems of the choices of the tasks: objects not in the library, conditions a task cannot test, too many
    particle filters, analyzers without filters, duplicate folder names."""
    errors = check_library(library)
    for task, parent in _parents(tasks):
        kinds = task_kinds(task, templates)
        for kind, names in choices(task).items():
            label = KIND_LABELS[kind].lower()[:-1]
            if kind not in kinds:
                errors.append("Task %s (%s) does not use %ss." % (task.name, task.cls, label))
                continue
            for name in names:
                definition = library.get(kind, name)
                if definition is None:
                    errors.append("Task %s: %s %s is not in the library." % (task.name, label, name))
                elif kind == "EVENT_FILTER":
                    accepted = subtypes_for(kind, is_glauber(task))
                    for c in definition.conditions:
                        if c.subtype in dict(EVENT_SUBTYPES, **GLAUBER_SUBTYPES) and c.subtype not in accepted:
                            errors.append("Task %s cannot test %s (event filter %s): %s." % (
                                task.name, dict(EVENT_SUBTYPES, **GLAUBER_SUBTYPES)[c.subtype][0], name,
                                "only the Glauber tasks can" if c.subtype in GLAUBER_SUBTYPES
                                else "a Glauber task tests impact parameter, participants, collisions or spectators"))
        if any(k in kinds for k in MAPS_KINDS):
            errors += check_maps_choices(task)
        if "EVENT" in kinds and _param_value(task, "EVENT:IMPORT", "false") == "true" \
                and _param_value(task, "EVENT:IMPORT:FILE_NAME", "NONE") in ("", "NONE"):
            errors.append("Task %s imports events but the file is not named." % task.name)
        if "EVENT" in kinds and _param_value(task, "EVENT:EXPORT", "false") == "true" \
                and _param_value(task, "EVENT:EXPORT:FILE_NAME", "NONE") in ("", "NONE"):
            errors.append("Task %s exports events but the file is not named." % task.name)
        mode = histogram_mode(task, templates)
        fills = any(list_used(task, l) and _flag(task, l, "CREATE") for l in folder_lists(task, templates) if l in (0, 1, 3))
        if fills and "EVENT" in kinds and not choices(task).get("EVENT"):
            errors.append("Task %s uses no event: choose one (Events)." % task.name)
        if is_calculator(task, templates):
            if parent is not None and parent.cls in ITERATOR_OF_MODE.values():
                errors.append("Task %s computes histograms from the output of an earlier job: put it outside of %s (a %s)."
                              % (task.name, parent.name, short_class(parent.cls)))
            if not _param_value(task, "SOURCE_TASK", ""):
                errors.append("Task %s: name the analyzer whose histograms it completes (SOURCE_TASK)." % task.name)
        elif mode in (FILL, DERIVED) and parent is not None and parent.cls != ITERATOR_OF_MODE[mode] \
                and parent.cls != "CAP::Task":
            errors.append("Task %s %s: put it in a%s (it is in %s, a %s)." % (
                task.name, "fills its histograms event by event" if mode == FILL
                else "computes its derived histograms from a file",
                "n EventIterator" if mode == FILL else " FileIterator", parent.name, short_class(parent.cls)))
        for l in folder_lists(task, templates):
            if l in (2, 4) and list_used(task, l) and not list_used(task, l - 1):
                errors.append("Task %s: its %s are computed from its %s: use them too (HISTOGRAM_%d)."
                              % (task.name, list_label(task, templates, l).lower(),
                                 list_label(task, templates, l - 1).lower(), l - 1))
        used = [l for l in folder_lists(task, templates) if list_used(task, l)]
        if used:
            if not choices(task).get("EVENT_FILTER") or not choices(task).get("PARTICLE_FILTER"):
                errors.append("Task %s fills histograms per event filter and particle filter: choose at least one of each."
                              % task.name)
            for l in used:
                label = list_label(task, templates, l).lower()
                flag = lambda name: _param_value(task, "%s:%s" % (list_key(l), name), "false") == "true"
                if not flag("CREATE") and not flag("IMPORT"):
                    errors.append("Task %s: its %s are used but neither created nor imported (%s)."
                                  % (task.name, label, list_key(l)))
            names = [f.name for f in folders(task, templates) if f.list in used]
            for name in sorted(set(n for n in names if names.count(n) > 1)):
                errors.append("Task %s: several histogram folders are named %s (a filter chosen twice?)." % (task.name, name))
    for task in _preorder(tasks):
        if "EVENT" in task_kinds(task, templates) and makes_events(task) and not any(
                _param_value(task, "EVENT:" + flag, "false") == "true" for flag in ("CREATE", "IMPORT")):
            errors.append("Task %s makes events but is set neither to create nor to import them: set Create (Events "
                          "line of its window)." % task.name)
    for task in _preorder(tasks):
        if task.cls == ITERATOR_OF_MODE[FILL]:
            made_below = events_made_below(task, templates)
            used = choices(task).get("EVENT", []) if _param_value(task, "EVENT:USE", "false") == "true" else []
            missing = [name for name in made_below if name not in used]
            if missing:
                errors.append("Event iterator %s does not use the events %s made by its tasks: it must use them (it "
                              "resets them before each event)." % (task.name, ", ".join(missing)))
    made = set()                                        # events created or imported by a task, or written by one
    for task in _preorder(tasks):
        if any(_param_value(task, "EVENT:" + flag, "false") == "true" for flag in ("CREATE", "IMPORT")):
            made.update(choices(task).get("EVENT", []))
        if written_event(task):
            made.add(written_event(task))
    for task in _preorder(tasks):
        if not writes_events(task):
            continue
        names = choices(task).get("EVENT", [])
        if len(names) != 2 or names[0] == names[1]:
            errors.append("Task %s reads one event and writes another (by default %s): choose two events, in this "
                          "order (Events)." % (task.name, EVENT_WRITERS[task.cls.replace("CAP::", "")]))
        elif _param_value(task, "EVENT:USE", "false") != "true":
            errors.append("Task %s must use its events (Events: Use)." % task.name)
        else:
            for other in _preorder(tasks):
                if other is not task and names[1] in choices(other).get("EVENT", []) and any(
                        _param_value(other, "EVENT:" + flag, "false") == "true" for flag in ("CREATE", "IMPORT")):
                    errors.append("Task %s writes the events %s, which task %s also makes: choose another event."
                                  % (task.name, names[1], other.name))
    for task in _preorder(tasks):
        if _param_value(task, "EVENT:USE", "false") == "true":
            for name in choices(task).get("EVENT", []):
                if name not in made:
                    errors.append("Task %s uses the events %s, which no task creates or imports (a generator or a "
                                  "reader creates them)." % (task.name, name))
    return errors


# ------------------------------------------------------------------------------------------------------------------
# RUNCAP element of the workflow file
# ------------------------------------------------------------------------------------------------------------------

def object_element(o):
    """XML element of an object of the library (see TAG_OF_KIND): its name, title, conditions or parameters."""
    element = ET.Element(TAG_OF_KIND[o.kind], {"name": o.name})
    if o.title:
        element.set("title", o.title)
    for c in o.conditions:
        attributes = {"quantity": c.subtype, "form": c.form}
        if c.form == VALUE:
            attributes["value"] = number_text(c.minimum)
        elif c.form == FLAG:
            attributes["value"] = "true" if c.minimum else "false"
        else:
            attributes.update(minimum=number_text(c.minimum), maximum=number_text(c.maximum))
            if c.form == DUAL_RANGE:
                attributes.update(minimum2=number_text(c.minimum2), maximum2=number_text(c.maximum2))
        if c.selection:
            attributes["selection"] = c.selection
        ET.SubElement(element, "Condition", attributes)
    for pname, _, _ in KIND_PARAMETERS.get(o.kind, []):
        ET.SubElement(element, "Parameter", {"name": pname}).text = o.parameters.get(pname, "")
    return element


def object_from_element(element, warnings, where=""):
    """Object of the library from its XML element (TAG_OF_KIND, or the OBJECT elements of earlier workflows), or None
    (a warning added) if it is not one. @param where name of the file or element, for the warnings"""
    where = where + ": " if where else ""
    if element.tag == "OBJECT":                                     # earlier versions
        kind = element.get("kind", "")
        if kind not in KINDS:
            warnings.append("%sunknown kind of object '%s' (ignored)." % (where, kind))
            return None
        conditions = []
        for c in element.findall("CONDITION"):
            try:
                conditions.append(Condition(c.get("subtype", ""), c.get("form", RANGE), c.get("minimum", "0"),
                                            c.get("maximum", "0"), c.get("minimum2", "0"), c.get("maximum2", "0"),
                                            c.get("selection", "")))
            except ValueError:
                warnings.append("%sinvalid condition of %s (ignored)." % (where, element.get("name", "?")))
        parameters = {p.get("name", ""): (p.text or "").strip() for p in element.findall("PARAMETER")}
        return FilterDef(kind, element.get("name", ""), element.get("title", ""), conditions, parameters)
    kind = KIND_OF_TAG.get(element.tag)
    if kind is None:
        warnings.append("%sunknown element <%s> (ignored)." % (where, element.tag))
        return None
    name = element.get("name", "")
    conditions = []
    for c in element.findall("Condition"):
        form = c.get("form", RANGE)
        try:
            if form == VALUE:
                value = c.get("value", c.get("minimum", "0"))
                condition = Condition(c.get("quantity", ""), form, value, value)
            elif form == FLAG:
                value = 1.0 if c.get("value", c.get("minimum", "true")).lower() in ("true", "1", "yes", "on") else 0.0
                condition = Condition(c.get("quantity", ""), form, value, value)
            else:
                condition = Condition(c.get("quantity", ""), form, c.get("minimum", "0"), c.get("maximum", "0"),
                                      c.get("minimum2", "0"), c.get("maximum2", "0"))
        except ValueError:
            warnings.append("%sa condition of %s is not a number (ignored)." % (where, name or "?"))
            continue
        condition.selection = c.get("selection", "")
        conditions.append(condition)
    parameters = {p.get("name", ""): (p.text or "").strip() for p in element.findall("Parameter")}
    unknown = sorted(set(parameters) - set(n for n, _, _ in KIND_PARAMETERS.get(kind, [])))
    if unknown:
        warnings.append("%s%s %s: unknown parameter%s %s (ignored)." % (where, KIND_LABELS[kind].lower()[:-1], name,
                                                                          "s" if len(unknown) > 1 else "",
                                                                          ", ".join(unknown)))
    return FilterDef(kind, name, element.get("title", ""), conditions, parameters)


def selections_element(objects, extras=()):
    """Root element of a selection file holding objects (and other elements, written as they are)."""
    root = ET.Element(SELECTIONS_ROOT, {"version": "1"})
    for o in objects:
        root.append(object_element(o))
    for e in extras:
        root.append(e)
    return root


def _xml_bytes(root):
    root = copy.deepcopy(root)
    _indent_xml(root)
    return b'<?xml version="1.0" encoding="UTF-8"?>\n' + ET.tostring(root, encoding="utf-8") + b"\n"


def _indent_xml(element, level=0):
    pad = "\n" + "  " * level
    if len(element):
        if not (element.text or "").strip():
            element.text = pad + "  "
        for child in element:
            _indent_xml(child, level + 1)
        if not (child.tail or "").strip():
            child.tail = pad
    if level and not (element.tail or "").strip():
        element.tail = pad


def write_selections(path, objects, extras=()):
    """Write a selection file: the objects (kind by kind, in order) and other elements. @throws OSError"""
    with open(path, "wb") as f:
        f.write(_xml_bytes(selections_element(objects, extras)))


def read_selections(path):
    """Objects of a selection file (or of the RUNCAP element of a workflow file). @return objects, the other elements
    of the file (kept as they are), warnings @throws OSError, ValueError if it is not a file of selections"""
    try:
        root = ET.parse(path).getroot()
    except ET.ParseError as error:
        raise ValueError("%s is not an XML file: %s" % (path, error))
    if root.tag == "CAP":                                       # a workflow: its RUNCAP element
        root = root.find("RUNCAP")
        if root is None:
            raise ValueError("%s is a workflow without filters (no RUNCAP element)." % path)
    elif root.tag != SELECTIONS_ROOT:
        raise ValueError("%s is not a selection file (root <%s>, not <%s>)." % (path, root.tag, SELECTIONS_ROOT))
    warnings, objects, extras, seen = [], [], [], set()
    where = os.path.basename(path)
    for element in root:
        maps = KIND_OF_TAG.get(element.tag) in MAPS_KINDS or (element.tag == "OBJECT" and element.get("kind") in MAPS_KINDS)
        if maps or (element.tag not in KIND_OF_TAG and element.tag != "OBJECT"):
            if root.tag == SELECTIONS_ROOT:
                extras.append(element)          # kept as they are: particle maps (from the maps file), later versions
            continue
        o = object_from_element(element, warnings, where)
        if o is None:
            continue
        if (o.kind, o.name) in seen:
            warnings.append("%s: two %ss named %s (the first is kept)." % (where, KIND_LABELS[o.kind].lower()[:-1],
                                                                          o.name))
            continue
        seen.add((o.kind, o.name))
        objects.append(o)
    return objects, extras, warnings


def save_linked(library):
    """Write the objects that come from linked selection files back to them (only the files whose content changed).
    @return paths written @throws OSError"""
    written = []
    for path in library.links:
        objects = library.linked(path) + library.link_shadowed.get(path, [])
        order = {key: k for k, key in enumerate(library.link_order.get(path, []))}
        objects.sort(key=lambda o: order.get((o.kind, o.name), len(order)))   # the order of the file, new ones last
        data = _xml_bytes(selections_element(objects, library.link_extras.get(path, ())))
        try:
            with open(path, "rb") as f:
                if f.read() == data:
                    continue
        except OSError:
            pass
        with open(path, "wb") as f:
            f.write(data)
        written.append(path)
    return written


def _relative(path, base_dir):
    if not base_dir:
        return path
    try:
        return os.path.relpath(path, base_dir)
    except ValueError:                                          # another drive (Windows)
        return path


def runcap_element(library, tasks, base_dir=None):
    """The RUNCAP element: the library (its own objects, and Include elements for the selection files it links, with
    paths relative to base_dir, the directory of the workflow) and the histogram options of the tasks."""
    root = ET.Element("RUNCAP", {"version": "2"})
    for path in library.links:
        ET.SubElement(root, "Include", {"file": _relative(path, base_dir)})
    for kind in KINDS:
        for o in library.objects[kind]:
            if not o.source:
                root.append(object_element(o))
    if any(library.job.values()):
        ET.SubElement(root, "JOB", {name.lower(): value for name, value in library.job.items() if value})
    for kind in KINDS:
        changed = {n: v for n, v in library.settings[kind].items() if v != default_settings(kind).get(n)}
        if changed:
            element = ET.SubElement(root, "SETTINGS", {"kind": kind})
            for name, value in sorted(changed.items()):
                ET.SubElement(element, "PARAMETER", {"name": name}).text = value
    for task in _preorder(tasks):
        options = getattr(task, "histograms", None)
        if options is None:
            continue
        element = ET.SubElement(root, "HISTOGRAMS", {"task": task.name,
                                                     "same_binning": "true" if options.same_binning else "false"})
        for combo, overrides in sorted(options.binning.items()):
            c = ET.SubElement(element, "COMBINATION", {"filters": ",".join(combo)})
            for name, value in sorted(overrides.items()):
                ET.SubElement(c, "PARAMETER", {"name": name}).text = value
    return root


def read_runcap_element(element, tasks, base_dir=None):
    """Library and histogram options of the tasks from a RUNCAP element (its objects, those of the selection files it
    includes, paths relative to base_dir, and the OBJECT elements of earlier versions). @return library, warnings"""
    library = FilterLibrary()
    warnings = []
    for e in element:                                   # the objects of the workflow, then those of the linked files
        if e.tag in KIND_OF_TAG or e.tag == "OBJECT":
            o = object_from_element(e, warnings, "RUNCAP")
            if o is not None:
                library.add(o)
    for e in element:
        if e.tag == "Include":
            path = e.get("file", "")
            if not os.path.isabs(path) and base_dir:
                path = os.path.join(base_dir, path)
            path = os.path.abspath(path)
            try:
                warnings += library.link(path)
            except (OSError, ValueError) as error:
                warnings.append("Selection file %s cannot be read (%s): its objects are missing; it stays linked."
                                % (path, error))
                if path not in library.links:
                    library.links.append(path)
    job = element.find("JOB")
    if job is not None:
        for name in JOB_SETTINGS:
            library.job[name] = job.get(name.lower(), "")
    for e in element.findall("SETTINGS"):
        if e.get("kind") in KINDS:
            for p in e.findall("PARAMETER"):
                library.settings[e.get("kind")][p.get("name", "")] = (p.text or "").strip()
    by_name = {t.name: t for t in _preorder(tasks)}
    for h in element.findall("HISTOGRAMS"):
        task = by_name.get(h.get("task", ""))
        if task is None:
            warnings.append("RUNCAP: histogram options of an unknown task '%s' (ignored)." % h.get("task", ""))
            continue
        binning = {}
        for c in h.findall("COMBINATION"):
            binning[tuple(c.get("filters", "").split(","))] = {p.get("name", ""): (p.text or "").strip()
                                                               for p in c.findall("PARAMETER")}
        task.histograms = HistogramOptions(h.get("same_binning", "true") == "true", binning)
    return library, warnings


def prepared_for_writing(tasks, library, templates, Param, TaskConfig):
    """The task FilterCreator (creator_task()), then copies of the tasks with the keys of their objects and folders
    (apply_choices()); the tasks are unchanged."""
    def duplicate(task):        # the lists are copied, not the parameters (which may hold widgets of the GUI)
        c = copy.copy(task)
        c.params = list(task.params)
        c.children = [duplicate(t) for t in task.children]
        return c
    copies = [duplicate(t) for t in tasks if t.cls != CREATOR_CLASS]
    apply_choices(copies, library, templates, Param)
    return [creator_task(library, templates, TaskConfig, Param)] + copies


# ------------------------------------------------------------------------------------------------------------------
# Particle efficiencies and smearers: read from a maps file (CapGui, Detector Maps)
# ------------------------------------------------------------------------------------------------------------------

#: Kinds read from a maps file: RunCAP does not define them, it lists those of the file (bin/CAP --maps-list) and the
#: tasks choose them by name. FilterCreator imports them (IMPORT from the file, CREATE off), each from the folder of
#: its name (KinematicMapFolder in CAP: the folders are self-describing).
MAPS_KINDS = ("PARTICLE_EFFICIENCY", "PARTICLE_SMEARER")
MAPS_LABELS = {"PARTICLE_EFFICIENCY": "Particle efficiency", "PARTICLE_SMEARER": "Particle smearer"}
#: Program listing the objects of a maps file.
CAP_PROGRAM = os.path.join(os.path.dirname(os.path.abspath(__file__)), "bin", "CAP")
#: How a task uses the efficiencies (or smearers) it chooses, by the number of names chosen (see EventProcessor::
#: objectFor() in CAP): none, one for all the particles, or one per event filter and particle filter.
NO_MAPS, ONE_FOR_ALL, PER_FILTERS = "none", "one", "per filters"


class MapsEntry:
    """An object of a maps file: kind, name, species (PDG codes, text) and the sources of its maps (text)."""

    def __init__(self, kind, name, species="", maps=""):
        self.kind, self.name, self.species, self.maps = kind, name, species, maps

    def __repr__(self):
        return "MapsEntry(%s, %s)" % (self.kind, self.name)


def maps_file(library):
    """The maps file of a workflow (from the import settings of its efficiencies and smearers), or ""."""
    for kind in MAPS_KINDS:
        settings = library.settings[kind]
        name = settings.get("IMPORT:FILE_NAME", "")
        if settings.get("IMPORT") == "true" and name not in ("", "NONE"):
            path = settings.get("IMPORT:PATH", "")
            return name if path in ("", "DEFAULT") else os.path.join(path, name)
    return ""


def set_maps_file(library, path):
    """Read the efficiencies and smearers from a maps file (imported by FilterCreator, never created nor exported);
    an empty path: none."""
    for kind in MAPS_KINDS:
        settings = library.settings[kind]
        settings.update({"CREATE": "false", "EXPORT": "false"})
        if path:
            settings.update({"IMPORT": "true", "IMPORT:PATH": os.path.dirname(os.path.abspath(path)),
                             "IMPORT:FILE_NAME": os.path.basename(path)})
        else:
            settings.update({"IMPORT": "false", "IMPORT:PATH": "DEFAULT", "IMPORT:FILE_NAME": "NONE"})


def share_maps_file(library):
    """One maps file for both kinds: the import settings of the kind that has one (the other may have been written
    off, having no object) go to the other."""
    for kind in MAPS_KINDS:
        settings = library.settings[kind]
        if settings.get("IMPORT") == "true" and settings.get("IMPORT:FILE_NAME", "") not in ("", "NONE"):
            for other in MAPS_KINDS:
                for name in ("CREATE", "IMPORT", "IMPORT:PATH", "IMPORT:FILE_NAME", "EXPORT"):
                    library.settings[other][name] = settings.get(name, default_settings(other)[name])
            return


def parse_maps_list(text):
    """Objects of a maps file from the output of bin/CAP --maps-list: [MapsEntry]."""
    entries = []
    for line in text.splitlines():
        fields = line.rstrip("\n").split("\t")
        if len(fields) >= 3 and fields[0] == "MAP" and fields[1] in MAPS_KINDS:
            entries.append(MapsEntry(fields[1], fields[2], fields[3] if len(fields) > 3 else "",
                                     fields[4] if len(fields) > 4 else ""))
    return entries


def list_maps(path, program=None):
    """Objects of a maps file, read by bin/CAP --maps-list. @return [MapsEntry]
    @throws RuntimeError if the file cannot be read"""
    program = program or CAP_PROGRAM
    if not os.path.isfile(path):
        raise RuntimeError("No maps file %s." % path)
    if not os.path.isfile(program):
        raise RuntimeError("%s not found: build CAP first (BuildCAP)." % program)
    result = subprocess.run([program, "--maps-list", path], capture_output=True, text=True)
    entries = parse_maps_list(result.stdout)
    if result.returncode != 0:
        raise RuntimeError("Cannot read %s:\n%s" % (path, (result.stdout + result.stderr).strip()[-600:]))
    return entries


def sync_maps(library, entries):
    """The efficiencies and smearers of the library become those of a maps file (in its order).
    @param entries [MapsEntry] @return names no longer in the file: {kind: [name]}"""
    removed = {}
    for kind in MAPS_KINDS:
        names = [e.name for e in entries if e.kind == kind]
        removed[kind] = [n for n in library.names(kind) if n not in names]
        library.objects[kind] = [library.get(kind, n) or FilterDef(kind, n) for n in names]
    return removed


def maps_mode(task, kind):
    """How a task uses its objects of a kind: NO_MAPS, ONE_FOR_ALL or PER_FILTERS (several names)."""
    n = len(choices(task).get(kind, []))
    return NO_MAPS if n == 0 else ONE_FOR_ALL if n == 1 else PER_FILTERS


def filter_grid(task):
    """Names of the event filters and particle filters a task chooses: (event filters, particle filters)."""
    return list(choices(task).get("EVENT_FILTER", [])), list(choices(task).get("PARTICLE_FILTER", []))


def set_maps_mode(task, kind, mode, library):
    """Set how a task uses its objects of a kind: none, one for all (the first chosen, or the first of the library),
    or one per event filter and particle filter (event filter major, as its histogram folders; the cells take the
    name chosen until then)."""
    chosen = list(choices(task).get(kind, []))
    names = library.names(kind)
    first = chosen[0] if chosen else (names[0] if names else "")
    if mode == NO_MAPS or not first:
        choices(task).pop(kind, None)
        return
    if mode == ONE_FOR_ALL:
        choices(task)[kind] = [first]
        return
    events, particles = filter_grid(task)
    n = max(1, len(events)) * max(1, len(particles))
    choices(task)[kind] = [chosen[k] if len(chosen) == n else first for k in range(n)]


def regrid_maps(task, before):
    """After the filters of a task changed: its grids of efficiencies and smearers (one per event filter and particle
    filter) take the new size, each cell of a pair of filters still chosen keeping its name (the others take the first
    name of the grid). @param before filter_grid() of the task before the change"""
    after = filter_grid(task)
    if after == before:
        return
    old_events, old_particles = before
    for kind in MAPS_KINDS:
        chosen = choices(task).get(kind, [])
        nef, npf = max(1, len(old_events)), max(1, len(old_particles))
        if len(chosen) <= 1 or len(chosen) not in (nef * npf, npf):
            continue
        cells = {}
        for i, e in enumerate(old_events or [""]):
            for j, p in enumerate(old_particles or [""]):
                cells[(e, p)] = chosen[i * npf + j] if len(chosen) == nef * npf else chosen[j]
        events, particles = after
        choices(task)[kind] = [cells.get((e, p), chosen[0]) for e in (events or [""]) for p in (particles or [""])]


def remove_maps_names(task, kind, names, library):
    """Objects of a kind no longer in the library: in the choices of a task, replaced by the first one left (a grid
    keeps its size), or none if none is left."""
    chosen = choices(task).get(kind, [])
    if not set(chosen) & set(names):
        return
    left = library.names(kind)
    if not left:
        choices(task).pop(kind, None)
    else:
        choices(task)[kind] = [left[0] if n in names else n for n in chosen]


def check_maps_choices(task):
    """Problems of the numbers of efficiencies and smearers a task chooses: [message]."""
    events, particles = filter_grid(task)
    nef, npf = max(1, len(events)), max(1, len(particles))
    problems = []
    for kind in MAPS_KINDS:
        n = len(choices(task).get(kind, []))
        if n not in (0, 1, npf, nef * npf):
            problems.append("Task %s: %d %s for %d event filter%s and %d particle filter%s: choose one for all, or one "
                            "per event filter and particle filter (%d)." % (
                                task.name, n, KIND_LABELS[kind].lower(), nef, "s" if nef > 1 else "", npf,
                                "s" if npf > 1 else "", nef * npf))
    return problems


#: Catalog of the functions of CAP (written by bin/CAP --function-catalog; see function_catalog()).
FUNCTION_CATALOG_FILE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "DB", "FunctionCatalog.xml")


class CatalogFunction:
    """A function of the catalog: name, rank, composite, and its variants {number of parameters: [(name, value)]}."""

    def __init__(self, name, rank, composite=False, variants=None, default=None):
        self.name = name
        self.rank = rank
        self.composite = composite
        self.variants = dict(variants or {})
        self.default = default if default is not None else (min(self.variants) if self.variants else 0)

    def parameters(self, n=None):
        """[(name, starting value)] of the variant with n parameters (default: the default variant)."""
        return self.variants.get(self.default if n is None else n, [])


_catalog = None


def load_function_catalog(path=FUNCTION_CATALOG_FILE):
    """Functions of a catalog file: {name: CatalogFunction} ({} if the file is missing or not a catalog)."""
    try:
        root = ET.parse(path).getroot()
    except (OSError, ET.ParseError):
        return {}
    if root.get("type") != "function-catalog":
        return {}
    catalog = {}
    for f in root.iter("FUNCTION"):
        variants, default = {}, None
        for v in f.iter("VARIANT"):
            n = int(v.get("nParameters", "0"))
            variants[n] = [(p.get("name", "p%d" % k), p.get("value", "0")) for k, p in enumerate(v.iter("PARAMETER"))]
            if v.get("default") == "true":
                default = n
        catalog[f.get("name")] = CatalogFunction(f.get("name"), int(f.get("rank", "1")), f.get("composite") == "true",
                                                 variants, default)
    return catalog


def function_catalog():
    """The catalog of the functions of CAP, read once from FUNCTION_CATALOG_FILE ({} if there is none yet)."""
    global _catalog
    if _catalog is None:
        _catalog = load_function_catalog()
    return _catalog


def set_function_catalog(catalog):
    """Use another catalog (None: read FUNCTION_CATALOG_FILE again when next needed)."""
    global _catalog
    _catalog = catalog


