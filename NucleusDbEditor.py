#!/usr/bin/env python3
# **********************************************************************
# Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
# All rights reserved.
#
# For the licensing terms see LICENSE.
# **********************************************************************
"""
NucleusDbEditor.py -- view and edit the nuclei of the Glauber Monte Carlo (DB/Glauber/NucleusDb.xml).

Usage (from anywhere; the script locates CAP from its own position):
    python3 NucleusDbEditor.py [file.xml]      # open the GUI (default: <CAP>/DB/Glauber/NucleusDb.xml)
    python3 NucleusDbEditor.py --check [FILE]  # load FILE, report problems and exit
    python3 NucleusDbEditor.py --list [FILE]   # print the nuclei as a table and exit

The table shows one nucleus per row; every attribute of the file is a column. Double-click a cell to edit it:
Return or a click elsewhere keeps the new value, Escape cancels, Tab moves to the next cell. Values are checked
against the type of the field (integers for A, Z and the radial type, reals for the lengths and the deformation and
reweighting parameters). An empty optional cell takes the default of GlauberDb: the neutron density is that of the
protons, beta2, beta4, r0, r1, r2, a and b are 0, and maxRadius is 3 x 1.2 A^(1/3) fm.

Columns that are not stored in the file:
  * N: number of neutrons, A - Z;
  * Density: the name of the radial type (see NuclearDistribution).

Rows with a problem (see File > Check database) are shown in red: duplicate names, Z larger than A, unknown radial
types, or parameters missing for the radial type (deformations for types 8 and 14, neutron parameters for types 11
and 13, reweighting parameters for types 12 and 14). The check also warns about the radial types that
NuclearDistribution does not compute yet (all but 0, 1, 2 and 9); --check exits with 1 for problems only.

Filters (top): a part of the name (or of the comment) and the radial type. Clicking a column title sorts the nuclei
by that column (click again to reverse; View > File order restores the order of the file, which is the order used
when saving). View > Plot densities draws the radial densities of the selected nuclei (normalized to 1 at r = 0
for the Fermi and Gaussian forms; deformed nuclei along and perpendicular to their axis).

File menu: Open, Save, Save As (the file written keeps the comment at its top and the format of CAP's nucleus
database). Edit menu: add, duplicate, delete and move nuclei.

Requires Python 3.7+ with tkinter (on macOS with Homebrew Python: brew install python-tk).
"""

import math
import os
import re
import sys
import xml.etree.ElementTree as ET
from xml.sax.saxutils import escape

CAP_DIR = os.path.dirname(os.path.abspath(__file__))
DEFAULT_FILE = os.path.join(CAP_DIR, "DB", "Glauber", "NucleusDb.xml")

#: Attributes of a nucleus, in the order of the file.
FIELDS = ["name", "A", "Z", "radialType", "radius", "skin", "w", "neutronRadius", "neutronSkin", "neutronW",
          "beta2", "beta4", "r0", "r1", "r2", "a", "b", "maxRadius", "comment"]
REQUIRED = ["name", "A", "Z", "radialType"]
INT_FIELDS = {"A", "Z", "radialType"}
FLOAT_FIELDS = {"radius", "skin", "w", "neutronRadius", "neutronSkin", "neutronW", "beta2", "beta4", "r0", "r1",
                "r2", "a", "b", "maxRadius"}
NEUTRON_FIELDS = ("neutronRadius", "neutronSkin", "neutronW")
FIELD_LABELS = {"A": "A", "Z": "Z", "radialType": "Type", "radius": "Radius", "skin": "Skin", "w": "w",
                "neutronRadius": "n Radius", "neutronSkin": "n Skin", "neutronW": "n w", "beta2": "\u03b22",
                "beta4": "\u03b24", "r0": "r0", "r1": "r1", "r2": "r2", "a": "a", "b": "b", "maxRadius": "Max r",
                "comment": "Comment"}

#: Radial types of NuclearDistribution.
RADIAL_TYPES = {
    0: "exponential",
    1: "3pF (Fermi)",
    2: "3pG (Gaussian)",
    3: "Hulthen",
    4: "Hulthen, constrained",
    5: "ellipsoid",
    6: "from file",
    7: "deformed (box)",
    8: "deformed (\u03b22, \u03b24)",
    9: "proton Gaussian",
    10: "proton double Gaussian",
    11: "3pF, p and n",
    12: "3pF reweighted",
    13: "3pF p and n, reweighted",
    14: "deformed, reweighted",
    15: "harmonic oscillator",
}
DEFORMED_TYPES = {8, 14}
NEUTRON_TYPES = {11, 13}
REWEIGHTED_TYPES = {12, 14}              # type 13 has its reweighting parameters built in
#: Radial types NuclearDistribution::evaluate() computes (the others are placeholders there: see --check).
IMPLEMENTED_TYPES = {0, 1, 2, 9}

COLORS = {"row_bg": "white", "row_fg": "black", "alt_bg": "#f0f0f0", "bad_fg": "#b00000",
          "selected_bg": "#2f6db5", "selected_fg": "white"}
#: Characters escaped in attribute values (as CAP's XmlWriter does), besides & < >.
ENTITIES = {'"': "&quot;", "'": "&apos;"}
INT_RE = re.compile(r"[+-]?\d+")
INT_PARTIAL = re.compile(r"[+-]?\d*")
FLOAT_PARTIAL = re.compile(r"[+-]?\d*\.?\d*([eE][+-]?\d*)?")
#: Comment written at the top of a new file (a file read keeps its own).
DEFAULT_HEADER = """<!--
  Nuclei of the Glauber Monte Carlo (CAP::GlauberDb, models/Glauber), edited with NucleusDbEditor.py.
  Attributes (lengths in fm): name, A, Z, radialType (see NuclearDistribution), radius, skin, w (proton density),
  neutronRadius, neutronSkin, neutronW (default: those of the protons), beta2, beta4, r0, r1, r2, a, b (default 0),
  maxRadius (default 3 x 1.2 A^(1/3)), comment.
-->"""


