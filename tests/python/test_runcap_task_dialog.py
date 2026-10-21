# **********************************************************************
# Copyright (C) 2019-2026, Claude Pruneau
# All rights reserved.
#
# For the licensing terms see LICENSE.
# **********************************************************************
"""Tests of the window of a task of RunCAP.py (RunCAPTaskDialog), skipped without tkinter or a display."""

import os
import sys
import unittest

CAP_DIR = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, CAP_DIR)

import RunCAP                                   # noqa: E402
import RunCAPFilters as F                       # noqa: E402

TEMPLATES = RunCAP.load_templates(os.path.join(CAP_DIR, "DB", "Templates"))
HELPERS = {"valid_value": RunCAP.valid_value, "partial_value": RunCAP.partial_value, "severities": RunCAP.SEVERITIES}

try:
    import tkinter as tk
    import RunCAPTaskDialog as D
    _root = tk.Tk()
    _root.withdraw()
except Exception:                               # no tkinter, or no display
    _root = None


def library():
    lib = F.FilterLibrary()
    for name in ("MB", "Central"):
        lib.add(F.FilterDef("EVENT_FILTER", name, "", [F.Condition("MULT_0", F.RANGE, 0, 100)]))
    for name in ("PiP", "PiM", "KP"):
        lib.add(F.FilterDef("PARTICLE_FILTER", name, "", [F.Condition("PT", F.RANGE, 0.2, 2)]))
    return lib


