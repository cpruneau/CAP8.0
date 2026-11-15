# **********************************************************************
# Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
# All rights reserved.
#
# For the licensing terms see LICENSE.
# **********************************************************************
"""
RunCAPSubSamples.py -- sub-samples of a workflow of RunCAP.py, and their combination (SubSampleStatCalculator).

A workflow is run as N independent jobs, the sub-samples, each with its own seeds, in the directories
<output>/<prefix>000, <prefix>001, ... of the output directory. Their outputs are then combined by the task
CAP::SubSampleStatCalculator (a CAP job of its own), in one of the modes:
  * Sub-sample average (SUBSAMPLE): each sub-sample runs the stages up to the stage combined; its output file is
    averaged over the sub-samples, with the uncertainty of the mean;
  * Jackknife on results: the same runs; the delete-one jackknife of the output files of the stage combined;
  * Jackknife through stages: the sub-samples run event processing only; the later stages (Derived Histos, Balance
    Fct, up to the stage combined) are run by the combination on the whole sample and on each replica (the sample
    without one sub-sample), so that the nonlinear observables are computed from (nearly) all the statistics.
The jackknife also writes the bias-corrected values, to a file of their own (<output>_biasCorrected.cap).

Seeds: in the workflow file of sub-sample k (<output>/<prefix>k.xml), every parameter Seed or SeedValue of the tasks
(BasicEventGen Random:Seed, PythiaEventGenerator SeedValue, ...) is increased by k and the matching SetSeed switches are
turned on; the seed of gRandom (key JOB:SEED of the task CAP runs, see CAP.cxx) is the base seed + k.

Where: the sub-samples run on this computer (Local: several at once, each running its stages in order), or on a Slurm
cluster (Slurm): RunCAP writes the files of the production (the workflow files of the sub-samples and an sbatch script
of a job array, submit.sbatch) in the output directory; Submit copies them to the cluster (rsync over ssh, which must
log in without a password: ssh keys) and submits the array; Status shows the queue and the number of sub-samples done;
Fetch copies their output files back. Combine then runs the combination here. The script can also be submitted by hand,
and Combine run on the outputs of jobs run elsewhere: it needs the directories <prefix>* holding the output files.

pT-hat bins (PYTHIA): with bin edges p0 < p1 < ... < pK (GeV/c), each sub-sample is K jobs, <prefix>sss_pthat<kk>,
generating the same number of events with PythiaEventGenerator PTHAT_MIN = p_k and PTHAT_MAX = p_k+1 (a hard process,
e.g. Option HardQCD:all = on; not SoftQCD), each with a seed of its own, keeping the raw histograms (JOB:NORMALIZE
false). The combination adds the bins of each sub-sample with the weights sigma_k/N_k (sigma_k the cross section PYTHIA
gives for the bin): the differential cross sections in mb (or, with the normalization per event, divided by the cross
section of the range covered), then combines the sub-samples as set above. The jobs run event processing only: the later
stages are run by the combination (Jackknife through stages).

The settings are saved in the workflow file (element SUBSAMPLES of the root, ignored by bin/CAP).
"""

import copy
import os
import re
import shlex
import signal
import subprocess
import threading
import xml.etree.ElementTree as ET

#: Modes of the combination (CAP::SubSampleStatCalculator), and their titles.
SUBSAMPLE, JACKKNIFE, JACKKNIFE_STAGES = "SUBSAMPLE", "JACKKNIFE", "JACKKNIFE_STAGES"
MODES = (SUBSAMPLE, JACKKNIFE, JACKKNIFE_STAGES)
MODE_TITLES = {SUBSAMPLE: "Sub-sample average", JACKKNIFE: "Jackknife on results",
               JACKKNIFE_STAGES: "Jackknife through stages"}
LOCAL, SLURM = "LOCAL", "SLURM"
#: Normalizations of a production in pT-hat bins: the differential cross sections (mb), or per event of the range
#: covered (divided by the sum of the cross sections of the bins).
SIGMA, COVERED = "SIGMA", "COVERED"
NORMALIZATION_TITLES = {SIGMA: "dσ (mb)", COVERED: "per event"}
#: Class of the generator whose pT-hat range is set per job, and its keys.
PTHAT_GENERATOR = "PythiaEventGenerator"
PTHAT_MIN_KEY, PTHAT_MAX_KEY = "PTHAT_MIN", "PTHAT_MAX"
#: Parameters holding a seed (key Seed or SeedValue, alone or after a group) and their switches.
SEED_KEY = re.compile(r"(?:^|:)(?:Seed|SeedValue)$")
SET_SEED_KEY = re.compile(r"(?:^|:)SetSeed$")
#: Largest seed accepted by Pythia (Random:seed).
MAX_SEED = 900000000
#: Name of the task of the combination, and files of the production.
COMBINE_TASK = "SubSampleCombine"
COMBINE_FILE, BASE_FILE, SBATCH_FILE = "RunCAP_combine.xml", "RunCAP_workflow.xml", "submit.sbatch"


