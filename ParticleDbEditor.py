#!/usr/bin/env python3
# **********************************************************************
# Copyright (C) 2019-2024, Claude Pruneau
# All rights reserved.
#
# For the licensing terms see LICENSE.
# **********************************************************************
"""
ParticleDbEditor.py -- view and edit the CAP particle database (DB/ParticleData/ParticleDb.xml).

Usage (from anywhere; the script locates CAP from its own position):
    python3 ParticleDbEditor.py [file.xml]      # open the GUI (default: <CAP>/DB/ParticleData/ParticleDb.xml)
    python3 ParticleDbEditor.py --check FILE    # load FILE, report problems (unknown decay products, duplicate
                                                # names or PDG codes) and exit

The table shows one particle per row, with its decays as rows under it (click the triangle, or use Expand all).
Every attribute of the file is a column. Double-click a cell to edit it: Return or a click elsewhere keeps the new
value, Escape cancels, Tab moves to the next cell. Values are checked against the type of the field (integers for
the PDG code and the quark content, reals for masses, widths, charges, spins, isospins and branching ratios,
true/false for Clebsch-Gordan). The decay products are particle names separated by spaces; names that are not in the
database are shown in red.

Columns that are not stored in the file:
  * Type: baryon, meson, lepton, quark, gauge boson or other, from the PDG code (or from the quark content when the
    code is not a standard PDG code);
  * the decay columns (BR, CG) are empty on particle rows, and the particle columns on decay rows.

Filters (top): a part of the name (or of the title), the kinds of particles shown, and charged / neutral.
Clicking a column title sorts the particles by that column (click again to reverse; View > File order restores the
order of the file, which is the order used when saving).

File menu: Open, Save, Save As (the file written keeps the format of CAP's particle database). Edit menu: add,
duplicate and delete particles and decays. Renaming a particle offers to update the decays that refer to it.

Requires Python 3.7+ with tkinter (on macOS with Homebrew Python: brew install python-tk).
"""

import os
import re
import sys
import xml.etree.ElementTree as ET
from xml.sax.saxutils import escape

CAP_DIR = os.path.dirname(os.path.abspath(__file__))
DEFAULT_FILE = os.path.join(CAP_DIR, "DB", "ParticleData", "ParticleDb.xml")

#: Types of the known attributes (others are strings).
INT_FIELDS = {"pdg", "quarks", "antiquarks", "leptons"}
FLOAT_FIELDS = {"charge", "mass", "massUncertainty", "width", "widthUncertainty", "spin", "isospin", "isospin3",
                "branchingRatio"}
BOOL_FIELDS = {"clebschGordan"}
#: Order of the particle and decay attributes when a new file is written (attributes found in a file are kept, in
#: the order of the file).
PARTICLE_FIELDS = ["name", "title", "pdg", "charge", "mass", "massUncertainty", "width", "widthUncertainty", "spin",
                   "isospin", "isospin3", "quarks", "antiquarks", "leptons"]
DECAY_FIELDS = ["branchingRatio", "clebschGordan", "products"]
FIELD_LABELS = {"title": "Title", "pdg": "PDG", "charge": "Charge", "mass": "Mass", "massUncertainty": "\u03b4Mass",
                "width": "Width", "widthUncertainty": "\u03b4Width", "spin": "Spin", "isospin": "Isospin",
                "isospin3": "I3", "quarks": "Quarks", "antiquarks": "Antiquarks", "leptons": "Leptons",
                "branchingRatio": "BR", "clebschGordan": "CG"}

#: Colors of the table.
COLORS = {
    "particle_bg": "black",   "particle_fg": "white",     # particle rows
    "expanded_bg": "#c8c8c8", "expanded_fg": "black",     # row of a particle whose decays are shown
    "decay_bg":    "#e4e4e4", "decay_fg":    "black",     # decay rows
    "bad_fg":      "#b00000",                             # decays with unknown products
    "selected_bg": "#2f6db5", "selected_fg": "white",     # selected rows
}

KINDS = ["baryon", "meson", "lepton", "gauge boson", "quark", "other"]
KIND_LABELS = {"baryon": "Baryons", "meson": "Mesons", "lepton": "Leptons", "gauge boson": "Gauge bosons",
               "quark": "Quarks", "other": "Other"}
