/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file CapGui.hpp
//! @brief CapGui: main window of the CAP graphical interface (panes, browser, menus, status bar).
#ifndef CAP__GUI__CapGui
#define CAP__GUI__CapGui
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "String.hpp"
#include "MenuBar.hpp"
#include "CanvasPane.hpp"
#include "DataPane.hpp"
#include "FitDialog.hpp"
#include "DetectorMapsDialog.hpp"
#include "FitPane.hpp"
#include "FileDialog.hpp"
#include "FileBrowser.hpp"

struct GLFWwindow;
struct ImFont;

namespace CAP
{
  namespace GUI
  {

  //!
  //! @brief Settings of the CapGui window (read from the XML configuration by CapApp).
  //!
  struct GuiSettings
  {
  String title      = "CAP";   //!< title of the window
  int    width      = 1400;   //!< width of the window (points)
  int    height     = 900;   //!< height of the window (points)
  String theme      = "light";   //!< light, dark or classic
  float  fontSize   = 15.0f;     //!< points
  String fontFile;               //!< TrueType font; empty: the font distributed with CAP
  bool   viewports  = true;      //!< windows (e.g. the file browser) can be dragged out of the main window
  int    columns    = 1;         //!< canvas panes per row
  int    rows       = 1;         //!< rows of canvas panes
  bool   toolbars   = true;      //!< toolbar above each canvas
  bool   browserVisible = true;  //!< file browser shown
  bool   browserDocked  = true;  //!< file browser docked in the main window (otherwise in its own window)
  String iniFile;                //!< where ImGui keeps window positions; empty: none
  };

  //!
  //! @brief Main window of the CAP graphical interface: menu bar, canvas panes, file browser, status bar.
  //!
  //! Data panes (addDataPane()) show data points as tables; they are docked beside the canvases and closed the same
  //! way, but are not part of the canvas layout. The fit pane (FitPane) shows the fits of the selected graph of the
  //! current canvas; it is docked below the canvases, and opens by itself the first time a graph with fits is
  //! selected (setFitPaneVisible()).
  //!
  //! The canvas panes are docked windows, first arranged in a grid of n x m (setLayout()): the borders between them
  //! can be dragged to resize them, a pane can be dragged elsewhere (by its tab), added (addPane(), or the + button of
  //! a toolbar) and closed (the x of its tab; the last pane cannot be closed).
  //!
  //! The file browser is docked on the left of the canvases, or in a window of its own (setBrowserDocked()); it can
  //! also be docked elsewhere or dragged out with the mouse.
  //!
  //! CapGui only displays: what the menu commands do is decided by the application (CapApp), registered as listener
  //! on the menu bar. Built on Dear ImGui with GLFW and OpenGL; no ROOT class is used.
  //!
  //! @code
  //!   CapGui gui;
  //!   gui.initialize(settings);
  //!   gui.menuBar().addMenu("File").addItem("Exit", "file.exit", "Ctrl+Q");
  //!   gui.menuBar().addListener(&app);
  //!   gui.run();          // returns when the window is closed
  //! @endcode
  //!
  class CapGui
  {
  public:

  //! @brief Constructor: nothing is shown until initialize().
  CapGui();
  ~CapGui();

  //! @brief Open the window. @param settings window settings @return false if the window could not be created
  bool initialize(const GuiSettings & settings);
  //! @brief Run the event loop until the window is closed or requestExit() is called.
  void run();
  //! @brief Close the window at the end of the current frame (onCloseRequest may refuse).
  void requestExit();

  MenuBar & menuBar() { return _menuBar; }   //!< The menu bar (CapApp fills it)
  FileDialog & fileDialog() { return _fileDialog; }   //!< The file dialog (Open, Open Folder, Save As)