class SlurmSettings:
    """Where and how the job array runs on a Slurm cluster."""

    FIELDS = ("host", "cap", "directory", "partition", "time", "memory", "account", "setup")

    def __init__(self):
        self.host = ""          # user@login node (ssh)
        self.cap = ""           # CAP directory on the cluster (bin/CAP, lib, DB)
        self.directory = ""     # directory of the production on the cluster
        self.partition = ""
        self.time = "04:00:00"
        self.memory = "4G"
        self.account = ""
        self.setup = ""         # lines run before CAP (e.g. module load root pythia gsl)


class SubSampleSettings:
    """Sub-samples of a workflow and their combination (saved in the workflow file)."""

    def __init__(self):
        self.enabled = False
        self.count = 10
        self.parallel = max(1, os.cpu_count() or 1)
        self.mode = SUBSAMPLE
        self.stage = ""          # stage combined (empty: the last stage with tasks)
        self.where = LOCAL
        self.prefix = "sub_"
        self.seed = 112133       # seed of gRandom of sub-sample 0 (sub-sample k: seed + k)
        self.weights = "EVENTS"
        self.ptHatEdges = ""      # edges of the pT-hat bins (GeV/c, e.g. "5 10 20 40 80"); empty: no pT-hat bins
        self.normalization = SIGMA
        self.slurm = SlurmSettings()

    def edges(self):
        """Edges of the pT-hat bins ([] without pT-hat bins). Raises ValueError if they are not numbers."""
        return [float(x) for x in re.split(r"[,; ]+", self.ptHatEdges.strip()) if x] if self.ptHatEdges.strip() else []

    def nbins(self):
        """Number of pT-hat bins (1 without pT-hat bins: one job per sub-sample)."""
        try:
            return max(1, len(self.edges()) - 1)
        except ValueError:
            return 1

    def pthat(self):
        """Whether the sub-samples are made of pT-hat bins."""
        try:
            return len(self.edges()) >= 2
        except ValueError:
            return False

    def njobs(self):
        """Number of jobs: one per sub-sample and pT-hat bin."""
        return self.count * (self.nbins() if self.pthat() else 1)

    def to_element(self):
        e = ET.Element("SUBSAMPLES", {"enabled": "true" if self.enabled else "false", "count": str(self.count),
                                      "parallel": str(self.parallel), "mode": self.mode, "stage": self.stage,
                                      "where": self.where, "prefix": self.prefix, "seed": str(self.seed),
                                      "weights": self.weights, "pthat": self.ptHatEdges.strip(),
                                      "normalization": self.normalization})
        s = ET.SubElement(e, "SLURM", {f: getattr(self.slurm, f) for f in SlurmSettings.FIELDS if f != "setup"})
        if self.slurm.setup.strip():
            ET.SubElement(s, "SETUP").text = self.slurm.setup.strip()
        return e

    @staticmethod
    def from_element(e):
        s = SubSampleSettings()
        if e is None:
            return s
        s.enabled = e.get("enabled", "false").lower() in ("1", "true", "yes", "on")
        for name, convert in (("count", int), ("parallel", int), ("seed", int)):
            try:
                setattr(s, name, convert(e.get(name, getattr(s, name))))
            except ValueError:
                pass
        s.mode = e.get("mode", s.mode) if e.get("mode") in MODES else s.mode
        s.stage = e.get("stage", "")
        s.where = e.get("where", s.where) if e.get("where") in (LOCAL, SLURM) else s.where
        s.prefix = e.get("prefix", s.prefix) or s.prefix
        s.weights = e.get("weights", s.weights) if e.get("weights") in ("EVENTS", "EQUAL") else s.weights
        s.ptHatEdges = e.get("pthat", "") or ""
        s.normalization = e.get("normalization") if e.get("normalization") in (SIGMA, COVERED) else SIGMA
        slurm = e.find("SLURM")
        if slurm is not None:
            for f in SlurmSettings.FIELDS:
                if f != "setup" and slurm.get(f) is not None:
                    setattr(s.slurm, f, slurm.get(f))
            setup = slurm.find("SETUP")
            if setup is not None:
                s.slurm.setup = (setup.text or "").strip()
        return s


def read_settings(path):
    """Sub-sample settings of a workflow file (defaults if it has none)."""
    try:
        return SubSampleSettings.from_element(ET.parse(path).getroot().find("SUBSAMPLES"))
    except (ET.ParseError, OSError):
        return SubSampleSettings()


def check_settings(settings, api, stages):
    """Problems of the settings for a workflow ({stage: tasks})."""
    errors = []
    if settings.count < 2:
        errors.append("At least 2 sub-samples are needed.")
    if settings.parallel < 1:
        errors.append("At least one job must run at a time.")
    if not re.fullmatch(r"[A-Za-z0-9_.-]+", settings.prefix or ""):
        errors.append("Invalid prefix of the sub-sample directories '%s' (letters, digits, _ . -)." % settings.prefix)
    present = [s for s in api.STAGES if stages.get(s)]
    if not present:
        errors.append("The workflow has no task.")
        return errors
    if settings.stage and settings.stage not in present:
        errors.append("The stage combined (%s) has no task." % api.STAGE_TITLES.get(settings.stage, settings.stage))
    if settings.mode == JACKKNIFE_STAGES:
        if present[0] != api.EVENTS:
            errors.append("The jackknife through stages starts from event processing: the workflow has none.")
        elif combined_stage(settings, api, stages) == api.EVENTS:
            errors.append("The jackknife through stages needs a later stage to combine (Derived Histos or Balance Fct).")
    errors += check_pthat(settings, api, stages, present)
    if settings.where == SLURM:
        for name, title in (("host", "host"), ("cap", "CAP directory"), ("directory", "production directory")):
            if not getattr(settings.slurm, name).strip():
                errors.append("Slurm: enter the %s (Slurm…)." % title)
    return errors


