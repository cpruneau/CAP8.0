/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file CapApp.cpp
//! @brief Implementation of CapApp.
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <sstream>
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "FileSystem.hpp"
#include "CapApp.hpp"
#include "XmlDocument.hpp"
#include "Exceptions.hpp"
#include "Normal.hpp"
#include "Exponential.hpp"
#include "DataPointsVector.hpp"
#include "Histogram.hpp"
#include "CapFile.hpp"
#include "LatexRenderer.hpp"
#include "GraphEditor.hpp"

#ifndef CAP_SOURCE_ROOT
//! Top directory of the CAP sources (set by the build; empty if not).
#define CAP_SOURCE_ROOT ""
#endif

namespace fs = std::filesystem;

namespace CAP
{
  namespace GUI
  {
  using namespace GRAPHICS;

  // ------------------------------------------------------------------------------------------------ configuration

  namespace
  {
  std::vector<String> split(const String & text)
  {
  std::vector<String> words;
  std::istringstream in(text.str());
  std::string word;
  while (in >> word) words.push_back(String(word));
  return words;
  }
  } // namespace

  bool AppConfiguration::read(const String & fileName, String & error)
  {
  try
    {
    XmlDocument document = XmlDocument::read(fileName);
    const XmlElement & root = document.root();
    if (!root.isNamed("CapGui")) { error = "Root element is not CapGui"; return false; }
    if (const XmlElement * e = root.child("Window"))
      {
      gui.title     = e->attribute("title", gui.title);
      gui.width     = int(e->attributeInt("width", gui.width));
      gui.height    = int(e->attributeInt("height", gui.height));
      gui.theme     = e->attribute("theme", gui.theme);
      gui.fontSize  = float(e->attributeDouble("fontSize", gui.fontSize));
      gui.fontFile  = e->attribute("font", gui.fontFile);
      gui.viewports = e->attributeBool("viewports", gui.viewports);
      }
    if (const XmlElement * e = root.child("Canvases"))
      {
      gui.columns  = int(e->attributeInt("columns", gui.columns));
      gui.rows     = int(e->attributeInt("rows", gui.rows));
      gui.toolbars = e->attributeBool("toolbars", gui.toolbars);
      }
    if (const XmlElement * e = root.child("Graphs"))
      {
      graphColumns = int(e->attributeInt("columns", graphColumns));
      graphRows    = int(e->attributeInt("rows", graphRows));
      legend       = e->attributeBool("legend", legend);
      logX         = e->attributeBool("logX", logX);
      logY         = e->attributeBool("logY", logY);
      mode         = e->attribute("mode", mode);
      }
    if (const XmlElement * e = root.child("Files"))
      {
      directory = e->attribute("directory", directory);
      if (e->hasAttribute("extensions")) extensions = split(e->attribute("extensions"));
      }
    if (const XmlElement * e = root.child("Browser"))
      {
      gui.browserVisible = e->attributeBool("visible", gui.browserVisible);
      gui.browserDocked  = e->attributeBool("docked", gui.browserDocked);
      }
    if (const XmlElement * e = root.child("Demo")) demo = e->attributeBool("enabled", demo);
    if (const XmlElement * e = root.child("Latex"))
      {
      latex         = e->attributeBool("enabled", latex);
      latexBin      = e->attribute("bin", latexBin);
      latexPreamble = e->attribute("preamble", latexPreamble);
      rootSyntax    = e->attributeBool("rootSyntax", rootSyntax);
      }
    }
  catch (const std::exception & exception)
    {
    error = exception.what();
    return false;
    }
  return true;
  }

  bool AppConfiguration::write(const String & fileName) const
  {
  try
    {
    XmlDocument document("CapGui");
    XmlElement & root = document.root();
    root.setAttribute("version", 1);
    XmlElement & window = root.addChild("Window");
    window.setAttribute("title", gui.title);
    window.setAttribute("width", gui.width);
    window.setAttribute("height", gui.height);
    window.setAttribute("theme", gui.theme);
    window.setAttribute("fontSize", double(gui.fontSize));
    if (!gui.fontFile.isNull()) window.setAttribute("font", gui.fontFile);
    window.setAttribute("viewports", gui.viewports);
    XmlElement & canvases = root.addChild("Canvases");
    canvases.setAttribute("columns", gui.columns);
    canvases.setAttribute("rows", gui.rows);
    canvases.setAttribute("toolbars", gui.toolbars);
    XmlElement & graphs = root.addChild("Graphs");
    graphs.setAttribute("columns", graphColumns);
    graphs.setAttribute("rows", graphRows);
    graphs.setAttribute("legend", legend);
    graphs.setAttribute("logX", logX);
    graphs.setAttribute("logY", logY);
    graphs.setAttribute("mode", mode);
    XmlElement & files = root.addChild("Files");
    files.setAttribute("directory", directory);
    String list;
    for (const auto & e : extensions) { if (!list.isNull()) list += " "; list += e; }
    files.setAttribute("extensions", list);
    XmlElement & browser = root.addChild("Browser");
    browser.setAttribute("visible", gui.browserVisible);
    browser.setAttribute("docked", gui.browserDocked);
    root.addChild("Demo").setAttribute("enabled", demo);
    XmlElement & latexElement = root.addChild("Latex");
    latexElement.setAttribute("enabled", latex);
    latexElement.setAttribute("bin", latexBin);
    latexElement.setAttribute("preamble", latexPreamble);
    latexElement.setAttribute("rootSyntax", rootSyntax);
    document.write(fileName);
    }
  catch (const std::exception &)
    {
    return false;
    }
  return true;
  }

  // ------------------------------------------------------------------------------------------------ CapApp

  CapApp::CapApp(int argc, char ** argv)
  :
  _arguments(),
  _configurationFile(),
  _configuration(),
  _gui(),
  _functions(),
  _data(),
  _files(),
  _closedFiles(),
  _nextFileId(1),
  _saveFile(),
  _undo(),
  _redo(),
  _clipboard(),
  _showPreferences(false),
  _showAbout(false),
  _showShortcuts(false),
  _showImGuiDemo(false),
  _customColumns(2), _customRows(2),
  _customLayoutRequested(false), _customGraphsRequested(false)
  {
  for (int k=1; k<argc; k++)
    {
    String argument(argv[k]);
    if ((argument == "--config" || argument == "-c") && k+1 < argc) _configurationFile = argv[++k];
    else if (argument.str().rfind("-psn_", 0) == 0) continue;   // process serial number added by older macOS Finders
    else _arguments.push_back(argument);
    }
  // Opened from the Finder or the Dock (CapGui.app), the current folder is "/": start in the home folder instead.
  std::error_code ec;
  String home = FileSystem::home();
  if (!home.isNull() && FileSystem::current().equalTo("/")) fs::current_path(home.str(), ec);
  }

