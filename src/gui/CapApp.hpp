/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file CapApp.hpp
//! @brief CapApp: the CAP graphical interface application (configuration, menus, files, undo).
#ifndef CAP__GUI__CapApp
#define CAP__GUI__CapApp
#include <deque>
#include <memory>
#include <vector>
#include "String.hpp"
#include "Action.hpp"
#include "CapGui.hpp"
#include "Graph.hpp"
#include "Folder.hpp"

namespace CAP
{
  namespace MATH
  {
  class Function;
  class DataPoints;
  }

  namespace GUI
  {

  //!
  //! @brief Configuration of the CAP graphical application, read from an XML file at startup.
  //!
  //! @code
  //!   <CapGui version="1">
  //!     <Window title="CAP" width="1400" height="900" theme="light" fontSize="15" viewports="true"/>
  //!     <Canvases columns="1" rows="1" toolbars="true"/>          <!-- n x m canvas panes -->
  //!     <Graphs columns="1" rows="1" legend="true" logX="false" logY="false" mode="add" keepShapes="true"/>  <!-- per new canvas -->
  //!     <Files directory="" extensions=".cap .root .xml"/>
  //!     <Browser visible="true" docked="true"/>                   <!-- docked: in the main window -->
  //!     <Demo enabled="false"/>                                <!-- true: demonstration objects at startup -->
  //!     <Latex enabled="true" bin="" preamble="\usepackage{amsmath,amssymb}" rootSyntax="true"/>  <!-- bin: "" to search -->
  //!   </CapGui>
  //! @endcode
  //! Missing elements or attributes keep their default values.
  //!
  struct AppConfiguration
  {
  GuiSettings gui;   //!< settings of the window
  int    graphColumns = 1;   //!< columns of graphs of a new canvas
  int    graphRows    = 1;   //!< rows of graphs of a new canvas
  bool   legend       = true;   //!< legend shown in new graphs
  bool   logX         = false;   //!< logarithmic x axis in new graphs
  bool   logY         = false;   //!< logarithmic y axis in new graphs
  String mode         = "add";   //!< what showing an object does: "add" or "replace"
  bool   keepShapes   = true;    //!< shapes stay in a graph when its contents are replaced
  String directory;   //!< folder where the file dialogs start (empty: the current folder)
  std::vector<String> extensions{".cap", ".root"};   // .root: only when CapGui is built with ROOT files   //!< types listed by the Open dialog (only those CapGui can open are used)
  bool   demo         = false;  //!< show the demonstration objects at startup (otherwise the canvases start empty)
  bool   latex        = true;   //!< typeset the texts with '$' or '\' by LaTeX (see GRAPHICS::LatexRenderer)
  String latexBin;              //!< directory of latex and dvisvgm (empty: looked for)
  String latexPreamble = "\\usepackage{amsmath,amssymb}";   //!< preamble of the LaTeX documents
  bool   rootSyntax   = true;   //!< translate the texts in ROOT syntax (#, _{}, ^{}) and typeset them by LaTeX

  //! @brief Read a configuration file (missing values keep their defaults). @param fileName file
  //! @param error set to the problem if false is returned @return false if the file cannot be read
  bool read(const String & fileName, String & error);
  //! @brief Write the configuration. @param fileName file @return false if it cannot be written
  bool write(const String & fileName) const;
  };

  //!
  //! @brief The CAP graphical application: main controller of the GUI.
  //!
  //! CapApp reads the configuration, opens the CapGui window, builds its menus and listens to them: every command
  //! ("file.open", "edit.undo", "window.layout.2x2", ...) goes through actionPerformed(). It owns the objects shown
  //! (functions, data points, and later the contents of the files opened) and keeps the undo/redo history.
  //!
  //! Command line: CapGui [--config file.xml] [file ...]
  //! Without --config, the configuration is $HOME/.CapGui.xml if it exists, otherwise \<CAP\>/config/CapGui.xml.
  //!
  class CapApp : public ActionListener
  {
  public:

  //! @brief Constructor. @param argc number of arguments @param argv arguments: --config file.xml (or -c), then files or folders to open
  CapApp(int argc, char ** argv);
  //! @brief Destructor.
  virtual ~CapApp();