def _tasks_of(tasks):
    for t in tasks:
        yield t
        yield from _tasks_of(t.children)


def pthat_generators(tasks):
    """The PYTHIA generators of tasks (and subtasks), whose pT-hat range is set per job."""
    return [t for t in _tasks_of(tasks) if t.cls.replace("CAP::", "") == PTHAT_GENERATOR]


def check_pthat(settings, api, stages, present):
    """Problems of a production in pT-hat bins: edges, a PYTHIA generator with a hard process, the stages."""
    if not settings.ptHatEdges.strip():
        return []
    try:
        edges = settings.edges()
    except ValueError:
        return ["pT-hat bins: the edges must be numbers (GeV/c), e.g. 5 10 20 40 80."]
    errors = []
    if len(edges) < 2:
        errors.append("pT-hat bins: at least two edges are needed (one bin).")
    elif any(not b > a for a, b in zip(edges, edges[1:])) or not edges[0] > 0:
        errors.append("pT-hat bins: the edges must be positive and increasing (the first one the hard pT-hat minimum).")
    generators = pthat_generators(stages.get(api.EVENTS) or [])
    if not generators:
        errors.append("pT-hat bins: event processing has no %s." % PTHAT_GENERATOR)
    for g in generators:
        options = [(p.value or "").replace(" ", "").lower() for p in g.params if re.fullmatch(r"Option\d+", p.key)]
        if any(o.startswith("softqcd:") and o.endswith("=on") for o in options):
            errors.append("pT-hat bins: %s generates SoftQCD processes, which ignore the pT-hat range: use a hard "
                          "process (e.g. HardQCD:all = on)." % g.name)
        if not any(o.startswith("hardqcd:") and o.endswith("=on") for o in options):
            errors.append("pT-hat bins: %s has no hard process switched on (e.g. an Option HardQCD:all = on)." % g.name)
    if present and present[0] != api.EVENTS:
        errors.append("pT-hat bins: the workflow has no event processing.")
    elif len(present) > 1 and settings.mode != JACKKNIFE_STAGES and combined_stage(settings, api, stages) != api.EVENTS:
        errors.append("pT-hat bins: the jobs run event processing only; the later stages are run on the combined "
                      "bins: choose Jackknife through stages (or combine event processing).")
    return errors


def combined_stage(settings, api, stages):
    """The stage whose output is combined: the stage chosen, or the last stage with tasks."""
    present = [s for s in api.STAGES if stages.get(s)]
    if settings.stage in present:
        return settings.stage
    return present[-1] if present else api.EVENTS


def subsample_stages(settings, api, stages):
    """Stages run by each sub-sample: event processing only for the jackknife through stages, otherwise the stages
    with tasks up to the stage combined."""
    present = [s for s in api.STAGES if stages.get(s)]
    last = combined_stage(settings, api, stages)
    if settings.mode == JACKKNIFE_STAGES or settings.pthat():
        return [present[0]]
    return present[:present.index(last) + 1]


def jackknife_stages(settings, api, stages):
    """Stages run by the combination on each replica (jackknife through stages): those after event processing, up to
    the stage combined."""
    if settings.mode != JACKKNIFE_STAGES:
        return []
    present = [s for s in api.STAGES if stages.get(s)]
    last = combined_stage(settings, api, stages)
    return present[1:present.index(last) + 1]


def seeded_value(value, k):
    """Seed of sub-sample k from the seed of the workflow."""
    try:
        seed = int(value)
    except ValueError:
        return value
    if seed <= 0:
        seed = 1
    return str((seed - 1 + k) % MAX_SEED + 1)


def seeded_tasks(tasks, k):
    """Copies of the tasks (and subtasks) for sub-sample k: seeds increased by k, SetSeed switches on. Returns the
    tasks and the keys changed ([task: key])."""
    copies = copy.deepcopy(tasks)
    changed = []

    def visit(t):
        for p in t.params:
            if SEED_KEY.search(p.key):
                p.value = seeded_value(p.value, k)
                changed.append("%s:%s" % (t.name, p.key))
            elif SET_SEED_KEY.search(p.key) and p.type == "bool":
                p.value = "true"
        for c in t.children:
            visit(c)
    for t in copies:
        visit(t)
    return copies, changed


def job_of(settings, j):
    """Sub-sample and pT-hat bin of job j: (j // K, j % K), K the number of bins (1 without pT-hat bins)."""
    n = settings.nbins() if settings.pthat() else 1
    return j // n, j % n


def directory_name(settings, j):
    """Directory of job j: <prefix>sss, or <prefix>sss_pthat<kk> (sub-sample sss, pT-hat bin kk)."""
    if not settings.pthat():
        return "%s%03d" % (settings.prefix, j)
    s, k = job_of(settings, j)
    return "%s%03d_pthat%02d" % (settings.prefix, s, k)