  CapApp::~CapApp()
  { }

  String CapApp::findConfigurationFile() const
  {
  // $CAP_ROOT_PATH left as written when not defined: that candidate then does not exist
  String home = FileSystem::home();
  return FileSystem::firstExisting({ home.isNull() ? String() : FileSystem::join(home, ".CapGui.xml"),
                                     FileSystem::expand("$CAP_ROOT_PATH/config/CapGui.xml"),
                                     FileSystem::join(CAP_SOURCE_ROOT, "config/CapGui.xml") });
  }

  int CapApp::run()
  {
  // configuration
  if (_configurationFile.isNull()) _configurationFile = findConfigurationFile();
  String configurationError;
  bool configured = !_configurationFile.isNull() && _configuration.read(_configurationFile, configurationError);
  if (!FileSystem::home().isNull()) _configuration.gui.iniFile = FileSystem::join(FileSystem::home(), ".CapGui.ini");
  if (_configuration.directory.isNull())
    {
    _configuration.directory = FileSystem::current();
    }

  _gui.onNewPane = [this](CanvasPane & pane) { setupPane(pane); };
  _gui.beforePanesChange = [this]() { recordUndo(); };
  if (!_gui.initialize(_configuration.gui))
    {
    std::fprintf(stderr, "CapGui: cannot open the window (is a display available?)\n");
    return 1;
    }
  buildMenus();
  _gui.menuBar().addListener(this);
  _gui.onFrame = [this]() { updateMenuStates(); renderWindows(); };

  FileBrowser & browser = _gui.browser();
  browser.setFiles(&_files);
  browser.onOpenFile   = [this](const String & path) { return openFile(path); };
  browser.onShowObject = [this](const ObjectReference & object) { showObject(object); };
  browser.onShowData   = [this](const ObjectReference & object) { showData(object); };
  browser.onShowFolder = [this](int file, const String & folder) { showFolder(file, folder); };
  browser.onCloseFile  = [this](int file) { closeFile(file); };
  browser.onReloadFile = [this](int file) { reloadFile(file); };
  browser.onCommand    = [this](const String & command) { actionPerformed(ActionEvent{command, nullptr}); };

  if (configured)                          _gui.setStatus(String("Configuration: ") + _configurationFile);
  else if (!configurationError.isNull())   _gui.showMessage("Configuration not read", _configurationFile + String(":\n") + configurationError);
  else                                     _gui.setStatus("No configuration file found: default settings");
  String latexProblem = applyLatex();
  if (!latexProblem.isNull()) _gui.setStatus(latexProblem);

  if (_configuration.demo) createDemo();
  for (const auto & argument : _arguments)
    {
    if (FileSystem::isDirectory(argument)) openFolder(argument); else openFile(argument);
    }
  _gui.run();
  return 0;
  }

  void CapApp::setupPane(CanvasPane & pane)
  {
  pane.divide(_configuration.graphColumns, _configuration.graphRows);
  for (int k=0; k<pane.canvas().nGraphs(); k++)
    {
    Graph & graph = pane.canvas().graph(k);
    graph.setShowLegend(_configuration.legend);
    graph.setLogX(_configuration.logX);
    graph.setLogY(_configuration.logY);
    }
  pane.setMode(_configuration.mode == "replace" ? CanvasPane::kReplace : CanvasPane::kAdd);
  pane.beforeChange = [this]() { recordUndo(); };
  pane.onDrop = [this, &pane](int graph, const ObjectReference & object) { showObject(pane, graph, object); };
  pane.onShowData = [this](const MATH::DataPoints * data, const String & label) { showDataTable(data, label); };
  }

  // ------------------------------------------------------------------------------------------------ menus

