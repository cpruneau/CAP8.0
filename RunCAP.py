#!/usr/bin/env python3
# **********************************************************************
# Copyright (C) 2019-2024, Claude Pruneau
# All rights reserved.
#
# For the licensing terms see LICENSE.
# **********************************************************************
"""
RunCAP.py -- graphical front end to compose, configure and run CAP jobs (workflows).

Usage (from anywhere; the script locates CAP from its own position):
    python3 RunCAP.py [workflow.xml]          # open the GUI (and load a workflow)
    python3 RunCAP.py --print-command FILE    # print the command that runs the workflow FILE and exit (no GUI)
    python3 RunCAP.py --check FILE            # load FILE, check its values against the templates and exit (no GUI)

The window has four parts, from top to bottom:
  * a toolbar: Run, Save workflow, Load workflow, Filters, Save log, Clear log, Kill and Exit. Filters opens the
    Filters panel (RunCAPFiltersGui.py): one tab per kind (event and particle filters, efficiencies, smearers), the
    number of objects of that kind, and for each its name and title and its conditions or binning parameters;
  * the workflow editor: the tasks available in the current build (left), the workflow (right), and between them two
    arrow buttons. The right arrow appends the tasks selected on the left to the workflow (a task can be added several
    times, each copy with its own parameters); the left arrow removes the tasks selected in the workflow. The buttons
    under the workflow reorder the tasks and nest them: a nested task is a subtask of the task above it (e.g. the
    event generator and the analyzers inside an EventIterator, which executes them for every event);
  * the tasks: one line per task of the workflow, in run order, with the filters it uses, the number of its
    histogram folders and its first problem. Double-clicking a task (here or in the workflow), or Configure task,
    opens its window (RunCAPTaskDialog.py): General (name, report level, parameters), Filters (for each kind the task
    supports, how many objects of the library it uses and which ones), Histograms (prefix of the folder names, same
    binning for all folders or binning folder by folder) and Histogram files (creation, import and export of the
    folders). Booleans have a true/false menu, integers and reals fields that only accept numbers;
  * the output of CAP, which can be saved to a log file.

Available tasks: the XML templates written by "bin/CAP --xml-templates" (one file per task class, listing every
parameter with its default value and type, see CAP::Task::exportXmlTemplates()). They are kept in <CAP>/DB/Templates
and regenerated automatically when bin/CAP or the libraries are newer than them (or with the Refresh button).

Workflow files: a CAP configuration file (root element CAP, TASK elements, see Task.hpp). A workflow of a single task
(possibly with subtasks) is written as that task; several top-level tasks are written as subtasks of a task named
"Workflow" of class CAP::Task, which executes them in order. Run uses the file last saved, or, when the workflow was
changed since (or never saved), a temporary file. The job is run as:
    <CAP>/bin/CAP <root task> <directory of the file>/ <file name> <output directory>/
in the output directory, with the environment of SetupCAP.sh (taken from BuildCAP.py's settings).

Filters (RunCAPFilters.py): the event and particle filters, efficiencies and smearers of a workflow form a library
shared by its tasks; each task chooses its objects by name, and the analyzers get one histogram folder per combination
of their filters. write_workflow() writes their keys in the tasks (the first task using an object defines it) and the
library in a RUNCAP element next to the TASK element; load_workflow_with_filters() reads them back, also from files
without RUNCAP element.

The settings are saved in <CAP>/.RunCAP.json. Requires Python 3.7+ with tkinter (on macOS with Homebrew Python:
brew install python-tk).
"""

import copy
import datetime
import glob
import json
import os
import queue
import re
import signal
import subprocess
import sys
import tempfile
import threading
import time
import xml.etree.ElementTree as ET

import RunCAPFilters as filters

CAP_DIR = os.path.dirname(os.path.abspath(__file__))
BIN_DIR = os.path.join(CAP_DIR, "bin")
CAP_EXE = os.path.join(BIN_DIR, "CAP")
TEMPLATES_DIR = os.path.join(CAP_DIR, "DB", "Templates")
TEMPLATES_STAMP = os.path.join(TEMPLATES_DIR, ".generated")
SETTINGS_FILE = os.path.join(CAP_DIR, ".RunCAP.json")

#: Report levels of a task (CAP::Object).
SEVERITIES = ["TRACE", "DEBUG0", "DEBUG1", "DEBUG2", "INFO", "WARNING", "ERROR", "FATAL"]
#: Parameter types (CAP::Property::typeName()) and their aliases.
TYPE_ALIASES = {"bool": "bool", "boolean": "bool", "int": "int", "integer": "int", "long": "long",
                "double": "double", "float": "double", "real": "double", "string": "string", "text": "string"}
#: Name and class of the task wrapping a workflow of several top-level tasks.
WRAPPER_NAME, WRAPPER_CLASS = "Workflow", "CAP::Task"

INT_PARTIAL = re.compile(r"[+-]?\d*")
FLOAT_PARTIAL = re.compile(r"[+-]?\d*\.?\d*([eE][+-]?\d*)?")
INT_FULL = re.compile(r"[+-]?\d+")
NAME_FULL = re.compile(r"[A-Za-z_][A-Za-z0-9_.-]*")
TRUE_WORDS = ("1", "TRUE", "YES", "ON")
FALSE_WORDS = ("0", "FALSE", "NO", "OFF")


# ------------------------------------------------------------------------------------------------------------------
# Model (no GUI: usable and testable without tkinter)
# ------------------------------------------------------------------------------------------------------------------

class Param:
    """A task parameter: key relative to the task (groups joined with ':'), type, value, description."""

    def __init__(self, key, ptype, value, description=""):
        self.key = key
        self.type = TYPE_ALIASES.get((ptype or "string").lower(), "string")
        self.value = normalize_value(self.type, value)
        self.default = self.value
        self.description = description or ""


class TaskConfig:
    """A task of the workflow: name (unique), class, report level, parameters, subtasks (used on load/save only)."""

    def __init__(self, name, cls, severity="INFO", params=None, known=True):
        self.name = name
        self.cls = cls
        self.severity = severity or "INFO"
        self.params = params or []
        self.children = []
        self.known = known       # False if the class is not among the templates (not in the current build)

    def clone(self):
        c = copy.deepcopy(self)
        c.children = []
        return c

    def param(self, key):
        for p in self.params:
            if p.key == key:
                return p
        return None


def short_class_name(cls):
    """Class name without namespace, as in CAP::Task::xmlTemplateName() (e.g. CAP::PtPtAnalyzer -> PtPtAnalyzer)."""
    s = cls.replace("CAP::", "").replace("::", "_").replace("<", "_").replace(",", "_")
    return s.replace(">", "").replace(" ", "")


def normalize_value(ptype, value):
    value = "" if value is None else str(value).strip()
    if ptype == "bool":
        if value.upper() in TRUE_WORDS:
            return "true"
        if value.upper() in FALSE_WORDS:
            return "false"
    return value


def valid_value(ptype, text):
    """True if text is a complete value of the given type."""
    if ptype == "bool":
        return text in ("true", "false")
    if ptype in ("int", "long"):
        if not INT_FULL.fullmatch(text):
            return False
        limit = 2 ** 31 if ptype == "int" else 2 ** 63
        return -limit <= int(text) < limit
    if ptype == "double":
        try:
            float(text)
            return text.strip() != "" and "_" not in text and text.lower().lstrip("+-") not in ("nan", "inf", "infinity")
        except ValueError:
            return False
    return True