def pthat_tasks(tasks, settings, j):
    """Set the pT-hat range of bin k of job j in the PYTHIA generators of tasks (copies), in place."""
    _, k = job_of(settings, j)
    edges = settings.edges()
    for g in pthat_generators(tasks):
        for key, value in ((PTHAT_MIN_KEY, edges[k]), (PTHAT_MAX_KEY, edges[k + 1])):
            p = g.param(key)
            if p is None:                       # templates of a build before the pT-hat bins
                if not g.params:
                    continue
                p = type(g.params[0])(key, "double", "-1")
                g.params.append(p)
            p.value = "%.17g" % value


def output_file(api, stages, library, templates, jobs, stage):
    """Output file of the job of a stage (its OUTPUT_FILE, or <task CAP runs>.cap)."""
    name = (jobs.get(stage) or {}).get("OUTPUT_FILE") or ""
    if not name:
        name = api.run_root_name(stages[stage], library, templates, stage) + ".cap"
    return name if name.endswith(".cap") else name[:-5] + ".cap" if name.endswith(".root") else name + ".cap"


class Production:
    """The files of a production of sub-samples, written in the output directory."""

    def __init__(self):
        self.directory = ""
        self.files = []           # workflow file of each sub-sample
        self.names = {}           # {stage: name of the task CAP runs}
        self.stages = []          # stages run by each sub-sample
        self.base = ""            # the workflow, without seeds (jackknife through stages)
        self.combine = ""         # CAP file of the combination
        self.result = ""          # output file of the combination
        self.sbatch = ""          # sbatch script (Slurm)
        self.seeds = []           # keys of the seeds changed
        self.outputs = []         # output files of the stages of a sub-sample


def write_production(directory, settings, api, stages, library, templates, jobs, subsample_files=True):
    """Write the workflow files of the sub-samples (<directory>/<prefix>k.xml), the workflow (RunCAP_workflow.xml),
    the configuration of the combination (RunCAP_combine.xml) and, for Slurm, the sbatch script (without
    subsample_files: only the workflow and the combination, to combine sub-samples run earlier). Returns a Production.
    Raises ValueError for invalid settings and OSError if a file cannot be written."""
    errors = check_settings(settings, api, stages)
    if errors:
        raise ValueError("\n".join(errors))
    os.makedirs(directory, exist_ok=True)
    production = Production()
    production.directory = os.path.abspath(directory)
    production.stages = subsample_stages(settings, api, stages)
    run = {s: stages[s] for s in production.stages}
    production.base = os.path.join(production.directory, BASE_FILE)
    production.names = api.write_stages(production.base, {s: stages[s] for s in api.STAGES if stages.get(s)}, library,
                                        templates, jobs, [settings.to_element()])
    production.outputs = [output_file(api, stages, library, templates, jobs, s) for s in production.stages]
    for k in range(settings.njobs() if subsample_files else 0):     # k: job (sub-sample, pT-hat bin)
        seeded, changed = {}, []
        for s in production.stages:
            seeded[s], more = seeded_tasks(run[s], k)
            if settings.pthat():
                pthat_tasks(seeded[s], settings, k)
            changed += more
        if k == 0:
            production.seeds = changed
        seeded_jobs = {s: dict(jobs.get(s) or {}) for s in api.STAGES}
        for s in production.stages:
            seeded_jobs[s]["SEED"] = str(settings.seed + k)
            if settings.pthat():
                seeded_jobs[s]["NORMALIZE"] = "false"        # raw histograms, added with sigma/N by the combination
        path = os.path.join(production.directory, directory_name(settings, k) + ".xml")
        production.names = api.write_stages(path, seeded, library, templates, seeded_jobs)
        production.files.append(path)
    production.combine = os.path.join(production.directory, COMBINE_FILE)
    production.result = write_combination(production.combine, production.directory, settings, api, stages, library,
                                          templates, jobs, production.base)
    if settings.where == SLURM and subsample_files:
        production.sbatch = os.path.join(production.directory, SBATCH_FILE)
        with open(production.sbatch, "w") as f:
            f.write(sbatch_script(settings, production))
    return production