  void CapApp::buildMenus()
  {
  MenuBar & bar = _gui.menuBar();

  Menu & file = bar.addMenu("File");
  file.addItem("Open...",        "file.open",       "Ctrl+O",       "Open a CAP file");
  file.addItem("Open Folder...", "file.openFolder", "Ctrl+Shift+O", "Show a folder of the file system in the browser");
  file.addItem("Close All Files", "file.closeAll",  "",             "Close the CAP files open in the browser");
  file.addSeparator();
  file.addItem("Save",           "file.save",       "Ctrl+S",       "Save the canvases shown in the window (SVG)");
  file.addItem("Save As...",     "file.saveAs",     "Ctrl+Shift+S", "Save the canvases shown in the window to a new file (SVG)");
  file.addSeparator();
  file.addItem("Exit",           "file.exit",       "Ctrl+Q",       "Close the window and quit");

  Menu & edit = bar.addMenu("Edit");
  edit.addItem("Undo",  "edit.undo",   "Ctrl+Z",       "Undo the last change to the canvases");
  edit.addItem("Redo",  "edit.redo",   "Ctrl+Shift+Z", "Redo the change undone last");
  edit.addSeparator();
  edit.addItem("Cut",    "edit.cut",    "Ctrl+X", "Copy the contents of the selected graph, then clear it");
  edit.addItem("Copy",   "edit.copy",   "Ctrl+C", "Copy the contents of the selected graph");
  edit.addItem("Paste",  "edit.paste",  "Ctrl+V", "Show the copied objects in the selected graph");
  edit.addItem("Delete", "edit.delete", "Delete", "Clear the selected graph");
  edit.addSeparator();
  edit.addItem("Clear Canvas",       "edit.clearCanvas", "", "Clear all the graphs of the current canvas");
  edit.addItem("Clear All Canvases", "edit.clearAll",    "", "Clear all the graphs of all the canvases");
  edit.addSeparator();
  edit.addItem("Preferences...", "edit.preferences", "Ctrl+,", "Window, canvas and graph settings; save them as configuration");

  Menu & view = bar.addMenu("View");
  MenuItem & browser = view.addItem("File Browser", "view.browser", "Ctrl+B", "Show or hide the file browser");
  browser.setCheckable(true);
  browser.setChecked(_configuration.gui.browserVisible);
  MenuItem & browserWindow = view.addItem("Browser in Separate Window", "view.browserWindow", "",
                                          "File browser in its own window instead of docked in the main window");
  browserWindow.setCheckable(true);
  browserWindow.setChecked(!_configuration.gui.browserDocked);
  view.addSeparator();
  MenuItem & toolbars = view.addItem("Canvas Toolbars", "view.toolbars", "", "Show the buttons above each canvas");
  toolbars.setCheckable(true);
  toolbars.setChecked(_configuration.gui.toolbars);
  MenuItem & legends = view.addItem("Legends", "view.legends", "", "Show the legend of every graph");
  legends.setCheckable(true);
  legends.setChecked(_configuration.legend);
  view.addSeparator();
  Menu & theme = view.addMenu("Theme");
  for (const char * t : {"light", "dark", "classic"})
    {
    String name(t);
    name.str()[0] = char(std::toupper(name.str()[0]));
    MenuItem & item = theme.addItem(name, String("view.theme.") + t, "", String("Use the ") + t + String(" theme"));
    item.setCheckable(true);
    item.setChecked(_configuration.gui.theme == t);
    }

  Menu & window = bar.addMenu("Window");
  Menu & layout = window.addMenu("Canvases", "Number of canvas panes in the window (columns x rows)");
  for (auto cr : std::vector<std::pair<int,int>>{{1,1},{2,1},{1,2},{2,2},{3,2},{3,3}})
    {
    char name[16], command[40];
    std::snprintf(name, sizeof(name), "%d x %d", cr.first, cr.second);
    std::snprintf(command, sizeof(command), "window.layout.%dx%d", cr.first, cr.second);
    layout.addItem(name, command, "", "Arrange the canvases in columns x rows panes").setCheckable(true);
    }
  layout.addSeparator();
  layout.addItem("Custom...", "window.layout.custom", "", "Choose the number of columns and rows of canvases");
  Menu & graphs = window.addMenu("Graphs in Canvas", "Number of graphs in the current canvas (columns x rows)");
  for (auto cr : std::vector<std::pair<int,int>>{{1,1},{2,1},{1,2},{2,2},{3,2},{3,3}})
    {
    char name[16], command[40];
    std::snprintf(name, sizeof(name), "%d x %d", cr.first, cr.second);
    std::snprintf(command, sizeof(command), "window.graphs.%dx%d", cr.first, cr.second);
    graphs.addItem(name, command, "", "Divide the current canvas into columns x rows graphs").setCheckable(true);
    }
  graphs.addSeparator();
  graphs.addItem("Custom...", "window.graphs.custom", "", "Choose the number of columns and rows of graphs");
  window.addSeparator();
  window.addItem("New Canvas",      "window.new",      "Ctrl+T", "Add a canvas pane beside the current one");
  window.addItem("Close Canvas",    "window.close",    "Ctrl+W", "Close the current canvas pane");
  window.addSeparator();
  window.addItem("Next Canvas",     "window.next",     "Ctrl+]", "Make the next canvas current");
  window.addItem("Previous Canvas", "window.previous", "Ctrl+[", "Make the previous canvas current");

  Menu & help = bar.addMenu("Help");
  help.addItem("About CAP GUI", "help.about", "", "Version and components");
  help.addItem("Keyboard Shortcuts", "help.shortcuts", "F1", "List of the keyboard shortcuts");
  help.addSeparator();
  help.addItem("ImGui Demo (developers)", "help.imguiDemo", "", "Dear ImGui's demonstration window: all the widgets available").setCheckable(true);
  }

  void CapApp::updateMenuStates()
  {
  MenuBar & bar = _gui.menuBar();
  if (MenuItem * item = bar.findItem("edit.undo"))  item->setEnabled(!_undo.empty());
  if (MenuItem * item = bar.findItem("edit.redo"))  item->setEnabled(!_redo.empty());
  if (MenuItem * item = bar.findItem("edit.paste")) item->setEnabled(!_clipboard.empty());
  for (auto cr : std::vector<std::pair<int,int>>{{1,1},{2,1},{1,2},{2,2},{3,2},{3,3}})
    {
    char command[40];
    std::snprintf(command, sizeof(command), "window.layout.%dx%d", cr.first, cr.second);
    if (MenuItem * item = bar.findItem(command)) item->setChecked(_gui.nColumns() == cr.first && _gui.nRows() == cr.second);
    std::snprintf(command, sizeof(command), "window.graphs.%dx%d", cr.first, cr.second);
    const Canvas & canvas = _gui.currentPane().canvas();
    if (MenuItem * item = bar.findItem(command)) item->setChecked(canvas.nColumns() == cr.first && canvas.nRows() == cr.second);
    }
  for (const char * t : {"light", "dark", "classic"})
    if (MenuItem * item = bar.findItem(String("view.theme.") + t)) item->setChecked(_gui.settings().theme == t);
  if (MenuItem * item = bar.findItem("help.imguiDemo")) item->setChecked(_showImGuiDemo);
  if (MenuItem * item = bar.findItem("view.browser"))       item->setChecked(_gui.browserVisible());
  if (MenuItem * item = bar.findItem("view.browserWindow")) item->setChecked(!_gui.browserDocked());
  if (MenuItem * item = bar.findItem("file.closeAll"))      item->setEnabled(!_files.empty());
  _configuration.gui.browserVisible = _gui.browserVisible();
  _configuration.gui.browserDocked  = _gui.browserDocked();
  }

