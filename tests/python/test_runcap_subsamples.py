# **********************************************************************
# Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
# All rights reserved.
#
# For the licensing terms see LICENSE.
# **********************************************************************
"""Tests of the sub-samples of RunCAP.py (RunCAPSubSamples.py): settings saved in the workflow file, seeds of the
sub-samples, stages run and combined in each mode, the files of a production (workflow files, combination, sbatch
script), the parallel runner and, when bin/CAP is built, a production run and combined for real.

Run from the CAP directory:  python3 -m pytest tests/python
"""

import io
import os
import queue
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET
from contextlib import redirect_stdout

CAP_DIR = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, CAP_DIR)
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import RunCAP                                   # noqa: E402
import RunCAPFilters as F                       # noqa: E402
import RunCAPSubSamples as S                    # noqa: E402
from test_runcap_stages import TEMPLATES, events_stage, library      # noqa: E402

#: bin/CAP used by the real run (CAP_TEST_EXE, or that of this CAP directory).
CAP_TEST_EXE = os.environ.get("CAP_TEST_EXE") or RunCAP.CAP_EXE


def workflow(n_events=100):
    """Event processing (generator and analyzers) and its derived histograms, with the files of their jobs."""
    lib = library()
    events = events_stage(lib)
    events[0].param("EVENT:REQUESTED:N").value = str(n_events)
    derived = F.derived_workflow(events, TEMPLATES, RunCAP.TaskConfig)
    stages = {RunCAP.EVENTS: events, RunCAP.DERIVED: derived, RunCAP.BF: []}
    jobs = {RunCAP.EVENTS: {"OUTPUT_FILE": "fill.cap", "INPUT_FILE": ""},
            RunCAP.DERIVED: {"OUTPUT_FILE": "fillDerived.cap", "INPUT_FILE": "fill.cap"}, RunCAP.BF: {}}
    return stages, lib, jobs


def parameters(path):
    """{key: value} of the parameters of the first TASK element of a CAP file."""
    def visit(element, prefix, out):
        for child in element:
            if child.tag == "GROUP":
                visit(child, prefix + child.get("name") + ":", out)
            elif child.tag == "PARAMETER":
                out[prefix + child.get("name")] = (child.text or "").strip()
        return out
    return visit(ET.parse(path).getroot().find("TASK"), "", {})


class TestSettings(unittest.TestCase):

    def test_saved_in_the_workflow(self):
        stages, lib, jobs = workflow()
        s = S.SubSampleSettings()
        s.enabled, s.count, s.parallel, s.mode, s.stage, s.where = True, 25, 3, S.JACKKNIFE_STAGES, RunCAP.DERIVED, S.SLURM
        s.slurm.host, s.slurm.cap, s.slurm.directory = "me@grid.wayne.edu", "/wsu/CAP8.0", "/wsu/prod/run1"
        s.slurm.setup = "module load root\nmodule load pythia"
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "w.xml")
            RunCAP.write_stages(path, stages, lib, TEMPLATES, jobs, [s.to_element()])
            r = S.read_settings(path)
            self.assertEqual((r.enabled, r.count, r.parallel, r.mode, r.stage, r.where),
                             (True, 25, 3, S.JACKKNIFE_STAGES, RunCAP.DERIVED, S.SLURM))
            self.assertEqual(vars(r.slurm), vars(s.slurm))
            # the workflow reads as before (bin/CAP and RunCAP ignore the element)
            loaded, lib2, jobs2, warnings = RunCAP.load_stages_with_filters(path, TEMPLATES)
            self.assertEqual(warnings, [])
            self.assertEqual(sorted(loaded), sorted([RunCAP.EVENTS, RunCAP.DERIVED]))
            # a file without the element: the defaults
            plain = os.path.join(d, "p.xml")
            RunCAP.write_stages(plain, stages, lib, TEMPLATES, jobs)
            self.assertFalse(S.read_settings(plain).enabled)

    def test_checks(self):
        stages, lib, jobs = workflow()
        s = S.SubSampleSettings()
        self.assertEqual(S.check_settings(s, RunCAP, stages), [])
        s.count = 1
        self.assertTrue(any("2 sub-samples" in e for e in S.check_settings(s, RunCAP, stages)))
        s.count, s.prefix = 4, "a b"
        self.assertTrue(any("prefix" in e for e in S.check_settings(s, RunCAP, stages)))
        s.prefix, s.mode, s.stage = "sub_", S.JACKKNIFE_STAGES, RunCAP.EVENTS
        self.assertTrue(any("later stage" in e for e in S.check_settings(s, RunCAP, stages)))
        s.mode, s.stage, s.where = S.SUBSAMPLE, RunCAP.BF, S.SLURM
        errors = S.check_settings(s, RunCAP, stages)
        self.assertTrue(any("no task" in e for e in errors))
        self.assertEqual(len([e for e in errors if e.startswith("Slurm")]), 3)