def write_combination(path, directory, settings, api, stages, library, templates, jobs, workflow):
    """Write the configuration of the CAP job combining the sub-samples. Returns the name of its output file."""
    last = combined_stage(settings, api, stages)
    first = subsample_stages(settings, api, stages)[-1]       # stage whose outputs are read
    imported = output_file(api, stages, library, templates, jobs, first)
    stem = output_file(api, stages, library, templates, jobs, last)[:-4]
    result = "%s_%s.cap" % (stem, "SubSample" if settings.mode == SUBSAMPLE else "Jackknife")
    mode = "SUBSAMPLE" if settings.mode == SUBSAMPLE else "JACKKNIFE"
    replica = jackknife_stages(settings, api, stages)
    params = [("HISTOGRAM:IMPORT:PATH", directory), ("HISTOGRAM:IMPORT:FILE_NAME", imported),
              ("HISTOGRAM:EXPORT:PATH", directory), ("HISTOGRAM:EXPORT:FILE_NAME", result),
              ("MODE", mode), ("SUBSAMPLE:PATTERN", settings.prefix + "*"), ("WEIGHTS", settings.weights),
              ("JACKKNIFE:WORKFLOW", workflow if replica else "NONE"),
              ("JACKKNIFE:STAGES", " ".join(api.STAGE_ROOTS[s] for s in replica) if replica else "NONE"),
              ("JOB:OUTPUT_FILE", "NONE")]
    if settings.pthat():
        params[-1:-1] = [("PTHAT:BINS", str(settings.nbins())), ("PTHAT:NORMALIZATION", settings.normalization)]
    root = ET.Element("CAP", {"version": "1"})
    task = ET.SubElement(root, "TASK", {"name": COMBINE_TASK, "class": "CAP::SubSampleStatCalculator",
                                        "severity": "INFO"})
    groups = {}
    for key, value in params:
        parent, group = task, ""
        for g in key.split(":")[:-1]:
            group += g + ":"
            if group not in groups:
                groups[group] = ET.SubElement(parent, "GROUP", {"name": g})
            parent = groups[group]
        ET.SubElement(parent, "PARAMETER", {"name": key.split(":")[-1], "type": "string"}).text = value
    if hasattr(ET, "indent"):          # Python 3.9+
        ET.indent(root)
    with open(path, "wb") as f:
        f.write(b'<?xml version="1.0" encoding="UTF-8"?>\n')
        f.write(ET.tostring(root, encoding="utf-8"))
        f.write(b"\n")
    return result


def subsample_commands(production, settings, api, k):
    """Commands of sub-sample k (its stages, in order) run here: [(description, command, directory)]."""
    directory = os.path.join(production.directory, directory_name(settings, k))
    return [("%s %s" % (directory_name(settings, k), api.STAGE_TITLES[s]),
             api.run_command(production.files[k], production.names[s], directory), directory)
            for s in production.stages]


def combine_command(production, api):
    return ("Combining the sub-samples", api.run_command(production.combine, COMBINE_TASK, production.directory),
            production.directory)


def sbatch_script(settings, production):
    """sbatch script of a job array running the sub-samples on the cluster (in settings.slurm.directory)."""
    slurm = settings.slurm
    remote = slurm.directory.rstrip("/")
    cap = slurm.cap.rstrip("/")
    name = "CAP_" + os.path.basename(production.directory.rstrip("/"))
    lines = ["#!/bin/bash",
             "# Sub-samples of a CAP workflow, written by RunCAP.py: one task of the array per sub-sample.",
             "#SBATCH --job-name=%s" % name,
             "#SBATCH --array=0-%d" % (settings.njobs() - 1),
             "#SBATCH --output=%s/logs/%s%%03a.log" % (remote, settings.prefix),
             "#SBATCH --ntasks=1",
             "#SBATCH --cpus-per-task=1"]
    for option, value in (("partition", slurm.partition), ("time", slurm.time), ("mem", slurm.memory),
                          ("account", slurm.account)):
        if value.strip():
            lines.append("#SBATCH --%s=%s" % (option, value.strip()))
    lines += ["", "set -e"]
    lines += [l for l in slurm.setup.splitlines() if l.strip()]
    lines += ["export CAP_ROOT_PATH=%s" % shlex.quote(cap),
              "export CAP_DATABASE_PATH=$CAP_ROOT_PATH/DB/",
              "export CAP_PROJECTS_PATH=$CAP_ROOT_PATH/projects/",
              "export LD_LIBRARY_PATH=$CAP_ROOT_PATH/lib:${LD_LIBRARY_PATH:-}",
              "export DYLD_LIBRARY_PATH=$CAP_ROOT_PATH/lib:${DYLD_LIBRARY_PATH:-}",
              ("K=$(printf '%%03d_pthat%%02d' $((SLURM_ARRAY_TASK_ID / %d)) $((SLURM_ARRAY_TASK_ID %% %d)))"
               % (settings.nbins(), settings.nbins())) if settings.pthat() else
              "K=$(printf '%%03d' $SLURM_ARRAY_TASK_ID)",
              "WORK=%s" % shlex.quote(remote),
              "OUT=$WORK/%s$K" % settings.prefix,
              "mkdir -p $OUT",
              "cd $OUT"]
    for s in production.stages:
        lines.append("$CAP_ROOT_PATH/bin/CAP %s $WORK/ %s$K.xml $OUT/" % (production.names[s], settings.prefix))
    lines.append("echo \"Job $K done\"")
    return "\n".join(lines) + "\n"


def remote_paths(production, settings):
    """Directory of the production on the cluster (the workflow files of the sub-samples only name files of their own
    directory, so they are copied as they are)."""
    return settings.slurm.directory.rstrip("/")


def ssh(host, command):
    return ["ssh", "-o", "BatchMode=yes", host, command]


def submit_commands(production, settings):
    """Copy the production to the cluster and submit the job array."""
    host, remote = settings.slurm.host.strip(), remote_paths(production, settings)
    local = production.directory.rstrip("/") + "/"
    return [("Creating %s on %s" % (remote, host), ssh(host, "mkdir -p %s/logs" % shlex.quote(remote)), local),
            ("Copying the workflow files", ["rsync", "-a", "--include=*.xml", "--include=*.sbatch", "--exclude=*",
                                            local, "%s:%s/" % (host, remote)], local),
            ("Submitting the job array", ssh(host, "cd %s && sbatch %s" % (shlex.quote(remote), SBATCH_FILE)), local)]