@unittest.skipIf(_root is None, "tkinter or a display is not available")
class TestDialog(unittest.TestCase):

    def setUp(self):
        self.task = TEMPLATES["CAP::ParticleSingleAnalyzer"].clone()
        self.task.name = "PSA"
        F.default_histogram_settings(self.task, TEMPLATES)
        self.library = library()
        self.changes, self.filters_shown = [], []
        self.dialog = D.TaskDialog(_root, self.task, TEMPLATES, self.library, HELPERS, rename=self.rename,
                                   changed=lambda: self.changes.append(1), show_filters=self.filters_shown.append)
        self.dialog.withdraw()

    def tearDown(self):
        self.dialog.destroy()

    def rename(self, task, new):
        if new == "Taken":
            return "Another task of the workflow is named Taken."
        task.name = new
        return None

    def test_tabs(self):
        tabs = [self.dialog.notebook.tab(t, "text") for t in self.dialog.notebook.tabs()]
        self.assertEqual(tabs, ["General", "Histograms"])
        general = D.general_parameters(self.task, TEMPLATES)
        self.assertFalse(any(F.MANAGED_KEY.match(p.key) or p.key.startswith("HISTOGRAM") for p in general))
        self.assertFalse(any(p.key.startswith(("EVENT_FILTER:", "PARTICLE_EFFICIENCY:")) for p in general))
        generator = TEMPLATES["CAP::BasicEventGen"].clone()
        dialog = D.TaskDialog(_root, generator, TEMPLATES, self.library, HELPERS, self.rename, lambda: None,
                              lambda k: None)
        self.assertEqual([dialog.notebook.tab(t, "text") for t in dialog.notebook.tabs()], ["General"])
        self.assertIsNotNone(dialog.filters_box)
        dialog.destroy()

    def test_filter_lines(self):
        """One line per kind: a count menu, then one name menu per filter used."""
        def menus():
            return [w for w in self.dialog.filters_box.winfo_children() if isinstance(w, tk.ttk.Combobox)]
        self.assertEqual(len(menus()), len(F.task_kinds(self.task, TEMPLATES)))          # the count menus
        self.dialog._set_count("PARTICLE_FILTER", 3)
        line = self.dialog.filters_box.grid_slaves(row=F.task_kinds(self.task, TEMPLATES).index("PARTICLE_FILTER"),
                                                   column=2)[0]
        names = [w.get() for w in line.winfo_children() if isinstance(w, tk.ttk.Combobox)]
        self.assertEqual(names, ["PiP", "PiM", "KP"])
        self.assertEqual(menus()[1].cget("values")[-1], "3")                 # at most the 3 particle filters defined

    def test_choices(self):
        self.dialog._set_count("EVENT_FILTER", 1)
        self.dialog._set_count("PARTICLE_FILTER", 2)
        self.assertEqual(self.task.filters, {"EVENT_FILTER": ["MB"], "PARTICLE_FILTER": ["PiP", "PiM"]})
        self.dialog._set_choice("PARTICLE_FILTER", 1, "KP")
        self.assertEqual(self.task.filters["PARTICLE_FILTER"], ["PiP", "KP"])
        self.dialog._set_count("PARTICLE_FILTER", 3)
        self.assertEqual(self.task.filters["PARTICLE_FILTER"], ["PiP", "KP", "PiM"])
        self.dialog._set_count("PARTICLE_FILTER", 0)
        self.assertNotIn("PARTICLE_FILTER", self.task.filters)
        self.assertTrue(self.changes)

    def test_folders(self):
        self.dialog._set_count("EVENT_FILTER", 1)
        self.dialog._set_count("PARTICLE_FILTER", 2)
        self.assertEqual(self.dialog.folder_list.get(0, tk.END), (D.DEFAULT_ENTRY,))      # same binning
        self.dialog._set_same(False)
        self.assertEqual(self.dialog.folder_list.get(0, tk.END), (D.DEFAULT_ENTRY, "PSA_MB_PiP", "PSA_MB_PiM"))
        self.dialog.folder_list.selection_clear(0, tk.END)
        self.dialog.folder_list.selection_set(2)
        self.dialog._show_binning()
        inner = self.dialog.binning_area.winfo_children()[0].inner
        row = next(w.grid_info()["row"] for w in inner.winfo_children()
                   if isinstance(w, tk.ttk.Label) and w.cget("text").startswith("pt_nbins "))
        entry = next(w for w in inner.winfo_children()
                     if isinstance(w, tk.ttk.Entry) and w.grid_info()["row"] == row)
        entry.delete(0, tk.END)
        entry.insert(0, "13")
        options = F.histogram_options(self.task)
        self.assertEqual(options.binning, {("MB", "PiM"): {"pt_nbins": "13"}})
        self.assertEqual(self.dialog.folder_list.get(2), "PSA_MB_PiM •")
        entry.delete(0, tk.END)
        entry.insert(0, self.task.param("HISTOGRAM:pt_nbins").value)        # back to the default
        self.assertEqual(options.binning, {})
        self.dialog._set_prefix("SG", tk.ttk.Entry(_root))
        self.assertEqual(options.prefix, "SG")
        self.assertEqual(self.dialog.folder_list.get(1), "SG_MB_PiP")

    def test_rename_and_problems(self):
        self.dialog.name_var.set("Taken")
        self.dialog._rename(tk.ttk.Entry(_root))
        self.assertEqual(self.task.name, "PSA")
        self.assertIn("Taken", self.dialog.problems.get())
        self.dialog.name_var.set("Single")
        self.dialog._rename(tk.ttk.Entry(_root))
        self.assertEqual(self.task.name, "Single")
        self.dialog._update_problems()
        self.assertIn("choose at least one of each", self.dialog.problems.get())

    def test_library_changed(self):
        self.dialog._set_count("PARTICLE_FILTER", 1)
        self.library.add(F.FilterDef("PARTICLE_FILTER", "Pr"))
        self.dialog.library_changed()
        self.assertIn("Pr", self.dialog._names["PARTICLE_FILTER"])

    def test_lists(self):
        """The Use toggle of a list: its folders appear (n*m) or go."""
        self.dialog._set_count("EVENT_FILTER", 2)
        self.dialog._set_count("PARTICLE_FILTER", 3)
        self.assertEqual(len(self.dialog._list_entries), 1)             # same binning: the defaults only
        self.dialog._set_same(False)
        self.assertEqual(len(self.dialog._list_entries), 1 + 6)         # 2 x 3 folders of histograms
        self.dialog._set_list(self.task.param("HISTOGRAM_1:USE"), "false", True)
        self.assertEqual(len(self.dialog._list_entries), 1)
        self.dialog._set_list(self.task.param("HISTOGRAM_2:USE"), "true", True)
        self.assertEqual(len(self.dialog._list_entries), 1)             # derived folders follow their folders
        self.assertIn("derived histograms are used but neither created nor imported", self.dialog.problems.get())

    def test_mode(self):
        self.assertEqual(F.histogram_mode(self.task, TEMPLATES), F.FILL)
        self.dialog._set_mode(F.DERIVED)
        self.assertEqual(F.histogram_mode(self.task, TEMPLATES), F.DERIVED)
        self.assertEqual(self.task.param("HISTOGRAM_1:IMPORT:FILE_NAME").value, "PSA.cap")
        self.assertTrue(self.changes)


if __name__ == "__main__":
    unittest.main()