class TestSeedsAndStages(unittest.TestCase):

    def test_seeds(self):
        stages, lib, jobs = workflow()
        events = stages[RunCAP.EVENTS]
        gen = next(t for t in RunCAP.all_tasks(events) if t.name == "Gen")
        gen.param("Random:SetSeed").value = "false"
        seed = int(gen.param("Random:Seed").value)
        copies, changed = S.seeded_tasks(events, 3)
        gen3 = next(t for t in RunCAP.all_tasks(copies) if t.name == "Gen")
        self.assertEqual(changed, ["Gen:Random:Seed"])
        self.assertEqual(int(gen3.param("Random:Seed").value), seed + 3)
        self.assertEqual(gen3.param("Random:SetSeed").value, "true")
        self.assertEqual(int(gen.param("Random:Seed").value), seed)          # the workflow is unchanged
        self.assertEqual(gen.param("Random:SetSeed").value, "false")
        self.assertEqual(S.seeded_value(str(S.MAX_SEED), 1), "1")          # within the range of Pythia
        self.assertEqual(S.seeded_value("0", 2), "3")

    def test_copies_without_widgets(self):
        """Parameters edited in the window of a task hold a Tk variable (widget_var), which cannot be copied."""
        import threading
        stages, lib, jobs = workflow()
        events = stages[RunCAP.EVENTS]
        for t in RunCAP.all_tasks(events):
            for p in t.params:
                p.widget_var = threading.Lock()                 # not copyable, as a tkinter variable
        copies, changed = S.seeded_tasks(events, 1)
        self.assertEqual(changed, ["Gen:Random:Seed"])
        self.assertFalse(any(hasattr(p, "widget_var") for t in RunCAP.all_tasks(copies) for p in t.params))
        self.assertTrue(all(hasattr(p, "widget_var") for t in RunCAP.all_tasks(events) for p in t.params))
        events[0].clone()

    def test_stages_of_the_modes(self):
        stages, lib, jobs = workflow()
        s = S.SubSampleSettings()
        for mode, run, replica in ((S.SUBSAMPLE, [RunCAP.EVENTS, RunCAP.DERIVED], []),
                                   (S.JACKKNIFE, [RunCAP.EVENTS, RunCAP.DERIVED], []),
                                   (S.JACKKNIFE_STAGES, [RunCAP.EVENTS], [RunCAP.DERIVED])):
            s.mode = mode
            self.assertEqual(S.subsample_stages(s, RunCAP, stages), run)
            self.assertEqual(S.jackknife_stages(s, RunCAP, stages), replica)
        s.mode, s.stage = S.SUBSAMPLE, RunCAP.EVENTS
        self.assertEqual(S.subsample_stages(s, RunCAP, stages), [RunCAP.EVENTS])