def status_commands(production, settings):
    host, remote = settings.slurm.host.strip(), remote_paths(production, settings)
    name = "CAP_" + os.path.basename(production.directory.rstrip("/"))
    files = " ".join("%s/%s*/%s" % (shlex.quote(remote), settings.prefix, f) for f in production.outputs[-1:])
    command = ("squeue --me --name=%s; echo; echo \"Sub-samples done: $(ls %s 2>/dev/null | wc -l) of %d\""
               % (name, files, settings.njobs()))
    return [("Status of the job array", ssh(host, command), production.directory)]


def fetch_commands(production, settings):
    """Copy the output files and the logs of the sub-samples back from the cluster."""
    host, remote = settings.slurm.host.strip(), remote_paths(production, settings)
    local = production.directory.rstrip("/") + "/"
    return [("Copying the outputs of the sub-samples", ["rsync", "-a", "-m", "--include=%s*/" % settings.prefix,
                                                         "--include=logs/", "--include=*.cap", "--include=*.log",
                                                         "--exclude=*", "%s:%s/" % (host, remote), local], local)]


class ParallelRunner:
    """Runs sequences of commands, several sequences at once (each sequence in order, stopping at its first failure),
    then the final commands. Lines of output go to put(("line", text)), with the name of the sequence."""

    def __init__(self, sequences, parallel, env, put, final=()):
        self.sequences = list(sequences)        # [(label, [(description, command, directory)])]
        self.parallel = max(1, parallel)
        self.env = env
        self.put = put
        self.final = list(final)
        self.processes = set()
        self.lock = threading.Lock()
        self.killed = False
        self.failed = []

    def kill(self, sig):
        self.killed = True
        with self.lock:
            processes = list(self.processes)
        for p in processes:
            try:
                os.killpg(p.pid, sig)
            except (OSError, AttributeError):
                try:
                    p.kill()
                except OSError:
                    pass

    def _run(self, label, description, cmd, cwd):
        os.makedirs(cwd, exist_ok=True)
        self.put(("text", "$ %s\n" % " ".join(shlex.quote(c) for c in cmd), "command"))
        try:
            p = subprocess.Popen(cmd, cwd=cwd, env=self.env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                 text=True, bufsize=1, errors="replace", start_new_session=True)
        except OSError as e:
            self.put(("text", "%s: cannot run %s: %s\n" % (label, cmd[0], e), "failure"))
            return False
        with self.lock:
            self.processes.add(p)
        if self.killed:                         # killed while the process was starting
            try:
                os.killpg(p.pid, signal.SIGKILL)
            except (OSError, AttributeError):
                p.kill()
        prefix = "[%s] " % label if label else ""
        for line in p.stdout:
            self.put(("line", prefix + line))
        code = p.wait()
        with self.lock:
            self.processes.discard(p)
        if code != 0 and not self.killed:
            self.put(("text", "%s%s failed (exit code %d)\n" % (prefix, description, code), "failure"))
        return code == 0

    def run(self):
        """Run everything. Returns (ok, message)."""
        pending = list(self.sequences)
        done = [0]

        def worker():
            while not self.killed:
                with self.lock:
                    if not pending:
                        return
                    label, commands = pending.pop(0)
                ok = True
                for description, cmd, cwd in commands:
                    if self.killed or not self._run(label, description, cmd, cwd):
                        ok = False
                        break
                with self.lock:
                    done[0] += 1
                    if not ok and not self.killed:
                        self.failed.append(label)
                    self.put(("status", "Sub-samples: %d of %d done%s" % (
                        done[0], len(self.sequences), (", %d failed" % len(self.failed)) if self.failed else "")))

        threads = [threading.Thread(target=worker, daemon=True) for _ in range(min(self.parallel, len(pending)) or 1)]
        for t in threads:
            t.start()
        for t in threads:
            t.join()
        if self.killed:
            return False, "killed by the user"
        if self.failed:
            return False, "%d sub-sample(s) failed: %s" % (len(self.failed), ", ".join(sorted(self.failed)))
        for description, cmd, cwd in self.final:
            self.put(("status", description + "…"))
            if not self._run("", description, cmd, cwd):
                return False, "killed by the user" if self.killed else description.lower() + " failed"
        return True, "sub-samples completed" + (" and combined" if self.final else "")


# ----------------------------------------------------------------------------------------------------------------
# GUI
# ----------------------------------------------------------------------------------------------------------------

