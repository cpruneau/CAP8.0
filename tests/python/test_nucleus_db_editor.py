# **********************************************************************
# Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
# All rights reserved.
#
# For the licensing terms see LICENSE.
# **********************************************************************
"""Tests of NucleusDbEditor.py: the model (reading, writing, checks, densities) and, with a display, the GUI."""

import os
import shutil
import subprocess
import sys
import tempfile
import textwrap
import unittest

CAP_DIR = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, CAP_DIR)

import NucleusDbEditor as E                     # noqa: E402

DB_FILE = os.path.join(CAP_DIR, "DB", "Glauber", "NucleusDb.xml")


def has_display():
    try:
        import tkinter as tk
        root = tk.Tk()
        root.destroy()
        return True
    except Exception:
        return False


class TestModel(unittest.TestCase):

    def setUp(self):
        self.dir = tempfile.mkdtemp()

    def tearDown(self):
        shutil.rmtree(self.dir)

    def test_file_delivered(self):
        db = E.NucleusDb.read(DB_FILE)
        self.assertEqual(len(db.nuclei), 46)
        self.assertEqual(db.problems(), [])
        pb = db.find("Pb")
        self.assertEqual((pb.get("A"), pb.get("Z"), pb.neutrons(), pb.radial_type()), ("208", "82", 126, 1))
        self.assertEqual((db.find("He3").get("Z"), db.find("H3").get("Z")), ("2", "1"))
        self.assertIn("NUCLEUS_DB", db.text())

    def test_round_trip_is_identical(self):
        db = E.NucleusDb.read(DB_FILE)
        with open(DB_FILE, encoding="utf-8") as f:
            self.assertEqual(db.text(), f.read())           # the comment at the top is kept

    def test_edit_write_read(self):
        db = E.NucleusDb.read(DB_FILE)
        db.find("Pb").attributes["skin"] = "0.55"
        db.nuclei.append(E.Nucleus({"name": "Ru", "A": "96", "Z": "44", "radialType": "8", "radius": "5.085",
                                    "skin": "0.46", "w": "0", "beta2": "0.158", "comment": "isobar"}))
        path = os.path.join(self.dir, "db.xml")
        db.write(path)
        again = E.NucleusDb.read(path)
        self.assertEqual(again.find("Pb").get("skin"), "0.55")
        self.assertEqual(again.find("Ru").get("beta2"), "0.158")
        self.assertEqual(again.find("Ru").get("beta4"), "")      # optional, not written when empty
        self.assertEqual(again.problems(), [])

    def test_problems(self):
        db = E.NucleusDb()
        db.nuclei = [E.Nucleus({"name": "X", "A": "2", "Z": "3", "radialType": "1", "radius": "1", "skin": "0.5"}),
                     E.Nucleus({"name": "X", "A": "4", "Z": "2", "radialType": "1", "radius": "1", "skin": "0.5"}),
                     E.Nucleus({"name": "Y", "A": "x", "Z": "2", "radialType": "1"}),
                     E.Nucleus({"name": "D", "A": "100", "Z": "40", "radialType": "8", "radius": "5", "skin": "0.5"}),
                     E.Nucleus({"name": "P", "A": "208", "Z": "82", "radialType": "11", "radius": "6", "skin": "0.5"}),
                     E.Nucleus({"name": "T", "A": "20", "Z": "10", "radialType": "99", "radius": "3", "skin": "0.5"})]
        text = "\n".join(db.problems())
        for expected in ("X: Z = 3 not in [0, A = 2]", "X: duplicate name", "Y: A is not an integer",
                         "D: deformed type 8 with beta2 = beta4 = 0", "P: type 11 without neutron parameters",
                         "T: unknown radial type 99"):
            self.assertIn(expected, text)
        self.assertTrue(any("not implemented" in w for w in db.warnings()))

    def test_densities(self):
        db = E.NucleusDb.read(DB_FILE)
        pb = db.find("Pb")
        self.assertAlmostEqual(E.density(pb, 6.62), 0.5)                # 3pF: half at the radius
        self.assertLess(E.density(pb, 9.0), 0.02)
        u2 = db.find("U2")
        self.assertGreater(E.density(u2, 7.4, 1.0), E.density(u2, 7.4, 0.0))   # prolate: longer along the axis
        self.assertIsNone(E.density(db.find("He3"), 1.0))                # configurations from a file

    def test_command_line(self):
        out = subprocess.run([sys.executable, os.path.join(CAP_DIR, "NucleusDbEditor.py"), "--check", DB_FILE],
                             capture_output=True, text=True)
        self.assertEqual(out.returncode, 0)
        self.assertIn("46 nuclei", out.stdout)
        out = subprocess.run([sys.executable, os.path.join(CAP_DIR, "NucleusDbEditor.py"), "--list", DB_FILE],
                             capture_output=True, text=True)
        self.assertEqual(out.returncode, 0)
        self.assertIn("Pbpn", out.stdout)


@unittest.skipUnless(has_display(), "tkinter or a display is not available")
class TestGui(unittest.TestCase):

    def test_edit_save_plot(self):
        directory = tempfile.mkdtemp()
        try:
            path = os.path.join(directory, "db.xml")
            shutil.copy(DB_FILE, path)
            script = textwrap.dedent("""
                import sys
                sys.path.insert(0, %r)
                import NucleusDbEditor as E
                result = {}
                def hook(editor, root):
                    def run():
                        editor.open(%r)
                        item = editor.nucleus_item[id(editor.db.find("Pb"))]
                        editor.edit_cell(item, "#%%d" %% (editor.columns.index("f:skin") + 1))
                        editor.editor["var"].set("0.6")
                        editor._commit()
                        editor.name_filter.set("Xe")
                        result["shown"] = len(editor.tree.get_children(""))
                        editor.name_filter.set("")
                        editor.tree.selection_set(editor.nucleus_item[id(editor.db.find("U2"))])
                        editor.plot_densities()
                        result["plot"] = editor.density_window.winfo_exists()
                        editor.move(-1)
                        result["moved"] = [n.name for n in editor.db.nuclei].index("U2")
                        editor.save()
                        result["dirty"] = editor.dirty
                        root.destroy()
                    root.after(200, run)
                E.run_gui(None, hook)
                db = E.NucleusDb.read(%r)
                print(result["shown"], result["plot"], result["moved"], result["dirty"], db.find("Pb").get("skin"),
                      [n.name for n in db.nuclei].index("U2"))
            """) % (CAP_DIR, path, path)
            out = subprocess.run([sys.executable, "-c", script], capture_output=True, text=True, timeout=60)
            self.assertEqual(out.returncode, 0, out.stderr)
            shown, plot, moved, dirty, skin, index = out.stdout.split()
            self.assertEqual(int(shown), 7)                    # Xe, Xes, Xe2, Xe2a, Xerw, Xesrw, Xe2arw
            self.assertEqual(plot, "1")
            self.assertEqual(dirty, "False")
            self.assertEqual(skin, "0.6")
            self.assertEqual(moved, index)
            self.assertEqual(int(index), 44)                   # U2 was last (45)
        finally:
            shutil.rmtree(directory)


if __name__ == "__main__":
    unittest.main()