  void CapApp::actionPerformed(const ActionEvent & event)
  {
  const String & c = event.command;
  int columns = 0, rows = 0;
  if (c == "file.open")
    {
    // the configured types that can be opened (CAP files only, for now)
    std::vector<String> types;
    for (const auto & e : _configuration.extensions) if (FileSystem::hasExtension(String("x") + e, ".cap")) types.push_back(e);
    if (types.empty()) types.push_back(".cap");
    _gui.fileDialog().open(FileDialog::kOpenFile, "Open a CAP file", _configuration.directory, types, "",
                           [this](const String & path) { openFile(path); });
    }
  else if (c == "file.openFolder")
    {
    _gui.fileDialog().open(FileDialog::kOpenFolder, "Open a folder", _configuration.directory, {}, "",
                           [this](const String & path) { openFolder(path); });
    }
  else if (c == "file.save" && !_saveFile.isNull()) save(_saveFile);
  else if (c == "file.save" || c == "file.saveAs")
    {
    String name = _saveFile.isNull() ? String("canvases.svg") : FileSystem::name(_saveFile);
    _gui.fileDialog().open(FileDialog::kSaveFile, "Save the canvases", _configuration.directory, {".svg"}, name,
                           [this](const String & path) { save(path); });
    }
  else if (c == "file.closeAll")
    {
    std::vector<int> ids;
    for (const auto & f : _files) ids.push_back(f.id);
    for (int id : ids) closeFile(id);
    }
  else if (c == "file.exit") _gui.requestExit();
  else if (c == "view.browser") _gui.setBrowserVisible(!_gui.browserVisible());
  else if (c == "view.browserWindow") _gui.setBrowserDocked(!_gui.browserDocked());
  else if (c == "edit.undo") undo();
  else if (c == "edit.redo") redo();
  else if (c == "edit.copy" || c == "edit.cut")
    {
    Graph & graph = _gui.currentPane().currentGraph();
    _clipboard = graph.items();
    if (c == "edit.cut") { recordUndo(); graph.clearItems(); }
    char text[64];
    std::snprintf(text, sizeof(text), "%d object(s) copied", int(_clipboard.size()));
    _gui.setStatus(text);
    }
  else if (c == "edit.paste" && !_clipboard.empty())
    {
    CanvasPane & pane = _gui.currentPane();
    recordUndo();
    if (pane.mode() == CanvasPane::kReplace) pane.currentGraph().clearItems();
    for (const auto & item : _clipboard) pane.currentGraph().add(item);
    }
  else if (c == "edit.delete")      { recordUndo(); _gui.currentPane().currentGraph().clearItems(); }
  else if (c == "edit.clearCanvas")
    {
    recordUndo();
    Canvas & canvas = _gui.currentPane().canvas();
    for (int k=0; k<canvas.nGraphs(); k++) canvas.graph(k).clearItems();
    }
  else if (c == "edit.clearAll")
    {
    recordUndo();
    for (int p=0; p<_gui.nPanes(); p++)
      for (int k=0; k<_gui.pane(p).canvas().nGraphs(); k++) _gui.pane(p).canvas().graph(k).clearItems();
    }
  else if (c == "edit.preferences") _showPreferences = true;
  else if (c == "view.toolbars") _gui.setShowToolbars(event.source ? event.source->isChecked() : !_gui.showToolbars());
  else if (c == "view.legends")
    {
    bool show = event.source ? event.source->isChecked() : true;
    _configuration.legend = show;
    for (int p=0; p<_gui.nPanes(); p++)
      for (int k=0; k<_gui.pane(p).canvas().nGraphs(); k++) _gui.pane(p).canvas().graph(k).setShowLegend(show);
    }
  else if (c.beginsWith("view.theme."))
    {
    String theme = c; theme.replaceAll("view.theme.", "");
    _gui.setTheme(theme);
    _configuration.gui.theme = theme;
    }
  else if (c == "window.layout.custom") { _customColumns = _gui.nColumns(); _customRows = _gui.nRows(); _customLayoutRequested = true; }
  else if (c == "window.graphs.custom")
    {
    _customColumns = _gui.currentPane().canvas().nColumns();
    _customRows    = _gui.currentPane().canvas().nRows();
    _customGraphsRequested = true;
    }
  else if (std::sscanf(c.data(), "window.layout.%dx%d", &columns, &rows) == 2) setLayout(columns, rows);
  else if (std::sscanf(c.data(), "window.graphs.%dx%d", &columns, &rows) == 2) setGraphs(columns, rows);
  else if (c == "window.new")      { recordUndo(); _gui.addPane(); }
  else if (c == "window.close")    { if (_gui.nPanes() > 1) { recordUndo(); _gui.closePane(_gui.currentPaneIndex()); } }
  else if (c == "window.next")     _gui.setCurrentPane((_gui.currentPaneIndex() + 1) % _gui.nPanes());
  else if (c == "window.previous") _gui.setCurrentPane((_gui.currentPaneIndex() + _gui.nPanes() - 1) % _gui.nPanes());
  else if (c == "help.about")      _showAbout = true;
  else if (c == "help.shortcuts")  _showShortcuts = true;
  else if (c == "help.imguiDemo")  _showImGuiDemo = !_showImGuiDemo;
  }

  // ------------------------------------------------------------------------------------------------ actions

  void CapApp::setLayout(int columns, int rows)
  {
  recordUndo();
  _gui.setLayout(columns, rows);
  _configuration.gui.columns = _gui.nColumns();
  _configuration.gui.rows    = _gui.nRows();
  }

  void CapApp::setGraphs(int columns, int rows)
  {
  recordUndo();
  CanvasPane & pane = _gui.currentPane();
  pane.divide(columns, rows);
  for (int k=0; k<pane.canvas().nGraphs(); k++) pane.canvas().graph(k).setShowLegend(_configuration.legend);
  }

  GraphItem & CapApp::show(MATH::Function * function, float xMinimum, float xMaximum)
  {
  _functions.emplace_back(function);
  return _gui.currentPane().show(GraphItem(function, xMinimum, xMaximum));
  }

  GraphItem & CapApp::show(MATH::Function * function, float xMinimum, float xMaximum, float yMinimum, float yMaximum)
  {
  _functions.emplace_back(function);
  return _gui.currentPane().show(GraphItem(function, xMinimum, xMaximum, yMinimum, yMaximum));
  }

  GraphItem & CapApp::show(MATH::DataPoints * data)
  {
  _data.emplace_back(data);
  return _gui.currentPane().show(GraphItem(data));
  }

  namespace
  {
  //! Absolute, normalized path (so that the same file is recognized whichever way it is named).
  String normalizedPath(const String & path)
  {
  return FileSystem::normalize(path);
  }