  //! @brief Read the configuration, open the window, run until it is closed. @return exit code
  int run();

  //! @brief Handle a menu command. @param event the action
  virtual void actionPerformed(const ActionEvent & event);

  //! @name Objects
  //! @{

  //! @brief Take ownership of a function and show it in the current graph. @param function function of rank 1
  //! @param xMinimum lower end of the curve @param xMaximum upper end @return the item shown
  GRAPHICS::GraphItem & show(MATH::Function * function, float xMinimum, float xMaximum);
  //! @brief Take ownership of a function of rank 2 and show it as a color map in the current graph.
  //! @param function function of rank 2 @param xMinimum left @param xMaximum right @param yMinimum bottom
  //! @param yMaximum top @return the item shown
  GRAPHICS::GraphItem & show(MATH::Function * function, float xMinimum, float xMaximum, float yMinimum, float yMaximum);
  //! @brief Take ownership of data points and show them in the current graph. @return the item shown
  GRAPHICS::GraphItem & show(MATH::DataPoints * data);
  //! @}

  //! @name Files
  //! @{

  //! @brief Open a CAP file and show its contents in a browser (a file already open is shared, not read again).
  //! @param fileName file @param browser browser (default: the one used last) @return id of the open file, or -1
  //! if it cannot be read
  int  openFile(const String & fileName, FileBrowser * browser=nullptr);
  //! @brief Show a folder of the file system in the browser used last. @param folder path
  void openFolder(const String & folder);
  //! @brief Close an open file in a browser; it is closed when no browser shows it any more (its objects stay in the
  //! graphs that show them). @param id file id @param browser browser (default: all)
  void closeFile(int id, FileBrowser * browser=nullptr);
  //! @brief Read an open file again; the graphs showing its objects show the new versions. @param id file id
  void reloadFile(int id);
  //! @brief Connect a file browser to the files of the application. @param browser browser
  void connectBrowser(FileBrowser & browser);
  //! @brief Make a new, empty tree (editable) in a browser. @param browser browser @return its id
  int  newTree(FileBrowser & browser);
  //!
  //! @brief Copy an object or a folder into a folder of an editable tree (deep copy, under a name not used there).
  //! @param source object or folder (an empty path: the whole tree of its file) @param folder whether it is a folder
  //! @param target tree @param folderPath path of the folder of the tree ("" for its top) @return false on failure
  //!
  bool copyInto(const ObjectReference & source, bool folder, int target, const String & folderPath);
  //! @brief Add a folder to a folder of an editable tree. @param target tree @param folderPath folder @param name name
  void newFolder(int target, const String & folderPath, const String & name);
  //! @brief Rename an entry of an editable tree. @param target tree @param path entry @param name new name
  void renameEntry(int target, const String & path, const String & name);
  //! @brief Remove an entry of an editable tree (its objects are kept for the graphs showing them). @param target
  //! tree @param path entry
  void removeEntry(int target, const String & path);
  //! @brief Save an editable tree to a CAP file chosen by the user. @param target tree
  void saveTree(int target);
  //! @brief Ask for a ROOT file, then write the histograms of a folder of a file or tree to it (see exportRootFile()).
  //! @param target file or tree @param folderPath folder ("" for the top)
  void exportRoot(int target, const String & folderPath);
  //! @brief Ask for a file, then write a copy of a file or of one of its folders to a new CAP file (e.g. the histograms
  //! converted from a ROOT file). @param target file or tree @param folderPath folder ("" for the whole file)
  void saveCopy(int target, const String & folderPath);
  //! @brief Write a copy of a file or folder to a new CAP file. @param target file or tree @param folderPath folder
  //! @param fileName CAP file (".cap" added if missing) @return true on success
  bool saveCopyTo(int target, const String & folderPath, const String & fileName);
  //! @brief Save an editable tree to a CAP file (".cap" added if missing); the tree then takes its name.
  //! @param target tree @param fileName file @return false if it cannot be saved (an open file, a write error)
  bool saveTreeTo(int target, const String & fileName);
  //! @brief Show an object of an open file in the current graph (added or replacing, see CanvasPane::Mode).
  //! @param object reference @return false if it cannot be shown
  bool showObject(const ObjectReference & object);
  //! @brief Show an object in a given graph of a pane. @param pane pane @param graph graph index @param object reference
  //! @return false if it cannot be shown
  bool showObject(CanvasPane & pane, int graph, const ObjectReference & object);
  //! @brief Show the values of an object of an open file in a new data pane. @param object reference
  //! @return false if it cannot be shown
  bool showData(const ObjectReference & object);
  //! @brief Show the values of data points in a new data pane. @param data data points @param label name shown
  void showDataTable(const MATH::DataPoints * data, const String & label);
  //!
  //! @brief Show the map of the pulls of data points of rank 2 to one of their fits (MATH::residualMap() for a
  //! histogram, MATH::residuals() otherwise) in a new canvas pane, with the diverging palette (undoable).
  //! @param data data points of rank 2 @param fit index of a fit of rank 2
  //!
  void showResidualMap(const MATH::DataPoints * data, unsigned int fit);
  //!
  //! @brief Show the projections of a histogram of rank 2 on x and y, with its fits of rank 2 projected
  //! (MATH::projection()), side by side in a new canvas pane, with the panel of the residuals (undoable).
  //! @param data histogram of rank 2
  //!
  void showProjections(const MATH::DataPoints * data);
  //!
  //! @brief Compare two histograms of the same binning (e.g. from two files): a new pane with the two histograms
  //! overlaid and their difference (second - first; for maps, the two maps and their difference side by side), and
  //! the Comparison window with the chi-square compatibility test and, for histograms of rank 1, the
  //! Kolmogorov-Smirnov test. @param first first histogram @param second second histogram @param firstLabel label of
  //! the first @param secondLabel label of the second
  //!
  void showComparison(const MATH::DataPoints * first, const MATH::DataPoints * second, const String & firstLabel,
                      const String & secondLabel);
  //! @brief Show the objects of a folder, one per graph of the current canvas (divided as needed).
  //! @param file file id @param folderPath folder in the file ("" for the top)
  void showFolder(int file, const String & folderPath);
  //! @brief Write the canvases laid out as on screen, one canvas or one graph to an SVG or PDF file (by the extension of
  //! the name; the format used last if it has none). @param fileName file name @param pane canvas pane (-1: the window)
  //! @param graph graph of the pane (-1: the whole canvas)
  void save(const String & fileName, int pane=-1, int graph=-1);
  //! @brief Ask for a file, then save the window, a canvas or a graph (see save()). @param pane canvas pane (-1: the
  //! window) @param graph graph of the pane (-1: the whole canvas)
  void saveAs(int pane=-1, int graph=-1);
  //! @brief Index of a canvas pane. @param pane pane @return index, -1 if not shown
  int paneIndex(const CanvasPane & pane);
  //! @}

