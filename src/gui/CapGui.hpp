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
//! @file CapGui.hpp
//! @brief CapGui: main window of the CAP graphical interface (panes, browser, menus, status bar).
#ifndef CAP__GUI__CapGui
#define CAP__GUI__CapGui
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "String.hpp"
#include "MenuBar.hpp"
#include "CanvasPane.hpp"
#include "DataPane.hpp"
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
  //! way, but are not part of the canvas layout.
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
  FileBrowser & browser() { return _browser; }   //!< The file browser
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
  //! @brief Close pane k (not the last one). @param k index of the pane
  void closePane(int k);
  int  nColumns() const { return _columns; }   //!< Columns of the last grid layout of the canvases
  int  nRows()    const { return _rows; }   //!< Rows of the last grid layout of the canvases
  int  nPanes()   const { return (int)_panes.size(); }   //!< Number of canvas panes
  CanvasPane & pane(int k) { return *_panes.at(k); }   //!< Canvas pane k
  CanvasPane & currentPane() { return *_panes.at(_currentPane); }   //!< The current canvas pane (target of Show, Copy, ...)
  int  currentPaneIndex() const { return _currentPane; }   //!< Index of the current canvas pane
  void setCurrentPane(int k) { if (k >= 0 && k < nPanes()) { _currentPane = k; _focusRequested = true; } }   //!< Make canvas pane k current (and give its window the focus)
  //! @brief Show or hide the toolbars of all the canvas panes. @param v true to show them
  void setShowToolbars(bool v);
  bool showToolbars() const { return _settings.toolbars; }   //!< Whether the toolbars of the canvas panes are shown
  //! @brief Add a data pane (a table of data points), docked beside the current canvas pane. @return the new pane
  DataPane & addDataPane();
  int  nDataPanes() const { return (int)_dataPanes.size(); }   //!< Number of data panes
  DataPane & dataPane(int k) { return *_dataPanes.at(k); }   //!< Data pane k
  //! Called for each new pane (to set its graphs and undo hook).
  std::function<void(CanvasPane &)> onNewPane;
  //! Called before a pane is added or closed by the user (undo).
  std::function<void()> beforePanesChange;
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

  //! @brief Write the canvases, laid out as on screen, to an SVG file. @return false if it cannot be written
  bool savePageSvg(const String & fileName);

  //! Called every frame inside the ImGui frame, to draw the application's own windows (preferences, about, ...).
  std::function<void()> onFrame;
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
  float  _areaX;   //!< screen rectangle covering the canvas panes (for savePageSvg())
  float  _areaY;   //!< screen rectangle covering the canvas panes (for savePageSvg())
  float  _areaWidth;   //!< screen rectangle covering the canvas panes (for savePageSvg())
  float  _areaHeight;   //!< screen rectangle covering the canvas panes (for savePageSvg())
  std::string _iniFile;   //!< file where ImGui keeps window positions (kept alive for ImGui)
  String _hoverText;   //!< data coordinates under the mouse (shown in the status bar on the next frame)
  FileBrowser _browser;   //!< file browser
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
  //! @brief Create a canvas pane (not yet docked). @return the pane
  CanvasPane & newPane();
  //! @brief Load the interface font and the fonts of the plots (see ImGuiPainter::setFont()).
  void loadFonts();
  };

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__CapGui
