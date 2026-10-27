# **********************************************************************
# Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
# All rights reserved.
#
# For the licensing terms see LICENSE.
# **********************************************************************
"""Tests of RunCAPFilters (filters of RunCAP.py workflows), with the templates of DB/Templates.

Run from the CAP directory:  python3 -m unittest discover -s tests/python
"""

import os
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET

CAP_DIR = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, CAP_DIR)

import RunCAP                                   # noqa: E402
import RunCAPFilters as F                       # noqa: E402

TEMPLATES = RunCAP.load_templates(os.path.join(CAP_DIR, "DB", "Templates"))


def task(cls, name=None):
    t = TEMPLATES["CAP::" + cls].clone()
    if name:
        t.name = name
    return t


def values(t):
    return {p.key: p.value for p in t.params}


def library():
    lib = F.FilterLibrary.with_defaults()
    lib.add(F.FilterDef("EVENT_FILTER", "MB", "minimum bias", [F.Condition("MULT_0", F.RANGE, 0, 1e6)]))
    lib.add(F.FilterDef("EVENT_FILTER", "Central", "", [F.Condition("MULT_0", F.RANGE, 100, 1e6)]))
    lib.add(F.FilterDef("PARTICLE_FILTER", "PiP", "pi+", [F.Condition("PDG", F.VALUE, 211),
                                                         F.Condition("PT", F.RANGE, 0.2, 2.0),
                                                         F.Condition("ETA", F.DUAL_RANGE, -0.8, -0.1, 0.1, 0.8),
                                                         F.Condition("LIVE", F.FLAG, 1)]))
    lib.add(F.FilterDef("PARTICLE_FILTER", "PiM", "pi-", [F.Condition("PDG", F.VALUE, -211)]))
    lib.add(F.FilterDef("PARTICLE_FILTER", "Unused", "", [F.Condition("CHARGE", F.VALUE, 1)]))
    lib.add(F.FilterDef("PARTICLE_EFFICIENCY", "Eff", "", parameters={"pt_nbins": "50"}))
    return lib


def workflow():
    """EventIterator with a generator and two analyzers (single and pair)."""
    it = task("EventIterator")
    gen = task("BasicEventGen")
    single = task("ParticleSingleAnalyzer", "Single")
    pair = task("ParticlePairAnalyzer", "Pair")
    it.children = [gen, single, pair]
    for t in (it, gen, single, pair):
        F.default_event_settings(t, TEMPLATES)                      # the generator creates the events
    for t in (single, pair):
        F.default_histogram_settings(t, TEMPLATES)                  # histograms used, created, exported
        for l in (2, 4) if t is pair else (2,):
            t.param("HISTOGRAM_%d:USE" % l).value = "true"           # derived histograms used too
            t.param("HISTOGRAM_%d:CREATE" % l).value = "true"
    gen.filters = {"EVENT_FILTER": ["MB"], "PARTICLE_FILTER": ["PiP", "PiM"], "EVENT": ["Event"],
                   "PARTICLE_DB": ["ParticleDb"]}
    single.filters = {"EVENT_FILTER": ["MB", "Central"], "PARTICLE_FILTER": ["PiP", "PiM"],
                      "PARTICLE_EFFICIENCY": ["Eff"], "EVENT": ["Event"], "PARTICLE_DB": ["ParticleDb"]}
    pair.filters = {"EVENT_FILTER": ["MB"], "PARTICLE_FILTER": ["PiP", "PiM"], "EVENT": ["Event"],
                    "PARTICLE_DB": ["ParticleDb"]}
    it.filters = {"EVENT": ["Event"]}
    it.param("EVENT:USE").value = "true"                             # the iterator resets the events of the generator
    return [it]


class TestTemplates(unittest.TestCase):

    def test_kinds_and_layouts(self):
        self.assertIn("PARTICLE_FILTER", F.task_kinds(task("ParticleSingleAnalyzer"), TEMPLATES))
        self.assertEqual(F.task_kinds(task("GlauberGenerator"), TEMPLATES), ["EVENT_FILTER", "EVENT"])
        self.assertEqual(F.histogram_layout(task("ParticleSingleAnalyzer"), TEMPLATES), "single")
        self.assertEqual(F.histogram_layout(task("ParticlePairAnalyzer"), TEMPLATES), "pair")
        self.assertIsNone(F.histogram_layout(task("BasicEventGen"), TEMPLATES))
        self.assertIn("pt_nbins", F.binning_keys(task("ParticleSingleAnalyzer"), TEMPLATES))
        self.assertTrue(F.is_glauber(task("GlauberGenerator")))


