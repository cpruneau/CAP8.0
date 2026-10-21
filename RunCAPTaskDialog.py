# **********************************************************************
# Copyright (C) 2019-2026, Claude Pruneau
# All rights reserved.
#
# For the licensing terms see LICENSE.
# **********************************************************************
"""
RunCAPTaskDialog -- the window of a task of a RunCAP.py workflow.

Tabs:
  * General: name, report level, class, and the parameters of the task (the keys of its filters and histogram folders
    are written from the choices below, see RunCAPFilters);
  * Filters (tasks that use filters, efficiencies or smearers): for each kind the task supports, how many objects it
    uses and which ones, by name, among those of the library of the workflow (Filters pane);
  * Histograms (analyzers): the prefix of the names of the folders, one folder per combination of the filters chosen
    (<prefix>_<event filter>_<particle filter>[_<particle filter 2>], derived folders ending with _D), and their
    binning: the same for all (the HISTOGRAM group of the task), or folder by folder, the parameters not set for a
    folder being those of the HISTOGRAM group;
  * Histogram files (analyzers): creation, import and export of the lists of folders (HISTOGRAM_1 to HISTOGRAM_4).
Changes apply to the task at once; the problems of the task (RunCAPFilters.check_choices()) are listed at the bottom.
"""

import tkinter as tk
from tkinter import ttk

import RunCAPFilters as F
from RunCAPFiltersGui import ScrollFrame

#: Label of the default binning in the list of folders.
DEFAULT_ENTRY = "All folders (HISTOGRAM group)"


def general_parameters(task, templates):
    """Parameters of the General tab: all but the keys managed by RunCAPFilters, the settings of the lists of
    filters, efficiencies and smearers (FilterCreator creates, imports and exports them) and, for analyzers, the
    histogram groups (Histograms and Histogram files tabs)."""
    analyzer = F.histogram_layout(task, templates) is not None
    return [p for p in task.params if not F.MANAGED_KEY.match(p.key) and not F.LIST_SETTING_KEY.match(p.key)
            and not (analyzer and (p.key.startswith("HISTOGRAM:") or _is_list_key(p.key)))]


def _is_list_key(key):
    return any(key.startswith("HISTOGRAM_%d:" % l) for l in (1, 2, 3, 4)) and not F.MANAGED_KEY.match(key)


def file_parameters(task):
    """Parameters of the lists of folders (HISTOGRAM_1 to HISTOGRAM_4: USE, CREATE, IMPORT, EXPORT, files)."""
    return [p for p in task.params if _is_list_key(p.key)]


