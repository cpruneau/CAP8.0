# **********************************************************************
# Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
# All rights reserved.
#
# For the licensing terms see LICENSE.
# **********************************************************************
"""Test of the Kill button of RunCAP.py: a job printing without pause is killed, and the window shows its end at once
(the output queued after the kill is not shown). Skipped without tkinter or a display.

Run from the CAP directory:  python3 -m unittest discover -s tests/python
"""

import os
import sys
import tempfile
import textwrap
import time
import unittest

CAP_DIR = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, CAP_DIR)

try:
    import tkinter as tk
    _root = tk.Tk()
    _root.destroy()
    HAVE_DISPLAY = True
except Exception:                               # no tkinter, or no display
    HAVE_DISPLAY = False

import RunCAP                                   # noqa: E402


@unittest.skipUnless(HAVE_DISPLAY, "tkinter and a display are needed")
class TestKill(unittest.TestCase):

    def setUp(self):
        self.directory = tempfile.mkdtemp()
        self.spam = os.path.join(self.directory, "spam.py")
        with open(self.spam, "w") as f:
            f.write(textwrap.dedent("""\
                i = 0
                while True:
                    i += 1
                    print("<INFO> Task::execute() event %d, a line as long as those of a verbose CAP job" % i, flush=True)
                """))
        self.settings = RunCAP.SETTINGS_FILE
        RunCAP.SETTINGS_FILE = os.path.join(self.directory, "settings.json")   # the user's settings are not touched

    def tearDown(self):
        RunCAP.SETTINGS_FILE = self.settings

    def test_kill_verbose_job(self):
        result = {}

        def hook(app, root):
            def start():
                app._start([("Verbose job", [sys.executable, self.spam], self.directory)], "job")
                root.after(1000, press)

            def press():
                result["pid"] = app.process.pid
                result["pressed"] = time.time()
                app.kill()
                root.after(50, watch)

            def watch():
                elapsed = time.time() - result["pressed"]
                if str(app.kill_button.cget("state")) == "disabled":
                    result["shown"] = elapsed
                    result["log"] = app.output.get("1.0", tk.END)
                    root.destroy()
                    return
                if elapsed > 30:
                    root.destroy()
                    return
                root.after(50, watch)
            root.after(500, start)

        RunCAP.run_gui(hook=hook)
        self.assertIn("shown", result, "the end of the killed job was not shown within 30 s")
        self.assertLess(result["shown"], 5.0)
        self.assertIn("Kill requested", result["log"])
        self.assertIn("Killed by the user", result["log"])
        with self.assertRaises(OSError):            # the process is gone
            os.kill(result["pid"], 0)


if __name__ == "__main__":
    unittest.main()