  //! Read a CAP file. @param error set to the problem @return the file, or nullptr
  std::shared_ptr<CapFile> readCapFile(const String & path, String & error)
  {
  try
    {
    auto file = std::make_shared<CapFile>(path, File::kReadOnly);
    file->load(File::Loading::kLazy);   // the tree at once; objects read when first shown
    return file;
    }
  catch (const std::exception & e) { error = e.what(); }
  catch (...) { error = "unknown error"; }
  return nullptr;
  }
  } // namespace

  BrowserFile * CapApp::file(int id)
  {
  for (auto & f : _files) if (f.id == id) return &f;
  return nullptr;
  }

  int CapApp::openFile(const String & fileName)
  {
  String path = normalizedPath(fileName);
  for (const auto & f : _files)
    if (f.path == path) { _gui.setBrowserVisible(true); _gui.browser().reveal(path); return f.id; }
  if (!FileSystem::isFile(path)) { _gui.showMessage("Open", String("No such file: ") + path); return -1; }
  String error;
  std::shared_ptr<CapFile> capFile = readCapFile(path, error);
  if (!capFile) { _gui.showMessage("Open", String("Cannot read ") + path + String(":\n") + error); return -1; }
  BrowserFile entry;
  entry.id   = _nextFileId++;
  entry.path = path;
  entry.file = capFile;
  _files.push_back(entry);
  _configuration.directory = FileSystem::parent(path);
  _gui.setBrowserVisible(true);
  _gui.browser().reveal(path);
  _gui.setStatus(String("Opened ") + path);
  return entry.id;
  }

  void CapApp::openFolder(const String & folder)
  {
  String path = normalizedPath(folder);
  if (!FileSystem::isDirectory(path)) { _gui.showMessage("Open Folder", String("No such folder: ") + path); return; }
  _gui.browser().addFolder(path);
  _gui.setBrowserVisible(true);
  _configuration.directory = path;
  _gui.setStatus(String("Folder ") + path);
  }

  void CapApp::closeFile(int id)
  {
  for (auto it = _files.begin(); it != _files.end(); ++it)
    {
    if (it->id != id) continue;
    _gui.setStatus(String("Closed ") + it->path);
    _closedFiles.push_back(*it);
    _files.erase(it);
    return;
    }
  }

  void CapApp::reloadFile(int id)
  {
  BrowserFile * entry = file(id);
  if (!entry) return;
  String error;
  std::shared_ptr<CapFile> fresh = readCapFile(entry->path, error);
  if (!fresh) { _gui.showMessage("Reload", String("Cannot read ") + entry->path + String(":\n") + error); return; }
  // the graphs showing objects of the old version show the objects of the same path in the new one
  std::shared_ptr<CapFile> old = entry->file;
  std::vector<String> paths = old->paths();
  int updated = 0;
  for (int p=0; p<_gui.nPanes(); p++)
    for (int g=0; g<_gui.pane(p).canvas().nGraphs(); g++)
      for (auto & item : _gui.pane(p).canvas().graph(g).items())
        {
        if (item.kind() != GraphItem::kData) continue;
        for (const auto & path : paths)
          {
          if (!old->isLoaded(path)) continue;   // never read: no graph shows it (and the file has changed)
          if (old->get<MATH::DataPoints>(path) != item.data()) continue;
          try { if (const MATH::DataPoints * d = fresh->get<MATH::DataPoints>(path)) { item.setData(d); updated++; } }
          catch (const std::exception &) { }   // no longer readable in the new version: the graph keeps the old one
          break;
          }
        }
  // the data panes showing objects of the old version show those of the new one
  for (int k=0; k<_gui.nDataPanes(); k++)
    {
    DataPane & pane = _gui.dataPane(k);
    for (const auto & path : paths)
      {
      if (!old->isLoaded(path) || old->get<MATH::DataPoints>(path) != pane.data()) continue;
      try { if (const MATH::DataPoints * d = fresh->get<MATH::DataPoints>(path)) pane.setData(d, pane.label()); }
      catch (const std::exception &) { }
      break;
      }
    }
  BrowserFile retired = *entry;
  _closedFiles.push_back(retired);
  entry->file = fresh;
  char text[64];
  std::snprintf(text, sizeof(text), " (%d graph item(s) updated)", updated);
  _gui.setStatus(String("Reloaded ") + entry->path + String(text));
  }

  bool CapApp::showData(const ObjectReference & object)
  {
  BrowserFile * entry = file(object.file);
  if (!entry || !entry->file) { _gui.setStatus("That file is no longer open"); return false; }
  const MATH::DataPoints * data = nullptr;
  try { data = entry->file->get<MATH::DataPoints>(object.path); }   // read now if the file was opened lazily
  catch (const std::exception & e) { _gui.setStatus(object.path + String(": cannot be read: ") + String(e.what())); return false; }
  if (!data) { _gui.setStatus(object.path + String(": has no values to show as a table")); return false; }
  showDataTable(data, data->name().isNull() ? object.path : data->name());
  return true;
  }

  void CapApp::showDataTable(const MATH::DataPoints * data, const String & label)
  {
  if (!data) return;
  _gui.addDataPane().setData(data, label);
  char text[64];
  std::snprintf(text, sizeof(text), ": %u rows", data->size());
  _gui.setStatus(String("Data table of ") + label + String(text));
  }

  bool CapApp::showObject(const ObjectReference & object)
  {
  CanvasPane & pane = _gui.currentPane();
  return showObject(pane, pane.selectedGraph(), object);
  }

