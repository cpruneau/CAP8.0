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
  String title      = "CAP";
  int    width      = 1400;
  int    height     = 900;
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

  CapGui();
  ~CapGui();

  //! @brief Open the window. @param settings window settings @return false if the window could not be created
  bool initialize(const GuiSettings & settings);
  //! @brief Run the event loop until the window is closed or requestExit() is called.
  void run();
  //! @brief Close the window at the end of the current frame (onCloseRequest may refuse).
  void requestExit();

  MenuBar & menuBar() { return _menuBar; }
  FileDialog & fileDialog() { return _fileDialog; }

  //! @name File browser
  //! @{
  FileBrowser & browser() { return _browser; }
  bool browserVisible() const { return _settings.browserVisible; }
  void setBrowserVisible(bool visible) { _settings.browserVisible = visible; }
  bool browserDocked() const { return _settings.browserDocked; }
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
  int  nColumns() const { return _columns; }
  int  nRows()    const { return _rows; }
  int  nPanes()   const { return (int)_panes.size(); }
  CanvasPane & pane(int k) { return *_panes.at(k); }
  CanvasPane & currentPane() { return *_panes.at(_currentPane); }
  int  currentPaneIndex() const { return _currentPane; }
  void setCurrentPane(int k) { if (k >= 0 && k < nPanes()) { _currentPane = k; _focusRequested = true; } }
  void setShowToolbars(bool v);
  bool showToolbars() const { return _settings.toolbars; }
  //! @brief Add a data pane (a table of data points), docked beside the current canvas pane. @return the new pane
  DataPane & addDataPane();
  int  nDataPanes() const { return (int)_dataPanes.size(); }
  DataPane & dataPane(int k) { return *_dataPanes.at(k); }
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
  void setTheme(const String & theme);
  const GuiSettings & settings() const { return _settings; }
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

  GLFWwindow * _window;
  GuiSettings  _settings;
  ImFont *     _font;
  MenuBar      _menuBar;
  FileDialog   _fileDialog;
  std::vector<std::unique_ptr<CanvasPane>> _panes;
  int    _columns, _rows, _currentPane;
  int    _paneCounter;
  String _status;
  double _statusTime;
  String _messageTitle, _messageText;
  bool   _messageRequested;
  bool   _exitRequested;
  float  _areaX, _areaY, _areaWidth, _areaHeight;
  std::string _iniFile;
  String _hoverText;   //!< data coordinates under the mouse (shown in the status bar on the next frame)
  FileBrowser _browser;
  bool   _firstFrame;
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

  void renderFrame();
  void renderStatusBar(const MenuItem * hovered, const String & hoverText);
  void renderCanvases(String & hoverText);
  void renderMessage();
  void renderBrowser();
  void buildDockLayout(unsigned int dockId);
  void buildPaneGrid(unsigned int nodeId);
  CanvasPane & newPane();
  void loadFonts();
  };

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__CapGui
