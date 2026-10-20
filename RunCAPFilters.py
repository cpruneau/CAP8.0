# **********************************************************************
# Copyright (C) 2019-2026, Claude Pruneau
# All rights reserved.
#
# For the licensing terms see LICENSE.
# **********************************************************************
"""
RunCAPFilters -- the filters of a CAP workflow and the histogram folders of its analyzers (model of RunCAP.py, no GUI).

A workflow has a library of named objects, shared by its tasks, of six kinds:
  * event filters and particle filters: each a list of conditions (a range, two ranges, a value or a flag of one
    quantity: multiplicity, transverse momentum, pseudorapidity, PDG code, ...);
  * event and particle efficiencies, event and particle smearers: each its binning parameters.
Each task chooses, for each kind it supports, how many objects it uses and which ones (by name). Analyzers fill one
folder of histograms per combination of their filters: (event filter, particle filter) for single-particle analyzers,
(event filter, particle filter 1, particle filter 2) for pair analyzers (with their single-particle folders too). The
folders are named <prefix>_<event filter>_<particle filter>[_<particle filter 2>] (derived folders end with _D), and
their binning is either the same for all (the HISTOGRAM group of the task) or set folder by folder.

CAP reads all this from flat keys of the configuration of each task (see ManagedObjects::configureObjects() and the
configure() functions of the filters, efficiencies, smearers and histogram folders):
    <task>:<KIND>:N, <task>:<KIND>:USE
    <task>:<KIND>_<k>:NAME, TITLE, OWNER
    <task>:<KIND>_<k>:CONDITION:N
    <task>:<KIND>_<k>:CONDITION_<i>:TYPE, SUBTYPE, MINIMUM, MAXIMUM[, MINIMUM2, MAXIMUM2]
    <task>:<KIND>_<k>:<parameter>                              (efficiencies, smearers)
    <task>:HISTOGRAM_<l>:N, <task>:HISTOGRAM_<l>_<k>:NAME, TITLE, OWNER, BASE_NAME[, <binning parameter>]
apply_choices() writes these keys from the library and the choices of the tasks; extract_choices() reads them back
(library, choices and histogram options), so workflows written by hand or by earlier versions load too. The object of
a name is defined (OWNER true, with its conditions or parameters) by the first task that uses it, in the order CAP
configures the tasks (a task, then its subtasks); the other tasks take it by name from CAP's store.

The library itself, including the objects no task uses, and the histogram options of the tasks are also written in a
RUNCAP element of the workflow file, next to its TASK element (CAP reads only the TASK elements).

No tkinter here: usable and testable without a display.
"""

import copy
import re
import xml.etree.ElementTree as ET

#: Kinds of objects of the library, in the order of the tabs of the Filters panel.
KINDS = ["EVENT_FILTER", "PARTICLE_FILTER", "EVENT_EFFICIENCY", "PARTICLE_EFFICIENCY", "EVENT_SMEARER", "PARTICLE_SMEARER"]
KIND_LABELS = {"EVENT_FILTER": "Event filters", "PARTICLE_FILTER": "Particle filters",
               "EVENT_EFFICIENCY": "Event efficiencies", "PARTICLE_EFFICIENCY": "Particle efficiencies",
               "EVENT_SMEARER": "Event smearers", "PARTICLE_SMEARER": "Particle smearers"}
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

#: The C++ code handles at most this many particle filters per task (EventProcessor::acceptedParticles()).
MAX_PARTICLE_FILTERS = 8
#: Suffix of the names of the folders of derived histograms.
DERIVED_SUFFIX = "_D"
#: Names of objects: as task names, without spaces or colons (they become parts of keys and histogram names).
NAME_FULL = re.compile(r"[A-Za-z_][A-Za-z0-9_.-]*")
#: Keys of the parameters of a task managed by this model (rewritten by apply_choices()).
MANAGED_KEY = re.compile(r"^(?:(?:%s)(?::N|:USE|_\d+:.*)|HISTOGRAM_[1-4](?::N|_\d+:.*))$" % "|".join(KINDS))
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
                "PARTICLE_SMEARER": "ParticleSmearer"}[kind]
        k = 0
        while len(items) < n:
            name = "%s%d" % (base, k)
            if self.get(kind, name) is None:
                items.append(FilterDef(kind, name))
            k += 1

    def __eq__(self, other):
        return isinstance(other, FilterLibrary) and self.objects == other.objects


# ------------------------------------------------------------------------------------------------------------------
# Choices of the tasks and their histogram folders
# ------------------------------------------------------------------------------------------------------------------

class HistogramOptions:
    """Histogram folders of an analyzer: prefix of their names; same binning for all (the HISTOGRAM group of the task),
    or binning per combination of filters ({combination (tuple of filter names): {parameter: value}}, values missing
    from a combination being those of the HISTOGRAM group)."""

    def __init__(self, prefix="", same_binning=True, binning=None):
        self.prefix = prefix
        self.same_binning = same_binning
        self.binning = dict(binning or {})

    def __eq__(self, other):
        return isinstance(other, HistogramOptions) and vars(self) == vars(other)

    def __repr__(self):
        return "HistogramOptions(%r, same=%r, %d combinations)" % (self.prefix, self.same_binning, len(self.binning))


class Folder:
    """A folder of histograms of an analyzer: list (1 to 4), index in the list, name, combination of filters, and
    whether it holds derived histograms."""

    def __init__(self, list_index, index, name, combination, derived):
        self.list = list_index
        self.index = index
        self.name = name
        self.combination = combination
        self.derived = derived

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
    """Histogram options of a task (created with the task name as prefix if needed)."""
    if not hasattr(task, "histograms") or task.histograms is None:
        task.histograms = HistogramOptions(task.name)
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