class TestProduction(unittest.TestCase):

    def test_files(self):
        stages, lib, jobs = workflow()
        s = S.SubSampleSettings()
        s.count, s.mode = 3, S.JACKKNIFE_STAGES
        with tempfile.TemporaryDirectory() as d:
            p = S.write_production(d, s, RunCAP, stages, lib, TEMPLATES, jobs)
            self.assertEqual([os.path.basename(f) for f in p.files], ["sub_000.xml", "sub_001.xml", "sub_002.xml"])
            self.assertEqual(p.stages, [RunCAP.EVENTS])
            self.assertEqual(p.seeds, ["Gen:Random:Seed"])
            self.assertEqual(p.outputs, ["fill.cap"])
            seeds = []
            for k, f in enumerate(p.files):
                tops = ET.parse(f).getroot().findall("TASK")
                self.assertEqual([t.get("stage") for t in tops], [RunCAP.EVENTS])       # event processing only
                keys = parameters(f)
                self.assertEqual(keys["JOB:SEED"], str(s.seed + k))
                self.assertEqual(keys["JOB:OUTPUT_FILE"], "fill.cap")
                seeds.append(next(e.text for e in ET.parse(f).iter("PARAMETER") if e.get("name") == "Seed"))
            self.assertEqual(len(set(seeds)), 3)
            c = parameters(p.combine)
            self.assertEqual(c["MODE"], "JACKKNIFE")
            self.assertEqual(c["JACKKNIFE:STAGES"], "DerivedHistos")
            self.assertEqual(c["JACKKNIFE:WORKFLOW"], p.base)
            self.assertEqual(c["HISTOGRAM:IMPORT:FILE_NAME"], "fill.cap")
            self.assertEqual(c["HISTOGRAM:EXPORT:FILE_NAME"], "fillDerived_Jackknife.cap")
            self.assertEqual(c["SUBSAMPLE:PATTERN"], "sub_*")
            self.assertEqual(c["JOB:OUTPUT_FILE"], "NONE")
            self.assertEqual(sorted(t.get("stage") for t in ET.parse(p.base).getroot().findall("TASK")),
                             sorted([RunCAP.EVENTS, RunCAP.DERIVED]))
            cmd = S.subsample_commands(p, s, RunCAP, 1)
            self.assertEqual(len(cmd), 1)
            self.assertEqual(cmd[0][1][1:4], ["Workflow", d + "/", "sub_001.xml"])
            self.assertEqual(cmd[0][2], os.path.join(d, "sub_001"))
            self.assertEqual(S.combine_command(p, RunCAP)[1][1:4], [S.COMBINE_TASK, d + "/", S.COMBINE_FILE])
            self.assertEqual(p.sbatch, "")

            # sub-sample average of the derived histograms: both stages run by each sub-sample
            s.mode = S.SUBSAMPLE
            p = S.write_production(d, s, RunCAP, stages, lib, TEMPLATES, jobs)
            self.assertEqual(p.stages, [RunCAP.EVENTS, RunCAP.DERIVED])
            self.assertEqual([c[1][1] for c in S.subsample_commands(p, s, RunCAP, 0)], ["Workflow", "DerivedHistos"])
            c = parameters(p.combine)
            self.assertEqual((c["MODE"], c["JACKKNIFE:STAGES"], c["HISTOGRAM:IMPORT:FILE_NAME"],
                              c["HISTOGRAM:EXPORT:FILE_NAME"]),
                             ("SUBSAMPLE", "NONE", "fillDerived.cap", "fillDerived_SubSample.cap"))

    def test_combine_only(self):
        stages, lib, jobs = workflow()
        s = S.SubSampleSettings()
        with tempfile.TemporaryDirectory() as d:
            p = S.write_production(d, s, RunCAP, stages, lib, TEMPLATES, jobs, subsample_files=False)
            self.assertEqual(sorted(os.listdir(d)), sorted([S.BASE_FILE, S.COMBINE_FILE]))
            self.assertEqual(p.files, [])

    def test_slurm(self):
        stages, lib, jobs = workflow()
        s = S.SubSampleSettings()
        s.count, s.where = 20, S.SLURM
        s.slurm.host, s.slurm.cap, s.slurm.directory = "me@grid", "/wsu/CAP8.0", "/wsu/prod/run1"
        s.slurm.partition, s.slurm.setup = "primary", "module load root\nmodule load pythia"
        with tempfile.TemporaryDirectory() as d:
            p = S.write_production(d, s, RunCAP, stages, lib, TEMPLATES, jobs)
            with open(p.sbatch) as f:
                text = f.read()
            for line in ("#SBATCH --array=0-19", "#SBATCH --partition=primary", "#SBATCH --time=04:00:00",
                         "#SBATCH --mem=4G", "#SBATCH --output=/wsu/prod/run1/logs/sub_%03a.log",
                         "module load root", "module load pythia", "export CAP_ROOT_PATH=/wsu/CAP8.0",
                         "$CAP_ROOT_PATH/bin/CAP Workflow $WORK/ sub_$K.xml $OUT/",
                         "$CAP_ROOT_PATH/bin/CAP DerivedHistos $WORK/ sub_$K.xml $OUT/"):
                self.assertIn(line, text)
            self.assertNotIn("#SBATCH --account", text)
            submit = S.submit_commands(p, s)
            self.assertEqual(submit[0][1], ["ssh", "-o", "BatchMode=yes", "me@grid", "mkdir -p /wsu/prod/run1/logs"])
            self.assertEqual(submit[1][1][-1], "me@grid:/wsu/prod/run1/")
            self.assertIn("sbatch submit.sbatch", submit[2][1][-1])
            self.assertIn("squeue", S.status_commands(p, s)[0][1][-1])
            self.assertIn("/wsu/prod/run1/sub_*/fillDerived.cap", S.status_commands(p, s)[0][1][-1])
            fetch = S.fetch_commands(p, s)[0][1]
            self.assertEqual(fetch[-2:], ["me@grid:/wsu/prod/run1/", d + "/"])

    def test_command_line(self):
        stages, lib, jobs = workflow()
        s = S.SubSampleSettings()
        s.count = 2
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "w.xml")
            RunCAP.write_stages(path, stages, lib, TEMPLATES, jobs, [s.to_element()])
            out = io.StringIO()
            with redirect_stdout(out):
                code = RunCAP.main(["--subsamples", path, os.path.join(d, "prod")])
            self.assertEqual(code, 0)
            text = out.getvalue()
            self.assertIn("2 sub-samples", text)
            self.assertIn("sub_001.xml", text)
            self.assertIn("Then combine with:", text)
            self.assertTrue(os.path.exists(os.path.join(d, "prod", "sub_001.xml")))