  //! @name Undo
  //! @{

  //! @brief Record the state of the canvases before a change.
  void recordUndo();
  //! @brief Go back to the state before the last change (canvases and their graphs).
  void undo();
  //! @brief Redo the change undone last.
  void redo();
  //! @}

  AppConfiguration & configuration() { return _configuration; }   //!< Configuration (read from the XML file, changed by Preferences)
  CapGui & gui() { return _gui; }   //!< The window

  protected:

  //! State of all the canvases, for undo/redo.
  struct Snapshot
  {
  int columns;   //!< grid layout and index of the current pane
  int rows;   //!< grid layout and index of the current pane
  int current;   //!< grid layout and index of the current pane
  std::vector<GRAPHICS::Canvas> canvases;   //!< copies of the canvases of the panes
  std::vector<int> selected;   //!< selected graph of each pane
  };

  std::vector<String> _arguments;   //!< files and folders given on the command line
  String _configurationFile;   //!< configuration file read (or given with --config)
  AppConfiguration _configuration;   //!< configuration
  CapGui _gui;   //!< the window
  std::vector<std::unique_ptr<MATH::Function>>   _functions;   //!< functions created by the application (demo)
  std::vector<std::unique_ptr<MATH::DataPoints>> _data;   //!< data points created by the application (demo)
  std::vector<BrowserFile> _files;        //!< open files, shown in the browser
  std::vector<BrowserFile> _closedFiles;  //!< closed or replaced files: kept, since graphs and undo states may show their objects
  int _nextFileId;   //!< identifier of the next file opened
  int _treeCounter;   //!< number of the last tree made (New Tree)
  std::vector<std::unique_ptr<FolderItem>> _removed;   //!< entries removed from trees: kept for the graphs showing them
  String _saveFile;   //!< file of the last Save Window As
  String _saveFormat;   //!< extension of the file saved last (".svg" or ".pdf")
  int  _renamePane;   //!< canvas pane renamed in the Rename Canvas dialog (-1: none)
  std::string _renameText;   //!< title edited in the Rename Canvas dialog
  bool _renameRequested;   //!< Rename Canvas dialog to open
  std::deque<Snapshot> _undo;   //!< states before the last changes (at most 50)
  std::deque<Snapshot> _redo;   //!< states undone
  std::vector<GRAPHICS::GraphItem> _clipboard;   //!< items copied (Edit > Copy)
  //! Comparison shown in the Comparison window (see showComparison())
  struct Comparison
  {
  const MATH::Histogram * first = nullptr;    //!< first histogram (not owned)
  const MATH::Histogram * second = nullptr;   //!< second histogram (not owned)
  String firstLabel;                          //!< its label
  String secondLabel;                         //!< its label
  int    mode = 0;                            //!< chi-square test: 0 counts, 1 values, 2 shapes
  String chiSquare;                           //!< result of the chi-square test
  String kolmogorov;                          //!< result of the Kolmogorov-Smirnov test (rank 1)
  bool   open = false;                        //!< window shown
  };
  Comparison _comparison;   //!< last comparison
  //! @brief Compute the tests of the comparison.
  void computeComparison();
  //! @brief Draw the Comparison window, if shown.
  void renderComparison();
  bool _showPreferences;   //!< Preferences window shown
  bool _showAbout;   //!< About window shown
  bool _showShortcuts;   //!< Keyboard Shortcuts window shown
  bool _showImGuiDemo;   //!< Dear ImGui demonstration window shown
  int  _customColumns;   //!< values edited in the Custom layout dialog
  int  _customRows;   //!< values edited in the Custom layout dialog
  bool _customLayoutRequested;   //!< Custom layout dialog to open, for the canvases or the graphs
  bool _customGraphsRequested;   //!< Custom layout dialog to open, for the canvases or the graphs