class TaskDialog(tk.Toplevel):
    """Window of a task.
    @param task RunCAP.TaskConfig edited in place
    @param templates task templates; @param library RunCAPFilters.FilterLibrary of the workflow
    @param helpers {"valid_value", "partial_value", "severities"} of RunCAP.py
    @param rename called with (task, new name): None if renamed, else the reason it is not
    @param changed called after any change; @param show_filters called with a kind to show the Filters pane"""

    def __init__(self, master, task, templates, library, helpers, rename, changed, show_filters):
        tk.Toplevel.__init__(self, master)
        self.task, self.templates, self.library = task, templates, library
        self.helpers, self.rename_task, self.changed, self.show_filters = helpers, rename, changed, show_filters
        self.geometry("1040x700")
        self.minsize(820, 480)
        style = ttk.Style()
        style.configure("Invalid.TEntry", foreground="firebrick")
        style.configure("Invalid.TCombobox", foreground="firebrick")
        style.configure("Overridden.TLabel", font=("TkDefaultFont", 11, "bold"))
        self.notebook = ttk.Notebook(self, padding=4)
        self.notebook.pack(fill=tk.BOTH, expand=True, padx=6, pady=(6, 0))
        self.layout = F.histogram_layout(task, templates)
        self.kinds = F.task_kinds(task, templates)
        self.general = ttk.Frame(self.notebook, padding=6)
        self.notebook.add(self.general, text="General")
        self.filters_box = self.histograms_tab = self.files_tab = None
        if self.layout:
            self.histograms_tab = ttk.Frame(self.notebook, padding=6)
            self.notebook.add(self.histograms_tab, text="Histograms")
            self.files_tab = ttk.Frame(self.notebook, padding=6)
            self.notebook.add(self.files_tab, text="Histogram files")
        bottom = ttk.Frame(self, padding=(8, 4, 8, 8))
        bottom.pack(fill=tk.X)
        ttk.Button(bottom, text="Close", command=self.destroy).pack(side=tk.RIGHT)
        self.problems = tk.StringVar(self)
        ttk.Label(bottom, textvariable=self.problems, foreground="firebrick", justify=tk.LEFT,
                  wraplength=860).pack(side=tk.LEFT, fill=tk.X, expand=True)
        self._names = None
        self._make_general()
        self._make_files()
        self.refresh()

    # -------------------------------------------------------------------------------------------------- common
    def refresh(self):
        """Remake the parts that depend on the library and the choices (Filters, Histograms) and the title."""
        self.title("CAP Workflow — task %s (%s)" % (self.task.name, self.task.cls))
        self._names = {k: self.library.names(k) for k in F.KINDS}
        if self.filters_box is not None:
            self._make_filters()
        if self.histograms_tab is not None:
            self._make_histograms()
        self._update_problems()

    def library_changed(self):
        """Called when the library changed: remake the menus if the names changed."""
        if self._names != {k: self.library.names(k) for k in F.KINDS}:
            self.refresh()
        else:
            self._update_problems()

    def _update_problems(self):
        errors = F.check_choices([self.task], self.library, self.templates)
        mine = [e for e in errors if ("Task %s " % self.task.name) in e or ("Task %s:" % self.task.name) in e]
        more = "\n(and %d more)" % (len(mine) - 4) if len(mine) > 4 else ""
        self.problems.set("\n".join(mine[:4]) + more)

    def _changed(self):
        self._update_problems()
        self.changed()

    def _param_row(self, parent, row, p, label=None):
        """Label and editor of a parameter (true/false menu for booleans, numbers checked as typed)."""
        valid, partial = self.helpers["valid_value"], self.helpers["partial_value"]
        ttk.Label(parent, text=label or "%s  (%s)" % (p.key, p.type)).grid(row=row, column=0, sticky="w",
                                                                          padx=(0, 8), pady=1)
        var = tk.StringVar(self, value=p.value)
        if p.type == "bool":
            widget = ttk.Combobox(parent, textvariable=var, values=("true", "false"), state="readonly", width=8)
            widget.grid(row=row, column=1, sticky="w", pady=1)
        else:
            widget = ttk.Entry(parent, textvariable=var)
            if p.type in ("int", "long", "double"):
                check = (parent.register(lambda text, t=p.type: partial(t, text)), "%P")
                widget.configure(validate="key", validatecommand=check, width=18)
                widget.grid(row=row, column=1, sticky="w", pady=1)
            else:
                widget.grid(row=row, column=1, sticky="ew", pady=1)
        p.widget_var = var

        def changed(*_args):
            if p.value != var.get():
                p.value = var.get()
                self._changed()
            if p.type != "bool":
                widget.configure(style="TEntry" if valid(p.type, p.value) else "Invalid.TEntry")
        var.trace_add("write", changed)
        if p.type != "bool" and not valid(p.type, p.value):
            widget.configure(style="Invalid.TEntry")
        return widget

    def _columns(self, parent, params):
        if not params:
            ttk.Label(parent, text="No other parameters.", foreground="gray40").pack(anchor="w", pady=4)
            return
        columns = ttk.Frame(parent)
        columns.pack(fill=tk.BOTH, expand=True)
        columns.columnconfigure(0, weight=1, uniform="col")
        columns.columnconfigure(1, weight=1, uniform="col")
        columns.rowconfigure(0, weight=1)
        half = (len(params) + 1) // 2
        for c, column in enumerate((params[:half], params[half:])):
            frame = ScrollFrame(columns)
            frame.grid(row=0, column=c, sticky="nsew", padx=(0, 6) if c == 0 else (6, 0))
            frame.inner.columnconfigure(1, weight=1)
            for r, p in enumerate(column):
                self._param_row(frame.inner, r, p)

    # -------------------------------------------------------------------------------------------------- general
    def _make_general(self):
        task = self.task
        head = ttk.Frame(self.general)
        head.pack(fill=tk.X, pady=(0, 6))
        ttk.Label(head, text="Task name:").pack(side=tk.LEFT)
        self.name_var = tk.StringVar(self, value=task.name)
        entry = ttk.Entry(head, textvariable=self.name_var, width=24)
        entry.pack(side=tk.LEFT, padx=(4, 12))
        entry.bind("<Return>", lambda e: self._rename(entry))
        entry.bind("<FocusOut>", lambda e: self._rename(entry))
        ttk.Label(head, text="Report level:").pack(side=tk.LEFT)
        severity = tk.StringVar(self, value=task.severity)
        box = ttk.Combobox(head, textvariable=severity, values=self.helpers["severities"], width=9, state="readonly")
        box.pack(side=tk.LEFT, padx=(4, 12))
        box.bind("<<ComboboxSelected>>", lambda e: self._set_severity(severity.get()))
        ttk.Label(head, text="Class: " + task.cls + ("" if task.known else "   (not in the current build)"),
                  foreground="gray35" if task.known else "firebrick").pack(side=tk.LEFT)
        ttk.Button(head, text="Defaults", command=self.reset).pack(side=tk.RIGHT)
        params = general_parameters(task, self.templates)
        ttk.Label(head, text="%d parameters" % len(params), foreground="gray40").pack(side=tk.RIGHT, padx=8)
        if self.kinds:
            self.filters_box = ttk.LabelFrame(self.general, text="  Used by this task (created by FilterCreator, "
                                                                 "defined in the Filters pane)  ", padding=(8, 4))
            self.filters_box.pack(fill=tk.X, pady=(0, 8))
        self._columns(self.general, params)

    def _rename(self, entry):
        new = self.name_var.get().strip()
        if new == self.task.name:
            entry.configure(style="TEntry")
            return
        error = self.rename_task(self.task, new)
        if error:
            entry.configure(style="Invalid.TEntry")
            self.problems.set(error)
            return
        entry.configure(style="TEntry")
        self.refresh()

    def _set_severity(self, value):
        if self.task.severity != value:
            self.task.severity = value
            self._changed()

    def reset(self):
        """Set the parameters of the task to their default values (its choices are kept)."""
        for p in self.task.params:
            if hasattr(p, "widget_var"):
                p.widget_var.set(p.default)
            else:
                p.value = p.default
        self._changed()

    # -------------------------------------------------------------------------------------------------- filters
    def _make_filters(self):
        """One line per kind the task supports: how many it uses, then a menu of names for each."""
        box = self.filters_box
        for child in box.winfo_children():
            child.destroy()
        box.columnconfigure(2, weight=1)
        choices = F.choices(self.task)
        for r, kind in enumerate(self.kinds):
            names = self.library.names(kind)
            chosen = list(choices.get(kind, []))
            ttk.Label(box, text=F.KIND_LABELS[kind] + ":").grid(row=r, column=0, sticky="nw", padx=(0, 6), pady=3)
            limit = len(set(names) | set(chosen))
            if kind == "PARTICLE_FILTER":
                limit = min(limit, F.MAX_PARTICLE_FILTERS)
            count = tk.StringVar(self, value=str(len(chosen)))
            menu = ttk.Combobox(box, textvariable=count, values=[str(n) for n in range(max(limit, len(chosen)) + 1)],
                                width=3, state="readonly")
            menu.grid(row=r, column=1, sticky="nw", pady=2)
            menu.bind("<<ComboboxSelected>>", lambda e, k=kind, v=count: self._set_count(k, int(v.get())))
            line = ttk.Frame(box)
            line.grid(row=r, column=2, sticky="ew", padx=(10, 0))
            if not names and not chosen:
                ttk.Label(line, text="none defined", foreground="gray40").pack(side=tk.LEFT, pady=3)
                ttk.Button(line, text="Filters pane…", command=lambda k=kind: self.show_filters(k)).pack(
                    side=tk.LEFT, padx=8)
            for k, name in enumerate(chosen):
                ttk.Label(line, text="%d" % (k + 1)).grid(row=k // 4, column=2 * (k % 4), sticky="e", padx=(8, 3))
                var = tk.StringVar(self, value=name)
                name_menu = ttk.Combobox(line, textvariable=var, values=names, width=16, state="readonly")
                name_menu.grid(row=k // 4, column=2 * (k % 4) + 1, sticky="w", pady=2)
                if name not in names:
                    name_menu.configure(style="Invalid.TCombobox")
                name_menu.bind("<<ComboboxSelected>>",
                               lambda e, kd=kind, i=k, v=var: self._set_choice(kd, i, v.get()))

    def _set_count(self, kind, n):
        choices = F.choices(self.task)
        chosen = list(choices.get(kind, []))
        names = self.library.names(kind)
        del chosen[n:]
        for name in names:
            if len(chosen) >= n:
                break
            if name not in chosen:
                chosen.append(name)
        if chosen:
            choices[kind] = chosen
        else:
            choices.pop(kind, None)
        self.refresh()
        self._changed()

    def _set_choice(self, kind, index, name):
        chosen = F.choices(self.task)[kind]
        if chosen[index] != name:
            chosen[index] = name
            self.refresh()
            self._changed()

    # -------------------------------------------------------------------------------------------------- histograms
    def _make_histograms(self):
        tab = self.histograms_tab
        selected = getattr(self, "_selected_folder", None)
        for child in tab.winfo_children():
            child.destroy()
        options = F.histogram_options(self.task)
        all_folders = F.folders(self.task, self.templates)
        base = [f for f in all_folders if not f.derived]
        head = ttk.Frame(tab)
        head.pack(fill=tk.X, pady=(0, 6))
        ttk.Label(head, text="Prefix of the folder names:").pack(side=tk.LEFT)
        prefix = tk.StringVar(self, value=options.prefix)
        entry = ttk.Entry(head, textvariable=prefix, width=18)
        entry.pack(side=tk.LEFT, padx=(4, 14))
        entry.bind("<Return>", lambda e: self._set_prefix(prefix.get().strip(), entry))
        entry.bind("<FocusOut>", lambda e: self._set_prefix(prefix.get().strip(), entry))
        same = tk.BooleanVar(self, value=options.same_binning)
        ttk.Checkbutton(head, text="Same binning for all folders", variable=same,
                        command=lambda: self._set_same(same.get())).pack(side=tk.LEFT)
        ttk.Label(head, text="%d folders (%d of histograms, %d of derived histograms)"
                  % (len(all_folders), len(base), len(all_folders) - len(base)),
                  foreground="gray40").pack(side=tk.RIGHT)
        body = ttk.Frame(tab)
        body.pack(fill=tk.BOTH, expand=True)
        left = ttk.Frame(body)
        left.pack(side=tk.LEFT, fill=tk.Y)
        ttk.Label(left, text="Binning of:", style="Section.TLabel").pack(anchor="w")
        box = ttk.Frame(left)
        box.pack(fill=tk.BOTH, expand=True)
        self.folder_list = tk.Listbox(box, width=34, exportselection=False, activestyle="none")
        bar = ttk.Scrollbar(box, orient=tk.VERTICAL, command=self.folder_list.yview)
        self.folder_list.configure(yscrollcommand=bar.set)
        self.folder_list.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        bar.pack(side=tk.RIGHT, fill=tk.Y)
        self._list_entries = [None]
        self.folder_list.insert(tk.END, DEFAULT_ENTRY)
        if not options.same_binning:
            for f in base:
                mark = " •" if options.binning.get(f.combination) else ""
                self.folder_list.insert(tk.END, f.name + mark)
                self._list_entries.append(f.combination)
        ttk.Label(left, text="Derived folders (%s) use the\nbinning of their folder; •: own binning."
                  % F.DERIVED_SUFFIX, foreground="gray40", justify=tk.LEFT).pack(anchor="w", pady=(4, 0))
        if not base:
            ttk.Label(left, text="No folder: choose event and\nparticle filters (Filters tab).",
                      foreground="firebrick", justify=tk.LEFT).pack(anchor="w", pady=(4, 0))
        self.binning_area = ttk.Frame(body)
        self.binning_area.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(12, 0))
        index = self._list_entries.index(selected) if selected in self._list_entries else 0
        self.folder_list.selection_set(index)
        self.folder_list.bind("<<ListboxSelect>>", lambda e: self._show_binning())
        self._show_binning()

    def _set_prefix(self, prefix, entry):
        options = F.histogram_options(self.task)
        if prefix == options.prefix:
            return
        if not F.NAME_FULL.fullmatch(prefix):
            entry.configure(style="Invalid.TEntry")
            self.problems.set("Invalid prefix '%s': letters, digits, _ . - (no spaces or colons)." % prefix)
            return
        options.prefix = prefix
        self._make_histograms()
        self._changed()

    def _set_same(self, same):
        F.histogram_options(self.task).same_binning = same
        self._make_histograms()
        self._changed()

    def _show_binning(self):
        for child in self.binning_area.winfo_children():
            child.destroy()
        selection = self.folder_list.curselection()
        combination = self._list_entries[selection[0]] if selection else None
        self._selected_folder = combination
        scroll = ScrollFrame(self.binning_area)
        scroll.pack(fill=tk.BOTH, expand=True)
        inner = scroll.inner
        inner.columnconfigure(2, weight=1)          # the notes of the defaults next to the fields
        keys = F.binning_keys(self.task, self.templates)
        if combination is None:
            ttk.Label(inner, text="Binning of all the folders (HISTOGRAM group of the task)",
                      style="Section.TLabel").grid(row=0, column=0, columnspan=3, sticky="w", pady=(0, 4))
            for r, key in enumerate(keys):
                p = self.task.param("HISTOGRAM:" + key)
                if p is not None:
                    self._param_row(inner, r + 1, p, label="%s  (%s)" % (key, p.type))
            return
        options = F.histogram_options(self.task)
        head = ttk.Frame(inner)
        head.grid(row=0, column=0, columnspan=3, sticky="ew", pady=(0, 4))
        ttk.Label(head, text="Binning of the folders of " + " × ".join(combination),
                  style="Section.TLabel").pack(side=tk.LEFT)
        ttk.Button(head, text="Use the defaults", command=lambda: self._clear_folder(combination)).pack(side=tk.RIGHT)
        overrides = options.binning.get(combination, {})
        valid, partial = self.helpers["valid_value"], self.helpers["partial_value"]
        for r, key in enumerate(keys):
            p = self.task.param("HISTOGRAM:" + key)
            if p is None:
                continue
            own = key in overrides
            label = ttk.Label(inner, text="%s  (%s)" % (key, p.type), style="Overridden.TLabel" if own else "TLabel")
            label.grid(row=r + 1, column=0, sticky="w", padx=(0, 8), pady=1)
            var = tk.StringVar(self, value=overrides.get(key, p.value))
            if p.type == "bool":
                widget = ttk.Combobox(inner, textvariable=var, values=("true", "false"), state="readonly", width=8)
            else:
                widget = ttk.Entry(inner, textvariable=var, width=18)
                if p.type in ("int", "long", "double"):
                    check = (inner.register(lambda text, t=p.type: partial(t, text)), "%P")
                    widget.configure(validate="key", validatecommand=check)
            widget.grid(row=r + 1, column=1, sticky="w", pady=1)
            note = ttk.Label(inner, text="" if not own else "default: " + p.value, foreground="gray40")
            note.grid(row=r + 1, column=2, sticky="w", padx=8)

            def changed(*_args, k=key, v=var, w=widget, lab=label, n=note, default=p.value, t=p.type):
                value = v.get()
                if t != "bool":
                    w.configure(style="TEntry" if valid(t, value) else "Invalid.TEntry")
                    if not valid(t, value):
                        return
                folder = options.binning.setdefault(combination, {})
                if value == default:
                    folder.pop(k, None)
                else:
                    folder[k] = value
                if not folder:
                    options.binning.pop(combination, None)
                own_ = k in folder
                lab.configure(style="Overridden.TLabel" if own_ else "TLabel")
                n.configure(text="default: " + default if own_ else "")
                self._mark_folder(combination)
                self._changed()
            var.trace_add("write", changed)

    def _mark_folder(self, combination):
        if combination not in self._list_entries:
            return
        i = self._list_entries.index(combination)
        text = self.folder_list.get(i).rstrip(" •")
        if F.histogram_options(self.task).binning.get(combination):
            text += " •"
        self.folder_list.delete(i)
        self.folder_list.insert(i, text)
        self.folder_list.selection_set(i)

    def _clear_folder(self, combination):
        F.histogram_options(self.task).binning.pop(combination, None)
        self._mark_folder(combination)
        self._show_binning()
        self._changed()

    # -------------------------------------------------------------------------------------------------- files
    def _make_files(self):
        if self.files_tab is None:
            return
        ttk.Label(self.files_tab, text="Lists of folders: HISTOGRAM_1 (histograms), HISTOGRAM_2 (derived histograms)%s."
                  " Set CREATE (or IMPORT) for the histograms to be filled, EXPORT and a file name to save them."
                  % (", HISTOGRAM_3 and HISTOGRAM_4 (the same for single particles)" if self.layout == "pair" else ""),
                  foreground="gray40", wraplength=960, justify=tk.LEFT).pack(anchor="w", pady=(0, 6))
        self._columns(self.files_tab, file_parameters(self.task))
