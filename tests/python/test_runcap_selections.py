# **********************************************************************
# Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
# All rights reserved.
#
# For the licensing terms see LICENSE.
# **********************************************************************
"""Tests of the event-filter conditions on the selection of a named particle filter (RunCAPFilters): written with
their SELECTION key and read back, checked against the library, renamed with the particle filter; and of the end of
the limit of 8 particle filters per task.

Run from the CAP directory:  python3 -m pytest tests/python
"""

import os
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET

CAP_DIR = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, CAP_DIR)
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import RunCAP                                   # noqa: E402
import RunCAPFilters as F                       # noqa: E402
from test_runcap_stages import TEMPLATES, events_stage, library      # noqa: E402


def selection_library():
    lib = library()
    lib.add(F.FilterDef("PARTICLE_FILTER", "Charged", "charged", [F.Condition("PT", F.RANGE, 0.2, 2.0)]))
    lib.add(F.FilterDef("EVENT_FILTER", "Central", "central", [F.Condition("MULT", F.RANGE, 50, 1e6, selection="Charged"),
                                                               F.Condition("PTSUM", F.RANGE, 0, 100, selection="PiP")]))
    return lib


class TestSelectionConditions(unittest.TestCase):

    def test_written_and_read(self):
        lib = selection_library()
        events = events_stage(lib)
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "w.xml")
            RunCAP.write_stages(path, {RunCAP.EVENTS: events}, lib, TEMPLATES, {RunCAP.EVENTS: {"OUTPUT_FILE": "a.cap"}})
            groups = []
            for e in ET.parse(path).iter("GROUP"):
                if e.get("name", "").startswith("CONDITION_"):
                    groups.append({p.get("name"): (p.text or "").strip() for p in e.findall("PARAMETER")})
            written = [v for v in groups if v.get("SUBTYPE") in ("MULT", "PTSUM")]
            self.assertEqual(sorted(v["SELECTION"] for v in written), ["Charged", "PiP"])
            legacy = [v for v in groups if v.get("SUBTYPE") == "MULT_0"]
            self.assertTrue(legacy and all("SELECTION" not in v for v in legacy))
            stages, lib2, jobs, warnings = RunCAP.load_stages_with_filters(path, TEMPLATES)
            self.assertEqual(warnings, [])
            central = next(o for o in lib2.objects["EVENT_FILTER"] if o.name == "Central")
            self.assertEqual([(c.subtype, c.selection) for c in central.conditions],
                             [("MULT", "Charged"), ("PTSUM", "PiP")])
            self.assertEqual(F.check_library(lib2), [])

    def test_checked_and_renamed(self):
        lib = selection_library()
        self.assertEqual(F.check_library(lib), [])
        self.assertEqual(F.selection_users(lib, "Charged"), ["Central"])
        F.rename_selection(lib, "Charged", "Hadrons")
        central = next(o for o in lib.objects["EVENT_FILTER"] if o.name == "Central")
        self.assertEqual(central.conditions[0].selection, "Hadrons")
        errors = F.check_library(lib)                       # not a particle filter of the library (yet)
        self.assertEqual(len(errors), 1)
        self.assertIn("'Hadrons' is not one", errors[0])
        central.conditions[0].selection = ""
        self.assertIn("none chosen", F.check_library(lib)[0])

    def test_no_limit_of_particle_filters(self):
        lib = library()
        for k in range(10):
            lib.add(F.FilterDef("PARTICLE_FILTER", "P%d" % k, "", [F.Condition("PT", F.RANGE, 0.1 * k, 0.1 * k + 0.1)]))
        events = events_stage(lib)
        single = next(t for t in RunCAP.all_tasks(events) if t.name == "Single")
        F.choices(single)["PARTICLE_FILTER"] = ["P%d" % k for k in range(10)]
        self.assertFalse(any("particle filters" in e for e in F.check_choices(events, lib, TEMPLATES)))


if __name__ == "__main__":
    unittest.main()
