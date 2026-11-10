# **********************************************************************
# Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
# All rights reserved.
#
# For the licensing terms see LICENSE.
# **********************************************************************
"""
RunCAPFiltersGui -- the Filters pane of the main window of RunCAP.py: the library of filters, efficiencies and smearers of a workflow.

On the left, the tree of the library: one node per kind (event filters, particle filters, event efficiencies, particle
efficiencies, event smearers, particle smearers, events, particle databases) holding its objects, with where each is
defined (the workflow, a linked selection file, the maps file) and the tasks that use it, and the selection files the
workflow links. On the right, the editor of what is selected: an object (its name, title, and its conditions or its
parameters), a kind (the settings of its list in FilterCreator, and its objects), a linked file, or the maps file.
A condition tests one quantity: a range [minimum, maximum), two ranges, a value (PDG code, charge, ...) or a flag (live,
enabled), only the tests the C++ code accepts for that quantity being offered. The problems of the library
(RunCAPFilters.check_library()) are listed at the bottom.

Selection files (RunCAPFilters: XML, root CapSelections) hold objects that several workflows share. Import copies
their objects into the workflow; Link uses them from the file (the workflow writes <Include file=.../>), changes being
written back to the file when the workflow is saved; Export writes the selected object, the objects of a kind, or all
of them to a file. The particle efficiencies and smearers are not defined here: they are made in CapGui (Detector
Maps) and saved in a maps file, which the pane names and lists (MapsEditor).

The tasks choose objects by name (RunCAPFilters): renaming an object renames it in the choices of the tasks, and
removing objects removes them from the choices (after a confirmation when tasks use them), through the callbacks given
to FiltersPanel.
"""

import os
import subprocess
import sys
import tkinter as tk
from tkinter import filedialog, messagebox, ttk

import RunCAPFilters as F

MAX_OBJECTS = {"EVENT_FILTER": 20, "PARTICLE_FILTER": 20}
MAX_OTHERS = 10
MAX_CONDITIONS = 12
_DOUBLE_FIELDS = {F.RANGE: ("minimum", "maximum"), F.DUAL_RANGE: ("minimum", "maximum", "minimum2", "maximum2")}
_FIELD_LABELS = {"minimum": "min", "maximum": "max", "minimum2": "min 2", "maximum2": "max 2"}


def _partial_number(text, integer=False):
    import re
    pattern = r"[+-]?\d*" if integer else r"[+-]?\d*\.?\d*([eE][+-]?\d*)?"
    return re.fullmatch(pattern, text) is not None


def _number(text, integer=False):
    try:
        return float(int(text)) if integer else float(text)
    except ValueError:
        return None


def quantities(kind):
    """{label shown: subtype} of the quantities a filter of a kind can test (Glauber quantities marked)."""
    if kind == "EVENT_FILTER":
        # the quantities of a selection only: MULT_k, ENERGY_k and SPHERO_k (set by GlobalAnalyzer, earlier versions)
        # are still read from older files, but not offered
        items = [(s, d) for s, (d, _) in F.EVENT_SUBTYPES.items() if s not in F.LEGACY_EVENT_SUBTYPES]
        items += [(s, d + "  (Glauber tasks)") for s, (d, _) in F.GLAUBER_SUBTYPES.items()]
    else:
        items = [(s, d) for s, (d, _) in F.subtypes_for(kind).items()]
    return {"%s — %s" % (s, d): s for s, d in items}


def forms_of(kind, subtype):
    accepted = dict(F.EVENT_SUBTYPES, **F.GLAUBER_SUBTYPES) if kind == "EVENT_FILTER" else F.subtypes_for(kind)
    return accepted.get(subtype, ("", (F.RANGE,)))[1]