  //! @name File browser
  //! @{
  FileBrowser & browser() { return *_browsers.front(); }   //!< The first file browser
  //! @brief The file browser used last (clicked or focused), where File > Open opens files. @return browser
  FileBrowser & activeBrowser();
  //! @brief The open file browsers, the first one first. @return browsers
  const std::vector<std::unique_ptr<FileBrowser>> & browsers() const { return _browsers; }
  //! @brief Open another file browser, docked with the first one (see onNewBrowser). @return browser
  FileBrowser & addBrowser();
  //! @brief Index of the browser used last. @return index (0 for the first browser)
  int activeBrowserIndex() { activeBrowser(); return _activeBrowser; }
  //! @brief Close a browser other than the first at the start of the next frame (onCloseBrowser is called then);
  //! the first one is hidden. @param browser browser
  void closeBrowser(FileBrowser & browser);
  bool browserVisible() const { return _settings.browserVisible; }   //!< Whether the file browser is shown
  void setBrowserVisible(bool visible) { _settings.browserVisible = visible; }   //!< Show or hide the file browser
  bool browserDocked() const { return _settings.browserDocked; }   //!< Whether the browser is docked in the main window
  //! @brief Dock the browser on the left of the canvases, or put it in a window of its own. @param docked choice
  void setBrowserDocked(bool docked);
  //! @}

  //! @name Canvas panes
  //! @{

