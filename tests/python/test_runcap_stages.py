# **********************************************************************
# Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
# All rights reserved.
#
# For the licensing terms see LICENSE.
# **********************************************************************
"""Tests of the stages of a workflow (event processing, derived histograms, balance functions): files holding several
stages, files written before the stages, and the stage buttons of RunDatum.py.

Run from the CAP directory:  python3 -m unittest discover -s tests/python
"""

import os
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET

CAP_DIR = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, CAP_DIR)

import RunDatum                                   # noqa: E402
import RunCAPFilters as F                       # noqa: E402

TEMPLATES = RunDatum.load_templates(os.path.join(CAP_DIR, "DB", "Templates"))


def task(cls, name=None):
    t = TEMPLATES["CAP::" + cls].clone()
    if name:
        t.name = name
    return t


def library():
    lib = F.FilterLibrary.with_defaults()
    lib.add(F.FilterDef("EVENT_FILTER", "MB", "minimum bias", [F.Condition("MULT_0", F.RANGE, 0, 1e6)]))
    lib.add(F.FilterDef("PARTICLE_FILTER", "PiP", "pi+", [F.Condition("PDG", F.VALUE, 211)]))
    lib.add(F.FilterDef("PARTICLE_FILTER", "PiM", "pi-", [F.Condition("PDG", F.VALUE, -211)]))
    return lib


def events_stage(lib):
    """EventIterator with a generator, a single-particle and a pair analyzer (events: the first of the library)."""
    it, gen = task("EventIterator", "Run"), task("BasicEventGen", "Gen")
    single, pair = task("ParticleSingleAnalyzer", "Single"), task("ParticlePairAnalyzer", "Pair")
    it.children = [gen, single, pair]
    event = lib.names("EVENT")[0]
    for t in (it, gen, single, pair):
        F.default_event_settings(t, TEMPLATES)
        F.default_histogram_settings(t, TEMPLATES)
        F.default_choices(t, lib, TEMPLATES)
    for t in (single, pair):
        F.choices(t).update({"EVENT_FILTER": ["MB"], "PARTICLE_FILTER": ["PiP", "PiM"]})
    F.choices(gen).update({"EVENT_FILTER": ["MB"], "PARTICLE_FILTER": ["PiP"]})
    F.choices(it)["EVENT"] = [event]
    it.param("EVENT:USE").value = "true"
    return [it]


