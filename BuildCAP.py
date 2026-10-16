#!/usr/bin/env python3
# **********************************************************************
# Copyright (C) 2019-2024, Claude Pruneau
# All rights reserved.
#
# For the licensing terms see LICENSE.
# **********************************************************************
"""
BuildCAP.py -- graphical front end to configure and build CAP.

Usage (from anywhere; the script locates CAP from its own position):
    python3 BuildCAP.py                  # open the GUI
    python3 BuildCAP.py --print-commands # print the commands for the saved settings and exit (no GUI)

The window has three parts:
  * a toolbar (top): Build, Stop, Save output, Clear, the build directory name, the build type, the number of parallel
    jobs and whether to install;
  * the options (upper half): the core modules (always built), the optional modules (Therminator3, Latex, Jets, Epos,
    PHSD, Pythia) and the Herwig generator, with the locations of ROOT and of the external packages;
  * the output of cmake and make (lower half), which can be saved to a file.

Build steps (each command is printed in the output pane before it runs):
    cmake -S <CAP>/src -B <CAP>/<build directory> -DCAP_WITH_<MODULE>=ON|OFF ... [-DCAP_PYTHIA8_DIR=...]
    make -C <CAP>/<build directory> -j<N>
    make -C <CAP>/<build directory> install        (if "Install" is checked)

The build directory is created under the CAP directory; several build directories (e.g. build, build-pythia) can
coexist. The libraries are installed in the "Install libraries to" directory ($CAP_LIB_PATH, <CAP>/lib by default) and
the CAP executable in <CAP>/bin. The environment of SetupCAP.sh is set for the commands, so the GUI does not need to be
started from a shell where SetupCAP.sh was sourced.

External packages (Pythia8, Herwig, FastJet): when the location field is empty, CMake uses a copy distributed with CAP
(<CAP>/pythia8*, <CAP>/Herwig*, <CAP>/fastjet*, or the same names under <CAP>/external), then the environment
(CAP_PYTHIA8_PATH or PYTHIA8, CAP_HERWIG_PATH or HERWIG_DIR, CAP_FASTJET_PATH or FASTJET), then the package's
configuration tool (pythia8-config, Herwig-config, fastjet-config). The Detect button shows what would be used.

The settings are saved in <CAP>/.BuildCAP.json. Requires Python 3.7+ with tkinter (on macOS with Homebrew Python:
brew install python-tk).
"""

import datetime
import glob
import json
import os
import queue
import re
import shutil
import signal
import subprocess
import sys
import threading
import time

CAP_DIR = os.path.dirname(os.path.abspath(__file__))
SRC_DIR = os.path.join(CAP_DIR, "src")
SETTINGS_FILE = os.path.join(CAP_DIR, ".BuildCAP.json")

#: Modules always built (see src/CMakeLists.txt).
CORE_MODULES = ["Base", "Math", "Helpers", "Particles", "Global", "ParticleSingle", "ParticlePair", "ParticlePair3D",
                "PtPt", "NuDyn", "Spherocity", "SubSample", "Performance", "BasicEventGen", "Glauber"]

#: Optional modules: CMake option, folder in src, description, default.
OPTIONAL_MODULES = [
    ("CAP_WITH_THERMINATOR3", "Therminator3", "Thermal model event generator", True),
    ("CAP_WITH_LATEX",        "Latex",        "LaTeX documents of results", True),
    ("CAP_WITH_JETS",         "Jets",         "Jet analysis (needs FastJet; in progress)", False),
    ("CAP_WITH_EPOS",         "Epos",         "EPOS event reader", False),
    ("CAP_WITH_PHSD",         "PHSD",         "PHSD event reader (in progress)", False),
    ("CAP_WITH_PYTHIA8",      "Pythia",       "Pythia8 event generator interface (needs Pythia8)", False),
]

#: External packages: key, label, CMake prefix variable, file identifying a valid prefix, bundled globs (relative to
#: the CAP directory), environment variables, configuration tool.
EXTERNALS = {
    "PYTHIA8": dict(label="Pythia8", var="CAP_PYTHIA8_DIR", file="include/Pythia8/Pythia.h",
                    bundled=["pythia8*", "external/pythia8*"], env=["CAP_PYTHIA8_PATH", "PYTHIA8"],
                    tool="pythia8-config"),
    "HERWIG":  dict(label="Herwig", var="CAP_HERWIG_DIR", file="include/Herwig/API/HerwigAPI.h",
                    bundled=["Herwig*", "herwig*", "external/Herwig*", "external/herwig*"],
                    env=["CAP_HERWIG_PATH", "HERWIG_DIR"], tool="Herwig-config"),
    "FASTJET": dict(label="FastJet", var="CAP_FASTJET_DIR", file="include/fastjet/ClusterSequence.hh",
                    bundled=["fastjet*", "external/fastjet*"], env=["CAP_FASTJET_PATH", "FASTJET"],
                    tool="fastjet-config"),
}