class TestUnusedGroups(unittest.TestCase):

    def test_dropped_on_load(self):
        """Keys of corrections in a calculator (written by older versions): dropped with one warning per task."""
        element = ET.fromstring(
            '<TASK name="SingleDerived" class="CAP::ParticleSingleDerivedCalculator">'
            '<GROUP name="PARTICLE_SMEARER"><PARAMETER name="USE" type="bool">false</PARAMETER>'
            '<GROUP name="IMPORT"><PARAMETER name="PATH" type="string">DEFAULT</PARAMETER></GROUP></GROUP>'
            '<GROUP name="EVENT_EFFICIENCY"><PARAMETER name="CREATE" type="bool">false</PARAMETER></GROUP>'
            '<GROUP name="EVENT_FILTER"><PARAMETER name="USE" type="bool">true</PARAMETER></GROUP>'
            '<PARAMETER name="Unknown" type="int">3</PARAMETER></TASK>')
        warnings = []
        t = RunCAP.task_from_element(element, TEMPLATES, warnings)
        self.assertFalse(any(p.key.startswith(("PARTICLE_SMEARER", "EVENT_EFFICIENCY")) for p in t.params))
        self.assertEqual(t.param("EVENT_FILTER:USE").value, "true")
        self.assertIsNotNone(t.param("Unknown"))                     # other keys: kept, with a warning
        self.assertEqual(len(warnings), 2)
        self.assertIn("parameter Unknown is not defined", warnings[0])
        self.assertIn("PARTICLE_SMEARER, EVENT_EFFICIENCY were dropped", warnings[1])