  bool CapApp::showObject(CanvasPane & pane, int graphIndex, const ObjectReference & object)
  {
  BrowserFile * entry = file(object.file);
  if (!entry || !entry->file) { _gui.setStatus("That file is no longer open"); return false; }
  const MATH::DataPoints * data = nullptr;
  try { data = entry->file->get<MATH::DataPoints>(object.path); }   // read now if the file was opened lazily
  catch (const std::exception & e) { _gui.setStatus(object.path + String(": cannot be read: ") + String(e.what())); return false; }
  if (!data) { _gui.setStatus(object.path + String(": cannot be shown in a graph")); return false; }
  const MATH::Histogram * histogram = dynamic_cast<const MATH::Histogram *>(data);
  unsigned int rank = histogram ? histogram->rank() : data->rank();
  if (rank > 2)
    {
    _gui.setStatus(object.path + String(": objects of rank 3 and more will be shown by the 3D views (to come)"));
    return false;
    }
  if (graphIndex < 0 || graphIndex >= pane.canvas().nGraphs()) graphIndex = pane.selectedGraph();
  pane.setSelectedGraph(graphIndex);
  for (int k=0; k<_gui.nPanes(); k++) if (&_gui.pane(k) == &pane) _gui.setCurrentPane(k);

  Graph & graph = pane.currentGraph();
  bool fresh = graph.items().empty() || pane.mode() == CanvasPane::kReplace;
  // label: the name, or the path when the graph already shows an object of the same name
  String name = data->name().isNull() ? object.path : data->name();
  String label = name;
  for (const auto & item : graph.items()) if (item.label() == name && item.object() != data) label = object.path;
  GraphItem & item = pane.show(GraphItem(data, label));
  styleItem(item, int(graph.items().size()) - 1);
  if (fresh)
    {
    graph.setTitle(data->title().isNull() ? name : data->title());
    graph.xAxis().setTitle(histogram ? histogram->dimensionTitle(0) : String(""));
    graph.yAxis().setTitle(histogram && rank == 2 ? histogram->dimensionTitle(1) : String(""));
    graph.zAxis().setTitle("");
    graph.setAutoRange();
    }
  _gui.setStatus(String("Showing ") + object.path + String("  (") + entry->path + String(")"));
  return true;
  }

  void CapApp::showFolder(int fileId, const String & folderPath)
  {
  BrowserFile * entry = file(fileId);
  if (!entry || !entry->file) return;
  const Folder * folder = folderPath.isNull() ? &entry->file->folder() : entry->file->folder(folderPath);
  if (!folder) return;
  // objects of the folder that can be shown (not those of its sub-folders)
  std::vector<ObjectReference> objects;
  for (const auto & name : folder->names())
    {
    if (folder->folder(name)) continue;
    String kind, details;
    try { if (!FileBrowser::describe(*folder, name, kind, details)) continue; }
    catch (const std::exception & e) { _gui.setStatus(name + String(": cannot be read: ") + String(e.what())); continue; }
    ObjectReference reference;
    reference.file = fileId;
    reference.path = folderPath.isNull() ? name : folderPath + String("/") + name;
    objects.push_back(reference);
    }
  if (objects.empty()) { _gui.setStatus(String("Nothing to show in ") + (folderPath.isNull() ? entry->path : folderPath)); return; }
  if (objects.size() > 36) objects.resize(36);
  int n = int(objects.size());
  int columns = int(std::ceil(std::sqrt(double(n))));
  int rows = (n + columns - 1)/columns;
  recordUndo();
  CanvasPane & pane = _gui.currentPane();
  pane.divide(columns, rows);
  for (int k=0; k<pane.canvas().nGraphs(); k++) pane.canvas().graph(k).clearItems();
  std::function<void()> hook = pane.beforeChange;
  pane.beforeChange = nullptr;                          // one undo step for the whole folder
  CanvasPane::Mode mode = pane.mode();
  pane.setMode(CanvasPane::kReplace);
  for (int k=0; k<n; k++) showObject(pane, k, objects[k]);
  pane.setMode(mode);
  pane.beforeChange = hook;
  pane.setSelectedGraph(0);
  }

  void CapApp::styleItem(GraphItem & item, int k)
  {
  static const int colors[]  = {1, 2, 4, 8, 6, 9, 28, 46};
  static const int markers[] = {20, 21, 22, 23, 33, 29, 34, 47};
  int color = colors[(k < 0 ? 0 : k) % 8];
  item.lineAttributes().setColor(color);
  item.markerAttributes().setColor(color);
  item.markerAttributes().setStyle(markers[(k < 0 ? 0 : k) % 8]);
  }

  void CapApp::save(const String & fileName)
  {
  String name = fileName;
  if (!name.endsWith(".svg")) name += ".svg";
  if (_gui.savePageSvg(name))
    {
    _saveFile = name;
    _configuration.directory = FileSystem::parent(name);
    _gui.setStatus(String("Saved ") + name);
    }
  else _gui.showMessage("Save", String("Cannot write ") + name);
  }

  // ------------------------------------------------------------------------------------------------ undo

  CapApp::Snapshot CapApp::snapshot() const
  {
  Snapshot s;
  CapGui & gui = const_cast<CapGui&>(_gui);
  s.columns = gui.nColumns();
  s.rows    = gui.nRows();
  s.current = gui.currentPaneIndex();
  for (int k=0; k<gui.nPanes(); k++)
    {
    s.canvases.push_back(gui.pane(k).canvas());
    s.selected.push_back(gui.pane(k).selectedGraph());
    }
  return s;
  }

  void CapApp::restore(const Snapshot & s)
  {
  if (_gui.nPanes() != int(s.canvases.size())) _gui.setPaneCount(int(s.canvases.size()));
  for (int k=0; k<_gui.nPanes() && k<int(s.canvases.size()); k++)
    {
    _gui.pane(k).canvas() = s.canvases[k];
    _gui.pane(k).setSelectedGraph(s.selected[k]);
    }
  _gui.setCurrentPane(s.current);
  }

  void CapApp::recordUndo()
  {
  _undo.push_back(snapshot());
  if (_undo.size() > 50) _undo.pop_front();
  _redo.clear();
  }

  void CapApp::undo()
  {
  if (_undo.empty()) return;
  _redo.push_back(snapshot());
  restore(_undo.back());
  _undo.pop_back();
  }

  void CapApp::redo()
  {
  if (_redo.empty()) return;
  _undo.push_back(snapshot());
  restore(_redo.back());
  _redo.pop_back();
  }

  // ------------------------------------------------------------------------------------------------ demo