# ------------------------------------------------------------------------------------------------------------------
# Model (no GUI)
# ------------------------------------------------------------------------------------------------------------------

class Nucleus:
    def __init__(self, attributes):
        self.attributes = dict(attributes)      # field -> text, in the order of the file

    @property
    def name(self):
        return self.attributes.get("name", "")

    def get(self, field, default=""):
        return self.attributes.get(field, default)

    def number(self, field, default=0.0):
        return to_float(self.attributes.get(field), default)

    def neutrons(self):
        try:
            return int(self.get("A")) - int(self.get("Z"))
        except ValueError:
            return None

    def radial_type(self):
        try:
            return int(self.get("radialType"))
        except ValueError:
            return None

    def max_radius(self):
        """Largest radius of the nucleons, as GlauberDb sets it."""
        if to_float(self.get("maxRadius"), 0.0) > 0:
            return to_float(self.get("maxRadius"))
        return 3.0 * 1.2 * max(1.0, self.number("A", 1.0)) ** 0.33333


class NucleusDb:
    def __init__(self):
        self.header = DEFAULT_HEADER
        self.root_tag = "CAP"
        self.root_attributes = {"version": "1", "type": "glauberNucleusDb"}
        self.db_tag = "NUCLEUS_DB"
        self.nuclei = []
        self.fields = list(FIELDS)

    @staticmethod
    def read(path):
        """Read a nucleus database file. Raises ValueError for files that are not nucleus databases."""
        try:
            with open(path, encoding="utf-8") as f:
                text = f.read()
            root = ET.fromstring(text)
        except ET.ParseError as e:
            raise ValueError("%s is not a valid XML file: %s" % (path, e))
        db = NucleusDb()
        # the comments before the root element are kept (ElementTree drops them)
        start = text.find("<" + root.tag)
        prolog = re.sub(r"<\?xml[^>]*\?>", "", text[:start] if start >= 0 else "").strip()
        db.header = prolog
        db.root_tag = root.tag
        db.root_attributes = dict(root.attrib)
        container = root if root.tag == "NUCLEUS_DB" else root.find("NUCLEUS_DB")
        if container is None:
            container = root
        db.db_tag = container.tag
        fields = []
        for element in container.iter("NUCLEUS"):
            fields += [k for k in element.attrib if k not in fields]
            db.nuclei.append(Nucleus(element.attrib))
        if not db.nuclei:
            raise ValueError("%s contains no NUCLEUS element." % path)
        db.fields = [f for f in FIELDS] + [f for f in fields if f not in FIELDS]
        return db

    def text(self):
        """The database in the format of CAP's nucleus database file."""
        def attrs(n):
            keys = [f for f in self.fields if n.attributes.get(f, "") != "" or f in REQUIRED]
            keys += [k for k in n.attributes if k not in keys and n.attributes[k] != ""]
            return " ".join('%s="%s"' % (k, escape(str(n.attributes.get(k, "")), ENTITIES)) for k in keys)
        lines = ['<?xml version="1.0" encoding="UTF-8"?>']
        if self.header:
            for body in re.findall(r"<!--(.*?)-->", self.header, re.S):
                if "--" in body:
                    raise ValueError("a comment at the top of the file holds \"--\", which XML forbids (CAP could "
                                     "not read the file)")
            lines.append(self.header)
        nested = self.root_tag != self.db_tag
        if nested:
            root_attrs = " ".join('%s="%s"' % (k, escape(v, ENTITIES)) for k, v in self.root_attributes.items())
            lines.append("<%s%s>" % (self.root_tag, " " + root_attrs if root_attrs else ""))
        pad = "  " if nested else ""
        lines.append(pad + "<%s>" % self.db_tag)
        for n in self.nuclei:
            lines.append(pad + "  <NUCLEUS %s/>" % attrs(n))
        lines.append(pad + "</%s>" % self.db_tag)
        if nested:
            lines.append("</%s>" % self.root_tag)
        return "\n".join(lines) + "\n"

    def write(self, path):
        tmp = path + ".tmp"
        with open(tmp, "w", encoding="utf-8") as f:
            f.write(self.text())
        os.replace(tmp, path)

    def names(self):
        return {n.name for n in self.nuclei}

    def find(self, name):
        for n in self.nuclei:
            if n.name == name:
                return n
        return None

    def problems(self):
        """List of problems (the nucleus, then what is wrong)."""
        out = []
        seen = set()
        for n in self.nuclei:
            out += ["%s: %s" % (n.name or "(no name)", p) for p in nucleus_problems(n, seen)]
            seen.add(n.name)
        return out

    def warnings(self):
        """List of warnings (the nucleus, then the warning)."""
        return ["%s: %s" % (n.name, w) for n in self.nuclei for w in nucleus_warnings(n)]