class TestSources(unittest.TestCase):

    def test_calculator_without_its_analyzer(self):
        """A calculator of derived histograms whose analyzer was removed from event processing: an error when both
        stages run (also before the sub-samples), none when the derived stage runs alone."""
        stages, lib, jobs = workflow()
        self.assertEqual(RunCAP.check_sources(stages), [])
        events = stages[RunCAP.EVENTS]
        iterator = events[0]
        iterator.children = [c for c in iterator.children if c.name != "Pair"]
        errors = RunCAP.check_sources(stages)
        self.assertEqual(len(errors), 1)
        self.assertIn("task PairDerived reads the histograms of Pair", errors[0])
        self.assertEqual(RunCAP.check_sources(stages, [RunCAP.DERIVED]), [])


class TestRunner(unittest.TestCase):

    def test_parallel_and_failures(self):
        q = queue.Queue()
        with tempfile.TemporaryDirectory() as d:
            sequences = [("a", [("one", ["sh", "-c", "echo a1"], d), ("two", ["sh", "-c", "echo a2"], d)]),
                         ("b", [("one", ["sh", "-c", "exit 3"], d), ("two", ["sh", "-c", "echo never"], d)]),
                         ("c", [("one", ["sh", "-c", "echo c1"], d)])]
            final = [("Final", ["sh", "-c", "echo final"], d)]
            ok, message = S.ParallelRunner(sequences, 2, dict(os.environ), q.put, final).run()
            lines = []
            while not q.empty():
                item = q.get()
                if item[0] == "line":
                    lines.append(item[1])
            self.assertFalse(ok)
            self.assertIn("1 sub-sample(s) failed: b", message)
            self.assertIn("[a] a2\n", lines)
            self.assertIn("[c] c1\n", lines)
            self.assertNotIn("[b] never\n", lines)
            self.assertNotIn("final\n", lines)                   # not combined after a failure
            ok, message = S.ParallelRunner(sequences[:1], 4, dict(os.environ), q.put, final).run()
            self.assertTrue(ok)


@unittest.skipUnless(os.path.exists(CAP_TEST_EXE), "bin/CAP is not built")
class TestRealRun(unittest.TestCase):

    def test_run_and_combine(self):
        """Three sub-samples of a small workflow, run by bin/CAP and combined by the jackknife through stages."""
        stages, lib, jobs = workflow(100)
        s = S.SubSampleSettings()
        s.count, s.parallel, s.mode = 3, 3, S.JACKKNIFE_STAGES
        saved = RunCAP.CAP_EXE
        RunCAP.CAP_EXE = CAP_TEST_EXE
        try:
            with tempfile.TemporaryDirectory() as d:
                p = S.write_production(d, s, RunCAP, stages, lib, TEMPLATES, jobs)
                sequences = [(S.directory_name(s, k), S.subsample_commands(p, s, RunCAP, k)) for k in range(s.count)]
                env = RunCAP.job_environment()
                env.setdefault("CAP_DATABASE_PATH", os.path.join(CAP_DIR, "DB") + "/")
                q = queue.Queue()
                ok, message = S.ParallelRunner(sequences, s.parallel, env, q.put, [S.combine_command(p, RunCAP)]).run()
                self.assertTrue(ok, message)
                outputs = [os.path.join(d, "sub_%03d" % k, "fill.cap") for k in range(3)]
                contents = set()
                for f in outputs:
                    with open(f, "rb") as h:
                        contents.add(h.read())
                self.assertEqual(len(contents), 3)                     # different seeds: different events
                self.assertTrue(os.path.getsize(os.path.join(d, "fillDerived_Jackknife.cap")) > 0)
                self.assertTrue(os.path.getsize(os.path.join(d, "fillDerived_Jackknife_biasCorrected.cap")) > 0)
        finally:
            RunCAP.CAP_EXE = saved


if __name__ == "__main__":
    unittest.main()