  void CapApp::createDemo()
  {
  // a Gaussian and pseudo-data scattered around it, with statistical (bars) and systematic (boxes) uncertainties
  MATH::Normal * gaussian = new MATH::Normal(1.0, 0.0, 1.0);
  gaussian->setName("Gaussian(0,1)");
  std::vector<double> x, ex, y, ey, sy;
  for (int k=0; k<25; k++)
    {
    double v = -3.0 + 0.25*k, t = std::exp(-0.5*v*v);
    x.push_back(v); ex.push_back(0.0);
    y.push_back(t*(1.0 + 0.08*std::sin(7.0*v)));
    ey.push_back(0.03 + 0.05*t); sy.push_back(0.02 + 0.06*t);
    }
  MATH::DataPointsVector * data = new MATH::DataPointsVector(x, ex, y, ey, sy);
  data->setName("pseudo-data");
  CanvasPane & first = _gui.pane(0);
  first.currentGraph().setTitle("Demonstration");
  first.currentGraph().xAxis().setTitle("x");
  first.currentGraph().yAxis().setTitle("f(x)");
  _data.emplace_back(data);
  _functions.emplace_back(gaussian);
  first.currentGraph().add(GraphItem(data));
  first.currentGraph().add(GraphItem(gaussian, -3.0, 3.0)).lineAttributes().set(4, 1.0, 1, 2.0);

  // an exponential, shown on a log scale in the second canvas when there is one
  MATH::Exponential * decay = new MATH::Exponential(100.0, -0.7);
  decay->setName("100 exp(-0.7 x)");
  _functions.emplace_back(decay);
  CanvasPane & second = _gui.nPanes() > 1 ? _gui.pane(1) : first;
  Graph & graph = (_gui.nPanes() > 1) ? second.currentGraph()
                : (first.canvas().nGraphs() > 1 ? first.canvas().graph(1) : first.currentGraph());
  if (&graph != &first.currentGraph())
    {
    graph.setTitle("Exponential decay");
    graph.setLogY(true);
    graph.xAxis().setTitle("t");
    graph.yAxis().setTitle("N(t)");
    graph.add(GraphItem(decay, 0.0, 10.0)).lineAttributes().set(2, 1.0, 1, 2.0);
    }
  _gui.setStatus("Demonstration objects shown (turn them off with <Demo enabled=\"false\"/> in the configuration)");
  }

  // ------------------------------------------------------------------------------------------------ windows

  void CapApp::renderWindows()
  {
  renderPreferences();
  renderAbout();
  renderShortcuts();
  renderCustomLayout();
  if (_showImGuiDemo) ImGui::ShowDemoWindow(&_showImGuiDemo);
  }

  String CapApp::applyLatex()
  {
  GRAPHICS::LatexRenderer & latex = GRAPHICS::LatexRenderer::instance();
  latex.setEnabled(_configuration.latex);
  latex.setRootSyntax(_configuration.rootSyntax);
  latex.setPreamble(_configuration.latexPreamble);
  String problem;
  if (!_configuration.latexBin.isNull())
    {
    String directory = FileSystem::expand(_configuration.latexBin);
    if (!latex.setBinDirectory(directory))
      problem = String("LaTeX: latex and dvisvgm not found in ") + directory
              + (latex.available() ? String(", using those of ") + latex.binDirectory() : String(""));
    }
  else if (!latex.available()) latex.findTools();
  if (_configuration.latex && !latex.available() && problem.isNull())
    problem = "LaTeX: latex and dvisvgm not found, texts drawn with the fonts (see Edit > Preferences)";
  return problem;
  }