class TestStageFiles(unittest.TestCase):

    def test_two_stages(self):
        """Event processing and derived histograms in one file: a TASK element each, with the files of its job."""
        lib = library()
        events = events_stage(lib)
        derived = F.derived_workflow(events, TEMPLATES, RunDatum.TaskConfig)
        self.assertEqual([t.cls for t in derived], ["CAP::ParticleSingleDerivedCalculator",
                                                    "CAP::ParticlePairDerivedCalculator"])
        jobs = {RunDatum.EVENTS: {"OUTPUT_FILE": "fill.cap", "INPUT_FILE": ""},
                RunDatum.DERIVED: {"OUTPUT_FILE": "fillDerived.cap", "INPUT_FILE": "fill.cap"}}
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "w.xml")
            names = RunDatum.write_stages(path, {RunDatum.EVENTS: events, RunDatum.DERIVED: derived, RunDatum.BF: []},
                                        lib, TEMPLATES, jobs)
            self.assertEqual(names, {RunDatum.EVENTS: "Workflow", RunDatum.DERIVED: "DerivedHistos"})
            tops = ET.parse(path).getroot().findall("TASK")
            self.assertEqual([(t.get("name"), t.get("stage")) for t in tops],
                             [("Workflow", "EventProcessing"), ("DerivedHistos", "DerivedHistos")])
            for top, job in zip(tops, (jobs[RunDatum.EVENTS], jobs[RunDatum.DERIVED])):
                keys = {p.get("name"): p.text for g in top.findall("GROUP") if g.get("name") == "JOB"
                        for p in g.findall("PARAMETER")}
                self.assertEqual(keys, {k: v for k, v in job.items() if v})
                self.assertEqual(top.find("TASK").get("class"), F.CREATOR_CLASS)      # FilterCreator first
            stages, lib2, jobs2, warnings = RunDatum.load_stages_with_filters(path, TEMPLATES)
            self.assertEqual(warnings, [])
            self.assertEqual(sorted(stages), sorted([RunDatum.EVENTS, RunDatum.DERIVED]))
            self.assertEqual([t.name for t in RunDatum.all_tasks(stages[RunDatum.EVENTS])], ["Run", "Gen", "Single", "Pair"])
            self.assertEqual([t.name for t in stages[RunDatum.DERIVED]], ["SingleDerived", "PairDerived"])
            self.assertEqual(jobs2[RunDatum.EVENTS], dict(RunDatum.new_job(), **jobs[RunDatum.EVENTS]))
            self.assertEqual(jobs2[RunDatum.DERIVED], dict(RunDatum.new_job(), **jobs[RunDatum.DERIVED]))
            self.assertEqual(lib2.objects, lib.objects)
            self.assertEqual(F.choices(stages[RunDatum.DERIVED][1])["PARTICLE_FILTER"], ["PiP", "PiM"])
            # the command of each stage runs its task
            self.assertEqual(RunDatum.run_root_name(stages[RunDatum.DERIVED], lib2, TEMPLATES, RunDatum.DERIVED),
                             "DerivedHistos")
            self.assertEqual(RunDatum.run_root_name(stages[RunDatum.EVENTS], lib2, TEMPLATES, RunDatum.EVENTS), "Workflow")
            # written again: the same file
            path2 = os.path.join(d, "w2.xml")
            RunDatum.write_stages(path2, stages, lib2, TEMPLATES, jobs2)
            def strip(p):
                with open(p) as f:
                    return [line for line in f if "written by RunDatum" not in line]
            self.assertEqual(strip(path), strip(path2))

    def test_earlier_files(self):
        """Files written before the stages: a fill run is event processing; a derived run (calculators only) is the
        stage of derived histograms, with the files of its job."""
        lib = library()
        events = events_stage(lib)
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "fill.xml")
            lib.job.update({"OUTPUT_FILE": "fill.cap"})
            RunDatum.write_workflow(path, events, lib, TEMPLATES)
            stages, _, jobs, _ = RunDatum.load_stages_with_filters(path, TEMPLATES)
            self.assertEqual(list(stages), [RunDatum.EVENTS])
            self.assertEqual(jobs[RunDatum.EVENTS]["OUTPUT_FILE"], "fill.cap")
            # a derived run written by the earlier RunDatum.py: one TASK element named Workflow, no stage attribute
            derived = F.derived_workflow(events, TEMPLATES, RunDatum.TaskConfig)
            lib.job.update({"INPUT_FILE": "fill.cap", "OUTPUT_FILE": "derived.cap"})
            path = os.path.join(d, "derived.xml")
            RunDatum.write_workflow(path, derived, lib, TEMPLATES)
            tree = ET.parse(path)
            for t in tree.getroot().findall("TASK"):
                del t.attrib["stage"]
            tree.write(path)
            stages, _, jobs, warnings = RunDatum.load_stages_with_filters(path, TEMPLATES)
            self.assertEqual(list(stages), [RunDatum.DERIVED])
            self.assertEqual([t.name for t in stages[RunDatum.DERIVED]], ["SingleDerived", "PairDerived"])
            self.assertEqual((jobs[RunDatum.DERIVED]["INPUT_FILE"], jobs[RunDatum.DERIVED]["OUTPUT_FILE"]),
                             ("fill.cap", "derived.cap"))

    def test_balance_function_tasks(self):
        """Tasks of balance functions belong to their stage; the names of the tasks of the stages are reserved."""
        bf = RunDatum.TaskConfig("BF", "CAP::ParticlePairBfCalculator")
        self.assertTrue(RunDatum.is_bf_task(bf))
        self.assertFalse(RunDatum.is_bf_task(task("ParticlePairDerivedCalculator")))
        self.assertIn("Task BF computes balance functions: it belongs to the stage Balance Fct.",
                      RunDatum.check_workflow([bf], RunDatum.DERIVED))
        self.assertEqual(RunDatum.check_workflow([bf], RunDatum.BF), [])
        self.assertIn("The name DerivedHistos is reserved", "\n".join(
            RunDatum.check_workflow([RunDatum.TaskConfig("DerivedHistos", "CAP::Task")], RunDatum.EVENTS)))


try:
    import tkinter
    _display = bool(os.environ.get("DISPLAY")) or sys.platform == "darwin"
except ImportError:
    _display = False