def partial_value(ptype, text):
    """True if text can be the beginning of a value of the given type (validation while typing)."""
    if ptype in ("int", "long"):
        return INT_PARTIAL.fullmatch(text) is not None
    if ptype == "double":
        return FLOAT_PARTIAL.fullmatch(text) is not None
    return True


def read_parameters(element, prefix=""):
    """Parameters of a TASK (or GROUP) element: PARAMETER children, and GROUP children recursively."""
    params = []
    for child in element:
        if child.tag == "PARAMETER":
            key = prefix + child.get("name", "")
            params.append(Param(key, child.get("type", "string"), child.text, child.get("description", "")))
        elif child.tag == "GROUP":
            params += read_parameters(child, prefix + child.get("name", "") + ":")
    return params


def load_templates(directory=TEMPLATES_DIR):
    """Templates of the task classes: {class name: TaskConfig with the default parameters}."""
    templates = {}
    for path in sorted(glob.glob(os.path.join(directory, "*.xml"))):
        try:
            root = ET.parse(path).getroot()
        except (ET.ParseError, OSError):
            continue
        for element in root.iter("TASK"):
            cls = element.get("class")
            if not cls:
                continue
            templates[cls] = TaskConfig(short_class_name(cls), cls, (element.get("severity") or "INFO").upper(),
                                        read_parameters(element))
            break
    return templates


def unique_name(base, used):
    """base, or base_2, base_3, ... : the first one not in used."""
    if base not in used:
        return base
    k = 2
    while "%s_%d" % (base, k) in used:
        k += 1
    return "%s_%d" % (base, k)


def new_task(cls, templates, used_names):
    """New task of class cls with the default parameters, named after its class (made unique)."""
    t = templates[cls].clone()
    t.name = unique_name(t.name, used_names)
    return t


def task_from_element(element, templates, warnings):
    """TaskConfig (with subtasks) from a TASK element: template defaults updated with the values of the element."""
    cls = element.get("class", "")
    name = element.get("name", "") or short_class_name(cls)
    if cls in templates:
        t = templates[cls].clone()
        t.name = name
    elif cls == WRAPPER_CLASS:
        t = TaskConfig(name, cls)
    else:
        t = TaskConfig(name, cls, known=False)
        warnings.append("Task %s: class %s is not in the current build (no template); its parameters are kept as read."
                        % (name, cls or "?"))
    if element.get("severity"):
        t.severity = element.get("severity").upper()
    for p in read_parameters(element):
        existing = t.param(p.key)
        if existing is None:
            if t.known and not filters.MANAGED_KEY.match(p.key):
                warnings.append("Task %s: parameter %s is not defined by class %s (kept)." % (name, p.key, cls))
            t.params.append(p)
        else:
            existing.value = normalize_value(existing.type, p.value)
    for child in element.findall("TASK"):
        t.children.append(task_from_element(child, templates, warnings))
    return t


def load_workflow(path, templates):
    """Top-level tasks (with subtasks) of a workflow file, and warnings. Raises ValueError for invalid files."""
    try:
        root = ET.parse(path).getroot()
    except ET.ParseError as e:
        raise ValueError("%s is not a valid XML file: %s" % (path, e))
    if root.tag == "TASK":
        elements = [root]
    else:
        elements = root.findall("TASK")
    if not elements:
        raise ValueError("%s contains no TASK element." % path)
    warnings = []
    tasks = [task_from_element(e, templates, warnings) for e in elements]
    if (len(tasks) == 1 and tasks[0].name == WRAPPER_NAME and tasks[0].cls == WRAPPER_CLASS
            and not any(p.value != p.default for p in tasks[0].params)):
        tasks = tasks[0].children
    return tasks, warnings


def all_tasks(tasks):
    for t in tasks:
        yield t
        yield from all_tasks(t.children)


def task_to_element(t):
    element = ET.Element("TASK", {"name": t.name, "class": t.cls, "severity": t.severity})
    groups = {}
    for p in t.params:
        parts = p.key.split(":")
        parent, path = element, ""
        for g in parts[:-1]:
            path += g + ":"
            if path not in groups:
                groups[path] = ET.SubElement(parent, "GROUP", {"name": g})
            parent = groups[path]
        attributes = {"name": parts[-1], "type": p.type}
        if p.description:
            attributes["description"] = p.description
        ET.SubElement(parent, "PARAMETER", attributes).text = p.value
    for c in t.children:
        element.append(task_to_element(c))
    return element


def root_task(tasks):
    """The task run by CAP: the single top-level task, or a wrapper holding the top-level tasks."""
    if len(tasks) == 1:
        return tasks[0]
    wrapper = TaskConfig(WRAPPER_NAME, WRAPPER_CLASS, "INFO")
    wrapper.children = list(tasks)
    return wrapper


def _indent(element, level=0):
    pad = "\n" + "  " * level
    if len(element):
        if not (element.text or "").strip():
            element.text = pad + "  "
        for child in element:
            _indent(child, level + 1)
        if not (child.tail or "").strip():
            child.tail = pad
    if level and not (element.tail or "").strip():
        element.tail = pad


def load_workflow_with_filters(path, templates):
    """Top-level tasks of a workflow file with their filters (RunCAPFilters): the library of filters, efficiencies and
    smearers (from the RUNCAP element of the file, if any, and the objects defined by the tasks), the choices of the
    tasks (task.filters) and their histogram options (task.histograms); the keys of these objects are removed from the
    parameters of the tasks (write_workflow() writes them again). Returns tasks, library, warnings."""
    tasks, warnings = load_workflow(path, templates)
    library = None
    element = ET.parse(path).getroot().find("RUNCAP")
    if element is not None:
        library, more = filters.read_runcap_element(element, tasks)
        warnings += more
    library, more = filters.extract_choices(tasks, templates, library)
    return tasks, library, warnings + more