def nucleus_problems(n, seen=()):
    """Problems of one nucleus; seen: names of the nuclei before it."""
    out = []
    if not n.name:
        out.append("no name")
    elif re.search(r"\s", n.name):
        out.append("the name has spaces")
    if n.name in seen:
        out.append("duplicate name")
    for f in REQUIRED[1:]:
        if INT_RE.fullmatch(n.get(f).strip()) is None:
            out.append("%s is not an integer (%r)" % (f, n.get(f)))
    for f in FLOAT_FIELDS:
        v = n.get(f)
        if v != "" and not valid(f, v):
            out.append("%s is not a number (%r)" % (f, v))
    if out:
        return out
    A, Z, t = int(n.get("A")), int(n.get("Z")), int(n.get("radialType"))
    if A < 1:
        out.append("A < 1")
    if Z < 0 or Z > A:
        out.append("Z = %d not in [0, A = %d]" % (Z, A))
    if t not in RADIAL_TYPES:
        out.append("unknown radial type %d" % t)
    if t in DEFORMED_TYPES and n.number("beta2") == 0 and n.number("beta4") == 0:
        out.append("deformed type %d with beta2 = beta4 = 0" % t)
    if t not in DEFORMED_TYPES and (n.number("beta2") != 0 or n.number("beta4") != 0):
        out.append("beta2 or beta4 set, but type %d ignores them (deformed types: 8, 14)" % t)
    if t in NEUTRON_TYPES and not any(n.get(f) != "" for f in NEUTRON_FIELDS):
        out.append("type %d without neutron parameters" % t)
    if t in REWEIGHTED_TYPES and n.number("r0") == 0:
        out.append("reweighted type %d with r0 = 0" % t)
    if t in (1, 2, 5, 8, 11, 12, 13, 14) and n.number("radius") <= 0:
        out.append("radius <= 0")
    if t in (1, 5, 8, 11, 12, 13, 14) and n.number("skin") <= 0:
        out.append("skin <= 0")
    return out


def nucleus_warnings(n):
    """Warnings of one nucleus: what the file asks that the code does not do."""
    t = n.radial_type()
    if t in RADIAL_TYPES and t not in IMPLEMENTED_TYPES:
        return ["radial type %d (%s) is not implemented by NuclearDistribution::evaluate()" % (t, RADIAL_TYPES[t])]
    return []


def to_float(text, default=0.0):
    try:
        return float(text)
    except (TypeError, ValueError):
        return default


def field_type(field):
    if field in INT_FIELDS:
        return "int"
    if field in FLOAT_FIELDS:
        return "float"
    return "string"


def valid(field, text):
    t = field_type(field)
    if t == "int":
        return INT_RE.fullmatch(text.strip()) is not None
    if t == "float":
        if text.strip() == "" and field not in REQUIRED:
            return True
        try:
            float(text)
            return True
        except ValueError:
            return False
    if field == "name":
        return re.fullmatch(r"\S+", text) is not None
    return True


def partial(field, text):
    t = field_type(field)
    if t == "int":
        return INT_PARTIAL.fullmatch(text) is not None
    if t == "float":
        return FLOAT_PARTIAL.fullmatch(text) is not None
    return True


def type_label(n):
    t = n.radial_type()
    return RADIAL_TYPES.get(t, "unknown") if t is not None else ""


def density(n, r, cos_theta=0.0, neutron=False):
    """Radial density of a nucleus at r (fm), for the common forms of NuclearDistribution (unnormalized: 1 at r = 0
    for the Fermi and Gaussian forms); None if the type is not drawn. For deformed types, cos_theta is the cosine of
    the angle to the symmetry axis."""
    t = n.radial_type()
    use_n = neutron and any(n.get(f) != "" for f in NEUTRON_FIELDS)
    R = n.number("neutronRadius" if use_n else "radius")
    a = n.number("neutronSkin" if use_n else "skin")
    w = n.number("neutronW" if use_n else "w")
    if t in (1, 5, 11, 12, 13, 8, 14):
        if t in (8, 14):
            y2 = 0.315 * (3 * cos_theta ** 2 - 1)
            y4 = 0.105 * (35 * cos_theta ** 4 - 30 * cos_theta ** 2 + 3)
            R = R * (1 + n.number("beta2") * y2 + n.number("beta4") * y4)
            w = 0.0
        if a <= 0 or R <= 0:
            return None
        if t == 5:
            w = 0.0
        x = (r - R) / a
        value = (1 + w * (r / R) ** 2) / (1 + math.exp(min(x, 700)))
        if t in (12, 13, 14):
            weight = n.number("r0") + n.number("r1") * r + n.number("r2") * r * r
            value = value / weight if weight > 0 else value
        return value
    if t == 2:
        if a <= 0 or R <= 0:
            return None
        return (1 + w * (r / R) ** 2) / (1 + math.exp(min((r * r - R * R) / (a * a), 700)))
    if t == 0:
        return math.exp(-r / R) if R > 0 else None
    if t == 9:
        return math.exp(-0.5 * (r / R) ** 2) if R > 0 else None
    if t in (3, 4):
        alpha, beta = R, a          # Hulthen: (exp(-alpha r) - exp(-beta r))^2 / r^2 (radius and skin, 1/fm)
        if alpha <= 0 or beta <= 0 or r <= 0:
            return None
        return ((math.exp(-alpha * r) - math.exp(-beta * r)) / r) ** 2
    if t == 15:
        if R <= 0:
            return None
        return (1 + a * (r / R) ** 2) * math.exp(-(r / R) ** 2)
    return None


def table(db):
    """The nuclei as a text table (--list)."""
    columns = ["name", "A", "Z", "N", "radialType", "Density", "radius", "skin", "w", "beta2", "beta4", "comment"]
    rows = []
    for n in db.nuclei:
        row = []
        for c in columns:
            if c == "N":
                row.append(str(n.neutrons()) if n.neutrons() is not None else "")
            elif c == "Density":
                row.append(type_label(n))
            else:
                row.append(n.get(c))
        rows.append(row)
    widths = [max(len(c), *(len(r[k]) for r in rows)) for k, c in enumerate(columns)]
    lines = ["  ".join(c.ljust(widths[k]) for k, c in enumerate(columns))]
    lines.append("  ".join("-" * w for w in widths))
    lines += ["  ".join(v.ljust(widths[k]) for k, v in enumerate(r)) for r in rows]
    return "\n".join(line.rstrip() for line in lines)