def make_bar(parent, app, settings, api, ToolTip):
    """The row of the sub-samples in the RunCAP window: switch, count, jobs at once, mode, stage combined, where, and
    the buttons (Slurm settings, Submit, Status, Fetch, Combine). app provides subsamples_changed(),
    run_subsamples(), combine_subsamples(), slurm_action(kind) and present_stages()."""
    import tkinter as tk
    from tkinter import ttk

    bar = ttk.Frame(parent)
    vars_ = {}
    showing = [False]

    def changed(*_):
        if showing[0]:
            return
        try:
            settings.count = int(vars_["count"].get())
        except ValueError:
            pass
        try:
            settings.parallel = int(vars_["parallel"].get())
        except ValueError:
            pass
        settings.enabled = bool(vars_["enabled"].get())
        settings.mode = {v: k for k, v in MODE_TITLES.items()}.get(vars_["mode"].get(), SUBSAMPLE)
        title = vars_["stage"].get()
        settings.stage = next((s for s in api.STAGES if api.STAGE_TITLES[s] == title), "")
        settings.where = SLURM if vars_["where"].get() == "Slurm" else LOCAL
        settings.ptHatEdges = vars_["pthat"].get().strip()
        settings.normalization = {v: k for k, v in NORMALIZATION_TITLES.items()}.get(vars_["normalization"].get(), SIGMA)
        update()
        app.subsamples_changed()

    def update():
        slurm = settings.where == SLURM
        for b in (bar.submit, bar.status, bar.fetch):
            b.configure(state=tk.NORMAL if slurm else tk.DISABLED)
        bar.parallel_entry.configure(state=tk.DISABLED if slurm else tk.NORMAL)
        bar.normalization_box.configure(state="readonly" if settings.pthat() else tk.DISABLED)
        present = app.present_stages()
        bar.stage_box.configure(values=["Last stage"] + [api.STAGE_TITLES[s] for s in present])

    vars_["enabled"] = tk.BooleanVar(bar, value=settings.enabled)
    b = ttk.Checkbutton(bar, text="Sub-samples:", variable=vars_["enabled"], command=changed)
    b.pack(side=tk.LEFT)
    ToolTip(b, "Run all stages runs the workflow as independent sub-samples (each with its own seeds, in "
               "<output>/%s000, ...), then combines them (SubSampleStatCalculator)" % settings.prefix)
    vars_["count"] = tk.StringVar(bar, value=str(settings.count))
    e = ttk.Entry(bar, textvariable=vars_["count"], width=5)
    e.pack(side=tk.LEFT, padx=(2, 8))
    ToolTip(e, "Number of sub-samples (each runs the number of events of the workflow)")
    ttk.Label(bar, text="at once:").pack(side=tk.LEFT)
    vars_["parallel"] = tk.StringVar(bar, value=str(settings.parallel))
    bar.parallel_entry = ttk.Entry(bar, textvariable=vars_["parallel"], width=4)
    bar.parallel_entry.pack(side=tk.LEFT, padx=(2, 8))
    ToolTip(bar.parallel_entry, "Sub-samples run at the same time on this computer (default: number of cores)")
    vars_["mode"] = tk.StringVar(bar, value=MODE_TITLES[settings.mode])
    box = ttk.Combobox(bar, textvariable=vars_["mode"], values=[MODE_TITLES[m] for m in MODES], state="readonly",
                       width=22)
    box.pack(side=tk.LEFT, padx=(0, 8))
    box.bind("<<ComboboxSelected>>", changed)
    ToolTip(box, "Sub-sample average: mean of the sub-samples with the uncertainty of the mean. Jackknife on results: "
                 "delete-one jackknife of their outputs. Jackknife through stages: the sub-samples run event processing "
                 "only; the later stages are run on the whole sample and on each replica (nonlinear observables). The "
                 "jackknife also writes the bias-corrected values (_biasCorrected)")
    ttk.Label(bar, text="combine:").pack(side=tk.LEFT)
    vars_["stage"] = tk.StringVar(bar, value=api.STAGE_TITLES.get(settings.stage, "Last stage"))
    bar.stage_box = ttk.Combobox(bar, textvariable=vars_["stage"], state="readonly", width=16)
    bar.stage_box.pack(side=tk.LEFT, padx=(2, 8))
    bar.stage_box.bind("<<ComboboxSelected>>", changed)
    ToolTip(bar.stage_box, "Stage whose output is combined")
    ttk.Label(bar, text="pT-hat bins:").pack(side=tk.LEFT)
    vars_["pthat"] = tk.StringVar(bar, value=settings.ptHatEdges)
    e = ttk.Entry(bar, textvariable=vars_["pthat"], width=18)
    e.pack(side=tk.LEFT, padx=(2, 2))
    ToolTip(e, "Edges of the pT-hat bins of PYTHIA (GeV/c, e.g. 5 10 20 40 80 160; empty: none). Each sub-sample is then "
               "one job per bin (%s000_pthat00, ...), the same number of events in each, with PythiaEventGenerator "
               "PTHAT_MIN/PTHAT_MAX set to the edges of the bin (a hard process: HardQCD:all = on). The combination "
               "adds the bins with the weights sigma/N of each bin" % settings.prefix)
    vars_["normalization"] = tk.StringVar(bar, value=NORMALIZATION_TITLES[settings.normalization])
    bar.normalization_box = ttk.Combobox(bar, textvariable=vars_["normalization"], state="readonly", width=9,
                                         values=[NORMALIZATION_TITLES[n] for n in (SIGMA, COVERED)])
    bar.normalization_box.pack(side=tk.LEFT, padx=(0, 8))
    bar.normalization_box.bind("<<ComboboxSelected>>", changed)
    ToolTip(bar.normalization_box, "pT-hat bins: the differential cross sections in mb, or per event of the pT-hat "
                                   "range covered (divided by the sum of the cross sections of the bins; for later "
                                   "stages computing ratios or correlation functions)")
    vars_["where"] = tk.StringVar(bar, value="Slurm" if settings.where == SLURM else "Local")
    for text in ("Local", "Slurm"):
        r = ttk.Radiobutton(bar, text=text, value=text, variable=vars_["where"], command=changed)
        r.pack(side=tk.LEFT)
    s = ttk.Button(bar, text="Slurm…", width=7, command=lambda: SlurmDialog(bar, settings, app.subsamples_changed))
    s.pack(side=tk.LEFT, padx=(4, 0))
    ToolTip(s, "Cluster, directories, partition, time, memory and setup of the jobs on Slurm")
    for name, text, kind, tip in (("submit", "Submit", "submit", "Write the production in the output directory, copy "
                                   "it to the cluster and submit the job array (ssh with keys, no password)"),
                                  ("status", "Status", "status", "Jobs of the array in the queue, sub-samples done"),
                                  ("fetch", "Fetch", "fetch", "Copy the outputs of the sub-samples back to the "
                                   "output directory")):
        b = ttk.Button(bar, text=text, width=7, command=lambda k=kind: app.slurm_action(k))
        b.pack(side=tk.LEFT, padx=(4, 0))
        ToolTip(b, tip)
        setattr(bar, name, b)
    c = ttk.Button(bar, text="Combine", width=8, command=app.combine_subsamples)
    c.pack(side=tk.LEFT, padx=(8, 0))
    ToolTip(c, "Combine the sub-samples of the output directory (%s*) as set here, without running them" %
            settings.prefix)
    for name in ("count", "parallel", "pthat"):
        vars_[name].trace_add("write", changed)
    bar.vars = vars_

    def show(new):
        """Show other settings (a workflow loaded)."""
        nonlocal settings
        settings = new
        showing[0] = True
        vars_["enabled"].set(new.enabled)
        vars_["count"].set(str(new.count))
        vars_["parallel"].set(str(new.parallel))
        vars_["mode"].set(MODE_TITLES[new.mode])
        vars_["stage"].set(api.STAGE_TITLES.get(new.stage, "Last stage"))
        vars_["where"].set("Slurm" if new.where == SLURM else "Local")
        vars_["pthat"].set(new.ptHatEdges)
        vars_["normalization"].set(NORMALIZATION_TITLES[new.normalization])
        showing[0] = False
        update()
        app.subsamples_changed(dirty=False)
    bar.show = show
    bar.update_stages = update
    update()
    return bar