  void CapApp::renderPreferences()
  {
  if (!_showPreferences) return;
  ImGui::SetNextWindowSize(ImVec2(470, 0), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Preferences", &_showPreferences, ImGuiWindowFlags_NoCollapse)) { ImGui::End(); return; }
  AppConfiguration & c = _configuration;

  ImGui::SeparatorText("Window");
  static const char * themes[] = {"light", "dark", "classic"};
  int theme = c.gui.theme == "dark" ? 1 : (c.gui.theme == "classic" ? 2 : 0);
  if (ImGui::Combo("Theme", &theme, themes, 3)) { c.gui.theme = themes[theme]; _gui.setTheme(c.gui.theme); }
  float fontSize = c.gui.fontSize;
  if (ImGui::SliderFloat("Font size", &fontSize, 10.0f, 28.0f, "%.0f"))
    {
    c.gui.fontSize = fontSize;
    ImGui::GetStyle().FontScaleMain = fontSize/_gui.settings().fontSize;
    }
  ImGui::InputInt("Width", &c.gui.width);
  ImGui::InputInt("Height", &c.gui.height);
  ImGui::Checkbox("Windows can leave the main window (restart)", &c.gui.viewports);

  ImGui::SeparatorText("Canvases");
  int layout[2] = {_gui.nColumns(), _gui.nRows()};
  if (ImGui::InputInt2("Columns, rows", layout)) setLayout(layout[0], layout[1]);
  bool toolbars = _gui.showToolbars();
  if (ImGui::Checkbox("Toolbars", &toolbars)) { _gui.setShowToolbars(toolbars); c.gui.toolbars = toolbars; }

  ImGui::SeparatorText("New canvases");
  int graphs[2] = {c.graphColumns, c.graphRows};
  if (ImGui::InputInt2("Graphs: columns, rows", graphs))
    {
    c.graphColumns = graphs[0] < 1 ? 1 : (graphs[0] > 8 ? 8 : graphs[0]);
    c.graphRows    = graphs[1] < 1 ? 1 : (graphs[1] > 8 ? 8 : graphs[1]);
    }
  ImGui::Checkbox("Legend", &c.legend);
  ImGui::SameLine();
  ImGui::Checkbox("Log X", &c.logX);
  ImGui::SameLine();
  ImGui::Checkbox("Log Y", &c.logY);
  int mode = c.mode == "replace" ? 1 : 0;
  ImGui::RadioButton("Add", &mode, 0); ImGui::SameLine(); ImGui::RadioButton("Replace", &mode, 1);
  c.mode = mode ? "replace" : "add";

  ImGui::SeparatorText("Startup");
  ImGui::Checkbox("Show the demonstration objects", &c.demo);

  ImGui::SeparatorText("LaTeX");
  GRAPHICS::LatexRenderer & latex = GRAPHICS::LatexRenderer::instance();
  if (latex.available()) ImGui::TextDisabled("latex and dvisvgm: %s", latex.binDirectory().data());
  else ImGui::TextColored(ImVec4(0.85f, 0.25f, 0.1f, 1.0f), "latex and dvisvgm not found: texts drawn with the fonts");
  if (ImGui::Checkbox("Typeset the texts with $ or \\ by LaTeX", &c.latex)) latex.setEnabled(c.latex);
  if (ImGui::Checkbox("Translate the ROOT syntax (#Delta, p_{T}, ...)", &c.rootSyntax)) latex.setRootSyntax(c.rootSyntax);
  std::string bin = c.latexBin.str();
  ImGui::SetNextItemWidth(300);
  if (ImGui::InputTextWithHint("TeX directory", "looked for", &bin)) c.latexBin = String(bin);
  ImGui::SetItemTooltip("Directory of latex and dvisvgm, e.g. /Library/TeX/texbin (empty: looked for in the PATH and the usual TeX installations); applied when you leave the field");
  if (ImGui::IsItemDeactivatedAfterEdit())
    {
    if (c.latexBin.isNull()) latex.findTools();
    String problem = applyLatex();
    _gui.setStatus(problem.isNull() ? String("LaTeX: ") + latex.binDirectory() : problem);
    }
  std::string preamble = c.latexPreamble.str();
  ImGui::SetNextItemWidth(300);
  if (ImGui::InputText("Preamble", &preamble)) c.latexPreamble = String(preamble);
  ImGui::SetItemTooltip("Packages and macros of the LaTeX documents, e.g. \\usepackage{amsmath,amssymb,siunitx}; applied when you leave the field (the texts are typeset again)");
  if (ImGui::IsItemDeactivatedAfterEdit()) latex.setPreamble(c.latexPreamble);
  static std::string sample = "$\\sqrt{s_{NN}} = 5.02$ TeV";
  ImGui::SetNextItemWidth(300);
  ImGui::InputText("Try a text", &sample);
  GraphEditor::latexPreview(String(sample));
  if (!latex.cacheDirectory().isNull()) ImGui::TextDisabled("Cache: %s", latex.cacheDirectory().data());

  ImGui::Separator();
  String target = _configurationFile;
  if (target.isNull() && !FileSystem::home().isNull()) target = FileSystem::join(FileSystem::home(), ".CapGui.xml");
  if (ImGui::Button("Save configuration"))
    {
    c.gui.columns = _gui.nColumns(); c.gui.rows = _gui.nRows();
    if (c.write(target)) { _configurationFile = target; _gui.setStatus(String("Configuration saved to ") + target); }
    else _gui.showMessage("Preferences", String("Cannot write ") + target);
    }
  ImGui::SameLine();
  ImGui::TextDisabled("%s", target.data());
  ImGui::End();
  }

  void CapApp::renderAbout()
  {
  if (!_showAbout) return;
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  if (ImGui::Begin("About CAP GUI", &_showAbout, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
    {
    ImGui::Text("CAP graphical interface");
    ImGui::Separator();
    ImGui::Text("Display of functions and data points; CAP files and folders.");
    ImGui::Text("Built on Dear ImGui %s and GLFW; no ROOT classes.", IMGUI_VERSION);
    ImGui::Text("Configuration: %s", _configurationFile.isNull() ? "(defaults)" : _configurationFile.data());
    }
  ImGui::End();
  }

  void CapApp::renderShortcuts()
  {
  if (!_showShortcuts) return;
  if (ImGui::Begin("Keyboard Shortcuts", &_showShortcuts, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
    {
    if (ImGui::BeginTable("shortcuts", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV))
      {
      for (const char * menuName : {"File", "Edit", "View", "Window", "Help"})
        {
        Menu * menu = _gui.menuBar().findMenu(menuName);
        if (!menu) continue;
        std::vector<MenuItem*> items;
        menu->collectItems(items);
        for (auto item : items)
          {
          if (item->shortcut().isNull()) continue;
          ImGui::TableNextRow();
          ImGui::TableNextColumn(); ImGui::TextUnformatted(MenuBar::displayShortcut(item->shortcut()).data());
          ImGui::TableNextColumn(); ImGui::Text("%s > %s", menuName, item->name().data());
          }
        }
      ImGui::EndTable();
      }
    ImGui::SeparatorText("Mouse on a graph");
    if (ImGui::BeginTable("mouse", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV))
      {
      const char * rows[][2] = {
        { "Drag in the frame",           "Zoom on the rectangle" },
        { "Drag along an axis",          "Zoom on that axis" },
        { "Double-click in the frame",   "Automatic ranges" },
        { "Drag a title or the legend",  "Move it" },
        { "Shift + drag an axis",        "Move its labels closer or farther" },
        { "Right-click a part",          "Edit it: title, axis, axis title, legend, data points" },
        { "Right-click elsewhere",       "Menu of the graph" },
        { "Escape",                      "Cancel the drag" },
        { "Drag a border between panes", "Resize the panes (x of a tab: close the pane; +: new pane)" } };
      for (const auto & row : rows)
        {
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::TextUnformatted(row[0]);
        ImGui::TableNextColumn(); ImGui::TextUnformatted(row[1]);
        }
      ImGui::EndTable();
      }
    }
  ImGui::End();
  }

  void CapApp::renderCustomLayout()
  {
  if (_customLayoutRequested) { ImGui::OpenPopup("Canvases##custom"); _customLayoutRequested = false; }
  if (_customGraphsRequested) { ImGui::OpenPopup("Graphs##custom");   _customGraphsRequested = false; }
  for (int which=0; which<2; which++)
    {
    const char * id = which == 0 ? "Canvases##custom" : "Graphs##custom";
    if (!ImGui::BeginPopupModal(id, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) continue;
    ImGui::Text(which == 0 ? "Canvas panes in the window" : "Graphs in the current canvas");
    ImGui::SetNextItemWidth(120); ImGui::InputInt("columns", &_customColumns);
    ImGui::SetNextItemWidth(120); ImGui::InputInt("rows", &_customRows);
    int limit = which == 0 ? 6 : 8;
    _customColumns = _customColumns < 1 ? 1 : (_customColumns > limit ? limit : _customColumns);
    _customRows    = _customRows    < 1 ? 1 : (_customRows    > limit ? limit : _customRows);
    if (ImGui::Button("Apply"))
      {
      if (which == 0) setLayout(_customColumns, _customRows); else setGraphs(_customColumns, _customRows);
      ImGui::CloseCurrentPopup();
      }
    ImGui::SameLine();
    if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
    }
  }

  } // namespace GUI
} // namespace CAP