class TestKeys(unittest.TestCase):

    def setUp(self):
        self.tasks = workflow()
        F.apply_choices(self.tasks, library(), TEMPLATES, RunCAP.Param)
        self.gen, self.single, self.pair = self.tasks[0].children

    def creator(self):
        creator = F.creator_task(library(), TEMPLATES, RunCAP.TaskConfig, RunCAP.Param)
        return creator, {t.cls: values(t) for t in creator.children}

    def test_creator(self):
        creator, subs = self.creator()
        self.assertEqual((creator.name, creator.cls), ("FilterCreator", "CAP::FilterCreator"))
        self.assertEqual([t.cls for t in creator.children], [c for _, c in F.CREATORS.values()])
        e = subs["CAP::EventFilterCreator"]
        self.assertEqual(e["EVENT_FILTER:N"], "2")
        self.assertEqual((e["EVENT_FILTER_0:NAME"], e["EVENT_FILTER_0:OWNER"]), ("MB", "true"))
        self.assertEqual(e["EVENT_FILTER_0:TITLE"], "minimum bias")
        self.assertEqual(e["EVENT_FILTER_1:CONDITION_0:MINIMUM"], "100")
        p = subs["CAP::ParticleFilterCreator"]
        self.assertEqual(p["PARTICLE_FILTER:N"], "3")                 # Unused too
        self.assertEqual(p["PARTICLE_FILTER_2:NAME"], "Unused")
        self.assertEqual(subs["CAP::EventSmearerCreator"]["EVENT_SMEARER:N"], "0")
        self.assertEqual(subs["CAP::ParticleEfficiencyCreator"]["PARTICLE_EFFICIENCY:CREATE"], "false")

    def test_tasks_by_name(self):
        g, s = values(self.gen), values(self.single)
        self.assertEqual(g["EVENT_FILTER:N"], "1")
        self.assertEqual(g["EVENT_FILTER:USE"], "true")
        self.assertEqual((g["EVENT_FILTER_0:NAME"], g["EVENT_FILTER_0:OWNER"]), ("MB", "false"))
        self.assertEqual(g["EVENT_FILTER_0:TITLE"], "minimum bias")
        self.assertNotIn("EVENT_FILTER_0:CONDITION:N", g)
        self.assertEqual((s["EVENT_FILTER_1:NAME"], s["EVENT_FILTER_1:OWNER"]), ("Central", "false"))
        self.assertEqual(s["PARTICLE_EFFICIENCY_0:OWNER"], "false")
        self.assertNotIn("PARTICLE_EFFICIENCY_0:pt_nbins", s)
        self.assertEqual(s["EVENT_SMEARER:N"], "0")
        self.assertEqual(s["EVENT_SMEARER:USE"], "false")

    def test_conditions(self):
        _, subs = self.creator()
        g = subs["CAP::ParticleFilterCreator"]
        p = "PARTICLE_FILTER_0:CONDITION_"
        self.assertEqual(g["PARTICLE_FILTER_0:CONDITION:N"], "4")
        self.assertEqual((g[p + "0:TYPE"], g[p + "0:SUBTYPE"], g[p + "0:MINIMUM"]), ("INTEGER", "PDG", "211"))
        self.assertEqual((g[p + "1:TYPE"], g[p + "1:MINIMUM"], g[p + "1:MAXIMUM"]), ("DOUBLE_RANGE", "0.2", "2"))
        self.assertEqual((g[p + "2:TYPE"], g[p + "2:MINIMUM2"], g[p + "2:MAXIMUM2"]), ("DOUBLE_DUAL_RANGE", "0.1", "0.8"))
        self.assertNotIn(p + "1:MINIMUM2", g)
        self.assertEqual(g[p + "3:TYPE"], "Bool")
        self.assertEqual(subs["CAP::EventFilterCreator"]["EVENT_FILTER_0:CONDITION_0:MAXIMUM"], "1000000")

    def test_parameters(self):
        _, subs = self.creator()
        e = subs["CAP::ParticleEfficiencyCreator"]
        self.assertEqual(e["PARTICLE_EFFICIENCY_0:OWNER"], "true")
        self.assertEqual(e["PARTICLE_EFFICIENCY_0:pt_nbins"], "50")
        self.assertEqual(e["PARTICLE_EFFICIENCY_0:eta_nbins"], "20")

    def test_settings(self):
        lib = library()
        lib.settings["PARTICLE_EFFICIENCY"].update({"IMPORT": "true", "IMPORT:FILE_NAME": "Eff.root"})
        creator = F.creator_task(lib, TEMPLATES, RunCAP.TaskConfig, RunCAP.Param)
        e = values(creator.children[3])
        self.assertEqual((e["PARTICLE_EFFICIENCY:IMPORT"], e["PARTICLE_EFFICIENCY:IMPORT:FILE_NAME"]),
                         ("true", "Eff.root"))
        self.assertEqual(len([k for k in e if k == "PARTICLE_EFFICIENCY:IMPORT"]), 1)

    def test_list_flags_off(self):
        """FilterCreator imports, creates and exports the objects: never the tasks (their model is unchanged)."""
        tasks = workflow()
        single = tasks[0].children[1]
        single.param("PARTICLE_EFFICIENCY:IMPORT").value = "true"
        copies = F.prepared_for_writing(tasks, library(), TEMPLATES, RunCAP.Param, RunCAP.TaskConfig)
        self.assertEqual(values(copies[1].children[1])["PARTICLE_EFFICIENCY:IMPORT"], "false")
        self.assertEqual(single.param("PARTICLE_EFFICIENCY:IMPORT").value, "true")

    def test_events_and_databases(self):
        creator, subs = self.creator()
        self.assertEqual([t.name for t in creator.children][-2:], ["Events", "ParticleDbs"])
        db = subs["CAP::ParticleDbCreator"]
        self.assertEqual((db["PARTICLE_DB:N"], db["PARTICLE_DB_0:NAME"], db["PARTICLE_DB_0:OWNER"]),
                         ("1", "ParticleDb", "true"))
        self.assertEqual((db["PARTICLE_DB:IMPORT"], db["PARTICLE_DB:IMPORT:PATH"], db["PARTICLE_DB:IMPORT:FILE_NAME"]),
                         ("true", "ParticleData", "ParticleDb.xml"))
        self.assertEqual(subs["CAP::EventCreator"]["EVENT_0:NAME"], "Event")
        g = values(self.gen)
        self.assertEqual((g["EVENT:N"], g["EVENT_0:NAME"], g["EVENT_0:OWNER"]), ("1", "Event", "false"))
        self.assertEqual((g["PARTICLE_DB_0:NAME"], g["PARTICLE_DB_0:OWNER"]), ("ParticleDb", "false"))
        self.assertEqual(g["PARTICLE_DB:IMPORT"], "false")
        it = values(self.tasks[0])
        self.assertIn("EVENT:REQUESTED:N", it)                      # not a setting of the list of events
        self.assertFalse(F.LIST_SETTING_KEY.match("EVENT:REQUESTED:N"))
        self.assertTrue(F.LIST_SETTING_KEY.match("PARTICLE_DB:IMPORT:DECAYS_FILE"))

    def test_default_choices(self):
        lib = library()
        analyzer, glauber = task("ParticleSingleAnalyzer"), task("GlauberGenerator")
        F.default_choices(analyzer, lib, TEMPLATES)
        F.default_choices(glauber, lib, TEMPLATES)
        self.assertEqual(analyzer.filters, {"EVENT": ["Event"], "PARTICLE_DB": ["ParticleDb"]})
        self.assertEqual(glauber.filters, {"EVENT": ["Event"]})
        empty = task("ParticleSingleAnalyzer")
        F.default_choices(empty, F.FilterLibrary(), TEMPLATES)
        self.assertEqual(F.choices(empty), {})

    def test_database_file(self):
        """Particle databases are read from XML files: the former text files are replaced, and reported."""
        lib = library()
        lib.settings["PARTICLE_DB"]["IMPORT:FILE_NAME"] = "particles.data"
        self.assertIn("is not one", "\n".join(F.check_library(lib)))
        warnings = F.update_database_files(lib)
        self.assertEqual(lib.settings["PARTICLE_DB"]["IMPORT:FILE_NAME"], "ParticleDb.xml")
        self.assertEqual(len(warnings), 1)
        self.assertEqual(F.check_library(lib), [])
        self.assertEqual(F.update_database_files(lib), [])

    def test_owner_settings_migrate(self):
        """A workflow written before FilterCreator: the import of the particle database moves to FilterCreator."""
        gen = task("BasicEventGen")
        P = RunCAP.Param
        gen.params += [P("PARTICLE_DB_0:NAME", "string", "DefaultDb"), P("PARTICLE_DB_0:OWNER", "bool", "true")]
        gen.param("PARTICLE_DB:N").value = "1"
        gen.param("PARTICLE_DB:IMPORT:FILE_NAME").value = "pdg.xml"
        lib, _ = F.extract_choices([gen], TEMPLATES)
        self.assertEqual(lib.names("PARTICLE_DB"), ["DefaultDb"])
        self.assertEqual(lib.settings["PARTICLE_DB"]["IMPORT:FILE_NAME"], "pdg.xml")

    def test_glauber(self):
        lib = library()
        lib.add(F.FilterDef("EVENT_FILTER", "B", "", [F.Condition("IMPACT", F.RANGE, 0, 5)]))
        gen, ana = task("GlauberGenerator"), task("GlauberAnalyzer")
        gen.filters = {"EVENT_FILTER": ["B"]}
        ana.filters = {"EVENT_FILTER": ["B"]}
        gen.filters["EVENT"] = ["Event"]
        ana.filters["EVENT"] = ["Event"]
        F.apply_choices([gen, ana], lib, TEMPLATES, RunCAP.Param)
        self.assertEqual(values(gen)["EVENT_FILTER_0:OWNER"], "true")       # store of the Glauber tasks
        self.assertEqual(values(gen)["EVENT_0:OWNER"], "true")
        self.assertEqual(values(ana)["EVENT_0:OWNER"], "false")
        self.assertEqual(values(gen)["EVENT_FILTER_0:CONDITION_0:SUBTYPE"], "IMPACT")
        self.assertEqual(values(ana)["EVENT_FILTER_0:OWNER"], "false")
        creator = F.creator_task(lib, TEMPLATES, RunCAP.TaskConfig, RunCAP.Param)
        self.assertEqual(values(creator.children[0])["EVENT_FILTER:N"], "2")  # MB, Central: not B

    def test_single_folders(self):
        s = values(self.single)
        self.assertEqual((s["HISTOGRAM_1:N"], s["HISTOGRAM_2:N"]), ("4", "4"))
        self.assertNotIn("HISTOGRAM_3:N", s)
        # index e*nP + p
        self.assertEqual(s["HISTOGRAM_1_0:NAME"], "Single_MB_PiP")
        self.assertEqual(s["HISTOGRAM_1_1:NAME"], "Single_MB_PiM")
        self.assertEqual(s["HISTOGRAM_1_2:NAME"], "Single_Central_PiP")
        self.assertEqual(s["HISTOGRAM_2_3:NAME"], "Single_Central_PiM_D")
        self.assertNotIn("HISTOGRAM_2_3:BASE_NAME", s)                    # short names: the folders tell them apart
        self.assertEqual(s["HISTOGRAM_1_0:OWNER"], "true")
        self.assertNotIn("HISTOGRAM_1_0:pt_nbins", s)

    def test_pair_folders(self):
        p = values(self.pair)
        self.assertEqual([p["HISTOGRAM_%d:N" % l] for l in (1, 2, 3, 4)], ["4", "4", "2", "2"])
        # index e*nP*nP + p1*nP + p2
        self.assertEqual([p["HISTOGRAM_1_%d:NAME" % k] for k in range(4)],
                         ["Pair_MB_PiP_PiP", "Pair_MB_PiP_PiM", "Pair_MB_PiM_PiP", "Pair_MB_PiM_PiM"])
        self.assertEqual(p["HISTOGRAM_2_1:NAME"], "Pair_MB_PiP_PiM_D")
        self.assertEqual(p["HISTOGRAM_3_1:NAME"], "Pair_MB_PiM")
        self.assertEqual(p["HISTOGRAM_4_0:NAME"], "Pair_MB_PiP_D")

    def test_reapply_replaces_keys(self):
        F.apply_choices(self.tasks, library(), TEMPLATES, RunCAP.Param)
        keys = [p.key for p in self.single.params]
        self.assertEqual(len(keys), len(set(keys)))

    def test_lists_used(self):
        """Folders only for the lists used: one per combination of filters (n*m), none for the others."""
        tasks = workflow()
        single = tasks[0].children[1]
        single.param("HISTOGRAM_2:USE").value = "false"
        F.apply_choices(tasks, library(), TEMPLATES, RunCAP.Param)
        s = values(single)
        self.assertEqual((s["HISTOGRAM_1:N"], s["HISTOGRAM_2:N"]), ("4", "0"))       # 2 event x 2 particle filters
        self.assertNotIn("HISTOGRAM_2_0:NAME", s)

    def test_default_histogram_settings(self):
        pair = task("ParticlePairAnalyzer", "Pair")
        F.default_histogram_settings(pair, TEMPLATES)
        v = values(pair)
        self.assertEqual([v["HISTOGRAM_%d:USE" % l] for l in (1, 2, 3, 4)], ["true", "false", "true", "false"])
        self.assertEqual((v["HISTOGRAM_1:CREATE"], v["HISTOGRAM_1:IMPORT"]), ("true", "false"))
        self.assertFalse(any(k.startswith("HISTOGRAM_1:EXPORT") or k.startswith("HISTOGRAM_1:IMPORT:") for k in v))
        self.assertEqual(F.list_label(pair, TEMPLATES, 4), "Derived single histograms")

    def test_histogram_modes(self):
        a = task("ParticleSingleAnalyzer", "PSA")
        F.set_histogram_mode(a, TEMPLATES, F.FILL)
        self.assertEqual(F.histogram_mode(a, TEMPLATES), F.FILL)
        F.set_histogram_mode(a, TEMPLATES, F.DERIVED)
        v = values(a)
        self.assertEqual(F.histogram_mode(a, TEMPLATES), F.DERIVED)
        self.assertEqual((v["HISTOGRAM_1:USE"], v["HISTOGRAM_1:IMPORT"], v["HISTOGRAM_1:CREATE"]),
                         ("true", "true", "false"))                          # from the input file of the job
        self.assertEqual((v["HISTOGRAM_2:USE"], v["HISTOGRAM_2:CREATE"], v["HISTOGRAM_2:IMPORT"]), ("true", "true", "false"))
        a.param("HISTOGRAM_2:CREATE").value = "false"
        self.assertEqual(F.histogram_mode(a, TEMPLATES), F.CUSTOM)

    def test_mode_checks(self):
        lib = library()
        tasks = workflow()
        it, gen, single, pair = tasks[0], *tasks[0].children
        F.set_histogram_mode(single, TEMPLATES, F.DERIVED)              # in an EventIterator
        F.set_histogram_mode(pair, TEMPLATES, F.FILL)
        pair.filters.pop("EVENT")
        text = "\n".join(F.check_choices(tasks, lib, TEMPLATES))
        self.assertIn("Task Single computes its derived histograms from a file: put it in a FileIterator", text)
        self.assertIn("Task Pair uses no event", text)
        single.filters.pop("EVENT")
        self.assertNotIn("Task Single uses no event", "\n".join(F.check_choices(tasks, lib, TEMPLATES)))
        pair.param("HISTOGRAM_1:USE").value = "false"
        pair.param("HISTOGRAM_2:USE").value = "true"
        self.assertIn("derived pair histograms are computed from its pair histograms: use them too",
                      "\n".join(F.check_choices(tasks, lib, TEMPLATES)))

    def test_derived_workflow(self):
        tasks = workflow()
        derived = F.derived_workflow(tasks, TEMPLATES, RunCAP.TaskConfig)
        self.assertEqual([(t.name, t.cls) for t in derived], [("EventIterator", "CAP::FileIterator")])
        self.assertEqual([t.name for t in derived[0].children], ["Single", "Pair"])      # no generator
        single = derived[0].children[0]
        self.assertEqual(F.histogram_mode(single, TEMPLATES), F.DERIVED)
        self.assertEqual(single.filters, tasks[0].children[1].filters)
        self.assertEqual(F.histogram_mode(tasks[0].children[1], TEMPLATES), F.CUSTOM)   # the workflow is unchanged
        self.assertEqual(single.param("EVENT:USE").value, "false")              # no event loop
        self.assertEqual(F.check_choices(derived, library(), TEMPLATES), [])

    def test_event_settings(self):
        """A generator creates its events, an analyzer uses them; the settings are written as set."""
        gen, ana, it = task("BasicEventGen"), task("ParticleSingleAnalyzer", "PSA"), task("EventIterator")
        for t in (gen, ana, it):
            F.default_event_settings(t, TEMPLATES)
        self.assertEqual((values(gen)["EVENT:CREATE"], values(gen)["EVENT:USE"]), ("true", "false"))
        self.assertEqual((values(ana)["EVENT:CREATE"], values(ana)["EVENT:USE"]), ("false", "true"))
        self.assertEqual((values(it)["EVENT:CREATE"], values(it)["EVENT:USE"]), ("false", "false"))
        self.assertTrue(F.makes_events(task("Therminator3")))
        tasks = workflow()
        gen, single = tasks[0].children[0], tasks[0].children[1]
        F.default_event_settings(gen, TEMPLATES)
        F.default_event_settings(single, TEMPLATES)
        gen.param("EVENT:EXPORT").value = "true"
        gen.param("EVENT:EXPORT:FILE_NAME").value = "events.cap"
        copies = F.prepared_for_writing(tasks, library(), TEMPLATES, RunCAP.Param, RunCAP.TaskConfig)
        g, s = values(copies[1].children[0]), values(copies[1].children[1])
        self.assertEqual((g["EVENT:CREATE"], g["EVENT:USE"], g["EVENT:EXPORT"]), ("true", "false", "true"))
        self.assertEqual((s["EVENT:CREATE"], s["EVENT:USE"]), ("false", "true"))
        self.assertEqual(len([k for k in g if k == "EVENT:USE"]), 1)
        events = values(copies[0].children[F.KINDS.index("EVENT")])
        self.assertEqual(events["EVENT:USE"], "true")                    # FilterCreator's list of events
        self.assertEqual(F.check_choices(tasks, library(), TEMPLATES), [])
        gen.param("EVENT:CREATE").value = "false"
        text = "\n".join(F.check_choices(tasks, library(), TEMPLATES))
        self.assertIn("Task Single uses the events Event, which no task creates or imports", text)
        gen.param("EVENT:IMPORT").value = "true"
        self.assertIn("Task BasicEventGen imports events but the file is not named",
                      "\n".join(F.check_choices(tasks, library(), TEMPLATES)))

    def test_job_files(self):
        """The files of the job: keys of the task CAP runs, in the RUNCAP element too; read back."""
        lib = library()
        lib.job.update({"OUTPUT_FILE": "fill.cap"})
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "w.xml")
            RunCAP.write_workflow(path, workflow(), lib, TEMPLATES)
            top = ET.parse(path).getroot().find("TASK")
            keys = {p.get("name"): p.text for g in top.findall("GROUP") if g.get("name") == "JOB"
                    for p in g.findall("PARAMETER")}
            self.assertEqual(keys, {"OUTPUT_FILE": "fill.cap"})
            _, lib2, warnings = RunCAP.load_workflow_with_filters(path, TEMPLATES)
            self.assertEqual(lib2.job, {"OUTPUT_FILE": "fill.cap", "INPUT_FILE": ""})
            tree = ET.parse(path)
            tree.getroot().remove(tree.getroot().find("RUNCAP"))     # from the keys alone
            tree.write(path)
            _, lib3, warnings = RunCAP.load_workflow_with_filters(path, TEMPLATES)
            self.assertEqual(lib3.job["OUTPUT_FILE"], "fill.cap")
            self.assertEqual(warnings, [])

    def test_check_job(self):
        tasks = workflow()
        single = tasks[0].children[1]
        F.set_histogram_mode(single, TEMPLATES, F.DERIVED)
        lib = library()
        self.assertIn("Single imports histograms: name the input file", "\n".join(F.check_job(tasks, lib, TEMPLATES)))
        lib.job.update({"INPUT_FILE": "a.cap", "OUTPUT_FILE": "a.cap"})
        self.assertIn("are the same file", "\n".join(F.check_job(tasks, lib, TEMPLATES)))
        lib.job["OUTPUT_FILE"] = "b.cap"
        self.assertEqual(F.check_job(tasks, lib, TEMPLATES), [])

    def test_obsolete_keys(self):
        """Keys of earlier versions (files of the lists of folders) are dropped when a workflow is read."""
        it = task("EventIterator")
        a = task("ParticleSingleAnalyzer", "PSA")
        it.children = [a]
        a.params += [RunCAP.Param("HISTOGRAM_1:EXPORT", "bool", "true"),
                     RunCAP.Param("HISTOGRAM_1:EXPORT:FILE_NAME", "string", "x.cap")]
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "old.xml")
            RunCAP.write_workflow(path, [it])
            tasks, _, warnings = RunCAP.load_workflow_with_filters(path, TEMPLATES)
        self.assertIsNone(tasks[0].children[0].param("HISTOGRAM_1:EXPORT"))
        self.assertFalse([w for w in warnings if "HISTOGRAM_1:EXPORT" in w])

    def test_event_settings_migrate(self):
        """A workflow written before the event settings were the tasks' (generator: USE true, CREATE false)."""
        tasks = workflow()
        gen, single = tasks[0].children[0], tasks[0].children[1]
        gen.param("EVENT:USE").value = "true"
        gen.param("EVENT:CREATE").value = "false"
        single.param("EVENT:USE").value = "true"
        text = "\n".join(F.check_choices(tasks, library(), TEMPLATES))
        self.assertIn("Task BasicEventGen makes events but is set neither to create nor to import them", text)
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "old.xml")
            RunCAP.write_workflow(path, tasks, library(), TEMPLATES)
            read, lib, warnings = RunCAP.load_workflow_with_filters(path, TEMPLATES)
        g = read[0].children[0]
        self.assertEqual((g.param("EVENT:CREATE").value, g.param("EVENT:USE").value), ("true", "false"))
        self.assertIn("Task BasicEventGen makes events: set to create them", "\n".join(warnings))
        self.assertEqual(read[0].children[1].param("EVENT:USE").value, "true")      # the analyzer is unchanged
        self.assertEqual(F.check_choices(read, lib, TEMPLATES), [])

    def test_iterator_uses_events(self):
        """A workflow whose EventIterator does not use the events its generator creates (written before the iterator
        reset them): on loading, the iterator uses them, and the checks report an iterator that does not."""
        tasks = workflow()
        it = tasks[0]
        it.filters = {}
        it.param("EVENT:USE").value = "false"
        text = "\n".join(F.check_choices(tasks, library(), TEMPLATES))
        self.assertIn("Event iterator EventIterator does not use the events Event made by its tasks", text)
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "old.xml")
            RunCAP.write_workflow(path, tasks, library(), TEMPLATES)
            read, lib, warnings = RunCAP.load_workflow_with_filters(path, TEMPLATES)
        self.assertEqual((F.choices(read[0]).get("EVENT"), read[0].param("EVENT:USE").value), (["Event"], "true"))
        self.assertIn("Event iterator EventIterator set to use the events of its tasks (Event)", "\n".join(warnings))
        self.assertEqual(F.check_choices(read, lib, TEMPLATES), [])
        self.assertEqual(F.update_iterator_events(read, TEMPLATES), [])            # nothing more to do

    def test_iterator_uses_events_of_all_generators(self):
        """Embedding: two generators fill events under one iterator, which uses (resets) all of them."""
        tasks = workflow()
        it, gen = tasks[0], tasks[0].children[0]
        second = task("BasicEventGen", "Embedded")
        F.default_event_settings(second, TEMPLATES)
        second.filters = dict(gen.filters, EVENT=["Event", "Other"])
        it.children.insert(1, second)
        self.assertEqual(F.events_made_below(it, TEMPLATES), ["Event", "Other"])
        self.assertEqual(len(F.update_iterator_events(tasks, TEMPLATES)), 1)
        self.assertEqual(F.choices(it)["EVENT"], ["Event", "Other"])

    def test_nested_iterator_resets_its_own_events(self):
        """The events made under a nested EventIterator are its own, not those of the enclosing iterator."""
        outer = task("EventIterator", "Outer")
        inner = workflow()[0]
        inner.name = "Inner"
        outer.children = [inner]
        self.assertEqual(F.events_made_below(outer, TEMPLATES), [])
        self.assertEqual(F.update_iterator_events([outer], TEMPLATES), [])
        self.assertEqual(F.choices(outer).get("EVENT", []), [])

    def test_use_migrates(self):
        """A workflow whose folders were written without USE (CAP did not read it): the lists are used."""
        a = task("ParticleSingleAnalyzer", "PSA")
        a.params += [RunCAP.Param("HISTOGRAM_1:N", "int", "1"), RunCAP.Param("HISTOGRAM_1_0:NAME", "string", "X")]
        F.extract_choices([a], TEMPLATES)
        self.assertTrue(F.list_used(a, 1))
        self.assertFalse(F.list_used(a, 2))

    def test_per_folder_binning(self):
        tasks = workflow()
        single = tasks[0].children[1]
        single.histograms = F.HistogramOptions(False, {("Central", "PiM"): {"pt_nbins": "7"}})
        F.apply_choices(tasks, library(), TEMPLATES, RunCAP.Param)
        s = values(single)
        default = s["HISTOGRAM:pt_nbins"]
        self.assertEqual(s["HISTOGRAM_1_0:NAME"], "Single_MB_PiP")
        self.assertEqual(s["HISTOGRAM_1_0:pt_nbins"], default)
        self.assertEqual(s["HISTOGRAM_1_3:pt_nbins"], "7")
        self.assertEqual(s["HISTOGRAM_2_3:pt_nbins"], "7")        # derived folders as their base folder
        self.assertEqual(s["HISTOGRAM_1_3:eta_nbins"], s["HISTOGRAM:eta_nbins"])