  //! @brief Arrange the canvases in nColumns x nRows panes; existing panes are kept, in order.
  void setLayout(int nColumns, int nRows);
  //! @brief Set the number of panes, arranged in a grid as square as possible (does nothing if it is already n).
  void setPaneCount(int n);
  //! @brief Add a pane, docked beside the current one, and make it current. @return the new pane
  CanvasPane & addPane();
  //! @brief Close pane k: not the last one, unless it was moved out of the main window (an empty pane then takes its
  //! place in the main window). @param k index of the pane
  void closePane(int k);
  //! @brief Ask to close a pane at the end of the frame (as its close buttons do: onClosePanes, then closePane()).
  //! @param k pane
  void requestClosePane(int k) { if (k >= 0 && k < nPanes()) _closeRequests.push_back(k); }
  int  nColumns() const { return _columns; }   //!< Columns of the last grid layout of the canvases
  int  nRows()    const { return _rows; }   //!< Rows of the last grid layout of the canvases
  int  nPanes()   const { return (int)_panes.size(); }   //!< Number of canvas panes
  CanvasPane & pane(int k) { return *_panes.at(k); }   //!< Canvas pane k
  CanvasPane & currentPane() { return *_panes.at(_currentPane); }   //!< The current canvas pane (target of Show, Copy, ...)
  int  currentPaneIndex() const { return _currentPane; }   //!< Index of the current canvas pane
  //! @brief Make canvas pane k current. @param k index @param focus give its window the keyboard focus
  void setCurrentPane(int k, bool focus=true) { if (k >= 0 && k < nPanes()) { _currentPane = k; if (focus) _focusRequested = true; } }
  //! @brief Show or hide the toolbars of all the canvas panes. @param v true to show them
  void setShowToolbars(bool v);
  bool showToolbars() const { return _settings.toolbars; }   //!< Whether the toolbars of the canvas panes are shown
  //! @name Windows of canvases
  //! Separate windows (OS windows when viewports are enabled), each holding canvas panes in a grid of its own, as
  //! the main window does (e.g. the windows of PlotHistos). Their canvases are listed in the registry of the session
  //! under the title of their window.
  //! @{
  //! @brief Open a window of canvases. @param title title @param columns columns of the grid @param rows rows
  //! @param width width (points) @param height height (points) @return index of the window
  int  addPaneWindow(const String & title, int columns, int rows, int width, int height);
  //! @brief Add a canvas pane to a window of canvases (laid out in its grid). @param window index of the window
  //! @return the pane
  CanvasPane & addPaneTo(int window);
  int  nPaneWindows() const { return int(_paneWindows.size()); }   //!< Number of windows of canvases
  //! @brief Set window of canvases k, made if needed (then the windows before it too): title and grid (its size is
  //! used when it is made or shown again). @param k index @param title title @param columns columns @param rows rows
  //! @param width width @param height height
  void setPaneWindow(int k, const String & title, int columns, int rows, int width, int height);
  //! @brief Set the number of canvas panes of window of canvases k (panes added or removed at the end; the window is
  //! laid out again if it changed). @param k index @param n number
  void setPaneWindowPaneCount(int k, int n);
  //! @brief Close the windows of canvases from index k on (their panes are removed). @param k first index
  void closePaneWindowsFrom(int k);
  //! @brief The panes of a window, in order. @param window index of a window of canvases, or -1 for the main window
  //! @return panes
  std::vector<CanvasPane *> panesOf(int window);
  //! @brief Title of window of canvases k. @param k index @return title
  const String & paneWindowTitle(int k) const { return _paneWindows.at(std::size_t(k)).title; }
  //! @brief Size and grid of window of canvases k as last shown. @param k index @param width width
  //! @param height height @param columns columns @param rows rows
  void paneWindowLayout(int k, int & width, int & height, int & columns, int & rows) const;
  //! @brief Dock a window of the program (e.g. the editor of PlotEditor) between the file browser and the canvases,
  //! taking a fraction of the width beside the browser; kept docked when the canvases are laid out again.
  //! @param windowName name of the ImGui window @param fraction fraction of the width of the canvas area
  void setSideWindow(const String & windowName, float fraction) { _sideWindow = windowName.str(); _sideFraction = fraction; _layoutRequested = true; }
  //! @brief Set the title of the main window (the window of the operating system). @param title title
  void setWindowTitle(const String & title);
  //! @brief Size of the main window. @param width width @param height height
  void windowSize(int & width, int & height) const;
  //! @brief Resize the main window. @param width width (points) @param height height (points)
  void setWindowSize(int width, int height);
  //! @}
  //! @brief Add a data pane (a table of data points), docked beside the current canvas pane. @return the new pane
  DataPane & addDataPane();
  int  nDataPanes() const { return (int)_dataPanes.size(); }   //!< Number of data panes
  DataPane & dataPane(int k) { return *_dataPanes.at(k); }   //!< Data pane k
  //! @brief Show or hide the fit pane (docked below the canvases the first time it is shown). @param visible choice
  void setFitPaneVisible(bool visible);
  bool fitPaneVisible() const { return _fitsVisible; }   //!< Whether the fit pane is shown
  FitPane & fitPane() { return _fitPane; }   //!< The fit pane
  //! @brief Show or hide the Canvases window: the canvases of the session by window, with their graphs, to go to any
  //! of them, and to save them all or make a presentation of them. @param visible choice
  void setCanvasListVisible(bool visible) { _canvasListVisible = visible; }
  bool canvasListVisible() const { return _canvasListVisible; }   //!< Whether the Canvases window is shown
  //! @brief Open the Fit window for the selected graph of the current canvas. @param item index of the item to fit (-1: any)
  void openFitDialog(int item=-1) { _fitDialog.open(item); }
  //! @brief Open the Detector Maps window, and a maps file if one is given. @param fileName file
  void openMapsDialog(const String & fileName = "") { _mapsDialog.open(fileName); }
  //! @brief The Detector Maps window. @return window
  DetectorMapsDialog & mapsDialog() { return _mapsDialog; }
  //! Called for each new pane (to set its graphs and undo hook).
  std::function<void(CanvasPane &)> onNewPane;
  //! Called for each file browser made (the first one included): the application connects it to its files.
  std::function<void(FileBrowser &)> onNewBrowser;
  //! Called when a browser other than the first is closed, before it is deleted.
  std::function<void(FileBrowser &)> onCloseBrowser;
  //! Called when the user closes a browser other than the first (the x of its window): true to close it now, false
  //! if the application closes it later (closeBrowser(), e.g. once its changed trees are saved).
  std::function<bool(FileBrowser &)> onBrowserCloseRequest;
  //! Called before a pane is added or closed by the user (undo).
  std::function<void()> beforePanesChange;
  //! Called with the panes closed by the user (x of their tab or of their toolbar, Window > Close Canvas), in
  //! decreasing order: the application may handle them (e.g. remove their canvases from a plot file) and returns those
  //! it leaves to closePane().
  std::function<std::vector<int>(const std::vector<int> &)> onClosePanes;
  //! @}