#: Event generators: CMake option, external package key, description.
GENERATORS = [
    ("CAP_WITH_HERWIG",  "HERWIG",  "Herwig generator (CAP interface not written yet)"),
]

BUILD_TYPES = ["Release", "RelWithDebInfo", "Debug", "MinSizeRel"]

#: Names that cannot be used as build directories (existing CAP folders).
RESERVED_NAMES = {"src", "lib", "bin", "DB", "doc", "projects", "publications", "Grid", "external"}

#: Extra directories searched for cmake and root-config (a GUI started from the macOS Finder has a minimal PATH).
EXTRA_PATHS = ["/opt/homebrew/bin", "/usr/local/bin", "/Applications/CMake.app/Contents/bin", "/opt/local/bin"]


# ------------------------------------------------------------------------------------------------------------------
# Settings and commands (no GUI: usable and testable without tkinter)
# ------------------------------------------------------------------------------------------------------------------

def default_settings():
    s = {
        "build_dir": "build",
        "build_type": "Release",
        "jobs": max(1, (os.cpu_count() or 2)),
        "install": True,
        "install_lib_dir": os.path.join(CAP_DIR, "lib"),
        "reconfigure": False,
        "root_dir": "",
        "options": {opt: default for opt, _, _, default in OPTIONAL_MODULES},
        "prefixes": {key: "" for key in EXTERNALS},
    }
    for opt, _, _ in GENERATORS:
        s["options"][opt] = False
    return s


def load_settings():
    s = default_settings()
    try:
        with open(SETTINGS_FILE) as f:
            saved = json.load(f)
        for k, v in saved.items():
            if isinstance(v, dict) and isinstance(s.get(k), dict):
                s[k].update(v)
            else:
                s[k] = v
    except (OSError, ValueError):
        pass
    return s


def save_settings(s):
    try:
        with open(SETTINGS_FILE, "w") as f:
            json.dump(s, f, indent=2)
    except OSError:
        pass


def module_dir(folder):
    """Directory of a module: src/<folder>, or src/<group>/<folder> for the module groups (core, models, analysis, tools)."""
    for group in ("", "core", "models", "analysis", "tools"):
        path = os.path.join(SRC_DIR, group, folder)
        if os.path.isdir(path):
            return path
    return None


def search_path():
    return os.environ.get("PATH", "").split(os.pathsep) + EXTRA_PATHS


def which(program, extra_dirs=()):
    """Path of a program in PATH, the given directories or the usual install directories, or None."""
    return shutil.which(program, path=os.pathsep.join(list(extra_dirs) + search_path()))


def run_quiet(cmd):
    try:
        return subprocess.run(cmd, capture_output=True, text=True, timeout=20).stdout.strip()
    except (OSError, subprocess.SubprocessError):
        return ""


def detect_root(root_dir=""):
    """ROOT installation prefix: the given one, $ROOTSYS, or root-config --prefix. Returns (prefix, how)."""
    if root_dir:
        return root_dir, "set"
    if os.environ.get("ROOTSYS"):
        return os.environ["ROOTSYS"], "$ROOTSYS"
    tool = which("root-config")
    if tool:
        prefix = run_quiet([tool, "--prefix"])
        if prefix:
            return prefix, "root-config"
    return "", "not found"


def detect_external(key, prefix=""):
    """Prefix that CMake would use for an external package (same order as cap_resolve_prefix in src/CMakeLists.txt).

    Returns (prefix, message); prefix is "" if not found.
    """
    ext = EXTERNALS[key]
    if prefix:
        if os.path.exists(os.path.join(prefix, ext["file"])):
            return prefix, "location field"
        return "", "%s does not contain %s" % (prefix, ext["file"])
    candidates = []
    for pattern in ext["bundled"]:
        candidates += [(p, "distributed with CAP") for p in sorted(glob.glob(os.path.join(CAP_DIR, pattern)))
                       if os.path.isdir(p)]
    for env in ext["env"]:
        if os.environ.get(env):
            candidates.append((os.environ[env], "$" + env))
    tool = which(ext["tool"])
    if tool:
        p = run_quiet([tool, "--prefix"])
        if p:
            candidates.append((p, ext["tool"]))
    unbuilt = []
    for path, how in candidates:
        path = path.rstrip("/")
        if os.path.exists(os.path.join(path, ext["file"])):
            return path, how
        unbuilt.append(path)
    if unbuilt:
        return "", "not usable (not built or installed?): " + ", ".join(unbuilt)
    return "", "not found"


