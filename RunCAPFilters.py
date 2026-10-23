# **********************************************************************
# Copyright (C) 2019-2026, Claude Pruneau
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

No tkinter here: usable and testable without a display.
"""

import copy
import re
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
#: Kinds defined by conditions (the others by parameters).
CONDITION_KINDS = ("EVENT_FILTER", "PARTICLE_FILTER")

#: Forms of a condition and the TYPE written for them.
RANGE, DUAL_RANGE, VALUE, FLAG = "range", "two ranges", "value", "flag"
FORMS = (RANGE, DUAL_RANGE, VALUE, FLAG)
TYPE_OF_FORM = {RANGE: "DOUBLE_RANGE", DUAL_RANGE: "DOUBLE_DUAL_RANGE", VALUE: "INTEGER", FLAG: "Bool"}
FORM_OF_TYPE = {"DOUBLE_RANGE": RANGE, "DOUBLE_DUAL_RANGE": DUAL_RANGE, "INTEGER": VALUE, "BOOL": FLAG}

#: Quantities of the conditions: {SUBTYPE: (description, forms accepted)}, as read by the configure() functions.
EVENT_SUBTYPES = {}
for _k in range(6):
    EVENT_SUBTYPES["MULT_%d" % _k] = ("multiplicity %d" % _k, (RANGE,))
    EVENT_SUBTYPES["ENERGY_%d" % _k] = ("energy %d" % _k, (RANGE,))
    EVENT_SUBTYPES["SPHERO_%d" % _k] = ("spherocity %d" % _k, (RANGE,))
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

#: Parameters of the efficiencies and smearers: {kind: [(name, type, default)]}, as read by their configure().
_TWO_PI = "6.283185307179586"
KIND_PARAMETERS = {
    "EVENT_EFFICIENCY": [("BASE_NAME", "string", ""), ("nBins_mult", "int", "100"), ("Min_mult", "double", "0"),
                         ("Max_mult", "double", "100")],
    "PARTICLE_EFFICIENCY": [("BASE_NAME", "string", ""), ("pt_nbins", "int", "100"), ("pt_min", "double", "0"),
                            ("pt_max", "double", "10"), ("eta_nbins", "int", "20"), ("eta_min", "double", "-1"),
                            ("eta_max", "double", "1"), ("phi_nbins", "int", "36"), ("phi_min", "double", "0"),
                            ("phi_max", "double", _TWO_PI)],
    "EVENT_SMEARER": [("BASE_NAME", "string", "")],
    "PARTICLE_SMEARER": [("BASE_NAME", "string", ""), ("n_nbins", "int", "100"), ("nSpecies", "bool", "false"),
                         ("pt_nbins", "int", "100"), ("pt_min", "double", "0"), ("pt_max", "double", "10"),
                         ("eta_nbins", "int", "20"), ("eta_min", "double", "-1"), ("eta_max", "double", "1"),
                         ("phi_nbins", "int", "36"), ("phi_min", "double", "0"), ("phi_max", "double", _TWO_PI)],
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
#: The C++ code handles at most this many particle filters per task (EventProcessor::acceptedParticles()).
MAX_PARTICLE_FILTERS = 8
#: Suffix of the names of the folders of derived histograms.
DERIVED_SUFFIX = "_D"
#: Names of objects: as task names, without spaces or colons (they become parts of keys and histogram names).
NAME_FULL = re.compile(r"[A-Za-z_][A-Za-z0-9_.-]*")
#: Keys of the parameters of a task managed by this model (rewritten by apply_choices()).
#: Kinds whose list settings (USE, CREATE, IMPORT, EXPORT and files) belong to each task: what the task does with its
#: events (a generator or reader creates them, an analyzer uses them). Those of the other kinds are FilterCreator's.
TASK_SETTING_KINDS = ("EVENT",)
MANAGED_KEY = re.compile(r"^(?:(?:%s)(?::N|_\d+:.*)|(?:%s):USE|HISTOGRAM_[1-4](?::N|_\d+:.*))$"
                         % ("|".join(KINDS), "|".join(k for k in KINDS if k not in TASK_SETTING_KINDS)))
#: Settings of the events of a task shown in its window.
EVENT_SETTINGS = ("USE", "CREATE", "IMPORT", "IMPORT:PATH", "IMPORT:FILE_NAME", "EXPORT", "EXPORT:PATH",
                  "EXPORT:FILE_NAME")
#: Tasks making events (generators, readers, models): they create them; the analyzers use them.
EVENT_SOURCES = ("BasicEventGen", "PythiaEventGenerator", "PythiaEventReader", "EposEventReader", "PHSDEventReader",
                 "Therminator3", "ThermalModel", "GlauberGenerator", "EventMerger")


def makes_events(task):
    """Whether a task makes events (a generator, a reader or a model)."""
    name = task.cls.replace("CAP::", "")
    return name in EVENT_SOURCES or name.endswith(("Gen", "Generator", "Reader"))


def default_event_settings(task, templates):
    """Events of a new task: created by a generator or reader, used by an analyzer, neither by the other tasks."""
    if "EVENT" not in task_kinds(task, templates):
        return
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
    """A condition of a filter on one quantity (SUBTYPE): a range [minimum, maximum), two ranges, a value or a flag."""

    def __init__(self, subtype, form=RANGE, minimum=0.0, maximum=0.0, minimum2=0.0, maximum2=0.0):
        self.subtype = subtype
        self.form = form
        self.minimum = float(minimum)
        self.maximum = float(maximum)
        self.minimum2 = float(minimum2)
        self.maximum2 = float(maximum2)

    @property
    def type(self):
        """TYPE written for CAP."""
        return TYPE_OF_FORM.get(self.form, self.form)

    def __eq__(self, other):
        return isinstance(other, Condition) and vars(self) == vars(other)

    def __repr__(self):
        return "Condition(%s, %s, %g, %g)" % (self.subtype, self.form, self.minimum, self.maximum)


class FilterDef:
    """An object of the library: kind, name, title, and its conditions (filters) or parameters (the others)."""

    def __init__(self, kind, name, title="", conditions=None, parameters=None):
        self.kind = kind
        self.name = name
        self.title = title or ""
        self.conditions = list(conditions or [])
        self.parameters = dict(default_parameters(kind))
        self.parameters.update(parameters or {})

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

    def __eq__(self, other):
        return (isinstance(other, FilterLibrary) and self.objects == other.objects and self.settings == other.settings
                and self.job == other.job)


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
        return "Folder(HISTOGRAM_%d_%d, %s)" % (self.list, self.index, self.name)


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
    """Folders of histograms of a task: "pair" (lists 1 to 4), "single" (lists 1 and 2) or None."""
    keys = _template_keys(task, templates)
    if "HISTOGRAM_3:N" in keys and "HISTOGRAM_1:N" in keys:
        return "pair"
    if "HISTOGRAM_1:N" in keys:
        return "single"
    return None


def binning_keys(task, templates):
    """Names of the binning parameters of the histograms of a task (the parameters of its HISTOGRAM group)."""
    template = templates.get(task.cls)
    params = template.params if template else task.params
    names = []
    for p in params:
        parts = p.key.split(":")
        if len(parts) == 2 and parts[0] == "HISTOGRAM" and parts[1] not in _LIST_ATTRIBUTES + ("BASE_NAME",):
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
    if layout == "pair":
        return [(e, p1, p2) for e in events for p1 in particles for p2 in particles] + singles
    if layout == "single":
        return singles
    return []


def folder_lists(task, templates):
    """Lists of folders of a task: [1, 2] (histograms, derived histograms) or, for a pair analyzer, [1, 2, 3, 4]
    (pairs, derived pairs, singles, derived singles); [] if it has no histograms."""
    layout = histogram_layout(task, templates)
    return [1, 2, 3, 4] if layout == "pair" else [1, 2] if layout == "single" else []


def list_label(task, templates, list_index):
    """Name of a list of folders, e.g. "Derived histograms"."""
    if histogram_layout(task, templates) == "pair":
        return {1: "Pair histograms", 2: "Derived pair histograms", 3: "Single histograms",
                4: "Derived single histograms"}[list_index]
    return {1: "Histograms", 2: "Derived histograms"}[list_index]


def list_used(task, list_index):
    """Whether a task uses a list of folders (HISTOGRAM_<l>:USE): it then has one folder per combination of its
    filters, none otherwise."""
    return _param_value(task, "HISTOGRAM_%d:USE" % list_index, "false") == "true"


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
    p = task.param("HISTOGRAM_%d:%s" % (list_index, name))
    if p is not None:
        p.value = value


def _flag(task, list_index, name):
    return _param_value(task, "HISTOGRAM_%d:%s" % (list_index, name), "false") == "true"


def set_histogram_mode(task, templates, mode):
    """Set the folder lists of an analyzer for a run: FILL (its histograms used and created; derived histograms not
    used) or DERIVED (its histograms used and imported from the input file of the job, the output file of the fill
    run; derived histograms used and created). The job writes them all to its output file."""
    for l in folder_lists(task, templates):
        base = l in (1, 3)
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
    base = [l for l in lists if l in (1, 3)]
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


def derived_workflow(tasks, templates, TaskConfig):
    """Workflow computing the derived histograms of the analyzers of a workflow from the histograms it exports: each
    task holding analyzers becomes a FileIterator of the same name holding them (set to DERIVED, same names and
    filters, so their folders and files match), the other tasks (generators, ...) are left out.
    @return top-level tasks (copies)"""
    def convert(task):
        analyzers = [_copy_task(c) for c in task.children if histogram_layout(c, templates) is not None]
        if histogram_layout(task, templates) is not None:
            analyzers = [_copy_task(task)]
            set_histogram_mode(analyzers[0], templates, DERIVED)
            if analyzers[0].param("EVENT:USE") is not None:
                analyzers[0].param("EVENT:USE").value = "false"
            return analyzers
        if not analyzers:
            return [t for c in task.children for t in convert(c)]
        template = templates.get(ITERATOR_OF_MODE[DERIVED])
        iterator = template.clone() if template is not None else TaskConfig(task.name, ITERATOR_OF_MODE[DERIVED])
        iterator.name = task.name
        for a in analyzers:
            set_histogram_mode(a, templates, DERIVED)
            use = a.param("EVENT:USE")
            if use is not None:
                use.value = "false"         # no event loop: histograms read from the input file of the job
        iterator.children = analyzers
        return [iterator]
    return [t for task in tasks for t in convert(task)]


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
    if layout == "pair":
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
            _set(task, cp + "MINIMUM", "double", number_text(c.minimum), Param)
            _set(task, cp + "MAXIMUM", "double", number_text(c.maximum), Param)
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
        for list_index in ([1, 2, 3, 4] if layout == "pair" else [1, 2]):
            _set(task, "HISTOGRAM_%d:N" % list_index, "int", str(counts.get(list_index, 0)), Param)
        for f in all_folders:
            prefix = "HISTOGRAM_%d_%d:" % (f.list, f.index)
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
                existing = library.get(kind, name)
                if existing is None:
                    library.add(definition)
                    if task.cls != CREATOR_CLASS and kind in SETTINGS_KINDS \
                            and library.settings[kind] == default_settings(kind):
                        for setting, _, _ in LIST_SETTINGS:       # settings of the owner (now FilterCreator's)
                            if kind + ":" + setting in values:
                                library.settings[kind][setting] = values[kind + ":" + setting]
                elif existing != definition:
                    warnings.append("Task %s: %s %s is defined differently by an earlier task or the library (the first "
                                    "definition is kept)." % (task.name, KIND_LABELS[kind].lower()[:-1], name))
            if names:
                task.filters[kind] = names
        layout = histogram_layout(task, templates)
        for l in folder_lists(task, templates):           # folders without USE (USE was not read by CAP): used
            n = values.get("HISTOGRAM_%d:N" % l, "0") or "0"
            p = task.param("HISTOGRAM_%d:USE" % l)
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
                                    number("MINIMUM2"), number("MAXIMUM2")))
    return FilterDef(kind, name, title, conditions)


def _histograms_from_keys(task, templates, values, warnings):
    """Binning of the folders of a task from its keys (one folder per combination of its filters)."""
    options = task.histograms
    found = [f for f in folders(task, templates) if list_used(task, f.list)]
    stated = 0
    for l in (1, 2, 3, 4):
        n = values.get("HISTOGRAM_%d:N" % l, "0") or "0"
        stated += int(n) if n.isdigit() else 0
    if stated and stated != len(found):
        warnings.append("Task %s: its histogram folders (%d) are not those of its filters (%d, one per combination): "
                        "they will be written as such." % (task.name, stated, len(found)))
    names = binning_keys(task, templates)
    per_folder = {}
    for f in found:
        if f.derived:
            continue
        prefix = "HISTOGRAM_%d_%d:" % (f.list, f.index)
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
            for pname, ptype, _ in KIND_PARAMETERS.get(kind, []):
                value = o.parameters.get(pname, "")
                if not _valid(ptype, value):
                    errors.append("%s %s: invalid %s value '%s' for %s." % (label.capitalize(), o.name, ptype, value, pname))
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
            if kind == "PARTICLE_FILTER" and len(names) > MAX_PARTICLE_FILTERS:
                errors.append("Task %s uses %d particle filters: CAP handles at most %d." % (task.name, len(names),
                                                                                          MAX_PARTICLE_FILTERS))
        if "EVENT" in kinds and _param_value(task, "EVENT:IMPORT", "false") == "true" \
                and _param_value(task, "EVENT:IMPORT:FILE_NAME", "NONE") in ("", "NONE"):
            errors.append("Task %s imports events but the file is not named." % task.name)
        if "EVENT" in kinds and _param_value(task, "EVENT:EXPORT", "false") == "true" \
                and _param_value(task, "EVENT:EXPORT:FILE_NAME", "NONE") in ("", "NONE"):
            errors.append("Task %s exports events but the file is not named." % task.name)
        mode = histogram_mode(task, templates)
        fills = any(list_used(task, l) and _flag(task, l, "CREATE") for l in folder_lists(task, templates) if l in (1, 3))
        if fills and "EVENT" in kinds and not choices(task).get("EVENT"):
            errors.append("Task %s uses no event: choose one (Events)." % task.name)
        if mode in (FILL, DERIVED) and parent is not None and parent.cls != ITERATOR_OF_MODE[mode] \
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
                flag = lambda name: _param_value(task, "HISTOGRAM_%d:%s" % (l, name), "false") == "true"
                if not flag("CREATE") and not flag("IMPORT"):
                    errors.append("Task %s: its %s are used but neither created nor imported (HISTOGRAM_%d)."
                                  % (task.name, label, l))
            names = [f.name for f in folders(task, templates) if f.list in used]
            for name in sorted(set(n for n in names if names.count(n) > 1)):
                errors.append("Task %s: several histogram folders are named %s (a filter chosen twice?)." % (task.name, name))
    made = set()                                        # events created or imported by a task
    for task in _preorder(tasks):
        if any(_param_value(task, "EVENT:" + flag, "false") == "true" for flag in ("CREATE", "IMPORT")):
            made.update(choices(task).get("EVENT", []))
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

def runcap_element(library, tasks):
    """The RUNCAP element: the library and the histogram options of the tasks."""
    root = ET.Element("RUNCAP", {"version": "1"})
    for kind in KINDS:
        for o in library.objects[kind]:
            element = ET.SubElement(root, "OBJECT", {"kind": kind, "name": o.name, "title": o.title})
            for c in o.conditions:
                attributes = {"subtype": c.subtype, "form": c.form, "minimum": number_text(c.minimum),
                              "maximum": number_text(c.maximum)}
                if c.form == DUAL_RANGE:
                    attributes.update(minimum2=number_text(c.minimum2), maximum2=number_text(c.maximum2))
                ET.SubElement(element, "CONDITION", attributes)
            for pname, ptype, _ in KIND_PARAMETERS.get(kind, []):
                ET.SubElement(element, "PARAMETER", {"name": pname, "type": ptype}).text = o.parameters.get(pname, "")
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


def read_runcap_element(element, tasks):
    """Library and histogram options of the tasks from a RUNCAP element. @return library, warnings"""
    library = FilterLibrary()
    warnings = []
    for o in element.findall("OBJECT"):
        kind = o.get("kind", "")
        if kind not in KINDS:
            warnings.append("RUNCAP: unknown kind of object '%s' (ignored)." % kind)
            continue
        conditions = []
        for c in o.findall("CONDITION"):
            try:
                conditions.append(Condition(c.get("subtype", ""), c.get("form", RANGE), c.get("minimum", "0"),
                                            c.get("maximum", "0"), c.get("minimum2", "0"), c.get("maximum2", "0")))
            except ValueError:
                warnings.append("RUNCAP: invalid condition of %s (ignored)." % o.get("name", "?"))
        parameters = {p.get("name", ""): (p.text or "").strip() for p in o.findall("PARAMETER")}
        library.add(FilterDef(kind, o.get("name", ""), o.get("title", ""), conditions, parameters))
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