# ------------------------------------------------------------------------------------------------------------------
# GUI
# ------------------------------------------------------------------------------------------------------------------

def run_gui(path=None, hook=None):
    import tkinter as tk
    from tkinter import ttk, filedialog, messagebox

    MOD = "Command" if sys.platform == "darwin" else "Control"
    MOD_TEXT = "Cmd" if sys.platform == "darwin" else "Ctrl"
    TYPE_CHOICES = ["%d  %s" % (k, v) for k, v in sorted(RADIAL_TYPES.items())]

    class Editor:
        def __init__(self, root):
            self.root = root
            self.db = None
            self.path = ""
            self.dirty = False
            self.items = {}          # tree item -> nucleus
            self.nucleus_item = {}   # id(nucleus) -> tree item
            self.sort = None         # (column, reverse) or None for file order
            self.editor = None
            root.geometry("1450x760")
            root.minsize(900, 420)
            style = ttk.Style()
            if "clam" in style.theme_names() and sys.platform != "darwin":
                style.theme_use("clam")
            style.configure("Treeview", rowheight=22, background=COLORS["row_bg"], fieldbackground=COLORS["row_bg"],
                            foreground=COLORS["row_fg"])
            style.map("Treeview", background=[("selected", COLORS["selected_bg"])],
                      foreground=[("selected", COLORS["selected_fg"])])
            self._make_menu()
            statusbar = ttk.Frame(root, relief=tk.SUNKEN)
            statusbar.pack(fill=tk.X, side=tk.BOTTOM)
            self.status = tk.StringVar(value="")
            self.count = ttk.Label(statusbar, text="", foreground="gray35")
            self.count.pack(side=tk.RIGHT, padx=6)
            ttk.Label(statusbar, textvariable=self.status, anchor="w").pack(side=tk.LEFT, fill=tk.X, expand=True)
            self._make_filters()
            self._make_table()
            root.protocol("WM_DELETE_WINDOW", self.quit)
            root.report_callback_exception = self._callback_error

        # ------------------------------------------------------------------------------------------ menus
        def _make_menu(self):
            menubar = tk.Menu(self.root)
            m = tk.Menu(menubar, tearoff=0)
            m.add_command(label="Open\u2026", accelerator=MOD_TEXT + "+O", command=self.open)
            m.add_command(label="Save", accelerator=MOD_TEXT + "+S", command=self.save)
            m.add_command(label="Save As\u2026", accelerator="Shift+" + MOD_TEXT + "+S", command=self.save_as)
            m.add_separator()
            m.add_command(label="Check database", command=self.show_problems)
            m.add_separator()
            m.add_command(label="Quit", accelerator=MOD_TEXT + "+Q", command=self.quit)
            menubar.add_cascade(label="File", menu=m)
            m = tk.Menu(menubar, tearoff=0)
            m.add_command(label="Add nucleus", command=self.add_nucleus)
            m.add_command(label="Duplicate nucleus", command=self.duplicate_nucleus)
            m.add_separator()
            m.add_command(label="Move up", accelerator=MOD_TEXT + "+Up", command=lambda: self.move(-1))
            m.add_command(label="Move down", accelerator=MOD_TEXT + "+Down", command=lambda: self.move(1))
            m.add_separator()
            m.add_command(label="Delete selected nuclei", accelerator="Delete", command=self.delete_selected)
            menubar.add_cascade(label="Edit", menu=m)
            m = tk.Menu(menubar, tearoff=0)
            m.add_command(label="Plot densities of the selected nuclei", command=self.plot_densities)
            m.add_command(label="File order", command=self.file_order)
            menubar.add_cascade(label="View", menu=m)
            self.root.configure(menu=menubar)
            self.root.bind_all("<%s-o>" % MOD, lambda e: self.open())
            self.root.bind_all("<%s-s>" % MOD, lambda e: self.save())
            self.root.bind_all("<%s-S>" % MOD, lambda e: self.save_as())
            self.root.bind_all("<%s-q>" % MOD, lambda e: self.quit())

        def _make_filters(self):
            bar = ttk.Frame(self.root, padding=(6, 6, 6, 4))
            bar.pack(fill=tk.X)
            ttk.Label(bar, text="Name contains:").pack(side=tk.LEFT)
            self.name_filter = tk.StringVar()
            ttk.Entry(bar, textvariable=self.name_filter, width=18).pack(side=tk.LEFT, padx=(4, 14))
            ttk.Label(bar, text="Radial type:").pack(side=tk.LEFT)
            self.type_filter = tk.StringVar(value="All")
            box = ttk.Combobox(bar, textvariable=self.type_filter, values=["All"] + TYPE_CHOICES, state="readonly",
                               width=28)
            box.pack(side=tk.LEFT, padx=4)
            box.bind("<<ComboboxSelected>>", lambda e: self.refresh())
            self.problems_only = tk.BooleanVar(value=False)
            ttk.Checkbutton(bar, text="With problems only", variable=self.problems_only,
                            command=self.refresh).pack(side=tk.LEFT, padx=10)
            self.name_filter.trace_add("write", lambda *a: self.refresh())

        def _make_table(self):
            frame = ttk.Frame(self.root)
            frame.pack(fill=tk.BOTH, expand=True, padx=6)
            self.tree = ttk.Treeview(frame, selectmode="extended", show="tree headings")
            ys = ttk.Scrollbar(frame, orient=tk.VERTICAL, command=self.tree.yview)
            xs = ttk.Scrollbar(frame, orient=tk.HORIZONTAL, command=self.tree.xview)
            self.tree.configure(yscrollcommand=ys.set, xscrollcommand=xs.set)
            self.tree.grid(row=0, column=0, sticky="nsew")
            ys.grid(row=0, column=1, sticky="ns")
            xs.grid(row=1, column=0, sticky="ew")
            frame.rowconfigure(0, weight=1)
            frame.columnconfigure(0, weight=1)
            self.tree.tag_configure("alt", background=COLORS["alt_bg"])
            self.tree.tag_configure("bad", foreground=COLORS["bad_fg"])
            self.tree.bind("<Double-Button-1>", self._on_double_click)
            self.tree.bind("<Delete>", lambda e: self.delete_selected())
            self.tree.bind("<BackSpace>", lambda e: self.delete_selected())
            self.tree.bind("<%s-Up>" % MOD, lambda e: (self.move(-1), "break")[1])
            self.tree.bind("<%s-Down>" % MOD, lambda e: (self.move(1), "break")[1])
            self.tree.bind("<<TreeviewSelect>>", lambda e: self._show_selected_problems())
            for seq in ("<MouseWheel>", "<Button-4>", "<Button-5>", "<Configure>"):
                self.tree.bind(seq, lambda e: self._close_editor(True), add="+")

        def _setup_columns(self):
            """Columns: the fields of the file but the name (the first column), with N after Z and Density after the
            radial type."""
            self.columns = []
            for f in self.db.fields:
                if f == "name":
                    continue
                self.columns.append("f:" + f)
                if f == "Z":
                    self.columns.append("N")
                if f == "radialType":
                    self.columns.append("Density")
            self.tree.configure(columns=self.columns, displaycolumns=self.columns)
            self.tree.heading("#0", text="Nucleus", command=lambda: self._sort_by("#0"))
            self.tree.column("#0", width=110, minwidth=70, stretch=False)
            widths = {"comment": 380, "radialType": 48, "A": 48, "Z": 44, "N": 44, "Density": 160}
            for c in self.columns:
                field = c[2:] if c.startswith("f:") else c
                label = FIELD_LABELS.get(field, field)
                self.tree.heading(c, text=label, command=lambda c=c: self._sort_by(c))
                anchor = "w" if field in ("comment", "Density") or (c.startswith("f:") and field_type(field) == "string") else "e"
                self.tree.column(c, width=widths.get(field, 70), minwidth=36, anchor=anchor,
                                 stretch=field == "comment")

        # ------------------------------------------------------------------------------------------ table
        def visible(self, n):
            text = self.name_filter.get().strip().lower()
            if text and text not in n.name.lower() and text not in n.get("comment").lower():
                return False
            choice = self.type_filter.get()
            if choice != "All" and str(n.radial_type()) != choice.split()[0]:
                return False
            if self.problems_only.get() and not nucleus_problems(n, self._names_before(n)):
                return False
            return True

        def _names_before(self, n):
            names = set()
            for m in self.db.nuclei:
                if m is n:
                    break
                names.add(m.name)
            return names

        def ordered(self):
            nuclei = list(self.db.nuclei)
            if self.sort is None:
                return nuclei
            column, reverse = self.sort
            if column == "#0":
                key = lambda n: n.name.lower()
            elif column == "N":
                key = lambda n: n.neutrons() if n.neutrons() is not None else -1
            elif column == "Density":
                key = lambda n: type_label(n)
            else:
                field = column[2:]
                if field_type(field) in ("int", "float"):
                    key = lambda n: to_float(n.get(field), float("-inf"))
                else:
                    key = lambda n: n.get(field).lower()
            return sorted(nuclei, key=key, reverse=reverse)

        def refresh(self):
            """Rebuild the table from the database and the filters."""
            if self.db is None:
                return
            self._close_editor(True)
            selected = {id(self.items[i]) for i in self.tree.selection() if i in self.items}
            self.tree.delete(*self.tree.get_children(""))
            self.items.clear()
            self.nucleus_item.clear()
            shown, keep = 0, []
            for n in self.ordered():
                if not self.visible(n):
                    continue
                item = self.tree.insert("", tk.END)
                self.items[item] = n
                self.nucleus_item[id(n)] = item
                self._fill(item, shown)
                if id(n) in selected:
                    keep.append(item)
                shown += 1
            if keep:
                self.tree.selection_set(keep)
            n_bad = sum(1 for k, n in enumerate(self.db.nuclei)
                        if nucleus_problems(n, {m.name for m in self.db.nuclei[:k]}))
            self.count.configure(text="Showing %d of %d nuclei%s" % (
                shown, len(self.db.nuclei), " (%d with problems)" % n_bad if n_bad else ""))

        def _fill(self, item, index=None):
            n = self.items[item]
            values = []
            for c in self.columns:
                if c == "N":
                    values.append(n.neutrons() if n.neutrons() is not None else "")
                elif c == "Density":
                    values.append(type_label(n))
                else:
                    values.append(n.get(c[2:]))
            if index is None:
                index = self.tree.index(item)
            tags = ["alt"] if index % 2 else []
            if nucleus_problems(n, self._names_before(n)):
                tags.append("bad")
            self.tree.item(item, text=n.name, values=values, tags=tags)

        def _show_selected_problems(self):
            for item in self.tree.selection():
                n = self.items.get(item)
                if n is not None:
                    problems = nucleus_problems(n, self._names_before(n)) + nucleus_warnings(n)
                    self.status.set("%s: %s" % (n.name, "; ".join(problems)) if problems else
                                    "%s: %s, %d protons and %s neutrons" % (n.name, type_label(n), int(n.get("Z", 0) or 0),
                                                                             n.neutrons()))
                    return

        def _sort_by(self, column):
            if self.sort and self.sort[0] == column:
                self.sort = (column, not self.sort[1])
            else:
                self.sort = (column, False)
            self.refresh()
            self.status.set("Sorted by %s%s (View > File order restores the order of the file)"
                            % (self.tree.heading(column, "text"), ", descending" if self.sort[1] else ""))

        def file_order(self):
            self.sort = None
            self.refresh()

        # ------------------------------------------------------------------------------------------ editing
        def _field_of(self, column):
            """Field edited in a column, or None if the column is computed."""
            if column == "#0":
                return "name"
            c = self.tree.column(column, "id")
            return c[2:] if c.startswith("f:") else None

        def _on_double_click(self, event):
            if self.tree.identify_region(event.x, event.y) not in ("cell", "tree"):
                return
            item = self.tree.identify_row(event.y)
            column = self.tree.identify_column(event.x)
            if item and item in self.items:
                if self._field_of(column) is None:
                    self.status.set("N and Density are computed (from A and Z, and from the radial type).")
                    return
                self.edit_cell(item, column)
                return "break"

        def edit_cell(self, item, column):
            self._close_editor(True)
            if item not in self.items or not self.tree.exists(item):
                return
            field = self._field_of(column)
            if field is None:
                return
            self.tree.see(item)
            self.root.update_idletasks()
            box = self.tree.bbox(item, column)
            if not box:
                return
            x, y, w, h = box
            n = self.items[item]
            value = n.get(field)
            if field == "radialType":
                t = n.radial_type()
                var = tk.StringVar(value="%d  %s" % (t, RADIAL_TYPES[t]) if t in RADIAL_TYPES else value)
                widget = ttk.Combobox(self.tree, textvariable=var, values=TYPE_CHOICES, state="readonly")
                widget.bind("<<ComboboxSelected>>", lambda e: self._commit())
                w = max(w, 220)
            else:
                var = tk.StringVar(value=value)
                widget = ttk.Entry(self.tree, textvariable=var)
                if field_type(field) in ("int", "float"):
                    check = (self.root.register(lambda text: partial(field, text)), "%P")
                    widget.configure(validate="key", validatecommand=check)
                widget.select_range(0, tk.END)
                if column == "#0":
                    x, w = x + 4, max(w - 4, 100)
                w = max(w, 140 if field_type(field) == "string" else w)
            widget.place(x=x, y=y, width=w, height=h)
            widget.focus_set()
            widget.bind("<Return>", lambda e: self._commit())
            widget.bind("<KP_Enter>", lambda e: self._commit())
            widget.bind("<Escape>", lambda e: self._close_editor(False))
            widget.bind("<Tab>", lambda e: self._commit_and_move(1))
            widget.bind("<Shift-Tab>", lambda e: self._commit_and_move(-1))
            widget.bind("<ISO_Left_Tab>", lambda e: self._commit_and_move(-1))
            widget.bind("<FocusOut>", lambda e: self.root.after(1, self._commit_if_unfocused))
            self.editor = dict(widget=widget, var=var, item=item, column=column, field=field, n=n)

        def _commit_if_unfocused(self):
            if self.editor and self.root.focus_get() is not self.editor["widget"]:
                self._commit()

        def _commit_and_move(self, step):
            if not self.editor:
                return "break"
            item, column = self.editor["item"], self.editor["column"]
            if not self._commit():
                return "break"
            columns = ["#0"] + ["#%d" % (k + 1) for k in range(len(self.columns))]
            k = columns.index(column) if column in columns else 0
            while 0 <= k + step < len(columns):
                k += step
                if self._field_of(columns[k]) is not None:
                    self.root.after(1, lambda: self.edit_cell(item, columns[k]))
                    break
            return "break"

        def _close_editor(self, keep):
            if self.editor:
                if keep:
                    self._commit()
                else:
                    self._discard_editor()

        def _discard_editor(self):
            if self.editor:
                widget = self.editor["widget"]
                self.editor = None
                widget.destroy()

        def _commit(self):
            """Store the value of the cell editor. Returns False (editor kept open) for an invalid value."""
            e = self.editor
            if not e:
                return True
            field, new = e["field"], e["var"].get().strip()
            if field == "radialType":
                new = new.split()[0] if new else ""
            n = e["n"]
            old = n.get(field)
            if new == old:
                self._discard_editor()
                return True
            if field in REQUIRED and new == "":
                self.status.set("%s cannot be empty (Escape cancels)" % field)
                self.root.bell()
                return False
            if not valid(field, new):
                self.status.set("Invalid %s value for %s: '%s' (Escape cancels)" % (field_type(field), field, new))
                self.root.bell()
                return False
            if field == "name" and new in self.db.names():
                self.status.set("A nucleus named %s already exists." % new)
                self.root.bell()
                return False
            self._discard_editor()
            if new == "":
                n.attributes.pop(field, None)
            else:
                n.attributes[field] = new
            self._changed()
            self._fill(e["item"])
            problems = nucleus_problems(n, self._names_before(n))
            self.status.set("%s: %s = %s%s" % (n.name, field, new if new else "(default)",
                                                 "  \u2014  " + "; ".join(problems) if problems else ""))
            return True

        # ------------------------------------------------------------------------------------------ edit menu
        def _selected(self):
            return [self.items[i] for i in self.tree.selection() if i in self.items]

        def _insert(self, new, after):
            index = self.db.nuclei.index(after) + 1 if after in self.db.nuclei else len(self.db.nuclei)
            self.db.nuclei.insert(index, new)
            self.name_filter.set("")
            self.type_filter.set("All")
            self.problems_only.set(False)
            self._changed()
            self.refresh()
            item = self.nucleus_item[id(new)]
            self.tree.selection_set(item)
            self.tree.see(item)
            self.edit_cell(item, "#0")

        def _unique(self, base):
            names = self.db.names()
            name, k = base, 2
            while name in names:
                name, k = "%s%d" % (base, k), k + 1
            return name

        def add_nucleus(self):
            selected = self._selected()
            self._insert(Nucleus({"name": self._unique("new"), "A": "1", "Z": "1", "radialType": "1",
                                  "radius": "1", "skin": "0.5", "w": "0"}), selected[0] if selected else None)

        def duplicate_nucleus(self):
            selected = self._selected()
            if not selected:
                self.status.set("Select the nucleus to duplicate.")
                return
            n = selected[0]
            self._insert(Nucleus(dict(n.attributes, name=self._unique(n.name + "_copy"))), n)

        def move(self, step):
            if self.editor:
                return
            if self.sort is not None:
                self.status.set("Moving changes the order of the file: View > File order first.")
                return
            selected = self._selected()
            if not selected:
                return
            nuclei = self.db.nuclei
            indices = sorted(nuclei.index(n) for n in selected)
            if (step < 0 and indices[0] == 0) or (step > 0 and indices[-1] == len(nuclei) - 1):
                return
            for k in (indices if step < 0 else reversed(indices)):
                nuclei[k], nuclei[k + step] = nuclei[k + step], nuclei[k]
            self._changed()
            self.refresh()
            self.tree.selection_set([self.nucleus_item[id(n)] for n in selected if id(n) in self.nucleus_item])

        def delete_selected(self):
            if self.editor:
                return
            selected = self._selected()
            if not selected:
                return
            text = "%d nucle%s: %s" % (len(selected), "us" if len(selected) == 1 else "i",
                                       ", ".join(n.name for n in selected[:8]) + (", \u2026" if len(selected) > 8 else ""))
            if not messagebox.askyesno("Delete", "Delete %s?" % text):
                return
            for n in selected:
                self.db.nuclei.remove(n)
            self._changed()
            self.refresh()
            self.status.set("Deleted %s." % text)

        # ------------------------------------------------------------------------------------------ densities
        def plot_densities(self):
            self._close_editor(True)
            selected = self._selected()
            if not selected:
                self.status.set("Select the nuclei whose densities are drawn.")
                return
            curves = []
            for n in selected:
                if density(n, 0.5) is None:
                    continue
                if n.radial_type() in DEFORMED_TYPES:
                    curves.append((n.name + " (axis)", n, 1.0, False))
                    curves.append((n.name + " (equator)", n, 0.0, False))
                else:
                    curves.append((n.name, n, 0.0, False))
                    if n.radial_type() in NEUTRON_TYPES:
                        curves.append((n.name + " (neutrons)", n, 0.0, True))
            if not curves:
                self.status.set("No density to draw for these radial types.")
                return
            window = tk.Toplevel(self.root)
            window.title("Radial densities")
            width, height = 760, 480
            canvas = tk.Canvas(window, width=width, height=height, background="white")
            canvas.pack(fill=tk.BOTH, expand=True)
            r_max = max(n.max_radius() for _, n, _, _ in curves) * 0.6
            samples = [r_max * k / 300.0 for k in range(301)]
            data = []
            for label, n, c, neutron in curves:
                values = [density(n, r, c, neutron) or 0.0 for r in samples]
                data.append((label, values))
            y_max = max(max(v) for _, v in data) * 1.08 or 1.0
            left, right, top, bottom = 70, 200, 20, 50
            def X(r):
                return left + (width - left - right) * r / r_max
            def Y(v):
                return height - bottom - (height - top - bottom) * v / y_max
            canvas.create_rectangle(left, top, width - right, height - bottom)
            for k in range(6):
                r = r_max * k / 5
                canvas.create_line(X(r), height - bottom, X(r), height - bottom + 5)
                canvas.create_text(X(r), height - bottom + 16, text="%.1f" % r)
                v = y_max * k / 5
                canvas.create_line(left - 5, Y(v), left, Y(v))
                canvas.create_text(left - 8, Y(v), text="%.2g" % v, anchor="e")
            canvas.create_text((left + width - right) / 2, height - 14, text="r (fm)")
            palette = ["#1f5fb4", "#c0392b", "#2e8b57", "#8e44ad", "#d35400", "#16a085", "#7f8c8d", "#b7950b"]
            for k, (label, values) in enumerate(data):
                color = palette[k % len(palette)]
                points = []
                for r, v in zip(samples, values):
                    points += [X(r), Y(v)]
                canvas.create_line(*points, fill=color, width=2)
                canvas.create_line(width - right + 12, top + 14 + 20 * k, width - right + 36, top + 14 + 20 * k,
                                   fill=color, width=2)
                canvas.create_text(width - right + 42, top + 14 + 20 * k, text=label, anchor="w")
            self.density_window = window

        # ------------------------------------------------------------------------------------------ files
        def _dialog(self, kind_, **options):
            if sys.platform == "darwin":          # Tk's native panel can abort when given type filters
                options.pop("filetypes", None)
            options.setdefault("parent", self.root)
            if options.get("initialdir") and not os.path.isdir(options["initialdir"]):
                options["initialdir"] = CAP_DIR
            try:
                return getattr(filedialog, kind_)(**options) or ""
            except tk.TclError as e:
                messagebox.showerror("File dialog", str(e))
                return ""

        def _changed(self):
            if not self.dirty:
                self.dirty = True
                self._update_title()

        def _update_title(self):
            name = os.path.basename(self.path) if self.path else "no file"
            self.root.title("CAP Nucleus DB \u2014 %s%s" % (name, " *" if self.dirty else ""))

        def _confirm_discard(self):
            if not self.dirty:
                return True
            answer = messagebox.askyesnocancel("Unsaved changes", "Save the changes made to %s?"
                                               % os.path.basename(self.path))
            if answer is None:
                return False
            return self.save() if answer else True

        def open(self, path=None):
            self._close_editor(True)
            if not self._confirm_discard():
                return
            if not path:
                path = self._dialog("askopenfilename", title="Open a nucleus database",
                                    initialdir=os.path.dirname(self.path or DEFAULT_FILE),
                                    filetypes=[("XML files", "*.xml"), ("All files", "*")])
            if not path:
                return
            try:
                db = NucleusDb.read(path)
            except (ValueError, OSError) as e:
                messagebox.showerror("Open", str(e))
                return
            self.db, self.path, self.dirty, self.sort = db, path, False, None
            self._setup_columns()
            self.refresh()
            self._update_title()
            problems = db.problems()
            self.status.set("Read %s: %d nuclei%s" % (path, len(db.nuclei),
                            "; %d problem(s), see File > Check database" % len(problems) if problems else ""))

        def save(self):
            if not self.path:
                return self.save_as()
            return self._write(self.path)

        def save_as(self):
            if self.db is None:
                return False
            path = self._dialog("asksaveasfilename", title="Write the nucleus database to",
                                initialdir=os.path.dirname(self.path or DEFAULT_FILE),
                                initialfile=os.path.basename(self.path or "NucleusDb.xml"),
                                defaultextension=".xml", filetypes=[("XML files", "*.xml"), ("All files", "*")])
            if not path:
                return False
            if self._write(path):
                self.path = path
                self._update_title()
                return True
            return False

        def _write(self, path):
            self._close_editor(True)
            if self.editor:          # invalid value being edited
                return False
            try:
                self.db.write(path)
            except (OSError, ValueError) as e:
                messagebox.showerror("Save", "Cannot write %s:\n%s" % (path, e))
                return False
            self.dirty = False
            self._update_title()
            self.status.set("Wrote %s (%d nuclei)" % (path, len(self.db.nuclei)))
            return True

        def show_problems(self):
            if self.db is None:
                return
            self._close_editor(True)
            problems = self.db.problems()
            warnings = self.db.warnings()
            window = tk.Toplevel(self.root)
            window.title("Check of the nucleus database")
            window.geometry("760x420")
            text = tk.Text(window, wrap=tk.NONE, font=("Menlo", 11) if sys.platform == "darwin" else ("TkFixedFont",))
            sb = ttk.Scrollbar(window, orient=tk.VERTICAL, command=text.yview)
            text.configure(yscrollcommand=sb.set)
            sb.pack(side=tk.RIGHT, fill=tk.Y)
            text.pack(fill=tk.BOTH, expand=True)
            text.insert(tk.END, "\n".join(problems) if problems else "No problem found.")
            if warnings:
                text.insert(tk.END, "\n\n%d warning(s)\n\n" % len(warnings) + "\n".join(warnings))
            text.insert("1.0", "%d problem(s)\n\n" % len(problems))
            text.configure(state=tk.DISABLED)

        def quit(self):
            self._close_editor(True)
            if self._confirm_discard():
                self.root.destroy()

        def _callback_error(self, exc, value, tb):
            import traceback
            sys.stderr.write("".join(traceback.format_exception(exc, value, tb)))
            messagebox.showerror("NucleusDbEditor.py error", "%s: %s" % (exc.__name__, value))

    root = tk.Tk()
    editor = Editor(root)
    editor._update_title()
    root.after(50, lambda: editor.open(path) if path else editor.status.set("File > Open to read a database."))
    if hook:
        hook(editor, root)
    root.mainloop()


