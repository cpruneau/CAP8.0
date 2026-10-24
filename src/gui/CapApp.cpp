/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
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
#ifdef CAP_WITH_ROOT_FILES
#include "RootFileConversion.hpp"
#endif
#include "CapApp.hpp"
#include "CanvasRegistry.hpp"
#ifdef CAP_GUI_PRESENTATIONS
#include "CanvasPresentation.hpp"
#endif
#include "XmlDocument.hpp"
#include "Exceptions.hpp"
#include "Normal.hpp"
#include "Exponential.hpp"
#include "DataPointsVector.hpp"
#include "Fitter.hpp"
#include "FitViews.hpp"
#include "FunctionRegistry.hpp"
#include "Histogram.hpp"
#include "StatisticalTests.hpp"
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
      keepShapes   = e->attributeBool("keepShapes", keepShapes);
      colorPalette   = e->attribute("colorPalette", colorPalette);
      markerPalette  = e->attribute("markerPalette", markerPalette);
      shadingPalette = e->attribute("shadingPalette", shadingPalette);
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
    graphs.setAttribute("keepShapes", keepShapes);
    graphs.setAttribute("colorPalette", colorPalette);
    graphs.setAttribute("markerPalette", markerPalette);
    graphs.setAttribute("shadingPalette", shadingPalette);
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
  _treeCounter(0),
  _removed(),
  _saveFile(),
  _saveFormat(".svg"),
  _renamePane(-1),
  _renameText(),
  _renameRequested(false),
  _undo(),
  _redo(),
  _clipboard(),
  _comparison(),
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
  // the palettes of the session, before any graph is made (new graphs take the current shading palette)
  GRAPHICS::Palettes & palettes = GRAPHICS::Palettes::session();
  if (!palettes.setColorPalette(_configuration.colorPalette))     _configuration.colorPalette = palettes.colorPalette().name();
  if (!palettes.setMarkerPalette(_configuration.markerPalette))   _configuration.markerPalette = palettes.markerPalette().name();
  if (!palettes.setShadingPalette(_configuration.shadingPalette)) _configuration.shadingPalette = palettes.shadingPaletteName();
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
  _gui.onCommand = [this](const String & command) { actionPerformed(ActionEvent{command, nullptr}); };

  connectBrowser(_gui.browser());
  _gui.onNewBrowser = [this](FileBrowser & browser) { connectBrowser(browser); };
  _gui.onCloseBrowser = [this](FileBrowser & browser)
    {
    std::vector<int> ids;
    for (const auto & f : _files) if (browser.showsFile(f.id)) ids.push_back(f.id);
    for (int id : ids) closeFile(id, &browser);
    };

  if (configured)                          _gui.setStatus(String("Configuration: ") + _configurationFile);
  else if (!configurationError.isNull())   _gui.showMessage("Configuration not read", _configurationFile + String(":\n") + configurationError);
  else                                     _gui.setStatus("No configuration file found: default settings");
  String latexProblem = applyLatex();
  if (!latexProblem.isNull()) _gui.setStatus(latexProblem);

  // composites saved by programs or earlier sessions (see MATH::FunctionRegistry::writeComposites())
  String composites = MATH::FunctionRegistry::defaultCompositesFile();
  if (FileSystem::isFile(composites))
    {
    String problems;
    int n = MATH::FunctionRegistry::readComposites(composites, &problems);
    if (n < 0) _gui.setStatus(String("Cannot read the composites of ") + composites);
    else if (!problems.isNull()) _gui.setStatus(String("Composites not read: ") + problems);
    }
  _gui.fitPane().onResidualMap = [this](const MATH::DataPoints * data, unsigned int fit) { showResidualMap(data, fit); };
  _gui.fitPane().onProjections = [this](const MATH::DataPoints * data) { showProjections(data); };
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
  pane.setKeepShapes(_configuration.keepShapes);
  pane.beforeChange = [this]() { recordUndo(); };
  pane.onDrop = [this, &pane](int graph, const ObjectReference & object) { showObject(pane, graph, object); };
  pane.onShowData = [this](const MATH::DataPoints * data, const String & label) { showDataTable(data, label); };
  pane.onFit = [this](int item) { _gui.openFitDialog(item); };
  pane.onResidualMap = [this](const MATH::DataPoints * data, unsigned int fit) { showResidualMap(data, fit); };
  pane.onProjections = [this](const MATH::DataPoints * data) { showProjections(data); };
  pane.onSaveGraph  = [this, &pane](int graph) { saveAs(paneIndex(pane), graph); };
  pane.onSaveCanvas = [this, &pane]() { saveAs(paneIndex(pane)); };
  pane.onRename     = [this, &pane]()
    {
    _renamePane = paneIndex(pane);
    _renameText = pane.title().str();
    _renameRequested = true;
    };
  pane.onCompare = [this](const MATH::DataPoints * a, const MATH::DataPoints * b, const String & la, const String & lb)
    { showComparison(a, b, la, lb); };
  }

  // ------------------------------------------------------------------------------------------------ menus

  void CapApp::buildMenus()
  {
  MenuBar & bar = _gui.menuBar();

  Menu & file = bar.addMenu("File");
  file.addItem("Open...",        "file.open",       "Ctrl+O",       "Open a CAP file, or the histograms of a ROOT file");
  file.addItem("Open Folder...", "file.openFolder", "Ctrl+Shift+O", "Show a folder of the file system in the browser");
  file.addItem("Close All Files", "file.closeAll",  "",             "Close the CAP files open in the browsers");
  file.addSeparator();
  file.addItem("New Browser",    "file.newBrowser", "",             "Open another file browser, with files of its own");
  file.addItem("New Tree",       "file.newTree",    "",             "Make a new, empty tree in the browser used last: "
                                                                     "add folders, drag objects onto it, save it");
  file.addSeparator();
  file.addItem("Save",           "file.save",       "Ctrl+S",       "Save the canvases shown in the window again (SVG or PDF)");
  file.addItem("Save Window As...", "file.saveAs",  "Ctrl+Shift+S", "Save the canvases, laid out as in the window, to a new SVG or PDF file");
  file.addItem("Save Canvas As...", "file.saveCanvas", "",          "Save the current canvas to an SVG or PDF file");
  file.addItem("Save Graph As...",  "file.saveGraph",  "",          "Save the selected graph of the current canvas to an SVG or PDF file");
  file.addSeparator();
  file.addItem("Save All Canvases...", "file.saveAll", "",          "Save every canvas to its own file in a folder (the format saved last)");
  file.addItem("Save All Graphs...",   "file.saveAllGraphs", "",    "Save every graph of every canvas to its own file in a folder");
  file.addItem("Save Book as PDF...",  "file.saveBook", "",         "Every canvas as a page of one PDF file, to print");
#ifdef CAP_GUI_PRESENTATIONS
  file.addItem("Make Presentation...", "file.presentation", "",     "A Beamer presentation (LaTeX) of the canvases, one frame each");
#endif
  file.addSeparator();
  file.addItem("Exit",           "file.exit",       "Ctrl+Q",       "Close the window and quit");

  Menu & edit = bar.addMenu("Edit");
  edit.addItem("Undo",  "edit.undo",   "Ctrl+Z",       "Undo the last change to the canvases");
  edit.addItem("Redo",  "edit.redo",   "Ctrl+Shift+Z", "Redo the change undone last");
  edit.addSeparator();
  edit.addItem("Cut",    "edit.cut",    "Ctrl+X", "Copy the contents of the selected graph, then clear it");
  edit.addItem("Copy",   "edit.copy",   "Ctrl+C", "Copy the contents of the selected graph");
  edit.addItem("Paste",  "edit.paste",  "Ctrl+V", "Show the copied objects in the selected graph");
  edit.addItem("Delete", "edit.delete", "Delete", "Remove the selected shape, or clear the selected graph");
  edit.addSeparator();
  edit.addItem("Clear Canvas",       "edit.clearCanvas", "", "Clear all the graphs of the current canvas");
  edit.addItem("Clear All Canvases", "edit.clearAll",    "", "Clear all the graphs of all the canvases");
  edit.addSeparator();
  edit.addItem("Fit...", "edit.fit", "", "Fit a function to data points, a histogram or a function of the selected graph");
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
  MenuItem & canvasList = view.addItem("Canvases", "view.canvases", "", "The canvases of the session by window, with their graphs");
  canvasList.setCheckable(true);
  MenuItem & fits = view.addItem("Fit Results", "view.fits", "", "Show the results of the fits of the selected graph, below the canvases");
  fits.setCheckable(true);
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
  window.addItem("Rename Canvas...", "window.rename",  "",       "Change the title of the current canvas pane");
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
  if (MenuItem * item = bar.findItem("view.fits"))          item->setChecked(_gui.fitPaneVisible());
  if (MenuItem * item = bar.findItem("view.canvases"))      item->setChecked(_gui.canvasListVisible());
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
    // the configured types that can be opened (CAP and ROOT files)
    std::vector<String> types;
    for (const auto & e : _configuration.extensions)
      {
      bool root = FileSystem::hasExtension(String("x") + e, ".root");
#ifndef CAP_WITH_ROOT_FILES
      if (root) continue;                                // ROOT files not supported: not offered
#endif
      if (root || FileSystem::hasExtension(String("x") + e, ".cap")) types.push_back(e);
      }
    if (types.empty()) types.push_back(".cap");
    _gui.fileDialog().open(FileDialog::kOpenFile, "Open a CAP or ROOT file", _configuration.directory, types, "",
                           [this](const String & path) { openFile(path); });
    }
  else if (c == "file.openFolder")
    {
    _gui.fileDialog().open(FileDialog::kOpenFolder, "Open a folder", _configuration.directory, {}, "",
                           [this](const String & path) { openFolder(path); });
    }
  else if (c == "file.save" && !_saveFile.isNull()) save(_saveFile);
  else if (c == "file.save" || c == "file.saveAs") saveAs();
  else if (c == "file.saveCanvas") saveAs(_gui.currentPaneIndex());
  else if (c == "file.saveGraph") saveAs(_gui.currentPaneIndex(), _gui.currentPane().selectedGraph());
  else if (c == "file.newBrowser")
    {
    FileBrowser & browser = _gui.addBrowser();
    _gui.setStatus(String("New browser: ") + String(browser.windowName()));
    }
  else if (c == "file.newTree") newTree(_gui.activeBrowser());
  else if (c == "file.closeAll")
    {
    std::vector<int> ids;
    for (const auto & f : _files) ids.push_back(f.id);
    for (int id : ids) closeFile(id);
    }
  else if (c == "file.exit") _gui.requestExit();
  else if (c == "view.browser") _gui.setBrowserVisible(!_gui.browserVisible());
  else if (c == "view.fits") _gui.setFitPaneVisible(!_gui.fitPaneVisible());
  else if (c == "view.canvases") _gui.setCanvasListVisible(!_gui.canvasListVisible());
  else if (c == "file.saveAll" || c == "file.saveAllGraphs")
    {
    bool perGraph = c == "file.saveAllGraphs";
    _gui.fileDialog().open(FileDialog::kOpenFolder, perGraph ? "Folder for all the graphs" : "Folder for all the canvases",
                           _configuration.directory, {}, "", [this, perGraph](const String & folder)
      {
      std::vector<String> files = GRAPHICS::CanvasRegistry::session().saveAll(folder, _saveFormat, perGraph);
      _configuration.directory = folder;
      _gui.setStatus(String(std::to_string(files.size()).c_str()) + String(" files written to ") + folder);
      });
    }
  else if (c == "file.saveBook")
    {
    _gui.fileDialog().open(FileDialog::kSaveFile, "Save all the canvases as one PDF", _configuration.directory, {".pdf"},
                           "canvases.pdf", [this](const String & chosen)
      {
      String path = FileSystem::hasExtension(chosen, ".pdf") ? chosen : chosen + String(".pdf");
      if (GRAPHICS::CanvasRegistry::session().writeBook(path))
        {
        _configuration.directory = FileSystem::parent(path);
        _gui.setStatus(String("Saved ") + path + String(" (") + String(std::to_string(GRAPHICS::CanvasRegistry::session().size()).c_str()) + String(" pages)"));
        }
      else _gui.showMessage("Save Book", String("Cannot write ") + path);
      });
    }
  else if (c == "file.presentation") _presentation.requested = true;
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
    if (pane.mode() == CanvasPane::kReplace) { pane.currentGraph().clearItems(); if (!pane.keepShapes()) pane.currentGraph().clearShapes(); }
    for (const auto & item : _clipboard) pane.currentGraph().add(item);
    }
  else if (c == "edit.delete" && _gui.currentPane().hasSelectedShape()) _gui.currentPane().deleteSelectedShape();
  else if (c == "edit.delete")      { recordUndo(); _gui.currentPane().currentGraph().clearItems(); }
  else if (c == "edit.clearCanvas")
    {
    recordUndo();
    Canvas & canvas = _gui.currentPane().canvas();
    for (int k=0; k<canvas.nGraphs(); k++) { canvas.graph(k).clearItems(); canvas.graph(k).clearShapes(); }
    }
  else if (c == "edit.clearAll")
    {
    recordUndo();
    for (int p=0; p<_gui.nPanes(); p++)
      for (int k=0; k<_gui.pane(p).canvas().nGraphs(); k++) { _gui.pane(p).canvas().graph(k).clearItems(); _gui.pane(p).canvas().graph(k).clearShapes(); }
    }
  else if (c == "edit.preferences") _showPreferences = true;
  else if (c == "edit.fit") _gui.openFitDialog();
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
  else if (c == "window.rename")
    {
    _renamePane = _gui.currentPaneIndex();
    _renameText = _gui.currentPane().title().str();
    _renameRequested = true;
    }
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

  //! Read a CAP file, or import the histograms of a ROOT file (kept in memory). @param error set to the problem
  //! @return the file, or nullptr
  std::shared_ptr<CapFile> readCapFile(const String & path, String & error)
  {
  try
    {
#ifdef CAP_WITH_ROOT_FILES
    if (FileSystem::hasExtension(path, ".root"))
      {
      auto file = std::make_shared<CapFile>(String(""), File::kReadWrite);
      file->setFolderName(FileSystem::stem(path));
      importRootFile(path, *file);
      return file;
      }
#else
    if (FileSystem::hasExtension(path, ".root"))
      { error = "this CapGui is built without ROOT files (BuildCAP.py: ROOT files)"; return nullptr; }
#endif
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

  int CapApp::openFile(const String & fileName, FileBrowser * browser)
  {
  FileBrowser & target = browser ? *browser : _gui.activeBrowser();
  String path = normalizedPath(fileName);
  for (const auto & f : _files)
    if (!f.editable && f.path == path)                   // open already: shared with this browser
      {
      target.showFile(f.id);
      _gui.setBrowserVisible(true);
      target.reveal(path);
      return f.id;
      }
  if (!FileSystem::isFile(path)) { _gui.showMessage("Open", String("No such file: ") + path); return -1; }
  String error;
  std::shared_ptr<CapFile> capFile = readCapFile(path, error);
  if (!capFile) { _gui.showMessage("Open", String("Cannot read ") + path + String(":\n") + error); return -1; }
  BrowserFile entry;
  entry.id   = _nextFileId++;
  entry.path = path;
  entry.file = capFile;
  _files.push_back(entry);
  target.showFile(entry.id);
  _configuration.directory = FileSystem::parent(path);
  _gui.setBrowserVisible(true);
  target.reveal(path);
  _gui.setStatus(String("Opened ") + path);
  return entry.id;
  }

  void CapApp::openFolder(const String & folder)
  {
  String path = normalizedPath(folder);
  if (!FileSystem::isDirectory(path)) { _gui.showMessage("Open Folder", String("No such folder: ") + path); return; }
  _gui.activeBrowser().addFolder(path);
  _gui.setBrowserVisible(true);
  _configuration.directory = path;
  _gui.setStatus(String("Folder ") + path);
  }

  void CapApp::closeFile(int id, FileBrowser * browser)
  {
  if (browser)
    {
    browser->hideFile(id);
    for (const auto & b : _gui.browsers())
      if (b->showsFile(id)) { _gui.setStatus("Closed in this browser (still shown in another one)"); return; }
    }
  else
    for (const auto & b : _gui.browsers()) b->hideFile(id);
  for (auto it = _files.begin(); it != _files.end(); ++it)
    {
    if (it->id != id) continue;
    _gui.setStatus(String("Closed ") + it->path);
    _closedFiles.push_back(*it);
    _files.erase(it);
    return;
    }
  }

  void CapApp::connectBrowser(FileBrowser & browser)
  {
  FileBrowser * b = &browser;
  browser.setFiles(&_files);
  browser.onOpenFile   = [this, b](const String & path) { return openFile(path, b); };
  browser.onShowObject = [this](const ObjectReference & object) { showObject(object); };
  browser.onShowData   = [this](const ObjectReference & object) { showData(object); };
  browser.onShowFolder = [this](int file, const String & folder) { showFolder(file, folder); };
  browser.onCloseFile  = [this, b](int file) { closeFile(file, b); };
  browser.onReloadFile = [this](int file) { reloadFile(file); };
  browser.onCommand    = [this](const String & command) { actionPerformed(ActionEvent{command, nullptr}); };
  browser.onNewTree    = [this, b]() { newTree(*b); };
  browser.onCopy       = [this](const ObjectReference & source, bool folder, int file, const String & path)
                           { copyInto(source, folder, file, path); };
  browser.onNewFolder  = [this](int file, const String & path, const String & name) { newFolder(file, path, name); };
  browser.onRename     = [this](int file, const String & path, const String & name) { renameEntry(file, path, name); };
  browser.onRemove     = [this](int file, const String & path) { removeEntry(file, path); };
  browser.onSaveTree   = [this](int file) { saveTree(file); };
#ifdef CAP_WITH_ROOT_FILES
  browser.onExportRoot = [this](int file, const String & path) { exportRoot(file, path); };
#endif
  browser.onSaveCopy   = [this](int file, const String & path) { saveCopy(file, path); };
  }

  namespace
  {
  //! Folder of a tree at a path ("" for its top), or null.
  Folder * treeFolder(BrowserFile & tree, const String & path)
  {
  Folder & top = *tree.file;
  return path.isEmpty() ? &top : top.folder(path);
  }
  } // namespace

  int CapApp::newTree(FileBrowser & browser)
  {
  BrowserFile tree;
  tree.id       = _nextFileId++;
  tree.editable = true;
  tree.name     = String("Tree ") + String(std::to_string(++_treeCounter).c_str());
  tree.file     = std::make_shared<CapFile>(String(""), File::kReadWrite);
  tree.file->setFolderName(tree.name);
  _files.push_back(tree);
  browser.showFile(tree.id);
  _gui.setBrowserVisible(true);
  _gui.setStatus(tree.name + String(": drag objects or folders onto it to copy them; right-click to add folders or "
                                    "save it"));
  return tree.id;
  }

  bool CapApp::copyInto(const ObjectReference & source, bool folder, int target, const String & folderPath)
  {
  BrowserFile * from = file(source.file);
  BrowserFile * to   = file(target);
  if (!from || !to || !from->file || !to->file) return false;
  if (!to->editable)
    {
    _gui.showMessage("Copy", "Files read from disk cannot be modified: copy into a tree (New Tree).");
    return false;
    }
  // a folder cannot be copied into itself or one of its sub-folders
  std::string sourcePath = source.path.str(), targetPath = folderPath.str();
  if (folder && from->id == to->id
      && (sourcePath.empty() || targetPath == sourcePath || targetPath.compare(0, sourcePath.size()+1, sourcePath+"/") == 0))
    {
    _gui.showMessage("Copy", "A folder cannot be copied into itself or into one of its folders.");
    return false;
    }
  Folder * destination = treeFolder(*to, folderPath);
  if (!destination) return false;
  try
    {
    String copied;
    if (folder && source.path.isEmpty())
      {
      // the whole tree of a file: a folder named after the file, holding copies of its entries
      Folder & whole = *from->file;
      String name = from->editable ? from->name : FileSystem::stem(from->path);
      Folder * copy = destination->addFolder(destination->uniqueName(name));
      for (const auto & entry : whole.names()) copy->copyEntry(whole, entry);
      copied = copy->name();
      }
    else
      {
      std::string leaf = sourcePath.substr(sourcePath.rfind('/') == std::string::npos ? 0 : sourcePath.rfind('/') + 1);
      copied = destination->copyEntry(*from->file, source.path, destination->uniqueName(String(leaf)));
      }
    _gui.setStatus(String("Copied ") + copied + String(" into ") + to->name
                   + (folderPath.isEmpty() ? String("") : String("/") + folderPath));
    return true;
    }
  catch (const std::exception & e)
    {
    _gui.showMessage("Copy", String("Cannot copy ") + source.path + String(":\n") + String(e.what()));
    return false;
    }
  }

  void CapApp::newFolder(int target, const String & folderPath, const String & name)
  {
  BrowserFile * tree = file(target);
  if (!tree || !tree->editable) return;
  Folder * parent = treeFolder(*tree, folderPath);
  if (!parent) return;
  try
    {
    if (parent->contains(name)) { _gui.showMessage("New Folder", String("An entry named ") + name + String(" exists")); return; }
    parent->addFolder(name);
    _gui.setStatus(String("New folder ") + name);
    }
  catch (const std::exception & e) { _gui.showMessage("New Folder", String(e.what())); }
  }

  void CapApp::renameEntry(int target, const String & path, const String & name)
  {
  BrowserFile * tree = file(target);
  if (!tree || !tree->editable) return;
  try
    {
    Folder & top = *tree->file;
    top.rename(path, name);
    _gui.setStatus(String("Renamed ") + path + String(" to ") + name);
    }
  catch (const std::exception & e) { _gui.showMessage("Rename", String(e.what())); }
  }

  void CapApp::removeEntry(int target, const String & path)
  {
  BrowserFile * tree = file(target);
  if (!tree || !tree->editable) return;
  Folder & top = *tree->file;
  std::unique_ptr<FolderItem> item = top.take(path);
  if (!item) return;
  _removed.push_back(std::move(item));                   // the graphs showing its objects keep them
  _gui.setStatus(String("Removed ") + path);
  }

  void CapApp::saveTree(int target)
  {
  BrowserFile * tree = file(target);
  if (!tree || !tree->editable) return;
  String name = tree->path.isEmpty() ? tree->name + String(".cap") : FileSystem::name(tree->path);
  _gui.fileDialog().open(FileDialog::kSaveFile, "Save the tree", _configuration.directory, {".cap"}, name,
                         [this, target](const String & chosen) { saveTreeTo(target, chosen); });
  }

#ifdef CAP_WITH_ROOT_FILES
  void CapApp::exportRoot(int target, const String & folderPath)
  {
  BrowserFile * entry = file(target);
  if (!entry || !entry->file) return;
  String base = folderPath.isEmpty() ? (entry->editable ? entry->name : FileSystem::stem(entry->path)) : FileSystem::name(folderPath);
  _gui.fileDialog().open(FileDialog::kSaveFile, "Export to a ROOT file", _configuration.directory, {".root"},
                         base + String(".root"), [this, target, folderPath](const String & chosen)
    {
    BrowserFile * from = file(target);
    if (!from || !from->file) return;
    String path = FileSystem::hasExtension(chosen, ".root") ? chosen : chosen + String(".root");
    Folder & top = from->file->folder();
    Folder * folder = folderPath.isEmpty() ? &top : top.folder(folderPath);
    if (!folder) { _gui.showMessage("Export to ROOT", String("No folder ") + folderPath); return; }
    try
      {
      std::vector<String> skipped;
      int n = exportRootFile(*folder, path, &skipped);
      _configuration.directory = FileSystem::parent(path);
      String status = String(std::to_string(n).c_str()) + String(n == 1 ? " histogram" : " histograms") + String(" written to ") + path;
      if (!skipped.empty()) status += String(" (") + String(std::to_string(skipped.size()).c_str()) + String(" other objects skipped)");
      _gui.setStatus(status);
      }
    catch (const std::exception & e) { _gui.showMessage("Export to ROOT", String(e.what())); }
    });
  }
#else
  void CapApp::exportRoot(int, const String &) { }
#endif

  void CapApp::saveCopy(int target, const String & folderPath)
  {
  BrowserFile * entry = file(target);
  if (!entry || !entry->file) return;
  String base = folderPath.isEmpty() ? (entry->editable ? entry->name : FileSystem::stem(entry->path)) : FileSystem::name(folderPath);
  _gui.fileDialog().open(FileDialog::kSaveFile, folderPath.isEmpty() ? "Save as a CAP file" : "Save the folder as a CAP file",
                         _configuration.directory, {".cap"}, base + String(".cap"),
                         [this, target, folderPath](const String & chosen) { saveCopyTo(target, folderPath, chosen); });
  }

  bool CapApp::saveCopyTo(int target, const String & folderPath, const String & fileName)
  {
  BrowserFile * entry = file(target);
  if (!entry || !entry->file) return false;
  String path = normalizedPath(FileSystem::hasExtension(fileName, ".cap") ? fileName : fileName + String(".cap"));
  for (const auto & f : _files)
    if (!f.editable && f.path == path)
      {
      _gui.showMessage("Save", String("The file ") + path + String(" is open: close it first (its objects are read from it)."));
      return false;
      }
  try
    {
    Folder & top = entry->file->folder();
    Folder * folder = folderPath.isEmpty() ? &top : top.folder(folderPath);
    if (!folder) { _gui.showMessage("Save", String("No folder ") + folderPath); return false; }
    folder->loadAll();                                   // objects of a file not read yet: read before the copy
    CapFile output(path, File::kWriteOnly);
    output = *folder;
    output.setFolderName(FileSystem::stem(path));
    output.write();
    _configuration.directory = FileSystem::parent(path);
    _gui.setStatus(String("Saved ") + path);
    return true;
    }
  catch (const std::exception & e)
    {
    _gui.showMessage("Save", String("Cannot save ") + path + String(":\n") + String(e.what()));
    return false;
    }
  }

  bool CapApp::saveTreeTo(int target, const String & fileName)
  {
  BrowserFile * tree = file(target);
  if (!tree || !tree->editable) return false;
  String path = normalizedPath(FileSystem::hasExtension(fileName, ".cap") ? fileName : fileName + String(".cap"));
  for (const auto & f : _files)
    if (!f.editable && f.path == path)
      {
      _gui.showMessage("Save Tree", String("The file ") + path + String(" is open: close it first (its objects are "
                       "read from it)."));
      return false;
      }
  try
    {
    Folder & content = *tree->file;
    CapFile output(path, File::kWriteOnly);
    output = content;
    output.setFolderName(FileSystem::stem(path));
    output.write();
    tree->path = path;
    tree->name = FileSystem::stem(path);
    tree->file->setFolderName(tree->name);
    _configuration.directory = FileSystem::parent(path);
    _gui.setStatus(String("Tree saved to ") + path);
    return true;
    }
  catch (const std::exception & e)
    {
    _gui.showMessage("Save Tree", String("Cannot save ") + path + String(":\n") + String(e.what()));
    return false;
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

  void CapApp::showResidualMap(const MATH::DataPoints * data, unsigned int k)
  {
  if (!data || k >= data->nFits() || data->rank() != 2) return;
  const MATH::FitRecord & fit = data->fit(k);
  String label = fit.label.isNull() ? String("fit") : fit.label;
  MATH::DataPoints * map = nullptr;
  try
    {
    if (const MATH::Histogram * histogram = dynamic_cast<const MATH::Histogram*>(data))
      {
      bool averaged = fit.result.method.contains("averaged over the bins");   // pulls as the fit compared them
      map = new MATH::Histogram(MATH::residualMap(*histogram, fit, MATH::Residual::kPull, averaged));
      }
    else
      {
      MATH::DataPointsVector * points = new MATH::DataPointsVector(MATH::residuals(*data, fit));
      points->setName(data->name() + String("_pulls"));
      points->setTitle(String("pulls of ") + label);
      map = points;
      }
    }
  catch (const std::exception & e) { _gui.setStatus(String("No residual map: ") + String(e.what())); return; }
  _data.emplace_back(map);
  recordUndo();
  CanvasPane & pane = _gui.addPane();
  pane.divide(1, 1);
  Graph & graph = pane.canvas().graph(0);
  graph.show(GraphItem(map, String("pulls of ") + label));
  if (const MATH::Histogram * histogram = dynamic_cast<const MATH::Histogram*>(data))
    {
    graph.xAxis().setTitle(histogram->dimensionTitle(0));
    graph.yAxis().setTitle(histogram->dimensionTitle(1));
    }
  graph.setResidualMapStyle(true);
  _gui.setStatus(String("Pulls of ") + data->name() + String(" to ") + label + String(" (data - fit)/uncertainty"));
  }

  void CapApp::showComparison(const MATH::DataPoints * first, const MATH::DataPoints * second, const String & firstLabel,
                              const String & secondLabel)
  {
  const MATH::Histogram * a = dynamic_cast<const MATH::Histogram*>(first);
  const MATH::Histogram * b = dynamic_cast<const MATH::Histogram*>(second);
  if (!a || !b) { _gui.setStatus("Compare: two histograms are needed"); return; }
  if (a->rank() != b->rank() || !a->compatibleWith(*b))
    {
    _gui.showMessage("Compare", firstLabel + String(" and ") + secondLabel + String(" do not have the same binning: "
                     "they cannot be compared bin by bin."));
    return;
    }
  MATH::Histogram * difference = nullptr;
  try { difference = new MATH::Histogram(*b - *a); }
  catch (const std::exception & e) { _gui.showMessage("Compare", String("No difference: ") + String(e.what())); return; }
  String differenceLabel = secondLabel + String(" - ") + firstLabel;
  difference->setName(differenceLabel);
  difference->setTitle(differenceLabel);
  _data.emplace_back(difference);
  recordUndo();
  CanvasPane & pane = _gui.addPane();
  if (a->rank() == 1)
    {
    pane.divide(1, 2);
    Graph & overlay = pane.canvas().graph(0);
    overlay.show(GraphItem(a, firstLabel));
    overlay.show(GraphItem(b, secondLabel));              // added to the first one
    pane.canvas().graph(1).show(GraphItem(difference, differenceLabel));
    }
  else
    {
    pane.divide(3, 1);
    pane.canvas().graph(0).show(GraphItem(a, firstLabel));
    pane.canvas().graph(1).show(GraphItem(b, secondLabel));
    pane.canvas().graph(2).show(GraphItem(difference, differenceLabel));
    }
  _comparison = Comparison();
  _comparison.first = a;
  _comparison.second = b;
  _comparison.firstLabel = firstLabel;
  _comparison.secondLabel = secondLabel;
  _comparison.open = true;
  computeComparison();
  _gui.setStatus(String("Comparison of ") + firstLabel + String(" and ") + secondLabel + String(": ")
                 + _comparison.chiSquare);
  }

  void CapApp::computeComparison()
  {
  Comparison & c = _comparison;
  if (!c.first || !c.second) return;
  static const MATH::HistogramComparison modes[3] = { MATH::HistogramComparison::kCounts,
                                                      MATH::HistogramComparison::kValues,
                                                      MATH::HistogramComparison::kShapes };
  try { c.chiSquare = MATH::histogramCompatibilityTest(*c.first, *c.second, modes[c.mode]).summary(); }
  catch (const std::exception & e) { c.chiSquare = String("cannot be computed: ") + String(e.what()); }
  if (c.first->rank() != 1) { c.kolmogorov = "for histograms of rank 1 only"; return; }
  try { c.kolmogorov = MATH::kolmogorovSmirnovTest(*c.first, *c.second).summary(); }
  catch (const std::exception & e) { c.kolmogorov = String("cannot be computed: ") + String(e.what()); }
  }

  void CapApp::renderComparison()
  {
  Comparison & c = _comparison;
  if (!c.open) return;
  ImGui::SetNextWindowSize(ImVec2(560, 300), ImGuiCond_FirstUseEver);
  if (ImGui::Begin("Comparison", &c.open))
    {
    ImGui::Text("%s", c.firstLabel.data());
    ImGui::TextDisabled("and");
    ImGui::Text("%s", c.secondLabel.data());
    ImGui::Separator();
    ImGui::Text("Chi-square compatibility test");
    const char * modes[3] = { "Counts (unweighted histograms)", "Values (with their uncertainties)",
                              "Shapes (normalized to the same integral)" };
    ImGui::SetNextItemWidth(320);
    if (ImGui::Combo("##mode", &c.mode, modes, 3)) computeComparison();
    ImGui::SetItemTooltip("Counts: Poisson counts, as ROOT's Chi2Test (UU); values: bin values with their "
                          "uncertainties; shapes: values normalized to the same integral");
    ImGui::TextWrapped("%s", c.chiSquare.data());
    ImGui::Separator();
    ImGui::Text("Kolmogorov-Smirnov test");
    ImGui::TextWrapped("%s", c.kolmogorov.data());
    }
  ImGui::End();
  }

  void CapApp::showProjections(const MATH::DataPoints * data)
  {
  const MATH::Histogram * histogram = dynamic_cast<const MATH::Histogram*>(data);
  if (!histogram || histogram->rank() != 2) { _gui.setStatus("Projections are made of histograms of rank 2"); return; }
  MATH::Histogram * projections[2] = { nullptr, nullptr };
  try
    {
    for (unsigned int axis=0; axis<2; axis++)
      {
      projections[axis] = new MATH::Histogram(MATH::projection(*histogram, axis));
      projections[axis]->setName(histogram->name() + String(axis == 0 ? "_px" : "_py"));
      projections[axis]->setTitle(histogram->title() + String(axis == 0 ? " (x projection)" : " (y projection)"));
      _data.emplace_back(projections[axis]);
      }
    }
  catch (const std::exception & e) { _gui.setStatus(String("No projections: ") + String(e.what())); return; }
  recordUndo();
  CanvasPane & pane = _gui.addPane();
  pane.divide(2, 1);
  for (unsigned int axis=0; axis<2; axis++)
    {
    Graph & graph = pane.canvas().graph(int(axis));
    graph.show(GraphItem(projections[axis], histogram->name()));
    graph.setShowResiduals(true);
    }
  _gui.setStatus(String("Projections of ") + histogram->name() + String(" with its fits projected"));
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
  // label: the name, or the path when the graph already shows an object of the same name
  String name = data->name().isNull() ? object.path : data->name();
  String label = name;
  for (const auto & item : graph.items()) if (item.label() == name && item.object() != data) label = object.path;
  pane.show(GraphItem(data, label));   // default style; on a fresh graph, titles from the object (Graph::show())
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
  recordUndo();
  CanvasPane & pane = _gui.currentPane();
  pane.canvas().divideFor(n);                          // the grid of Canvas::showOnePerGraph()
  pane.divide(pane.canvas().nColumns(), pane.canvas().nRows());
  for (int k=0; k<pane.canvas().nGraphs(); k++)
    {
    pane.canvas().graph(k).clearItems();
    if (!pane.keepShapes()) pane.canvas().graph(k).clearShapes();
    }
  std::function<void()> hook = pane.beforeChange;
  pane.beforeChange = nullptr;                          // one undo step for the whole folder
  CanvasPane::Mode mode = pane.mode();
  pane.setMode(CanvasPane::kReplace);
  for (int k=0; k<n; k++) showObject(pane, k, objects[k]);
  pane.setMode(mode);
  pane.beforeChange = hook;
  pane.setSelectedGraph(0);
  }

  int CapApp::paneIndex(const CanvasPane & pane)
  {
  for (int k=0; k<_gui.nPanes(); k++) if (&_gui.pane(k) == &pane) return k;
  return -1;
  }

  void CapApp::saveAs(int pane, int graph)
  {
  if (pane >= _gui.nPanes()) return;
  // default name: the last file of the window, the title of the canvas, or of the graph
  auto fileName = [](const String & text)
    {
    std::string name;
    for (char ch : text.str()) name += (std::isalnum((unsigned char) ch) || ch == '-' || ch == '_' || ch == '.') ? ch : '_';
    return name;
    };
  String name;
  if (pane < 0) name = _saveFile.isNull() ? String("canvases") + _saveFormat : FileSystem::name(_saveFile);
  else
    {
    CanvasPane & p = _gui.pane(pane);
    std::string base = fileName(p.title());
    if (graph >= 0 && graph < p.canvas().nGraphs())
      {
      const String & title = p.canvas().graph(graph).title();
      base = title.isNull() ? base + "_" + std::to_string(graph + 1) : fileName(title);
      }
    name = String(base.c_str()) + _saveFormat;
    }
  const char * title = pane < 0 ? "Save the window" : (graph < 0 ? "Save the canvas" : "Save the graph");
  std::vector<String> types = _saveFormat == ".pdf" ? std::vector<String>{".pdf", ".svg"} : std::vector<String>{".svg", ".pdf"};
  _gui.fileDialog().open(FileDialog::kSaveFile, title, _configuration.directory, types, name,
                         [this, pane, graph](const String & path) { save(path, pane, graph); });
  }

  void CapApp::save(const String & fileName, int pane, int graph)
  {
  String name = fileName;
  if (!name.endsWith(".svg") && !name.endsWith(".pdf")) name += _saveFormat;
  if (_gui.saveFigure(name, pane, graph))
    {
    if (pane < 0) _saveFile = name;
    _saveFormat = name.endsWith(".pdf") ? ".pdf" : ".svg";
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
  try
    {
    MATH::Normal model(1.0, 0.0, 1.0);                      // a fit, attached to the data (see the Fits pane)
    MATH::Fitter fitter(model, *data);
    fitter.fit();
    data->addFit(fitter.record("Gaussian fit"));
    }
  catch (const Exception &) { }
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
  renderRenameCanvas();
  renderPresentation();
  renderComparison();
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
  ImGui::SameLine(0, 20);
  ImGui::Checkbox("Keep shapes", &c.keepShapes);
  ImGui::SetItemTooltip("Shapes and texts stay in a graph when its contents are replaced");

  ImGui::SeparatorText("Palettes of the session");
  {
  // the current palettes: graphs that do not choose their own follow them (colors and markers of the objects shown
  // from now on, shading of the new graphs); "Apply to all graphs" restyles the graphs already there
  GRAPHICS::Palettes & palettes = GRAPHICS::Palettes::session();
  auto combo = [](const char * label, String & current, const std::vector<String> & names, const char * tip)
    {
    bool changed = false;
    ImGui::SetNextItemWidth(170);
    if (ImGui::BeginCombo(label, current.data()))
      {
      for (const String & n : names)
        if (ImGui::Selectable(n.data(), n == current)) { current = n; changed = true; }
      ImGui::EndCombo();
      }
    ImGui::SetItemTooltip("%s", tip);
    return changed;
    };
  if (combo("colors", c.colorPalette, palettes.colorPaletteNames(), "Colors given in turn to the objects of a graph"))
    palettes.setColorPalette(c.colorPalette);
  ImGui::SameLine();
  if (combo("markers", c.markerPalette, palettes.markerPaletteNames(), "Markers given in turn to the objects of a graph"))
    palettes.setMarkerPalette(c.markerPalette);
  if (combo("shading", c.shadingPalette, palettes.shadingPaletteNames(), "Colors of the maps, contours and 3D views of new graphs"))
    palettes.setShadingPalette(c.shadingPalette);
  ImGui::SameLine();
  if (ImGui::Button("Apply to all graphs"))
    {
    recordUndo();
    for (int p=0; p<_gui.nPanes(); p++)
      for (int k=0; k<_gui.pane(p).canvas().nGraphs(); k++)
        {
        GRAPHICS::Graph & g = _gui.pane(p).canvas().graph(k);
        g.restyleItems();                                    // with its own palettes, or the session's
        g.setPalette(palettes.shadingPalette());
        }
    }
  ImGui::SetItemTooltip("Give every object of every graph its color and marker from the palettes again, and every graph "
                        "the shading palette (Edit > Undo to go back)");
  }

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

  void CapApp::renderRenameCanvas()
  {
  if (_renameRequested) { ImGui::OpenPopup("Rename Canvas##rename"); _renameRequested = false; }
  if (!ImGui::BeginPopupModal("Rename Canvas##rename", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;
  if (ImGui::IsWindowAppearing()) ImGui::SetKeyboardFocusHere();
  ImGui::SetNextItemWidth(260);
  bool enter = ImGui::InputText("title", &_renameText, ImGuiInputTextFlags_EnterReturnsTrue);
  bool valid = _renamePane >= 0 && _renamePane < _gui.nPanes() && !_renameText.empty();
  ImGui::BeginDisabled(!valid);
  if (ImGui::Button("Rename") || (enter && valid))
    {
    _gui.pane(_renamePane).setTitle(String(_renameText.c_str()));
    ImGui::CloseCurrentPopup();
    }
  ImGui::EndDisabled();
  ImGui::SameLine();
  if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) ImGui::CloseCurrentPopup();
  ImGui::EndPopup();
  }

  void CapApp::renderPresentation()
  {
#ifdef CAP_GUI_PRESENTATIONS
  if (_presentation.requested) { ImGui::OpenPopup("Presentation##dialog"); _presentation.requested = false; }
  if (!ImGui::BeginPopupModal("Presentation##dialog", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) return;
  PresentationSettings & p = _presentation;
  int n = GRAPHICS::CanvasRegistry::session().size();
  ImGui::Text("A Beamer presentation of the %d canvas%s of the session", n, n == 1 ? "" : "es");
  ImGui::SetNextItemWidth(320); ImGui::InputText("title", &p.title);
  ImGui::SetNextItemWidth(320); ImGui::InputText("author", &p.author);
  ImGui::SetNextItemWidth(320); ImGui::InputText("affiliation", &p.affiliation);
  ImGui::Checkbox("One frame per graph", &p.perGraph);
  ImGui::Checkbox("Compile with pdflatex", &p.compile);
  if (p.compile) { ImGui::SameLine(); ImGui::Checkbox("Open the PDF", &p.open); }
  ImGui::BeginDisabled(n == 0);
  if (ImGui::Button("Write..."))
    {
    ImGui::CloseCurrentPopup();
    _gui.fileDialog().open(FileDialog::kSaveFile, "Presentation (.tex, its figures beside it)", _configuration.directory,
                           {".tex"}, "presentation.tex", [this](const String & chosen) { makePresentation(chosen); });
    }
  ImGui::EndDisabled();
  ImGui::SameLine();
  if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) ImGui::CloseCurrentPopup();
  ImGui::EndPopup();
#endif
  }

  void CapApp::makePresentation(const String & texFile)
  {
#ifdef CAP_GUI_PRESENTATIONS
  const PresentationSettings & p = _presentation;
  CanvasPresentation presentation(String(p.title.c_str()));
  if (!p.author.empty()) presentation.setAuthor(String(p.author.c_str()), "", String(p.affiliation.c_str()));
  presentation.setOnePerGraph(p.perGraph);
  presentation.addCanvases(GRAPHICS::CanvasRegistry::session());
  String path = FileSystem::hasExtension(texFile, ".tex") ? texFile : texFile + String(".tex");
  String tex = presentation.write(FileSystem::parent(path), FileSystem::stem(path));
  if (tex.str().empty()) { _gui.showMessage("Presentation", String("Cannot write ") + path); return; }
  _configuration.directory = FileSystem::parent(path);
  if (!p.compile) { _gui.setStatus(String("Presentation written: ") + tex); return; }
  String log;
  if (!CanvasPresentation::compile(tex, &log))
    {
    _gui.showMessage("Presentation", String("pdflatex failed (") + tex + String("):\n") + String(log.str().substr(log.str().size() > 1500 ? log.str().size() - 1500 : 0)));
    return;
    }
  String pdf = FileSystem::withExtension(tex, ".pdf");
  _gui.setStatus(String("Presentation made: ") + pdf);
  if (p.open) std::system((String("open '") + pdf + String("' &")).data());
#else
  (void)texFile;
#endif
  }

  } // namespace GUI
} // namespace CAP