#: Characters escaped in attribute values (as CAP's XmlWriter does), besides & < >.
ENTITIES = {'"': "&quot;", "'": "&apos;"}
INT_RE = re.compile(r"[+-]?\d+")
INT_PARTIAL = re.compile(r"[+-]?\d*")
FLOAT_PARTIAL = re.compile(r"[+-]?\d*\.?\d*([eE][+-]?\d*)?")


# ------------------------------------------------------------------------------------------------------------------
# Model (no GUI)
# ------------------------------------------------------------------------------------------------------------------

class Particle:
    def __init__(self, attributes, decays=None):
        self.attributes = dict(attributes)      # field -> text, in the order of the file
        self.decays = decays or []              # list of dicts field -> text

    @property
    def name(self):
        return self.attributes.get("name", "")


class ParticleDb:
    def __init__(self):
        self.root_tag = "CAP"
        self.root_attributes = {"version": "1", "type": "particleDb"}
        self.db_tag = "PARTICLE_DB"
        self.particles = []
        self.particle_fields = list(PARTICLE_FIELDS)
        self.decay_fields = list(DECAY_FIELDS)

    @staticmethod
    def read(path):
        """Read a particle database file. Raises ValueError for files that are not particle databases."""
        try:
            root = ET.parse(path).getroot()
        except ET.ParseError as e:
            raise ValueError("%s is not a valid XML file: %s" % (path, e))
        db = ParticleDb()
        db.root_tag = root.tag
        db.root_attributes = dict(root.attrib)
        container = root if root.tag == "PARTICLE_DB" else root.find("PARTICLE_DB")
        if container is None:
            container = root
        db.db_tag = container.tag
        particle_fields, decay_fields = [], []
        for element in container.iter("PARTICLE"):
            decays = []
            for d in element.findall("DECAY"):
                decays.append(dict(d.attrib))
                decay_fields += [k for k in d.attrib if k not in decay_fields]
            particle_fields += [k for k in element.attrib if k not in particle_fields]
            db.particles.append(Particle(element.attrib, decays))
        if not db.particles:
            raise ValueError("%s contains no PARTICLE element." % path)
        db.particle_fields = particle_fields + [f for f in PARTICLE_FIELDS if f not in particle_fields]
        db.decay_fields = decay_fields + [f for f in DECAY_FIELDS if f not in decay_fields]
        return db

    def write(self, path):
        """Write the database in the format of CAP's particle database files."""
        def attrs(d, fields):
            keys = [f for f in fields if f in d] + [k for k in d if k not in fields]
            return " ".join('%s="%s"' % (k, escape(str(d[k]), ENTITIES)) for k in keys)
        lines = ['<?xml version="1.0" encoding="UTF-8"?>']
        nested = self.root_tag != self.db_tag
        if nested:
            root_attrs = " ".join('%s="%s"' % (k, escape(v, ENTITIES)) for k, v in self.root_attributes.items())
            lines.append("<%s%s>" % (self.root_tag, " " + root_attrs if root_attrs else ""))
        pad = "  " if nested else ""
        lines.append(pad + "<%s>" % self.db_tag)
        for p in self.particles:
            head = pad + "  <PARTICLE %s" % attrs(p.attributes, self.particle_fields)
            if p.decays:
                lines.append(head + ">")
                for d in p.decays:
                    lines.append(pad + "    <DECAY %s/>" % attrs(d, self.decay_fields))
                lines.append(pad + "  </PARTICLE>")
            else:
                lines.append(head + "/>")
        lines.append(pad + "</%s>" % self.db_tag)
        if nested:
            lines.append("</%s>" % self.root_tag)
        text = "\n".join(lines) + "\n"
        tmp = path + ".tmp"
        with open(tmp, "w", encoding="utf-8") as f:
            f.write(text)
        os.replace(tmp, path)

    def names(self):
        return {p.name for p in self.particles}

    def problems(self):
        """List of problems: duplicate names or codes, decays without products or with unknown products."""
        out = []
        seen_names, seen_codes = {}, {}
        for p in self.particles:
            if p.name in seen_names:
                out.append("Duplicate name %s" % p.name)
            seen_names[p.name] = 1
            code = p.attributes.get("pdg", "")
            if code in seen_codes:
                out.append("Duplicate PDG code %s (%s and %s)" % (code, seen_codes[code], p.name))
            seen_codes[code] = p.name
        names = set(seen_names)
        for p in self.particles:
            for d in p.decays:
                unknown = [n for n in d.get("products", "").split() if n not in names]
                if not d.get("products", "").split():
                    out.append("%s: decay without products" % p.name)
                elif unknown:
                    out.append("%s: decay to %s: unknown product(s) %s" % (p.name, d.get("products", ""),
                                                                          " ".join(unknown)))
        return out