  //! @name Focus
  //! @{
  //! Part of the window used last: what File > Save, Duplicate and Close act on.
  enum Focus { kFocusCanvas, kFocusBrowser, kFocusSide };
  //! @brief Part of the window focused last (kept while a menu is open). @return part
  Focus lastFocus() const { return _lastFocus; }
  //! @brief Record the focus of a window of the application (the side window). @param focus part
  void setLastFocus(Focus focus) { _lastFocus = focus; }
  //! @}

  //! @name Messages
  //! @{

  //! @brief Message shown in the status bar for a few seconds.
  void setStatus(const String & message);
  //! @brief Modal message box. @param title title @param text message
  void showMessage(const String & title, const String & text);
  //! @}

  //! @name Appearance
  //! @{
  void setTheme(const String & theme);   //!< Set the color theme: "dark", "light" or "classic"
  const GuiSettings & settings() const { return _settings; }   //!< Current settings of the window
  //! @brief Font of the plots and the interface. @return font
  ImFont * font() const { return _font; }
  //! @}

  //!
  //! @brief Write a figure as shown on screen (1 pixel = 1 point) to an SVG or a PDF file (by its extension: ".pdf"
  //! for PDF, SVG otherwise): the whole window (all the canvases, laid out as on screen), one canvas, or one graph.
  //! @param fileName file @param pane canvas pane (-1: the whole window) @param graph graph of the pane (-1: the whole
  //! pane) @return false if there is nothing to draw or the file cannot be written
  //!
  bool saveFigure(const String & fileName, int pane=-1, int graph=-1);

  //! Called every frame inside the ImGui frame, to draw the application's own windows (preferences, about, ...).
  std::function<void()> onFrame;
  //! Called by the buttons of the Canvases window, with a menu command ("file.saveAll", "file.saveBook", ...).
  std::function<void(const String & command)> onCommand;
  //! Called when the user closes the window; return false to keep it open.
  std::function<bool()> onCloseRequest;

  protected:

  GLFWwindow * _window;   //!< the main window (GLFW)
  GuiSettings  _settings;   //!< settings of the window
  ImFont *     _font;   //!< font of the interface and of the plots
  MenuBar      _menuBar;   //!< menu bar
  FileDialog   _fileDialog;   //!< file dialog
  std::vector<std::unique_ptr<CanvasPane>> _panes;   //!< canvas panes
  int    _columns;   //!< last grid layout, and index of the current pane
  int    _rows;   //!< last grid layout, and index of the current pane
  int    _currentPane;   //!< last grid layout, and index of the current pane
  int    _paneCounter;   //!< for the names of new canvas panes
  String _status;   //!< message of the status bar
  double _statusTime;   //!< time the status message was set (it is shown for 10 s)
  String _messageTitle;   //!< message box to show
  String _messageText;   //!< message box to show
  bool   _messageRequested;   //!< the message box opens at the next frame
  bool   _exitRequested;   //!< close the window at the end of the frame
  float  _areaX;   //!< screen rectangle covering the canvas panes (for saveFigure())
  float  _areaY;   //!< screen rectangle covering the canvas panes (for saveFigure())
  float  _areaWidth;   //!< screen rectangle covering the canvas panes (for saveFigure())
  float  _areaHeight;   //!< screen rectangle covering the canvas panes (for saveFigure())
  std::string _iniFile;   //!< file where ImGui keeps window positions (kept alive for ImGui)
  String _hoverText;   //!< data coordinates under the mouse (shown in the status bar on the next frame)
  std::vector<std::unique_ptr<FileBrowser>> _browsers;   //!< file browsers (the first one cannot be closed)
  int _activeBrowser;   //!< index of the browser used last
  std::vector<FileBrowser *> _browsersToClose;   //!< browsers closed by closeBrowser(), at the next frame
  Focus _lastFocus = kFocusCanvas;   //!< part of the window focused last
  int _browserCounter;   //!< number of the last browser made
  bool   _firstFrame;   //!< the next frame is the first one (the dock layout is built)
  bool   _layoutRequested;   //!< rebuild the dock layout (browser on the left of the canvases)
  bool   _undockRequested;   //!< move the browser to a window of its own
  int    _gridColumns;       //!< columns of the grid in which the panes are laid out by the next layout
  std::vector<std::pair<std::string,std::string>> _dockBeside;   //!< (added pane, pane beside which it is docked)
  std::vector<int> _closeRequests;   //!< panes closed by the user during the frame
  std::vector<std::unique_ptr<DataPane>> _dataPanes;   //!< data panes (not part of the canvas layout)
  int    _dataCounter;       //!< for the names of the data panes
  bool   _addRequested;      //!< the + button of a toolbar was clicked
  int    _focusedPane;       //!< pane whose window had the focus at the last frame
  bool   _focusRequested;    //!< give the focus to the current pane (changed by the program)
  FitPane _fitPane;          //!< results of the fits of the selected graph
  FitDialog _fitDialog;      //!< the Fit window
  DetectorMapsDialog _mapsDialog;   //!< the Detector Maps window (efficiencies and smearers)
  bool   _fitsVisible = false;   //!< the fit pane is shown
  bool   _canvasListVisible = false;   //!< the Canvases window is shown
  std::map<unsigned int, int> _windowNumbers;   //!< number of each separate window (viewport id), for the groups
  bool   _fitsDockRequested = false;   //!< dock the fit pane below the canvases at the next frame
  bool   _fitsShownOnce = false;   //!< the fit pane was shown (it opens by itself only once)

