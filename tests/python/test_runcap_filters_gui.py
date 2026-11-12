# **********************************************************************
# Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
# All rights reserved.
#
# For the licensing terms see LICENSE.
# **********************************************************************
"""Tests of the Filters pane of RunCAP.py (RunCAPFiltersGui), skipped without tkinter or a display."""

import os
import sys
import tempfile
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

    def inner(self, kind=None):
        """The editor area (what is shown for the selection)."""
        return self.panel.editor.inner

    def entries(self):
        """The entries of the editor shown (not those of the maps editor, kept hidden)."""
        hidden = set(widgets(self.panel.maps.frame, tk.ttk.Entry))
        return [e for e in widgets(self.inner(), tk.ttk.Entry) if not isinstance(e, tk.ttk.Combobox) and e not in hidden]

    def test_tree(self):
        tree = self.panel.tree
        self.assertEqual(list(tree.get_children()), ["kind\t" + k for k in F.KINDS])      # no linked file
        self.assertEqual(tree.item("kind\tPARTICLE_FILTER", "text"), "Particle filters")
        self.assertEqual(self.panel.selected, ("kind", "EVENT_FILTER"))
        self.assertEqual(str(self.panel.duplicate_button.cget("state")), "disabled")

    def test_add(self):
        self.panel.add_object("PARTICLE_FILTER")
        self.panel.add_object("PARTICLE_FILTER")
        self.assertEqual(self.library.names("PARTICLE_FILTER"), ["ParticleFilter0", "ParticleFilter1"])
        self.assertEqual(self.panel.selected, ("object", "PARTICLE_FILTER", "ParticleFilter1"))
        self.assertEqual(self.panel.tree.selection(), ("object\tPARTICLE_FILTER\tParticleFilter1",))
        self.assertEqual(self.panel.tree.item("kind\tPARTICLE_FILTER", "text"), "Particle filters (2)")
        self.assertEqual(self.entries()[0].get(), "ParticleFilter1")
        self.panel.duplicate_selected()
        self.assertEqual(self.library.names("PARTICLE_FILTER"), ["ParticleFilter0", "ParticleFilter1", "ParticleFilter1_1"])
        self.panel.set_count("PARTICLE_FILTER", 1)
        self.assertEqual(self.removed, [("PARTICLE_FILTER", ["ParticleFilter1", "ParticleFilter1_1"])])
        self.assertEqual(self.panel.selected, ("kind", "PARTICLE_FILTER"))
        self.assertTrue(self.changes)

    def test_remove_used_asks(self):
        self.panel.set_count("EVENT_FILTER", 2)
        self.used[("EVENT_FILTER", "EventFilter1")] = ["Ana"]
        self.panel.refresh()                            # RunCAP refreshes the pane when tasks choose other objects
        answers = []
        original = messagebox.askyesno
        messagebox.askyesno = lambda *a, **k: answers.append(a) or False
        try:
            self.panel.select_object("EVENT_FILTER", "EventFilter1")
            self.panel.remove_selected()
        finally:
            messagebox.askyesno = original
        self.assertEqual(len(answers), 1)
        self.assertEqual(len(self.library.names("EVENT_FILTER")), 2)
        self.assertEqual(self.panel.tree.set("object\tEVENT_FILTER\tEventFilter1", "used"), "Ana")
        self.panel.select_object("EVENT_FILTER", "EventFilter0")
        self.panel.remove_selected()
        self.assertEqual(self.library.names("EVENT_FILTER"), ["EventFilter1"])
        self.assertEqual(self.panel.selected, ("object", "EVENT_FILTER", "EventFilter1"))

    def test_conditions(self):
        self.panel.add_object("PARTICLE_FILTER")
        d = self.library.objects["PARTICLE_FILTER"][0]
        self.panel._add_condition(d)
        self.panel._add_condition(d)
        self.assertEqual([c.subtype for c in d.conditions], ["PT", "PX"])
        self.assertEqual((d.conditions[0].minimum, d.conditions[0].maximum), (0.0, 10.0))
        self.panel._set_quantity(d, d.conditions[1], "PDG")
        self.assertEqual(d.conditions[1].form, F.VALUE)
        self.panel._set_form(d, d.conditions[0], F.DUAL_RANGE)
        self.assertEqual(d.conditions[0].maximum2, 10.0)
        entries = self.entries()
        self.assertEqual(len(entries), 2 + 5)          # name, title, then min, max, min 2, max 2 of PT, the value of PDG
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
        self.assertIn("MULT", events.values())
        self.assertNotIn("MULT_0", events.values())        # earlier versions: read, not offered
        self.assertFalse(F.LEGACY_EVENT_SUBTYPES & set(events.values()))
        self.assertEqual(G.forms_of("PARTICLE_FILTER", "LIVE"), (F.FLAG,))
        self.assertEqual(G.forms_of("EVENT_FILTER", "IMPACT"), (F.RANGE, F.DUAL_RANGE))

    def test_rename(self):
        self.panel.set_count("EVENT_FILTER", 2)
        self.panel.select_object("EVENT_FILTER", "EventFilter0")
        d = self.library.objects["EVENT_FILTER"][0]
        name = self.entries()[0]
        var = tk.StringVar(value="MB")
        self.panel._rename(d, var, name)
        self.assertEqual(d.name, "MB")
        self.assertEqual(self.renamed, [("EVENT_FILTER", "EventFilter0", "MB")])
        self.assertEqual(self.panel.selected, ("object", "EVENT_FILTER", "MB"))
        self.assertTrue(self.panel.tree.exists("object\tEVENT_FILTER\tMB"))
        var.set("EventFilter1")                         # duplicate
        self.panel._rename(d, var, name)
        self.assertEqual(d.name, "MB")
        self.assertIn("duplicate", self.panel.problems.get())
        self.panel._set_title(d, "minimum bias")
        self.assertEqual(self.panel.tree.item("object\tEVENT_FILTER\tMB", "text"), "MB — minimum bias")

    def test_parameters(self):
        self.panel.add_object("EVENT_EFFICIENCY")
        d = self.library.objects["EVENT_EFFICIENCY"][0]
        entries = self.entries()
        self.assertEqual(len(entries), 2 + len(F.KIND_PARAMETERS["EVENT_EFFICIENCY"]))
        entries[3].delete(0, tk.END)                   # nBins_mult (after name, title, BASE_NAME)
        entries[3].insert(0, "40")
        self.assertEqual(d.parameters["nBins_mult"], "40")
        entries[3].delete(0, tk.END)
        self.assertIn("invalid int value", self.panel.problems.get())

    def test_all_kinds(self):
        """Every kind with objects: filters, event corrections, events and particle databases (the particle
        efficiencies and smearers come from the maps file)."""
        for kind in F.KINDS:
            if kind in F.MAPS_KINDS:
                continue
            self.panel.add_object(kind)
            self.assertEqual(self.panel.selected[:2], ("object", kind))
            self.panel.select(kind)
            self.assertEqual(sorted(self.panel.setting_vars), [kind] if kind in F.SETTINGS_KINDS else [])
        self.assertEqual(self.library.settings["PARTICLE_DB"]["IMPORT:FILE_NAME"], "ParticleDb.xml")
        self.assertEqual(sorted(self.panel.setting_vars["PARTICLE_DB"]), ["IMPORT:FILE_NAME", "IMPORT:PATH"])

    def test_settings(self):
        self.panel.select("EVENT_EFFICIENCY")
        variables = self.panel.setting_vars["EVENT_EFFICIENCY"]
        variables["IMPORT"].set(True)
        variables["IMPORT:FILE_NAME"].set("Eff.root")
        self.assertEqual(self.library.settings["EVENT_EFFICIENCY"]["IMPORT"], "true")
        self.assertEqual(self.library.settings["EVENT_EFFICIENCY"]["IMPORT:FILE_NAME"], "Eff.root")
        self.assertTrue(self.changes)
        self.library.settings["EVENT_SMEARER"]["CREATE"] = "true"
        self.panel.select("EVENT_SMEARER")
        self.assertTrue(self.panel.setting_vars["EVENT_SMEARER"]["CREATE"].get())
        self.panel.select("PARTICLE_FILTER")
        self.assertEqual(self.panel.setting_vars, {})

    def test_selection_files(self):
        """Export, Import (with or without replacing) and Link of selection files."""
        pip = F.FilterDef("PARTICLE_FILTER", "PiP", "pi+", [F.Condition("PDG", F.VALUE, 211, 211)])
        self.library.add(pip)
        self.library.add(F.FilterDef("EVENT_FILTER", "MB"))
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "sel.xml")
            self.panel.select_object("PARTICLE_FILTER", "PiP")
            self.panel.export_file("object", path)
            self.assertEqual([o.name for o in F.read_selections(path)[0]], ["PiP"])
            self.panel.export_file("all", path)
            self.assertEqual([o.name for o in F.read_selections(path)[0]], ["MB", "PiP"])
            # imported into another library: a copy; an object already there replaced or kept, as answered
            other = F.FilterLibrary()
            other.add(F.FilterDef("EVENT_FILTER", "MB", "mine"))
            self.panel.library = other
            self.panel.refresh()
            original = messagebox.askyesnocancel
            try:
                messagebox.askyesnocancel = lambda *a, **k: False
                self.panel.import_file(path)
                self.assertEqual((other.get("EVENT_FILTER", "MB").title, other.get("PARTICLE_FILTER", "PiP").title),
                                 ("mine", "pi+"))
                messagebox.askyesnocancel = lambda *a, **k: True
                self.panel.import_file(path)
                self.assertEqual(other.get("EVENT_FILTER", "MB").title, "")
            finally:
                messagebox.askyesnocancel = original
            self.assertEqual(other.links, [])
            # linked: its objects shown as coming from the file, and the file in the tree
            linked = F.FilterLibrary()
            self.panel.library = linked
            self.panel.refresh()
            self.panel.link_file(path)
            self.assertEqual(linked.links, [os.path.abspath(path)])
            self.assertEqual(self.panel.selected, ("link", os.path.abspath(path)))
            self.assertEqual(self.panel.tree.set("object\tPARTICLE_FILTER\tPiP", "source"), "sel.xml")
            self.assertTrue(self.panel.tree.exists("link\t" + os.path.abspath(path)))
            self.panel.select_object("PARTICLE_FILTER", "PiP")
            self.assertTrue(any(isinstance(b, tk.ttk.Button) and b.cget("text") == "Copy into the workflow"
                                for b in widgets(self.inner(), tk.ttk.Button)))
            self.panel.unlink(os.path.abspath(path), False)
            self.assertEqual((linked.links, linked.names("PARTICLE_FILTER")), ([], []))
            self.assertIn(("PARTICLE_FILTER", ["PiP"]), self.removed)

    def test_maps_file(self):
        """The maps file names the particle efficiencies and smearers of the library (as listed by bin/CAP); objects
        gone from the file are removed from the tasks; no file: none."""
        listed = [F.MapsEntry("PARTICLE_EFFICIENCY", "EffPi", "211", "EFFICIENCY=function"),
                  F.MapsEntry("PARTICLE_EFFICIENCY", "EffK", "", "EFFICIENCY=histogram"),
                  F.MapsEntry("PARTICLE_SMEARER", "Smear", "", "PT=function")]
        original = F.list_maps
        F.list_maps = lambda path, program=None: list(listed)
        try:
            with tempfile.TemporaryDirectory() as d:
                path = os.path.join(d, "Maps.cap")
                open(path, "w").close()
                maps = self.panel.maps
                maps.set_file(path)
                self.assertEqual(F.maps_file(self.library), path)
                self.assertEqual(self.library.names("PARTICLE_EFFICIENCY"), ["EffPi", "EffK"])
                self.assertEqual(self.library.names("PARTICLE_SMEARER"), ["Smear"])
                self.assertEqual(self.panel.tree.set("object\tPARTICLE_SMEARER\tSmear", "source"), "maps file")
                self.panel.select("PARTICLE_SMEARER")
                self.assertTrue(maps.frame.winfo_manager())
                rows = [maps.table.item(i, "values") for i in maps.table.get_children()]
                self.assertEqual([r[1] for r in rows], ["EffPi", "EffK", "Smear"])
                self.assertEqual(rows[0][2], "211")
                self.assertEqual(rows[1][2], "all")
                self.panel.select_object("PARTICLE_EFFICIENCY", "EffK")
                self.assertEqual(maps.table.item(maps.table.selection()[0], "values")[1], "EffK")
                del listed[1]                                  # EffK gone from the file
                maps.reload()
                self.assertEqual(self.library.names("PARTICLE_EFFICIENCY"), ["EffPi"])
                self.assertIn(("PARTICLE_EFFICIENCY", ["EffK"]), self.removed)
                maps.set_file("")
                self.assertEqual(F.maps_file(self.library), "")
                self.assertEqual(self.library.names("PARTICLE_SMEARER"), [])
                self.panel.select("EVENT_FILTER")
                self.assertFalse(maps.frame.winfo_manager())
        finally:
            F.list_maps = original


if __name__ == "__main__":
    unittest.main()