def to_float(text, default=0.0):
    try:
        return float(text)
    except (TypeError, ValueError):
        return default


def digit_sum(text):
    try:
        return sum(int(c) for c in str(abs(int(text))))
    except (TypeError, ValueError):
        return 0


def kind(p):
    """baryon, meson, lepton, quark, gauge boson or other: from the PDG code, or from the quark content when the code
    is not a standard hadron code."""
    try:
        code = abs(int(p.attributes.get("pdg", "0")))
    except ValueError:
        code = 0
    if 1 <= code <= 8:
        return "quark"
    if 11 <= code <= 18:
        return "lepton"
    if code in (21, 22, 23, 24):
        return "gauge boson"
    if code >= 100 and code < 1000000000 and (code // 10) % 10 != 0:      # standard hadron code (nq3 != 0)
        return "baryon" if (code // 1000) % 10 != 0 else "meson"
    q, a = digit_sum(p.attributes.get("quarks")), digit_sum(p.attributes.get("antiquarks"))
    if (q, a) in ((3, 0), (0, 3)):
        return "baryon"
    if (q, a) == (1, 1):
        return "meson"
    return "other"


def is_charged(p):
    return abs(to_float(p.attributes.get("charge"))) > 1e-6


def field_type(field):
    if field in INT_FIELDS:
        return "int"
    if field in FLOAT_FIELDS:
        return "float"
    if field in BOOL_FIELDS:
        return "bool"
    return "string"


def valid(field, text):
    t = field_type(field)
    if t == "int":
        return INT_RE.fullmatch(text.strip()) is not None
    if t == "float":
        try:
            float(text)
            return True
        except ValueError:
            return False
    if t == "bool":
        return text in ("true", "false")
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


# ------------------------------------------------------------------------------------------------------------------
# GUI
# ------------------------------------------------------------------------------------------------------------------

def run_gui(path=None, hook=None):
    import tkinter as tk
    from tkinter import ttk, filedialog, messagebox

    MOD = "Command" if sys.platform == "darwin" else "Control"
    MOD_TEXT = "Cmd" if sys.platform == "darwin" else "Ctrl"

    class Editor:
        def __init__(self, root):
            self.root = root
            self.db = None
            self.path = ""
            self.dirty = False
            self.items = {}          # tree item -> (particle, decay dict or None)
            self.particle_item = {}  # id(particle) -> tree item
            self.sort = None         # (column, reverse) or None for file order
            self.editor = None
            root.geometry("1500x900")
            root.minsize(900, 500)
            style = ttk.Style()
            if "clam" in style.theme_names() and sys.platform != "darwin":
                style.theme_use("clam")
            # Table colors (fixed, independent of the light/dark mode of the system): particles white on black, the
            # row of an expanded particle light gray, its decays black on light gray.
            style.configure("Treeview", rowheight=22, background=COLORS["particle_bg"],
                            fieldbackground=COLORS["particle_bg"], foreground=COLORS["particle_fg"])
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
            m.add_command(label="Add particle", command=self.add_particle)
            m.add_command(label="Duplicate particle", command=self.duplicate_particle)
            m.add_command(label="Add decay", command=self.add_decay)
            m.add_separator()
            m.add_command(label="Delete selected rows", accelerator="Delete", command=self.delete_selected)
            menubar.add_cascade(label="Edit", menu=m)
            m = tk.Menu(menubar, tearoff=0)
            m.add_command(label="Expand all (show decays)", command=lambda: self.expand(True))
            m.add_command(label="Collapse all", command=lambda: self.expand(False))
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
            entry = ttk.Entry(bar, textvariable=self.name_filter, width=18)
            entry.pack(side=tk.LEFT, padx=(4, 14))
            self.kind_vars = {}
            for k in KINDS:
                var = tk.BooleanVar(value=True)
                self.kind_vars[k] = var
                ttk.Checkbutton(bar, text=KIND_LABELS[k], variable=var, command=self.refresh).pack(side=tk.LEFT)
            ttk.Separator(bar, orient=tk.VERTICAL).pack(side=tk.LEFT, fill=tk.Y, padx=10)
            self.charge_filter = tk.StringVar(value="all")
            for value, text in (("all", "All"), ("charged", "Charged"), ("neutral", "Neutral")):
                ttk.Radiobutton(bar, text=text, value=value, variable=self.charge_filter,
                                command=self.refresh).pack(side=tk.LEFT)
            ttk.Separator(bar, orient=tk.VERTICAL).pack(side=tk.LEFT, fill=tk.Y, padx=10)
            ttk.Button(bar, text="All kinds", command=lambda: self._set_kinds(True)).pack(side=tk.LEFT)
            ttk.Button(bar, text="Hadrons", command=lambda: self._set_kinds(False, ("baryon", "meson"))).pack(
                side=tk.LEFT, padx=4)
            self.name_filter.trace_add("write", lambda *a: self.refresh())

        def _set_kinds(self, value, only=()):
            for k, var in self.kind_vars.items():
                var.set(value or k in only)
            self.refresh()

        def _make_table(self):
            frame = ttk.Frame(self.root)
            frame.pack(fill=tk.BOTH, expand=True, padx=6)
            self.tree = ttk.Treeview(frame, selectmode="extended")
            ys = ttk.Scrollbar(frame, orient=tk.VERTICAL, command=self.tree.yview)
            xs = ttk.Scrollbar(frame, orient=tk.HORIZONTAL, command=self.tree.xview)
            self.tree.configure(yscrollcommand=ys.set, xscrollcommand=xs.set)
            self.tree.grid(row=0, column=0, sticky="nsew")
            ys.grid(row=0, column=1, sticky="ns")
            xs.grid(row=1, column=0, sticky="ew")
            frame.rowconfigure(0, weight=1)
            frame.columnconfigure(0, weight=1)
            self.tree.tag_configure("particle", background=COLORS["particle_bg"], foreground=COLORS["particle_fg"])
            self.tree.tag_configure("expanded", background=COLORS["expanded_bg"], foreground=COLORS["expanded_fg"])
            self.tree.tag_configure("decay", background=COLORS["decay_bg"], foreground=COLORS["decay_fg"])
            self.tree.tag_configure("bad", foreground=COLORS["bad_fg"])
            self.tree.bind("<Double-Button-1>", self._on_double_click)
            self.tree.bind("<Delete>", lambda e: self.delete_selected())
            self.tree.bind("<BackSpace>", lambda e: self.delete_selected())
            self.tree.bind("<<TreeviewOpen>>", self._on_open_close)
            self.tree.bind("<<TreeviewClose>>", self._on_open_close)
            for seq in ("<MouseWheel>", "<Button-4>", "<Button-5>", "<Configure>"):
                self.tree.bind(seq, lambda e: self._close_editor(True), add="+")

        def _setup_columns(self):
            """Columns: Type, then the decay fields other than the products, then the particle fields but the name."""
            pf = [f for f in self.db.particle_fields if f != "name"]
            df = [f for f in self.db.decay_fields if f != "products"]
            self.columns = ["kind"] + ["d:" + f for f in df] + ["p:" + f for f in pf]
            self.tree.configure(columns=self.columns, displaycolumns=self.columns)
            self.tree.heading("#0", text="Particle  /  decay products", command=lambda: self._sort_by("#0"))
            self.tree.column("#0", width=380, minwidth=160, stretch=False)
            widths = {"title": 150, "pdg": 80, "branchingRatio": 70, "clebschGordan": 55, "quarks": 80,
                      "antiquarks": 80}
            for c in self.columns:
                field = c.split(":", 1)[-1]
                label = "Type" if c == "kind" else FIELD_LABELS.get(field, field)
                self.tree.heading(c, text=label, command=lambda c=c: self._sort_by(c))
                width = 95 if c == "kind" else widths.get(field, 72)
                anchor = "w" if c == "kind" or field_type(field) == "string" else "e"
                if field_type(field) == "bool":
                    anchor = "center"
                self.tree.column(c, width=width, minwidth=40, anchor=anchor, stretch=False)

        # ------------------------------------------------------------------------------------------ table
        def visible(self, p):
            text = self.name_filter.get().strip().lower()
            if text and text not in p.name.lower() and text not in p.attributes.get("title", "").lower():
                return False
            if not self.kind_vars[kind(p)].get():
                return False
            charge = self.charge_filter.get()
            if charge == "charged" and not is_charged(p):
                return False
            if charge == "neutral" and is_charged(p):
                return False
            return True

        def ordered(self):
            particles = list(self.db.particles)
            if self.sort is None:
                return particles
            column, reverse = self.sort
            if column == "#0":
                key = lambda p: p.name.lower()
            elif column == "kind":
                key = lambda p: kind(p)
            elif column.startswith("d:"):
                key = lambda p: len(p.decays)
            else:
                field = column[2:]
                if field_type(field) in ("int", "float"):
                    key = lambda p: to_float(p.attributes.get(field), float("-inf"))
                else:
                    key = lambda p: p.attributes.get(field, "").lower()
            return sorted(particles, key=key, reverse=reverse)

        def refresh(self):
            """Rebuild the table from the database and the filters (keeping the expanded particles open)."""
            if self.db is None:
                return
            self._close_editor(True)
            opened = {id(self.items[i][0]) for i in self.tree.get_children("") if self.tree.item(i, "open")}
            selected = {(id(self.items[i][0]), id(self.items[i][1]) if self.items[i][1] is not None else None)
                        for i in self.tree.selection() if i in self.items}
            self.tree.delete(*self.tree.get_children(""))
            self.items.clear()
            self.particle_item.clear()
            names = self.db.names()
            shown, keep = 0, []
            for p in self.ordered():
                if not self.visible(p):
                    continue
                item = self.tree.insert("", tk.END, open=id(p) in opened)
                self.items[item] = (p, None)
                self.particle_item[id(p)] = item
                self._fill_particle(item, shown)
                if (id(p), None) in selected:
                    keep.append(item)
                for d in p.decays:
                    sub = self.tree.insert(item, tk.END)
                    self.items[sub] = (p, d)
                    self._fill_decay(sub, names)
                    if (id(p), id(d)) in selected:
                        keep.append(sub)
                shown += 1
            if keep:
                self.tree.selection_set(keep)
            n_decays = sum(len(p.decays) for p in self.db.particles)
            self.count.configure(text="Showing %d of %d particles (%d decays in the file)"
                                 % (shown, len(self.db.particles), n_decays))

        def _fill_particle(self, item, index=None):
            p = self.items[item][0]
            values = []
            for c in self.columns:
                if c == "kind":
                    values.append(kind(p))
                elif c.startswith("d:"):
                    values.append("")
                else:
                    values.append(p.attributes.get(c[2:], ""))
            tags = ["expanded" if self.tree.item(item, "open") and p.decays else "particle"]
            n = len(p.decays)
            text = p.name + ("   (%d decay%s)" % (n, "s" if n > 1 else "") if n else "")
            self.tree.item(item, text=text, values=values, tags=tags)

        def _fill_decay(self, item, names=None):
            p, d = self.items[item]
            names = names if names is not None else self.db.names()
            values = [d.get(c[2:], "") if c.startswith("d:") else "" for c in self.columns]
            products = d.get("products", "")
            bad = any(n not in names for n in products.split()) or not products.strip()
            self.tree.item(item, text="\u2192 " + products, values=values, tags=("decay", "bad") if bad else ("decay",))

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

        def expand(self, value):
            for item in self.tree.get_children(""):
                self.tree.item(item, open=value)
            self._update_open_tags()

        def _on_open_close(self, _event=None):
            self._close_editor(True)
            self.root.after_idle(self._update_open_tags)    # the open state changes after the event

        def _update_open_tags(self):
            for item in self.tree.get_children(""):
                p = self.items[item][0]
                tag = "expanded" if self.tree.item(item, "open") and p.decays else "particle"
                if self.tree.item(item, "tags") != (tag,):
                    self.tree.item(item, tags=(tag,))

        # ------------------------------------------------------------------------------------------ editing
        def _field_of(self, item, column):
            """Field edited in a cell, or None if the cell is not editable."""
            p, d = self.items[item]
            if column == "#0":
                return "products" if d is not None else "name"
            c = self.tree.column(column, "id")
            if c == "kind":
                return None
            if d is None:
                return c[2:] if c.startswith("p:") else None
            return c[2:] if c.startswith("d:") else None

        def _on_double_click(self, event):
            if self.tree.identify_region(event.x, event.y) not in ("cell", "tree"):
                return
            item = self.tree.identify_row(event.y)
            column = self.tree.identify_column(event.x)
            if item and item in self.items:
                if self._field_of(item, column) is None:
                    self.status.set("This cell cannot be edited (Type is computed from the PDG code; particle rows "
                                    "have no decay fields, decay rows no particle fields).")
                    return
                self.edit_cell(item, column)
                return "break"

        def edit_cell(self, item, column):
            self._close_editor(True)
            if item not in self.items or not self.tree.exists(item):
                return
            field = self._field_of(item, column)
            if field is None:
                return
            self.tree.see(item)
            self.root.update_idletasks()
            box = self.tree.bbox(item, column)
            if not box:
                return
            x, y, w, h = box
            p, d = self.items[item]
            target = d if d is not None else p.attributes
            value = target.get(field, "")
            var = tk.StringVar(value=value)
            if field_type(field) == "bool":
                widget = ttk.Combobox(self.tree, textvariable=var, values=("true", "false"), state="readonly")
                widget.bind("<<ComboboxSelected>>", lambda e: self._commit())
            else:
                widget = ttk.Entry(self.tree, textvariable=var)
                if field_type(field) in ("int", "float"):
                    check = (self.root.register(lambda text: partial(field, text)), "%P")
                    widget.configure(validate="key", validatecommand=check)
                widget.select_range(0, tk.END)
            if column == "#0":           # leave the tree indentation and the arrow visible
                indent = 20 if d is None else 40
                x, w = x + indent, max(w - indent, 120)
            widget.place(x=x, y=y, width=max(w, 140 if field_type(field) == "string" else w), height=h)
            widget.focus_set()
            widget.bind("<Return>", lambda e: self._commit())
            widget.bind("<KP_Enter>", lambda e: self._commit())
            widget.bind("<Escape>", lambda e: self._close_editor(False))
            widget.bind("<Tab>", lambda e: self._commit_and_move(1))
            widget.bind("<Shift-Tab>", lambda e: self._commit_and_move(-1))
            widget.bind("<ISO_Left_Tab>", lambda e: self._commit_and_move(-1))
            widget.bind("<FocusOut>", lambda e: self.root.after(1, self._commit_if_unfocused))
            self.editor = dict(widget=widget, var=var, item=item, column=column, field=field, target=target, p=p, d=d)

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
                if self._field_of(item, columns[k]) is not None:
                    self.root.after(1, lambda: self.edit_cell(item, columns[k]))
                    break
            return "break"

        def _close_editor(self, keep):
            if self.editor:
                if keep:
                    self._commit()
                else:
                    self.editor["widget"].destroy()
                    self.editor = None

        def _commit(self):
            """Store the value of the cell editor. Returns False (editor kept open) for an invalid value."""
            e = self.editor
            if not e:
                return True
            field, new = e["field"], e["var"].get().strip()
            if field == "products":
                new = " ".join(new.replace("\u2192", " ").split())
            old = e["target"].get(field, "")
            if new == old:
                self._discard_editor()
                return True
            if not valid(field, new):
                self.status.set("Invalid %s value for %s: '%s' (Escape cancels)" % (field_type(field), field, new))
                self.root.bell()
                return False
            if field == "name" and new in self.db.names():
                self.status.set("A particle named %s already exists." % new)
                self.root.bell()
                return False
            self._discard_editor()
            e["target"][field] = new
            if field == "name":
                self._rename_in_decays(old, new)
            self._changed()
            item = e["item"]
            if e["d"] is None:
                self._fill_particle(item)
            else:
                self._fill_decay(item)
                self._fill_particle(self.tree.parent(item))
            if field == "products":
                unknown = [n for n in new.split() if n not in self.db.names()]
                if unknown:
                    self.status.set("Unknown particle(s) in the decay products: %s" % " ".join(unknown))
                    return True
            self.status.set("%s: %s = %s" % (e["p"].name, field, new))
            return True

        def _discard_editor(self):
            if self.editor:
                widget = self.editor["widget"]
                self.editor = None
                widget.destroy()

        def _rename_in_decays(self, old, new):
            """Offer to rename a particle in the decay products, then update the rows in place."""
            users = [d for p in self.db.particles for d in p.decays if old in d.get("products", "").split()]
            if users and messagebox.askyesno(
                    "Rename particle", "%d decay(s) have %s among their products.\nRename them to %s too?"
                                       % (len(users), old, new)):
                for d in users:
                    d["products"] = " ".join(new if n == old else n for n in d["products"].split())
            names = self.db.names()
            for item, (p, d) in list(self.items.items()):
                if d is not None and self.tree.exists(item):
                    self._fill_decay(item, names)

        # ------------------------------------------------------------------------------------------ edit menu
        def _selected_particle(self):
            for item in self.tree.selection():
                if item in self.items:
                    return self.items[item][0]
            return None

        def _insert_particle(self, new, after):
            index = self.db.particles.index(after) + 1 if after in self.db.particles else len(self.db.particles)
            self.db.particles.insert(index, new)
            self.name_filter.set("")
            for var in self.kind_vars.values():
                var.set(True)
            self.charge_filter.set("all")
            self._changed()
            self.refresh()
            item = self.particle_item[id(new)]
            self.tree.selection_set(item)
            self.tree.see(item)
            self.edit_cell(item, "#0")

        def add_particle(self):
            names = self.db.names()
            name, k = "new_particle", 2
            while name in names:
                name, k = "new_particle_%d" % k, k + 1
            attributes = {f: ("0" if field_type(f) in ("int", "float") else "") for f in self.db.particle_fields}
            attributes.update(name=name, title=name)
            self._insert_particle(Particle(attributes), self._selected_particle())

        def duplicate_particle(self):
            p = self._selected_particle()
            if p is None:
                self.status.set("Select the particle to duplicate.")
                return
            names = self.db.names()
            name, k = p.name + "_copy", 2
            while name in names:
                name, k = "%s_copy%d" % (p.name, k), k + 1
            attributes = dict(p.attributes, name=name)
            self._insert_particle(Particle(attributes, [dict(d) for d in p.decays]), p)

        def add_decay(self):
            p = self._selected_particle()
            if p is None:
                self.status.set("Select the particle that decays.")
                return
            d = {f: "" for f in self.db.decay_fields}
            d.update(branchingRatio="0", clebschGordan="false", products="")
            p.decays.append(d)
            self._changed()
            item = self.particle_item.get(id(p))
            if item is None:
                return
            self.tree.item(item, open=True)
            sub = self.tree.insert(item, tk.END)
            self.items[sub] = (p, d)
            self._fill_decay(sub)
            self._fill_particle(item)
            self.tree.selection_set(sub)
            self.tree.see(sub)
            self.edit_cell(sub, "#0")

        def delete_selected(self):
            if self.editor:
                return
            rows = [self.items[i] for i in self.tree.selection() if i in self.items]
            particles = [p for p, d in rows if d is None]
            decays = [(p, d) for p, d in rows if d is not None and p not in particles]
            if not rows:
                return
            text = []
            if particles:
                text.append("%d particle(s): %s" % (len(particles), ", ".join(p.name for p in particles[:8])
                                                     + (", \u2026" if len(particles) > 8 else "")))
            if decays:
                text.append("%d decay(s)" % len(decays))
            if not messagebox.askyesno("Delete", "Delete %s?" % " and ".join(text)):
                return
            for p in particles:
                self.db.particles.remove(p)
            for p, d in decays:
                p.decays = [x for x in p.decays if x is not d]
            self._changed()
            self.refresh()
            self.status.set("Deleted %s." % " and ".join(text))

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
            self.root.title("CAP Particle DB \u2014 %s%s" % (name, " *" if self.dirty else ""))

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
                path = self._dialog("askopenfilename", title="Open a particle database",
                                    initialdir=os.path.dirname(self.path or DEFAULT_FILE),
                                    filetypes=[("XML files", "*.xml"), ("All files", "*")])
            if not path:
                return
            try:
                db = ParticleDb.read(path)
            except (ValueError, OSError) as e:
                messagebox.showerror("Open", str(e))
                return
            self.db, self.path, self.dirty, self.sort = db, path, False, None
            self._setup_columns()
            self.refresh()
            self._update_title()
            problems = db.problems()
            self.status.set("Read %s: %d particles, %d decays%s" % (
                path, len(db.particles), sum(len(p.decays) for p in db.particles),
                "; %d problem(s), see File > Check database" % len(problems) if problems else ""))

        def save(self):
            if not self.path:
                return self.save_as()
            return self._write(self.path)

        def save_as(self):
            if self.db is None:
                return False
            path = self._dialog("asksaveasfilename", title="Write the particle database to",
                                initialdir=os.path.dirname(self.path or DEFAULT_FILE),
                                initialfile=os.path.basename(self.path or "ParticleDb.xml"),
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
            except OSError as e:
                messagebox.showerror("Save", "Cannot write %s:\n%s" % (path, e))
                return False
            self.dirty = False
            self._update_title()
            self.status.set("Wrote %s (%d particles)" % (path, len(self.db.particles)))
            return True

        def show_problems(self):
            if self.db is None:
                return
            self._close_editor(True)
            problems = self.db.problems()
            window = tk.Toplevel(self.root)
            window.title("Check of the particle database")
            window.geometry("760x420")
            text = tk.Text(window, wrap=tk.NONE, font=("Menlo", 11) if sys.platform == "darwin" else ("TkFixedFont",))
            sb = ttk.Scrollbar(window, orient=tk.VERTICAL, command=text.yview)
            text.configure(yscrollcommand=sb.set)
            sb.pack(side=tk.RIGHT, fill=tk.Y)
            text.pack(fill=tk.BOTH, expand=True)
            text.insert(tk.END, "\n".join(problems) if problems else "No problem found.")
            text.insert("1.0", "%d problem(s)\n\n" % len(problems))
            text.configure(state=tk.DISABLED)

        def quit(self):
            self._close_editor(True)
            if self._confirm_discard():
                self.root.destroy()

        def _callback_error(self, exc, value, tb):
            import traceback
            sys.stderr.write("".join(traceback.format_exception(exc, value, tb)))
            messagebox.showerror("ParticleDbEditor.py error", "%s: %s" % (exc.__name__, value))

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
    if args and args[0] == "--check":
        path = args[1] if len(args) > 1 else DEFAULT_FILE
        try:
            db = ParticleDb.read(path)
        except (ValueError, OSError) as e:
            print(e, file=sys.stderr)
            return 1
        problems = db.problems()
        for p in problems:
            print(p)
        kinds = {}
        for p in db.particles:
            kinds[kind(p)] = kinds.get(kind(p), 0) + 1
        print("%d particles (%s), %d decays, %d problem(s)" % (
            len(db.particles), ", ".join("%d %s" % (n, k) for k, n in sorted(kinds.items())),
            sum(len(p.decays) for p in db.particles), len(problems)))
        return 1 if problems else 0
    try:
        import tkinter  # noqa: F401
    except ImportError:
        print("ParticleDbEditor.py needs tkinter (macOS with Homebrew Python: brew install python-tk; "
              "Debian/Ubuntu: apt install python3-tk).", file=sys.stderr)
        return 1
    path = args[0] if args else (DEFAULT_FILE if os.path.exists(DEFAULT_FILE) else None)
    run_gui(path)
    return 0


if __name__ == "__main__":
    sys.exit(main())
