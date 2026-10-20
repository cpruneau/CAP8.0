# **********************************************************************
# Copyright (C) 2019-2026, Claude Pruneau
# All rights reserved.
#
# For the licensing terms see LICENSE.
# **********************************************************************
"""
RunCAPFiltersGui -- the Filters pane of the main window of RunCAP.py: the library of filters, efficiencies and smearers of a workflow.

One tab per kind (event filters, particle filters, event and particle efficiencies, event and particle smearers). In
each tab, a menu sets how many objects of that kind the library holds, and each object has a frame with its name and
title, and its conditions (filters) or its binning parameters (efficiencies and smearers). A condition tests one
quantity: a range [minimum, maximum), two ranges, a value (PDG code, charge, ...) or a flag (live, enabled), only the
tests the C++ code accepts for that quantity being offered. The problems of the library (RunCAPFilters.check_library())
are listed at the bottom.

The tasks choose objects by name (RunCAPFilters): renaming an object renames it in the choices of the tasks, and
removing objects removes them from the choices (after a confirmation when tasks use them), through the callbacks given
to FiltersPanel.
"""

import tkinter as tk
from tkinter import messagebox, ttk

import RunCAPFilters as F

#: Largest number of objects of a kind offered by the count menu.
MAX_OBJECTS = {"EVENT_FILTER": 20, "PARTICLE_FILTER": 20}
MAX_OTHERS = 10
#: Largest number of conditions of a filter.
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
        items = [(s, d) for s, (d, _) in F.EVENT_SUBTYPES.items()]
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
    """The Filters pane: a notebook with one tab per kind of the library.
    @param library RunCAPFilters.FilterLibrary edited in place
    @param on_change called after any change of the library
    @param on_rename called with (kind, old name, new name) when an object is renamed
    @param on_remove called with (kind, names) when objects are removed
    @param users called with (kind, name), returns the names of the tasks that use an object"""

    def __init__(self, parent, library, on_change=None, on_rename=None, on_remove=None, users=None):
        ttk.Frame.__init__(self, parent, padding=6)
        self.library = library
        self.on_change = on_change or (lambda: None)
        self.on_rename = on_rename or (lambda kind, old, new: None)
        self.on_remove = on_remove or (lambda kind, names: None)
        self.users = users or (lambda kind, name: [])
        style = ttk.Style()
        style.configure("Invalid.TEntry", foreground="firebrick")
        style.configure("Object.TLabelframe.Label", font=("TkDefaultFont", 11, "bold"))
        self.notebook = ttk.Notebook(self)
        self.notebook.pack(fill=tk.BOTH, expand=True)
        self.tabs, self.counts, self.scrolls = {}, {}, {}
        for kind in F.KINDS:
            self._make_tab(kind)
        self.problems = tk.StringVar(self)
        ttk.Label(self, textvariable=self.problems, foreground="firebrick", justify=tk.LEFT,
                  wraplength=760).pack(fill=tk.X, pady=(6, 0))
        self.refresh()

    # -------------------------------------------------------------------------------------------------- tabs
    def _make_tab(self, kind):
        tab = ttk.Frame(self.notebook, padding=6)
        self.notebook.add(tab, text=F.KIND_LABELS[kind])
        head = ttk.Frame(tab)
        head.pack(fill=tk.X, pady=(0, 6))
        ttk.Label(head, text="Number of %s:" % F.KIND_LABELS[kind].lower()).pack(side=tk.LEFT)
        count = tk.StringVar(self)
        box = ttk.Combobox(head, textvariable=count, width=4, state="readonly")
        box.pack(side=tk.LEFT, padx=6)
        box.bind("<<ComboboxSelected>>", lambda e, k=kind: self.set_count(k, int(self.counts[k].get())))
        hint = {"EVENT_FILTER": "Each filter selects events with all its conditions.",
                "PARTICLE_FILTER": "Each filter selects particles with all its conditions (at most %d per task)."
                                   % F.MAX_PARTICLE_FILTERS}.get(kind, "Binning of the corrections, and the base "
                                                                       "name of their histograms.")
        ttk.Label(head, text=hint, foreground="gray40").pack(side=tk.LEFT, padx=12)
        scroll = ScrollFrame(tab)
        scroll.pack(fill=tk.BOTH, expand=True)
        scroll.inner.columnconfigure(0, weight=1)
        self.tabs[kind], self.counts[kind], self.scrolls[kind] = tab, count, scroll
        count.box = box

    def refresh(self, kinds=None):
        """Remake the content of the tabs from the library (all, or those of the given kinds)."""
        for kind in kinds or F.KINDS:
            objects = self.library.objects[kind]
            limit = max(MAX_OBJECTS.get(kind, MAX_OTHERS), len(objects))
            self.counts[kind].box.configure(values=[str(n) for n in range(limit + 1)])
            self.counts[kind].set(str(len(objects)))
            scroll = self.scrolls[kind]
            position = scroll.canvas.yview()[0]
            for child in scroll.inner.winfo_children():
                child.destroy()
            for k, definition in enumerate(objects):
                self._make_object(scroll.inner, k, definition)
            if not objects:
                ttk.Label(scroll.inner, text="No %s: choose how many above." % F.KIND_LABELS[kind].lower(),
                          foreground="gray40").grid(row=0, column=0, pady=20)
            scroll.update_idletasks()
            scroll.canvas.yview_moveto(position)
            n = len(objects)
            self.notebook.tab(self.tabs[kind], text=F.KIND_LABELS[kind] + (" (%d)" % n if n else ""))
        self._update_problems()

    def select(self, kind):
        self.notebook.select(self.tabs[kind])

    def set_count(self, kind, n):
        """Keep the first n objects of a kind, or add new ones up to n."""
        objects = self.library.objects[kind]
        removed = [o.name for o in objects[n:]]
        used = {name: self.users(kind, name) for name in removed}
        used = {name: tasks for name, tasks in used.items() if tasks}
        if used and not messagebox.askyesno(
                "Filters", "Remove %s used by tasks?\n\n%s" % (
                    ", ".join(sorted(used)), "\n".join("%s: %s" % (n_, ", ".join(t)) for n_, t in sorted(used.items()))),
                parent=self):
            self.counts[kind].set(str(len(objects)))
            return
        self.library.resize(kind, n)
        if removed:
            self.on_remove(kind, removed)
        self.refresh([kind])
        self.on_change()

    # -------------------------------------------------------------------------------------------------- objects
    def _make_object(self, parent, k, definition):
        kind = definition.kind
        frame = ttk.LabelFrame(parent, text="  %d  " % (k + 1), style="Object.TLabelframe", padding=(8, 4))
        frame.grid(row=k, column=0, sticky="ew", pady=(0, 8), padx=(0, 4))
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
        title.trace_add("write", lambda *a: self._set(definition, "title", title.get()))
        users = self.users(kind, definition.name)
        if users:
            ttk.Label(head, text="used by " + ", ".join(users), foreground="gray40").pack(side=tk.LEFT, padx=10)
        if kind in F.CONDITION_KINDS:
            self._make_conditions(frame, definition)
        else:
            self._make_parameters(frame, definition)

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
        self._update_problems()
        self.on_change()

    def _set(self, target, attribute, value):
        if getattr(target, attribute) != value:
            setattr(target, attribute, value)
            self._update_problems()
            self.on_change()

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
            quantity = tk.StringVar(self, value=label_of.get(condition.subtype, condition.subtype))
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
        definition.conditions.append(F.Condition(subtype, form, minimum, maximum))
        self.refresh([kind])
        self.on_change()

    def _remove_condition(self, definition, condition):
        definition.conditions.remove(condition)
        self.refresh([definition.kind])
        self.on_change()

    def _set_quantity(self, definition, condition, subtype):
        condition.subtype = subtype
        forms = forms_of(definition.kind, subtype)
        if condition.form not in forms:
            condition.form = forms[0]
        self.refresh([definition.kind])
        self.on_change()

    def _set_form(self, definition, condition, form):
        if form != condition.form:
            condition.form = form
            if form == F.DUAL_RANGE and not condition.minimum2 < condition.maximum2:
                condition.minimum2, condition.maximum2 = condition.minimum, condition.maximum
            self.refresh([definition.kind])
            self.on_change()

    # -------------------------------------------------------------------------------------------------- parameters
    def _make_parameters(self, frame, definition):
        grid = ttk.Frame(frame)
        grid.pack(fill=tk.X, pady=(6, 0))
        per_row = 3
        for i, (name, ptype, _) in enumerate(F.KIND_PARAMETERS[definition.kind]):
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

    # -------------------------------------------------------------------------------------------------- problems
    def _update_problems(self):
        errors = F.check_library(self.library)
        more = "\n(and %d more)" % (len(errors) - 6) if len(errors) > 6 else ""
        self.problems.set("\n".join(errors[:6]) + more)