class ScrollFrame(ttk.Frame):
    """Frame with a vertical scroll bar; widgets go in self.inner."""

    def __init__(self, parent):
        ttk.Frame.__init__(self, parent)
        background = ttk.Style().lookup("TFrame", "background")
        self.canvas = tk.Canvas(self, highlightthickness=0, borderwidth=0)
        if background:
            self.canvas.configure(background=background)
        bar = ttk.Scrollbar(self, orient=tk.VERTICAL, command=self.canvas.yview)
        self.inner = ttk.Frame(self.canvas, padding=(4, 2))
        self.window = self.canvas.create_window((0, 0), window=self.inner, anchor="nw")
        self.canvas.configure(yscrollcommand=bar.set)
        self.canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        bar.pack(side=tk.RIGHT, fill=tk.Y)
        self.inner.bind("<Configure>", lambda e: self.canvas.configure(scrollregion=self.canvas.bbox("all")))
        self.canvas.bind("<Configure>", lambda e: self.canvas.itemconfigure(self.window, width=e.width))
        for w in (self.canvas, self.inner):
            w.bind("<Enter>", lambda e: self.canvas.bind_all("<MouseWheel>", self._wheel), add="+")
            w.bind("<Leave>", lambda e: self.canvas.unbind_all("<MouseWheel>"), add="+")

    def _wheel(self, event):
        if self.inner.winfo_reqheight() > self.canvas.winfo_height():
            self.canvas.yview_scroll(-1 * (event.delta if abs(event.delta) < 10 else event.delta // 120), "units")


class FiltersPanel(ttk.Frame):
    """The Filters pane: on the left, the tree of the library (kinds, their objects, the linked selection files); on
    the right, the editor of what is selected there.
    @param library RunCAPFilters.FilterLibrary edited in place
    @param on_change called after any change of the library
    @param on_rename called with (kind, old name, new name) when an object is renamed
    @param on_remove called with (kind, names) when objects are removed
    @param users called with (kind, name), returns the names of the tasks that use an object
    @param directory called without argument, returns the directory where to look for selection files first (that of
    the workflow)"""

    def __init__(self, parent, library, on_change=None, on_rename=None, on_remove=None, users=None, directory=None):
        ttk.Frame.__init__(self, parent, padding=6)
        self.library = library
        self.on_change = on_change or (lambda: None)
        self.on_rename = on_rename or (lambda kind, old, new: None)
        self.on_remove = on_remove or (lambda kind, names: None)
        self.users = users or (lambda kind, name: [])
        self.directory = directory or (lambda: os.getcwd())
        self.selected = ("kind", "EVENT_FILTER")     # ("kind", kind), ("object", kind, name), ("link", path)
        self.setting_vars = {}                       # {kind: {setting: variable}} of the editor shown
        self._loading = False
        style = ttk.Style()
        style.configure("Invalid.TEntry", foreground="firebrick")
        style.configure("Object.TLabelframe.Label", font=("TkDefaultFont", 11, "bold"))
        style.configure("Editor.TLabel", font=("TkDefaultFont", 12, "bold"))
        self._make_toolbar()
        panes = ttk.PanedWindow(self, orient=tk.HORIZONTAL)
        panes.pack(fill=tk.BOTH, expand=True, pady=(4, 0))
        left = ttk.Frame(panes)
        self.tree = ttk.Treeview(left, columns=("source", "used"), selectmode="browse", height=10)
        self.tree.heading("#0", text="Object")
        self.tree.heading("source", text="Defined in")
        self.tree.heading("used", text="Used by")
        self.tree.column("#0", width=210, stretch=True)
        self.tree.column("source", width=110, stretch=False)
        self.tree.column("used", width=110, stretch=False)
        bar = ttk.Scrollbar(left, orient=tk.VERTICAL, command=self.tree.yview)
        self.tree.configure(yscrollcommand=bar.set)
        self.tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        bar.pack(side=tk.RIGHT, fill=tk.Y)
        self.tree.bind("<<TreeviewSelect>>", self._tree_selected)
        self.tree.bind("<Delete>", lambda e: self.remove_selected())
        self.tree.bind("<BackSpace>", lambda e: self.remove_selected())
        self.editor = ScrollFrame(panes)
        self.editor.inner.columnconfigure(0, weight=1)
        panes.add(left, weight=1)
        panes.add(self.editor, weight=3)
        self.maps = MapsEditor(self)
        self.problems = tk.StringVar(self)
        ttk.Label(self, textvariable=self.problems, foreground="firebrick", justify=tk.LEFT,
                  wraplength=900).pack(fill=tk.X, pady=(6, 0))
        self.refresh()

    # -------------------------------------------------------------------------------------------------- toolbar
    def _make_toolbar(self):
        bar = ttk.Frame(self)
        bar.pack(fill=tk.X)
        add = ttk.Menubutton(bar, text="Add ▾")
        menu = tk.Menu(add, tearoff=False)
        for kind in F.KINDS:
            if kind not in F.MAPS_KINDS:
                menu.add_command(label=F.KIND_LABELS[kind][:-1], command=lambda k=kind: self.add_object(k))
        add.configure(menu=menu)
        add.pack(side=tk.LEFT)
        self.duplicate_button = ttk.Button(bar, text="Duplicate", command=self.duplicate_selected)
        self.duplicate_button.pack(side=tk.LEFT, padx=(4, 0))
        self.remove_button = ttk.Button(bar, text="Remove", command=self.remove_selected)
        self.remove_button.pack(side=tk.LEFT, padx=(4, 0))
        ttk.Separator(bar, orient=tk.VERTICAL).pack(side=tk.LEFT, fill=tk.Y, padx=10)
        ttk.Button(bar, text="Import…", command=self.import_file).pack(side=tk.LEFT)
        ttk.Button(bar, text="Link…", command=self.link_file).pack(side=tk.LEFT, padx=(4, 0))
        export = ttk.Menubutton(bar, text="Export ▾")
        self.export_menu = tk.Menu(export, tearoff=False)
        self.export_menu.add_command(label="Selected object…", command=lambda: self.export_file("object"))
        self.export_menu.add_command(label="Objects of the selected kind…", command=lambda: self.export_file("kind"))
        self.export_menu.add_command(label="All the filters, corrections, events and databases…",
                                     command=lambda: self.export_file("all"))
        export.configure(menu=self.export_menu)
        export.pack(side=tk.LEFT, padx=(4, 0))
        ttk.Label(bar, text="selection files: Import copies their objects, Link uses them from the file",
                  foreground="gray40").pack(side=tk.LEFT, padx=12)

    # -------------------------------------------------------------------------------------------------- tree
    def refresh(self, kinds=None):
        """Remake the tree and the editor from the library (kinds: ignored, the whole pane is remade)."""
        self.maps.load()
        self._fill_tree()
        self._fix_selection()
        self._select_in_tree()
        self._show_editor()
        self._update_problems()

    @staticmethod
    def _object_id(kind, name):
        return "object\t%s\t%s" % (kind, name)

    def _source_text(self, o):
        if o.kind in F.MAPS_KINDS:
            return "maps file"
        return os.path.basename(o.source) if o.source else "workflow"

    def _fill_tree(self):
        tree = self.tree
        opened = {i for i in tree.get_children() if tree.item(i, "open")}
        first = not tree.get_children()
        tree.delete(*tree.get_children())
        for kind in F.KINDS:
            objects = self.library.objects[kind]
            label = F.KIND_LABELS[kind] + (" (%d)" % len(objects) if objects else "")
            iid = "kind\t" + kind
            tree.insert("", tk.END, iid=iid, text=label, open=first or iid in opened or bool(objects) and first)
            for o in objects:
                users = self.users(kind, o.name)
                tree.insert(iid, tk.END, iid=self._object_id(kind, o.name), text=o.name + (" — " + o.title if o.title else ""),
                            values=(self._source_text(o), ", ".join(users)))
            if iid in opened:
                tree.item(iid, open=True)
        if self.library.links:
            tree.insert("", tk.END, iid="links", text="Linked selection files (%d)" % len(self.library.links), open=True)
            for path in self.library.links:
                n = len(self.library.linked(path))
                tree.insert("links", tk.END, iid="link\t" + path, text=os.path.basename(path),
                            values=("%d object%s" % (n, "s" if n != 1 else "") if os.path.isfile(path) else "missing",
                                    ""))

    def _fix_selection(self):
        """The selection after a change: the object if it is still there, else its kind."""
        s = self.selected
        if s[0] == "object" and self.library.get(s[1], s[2]) is None:
            self.selected = ("kind", s[1])
        elif s[0] == "link" and s[1] not in self.library.links:
            self.selected = ("kind", "EVENT_FILTER")

    def _iid(self, selection):
        if selection[0] == "object":
            return self._object_id(selection[1], selection[2])
        return ("kind\t" if selection[0] == "kind" else "link\t") + selection[1]

    def _select_in_tree(self):
        iid = self._iid(self.selected)
        if self.tree.exists(iid):
            self._loading = True
            parent = self.tree.parent(iid)
            if parent:
                self.tree.item(parent, open=True)
            self.tree.selection_set(iid)
            self.tree.focus(iid)
            self.tree.see(iid)
            self.update_idletasks()
            self._loading = False
        state = tk.NORMAL if self.selected[0] == "object" and self.selected[1] not in F.MAPS_KINDS else tk.DISABLED
        self.duplicate_button.configure(state=state)
        self.remove_button.configure(state=state if self.selected[0] != "link" else tk.NORMAL)

    def _tree_selected(self, _event=None):
        if self._loading:
            return
        chosen = self.tree.selection()
        if not chosen:
            return
        parts = chosen[0].split("\t")
        if parts[0] == "links":
            return
        selection = tuple(parts)
        if selection != self.selected:
            self.selected = selection
            self._select_in_tree()
            self._show_editor()

    def select(self, kind):
        """Show a kind (its node selected)."""
        self.selected = ("kind", kind)
        self._select_in_tree()
        self._show_editor()

    def select_object(self, kind, name):
        self.selected = ("object", kind, name)
        self._select_in_tree()
        self._show_editor()

    # -------------------------------------------------------------------------------------------------- editor
    def _clear_editor(self):
        for child in self.editor.inner.winfo_children():
            child.pack_forget() if child is self.maps.frame else child.destroy()
        self.setting_vars = {}

    def _show_editor(self):
        position = self.editor.canvas.yview()[0]
        self._clear_editor()
        s = self.selected
        if s[0] == "kind" and s[1] in F.MAPS_KINDS or s[0] == "object" and s[1] in F.MAPS_KINDS:
            self.maps.show(s[1], s[2] if s[0] == "object" else None)
        elif s[0] == "kind":
            self._show_kind(s[1])
        elif s[0] == "object":
            self._make_object(self.editor.inner, self.library.get(s[1], s[2]))
        elif s[0] == "link":
            self._show_link(s[1])
        self.editor.update_idletasks()
        self.editor.canvas.yview_moveto(position if s[0] == "object" else 0.0)

    def _show_kind(self, kind):
        inner = self.editor.inner
        ttk.Label(inner, text=F.KIND_LABELS[kind], style="Editor.TLabel").pack(anchor="w")
        hint = {"EVENT_FILTER": "Each filter selects events with all its conditions.",
                "PARTICLE_FILTER": "Each filter selects particles with all its conditions (once per event, shared by "
                                   "the tasks using it).",
                "EVENT": "Events, filled by the generators or readers and analyzed by the analyzers.",
                "PARTICLE_DB": "Particles and their decay modes, imported by FilterCreator from an XML file."
                }.get(kind, "Their histograms are created or imported by FilterCreator.")
        ttk.Label(inner, text=hint, foreground="gray40", wraplength=640, justify=tk.LEFT).pack(anchor="w", pady=(2, 8))
        if kind in F.SETTINGS_KINDS:
            self._make_settings(inner, kind)
        objects = self.library.objects[kind]
        line = ttk.Frame(inner)
        line.pack(fill=tk.X, pady=(4, 4))
        ttk.Label(line, text="%d %s." % (len(objects), F.KIND_LABELS[kind].lower()) if objects
                  else "No %s." % F.KIND_LABELS[kind].lower()).pack(side=tk.LEFT)
        add = ttk.Button(line, text="+ Add", command=lambda: self.add_object(kind))
        add.pack(side=tk.LEFT, padx=10)
        if len(objects) >= MAX_OBJECTS.get(kind, MAX_OTHERS):
            add.configure(state=tk.DISABLED)
        for o in objects:
            row = ttk.Frame(inner)
            row.pack(fill=tk.X)
            link = ttk.Label(row, text=o.name, foreground="#1f5fa8", cursor="hand2")
            link.pack(side=tk.LEFT, padx=(12, 6))
            link.bind("<Button-1>", lambda e, n=o.name: self.select_object(kind, n))
            details = [o.title] if o.title else []
            if kind in F.CONDITION_KINDS:
                details.append("%d condition%s" % (len(o.conditions), "s" if len(o.conditions) != 1 else ""))
            if o.source:
                details.append("from " + os.path.basename(o.source))
            ttk.Label(row, text="  ·  ".join(details), foreground="gray40").pack(side=tk.LEFT)

    def _show_link(self, path):
        inner = self.editor.inner
        ttk.Label(inner, text="Selection file", style="Editor.TLabel").pack(anchor="w")
        ttk.Label(inner, text=path, wraplength=640, justify=tk.LEFT).pack(anchor="w", pady=(2, 6))
        objects = self.library.linked(path)
        if not os.path.isfile(path):
            ttk.Label(inner, text="The file is missing: its objects are not in the workflow.",
                      foreground="firebrick").pack(anchor="w")
        ttk.Label(inner, text="The workflow uses its objects from the file (written as <Include file=…/>): they are "
                              "read from the file each time the workflow is loaded, and the changes made to them here "
                              "are written to the file when the workflow is saved.", foreground="gray40",
                  wraplength=640, justify=tk.LEFT).pack(anchor="w", pady=(0, 6))
        for o in objects:
            row = ttk.Frame(inner)
            row.pack(fill=tk.X)
            link = ttk.Label(row, text="%s %s" % (F.KIND_LABELS[o.kind][:-1].lower(), o.name), foreground="#1f5fa8",
                             cursor="hand2")
            link.pack(side=tk.LEFT, padx=(12, 6))
            link.bind("<Button-1>", lambda e, k=o.kind, n=o.name: self.select_object(k, n))
        buttons = ttk.Frame(inner)
        buttons.pack(anchor="w", pady=(10, 0))
        ttk.Button(buttons, text="Copy into the workflow", command=lambda: self.unlink(path, True)).pack(side=tk.LEFT)
        ttk.Button(buttons, text="Unlink and remove its objects",
                   command=lambda: self.unlink(path, False)).pack(side=tk.LEFT, padx=6)

    def _make_settings(self, parent, kind):
        """Settings of the list of objects of a kind in FilterCreator: creation or import of their histograms, export."""
        line = ttk.Frame(parent)
        line.pack(fill=tk.X, pady=(0, 6))
        variables = {}
        shown = F.KIND_SETTING_NAMES.get(kind)
        for name, ptype, _ in F.LIST_SETTINGS:
            if shown is not None and name not in shown:
                continue
            label = {"CREATE": "Create", "IMPORT": "Import", "IMPORT:PATH": "from", "IMPORT:FILE_NAME": "file",
                     "EXPORT": "Export", "EXPORT:PATH": "to", "EXPORT:FILE_NAME": "file"}[name]
            value = self.library.settings[kind].get(name, "")
            if ptype == "bool":
                var = tk.BooleanVar(self, value=value == "true")
                ttk.Checkbutton(line, text=label, variable=var).pack(side=tk.LEFT, padx=(0 if name == "CREATE" else 12, 0))
            else:
                var = tk.StringVar(self, value=value)
                ttk.Label(line, text=label if shown is None else {"IMPORT:PATH": "Import from",
                                                                  "IMPORT:FILE_NAME": "file"}[name]).pack(side=tk.LEFT,
                                                                                                          padx=(6, 2))
                ttk.Entry(line, textvariable=var, width=14 if name.endswith("FILE_NAME") else 10).pack(side=tk.LEFT)
            var.trace_add("write", lambda *a, k=kind, n=name, v=var: self._set_setting(k, n, v))
            variables[name] = var
        self.setting_vars[kind] = variables

    def _set_setting(self, kind, name, var):
        if self._loading:
            return
        value = var.get()
        value = ("true" if value else "false") if isinstance(value, bool) else value.strip()
        if self.library.settings[kind].get(name) != value:
            self.library.settings[kind][name] = value
            self._update_problems()
            self.on_change()

    # -------------------------------------------------------------------------------------------------- actions
    def add_object(self, kind):
        """Add an object of a kind (named after the kind) and show it."""
        if len(self.library.objects[kind]) >= MAX_OBJECTS.get(kind, MAX_OTHERS):
            return
        before = set(self.library.names(kind))
        self.library.resize(kind, len(self.library.objects[kind]) + 1)
        name = next(n for n in self.library.names(kind) if n not in before)
        self.selected = ("object", kind, name)
        self.refresh()
        self.on_change()

    def set_count(self, kind, n):
        """Keep the first n objects of a kind, or add new ones up to n (the removed ones used by tasks: asked first)."""
        objects = self.library.objects[kind]
        removed = [o.name for o in objects[n:]]
        if removed and not self._confirm_removal(kind, removed):
            return
        self.library.resize(kind, n)
        if removed:
            self.on_remove(kind, removed)
        self.refresh()
        self.on_change()

    def _confirm_removal(self, kind, names):
        used = {name: self.users(kind, name) for name in names}
        used = {name: tasks for name, tasks in used.items() if tasks}
        return not used or messagebox.askyesno(
            "Filters", "Remove %s used by tasks?\n\n%s" % (
                ", ".join(sorted(used)), "\n".join("%s: %s" % (n_, ", ".join(t)) for n_, t in sorted(used.items()))),
            parent=self)

    def duplicate_selected(self):
        s = self.selected
        if s[0] != "object" or s[1] in F.MAPS_KINDS:
            return
        o = self.library.get(s[1], s[2])
        k = 1
        while self.library.get(s[1], "%s_%d" % (o.name, k)) is not None:
            k += 1
        new = o.copy(o.source)
        new.name = "%s_%d" % (o.name, k)
        objects = self.library.objects[s[1]]
        objects.insert(objects.index(o) + 1, new)
        self.selected = ("object", s[1], new.name)
        self.refresh()
        self.on_change()

    def remove_selected(self):
        s = self.selected
        if s[0] == "link":
            self.unlink(s[1], True)
            return
        if s[0] != "object" or s[1] in F.MAPS_KINDS:
            return
        if not self._confirm_removal(s[1], [s[2]]):
            return
        objects = self.library.objects[s[1]]
        o = self.library.get(s[1], s[2])
        k = objects.index(o)
        objects.remove(o)
        self.on_remove(s[1], [s[2]])
        self.selected = ("object", s[1], objects[min(k, len(objects) - 1)].name) if objects else ("kind", s[1])
        self.refresh()
        self.on_change()

    def _ask_selection_file(self, title, save=False):
        options = dict(parent=self, title=title, initialdir=self.directory(),
                       filetypes=[("Selection files", "*.xml"), ("All files", "*")])
        if sys.platform == "darwin":
            options.pop("filetypes")
        if save:
            return filedialog.asksaveasfilename(defaultextension=".xml", **options)
        return filedialog.askopenfilename(**options)

    def import_file(self, path=None):
        """Copy the objects of a selection file (or of the RUNCAP element of a workflow) into the workflow; objects of
        the same kind and name are replaced, or kept, as asked."""
        path = path or self._ask_selection_file("Import a selection file")
        if not path:
            return
        try:
            objects, _, warnings = F.read_selections(path)
        except (OSError, ValueError) as error:
            messagebox.showerror("Import", str(error), parent=self)
            return
        conflicts = [(o.kind, o.name) for o in objects if self.library.get(o.kind, o.name) is not None]
        names = None
        if conflicts:
            answer = messagebox.askyesnocancel(
                "Import", "%d object%s of %s %s already in the workflow:\n\n%s\n\nReplace them?" % (
                    len(conflicts), "s" if len(conflicts) > 1 else "", os.path.basename(path),
                    "are" if len(conflicts) > 1 else "is",
                    "\n".join("%s %s" % (F.KIND_LABELS[k][:-1].lower(), n) for k, n in conflicts[:12])), parent=self)
            if answer is None:
                return
            if not answer:
                names = {(o.kind, o.name) for o in objects} - set(conflicts)
        self.library.import_objects(objects, names)
        self._report("Imported %d object%s from %s." % (len(objects) if names is None else len(names),
                                                         "s" if (len(objects) if names is None else len(names)) != 1
                                                         else "", path), warnings)
        self.refresh()
        self.on_change()

    def link_file(self, path=None):
        """Link a selection file: its objects are used from it, and saved back to it with the workflow."""
        path = path or self._ask_selection_file("Link a selection file")
        if not path:
            return
        if os.path.abspath(path) in self.library.links:
            self.selected = ("link", os.path.abspath(path))
            self.refresh()
            return
        try:
            warnings = self.library.link(path)
        except (OSError, ValueError) as error:
            messagebox.showerror("Link", str(error), parent=self)
            return
        self.selected = ("link", os.path.abspath(path))
        self._report("Linked %s." % path, warnings)
        self.refresh()
        self.on_change()

    def unlink(self, path, keep):
        """Stop linking a selection file: its objects copied into the workflow (keep), or removed."""
        objects = self.library.linked(path)
        if not keep:
            by_kind = {}
            for o in objects:
                by_kind.setdefault(o.kind, []).append(o.name)
            if any(self.users(k, n) for k, names in by_kind.items() for n in names) and not messagebox.askyesno(
                    "Unlink", "Tasks use objects of %s: remove them from the tasks?" % os.path.basename(path),
                    parent=self):
                return
        removed = self.library.unlink(path, keep)
        for kind, names in removed.items():
            self.on_remove(kind, names)
        self.selected = ("kind", objects[0].kind) if objects else ("kind", "EVENT_FILTER")
        self.refresh()
        self.on_change()

    def _export_objects(self, scope):
        s = self.selected
        everything = [o for o in self.library.all_objects() if o.kind not in F.MAPS_KINDS]
        if scope == "object":
            return [self.library.get(s[1], s[2])] if s[0] == "object" and s[1] not in F.MAPS_KINDS else []
        if scope == "kind":
            kind = s[1] if s[0] in ("kind", "object") else None
            return list(self.library.objects[kind]) if kind and kind not in F.MAPS_KINDS else []
        return everything

    def export_file(self, scope="all", path=None):
        """Write objects to a selection file: the selected object, those of the selected kind, or all of them."""
        objects = self._export_objects(scope)
        if not objects:
            messagebox.showinfo("Export", {"object": "Select an object first.",
                                           "kind": "Select a kind with objects first (the particle efficiencies and "
                                                   "smearers are in the maps file)."}.get(scope, "Nothing to export."),
                                parent=self)
            return
        path = path or self._ask_selection_file("Export %d object%s" % (len(objects), "s" if len(objects) > 1 else ""),
                                                save=True)
        if not path:
            return
        try:
            F.write_selections(path, objects)
        except OSError as error:
            messagebox.showerror("Export", "Cannot write %s:\n%s" % (path, error), parent=self)
            return
        self._report("Exported %d object%s to %s." % (len(objects), "s" if len(objects) > 1 else "", path), [])

    def _report(self, message, warnings):
        self.problems.set("\n".join([message] + warnings[:8]))
        self.after(6000, self._update_problems)

    # -------------------------------------------------------------------------------------------------- objects
    def _make_object(self, parent, definition):
        kind = definition.kind
        ttk.Label(parent, text=F.KIND_LABELS[kind][:-1], style="Editor.TLabel").pack(anchor="w")
        if definition.source:
            line = ttk.Frame(parent)
            line.pack(fill=tk.X, pady=(2, 0))
            ttk.Label(line, text="From the selection file %s (saved there with the workflow)."
                      % os.path.basename(definition.source), foreground="gray40").pack(side=tk.LEFT)
            ttk.Button(line, text="Copy into the workflow",
                       command=lambda: self._copy_into_workflow(definition)).pack(side=tk.LEFT, padx=8)
        frame = ttk.Frame(parent, padding=(0, 6))
        frame.pack(fill=tk.X)
        head = ttk.Frame(frame)
        head.pack(fill=tk.X)
        ttk.Label(head, text="Name:").pack(side=tk.LEFT)
        name = tk.StringVar(self, value=definition.name)
        entry = ttk.Entry(head, textvariable=name, width=22)
        entry.pack(side=tk.LEFT, padx=(4, 14))
        entry.bind("<Return>", lambda e: self._rename(definition, name, entry))
        entry.bind("<FocusOut>", lambda e: self._rename(definition, name, entry))
        ttk.Label(head, text="Title:").pack(side=tk.LEFT)
        title = tk.StringVar(self, value=definition.title)
        ttk.Entry(head, textvariable=title, width=36).pack(side=tk.LEFT, padx=(4, 0), fill=tk.X, expand=True)
        title.trace_add("write", lambda *a: self._set_title(definition, title.get()))
        users = self.users(kind, definition.name)
        ttk.Label(frame, text=("Used by " + ", ".join(users)) if users else "Used by no task.",
                  foreground="gray40").pack(anchor="w", pady=(4, 0))
        if kind in F.CONDITION_KINDS:
            self._make_conditions(frame, definition)
        else:
            self._make_parameters(frame, definition)

    def _copy_into_workflow(self, definition):
        """An object of a linked file becomes an object of the workflow (the file keeps its own)."""
        definition.source = ""
        self.refresh()
        self.on_change()

    def _rename(self, definition, var, entry):
        new = var.get().strip()
        if new == definition.name:
            entry.configure(style="TEntry")
            return
        if not F.NAME_FULL.fullmatch(new) or self.library.get(definition.kind, new) is not None:
            entry.configure(style="Invalid.TEntry")
            self.problems.set("Invalid or duplicate name '%s': names are made of letters, digits, _ . - and are "
                              "unique within a kind." % new)
            return
        entry.configure(style="TEntry")
        old, definition.name = definition.name, new
        self.on_rename(definition.kind, old, new)
        if self.selected == ("object", definition.kind, old):
            self.selected = ("object", definition.kind, new)
        self._fill_tree()
        self._select_in_tree()
        self._update_problems()
        self.on_change()

    def _set_title(self, definition, value):
        if definition.title != value:
            definition.title = value
            iid = self._object_id(definition.kind, definition.name)
            if self.tree.exists(iid):
                self.tree.item(iid, text=definition.name + (" — " + value if value else ""))
            self.on_change()

    def _set(self, target, attribute, value):
        if getattr(target, attribute) != value:
            setattr(target, attribute, value)
            self._update_problems()
            self.on_change()

    def _redraw(self):
        """Remake the editor of the object shown (after a change of its conditions)."""
        self._show_editor()
        self._update_problems()

    # -------------------------------------------------------------------------------------------------- conditions
    def _make_conditions(self, frame, definition):
        kind = definition.kind
        box = ttk.Frame(frame)
        box.pack(fill=tk.X, pady=(6, 0))
        labels = quantities(kind)
        label_of = {s: label for label, s in labels.items()}
        for i, condition in enumerate(definition.conditions):
            row = ttk.Frame(box)
            row.pack(fill=tk.X, pady=1)
            remove = ttk.Button(row, text="✕", width=2,
                                command=lambda c=condition: self._remove_condition(definition, c))
            remove.pack(side=tk.RIGHT)          # first, so that it keeps its place
            quantity = tk.StringVar(self, value=label_of.get(condition.subtype, condition.subtype
                                                             + ("  (earlier version: choose a quantity)"
                                                                if condition.subtype in F.LEGACY_EVENT_SUBTYPES else "")))
            q = ttk.Combobox(row, textvariable=quantity, values=list(labels), width=26, state="readonly")
            q.pack(side=tk.LEFT)
            q.bind("<<ComboboxSelected>>",
                   lambda e, c=condition, v=quantity: self._set_quantity(definition, c, labels[v.get()]))
            forms = forms_of(kind, condition.subtype)
            form = tk.StringVar(self, value=condition.form)
            f = ttk.Combobox(row, textvariable=form, values=list(forms), width=9,
                             state="readonly" if len(forms) > 1 else "disabled")
            f.pack(side=tk.LEFT, padx=6)
            f.bind("<<ComboboxSelected>>", lambda e, c=condition, v=form: self._set_form(definition, c, v.get()))
            if kind == "EVENT_FILTER" and condition.subtype in F.SELECTION_SUBTYPES:
                names = self.library.names("PARTICLE_FILTER")
                ttk.Label(row, text="of").pack(side=tk.LEFT, padx=(2, 2))
                selection = tk.StringVar(self, value=condition.selection)
                s = ttk.Combobox(row, textvariable=selection, values=names, width=12, state="readonly")
                s.pack(side=tk.LEFT, padx=(0, 6))
                s.bind("<<ComboboxSelected>>",
                       lambda e, c=condition, v=selection: self._set(c, "selection", v.get()))
            self._make_fields(row, condition)
        if not definition.conditions:
            ttk.Label(box, text="No condition: the filter accepts every %s."
                      % ("event" if kind == "EVENT_FILTER" else "particle"), foreground="gray40").pack(anchor="w")
        add = ttk.Button(frame, text="+ Condition", command=lambda: self._add_condition(definition))
        add.pack(anchor="w", pady=(4, 2))
        if len(definition.conditions) >= MAX_CONDITIONS:
            add.configure(state=tk.DISABLED)

    def _make_fields(self, row, condition):
        if condition.form == F.FLAG:
            var = tk.StringVar(self, value="true" if condition.minimum else "false")
            box = ttk.Combobox(row, textvariable=var, values=("true", "false"), width=6, state="readonly")
            box.pack(side=tk.LEFT)
            var.trace_add("write", lambda *a: self._set(condition, "minimum", 1.0 if var.get() == "true" else 0.0))
            return
        integer = condition.form == F.VALUE
        names = ("minimum",) if integer else _DOUBLE_FIELDS.get(condition.form, ("minimum", "maximum"))
        for name in names:
            ttk.Label(row, text="value" if integer else _FIELD_LABELS[name]).pack(side=tk.LEFT, padx=(4, 2))
            value = getattr(condition, name)
            var = tk.StringVar(self, value=F.number_text(value))
            entry = ttk.Entry(row, textvariable=var, width=8)
            check = (row.register(lambda text, i=integer: _partial_number(text, i)), "%P")
            entry.configure(validate="key", validatecommand=check)
            entry.pack(side=tk.LEFT)

            def changed(*_args, n=name, v=var, e=entry, i=integer):
                x = _number(v.get(), i)
                e.configure(style="TEntry" if x is not None else "Invalid.TEntry")
                if x is not None:
                    self._set(condition, n, x)
            var.trace_add("write", changed)

    def _add_condition(self, definition):
        kind = definition.kind
        used = set(c.subtype for c in definition.conditions)
        order = list(quantities(kind).values())
        subtype = next((s for s in order if s not in used), order[0])
        form = forms_of(kind, subtype)[0]
        maximum = {"PT": 10.0, "ETA": 1.0, "RAPIDITY": 1.0, "PHI": 6.283185307179586}.get(subtype, 1000.0)
        minimum = -maximum if subtype in ("ETA", "RAPIDITY", "PX", "PY", "PZ") else 0.0
        condition = F.Condition(subtype, form, minimum, maximum)
        self._default_selection(condition)
        definition.conditions.append(condition)
        self._redraw()
        self.on_change()

    def _remove_condition(self, definition, condition):
        definition.conditions.remove(condition)
        self._redraw()
        self.on_change()

    def _default_selection(self, condition):
        """The quantity of a selection: the first particle filter of the library, if none chosen yet."""
        names = self.library.names("PARTICLE_FILTER")
        if condition.subtype in F.SELECTION_SUBTYPES and condition.selection not in names:
            condition.selection = names[0] if names else ""

    def _set_quantity(self, definition, condition, subtype):
        condition.subtype = subtype
        self._default_selection(condition)
        forms = forms_of(definition.kind, subtype)
        if condition.form not in forms:
            condition.form = forms[0]
        self._redraw()
        self.on_change()

    def _set_form(self, definition, condition, form):
        if form != condition.form:
            condition.form = form
            if form == F.DUAL_RANGE and not condition.minimum2 < condition.maximum2:
                condition.minimum2, condition.maximum2 = condition.minimum, condition.maximum
            self._redraw()
            self.on_change()

    # -------------------------------------------------------------------------------------------------- parameters
    def _make_parameters(self, frame, definition):
        grid = ttk.Frame(frame)
        grid.pack(fill=tk.X, pady=(6, 0))
        per_row = 3
        for i, (name, ptype, _) in enumerate(F.KIND_PARAMETERS.get(definition.kind, [])):
            r, c = divmod(i, per_row)
            ttk.Label(grid, text="%s (%s)" % (name, ptype)).grid(row=r, column=2 * c, sticky="w", padx=(0, 4), pady=1)
            var = tk.StringVar(self, value=definition.parameters.get(name, ""))
            if ptype == "bool":
                widget = ttk.Combobox(grid, textvariable=var, values=("true", "false"), width=7, state="readonly")
            else:
                widget = ttk.Entry(grid, textvariable=var, width=22 if ptype == "string" else 10)
                if ptype in ("int", "double"):
                    check = (grid.register(lambda text, t=ptype: _partial_number(text, t == "int")), "%P")
                    widget.configure(validate="key", validatecommand=check)
            widget.grid(row=r, column=2 * c + 1, sticky="w", padx=(0, 18), pady=1)

            def changed(*_args, n=name, t=ptype, v=var, w=widget):
                if t != "bool":
                    w.configure(style="TEntry" if F._valid(t, v.get()) else "Invalid.TEntry")
                definition.parameters[n] = v.get()
                self._update_problems()
                self.on_change()
            var.trace_add("write", changed)

    def _parameter_changed(self):
        self._update_problems()
        self.on_change()

    # -------------------------------------------------------------------------------------------------- problems
    def _update_problems(self):
        errors = F.check_library(self.library)
        more = "\n(and %d more)" % (len(errors) - 6) if len(errors) > 6 else ""
        self.problems.set("\n".join(errors[:6]) + more)


# ------------------------------------------------------------------------------------------------------------------
# Particle efficiencies and smearers: a maps file
# ------------------------------------------------------------------------------------------------------------------

class MapsEditor:
    """The editor of the particle efficiencies and smearers in the Filters pane: the maps file of the workflow (made
    and edited in CapGui, Edit > Detector Maps) and the objects it holds (listed by bin/CAP --maps-list), which the
    tasks choose by name. RunCAP does not edit them: Edit in CapGui opens the file there, Reload reads it again.
    @param panel FiltersPanel (library, callbacks); the editor is drawn in its editor area when shown"""

    def __init__(self, panel):
        self.panel = panel
        self.entries = None                 # [F.MapsEntry] of the file last read
        self.read_path = None               # file last read, and its modification time
        self.read_time = None
        self.frame = frame = ttk.Frame(panel.editor.inner)
        ttk.Label(frame, text="Particle efficiencies and smearers", style="Editor.TLabel").pack(anchor="w")
        ttk.Label(frame, text="They are made and edited in CapGui (Edit > Detector Maps) and saved in a maps file. "
                              "RunCAP reads them from it: each task chooses them by name in its window (one for all "
                              "particles, or one per event filter and particle filter).",
                  foreground="gray40", wraplength=640, justify=tk.LEFT).pack(fill=tk.X, pady=(2, 6))
        line = ttk.Frame(frame)
        line.pack(fill=tk.X)
        ttk.Label(line, text="Maps file:").pack(side=tk.LEFT)
        self.path = tk.StringVar(frame)
        ttk.Entry(line, textvariable=self.path, width=20, state="readonly").pack(side=tk.LEFT, padx=6, fill=tk.X,
                                                                                 expand=True)
        ttk.Button(line, text="Choose…", command=self.choose).pack(side=tk.LEFT)
        ttk.Button(line, text="Reload", command=lambda: self.reload()).pack(side=tk.LEFT, padx=4)
        ttk.Button(line, text="Edit in CapGui…", command=self.open_capgui).pack(side=tk.LEFT)
        ttk.Button(line, text="Clear", command=lambda: self.set_file("")).pack(side=tk.LEFT, padx=(4, 0))
        columns = ("kind", "name", "species", "maps", "used")
        self.table = ttk.Treeview(frame, columns=columns, show="headings", height=8)
        for c, label, width in (("kind", "Kind", 120), ("name", "Name", 140), ("species", "Species", 100),
                                ("maps", "Maps", 260), ("used", "Used by", 140)):
            self.table.heading(c, text=label)
            self.table.column(c, width=width, anchor="w")
        self.table.pack(fill=tk.BOTH, expand=True, pady=(6, 0))
        self.status = tk.StringVar(frame)
        ttk.Label(frame, textvariable=self.status, foreground="firebrick", wraplength=640,
                  justify=tk.LEFT).pack(fill=tk.X, pady=(4, 0))

    @property
    def library(self):
        return self.panel.library

    def show(self, kind=None, name=None):
        """Draw the editor in the editor area, the object (kind, name) selected in the table."""
        self.frame.pack(fill=tk.BOTH, expand=True)
        self.refresh()
        for iid in self.table.get_children():
            if name is not None and self.table.item(iid, "values")[1] == name \
                    and self.table.item(iid, "values")[0] == F.MAPS_LABELS[kind]:
                self.table.selection_set(iid)
                self.table.see(iid)

    def refresh(self):
        """Show the file and its objects."""
        self.path.set(F.maps_file(self.library))
        self.table.delete(*self.table.get_children())
        listed = {(e.kind, e.name): e for e in self.entries or []}
        for kind in F.MAPS_KINDS:
            for name in self.library.names(kind):
                e = listed.get((kind, name))
                users = self.panel.users(kind, name)
                self.table.insert("", tk.END, values=(F.MAPS_LABELS[kind], name, (e.species or "all") if e else "",
                                                      e.maps if e else "(not in the file)", ", ".join(users)))

    def load(self, force=False):
        """Read the objects of the maps file (if it changed, or force) and make them those of the library. @return
        whether the library changed"""
        path = F.maps_file(self.library)
        if not path:
            self.entries, self.read_path, self.read_time = None, None, None
            self.status.set("" if not any(self.library.objects[k] for k in F.MAPS_KINDS)
                            else "Choose the maps file of the efficiencies and smearers.")
            return False
        try:
            mtime = os.path.getmtime(path)
        except OSError:
            mtime = None
        if not force and path == self.read_path and mtime == self.read_time:
            return False
        self.read_path, self.read_time = path, mtime
        try:
            self.entries = F.list_maps(path)
        except RuntimeError as error:
            self.entries = None
            self.status.set(str(error))
            return False
        self.status.set("" if self.entries else "No particle efficiency or smearer in %s." % path)
        before = {k: self.library.names(k) for k in F.MAPS_KINDS}
        removed = F.sync_maps(self.library, self.entries)
        for kind, names in removed.items():
            if names:
                self.panel.on_remove(kind, names)
        changed = before != {k: self.library.names(k) for k in F.MAPS_KINDS}
        if changed:
            self.panel.on_change()
        return changed

    def reload(self):
        self.load(force=True)
        self.panel.refresh()

    def set_file(self, path):
        """Use another maps file ("" for none: the tasks lose their efficiencies and smearers)."""
        if not path:
            used = {(k, n): self.panel.users(k, n) for k in F.MAPS_KINDS for n in self.library.names(k)}
            used = {key: tasks for key, tasks in used.items() if tasks}
            if used and not messagebox.askyesno("Detector maps", "Tasks use efficiencies or smearers of the file: "
                                                "remove them from the tasks?", parent=self.panel):
                return
            for kind in F.MAPS_KINDS:
                names = self.library.names(kind)
                self.library.objects[kind] = []
                if names:
                    self.panel.on_remove(kind, names)
        F.set_maps_file(self.library, path)
        self.read_path = None
        self.panel.refresh()
        self.panel.on_change()

    def choose(self):
        current = F.maps_file(self.library)
        root = os.path.dirname(os.path.abspath(__file__))
        options = dict(parent=self.panel, title="Maps file (efficiencies and smearers)",
                       initialdir=os.path.dirname(current) if current else root,
                       filetypes=[("CAP files", "*.cap *.root"), ("All files", "*")])
        if sys.platform == "darwin":
            options.pop("filetypes")
        name = filedialog.askopenfilename(**options)
        if name:
            self.set_file(name)

    def open_capgui(self):
        """Open the maps file (or a new one) in the Detector Maps window of CapGui (bin/CapGui --maps)."""
        root = os.path.dirname(os.path.abspath(__file__))
        program = os.path.join(root, "bin", "CapGui")
        if not os.path.isfile(program):
            messagebox.showerror("CapGui", "bin/CapGui not found: build CAP with the GUI first (BuildCAP).",
                                 parent=self.panel)
            return
        path = F.maps_file(self.library)
        command = [program, "--maps", path] if path else [program]
        if not path:
            self.status.set("In CapGui: Edit > Detector Maps, then save the maps file and choose it here.")
        try:
            subprocess.Popen(command, cwd=root)
        except OSError as error:
            messagebox.showerror("CapGui", "Cannot start CapGui: %s" % error, parent=self.panel)