class TestReading(unittest.TestCase):

    def roundtrip(self, tasks, lib):
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "w.xml")
            RunCAP.write_workflow(path, tasks, lib, TEMPLATES)
            root = ET.parse(path).getroot()
            self.assertEqual([e.tag for e in root], ["TASK", "RUNCAP"])
            return RunCAP.load_workflow_with_filters(path, TEMPLATES)

    def test_roundtrip(self):
        tasks = workflow()
        tasks[0].children[1].histograms = F.HistogramOptions(False, {("Central", "PiM"): {"pt_nbins": "7"}})
        lib = library()
        before = [p.key for p in tasks[0].children[1].params]
        read, lib2, warnings = self.roundtrip(tasks, lib)
        self.assertEqual([p.key for p in tasks[0].children[1].params], before)       # tasks unchanged by writing
        self.assertEqual(warnings, [])
        self.assertEqual(lib2, lib)                                                   # Unused kept
        gen, single, pair = read[0].children
        self.assertEqual(single.filters, tasks[0].children[1].filters)
        self.assertEqual(pair.filters, tasks[0].children[2].filters)
        self.assertEqual(single.histograms, tasks[0].children[1].histograms)
        self.assertEqual(pair.histograms, F.HistogramOptions())
        self.assertFalse(any(F.MANAGED_KEY.match(p.key) for p in single.params))
        self.assertEqual(len(read), 1)                                                # FilterCreator removed

    def test_creator_first(self):
        """The file runs FilterCreator first; without RUNCAP element, the library comes from FilterCreator."""
        lib = library()
        lib.settings["PARTICLE_EFFICIENCY"]["IMPORT"] = "true"
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "w.xml")
            RunCAP.write_workflow(path, workflow(), lib, TEMPLATES)
            tree = ET.parse(path)
            top = tree.getroot().find("TASK")
            self.assertEqual(top.get("class"), "CAP::Task")                         # Workflow wrapper
            self.assertEqual([t.get("name") for t in top.findall("TASK")], ["FilterCreator", "EventIterator"])
            self.assertEqual(len(top.find("TASK").findall("TASK")), 8)
            tree.getroot().remove(tree.getroot().find("RUNCAP"))
            tree.write(path)
            read, lib2, warnings = RunCAP.load_workflow_with_filters(path, TEMPLATES)
        self.assertEqual(warnings, [])
        self.assertEqual([t.name for t in read], ["EventIterator"])
        self.assertEqual(lib2, lib)

    def test_run_root_name(self):
        """CAP runs the task written at the root of the file (the wrapper holding FilterCreator and the tasks), also
        for a workflow of one top-level task, as RunCAP's Run and --print-command give it."""
        lib = library()
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "w.xml")
            written = RunCAP.write_workflow(path, workflow(), lib, TEMPLATES)
            tasks, lib2, _ = RunCAP.load_workflow_with_filters(path, TEMPLATES)
        self.assertEqual(len(tasks), 1)                                 # EventIterator alone, FilterCreator removed
        self.assertEqual(written, "Workflow")
        self.assertEqual(RunCAP.run_root_name(tasks, lib2, TEMPLATES), "Workflow")

    def test_writing_keeps_parameters(self):
        """Writing copies the tasks without copying their parameters (the GUI attaches widgets to them)."""
        tasks = workflow()
        single = tasks[0].children[1]
        kept = single.param("HISTOGRAM:pt_nbins")
        kept.widget_var = object()
        values_before = [(p.key, p.value) for p in single.params]
        copies = F.prepared_for_writing(tasks, library(), TEMPLATES, RunCAP.Param, RunCAP.TaskConfig)
        self.assertEqual(copies[0].cls, F.CREATOR_CLASS)
        self.assertIs(copies[1].children[1].param("HISTOGRAM:pt_nbins"), kept)
        self.assertEqual([(p.key, p.value) for p in single.params], values_before)
        self.assertIn("HISTOGRAM_1_0:NAME", values(copies[1].children[1]))

    def test_old_format(self):
        """A workflow without RUNCAP element, with the keys written by hand."""
        it = task("EventIterator")
        a = task("ParticleSingleAnalyzer", "PSA")
        it.children = [a]
        P = RunCAP.Param
        a.params += [P("EVENT_FILTER:N", "int", "1"), P("EVENT_FILTER_0:NAME", "string", "All"),
                     P("EVENT_FILTER_0:OWNER", "bool", "1"), P("EVENT_FILTER_0:CONDITION:N", "int", "1"),
                     P("EVENT_FILTER_0:CONDITION_0:TYPE", "string", "DOUBLE_RANGE"),
                     P("EVENT_FILTER_0:CONDITION_0:SUBTYPE", "string", "MULT_0"),
                     P("EVENT_FILTER_0:CONDITION_0:MINIMUM", "double", "0"),
                     P("EVENT_FILTER_0:CONDITION_0:MAXIMUM", "double", "1000"),
                     P("PARTICLE_FILTER:N", "int", "1"), P("PARTICLE_FILTER_0:NAME", "string", "Ch"),
                     P("PARTICLE_FILTER_0:OWNER", "bool", "true"), P("PARTICLE_FILTER_0:CONDITION:N", "int", "1"),
                     P("PARTICLE_FILTER_0:CONDITION_0:TYPE", "string", "Bool"),
                     P("PARTICLE_FILTER_0:CONDITION_0:SUBTYPE", "string", "LIVE"),
                     P("PARTICLE_FILTER_0:CONDITION_0:MINIMUM", "double", "1"),
                     P("HISTOGRAM_1:N", "int", "1"), P("HISTOGRAM_1_0:NAME", "string", "SG_All_Ch"),
                     P("HISTOGRAM_1_0:pt_nbins", "int", "11"),
                     P("HISTOGRAM_2:N", "int", "1"), P("HISTOGRAM_2_0:NAME", "string", "SG_All_Ch_D")]
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "old.xml")
            RunCAP.write_workflow(path, [it])
            self.assertIsNone(ET.parse(path).getroot().find("RUNCAP"))
            tasks, lib, warnings = RunCAP.load_workflow_with_filters(path, TEMPLATES)
        self.assertEqual(warnings, [])
        psa = tasks[0].children[0]
        self.assertEqual(psa.filters, {"EVENT_FILTER": ["All"], "PARTICLE_FILTER": ["Ch"]})
        self.assertEqual(lib.get("EVENT_FILTER", "All").conditions, [F.Condition("MULT_0", F.RANGE, 0, 1000)])
        self.assertEqual(lib.get("PARTICLE_FILTER", "Ch").conditions[0].form, F.FLAG)
        self.assertEqual(psa.histograms, F.HistogramOptions(False, {("All", "Ch"): {"pt_nbins": "11"}}))

    def test_renamed_folders_warn(self):
        it = task("EventIterator")
        a = task("ParticleSingleAnalyzer", "PSA")
        it.children = [a]
        a.params += [RunCAP.Param("HISTOGRAM_1:N", "int", "3")]
        _, warnings = F.extract_choices([it], TEMPLATES)
        self.assertEqual(len(warnings), 1)

    def test_conflicting_definitions(self):
        """A task defining an object of the creator differently: the definition of the creator is kept."""
        written = F.prepared_for_writing(workflow(), library(), TEMPLATES, RunCAP.Param, RunCAP.TaskConfig)
        single = written[1].children[1]
        single.param("EVENT_FILTER_0:OWNER").value = "true"
        single.params += [RunCAP.Param("EVENT_FILTER_0:CONDITION:N", "int", "0")]
        lib, warnings = F.extract_choices(written, TEMPLATES)
        self.assertEqual(len(lib.get("EVENT_FILTER", "MB").conditions), 1)
        self.assertTrue(any("MB" in w for w in warnings))

    def test_undefined_name(self):
        it = task("EventIterator")
        a = task("BasicEventGen")
        it.children = [a]
        a.params += [RunCAP.Param("EVENT_FILTER:N", "int", "1"), RunCAP.Param("EVENT_FILTER_0:NAME", "string", "X")]
        lib, warnings = F.extract_choices([it], TEMPLATES)
        self.assertEqual(lib.names("EVENT_FILTER"), ["X"])
        self.assertEqual(len(warnings), 1)