def write_workflow(path, tasks, library=None, templates=None):
    """Write the workflow to path. With a library (RunCAPFilters.FilterLibrary), the keys of the filters, efficiencies,
    smearers and histogram folders chosen by the tasks are written in their parameters (the tasks are unchanged), and
    the library in a RUNCAP element. Returns the name of the task to run."""
    if library is not None:
        written = filters.prepared_for_writing(tasks, library, templates or {}, Param)
    else:
        written = tasks
    top = root_task(written)
    document = ET.Element("CAP", {"version": "1"})
    document.append(task_to_element(top))
    if library is not None:
        document.append(filters.runcap_element(library, written))
    _indent(document)
    with open(path, "wb") as f:
        f.write(b'<?xml version="1.0" encoding="UTF-8"?>\n')
        f.write(b"<!-- CAP workflow written by RunCAP.py on %s -->\n"
                % datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S").encode())
        f.write(ET.tostring(document, encoding="utf-8"))
        f.write(b"\n")
    return top.name


def check_workflow(tasks):
    """List of problems that prevent saving or running the workflow."""
    errors = []
    if not tasks:
        return ["The workflow is empty: add tasks with the right arrow."]
    names = [t.name for t in all_tasks(tasks)]
    if len(tasks) > 1 and WRAPPER_NAME in names:
        errors.append("The name %s is reserved for the task holding a workflow of several tasks." % WRAPPER_NAME)
    for name in sorted(set(n for n in names if names.count(n) > 1)):
        errors.append("Several tasks are named %s: task names must be unique." % name)
    for t in all_tasks(tasks):
        if not NAME_FULL.fullmatch(t.name):
            errors.append("Invalid task name '%s' (letters, digits, _ . -; no spaces or colons)." % t.name)
        for p in t.params:
            if not valid_value(p.type, p.value):
                errors.append("Task %s: invalid %s value '%s' for %s." % (t.name, p.type, p.value, p.key))
    return errors


def run_command(xml_path, root_name, output_dir):
    xml_path = os.path.abspath(xml_path)
    return [CAP_EXE, root_name, os.path.dirname(xml_path) + "/", os.path.basename(xml_path),
            os.path.abspath(output_dir).rstrip("/") + "/"]


def template_command():
    return [CAP_EXE, "--xml-templates", TEMPLATES_DIR + "/"]


def build_time():
    """Modification time of the newest of bin/CAP and the CAP libraries (0 if CAP is not built)."""
    if not os.path.exists(CAP_EXE):
        return 0
    files = [CAP_EXE] + glob.glob(os.path.join(lib_dir(), "lib*.dylib")) + glob.glob(os.path.join(lib_dir(), "lib*.so"))
    return max(os.path.getmtime(f) for f in files if os.path.exists(f))


def templates_outdated():
    if not os.path.exists(CAP_EXE):
        return False
    if not glob.glob(os.path.join(TEMPLATES_DIR, "*.xml")) or not os.path.exists(TEMPLATES_STAMP):
        return True
    return build_time() > os.path.getmtime(TEMPLATES_STAMP)


def lib_dir():
    try:
        import BuildCAP
        return BuildCAP.load_settings().get("install_lib_dir") or os.path.join(CAP_DIR, "lib")
    except Exception:
        return os.path.join(CAP_DIR, "lib")


def job_environment():
    """Environment of SetupCAP.sh, with ROOT and the CAP libraries (from BuildCAP.py's settings when available)."""
    try:
        import BuildCAP
        return BuildCAP.build_environment(BuildCAP.load_settings())
    except Exception:
        env = dict(os.environ)
        lib = os.path.join(CAP_DIR, "lib") + "/"
        env.update({"CAP_ROOT_PATH": CAP_DIR, "CAP_SRC_PATH": os.path.join(CAP_DIR, "src") + "/",
                    "CAP_BIN_PATH": BIN_DIR + "/", "CAP_LIB_PATH": lib,
                    "CAP_MACROS_PATH": os.path.join(CAP_DIR, "src", "Macros") + "/",
                    "CAP_PROJECTS_PATH": os.path.join(CAP_DIR, "projects") + "/",
                    "CAP_DATABASE_PATH": os.path.join(CAP_DIR, "DB") + "/"})
        for var in ("DYLD_LIBRARY_PATH", "LD_LIBRARY_PATH"):
            env[var] = lib + (os.pathsep + env[var] if env.get(var) else "")
        return env


def shell_text(cmd):
    return " ".join(c if re.fullmatch(r"[\w@%+=:,./-]+", c) else "'" + c.replace("'", "'\\''") + "'" for c in cmd)


def default_settings():
    return {
        "output_dir": os.environ.get("CAP_USER_HISTO_EXPORT_PATH") or os.path.join(CAP_DIR, "output"),
        "last_workflow": "",
        "workflow_dir": os.path.join(CAP_DIR, "projects"),
        "geometry": "1280x960",
    }


def load_settings():
    s = default_settings()
    try:
        with open(SETTINGS_FILE) as f:
            s.update(json.load(f))
    except (OSError, ValueError):
        pass
    return s


def save_settings(s):
    try:
        with open(SETTINGS_FILE, "w") as f:
            json.dump(s, f, indent=2)
    except OSError:
        pass


# ------------------------------------------------------------------------------------------------------------------
# GUI
# ------------------------------------------------------------------------------------------------------------------

def run_gui(initial_file=None, hook=None):
    import tkinter as tk
    from tkinter import ttk, filedialog, messagebox
    from tkinter.scrolledtext import ScrolledText

    MONO = ("Menlo", 11) if sys.platform == "darwin" else ("DejaVu Sans Mono", 10)

    class ToolTip:
        """Text shown when the pointer rests on a widget."""

        def __init__(self, widget, text):
            self.widget, self.text, self.window, self.job = widget, text, None, None
            widget.bind("<Enter>", self._schedule, add="+")
            widget.bind("<Leave>", self._hide, add="+")
            widget.bind("<ButtonPress>", self._hide, add="+")

        def _schedule(self, _event=None):
            self.job = self.widget.after(600, self._show)

        def _show(self):
            if self.window or not self.text:
                return
            x = self.widget.winfo_rootx() + 12
            y = self.widget.winfo_rooty() + self.widget.winfo_height() + 4
            self.window = tw = tk.Toplevel(self.widget)
            tw.wm_overrideredirect(True)
            tw.wm_geometry("+%d+%d" % (x, y))
            tk.Label(tw, text=self.text, justify=tk.LEFT, background="#ffffe0", relief=tk.SOLID, borderwidth=1,
                     wraplength=420, padx=4, pady=2).pack()

        def _hide(self, _event=None):
            if self.job:
                self.widget.after_cancel(self.job)
                self.job = None
            if self.window:
                self.window.destroy()
                self.window = None

    class ArrowButton(tk.Canvas):
        """Arrow-shaped button (direction 'right' or 'left') that can be enabled or disabled."""

        COLORS = {"normal": "#2f74c0", "active": "#1d5594", "disabled": "#c4c4c4"}

        def __init__(self, parent, direction, command, background):
            super().__init__(parent, width=56, height=40, highlightthickness=0, background=background,
                             cursor="hand2")
            pts = [6, 13, 30, 13, 30, 4, 50, 20, 30, 36, 30, 27, 6, 27]
            if direction == "left":
                pts = [56 - v if i % 2 == 0 else v for i, v in enumerate(pts)]
            self.shape = self.create_polygon(pts, outline="", fill=self.COLORS["normal"])
            self.command, self.enabled = command, True
            self.bind("<Enter>", lambda e: self._paint("active"))
            self.bind("<Leave>", lambda e: self._paint("normal"))
            self.bind("<ButtonRelease-1>", self._click)

        def _paint(self, state):
            self.itemconfigure(self.shape, fill=self.COLORS[state if self.enabled else "disabled"])

        def _click(self, _event):
            if self.enabled:
                self.command()

        def set_enabled(self, enabled):
            self.enabled = bool(enabled)
            self.configure(cursor="hand2" if enabled else "arrow")
            self._paint("normal")

    class ScrollFrame(ttk.Frame):
        """Frame with a vertical scroll bar; widgets go in self.inner."""

        def __init__(self, parent):
            super().__init__(parent)
            self.canvas = tk.Canvas(self, highlightthickness=0, borderwidth=0)
            bar = ttk.Scrollbar(self, orient=tk.VERTICAL, command=self.canvas.yview)
            self.inner = ttk.Frame(self.canvas, padding=(4, 2))
            self.window = self.canvas.create_window((0, 0), window=self.inner, anchor="nw")
            self.canvas.configure(yscrollcommand=bar.set)
            self.canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
            bar.pack(side=tk.RIGHT, fill=tk.Y)
            self.inner.bind("<Configure>", lambda e: self.canvas.configure(scrollregion=self.canvas.bbox("all")))
            self.canvas.bind("<Configure>", lambda e: self.canvas.itemconfigure(self.window, width=e.width))
            for w in (self.canvas, self.inner):
                w.bind("<Enter>", self._bind_wheel, add="+")
                w.bind("<Leave>", self._unbind_wheel, add="+")

        def _bind_wheel(self, _event):
            self.canvas.bind_all("<MouseWheel>", self._wheel)
            self.canvas.bind_all("<Button-4>", lambda e: self.canvas.yview_scroll(-3, "units"))
            self.canvas.bind_all("<Button-5>", lambda e: self.canvas.yview_scroll(3, "units"))

        def _unbind_wheel(self, _event):
            for seq in ("<MouseWheel>", "<Button-4>", "<Button-5>"):
                self.canvas.unbind_all(seq)

        def _wheel(self, event):
            if self.inner.winfo_reqheight() <= self.canvas.winfo_height():
                return
            step = -event.delta if sys.platform == "darwin" else -int(event.delta / 120) * 3
            self.canvas.yview_scroll(step, "units")

    class RunCAPApp:
        def __init__(self, root):
            self.root = root
            self.settings = load_settings()
            self.templates = {}
            self.items = {}             # workflow tree item -> TaskConfig
            self.dialogs = {}           # workflow tree item -> window of the task (RunCAPTaskDialog)
            self.library = filters.FilterLibrary()   # filters, efficiencies and smearers of the workflow
            self.filters_window = None
            self.saved_path = ""        # file of the last save or load
            self.dirty = False
            self.process = None
            self.worker = None
            self.kill_requested = False
            self.queue = queue.Queue()
            self.start_time = None
            root.title("CAP Workflow")
            root.geometry(self.settings.get("geometry") or "1280x960")
            root.minsize(980, 700)
            style = ttk.Style()
            if "clam" in style.theme_names() and sys.platform != "darwin":
                style.theme_use("clam")
            style.configure("Run.TButton", font=("TkDefaultFont", 12, "bold"))
            style.configure("Invalid.TEntry", foreground="firebrick")
            style.configure("Section.TLabel", font=("TkDefaultFont", 11, "bold"))
            self.background = style.lookup("TFrame", "background") or root.cget("background")
            self._make_toolbar()
            self.status = tk.StringVar(value="Ready")
            ttk.Label(root, textvariable=self.status, anchor="w", relief=tk.SUNKEN).pack(fill=tk.X, side=tk.BOTTOM)
            panes = ttk.PanedWindow(root, orient=tk.VERTICAL)
            panes.pack(fill=tk.BOTH, expand=True, padx=6, pady=(0, 6))
            self.panes = panes
            selection, parameters, output = ttk.Frame(panes), ttk.Frame(panes), ttk.Frame(panes)
            panes.add(selection, weight=2)
            panes.add(parameters, weight=3)
            panes.add(output, weight=2)
            self._make_selection(selection)
            self._make_parameters(parameters)
            self._make_output(output)
            self._update_arrows()
            self._update_title()
            root.protocol("WM_DELETE_WINDOW", self.exit)
            root.report_callback_exception = self._callback_error
            root.after(50, self._poll_queue)
            root.after(100, lambda: self._startup(initial_file))
            root.after(200, self._place_sashes)

        def _place_sashes(self):
            # initial heights: tasks 30%, parameters 45%, output 25%
            self.root.update_idletasks()
            h = self.panes.winfo_height()
            if h > 100:
                self.panes.sashpos(0, int(h * 0.30))
                self.panes.sashpos(1, int(h * 0.75))

        def _dialog(self, kind, **options):
            """File dialog of filedialog.<kind>. On macOS the file types are left out: Tk's native panel can abort
            the program when it is given type filters (seen with Load workflow), and it shows all files anyway."""
            if sys.platform == "darwin":
                options.pop("filetypes", None)
            options.setdefault("parent", self.root)
            initial = options.get("initialdir")
            if initial and not os.path.isdir(initial):
                options["initialdir"] = CAP_DIR
            try:
                return getattr(filedialog, kind)(**options) or ""
            except tk.TclError as e:
                messagebox.showerror("File dialog", str(e))
                return ""

        def _callback_error(self, exc, value, tb):
            """Errors of the GUI callbacks: shown in the output and in a dialog instead of only on the terminal."""
            import traceback
            text = "".join(traceback.format_exception(exc, value, tb))
            sys.stderr.write(text)
            self.append("\nInternal error of RunCAP.py:\n" + text, "failure")
            messagebox.showerror("RunCAP.py error", "%s: %s\n\nDetails in the output pane." % (exc.__name__, value))

        # ---------------------------------------------------------------------------------------------- toolbar
        def _make_toolbar(self):
            bar = ttk.Frame(self.root, padding=(6, 6, 6, 4))
            bar.pack(fill=tk.X)
            self.run_button = ttk.Button(bar, text="\u25B6 Run workflow", style="Run.TButton", command=self.run)
            self.run_button.pack(side=tk.LEFT)
            buttons = [("Save workflow\u2026", self.save_workflow, "Save the workflow and its parameters to an XML file"),
                       ("Load workflow\u2026", self.load_workflow, "Load a workflow XML file saved earlier"),
                       ("Filters\u2026", self.show_filters, "Event and particle filters, efficiencies and smearers of "
                                                            "the workflow, used by the tasks by name"),
                       ("Save log\u2026", self.save_log, "Save the output below to a log file"),
                       ("Clear log", self.clear_log, "Clear the output below")]
            for text, command, tip in buttons:
                b = ttk.Button(bar, text=text, command=command)
                b.pack(side=tk.LEFT, padx=(4, 0))
                ToolTip(b, tip)
            self.kill_button = ttk.Button(bar, text="\u25A0 Kill", command=self.kill, state=tk.DISABLED)
            self.kill_button.pack(side=tk.LEFT, padx=(12, 0))
            ToolTip(self.kill_button, "Stop the running job")
            ttk.Button(bar, text="Exit", command=self.exit).pack(side=tk.LEFT, padx=(12, 0))
            ttk.Label(bar, text="CAP: " + CAP_DIR, foreground="gray40").pack(side=tk.RIGHT)

        # ---------------------------------------------------------------------------------------------- tasks
        def _make_selection(self, parent):
            parent.columnconfigure(0, weight=2)
            parent.columnconfigure(2, weight=3)
            parent.rowconfigure(1, weight=1)

            head = ttk.Frame(parent)
            head.grid(row=0, column=0, sticky="ew", pady=(4, 2))
            ttk.Label(head, text="Available tasks", style="Section.TLabel").pack(side=tk.LEFT)
            refresh = ttk.Button(head, text="Refresh", width=8, command=lambda: self.generate_templates(True))
            refresh.pack(side=tk.RIGHT)
            ToolTip(refresh, "Regenerate the task templates from bin/CAP (bin/CAP --xml-templates)")
            self.filter = tk.StringVar()
            self.filter.trace_add("write", lambda *a: self._fill_available())
            filter_entry = ttk.Entry(head, textvariable=self.filter, width=14)
            filter_entry.pack(side=tk.RIGHT, padx=4)
            ttk.Label(head, text="Filter:").pack(side=tk.RIGHT)

            box = ttk.Frame(parent)
            box.grid(row=1, column=0, sticky="nsew")
            self.available = tk.Listbox(box, selectmode=tk.EXTENDED, exportselection=False, activestyle="none",
                                        font=("TkDefaultFont", 12))
            sb = ttk.Scrollbar(box, orient=tk.VERTICAL, command=self.available.yview)
            self.available.configure(yscrollcommand=sb.set)
            self.available.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
            sb.pack(side=tk.RIGHT, fill=tk.Y)
            self.available.bind("<<ListboxSelect>>", self._on_available_select)
            self.available.bind("<Double-Button-1>", lambda e: self.add_tasks())
            self.available.bind("<Return>", lambda e: self.add_tasks())
            self.available_classes = []

            arrows = ttk.Frame(parent)
            arrows.grid(row=1, column=1, padx=10)
            self.right_arrow = ArrowButton(arrows, "right", self.add_tasks, self.background)
            self.right_arrow.pack(pady=(0, 14))
            self.left_arrow = ArrowButton(arrows, "left", self.remove_tasks, self.background)
            self.left_arrow.pack()
            ToolTip(self.right_arrow, "Add the selected tasks to the workflow")
            ToolTip(self.left_arrow, "Remove the selected tasks from the workflow")

            ttk.Label(parent, text="Workflow (run order)", style="Section.TLabel").grid(
                row=0, column=2, sticky="w", pady=(4, 2))
            box = ttk.Frame(parent)
            box.grid(row=1, column=2, sticky="nsew")
            self.tree = ttk.Treeview(box, columns=("class",), selectmode="extended")
            self.tree.heading("#0", text="Task")
            self.tree.heading("class", text="Class")
            self.tree.column("#0", width=230, stretch=True)
            self.tree.column("class", width=230, stretch=True)
            self.tree.tag_configure("unknown", foreground="firebrick")
            sb = ttk.Scrollbar(box, orient=tk.VERTICAL, command=self.tree.yview)
            self.tree.configure(yscrollcommand=sb.set)
            self.tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
            sb.pack(side=tk.RIGHT, fill=tk.Y)
            self.tree.bind("<<TreeviewSelect>>", self._on_tree_select)
            self.tree.bind("<Delete>", lambda e: self.remove_tasks())
            self.tree.bind("<BackSpace>", lambda e: self.remove_tasks())
            self.tree.bind("<Double-Button-1>", self._on_tree_double_click)

            moves = ttk.Frame(parent)
            moves.grid(row=2, column=2, sticky="w", pady=(4, 0))
            for text, command, tip in [
                    ("\u25B2 Up", lambda: self.move(-1), "Move the selected task up"),
                    ("\u25BC Down", lambda: self.move(1), "Move the selected task down"),
                    ("\u21E5 Nest", self.nest, "Make the selected task a subtask of the task above it (e.g. "
                                               "analyzers inside an EventIterator, which runs them for every event)"),
                    ("\u21E4 Unnest", self.unnest, "Move the selected task out of its parent task")]:
                b = ttk.Button(moves, text=text, command=command, width=9)
                b.pack(side=tk.LEFT, padx=(0, 4))
                ToolTip(b, tip)
            self.count_label = ttk.Label(moves, text="", foreground="gray40")
            self.count_label.pack(side=tk.LEFT, padx=8)

        def _fill_available(self):
            text = self.filter.get().strip().lower()
            self.available.delete(0, tk.END)
            self.available_classes = []
            for cls in sorted(self.templates, key=lambda c: short_class_name(c).lower()):
                name = short_class_name(cls)
                if text and text not in name.lower():
                    continue
                self.available.insert(tk.END, name)
                self.available_classes.append(cls)
            self._update_arrows()

        def _on_available_select(self, _event=None):
            if self.available.curselection() and self.tree.selection():
                self.tree.selection_remove(self.tree.selection())
            self._update_arrows()

        def _on_tree_select(self, _event=None):
            if self.tree.selection() and self.available.curselection():
                self.available.selection_clear(0, tk.END)
            self._update_arrows()

        def _on_tree_double_click(self, event):
            item = self.tree.identify_row(event.y)
            if item:
                self.configure_task(item)
                return "break"          # no open/close of the subtasks

        def _update_arrows(self):
            self.right_arrow.set_enabled(bool(self.available.curselection()))
            self.left_arrow.set_enabled(bool(self.tree.selection()))

        def _workflow_items(self, parent=""):
            """Workflow items in run order (depth first)."""
            out = []
            for item in self.tree.get_children(parent):
                out.append(item)
                out += self._workflow_items(item)
            return out

        def _used_names(self):
            return {self.items[i].name for i in self._workflow_items()}

        def _insert_task(self, parent, task, index=tk.END):
            item = self.tree.insert(parent, index, text=task.name, values=(task.cls,), open=True,
                                    tags=() if task.known else ("unknown",))
            self.items[item] = task
            for child in task.children:
                self._insert_task(item, child)
            task.children = []
            return item

        def add_tasks(self):
            selected = [self.available_classes[i] for i in self.available.curselection()]
            if not selected:
                return
            new = []
            for cls in selected:
                new.append(self._insert_task("", new_task(cls, self.templates, self._used_names())))
            self.tree.see(new[-1])
            self._changed("Added %s; double-click a task to configure it."
                          % ", ".join(self.items[i].name for i in new))

        def remove_tasks(self):
            selected = [i for i in self.tree.selection() if self.tree.exists(i)]
            if not selected:
                return
            names = []
            for item in selected:
                if not self.tree.exists(item):
                    continue       # child of a task removed before
                for sub in [item] + self._workflow_items(item):
                    names.append(self.items[sub].name)
                    self._close_dialogs([sub])
                    self.items.pop(sub, None)
                self.tree.delete(item)
            self._changed("Removed %s." % ", ".join(names))

        def _selected_one(self):
            selection = self.tree.selection()
            if len(selection) != 1:
                self.status.set("Select one task of the workflow.")
                return None
            return selection[0]

        def move(self, step):
            item = self._selected_one()
            if not item:
                return
            parent = self.tree.parent(item)
            index = self.tree.index(item)
            # Moving an item to a higher index within the same parent is interpreted differently by Tk versions
            # (some count the item itself, which makes it a no-op), so a move down is done by moving the next
            # sibling up: moves to a lower index behave the same in all versions.
            if step < 0 and index > 0:
                self.tree.move(item, parent, index - 1)
            elif step > 0 and self.tree.next(item):
                self.tree.move(self.tree.next(item), parent, index)
            else:
                return
            self.tree.selection_set(item)
            self.tree.see(item)
            self._changed()

        def nest(self):
            item = self._selected_one()
            if not item:
                return
            above = self.tree.prev(item)
            if not above:
                self.status.set("There is no task above %s at its level to nest it into." % self.items[item].name)
                return
            self.tree.move(item, above, tk.END)
            self.tree.item(above, open=True)
            self.tree.see(item)
            self._changed("%s is now a subtask of %s." % (self.items[item].name, self.items[above].name))

        def unnest(self):
            item = self._selected_one()
            if not item:
                return
            parent = self.tree.parent(item)
            if not parent:
                return
            self.tree.move(item, self.tree.parent(parent), self.tree.index(parent) + 1)
            self.tree.see(item)
            self._changed()

        def workflow(self):
            """Top-level TaskConfigs, with their children set from the tree."""
            def build(item):
                t = self.items[item]
                t.children = [build(c) for c in self.tree.get_children(item)]
                return t
            return [build(i) for i in self.tree.get_children("")]

        def set_workflow(self, tasks):
            self._close_dialogs()
            self.items.clear()
            self.tree.delete(*self.tree.get_children(""))
            for t in tasks:
                self._insert_task("", t)
            self._update_summary()

        # ---------------------------------------------------------------------------------------------- parameters
        def _make_parameters(self, parent):
            line = ttk.Frame(parent)
            line.pack(fill=tk.X, pady=(6, 4))
            configure = ttk.Button(line, text="Configure task\u2026", command=self.configure_selected)
            configure.pack(side=tk.LEFT)
            ToolTip(configure, "Open the window of the selected task (or double-click a task): its parameters, "
                               "filters and histograms")
            ttk.Label(line, text="Double-click a task to configure it.", foreground="gray40").pack(side=tk.LEFT,
                                                                                                padx=10)
            ttk.Button(line, text="Browse\u2026", width=8, command=self._browse_output).pack(side=tk.RIGHT)
            self.output_dir = tk.StringVar(value=self.settings["output_dir"])
            ttk.Entry(line, textvariable=self.output_dir, width=46).pack(side=tk.RIGHT, padx=4)
            label = ttk.Label(line, text="Output directory:")
            label.pack(side=tk.RIGHT)
            ToolTip(label, "Directory of the histograms written by the job (last argument of bin/CAP); "
                           "the job runs in this directory")
            box = ttk.Frame(parent)
            box.pack(fill=tk.BOTH, expand=True)
            columns = ("class", "events", "particles", "others", "folders", "problems")
            self.summary = ttk.Treeview(box, columns=columns, selectmode="browse")
            for column, text, width in (("#0", "Task (run order)", 200), ("class", "Class", 170),
                                        ("events", "Event filters", 150), ("particles", "Particle filters", 190),
                                        ("others", "Efficiencies, smearers", 150), ("folders", "Histogram folders", 120),
                                        ("problems", "Problems", 260)):
                self.summary.heading(column, text=text, anchor="w")
                self.summary.column(column, width=width, stretch=column in ("#0", "particles", "problems"))
            self.summary.tag_configure("problem", foreground="firebrick")
            bar = ttk.Scrollbar(box, orient=tk.VERTICAL, command=self.summary.yview)
            self.summary.configure(yscrollcommand=bar.set)
            self.summary.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
            bar.pack(side=tk.RIGHT, fill=tk.Y)
            self.summary.bind("<Double-Button-1>", lambda e: self.configure_task(self.summary.identify_row(e.y)))
            self.summary.bind("<Return>", lambda e: self.configure_task(self.summary.focus()))

        def _browse_output(self):
            path = self._dialog("askdirectory", initialdir=self.output_dir.get() or CAP_DIR)
            if path:
                self.output_dir.set(path)

        def _update_summary(self):
            """One line per task of the workflow, in run order: its choices, folders and problems."""
            selected = self.summary.selection()
            self.summary.delete(*self.summary.get_children(""))
            for item in self._workflow_items():
                task = self.items[item]
                depth, p = 0, self.tree.parent(item)
                while p:
                    depth, p = depth + 1, self.tree.parent(p)
                choices = filters.choices(task)
                others = sum(len(choices.get(k, [])) for k in filters.KINDS[2:])
                folders = filters.folders(task, self.templates)
                errors = [e for e in filters.check_choices([task], self.library, self.templates)
                          if ("Task %s " % task.name) in e or ("Task %s:" % task.name) in e]
                self.summary.insert("", tk.END, iid=item, text="  " * depth + task.name, tags=("problem",) if errors else (),
                                    values=(short_class_name(task.cls), ", ".join(choices.get("EVENT_FILTER", [])),
                                            ", ".join(choices.get("PARTICLE_FILTER", [])), others or "",
                                            len(folders) if filters.histogram_layout(task, self.templates) else "",
                                            errors[0] if errors else ""))
            for item in selected:
                if self.summary.exists(item):
                    self.summary.selection_set(item)

        def configure_selected(self):
            item = self._selected_one() or (self.summary.selection() or [None])[0]
            if item is None:
                messagebox.showinfo("Configure task", "Select a task of the workflow.")
                return
            self.configure_task(item)

        def configure_task(self, item):
            """Open (or raise) the window of a task."""
            if not item or item not in self.items:
                return
            dialog = self.dialogs.get(item)
            if dialog is not None and dialog.winfo_exists():
                dialog.deiconify()
                dialog.lift()
                return
            import RunCAPTaskDialog
            dialog = RunCAPTaskDialog.TaskDialog(
                self.root, self.items[item], self.templates, self.library,
                {"valid_value": valid_value, "partial_value": partial_value, "severities": SEVERITIES},
                rename=self._rename_task, changed=self._changed, show_filters=self.show_filters)
            self.dialogs[item] = dialog
            dialog.bind("<Destroy>", lambda e, i=item, d=dialog: self.dialogs.pop(i, None) if e.widget is d
                        and self.dialogs.get(i) is d else None, add="+")

        def _close_dialogs(self, items=None):
            for item in list(items if items is not None else self.dialogs):
                dialog = self.dialogs.pop(item, None)
                if dialog is not None and dialog.winfo_exists():
                    dialog.destroy()

        def _rename_task(self, task, new):
            """Rename a task (from its window). Returns None, or why the name is refused."""
            if not NAME_FULL.fullmatch(new):
                return "Invalid task name '%s': use letters, digits, _ . - (no spaces or colons)." % new
            if new in self._used_names() - {task.name}:
                return "Another task of the workflow is named %s." % new
            options = getattr(task, "histograms", None)
            if options is not None and options.prefix == task.name:
                options.prefix = new                    # folders named after the task follow it
            task.name = new
            for item, t in self.items.items():
                if t is task:
                    self.tree.item(item, text=new)
            self._changed()
            return None

        # ---------------------------------------------------------------------------------------------- output
        def _make_output(self, parent):
            header = ttk.Frame(parent)
            header.pack(fill=tk.X, pady=(4, 0))
            ttk.Label(header, text="Output", style="Section.TLabel").pack(side=tk.LEFT)
            self.autoscroll = tk.BooleanVar(value=True)
            ttk.Checkbutton(header, text="Follow output", variable=self.autoscroll).pack(side=tk.RIGHT)
            self.output = ScrolledText(parent, wrap=tk.NONE, font=MONO, height=10, background="#1e1e1e",
                                       foreground="#d4d4d4", insertbackground="#d4d4d4")
            self.output.pack(fill=tk.BOTH, expand=True)
            hbar = ttk.Scrollbar(parent, orient=tk.HORIZONTAL, command=self.output.xview)
            hbar.pack(fill=tk.X)
            self.output.configure(xscrollcommand=hbar.set)
            self.output.tag_configure("command", foreground="#4fc1ff", font=MONO + ("bold",))
            self.output.tag_configure("error", foreground="#f48771")
            self.output.tag_configure("warning", foreground="#dcdcaa")
            self.output.tag_configure("success", foreground="#89d185", font=MONO + ("bold",))
            self.output.tag_configure("failure", foreground="#f48771", font=MONO + ("bold",))
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
            if "error" in low or "fatal" in low or "exception" in low or "failed" in low:
                return "error"
            if "warning" in low:
                return "warning"
            return None

        def clear_log(self):
            self.output.configure(state=tk.NORMAL)
            self.output.delete("1.0", tk.END)
            self.output.configure(state=tk.DISABLED)

        def save_log(self):
            base = os.path.splitext(os.path.basename(self.saved_path))[0] if self.saved_path else "CAP"
            directory = self.output_dir.get().strip()
            if not os.path.isdir(directory):
                directory = CAP_DIR
            filename = self._dialog("asksaveasfilename", 
                title="Save the output", initialdir=directory,
                initialfile="%s_%s.log" % (base, datetime.datetime.now().strftime("%Y%m%d_%H%M%S")),
                defaultextension=".log", filetypes=[("Log files", "*.log"), ("Text files", "*.txt"), ("All files", "*")])
            if not filename:
                return
            try:
                with open(filename, "w") as f:
                    f.write(self.output.get("1.0", "end-1c"))
                self.status.set("Output saved to %s" % filename)
            except OSError as e:
                messagebox.showerror("Save log", "Cannot write %s:\n%s" % (filename, e))

        # ---------------------------------------------------------------------------------------------- files
        def _changed(self, message=None):
            self.dirty = True
            self._update_title()
            self._update_summary()
            self.count_label.configure(text="%d task(s)" % len(self._workflow_items()))
            self._update_arrows()
            if message:
                self.status.set(message)

        def _update_title(self):
            name = os.path.basename(self.saved_path) if self.saved_path else "untitled"
            self.root.title("CAP Workflow \u2014 %s%s" % (name, " *" if self.dirty else ""))

        def _check(self, title, run=False):
            """Problems that prevent saving (the workflow, the library) or running (also the choices of the tasks)."""
            self.root.focus_set()           # commit a task name being edited
            self.root.update_idletasks()
            tasks = self.workflow()
            errors = check_workflow(tasks)
            if run:
                errors += filters.check_choices(tasks, self.library, self.templates)
            else:
                errors += filters.check_library(self.library)
            if errors:
                more = "\n(and %d more)" % (len(errors) - 12) if len(errors) > 12 else ""
                messagebox.showerror(title, "\n".join(errors[:12]) + more)
                return False
            return True

        def save_workflow(self):
            if not self._check("Cannot save the workflow"):
                return False
            initial = self.saved_path or os.path.join(self.settings.get("workflow_dir") or CAP_DIR, "workflow.xml")
            filename = self._dialog("asksaveasfilename", 
                title="Save the workflow", initialdir=os.path.dirname(initial), initialfile=os.path.basename(initial),
                defaultextension=".xml", filetypes=[("CAP workflow", "*.xml"), ("All files", "*")])
            if not filename:
                return False
            try:
                write_workflow(filename, self.workflow(), self.library, self.templates)
            except OSError as e:
                messagebox.showerror("Save workflow", "Cannot write %s:\n%s" % (filename, e))
                return False
            self.saved_path = filename
            self.settings["last_workflow"] = filename
            self.settings["workflow_dir"] = os.path.dirname(filename)
            self.dirty = False
            self._update_title()
            self.status.set("Workflow saved to %s" % filename)
            return True

        def load_workflow(self, filename=None):
            if self.dirty and self._workflow_items() and not messagebox.askyesno(
                    "Load workflow", "Discard the changes made to the current workflow?"):
                return
            if not filename:
                filename = self._dialog("askopenfilename", 
                    title="Load a workflow", initialdir=self.settings.get("workflow_dir") or CAP_DIR,
                    filetypes=[("CAP workflow", "*.xml"), ("All files", "*")])
            if not filename:
                return
            try:
                tasks, library, warnings = load_workflow_with_filters(filename, self.templates)
            except (ValueError, OSError) as e:
                messagebox.showerror("Load workflow", str(e))
                return
            self.library = library
            self.set_workflow(tasks)
            if self.filters_window is not None:
                self.filters_window.panel.library = library
                self.filters_window.panel.refresh()
            self.saved_path = filename
            self.settings["last_workflow"] = filename
            self.settings["workflow_dir"] = os.path.dirname(filename)
            self.dirty = False
            self._update_title()
            self.count_label.configure(text="%d task(s)" % len(self._workflow_items()))
            self.status.set("Workflow loaded from %s" % filename)
            for w in warnings:
                self.append("Warning: %s\n" % w, "warning")
            if warnings:
                messagebox.showwarning("Load workflow", "%d warning(s), see the output." % len(warnings))

        # ---------------------------------------------------------------------------------------------- filters
        def show_filters(self, kind=None):
            """Show the Filters panel (made the first time), at the tab of a kind."""
            if self.filters_window is None:
                import RunCAPFiltersGui
                self.filters_window = RunCAPFiltersGui.FiltersWindow(
                    self.root, self.library, on_change=self._library_changed, on_rename=self._rename_object,
                    on_remove=self._remove_objects, users=self._object_users)
            self.filters_window.show(self.library)
            if kind:
                self.filters_window.panel.select(kind)

        def _library_changed(self):
            self._changed()
            for dialog in list(self.dialogs.values()):
                if dialog.winfo_exists():
                    dialog.library_changed()

        def _object_users(self, kind, name):
            """Names of the tasks of the workflow that use an object of the library."""
            return [t.name for t in all_tasks(self.workflow()) if name in filters.choices(t).get(kind, [])]

        def _rename_object(self, kind, old, new):
            for t in self.items.values():
                names = filters.choices(t).get(kind)
                if names:
                    filters.choices(t)[kind] = [new if n == old else n for n in names]
                options = getattr(t, "histograms", None)
                if options is not None and kind in filters.CONDITION_KINDS:      # binning per combination of names
                    options.binning = {tuple(new if n == old else n for n in combo): values
                                       for combo, values in options.binning.items()}

        def _remove_objects(self, kind, names):
            for t in self.items.values():
                if filters.choices(t).get(kind):
                    filters.choices(t)[kind] = [n for n in filters.choices(t)[kind] if n not in names]
                options = getattr(t, "histograms", None)
                if options is not None and kind in filters.CONDITION_KINDS:
                    options.binning = {combo: values for combo, values in options.binning.items()
                                       if not set(combo) & set(names)}

        # ---------------------------------------------------------------------------------------------- run
        def _busy(self):
            return self.worker is not None and self.worker.is_alive()

        def run(self):
            if self._busy():
                return
            if not os.path.exists(CAP_EXE):
                messagebox.showerror("Run", "%s does not exist: build and install CAP first (BuildCAP.py)." % CAP_EXE)
                return
            if not self._check("Cannot run the workflow", run=True):
                return
            output_dir = self.output_dir.get().strip()
            if not output_dir:
                messagebox.showerror("Run", "Enter the output directory.")
                return
            try:
                os.makedirs(output_dir, exist_ok=True)
                tasks = self.workflow()
                if self.saved_path and not self.dirty:
                    path = self.saved_path
                    name = root_task(tasks).name
                else:
                    stamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
                    fd, path = tempfile.mkstemp(prefix="RunCAP_%s_" % stamp, suffix=".xml")
                    os.close(fd)
                    name = write_workflow(path, tasks, self.library, self.templates)
            except OSError as e:
                messagebox.showerror("Run", str(e))
                return
            self.settings["output_dir"] = output_dir
            save_settings(self.settings)
            cmd = run_command(path, name, output_dir)
            stamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            self.append("\n=== CAP job %s (%s) ===\n" % (name, stamp), "command")
            if path != self.saved_path:
                self.append("Workflow not saved since the last change: running the temporary file %s\n" % path)
            self._start([("Running CAP", cmd, output_dir)], "job")

        def generate_templates(self, force=False):
            if self._busy():
                return
            if not os.path.exists(CAP_EXE):
                self.status.set("bin/CAP not found: build CAP first. Available tasks come from %s" % TEMPLATES_DIR)
                return
            if not force and not templates_outdated():
                return
            os.makedirs(TEMPLATES_DIR, exist_ok=True)
            self.append("\n=== Task templates (bin/CAP or the libraries changed) ===\n", "command")
            self._start([("Writing the task templates", template_command(), CAP_DIR)], "templates")

        def _start(self, commands, kind):
            self.kill_requested = False
            self.start_time = time.time()
            self.run_button.configure(state=tk.DISABLED)
            self.kill_button.configure(state=tk.NORMAL)
            env = job_environment()
            self.worker = threading.Thread(target=self._run_commands, args=(commands, env, kind), daemon=True)
            self.worker.start()

        def _run_commands(self, commands, env, kind):
            for description, cmd, cwd in commands:
                self.queue.put(("status", description + "\u2026"))
                self.queue.put(("text", "$ %s\n" % shell_text(cmd), "command"))
                try:
                    self.process = subprocess.Popen(cmd, cwd=cwd, env=env, stdout=subprocess.PIPE,
                                                    stderr=subprocess.STDOUT, text=True, bufsize=1,
                                                    errors="replace", start_new_session=True)
                except OSError as e:
                    self.queue.put(("done", kind, False, "cannot run %s: %s" % (cmd[0], e)))
                    return
                for line in self.process.stdout:
                    self.queue.put(("line", line))
                code = self.process.wait()
                self.process = None
                if self.kill_requested:
                    self.queue.put(("done", kind, False, "killed by the user"))
                    return
                if code != 0:
                    self.queue.put(("done", kind, False, "%s failed (exit code %d)" % (description.lower(), code)))
                    return
            self.queue.put(("done", kind, True, "job completed" if kind == "job" else "templates written"))

        def kill(self):
            process = self.process
            if process and process.poll() is None:
                self.kill_requested = True
                try:
                    os.killpg(process.pid, signal.SIGTERM)
                except (OSError, AttributeError):
                    process.terminate()
                self.status.set("Killing the job\u2026")

                def force():
                    if process.poll() is None:
                        try:
                            os.killpg(process.pid, signal.SIGKILL)
                        except (OSError, AttributeError):
                            process.kill()
                self.root.after(3000, force)

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
                        self._finished(*item[1:])
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

        def _finished(self, kind, ok, message):
            elapsed = time.time() - (self.start_time or time.time())
            text = "%s (%dm%02ds)" % (message[0].upper() + message[1:], elapsed // 60, elapsed % 60)
            self.append("=== %s ===\n" % text, "success" if ok else "failure")
            self.status.set(text)
            self.run_button.configure(state=tk.NORMAL)
            self.kill_button.configure(state=tk.DISABLED)
            if kind == "templates":
                if ok:
                    try:
                        with open(TEMPLATES_STAMP, "w") as f:
                            f.write(datetime.datetime.now().isoformat() + "\n")
                    except OSError:
                        pass
                self.reload_templates()

        def reload_templates(self):
            self.templates = load_templates()
            self._fill_available()
            if not self.templates:
                self.status.set("No task templates in %s: build CAP, then click Refresh." % TEMPLATES_DIR)
            else:
                self.status.set("%d task classes available." % len(self.templates))
            for item, task in self.items.items():      # tasks of the workflow whose class appeared or disappeared
                known = task.cls in self.templates or task.cls == WRAPPER_CLASS
                if known != task.known:
                    task.known = known
                    self.tree.item(item, tags=() if known else ("unknown",))

        def _startup(self, initial_file):
            self.reload_templates()
            if initial_file:
                self.load_workflow(initial_file)
            if templates_outdated():
                self.generate_templates()
            elif not os.path.exists(CAP_EXE) and not self.templates:
                self.append("bin/CAP was not found: build CAP with BuildCAP.py, then click Refresh.\n", "warning")

        def exit(self):
            if self._busy():
                if not messagebox.askyesno("Exit", "A job is running. Kill it and exit?"):
                    return
                self.kill()
            elif self.dirty and self._workflow_items():
                answer = messagebox.askyesnocancel("Exit", "Save the changes made to the workflow before exiting?")
                if answer is None or (answer and not self.save_workflow()):
                    return
            self.settings["output_dir"] = self.output_dir.get().strip() or self.settings["output_dir"]
            self.settings["geometry"] = self.root.winfo_geometry()
            save_settings(self.settings)
            self.root.destroy()

    root = tk.Tk()
    app = RunCAPApp(root)
    if hook:
        hook(app, root)
    root.mainloop()


# ------------------------------------------------------------------------------------------------------------------
# Command line
# ------------------------------------------------------------------------------------------------------------------

def main():
    args = sys.argv[1:]
    if "-h" in args or "--help" in args:
        print(__doc__)
        return 0
    if args and args[0] in ("--check", "--print-command"):
        if len(args) < 2:
            print("Usage: RunCAP.py %s workflow.xml" % args[0], file=sys.stderr)
            return 2
        templates = load_templates()
        try:
            tasks, library, warnings = load_workflow_with_filters(args[1], templates)
        except (ValueError, OSError) as e:
            print(e, file=sys.stderr)
            return 1
        if args[0] == "--print-command":
            print(shell_text(run_command(args[1], root_task(tasks).name, load_settings()["output_dir"])))
            return 0
        for w in warnings:
            print("warning: " + w)
        errors = check_workflow(tasks) + filters.check_choices(tasks, library, templates)
        for e in errors:
            print("error: " + e)
        print("%d task(s), %d warning(s), %d error(s)" % (len(list(all_tasks(tasks))), len(warnings), len(errors)))
        return 1 if errors else 0
    try:
        import tkinter  # noqa: F401
    except ImportError:
        print("RunCAP.py needs tkinter (macOS with Homebrew Python: brew install python-tk; "
              "Debian/Ubuntu: apt install python3-tk).", file=sys.stderr)
        return 1
    try:
        import faulthandler      # a crash of Tk itself leaves the Python stack in <CAP>/.RunCAP.crash.log
        faulthandler.enable(open(os.path.join(CAP_DIR, ".RunCAP.crash.log"), "w"))
    except OSError:
        pass
    run_gui(args[0] if args else None)
    return 0


if __name__ == "__main__":
    sys.exit(main())