  //! @brief Draw one frame: menu bar, dock space, panes, browser, dialogs, status bar.
  void renderFrame();
  //! @brief Draw the status bar. @param hovered menu item under the mouse, or null @param hoverText data coordinates under the mouse
  void renderStatusBar(const MenuItem * hovered, const String & hoverText);
  //! @brief Draw the canvas and data panes; add and close panes as asked. @param hoverText set to the data coordinates under the mouse
  void renderCanvases(String & hoverText);
  //! @brief Draw the message box, if one is shown.
  void renderMessage();
  //! @brief Draw the file browser window, if shown.
  void renderBrowser();
  //! @brief Lay out the dock space: the browser on the left (if docked), the canvas panes in a grid. @param dockId dock space
  void buildDockLayout(unsigned int dockId);
  //! @brief Dock the canvas panes in a grid of the given dock node (_gridColumns columns). @param nodeId dock node
  void buildPaneGrid(unsigned int nodeId);
  //! A window of canvases (see addPaneWindow())
  struct PaneWindow
  {
  String title;           //!< title
  std::string label;      //!< ImGui label: "title###CapPaneWindow<k>"
  int columns = 1;        //!< grid of the panes
  int rows = 1;           //!< grid of the panes
  int width = 1000;       //!< size (points)
  int height = 700;       //!< size (points)
  bool placed = false;    //!< placed and sized on screen
  bool layoutPending = true;   //!< the panes are to be docked in its grid
  };
  std::vector<PaneWindow> _paneWindows;   //!< windows of canvases
  std::string _sideWindow;   //!< window docked between the browser and the canvases ("" for none)
  float _sideFraction = 0.4f;   //!< its fraction of the width of the canvas area
  //! @brief Dock panes in a grid of a dock node. @param nodeId dock node @param panes panes @param columns columns
  void dockGrid(unsigned int nodeId, const std::vector<CanvasPane *> & panes, int columns);
  //! @brief Draw the windows of canvases (their dock spaces; the panes are drawn by renderCanvases()).
  void renderPaneWindows();
  //! @brief Draw the fit pane, if shown (opened by itself the first time the selected graph has fits).
  void renderFitPane();
  //! @brief The Canvases window, if shown.
  void renderCanvasList();
  //! @brief Dock the fit pane below the canvases.
  void dockFitPane();
  //! @brief Create a canvas pane (not yet docked). @return the pane
  CanvasPane & newPane();
  //! @brief Load the interface font and the fonts of the plots (see ImGuiPainter::setFont()).
  void loadFonts();
  };

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__CapGui