@unittest.skipUnless(_display, "tkinter or a display is not available")
class TestStageButtons(unittest.TestCase):
    """The buttons of the stages of RunDatum.py, on the application itself."""

    def run_app(self, steps):
        """Run the application and the function steps(app) once it is up; the window is then closed."""
        errors = []

        def hook(app, root):
            def go():
                try:
                    steps(app)
                except Exception as e:           # reported after the main loop
                    import traceback
                    errors.append(traceback.format_exc())
                finally:
                    root.quit()            # (other Tk windows of the tests may exist: mainloop would not return)
                    root.destroy()
            app.templates = TEMPLATES
            root.after(300, go)
        settings = RunDatum.load_settings
        RunDatum.load_settings = lambda: dict(output_dir=tempfile.gettempdir(), geometry="")
        try:
            RunDatum.run_gui(hook=hook)
        finally:
            RunDatum.load_settings = settings
        if errors:
            self.fail(errors[0])

    def test_stages(self):
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "w.xml")

            def steps(app):
                app.templates = TEMPLATES
                app.library = library()
                app.jobs[RunDatum.EVENTS]["OUTPUT_FILE"] = "fill.cap"
                app.library.job = app.jobs[RunDatum.EVENTS]
                app.filters_panel.library = app.library
                app.set_workflow(events_stage(app.library))
                app._changed()
                buttons = app.stage_buttons
                self.assertEqual([buttons[s].cget("text") for s in RunDatum.STAGES],
                                 ["Event Processing", "Derived Histos", "Balance Fct"])
                self.assertIn("disabled", buttons[RunDatum.BF].state())          # no task of balance functions
                self.assertEqual(app.run_button.cget("text"), "▶ Run Event Processing")
                # Derived Histos: the calculators of the analyzers; event processing is kept
                app.show_stage(RunDatum.DERIVED)
                self.assertEqual(app.stage, RunDatum.DERIVED)
                self.assertEqual([t.name for t in app.workflow()], ["SingleDerived", "PairDerived"])
                self.assertEqual(app.library.job, {"INPUT_FILE": "fill.cap", "OUTPUT_FILE": "fillDerived.cap", "INPUT_PATH": ""})
                self.assertEqual(app.job_vars["INPUT_FILE"].get(), "fill.cap")
                self.assertEqual([t.name for t in RunDatum.all_tasks(app.stage_tasks(RunDatum.EVENTS))],
                                 ["Run", "Gen", "Single", "Pair"])
                self.assertEqual(app.run_button.cget("text"), "▶ Run Derived Histos")
                # back to event processing: its tasks and files
                app.show_stage(RunDatum.EVENTS)
                self.assertEqual([t.name for t in app.workflow()], ["Run"])
                self.assertEqual(app.job_vars["OUTPUT_FILE"].get(), "fill.cap")
                self.assertEqual(app.job_vars["INPUT_FILE"].get(), "")
                # shown again: the calculators are those made before (not made again)
                app.show_stage(RunDatum.DERIVED)
                app.items[app.tree.get_children("")[0]].name = "Renamed"
                app.show_stage(RunDatum.EVENTS)
                app.show_stage(RunDatum.DERIVED)
                self.assertEqual([t.name for t in app.workflow()], ["Renamed", "PairDerived"])
                # saved and loaded: both stages
                RunDatum.write_stages(path, app.all_stages(), app.library, app.templates, app.jobs)
                app.dirty = False
                app.load_workflow(path)
                self.assertEqual(app.stage, RunDatum.EVENTS)
                self.assertEqual([t.name for t in app.stage_tasks(RunDatum.DERIVED)], ["Renamed", "PairDerived"])
                self.assertIn("disabled", app.stage_buttons[RunDatum.BF].state())
                # a task of balance functions (a class of a later build) makes the stage Balance Fct live
                app.stages[RunDatum.BF] = [RunDatum.TaskConfig("BF", "CAP::ParticlePairBfCalculator", known=False)]
                app._update_stages()
                self.assertNotIn("disabled", app.stage_buttons[RunDatum.BF].state())
                app.show_stage(RunDatum.BF)
                self.assertEqual([t.name for t in app.workflow()], ["BF"])
                self.assertEqual(app.stage_var.get(), RunDatum.BF)

            self.run_app(steps)


if __name__ == "__main__":
    unittest.main()