def folders(task, templates):
    """Folders of histograms of a task, in the order of their lists and indices (as the C++ code indexes them:
    e*nP + p for single folders, e*nP*nP + p1*nP + p2 for pair folders)."""
    layout = histogram_layout(task, templates)
    if layout is None:
        return []
    prefix = histogram_options(task).prefix or task.name
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


def apply_choices(tasks, library, templates, Param):
    """Write in the parameters of the tasks the keys of their objects and histogram folders (the parameters managed by
    this model are replaced). The tasks are changed in place: apply it to copies to keep the model.
    @param Param class of the parameters (RunCAP.Param)"""
    owned = set()                                       # (kind, name) defined by an earlier task
    for task in _preorder(tasks):
        strip_managed(task)
        kinds = task_kinds(task, templates)
        for kind in kinds:
            names = choices(task).get(kind, [])
            _set(task, kind + ":N", "int", str(len(names)), Param)
            _set(task, kind + ":USE", "bool", "true" if names else "false", Param)
            for k, name in enumerate(names):
                definition = library.get(kind, name)
                prefix = "%s_%d:" % (kind, k)
                owner = (kind, name) not in owned and definition is not None
                _set(task, prefix + "NAME", "string", name, Param)
                _set(task, prefix + "TITLE", "string", definition.title if definition else "", Param)
                _set(task, prefix + "OWNER", "bool", "true" if owner else "false", Param)
                if not owner:
                    continue
                owned.add((kind, name))
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
                    for pname, ptype, _ in KIND_PARAMETERS[kind]:
                        _set(task, prefix + pname, ptype, definition.parameters.get(pname, ""), Param)
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
        all_folders = folders(task, templates)
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
            _set(task, prefix + "BASE_NAME", "string", f.name, Param)
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
                elif existing != definition:
                    warnings.append("Task %s: %s %s is defined differently by an earlier task or the library (the first "
                                    "definition is kept)." % (task.name, KIND_LABELS[kind].lower()[:-1], name))
            if names:
                task.filters[kind] = names
        layout = histogram_layout(task, templates)
        if layout is not None:
            previous = getattr(task, "histograms", None)
            task.histograms = previous if previous is not None else HistogramOptions(task.name)
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


def _definition_from_keys(task, kind, k, name, values, warnings):
    prefix = "%s_%d:" % (kind, k)
    title = values.get(prefix + "TITLE", "")
    if kind not in CONDITION_KINDS:
        parameters = {pname: values[prefix + pname] for pname, _, _ in KIND_PARAMETERS[kind] if prefix + pname in values}
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
    """Prefix and binning of the folders of a task from its keys (folders named <prefix>_<filters...>)."""
    options = task.histograms
    found = folders(task, templates)
    first = values.get("HISTOGRAM_1_0:NAME")
    if first and found:
        suffix = "_" + "_".join(found[0].combination)
        if first.endswith(suffix) and len(first) > len(suffix):
            options.prefix = first[:-len(suffix)]
        found = folders(task, templates)
    renamed = [(f, values.get("HISTOGRAM_%d_%d:NAME" % (f.list, f.index))) for f in found]
    renamed = [(f, old) for f, old in renamed if old and old != f.name]
    stated = 0
    for l in (1, 2, 3, 4):
        n = values.get("HISTOGRAM_%d:N" % l, "0") or "0"
        stated += int(n) if n.isdigit() else 0
    if renamed or (stated and stated != len(found)):
        warnings.append("Task %s: its histogram folders (%d) are not those of its filters (%d, named %s_<filters>): they "
                        "will be written as such." % (task.name, stated, len(found), options.prefix))
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
    """Problems of the objects of the library (names, conditions, parameters)."""
    errors = []
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


def check_choices(tasks, library, templates):
    """Problems of the choices of the tasks: objects not in the library, conditions a task cannot test, too many
    particle filters, analyzers without filters, duplicate folder names."""
    errors = check_library(library)
    for task in _preorder(tasks):
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
        if histogram_layout(task, templates) is not None:
            if not choices(task).get("EVENT_FILTER") or not choices(task).get("PARTICLE_FILTER"):
                errors.append("Task %s fills histograms per event filter and particle filter: choose at least one of each."
                              % task.name)
            prefix = histogram_options(task).prefix
            if prefix and not NAME_FULL.fullmatch(prefix):
                errors.append("Task %s: invalid prefix '%s' of the histogram folders." % (task.name, prefix))
            for list_index in ((1, 3) if histogram_layout(task, templates) == "pair" else (1,)):
                if not any(_param_value(task, "HISTOGRAM_%d:%s" % (list_index, flag), "false") == "true"
                           for flag in ("CREATE", "IMPORT")):
                    errors.append("Task %s: the histograms of HISTOGRAM_%d are neither created nor imported (set "
                                  "HISTOGRAM_%d:CREATE or IMPORT)." % (task.name, list_index, list_index))
            names = [f.name for f in folders(task, templates)]
            for name in sorted(set(n for n in names if names.count(n) > 1)):
                errors.append("Task %s: several histogram folders are named %s (a filter chosen twice?)." % (task.name, name))
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
    for task in _preorder(tasks):
        options = getattr(task, "histograms", None)
        if options is None:
            continue
        element = ET.SubElement(root, "HISTOGRAMS", {"task": task.name, "prefix": options.prefix,
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
        task.histograms = HistogramOptions(h.get("prefix", task.name), h.get("same_binning", "true") == "true", binning)
    return library, warnings


def prepared_for_writing(tasks, library, templates, Param):
    """Copies of the tasks with the keys of their objects and folders (apply_choices()); the tasks are unchanged."""
    copies = copy.deepcopy(list(tasks))
    apply_choices(copies, library, templates, Param)
    return copies