class TestChecks(unittest.TestCase):

    def test_valid(self):
        self.assertEqual(F.check_choices(workflow(), library(), TEMPLATES), [])

    def test_errors(self):
        lib = library()
        for k in range(9):
            lib.add(F.FilterDef("PARTICLE_FILTER", "P%d" % k, "", [F.Condition("PT", F.RANGE, k, k + 1)]))
        lib.add(F.FilterDef("EVENT_FILTER", "Glau", "", [F.Condition("IMPACT", F.RANGE, 0, 5)]))
        lib.add(F.FilterDef("EVENT_FILTER", "Bad name", "", [F.Condition("MULT_0", F.VALUE, 0, 5)]))
        lib.add(F.FilterDef("EVENT_FILTER", "Empty", "", [F.Condition("MULT_1", F.RANGE, 5, 5)]))
        tasks = workflow()
        gen, single, pair = tasks[0].children
        gen.filters["PARTICLE_FILTER"] = ["P%d" % k for k in range(9)]
        gen.filters["EVENT_FILTER"] = ["Glau", "Missing"]
        pair.filters = {"EVENT_FILTER": ["MB"]}                 # no particle filter, no event
        errors = F.check_choices(tasks, lib, TEMPLATES)
        text = "\n".join(errors)
        self.assertIn("Invalid name 'Bad name'", text)
        self.assertIn("cannot be tested as a value", text)
        self.assertIn("minimum of multiplicity 1 is not below", text)
        self.assertIn("at most 8", text)
        self.assertIn("only the Glauber tasks", text)
        self.assertIn("Missing is not in the library", text)
        self.assertIn("Task Pair fills histograms", text)
        self.assertIn("Task Pair uses no event", text)
        self.assertEqual(len(errors), 8)
        pair.param("HISTOGRAM_3:CREATE").value = "false"
        text = "\n".join(F.check_choices(tasks, lib, TEMPLATES))
        self.assertIn("single histograms are used but neither created nor imported (HISTOGRAM_3)", text)

    def test_list_flags_off(self):
        """FilterCreator imports, creates and exports the objects: never the tasks (their model is unchanged)."""
        tasks = workflow()
        single = tasks[0].children[1]
        single.param("PARTICLE_EFFICIENCY:IMPORT").value = "true"
        copies = F.prepared_for_writing(tasks, library(), TEMPLATES, RunCAP.Param, RunCAP.TaskConfig)
        self.assertEqual(values(copies[1].children[1])["PARTICLE_EFFICIENCY:IMPORT"], "false")
        self.assertEqual(single.param("PARTICLE_EFFICIENCY:IMPORT").value, "true")

    def test_glauber(self):
        lib = F.FilterLibrary()
        lib.add(F.FilterDef("EVENT_FILTER", "B", "", [F.Condition("IMPACT", F.DUAL_RANGE, 0, 2, 4, 6)]))
        g = task("GlauberGenerator")
        F.default_event_settings(g, TEMPLATES)
        g.filters = {"EVENT_FILTER": ["B"]}
        self.assertEqual(F.check_choices([g], lib, TEMPLATES), [])
        g.filters["PARTICLE_FILTER"] = ["B"]
        self.assertEqual(len(F.check_choices([g], lib, TEMPLATES)), 1)

    def test_resize(self):
        lib = library()
        lib.resize("PARTICLE_FILTER", 5)
        self.assertEqual(lib.names("PARTICLE_FILTER"), ["PiP", "PiM", "Unused", "ParticleFilter0", "ParticleFilter1"])
        lib.resize("PARTICLE_FILTER", 1)
        self.assertEqual(lib.names("PARTICLE_FILTER"), ["PiP"])


if __name__ == "__main__":
    unittest.main()