def valid_build_dir_name(name):
    """Error message for an invalid build directory name, or None."""
    if not name:
        return "Enter a build directory name."
    if not re.fullmatch(r"[A-Za-z0-9._+-]+", name) or name in (".", ".."):
        return "Use letters, digits, '.', '_', '+' or '-' only (one directory directly under CAP)."
    if name in RESERVED_NAMES:
        return "'%s' is a CAP folder; choose another name." % name
    path = os.path.join(CAP_DIR, name)
    if os.path.exists(path) and not os.path.isdir(path):
        return "'%s' exists and is not a directory." % name
    if os.path.isdir(path) and os.listdir(path) and not os.path.exists(os.path.join(path, "CMakeCache.txt")):
        return "'%s' exists, is not empty, and is not a CMake build directory." % name
    return None


def existing_build_dirs():
    return sorted(d for d in os.listdir(CAP_DIR)
                  if os.path.isfile(os.path.join(CAP_DIR, d, "CMakeCache.txt")))


def read_cmake_cache(build_dir):
    """CAP options and prefixes stored in the CMake cache of a build directory: {name: value}."""
    values = {}
    try:
        with open(os.path.join(CAP_DIR, build_dir, "CMakeCache.txt")) as f:
            for line in f:
                m = re.match(r"(CAP_WITH_\w+|CAP_\w+_DIR|CMAKE_BUILD_TYPE):\w+=(.*)$", line.strip())
                if m:
                    values[m.group(1)] = m.group(2)
    except OSError:
        pass
    return values


def build_environment(s):
    """Environment of the build commands: that of SetupCAP.sh, ROOT, and the install directories."""
    env = dict(os.environ)
    lib = s["install_lib_dir"].rstrip("/") + "/"
    env.update({
        "CAP_ROOT_PATH": CAP_DIR,
        "CAP_SRC_PATH": SRC_DIR + "/",
        "CAP_BIN_PATH": os.path.join(CAP_DIR, "bin") + "/",
        "CAP_LIB_PATH": lib,
        "CAP_MACROS_PATH": os.path.join(SRC_DIR, "Macros") + "/",
        "CAP_PROJECTS_PATH": os.path.join(CAP_DIR, "projects") + "/",
        "CAP_DATABASE_PATH": os.path.join(CAP_DIR, "DB") + "/",
    })
    path = env.get("PATH", "").split(os.pathsep)
    root, _ = detect_root(s.get("root_dir", ""))
    if root:
        env["ROOTSYS"] = root
        path.insert(0, os.path.join(root, "bin"))
        prefix_path = env.get("CMAKE_PREFIX_PATH", "")
        env["CMAKE_PREFIX_PATH"] = root + (os.pathsep + prefix_path if prefix_path else "")
    path += [p for p in EXTRA_PATHS if p not in path]
    env["PATH"] = os.pathsep.join(path)
    for var in ("DYLD_LIBRARY_PATH", "LD_LIBRARY_PATH"):
        env[var] = lib + (os.pathsep + env[var] if env.get(var) else "")
    return env


def build_commands(s):
    """List of (description, command) of a build with settings s (see the module documentation)."""
    build_dir = os.path.join(CAP_DIR, s["build_dir"])
    cmake = which("cmake") or "cmake"
    make = which("make") or "make"
    configure = [cmake, "-S", SRC_DIR, "-B", build_dir]
    if not os.path.exists(os.path.join(build_dir, "CMakeCache.txt")) or s.get("reconfigure"):
        configure += ["-G", "Unix Makefiles"]          # the generator can only be chosen for a new cache
    configure.append("-DCMAKE_BUILD_TYPE=%s" % s["build_type"])
    for opt, value in sorted(s["options"].items()):
        configure.append("-D%s=%s" % (opt, "ON" if value else "OFF"))
    for key, ext in EXTERNALS.items():
        # an empty value lets CMake search again (distributed copy, environment, configuration tool)
        configure.append("-D%s=%s" % (ext["var"], s["prefixes"].get(key, "")))
    commands = [("Configuring with CMake", configure),
                ("Building", [make, "-C", build_dir, "-j%d" % int(s["jobs"])])]
    if s.get("install"):
        commands.append(("Installing", [make, "-C", build_dir, "install"]))
    return commands