  //! @brief First configuration file found: $HOME/.CapGui.xml, $CAP_ROOT_PATH/config/CapGui.xml, the one of the sources. @return path, or ""
  String findConfigurationFile() const;
  //! @brief Create the menus of the menu bar.
  void buildMenus();
  //! @brief Prepare a new canvas pane: graphs, mode, callbacks (undo, drop, show data). @param pane pane
  void setupPane(CanvasPane & pane);
  //! @brief Show the demonstration objects (a Gaussian with pseudo-data, an exponential).
  void createDemo();
  //! @brief Give the LaTeX settings of the configuration to GRAPHICS::LatexRenderer. @return a problem to report, or ""
  String applyLatex();
  //! @brief Arrange the canvases in columns x rows panes (undoable). @param columns columns @param rows rows
  void setLayout(int columns, int rows);
  //! @brief Divide the current canvas into columns x rows graphs (undoable). @param columns columns @param rows rows
  void setGraphs(int columns, int rows);
  //! @brief State of all the canvases. @return state
  Snapshot snapshot() const;
  //! @brief Go back to a state of the canvases. @param state state
  void restore(const Snapshot & state);
  //! @brief Enable, disable and check the menu items according to the state.
  void updateMenuStates();
  //! @brief Draw the application windows (preferences, about, shortcuts, dialogs) every frame.
  void renderWindows();
  //! @brief Draw the Preferences window, if shown.
  void renderPreferences();
  //! @brief Draw the About window, if shown.
  void renderAbout();
  //! @brief Draw the Keyboard Shortcuts window (keys of the menus, mouse actions), if shown.
  void renderShortcuts();
  //! @brief Draw the Custom layout dialog, if open.
  void renderCustomLayout();
  //! @brief Rename Canvas dialog.
  void renderRenameCanvas();
  //! @brief Open file of an identifier. @param id identifier @return file, or null
  BrowserFile * file(int id);
  };

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__CapApp