def SlurmDialog(parent, settings, changed):
    """Window of the Slurm settings."""
    import tkinter as tk
    from tkinter import ttk
    window = tk.Toplevel(parent)
    window.title("Sub-samples on Slurm")
    window.transient(parent.winfo_toplevel())
    frame = ttk.Frame(window, padding=10)
    frame.pack(fill=tk.BOTH, expand=True)
    rows = (("host", "Login host (user@host):", "ssh must log in without a password (ssh keys)"),
            ("cap", "CAP directory on the cluster:", "Holds bin/CAP, lib and DB (CAP built on the cluster)"),
            ("directory", "Production directory on the cluster:", "Where the workflow files, the outputs of the "
                                                                  "sub-samples and the logs go"),
            ("partition", "Partition:", ""), ("time", "Time limit:", "e.g. 04:00:00"),
            ("memory", "Memory:", "e.g. 4G"), ("account", "Account:", "optional"))
    entries = {}
    for r, (name, text, tip) in enumerate(rows):
        ttk.Label(frame, text=text).grid(row=r, column=0, sticky="w", pady=2)
        var = tk.StringVar(window, value=getattr(settings.slurm, name))
        ttk.Entry(frame, textvariable=var, width=48).grid(row=r, column=1, sticky="ew", pady=2)
        if tip:
            ttk.Label(frame, text=tip, foreground="gray40").grid(row=r, column=2, sticky="w", padx=6)
        entries[name] = var
    ttk.Label(frame, text="Setup (run before CAP):").grid(row=len(rows), column=0, sticky="nw", pady=2)
    setup = tk.Text(frame, width=48, height=5)
    setup.insert("1.0", settings.slurm.setup)
    setup.grid(row=len(rows), column=1, sticky="ew", pady=2)
    ttk.Label(frame, text="e.g. module load root pythia gsl", foreground="gray40").grid(row=len(rows), column=2,
                                                                                        sticky="nw", padx=6)
    frame.columnconfigure(1, weight=1)

    def ok():
        for name, var in entries.items():
            setattr(settings.slurm, name, var.get().strip())
        settings.slurm.setup = setup.get("1.0", tk.END).strip()
        changed()
        window.destroy()
    buttons = ttk.Frame(frame)
    buttons.grid(row=len(rows) + 1, column=0, columnspan=3, sticky="e", pady=(8, 0))
    ttk.Button(buttons, text="Cancel", command=window.destroy).pack(side=tk.RIGHT)
    ttk.Button(buttons, text="OK", command=ok).pack(side=tk.RIGHT, padx=4)
    return window