def shell_text(cmd):
    return " ".join(c if re.fullmatch(r"[\w@%+=:,./-]+", c) else "'" + c.replace("'", "'\\''") + "'" for c in cmd)


# ------------------------------------------------------------------------------------------------------------------
# GUI
# ------------------------------------------------------------------------------------------------------------------

def run_gui():
    import tkinter as tk
    from tkinter import ttk, filedialog, messagebox
    from tkinter.scrolledtext import ScrolledText

    class BuildCAPApp:
        def __init__(self, root):
            self.root = root
            self.settings = load_settings()
            self.process = None
            self.worker = None
            self.stop_requested = False
            self.queue = queue.Queue()
            self.start_time = None
            root.title("CAP Build")
            root.geometry("1180x860")
            root.minsize(900, 600)
            style = ttk.Style()
            if "clam" in style.theme_names() and sys.platform != "darwin":
                style.theme_use("clam")
            style.configure("Build.TButton", font=("TkDefaultFont", 12, "bold"))
            self._make_toolbar()
            panes = ttk.PanedWindow(root, orient=tk.VERTICAL)
            panes.pack(fill=tk.BOTH, expand=True, padx=6, pady=(0, 6))
            options = ttk.Frame(panes)
            output = ttk.Frame(panes)
            panes.add(options, weight=1)
            panes.add(output, weight=1)
            self._make_options(options)
            self._make_output(output)
            self.status = tk.StringVar(value="Ready")
            ttk.Label(root, textvariable=self.status, anchor="w", relief=tk.SUNKEN).pack(fill=tk.X, side=tk.BOTTOM)
            self._on_build_dir_changed()
            self._detect_all()
            root.protocol("WM_DELETE_WINDOW", self._on_close)
            root.after(50, self._poll_queue)

        # ---------------------------------------------------------------------------------------------- toolbar
        def _make_toolbar(self):
            s = self.settings
            bar = ttk.Frame(self.root, padding=(6, 6, 6, 2))
            bar.pack(fill=tk.X)
            self.build_button = ttk.Button(bar, text="\u25B6 Build", style="Build.TButton", command=self.start_build)
            self.build_button.pack(side=tk.LEFT)
            self.stop_button = ttk.Button(bar, text="\u25A0 Stop", command=self.stop_build, state=tk.DISABLED)
            self.stop_button.pack(side=tk.LEFT, padx=(4, 0))
            ttk.Button(bar, text="Save output\u2026", command=self.save_output).pack(side=tk.LEFT, padx=(4, 0))
            ttk.Button(bar, text="Clear", command=self.clear_output).pack(side=tk.LEFT, padx=(4, 12))
            ttk.Label(bar, text="CAP: " + CAP_DIR, foreground="gray40").pack(side=tk.RIGHT)
            bar = ttk.Frame(self.root, padding=(6, 2, 6, 4))     # second row: build settings
            bar.pack(fill=tk.X)
            ttk.Label(bar, text="Build directory:").pack(side=tk.LEFT)
            self.build_dir = tk.StringVar(value=s["build_dir"])
            self.build_dir_box = ttk.Combobox(bar, textvariable=self.build_dir, width=16,
                                              values=existing_build_dirs(), postcommand=self._refresh_build_dirs)
            self.build_dir_box.pack(side=tk.LEFT, padx=(4, 0))
            self.build_dir_box.bind("<<ComboboxSelected>>", lambda e: self._on_build_dir_changed())
            self.build_dir_box.bind("<FocusOut>", lambda e: self._on_build_dir_changed())
            self.build_dir_box.bind("<Return>", lambda e: self._on_build_dir_changed())
            self.build_dir_note = ttk.Label(bar, text="", foreground="gray40")
            self.build_dir_note.pack(side=tk.LEFT, padx=(4, 12))
            ttk.Label(bar, text="Type:").pack(side=tk.LEFT)
            self.build_type = tk.StringVar(value=s["build_type"])
            ttk.Combobox(bar, textvariable=self.build_type, values=BUILD_TYPES, width=13,
                         state="readonly").pack(side=tk.LEFT, padx=(4, 12))
            ttk.Label(bar, text="Jobs:").pack(side=tk.LEFT)
            self.jobs = tk.IntVar(value=int(s["jobs"]))
            ttk.Spinbox(bar, from_=1, to=128, textvariable=self.jobs, width=4).pack(side=tk.LEFT, padx=(4, 12))
            self.install = tk.BooleanVar(value=s["install"])
            ttk.Checkbutton(bar, text="Install", variable=self.install).pack(side=tk.LEFT)
            self.reconfigure = tk.BooleanVar(value=False)
            ttk.Checkbutton(bar, text="Fresh configure", variable=self.reconfigure).pack(side=tk.LEFT, padx=(8, 0))

        # ---------------------------------------------------------------------------------------------- options
        def _make_options(self, parent):
            s = self.settings
            parent.columnconfigure(0, weight=0)
            parent.columnconfigure(1, weight=1)
            parent.columnconfigure(2, weight=2)
            parent.rowconfigure(0, weight=1)

            core = ttk.LabelFrame(parent, text="Core modules (always built)", padding=8)
            core.grid(row=0, column=0, sticky="nsew", padx=(0, 6), pady=6)
            self.core_vars = []
            for i, name in enumerate(CORE_MODULES):
                var = tk.BooleanVar(value=True)
                self.core_vars.append(var)
                ttk.Checkbutton(core, text=name, variable=var, state=tk.DISABLED).grid(
                    row=i % 8, column=i // 8, sticky="w", padx=(0, 10))

            optional = ttk.LabelFrame(parent, text="Optional modules", padding=8)
            optional.grid(row=0, column=1, sticky="nsew", padx=6, pady=6)
            self.option_vars = {}
            for i, (opt, folder, desc, _) in enumerate(OPTIONAL_MODULES):
                var = tk.BooleanVar(value=bool(s["options"].get(opt)))
                self.option_vars[opt] = var
                ttk.Checkbutton(optional, text=folder, variable=var,
                                command=self._update_dependent_rows).grid(row=i, column=0, sticky="w")
                ttk.Label(optional, text=desc, foreground="gray35").grid(row=i, column=1, sticky="w", padx=(6, 0))
                if not module_dir(folder):
                    ttk.Label(optional, text="(missing from src)", foreground="red").grid(row=i, column=2, sticky="w")

            ext = ttk.LabelFrame(parent, text="Event generators and external packages", padding=8)
            ext.grid(row=0, column=2, sticky="nsew", padx=(6, 0), pady=6)
            ext.columnconfigure(2, weight=1)
            self.prefix_vars = {}
            self.detect_labels = {}
            self.prefix_widgets = {}
            row = 0
            for opt, key, desc in GENERATORS:
                var = tk.BooleanVar(value=bool(s["options"].get(opt)))
                self.option_vars[opt] = var
                ttk.Checkbutton(ext, text=EXTERNALS[key]["label"], variable=var,
                                command=self._update_dependent_rows).grid(row=row, column=0, sticky="w")
                ttk.Label(ext, text=desc, foreground="gray35").grid(row=row, column=1, columnspan=3, sticky="w")
                row = self._make_prefix_row(ext, row + 1, key, "location:")
            ttk.Separator(ext).grid(row=row, column=0, columnspan=4, sticky="ew", pady=6)
            row += 1
            ttk.Label(ext, text="Pythia8 (for Pythia)").grid(row=row, column=0, columnspan=2, sticky="w")
            row = self._make_prefix_row(ext, row + 1, "PYTHIA8", "location:")
            ttk.Separator(ext).grid(row=row, column=0, columnspan=4, sticky="ew", pady=6)
            row += 1
            ttk.Label(ext, text="FastJet (for Jets)").grid(row=row, column=0, columnspan=2, sticky="w")
            row = self._make_prefix_row(ext, row + 1, "FASTJET", "location:")
            ttk.Separator(ext).grid(row=row, column=0, columnspan=4, sticky="ew", pady=6)
            row += 1
            ttk.Label(ext, text="ROOT").grid(row=row, column=0, sticky="w")
            self.root_dir = tk.StringVar(value=s.get("root_dir", ""))
            ttk.Entry(ext, textvariable=self.root_dir).grid(row=row, column=1, columnspan=2, sticky="ew", padx=4)
            ttk.Button(ext, text="Browse\u2026", width=8,
                       command=lambda: self._browse(self.root_dir)).grid(row=row, column=3)
            row += 1
            self.root_label = ttk.Label(ext, text="", foreground="gray35")
            self.root_label.grid(row=row, column=1, columnspan=3, sticky="w", padx=4)
            row += 1
            ttk.Label(ext, text="Install libraries to").grid(row=row, column=0, columnspan=2, sticky="w", pady=(6, 0))
            row += 1
            self.install_lib_dir = tk.StringVar(value=s["install_lib_dir"])
            ttk.Entry(ext, textvariable=self.install_lib_dir).grid(row=row, column=0, columnspan=3, sticky="ew", padx=(0, 4))
            ttk.Button(ext, text="Browse\u2026", width=8,
                       command=lambda: self._browse(self.install_lib_dir)).grid(row=row, column=3)
            row += 1
            ttk.Button(ext, text="Detect", command=self._detect_all).grid(row=row, column=3, sticky="e", pady=(6, 0))
            self._update_dependent_rows()

        def _make_prefix_row(self, parent, row, key, text):
            var = tk.StringVar(value=self.settings["prefixes"].get(key, ""))
            self.prefix_vars[key] = var
            label = ttk.Label(parent, text=text)
            label.grid(row=row, column=1, sticky="e", padx=(16, 0))
            entry = ttk.Entry(parent, textvariable=var)
            entry.grid(row=row, column=2, sticky="ew", padx=4)
            button = ttk.Button(parent, text="Browse\u2026", width=8, command=lambda: self._browse(var))
            button.grid(row=row, column=3)
            detected = ttk.Label(parent, text="", foreground="gray35", wraplength=300, justify=tk.LEFT)
            detected.grid(row=row + 1, column=2, columnspan=2, sticky="w", padx=4)
            self.detect_labels[key] = detected
            self.prefix_widgets[key] = (entry, button)
            var.trace_add("write", lambda *a: self._detect(key))
            return row + 2

        def _update_dependent_rows(self):
            needed = {"PYTHIA8": self.option_vars["CAP_WITH_PYTHIA8"].get(),
                      "HERWIG": self.option_vars["CAP_WITH_HERWIG"].get(),
                      "FASTJET": self.option_vars["CAP_WITH_JETS"].get()}
            for key, (entry, button) in self.prefix_widgets.items():
                state = tk.NORMAL if needed[key] else tk.DISABLED
                entry.configure(state=state)
                button.configure(state=state)

        def _browse(self, var):
            path = filedialog.askdirectory(initialdir=var.get() or CAP_DIR, mustexist=True)
            if path:
                var.set(path)

        def _detect(self, key):
            prefix, how = detect_external(key, self.prefix_vars[key].get().strip())
            if prefix:
                text, color = "\u2714 %s (%s)" % (prefix, how), "dark green"
            else:
                text, color = "\u2716 %s" % how, "firebrick"
            self.detect_labels[key].configure(text=text, foreground=color)

        def _detect_all(self):
            for key in EXTERNALS:
                self._detect(key)
            root, how = detect_root(self.root_dir.get().strip())
            if root:
                self.root_label.configure(text="\u2714 %s (%s)" % (root, how), foreground="dark green")
            else:
                self.root_label.configure(text="\u2716 not found: enter the ROOT installation directory",
                                          foreground="firebrick")

        def _refresh_build_dirs(self):
            self.build_dir_box.configure(values=existing_build_dirs())

        def _on_build_dir_changed(self):
            name = self.build_dir.get().strip()
            error = valid_build_dir_name(name)
            if error:
                self.build_dir_note.configure(text="\u2716 invalid", foreground="firebrick")
                return
            cache = read_cmake_cache(name)
            if cache:
                self.build_dir_note.configure(text="existing (options loaded)", foreground="gray40")
                for opt, var in self.option_vars.items():
                    if opt in cache:
                        var.set(cache[opt].upper() in ("ON", "1", "TRUE", "YES"))
                for key, ext in EXTERNALS.items():
                    if cache.get(ext["var"]) is not None and key in self.prefix_vars:
                        self.prefix_vars[key].set(cache[ext["var"]])
                if cache.get("CMAKE_BUILD_TYPE") in BUILD_TYPES:
                    self.build_type.set(cache["CMAKE_BUILD_TYPE"])
                self._update_dependent_rows()
            else:
                self.build_dir_note.configure(text="new", foreground="gray40")

        # ---------------------------------------------------------------------------------------------- output
        def _make_output(self, parent):
            header = ttk.Frame(parent)
            header.pack(fill=tk.X)
            ttk.Label(header, text="Output of cmake and make").pack(side=tk.LEFT)
            self.autoscroll = tk.BooleanVar(value=True)
            ttk.Checkbutton(header, text="Follow output", variable=self.autoscroll).pack(side=tk.RIGHT)
            font = ("Menlo", 11) if sys.platform == "darwin" else ("DejaVu Sans Mono", 10)
            self.output = ScrolledText(parent, wrap=tk.NONE, font=font, height=15, background="#1e1e1e",
                                       foreground="#d4d4d4", insertbackground="#d4d4d4")
            self.output.pack(fill=tk.BOTH, expand=True)
            hbar = ttk.Scrollbar(parent, orient=tk.HORIZONTAL, command=self.output.xview)
            hbar.pack(fill=tk.X)
            self.output.configure(xscrollcommand=hbar.set)
            self.output.tag_configure("command", foreground="#4fc1ff", font=font + ("bold",))
            self.output.tag_configure("error", foreground="#f48771")
            self.output.tag_configure("warning", foreground="#dcdcaa")
            self.output.tag_configure("success", foreground="#89d185", font=font + ("bold",))
            self.output.tag_configure("failure", foreground="#f48771", font=font + ("bold",))
            self.output.configure(state=tk.DISABLED)

        def append(self, text, tag=None):
            self.output.configure(state=tk.NORMAL)
            self.output.insert(tk.END, text, tag)
            self.output.configure(state=tk.DISABLED)
            if self.autoscroll.get():
                self.output.see(tk.END)

        @staticmethod
        def line_tag(line):
            low = line.lower()
            if "error" in low or "fatal" in low or "undefined reference" in low or "undefined symbol" in low:
                return "error"
            if "warning" in low:
                return "warning"
            return None

        def clear_output(self):
            self.output.configure(state=tk.NORMAL)
            self.output.delete("1.0", tk.END)
            self.output.configure(state=tk.DISABLED)

        def save_output(self):
            name = self.build_dir.get().strip() or "build"
            directory = os.path.join(CAP_DIR, name) if os.path.isdir(os.path.join(CAP_DIR, name)) else CAP_DIR
            filename = filedialog.asksaveasfilename(
                title="Save the build output", initialdir=directory,
                initialfile="%s_%s.log" % (name, datetime.datetime.now().strftime("%Y%m%d_%H%M%S")),
                defaultextension=".log", filetypes=[("Log files", "*.log"), ("Text files", "*.txt"), ("All files", "*")])
            if not filename:
                return
            try:
                with open(filename, "w") as f:
                    f.write(self.output.get("1.0", "end-1c"))
                self.status.set("Output saved to %s" % filename)
            except OSError as e:
                messagebox.showerror("Save output", "Cannot write %s:\n%s" % (filename, e))

        # ---------------------------------------------------------------------------------------------- build
        def current_settings(self):
            s = self.settings
            s["build_dir"] = self.build_dir.get().strip()
            s["build_type"] = self.build_type.get()
            try:
                s["jobs"] = max(1, int(self.jobs.get()))
            except (ValueError, tk.TclError):
                s["jobs"] = 1
            s["install"] = bool(self.install.get())
            s["reconfigure"] = bool(self.reconfigure.get())
            s["install_lib_dir"] = self.install_lib_dir.get().strip() or os.path.join(CAP_DIR, "lib")
            s["root_dir"] = self.root_dir.get().strip()
            s["options"] = {opt: bool(var.get()) for opt, var in self.option_vars.items()}
            s["prefixes"] = {key: var.get().strip() for key, var in self.prefix_vars.items()}
            return s

        def check(self, s):
            error = valid_build_dir_name(s["build_dir"])
            if error:
                return error
            if not which("cmake"):
                return "cmake was not found (looked in PATH and %s)." % ", ".join(EXTRA_PATHS)
            if not which("make"):
                return "make was not found."
            if not detect_root(s["root_dir"])[0]:
                return "ROOT was not found: enter its installation directory (the one containing bin/root-config)."
            missing = []
            for opt, key in (("CAP_WITH_PYTHIA8", "PYTHIA8"), ("CAP_WITH_HERWIG", "HERWIG"), ("CAP_WITH_JETS", "FASTJET")):
                if s["options"].get(opt) and not detect_external(key, s["prefixes"].get(key, ""))[0]:
                    missing.append(EXTERNALS[key]["label"])
            if missing:
                return ("%s selected but not found. Enter the installation directory, or uncheck the option."
                        % " and ".join(missing))
            return None

        def start_build(self):
            if self.worker and self.worker.is_alive():
                return
            s = self.current_settings()
            error = self.check(s)
            if error:
                messagebox.showerror("Cannot build", error)
                return
            save_settings({k: v for k, v in s.items() if k != "reconfigure"})
            build_dir = os.path.join(CAP_DIR, s["build_dir"])
            try:
                os.makedirs(build_dir, exist_ok=True)
                os.makedirs(s["install_lib_dir"], exist_ok=True)
                os.makedirs(os.path.join(CAP_DIR, "bin"), exist_ok=True)
                if s["reconfigure"] and os.path.exists(os.path.join(build_dir, "CMakeCache.txt")):
                    os.remove(os.path.join(build_dir, "CMakeCache.txt"))
            except OSError as e:
                messagebox.showerror("Cannot build", str(e))
                return
            commands = build_commands(s)
            env = build_environment(s)
            self.stop_requested = False
            self.start_time = time.time()
            self.build_button.configure(state=tk.DISABLED)
            self.stop_button.configure(state=tk.NORMAL)
            stamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            self.append("\n=== CAP build in %s (%s, %s) ===\n" % (build_dir, s["build_type"], stamp), "command")
            self.worker = threading.Thread(target=self._run_commands, args=(commands, env), daemon=True)
            self.worker.start()

        def _run_commands(self, commands, env):
            for description, cmd in commands:
                self.queue.put(("status", description + "\u2026"))
                self.queue.put(("text", "\n$ %s\n" % shell_text(cmd), "command"))
                try:
                    self.process = subprocess.Popen(cmd, cwd=CAP_DIR, env=env, stdout=subprocess.PIPE,
                                                    stderr=subprocess.STDOUT, text=True, bufsize=1,
                                                    errors="replace", start_new_session=True)
                except OSError as e:
                    self.queue.put(("done", False, "cannot run %s: %s" % (cmd[0], e)))
                    return
                for line in self.process.stdout:
                    self.queue.put(("line", line))
                code = self.process.wait()
                self.process = None
                if self.stop_requested:
                    self.queue.put(("done", False, "stopped by the user"))
                    return
                if code != 0:
                    self.queue.put(("done", False, "%s failed (exit code %d)" % (description.lower(), code)))
                    return
            self.queue.put(("done", True, "build succeeded"))

        def stop_build(self):
            if self.process and self.process.poll() is None:
                self.stop_requested = True
                try:
                    os.killpg(self.process.pid, signal.SIGTERM)
                except (OSError, AttributeError):
                    self.process.terminate()
                self.status.set("Stopping\u2026")

        def _poll_queue(self):
            lines = []
            try:
                while True:
                    item = self.queue.get_nowait()
                    if item[0] == "line":
                        lines.append(item[1])
                        if len(lines) >= 500:
                            break
                        continue
                    self._flush(lines)
                    lines = []
                    if item[0] == "text":
                        self.append(item[1], item[2])
                    elif item[0] == "status":
                        self.status.set(item[1])
                    elif item[0] == "done":
                        self._finished(item[1], item[2])
            except queue.Empty:
                pass
            self._flush(lines)
            self.root.after(50, self._poll_queue)

        def _flush(self, lines):
            if not lines:
                return
            self.output.configure(state=tk.NORMAL)
            for line in lines:
                self.output.insert(tk.END, line, self.line_tag(line))
            self.output.configure(state=tk.DISABLED)
            if self.autoscroll.get():
                self.output.see(tk.END)

        def _finished(self, ok, message):
            elapsed = time.time() - (self.start_time or time.time())
            text = "%s (%dm%02ds)" % (message[0].upper() + message[1:], elapsed // 60, elapsed % 60)
            self.append("\n=== %s ===\n" % text, "success" if ok else "failure")
            self.status.set(text)
            self.build_button.configure(state=tk.NORMAL)
            self.stop_button.configure(state=tk.DISABLED)
            self.reconfigure.set(False)
            self._refresh_build_dirs()
            self._on_build_dir_changed()

        def _on_close(self):
            if self.worker and self.worker.is_alive():
                if not messagebox.askyesno("Quit", "A build is running. Stop it and quit?"):
                    return
                self.stop_build()
            self.current_settings()
            save_settings({k: v for k, v in self.settings.items() if k != "reconfigure"})
            self.root.destroy()

    root = tk.Tk()
    BuildCAPApp(root)
    root.mainloop()


def main():
    if "--print-commands" in sys.argv:
        s = load_settings()
        for description, cmd in build_commands(s):
            print("# " + description)
            print(shell_text(cmd))
        return 0
    if "-h" in sys.argv or "--help" in sys.argv:
        print(__doc__)
        return 0
    try:
        import tkinter  # noqa: F401
    except ImportError:
        print("BuildCAP.py needs tkinter (macOS with Homebrew Python: brew install python-tk; "
              "Debian/Ubuntu: apt install python3-tk).", file=sys.stderr)
        return 1
    run_gui()
    return 0


if __name__ == "__main__":
    sys.exit(main())