def main():
    args = sys.argv[1:]
    if "-h" in args or "--help" in args:
        print(__doc__)
        return 0
    if args and args[0] in ("--check", "--list"):
        path = args[1] if len(args) > 1 else DEFAULT_FILE
        try:
            db = NucleusDb.read(path)
        except (ValueError, OSError) as e:
            print(e, file=sys.stderr)
            return 1
        if args[0] == "--list":
            print(table(db))
            return 0
        problems = db.problems()
        warnings = db.warnings()
        for p in problems:
            print(p)
        for w in warnings:
            print("warning: " + w)
        types = {}
        for n in db.nuclei:
            types[type_label(n)] = types.get(type_label(n), 0) + 1
        print("%d nuclei (%s), %d problem(s), %d warning(s)" % (
            len(db.nuclei), ", ".join("%d %s" % (k, t) for t, k in sorted(types.items())), len(problems),
            len(warnings)))
        return 1 if problems else 0
    try:
        import tkinter  # noqa: F401
    except ImportError:
        print("NucleusDbEditor.py needs tkinter (macOS with Homebrew Python: brew install python-tk; "
              "Debian/Ubuntu: apt install python3-tk).", file=sys.stderr)
        return 1
    path = args[0] if args else (DEFAULT_FILE if os.path.exists(DEFAULT_FILE) else None)
    run_gui(path)
    return 0


if __name__ == "__main__":
    sys.exit(main())
