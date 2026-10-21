# **********************************************************************
# Copyright (C) 2019-2026, Claude Pruneau
# All rights reserved.
#
# For the licensing terms see LICENSE.
# **********************************************************************
"""Tests of the Filters pane of RunCAP.py (RunCAPFiltersGui), skipped without tkinter or a display."""

import os
import sys
import unittest

CAP_DIR = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, CAP_DIR)

import RunCAPFilters as F                       # noqa: E402

try:
    import tkinter as tk
    from tkinter import messagebox
    import RunCAPFiltersGui as G
    _root = tk.Tk()
    _root.withdraw()
except Exception:                               # no tkinter, or no display
    _root = None


def widgets(widget, cls):
    found = []
    for child in widget.winfo_children():
        if isinstance(child, cls):
            found.append(child)
        found += widgets(child, cls)
    return found


@unittest.skipIf(_root is None, "tkinter or a display is not available")
class TestPanel(unittest.TestCase):

    def setUp(self):
        self.library = F.FilterLibrary()
        self.changes, self.renamed, self.removed = [], [], []
        self.used = {}
        self.panel = G.FiltersPanel(_root, self.library, on_change=lambda: self.changes.append(1),
                                    on_rename=lambda k, o, n: self.renamed.append((k, o, n)),
                                    on_remove=lambda k, names: self.removed.append((k, names)),
                                    users=lambda k, n: self.used.get((k, n), []))
        self.panel.pack()
        _root.update_idletasks()

    def tearDown(self):
        self.panel.destroy()

    def inner(self, kind):
        return self.panel.scrolls[kind].inner

    def test_tabs(self):
        self.assertEqual(len(self.panel.notebook.tabs()), 8)
        self.assertEqual(self.panel.counts["PARTICLE_FILTER"].get(), "0")
        self.assertEqual(self.panel.counts["EVENT_FILTER"].box.cget("values")[-1], "20")

    def test_count(self):
        self.panel.set_count("PARTICLE_FILTER", 3)
        self.assertEqual(self.library.names("PARTICLE_FILTER"), ["ParticleFilter0", "ParticleFilter1", "ParticleFilter2"])
        self.assertEqual(len(widgets(self.inner("PARTICLE_FILTER"), tk.ttk.Labelframe)), 3)
        self.assertEqual(self.panel.notebook.tab(self.panel.tabs["PARTICLE_FILTER"], "text"), "Particle filters (3)")
        self.panel.set_count("PARTICLE_FILTER", 1)
        self.assertEqual(self.removed, [("PARTICLE_FILTER", ["ParticleFilter1", "ParticleFilter2"])])
        self.assertTrue(self.changes)

    def test_remove_used_asks(self):
        self.panel.set_count("EVENT_FILTER", 2)
        self.used[("EVENT_FILTER", "EventFilter1")] = ["Ana"]
        answers = []
        original = messagebox.askyesno
        messagebox.askyesno = lambda *a, **k: answers.append(a) or False
        try:
            self.panel.set_count("EVENT_FILTER", 1)
        finally:
            messagebox.askyesno = original
        self.assertEqual(len(answers), 1)
        self.assertEqual(len(self.library.names("EVENT_FILTER")), 2)
        self.assertEqual(self.panel.counts["EVENT_FILTER"].get(), "2")

    def test_conditions(self):
        self.panel.set_count("PARTICLE_FILTER", 1)
        d = self.library.objects["PARTICLE_FILTER"][0]
        self.panel._add_condition(d)
        self.panel._add_condition(d)
        self.assertEqual([c.subtype for c in d.conditions], ["PT", "PX"])
        self.assertEqual((d.conditions[0].minimum, d.conditions[0].maximum), (0.0, 10.0))
        self.panel._set_quantity(d, d.conditions[1], "PDG")
        self.assertEqual(d.conditions[1].form, F.VALUE)
        self.panel._set_form(d, d.conditions[0], F.DUAL_RANGE)
        self.assertEqual(d.conditions[0].maximum2, 10.0)
        # the fields of the conditions: min, max, min 2, max 2 of PT, the value of PDG
        entries = [e for e in widgets(self.inner("PARTICLE_FILTER"), tk.ttk.Entry)
                   if not isinstance(e, tk.ttk.Combobox)]
        self.assertEqual(len(entries), 2 + 5)          # name, title, then the fields
        entries[1 + 2].delete(0, tk.END)
        entries[1 + 2].insert(0, "2.5")                # max of PT
        self.assertEqual(d.conditions[0].maximum, 2.5)
        entries[-1].delete(0, tk.END)
        entries[-1].insert(0, "-211")
        self.assertEqual(d.conditions[1].minimum, -211.0)
        self.panel._remove_condition(d, d.conditions[0])
        self.assertEqual([c.subtype for c in d.conditions], ["PDG"])
        self.assertEqual(F.check_library(self.library), [])

    def test_quantities(self):
        events = G.quantities("EVENT_FILTER")
        self.assertIn("IMPACT", events.values())
        self.assertIn("MULT_0", events.values())
        self.assertEqual(G.forms_of("PARTICLE_FILTER", "LIVE"), (F.FLAG,))
        self.assertEqual(G.forms_of("EVENT_FILTER", "IMPACT"), (F.RANGE, F.DUAL_RANGE))

    def test_rename(self):
        self.panel.set_count("EVENT_FILTER", 2)
        d = self.library.objects["EVENT_FILTER"][0]
        name = widgets(self.inner("EVENT_FILTER"), tk.ttk.Entry)[0]
        var = tk.StringVar(value="MB")
        self.panel._rename(d, var, name)
        self.assertEqual(d.name, "MB")
        self.assertEqual(self.renamed, [("EVENT_FILTER", "EventFilter0", "MB")])
        var.set("EventFilter1")                         # duplicate
        self.panel._rename(d, var, name)
        self.assertEqual(d.name, "MB")
        self.assertIn("duplicate", self.panel.problems.get())

    def test_parameters(self):
        self.panel.set_count("PARTICLE_EFFICIENCY", 1)
        d = self.library.objects["PARTICLE_EFFICIENCY"][0]
        entries = [e for e in widgets(self.inner("PARTICLE_EFFICIENCY"), tk.ttk.Entry)
                   if not isinstance(e, tk.ttk.Combobox)]
        self.assertEqual(len(entries), 2 + len(F.KIND_PARAMETERS["PARTICLE_EFFICIENCY"]))
        entries[3].delete(0, tk.END)                   # pt_nbins (after name, title, BASE_NAME)
        entries[3].insert(0, "40")
        self.assertEqual(d.parameters["pt_nbins"], "40")
        entries[3].delete(0, tk.END)
        self.assertIn("invalid int value", self.panel.problems.get())

    def test_all_kinds(self):
        """Every tab with objects: filters, corrections, events and particle databases."""
        for kind in F.KINDS:
            self.panel.set_count(kind, 1)
            self.assertEqual(len(widgets(self.inner(kind), tk.ttk.Labelframe)), 1)
        self.assertEqual(sorted(self.panel.setting_vars), sorted(F.SETTINGS_KINDS))
        self.assertEqual(self.library.settings["PARTICLE_DB"]["IMPORT:FILE_NAME"], "particles.data")

    def test_settings(self):
        self.assertNotIn("PARTICLE_FILTER", self.panel.setting_vars)
        variables = self.panel.setting_vars["PARTICLE_EFFICIENCY"]
        variables["IMPORT"].set(True)
        variables["IMPORT:FILE_NAME"].set("Eff.root")
        self.assertEqual(self.library.settings["PARTICLE_EFFICIENCY"]["IMPORT"], "true")
        self.assertEqual(self.library.settings["PARTICLE_EFFICIENCY"]["IMPORT:FILE_NAME"], "Eff.root")
        self.assertTrue(self.changes)
        self.library.settings["EVENT_SMEARER"]["CREATE"] = "true"
        self.panel.refresh()
        self.assertTrue(self.panel.setting_vars["EVENT_SMEARER"]["CREATE"].get())


if __name__ == "__main__":
    unittest.main()
