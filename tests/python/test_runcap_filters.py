# **********************************************************************
# Copyright (C) 2019-2026, Claude Pruneau
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
    lib = F.FilterLibrary()
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
    for t, lists in ((single, (1,)), (pair, (1, 3))):
        for l in lists:
            t.param("HISTOGRAM_%d:CREATE" % l).value = "true"
    gen.filters = {"EVENT_FILTER": ["MB"], "PARTICLE_FILTER": ["PiP", "PiM"]}
    single.filters = {"EVENT_FILTER": ["MB", "Central"], "PARTICLE_FILTER": ["PiP", "PiM"],
                      "PARTICLE_EFFICIENCY": ["Eff"]}
    pair.filters = {"EVENT_FILTER": ["MB"], "PARTICLE_FILTER": ["PiP", "PiM"]}
    return [it]


class TestTemplates(unittest.TestCase):

    def test_kinds_and_layouts(self):
        self.assertIn("PARTICLE_FILTER", F.task_kinds(task("ParticleSingleAnalyzer"), TEMPLATES))
        self.assertEqual(F.task_kinds(task("GlauberGenerator"), TEMPLATES), ["EVENT_FILTER"])
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

    def test_owners(self):
        g, s = values(self.gen), values(self.single)
        self.assertEqual(g["EVENT_FILTER:N"], "1")
        self.assertEqual(g["EVENT_FILTER:USE"], "true")
        self.assertEqual(g["EVENT_FILTER_0:NAME"], "MB")
        self.assertEqual(g["EVENT_FILTER_0:OWNER"], "true")
        self.assertEqual(g["EVENT_FILTER_0:TITLE"], "minimum bias")
        self.assertEqual(g["PARTICLE_FILTER_1:OWNER"], "true")
        # MB and the particle filters are defined by the generator, Central by the analyzer
        self.assertEqual(s["EVENT_FILTER_0:OWNER"], "false")
        self.assertNotIn("EVENT_FILTER_0:CONDITION:N", s)
        self.assertEqual(s["EVENT_FILTER_1:OWNER"], "true")
        self.assertEqual(s["EVENT_FILTER_1:CONDITION_0:MINIMUM"], "100")
        self.assertEqual(s["PARTICLE_FILTER_0:OWNER"], "false")
        self.assertEqual(s["EVENT_SMEARER:N"], "0")
        self.assertEqual(s["EVENT_SMEARER:USE"], "false")

    def test_conditions(self):
        g = values(self.gen)
        p = "PARTICLE_FILTER_0:CONDITION_"
        self.assertEqual(g["PARTICLE_FILTER_0:CONDITION:N"], "4")
        self.assertEqual((g[p + "0:TYPE"], g[p + "0:SUBTYPE"], g[p + "0:MINIMUM"]), ("INTEGER", "PDG", "211"))
        self.assertEqual((g[p + "1:TYPE"], g[p + "1:MINIMUM"], g[p + "1:MAXIMUM"]), ("DOUBLE_RANGE", "0.2", "2"))
        self.assertEqual((g[p + "2:TYPE"], g[p + "2:MINIMUM2"], g[p + "2:MAXIMUM2"]), ("DOUBLE_DUAL_RANGE", "0.1", "0.8"))
        self.assertNotIn(p + "1:MINIMUM2", g)
        self.assertEqual(g[p + "3:TYPE"], "Bool")
        self.assertEqual(values(self.gen)["EVENT_FILTER_0:CONDITION_0:MAXIMUM"], "1000000")

    def test_parameters(self):
        s = values(self.single)
        self.assertEqual(s["PARTICLE_EFFICIENCY_0:OWNER"], "true")
        self.assertEqual(s["PARTICLE_EFFICIENCY_0:pt_nbins"], "50")
        self.assertEqual(s["PARTICLE_EFFICIENCY_0:eta_nbins"], "20")

    def test_single_folders(self):
        s = values(self.single)
        self.assertEqual((s["HISTOGRAM_1:N"], s["HISTOGRAM_2:N"]), ("4", "4"))
        self.assertNotIn("HISTOGRAM_3:N", s)
        # index e*nP + p
        self.assertEqual(s["HISTOGRAM_1_0:NAME"], "Single_MB_PiP")
        self.assertEqual(s["HISTOGRAM_1_1:NAME"], "Single_MB_PiM")
        self.assertEqual(s["HISTOGRAM_1_2:NAME"], "Single_Central_PiP")
        self.assertEqual(s["HISTOGRAM_2_3:NAME"], "Single_Central_PiM_D")
        self.assertEqual(s["HISTOGRAM_2_3:BASE_NAME"], "Single_Central_PiM_D")
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

    def test_per_folder_binning(self):
        tasks = workflow()
        single = tasks[0].children[1]
        single.histograms = F.HistogramOptions("S", False, {("Central", "PiM"): {"pt_nbins": "7"}})
        F.apply_choices(tasks, library(), TEMPLATES, RunCAP.Param)
        s = values(single)
        default = s["HISTOGRAM:pt_nbins"]
        self.assertEqual(s["HISTOGRAM_1_0:NAME"], "S_MB_PiP")
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
        tasks[0].children[1].histograms = F.HistogramOptions("S", False, {("Central", "PiM"): {"pt_nbins": "7"}})
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
        self.assertEqual(pair.histograms, F.HistogramOptions("Pair"))
        self.assertFalse(any(F.MANAGED_KEY.match(p.key) for p in single.params))

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
        self.assertEqual(psa.histograms, F.HistogramOptions("SG", False, {("All", "Ch"): {"pt_nbins": "11"}}))

    def test_renamed_folders_warn(self):
        it = task("EventIterator")
        a = task("ParticleSingleAnalyzer", "PSA")
        it.children = [a]
        a.params += [RunCAP.Param("HISTOGRAM_1:N", "int", "3")]
        _, warnings = F.extract_choices([it], TEMPLATES)
        self.assertEqual(len(warnings), 1)

    def test_conflicting_definitions(self):
        tasks = workflow()
        F.apply_choices(tasks, library(), TEMPLATES, RunCAP.Param)
        single = tasks[0].children[1]
        single.param("EVENT_FILTER_0:OWNER").value = "true"
        single.params += [RunCAP.Param("EVENT_FILTER_0:CONDITION:N", "int", "0")]
        lib, warnings = F.extract_choices(tasks, TEMPLATES)
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
        pair.filters = {"EVENT_FILTER": ["MB"]}
        errors = F.check_choices(tasks, lib, TEMPLATES)
        text = "\n".join(errors)
        self.assertIn("Invalid name 'Bad name'", text)
        self.assertIn("cannot be tested as a value", text)
        self.assertIn("minimum of multiplicity 1 is not below", text)
        self.assertIn("at most 8", text)
        self.assertIn("only the Glauber tasks", text)
        self.assertIn("Missing is not in the library", text)
        self.assertIn("Task Pair fills histograms", text)
        self.assertEqual(len(errors), 7)
        pair.param("HISTOGRAM_3:CREATE").value = "false"
        self.assertIn("HISTOGRAM_3 are neither created nor imported", "\n".join(F.check_choices(tasks, lib, TEMPLATES)))

    def test_glauber(self):
        lib = F.FilterLibrary()
        lib.add(F.FilterDef("EVENT_FILTER", "B", "", [F.Condition("IMPACT", F.DUAL_RANGE, 0, 2, 4, 6)]))
        g = task("GlauberGenerator")
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
