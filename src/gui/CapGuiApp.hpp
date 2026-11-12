/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file CapGuiApp.hpp
//! @brief CapGuiApp: the application of CapGui, where every session is a plot file (also PlotEditor and the windows
//! of PlotHistos).
#ifndef CAP__GUI__CapGuiApp
#define CAP__GUI__CapGuiApp
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <vector>
#include "CapApp.hpp"
#include "PlotDocument.hpp"
#include "PlotSpec.hpp"
#include "PlotTemplate.hpp"

namespace CAP
{
  namespace GUI
  {

  //!
  //! @brief The application of CapGui: the file browser, the plot tree, and the canvases, which are the windows of a
  //! plot file (the XML files of PlotHistos, see GRAPHICS::PlotSpec and GRAPHICS::PlotDocument).
  //!
  //! Every session is a plot file: an untitled one at first, or those opened (one tab each in the Plot Tree). The
  //! canvases on screen are the windows of the plot file shown: the window holding the selection (or, with View > All
  //! Windows, the first window in the main window and each other window in a window of its own). What is done on
  //! the canvases (a histogram dropped, a color, a log axis, a legend moved, a grid of graphs) is written to the
  //! file; what is done in the tree draws the canvases again. Shapes drawn on the canvases and fits stay on screen
  //! only.
  //!
  //! The selection is shared: a graph clicked on a canvas is the Graph of the tree selected, and a row of the tree
  //! selected makes its canvas current. Edit > Copy, Cut, Paste, Duplicate and Delete act on it (Delete removes a
  //! shape selected on a canvas first); Edit > Undo and Redo undo the changes of the plot file, made on the canvases
  //! or in the tree.
  //!
  //! Modes: BASIC starts with a new plot file and the current folder in the browser; EDIT starts with the plot files
  //! given (or asks for one). Both have the same tools.
  //!
  class CapGuiApp : public CapApp
  {
  public:
  //! Starting point
  enum Mode { kBasic, kEdit };
  //! Catalog of the histograms of sources, for a new plot file (see catalogPlots()): given by the programs that link
  //! the analysis libraries (workflows are configured to know their histograms)
  using Catalog = std::function<bool(const std::vector<String> & sources, const String & dataDirectory,
                                     GRAPHICS::PlotTemplate & catalog, String & problems)>;

  //! @brief Constructor. @param mode mode @param plotFiles plot files to open @param argc arguments of CapGui
  //! (--config, CAP files and folders for the browser) @param argv arguments
  CapGuiApp(Mode mode, const std::vector<String> & plotFiles, int argc, char ** argv);
  //! @brief Destructor.
  virtual ~CapGuiApp();
  //! @brief Handle the commands of the menus. @param event action
  void actionPerformed(const ActionEvent & event) override;

  //! @name Settings of the programs (before run())
  //! @{
  //! @brief The catalog used by File > New from Data. @param catalog function
  void setCatalog(const Catalog & catalog) { _catalog = catalog; }
  //! @brief Show every window of the plot file (View > All Windows). @param v flag
  void setAllWindows(bool v) { _allWindows = v; }
  //! @brief Files replacing the inputs of the first plot file opened, in order (PlotHistos plots.xml data.cap).
  //! @param files files
  void setInputFiles(const std::vector<String> & files) { _inputFiles = files; }
  //! @brief Open the Plot Template window with a catalog at startup (PlotHistos --template --choose).
  //! @param catalog catalog @param options layout @param fileName plot file written by Write and Open
  void startTemplate(const GRAPHICS::PlotTemplate & catalog, const GRAPHICS::PlotTemplate::Options & options,
                     const String & fileName);
  //! @}

  //! @name For programs and tests
  //! @{
  int nDocuments() const { return int(_documents.size()); }   //!< Number of open plot files
  //! @brief Plot file k. @param k index @return document
  GRAPHICS::PlotDocument & document(int k) { return *_documents.at(std::size_t(k)).document; }
  int current() const { return _current; }   //!< Index of the plot file shown (-1: none)
  //! @brief Select an element of the plot file shown. @param path path
  void select(const GRAPHICS::PlotDocument::Path & path);
  //! @brief Open a plot file in a new tab. @param fileName file @return false if it cannot be read
  bool openDocument(const String & fileName);
  //! @brief Add data where dropped (as a drag from the browser does). @param target element @param object reference
  //! @param folder whether the reference is a folder @return false if nothing was added
  bool drop(const GRAPHICS::PlotDocument::Path & target, const ObjectReference & object, bool folder);
  //! @}

  protected:
  //! An open plot file
  struct Tab
  {
  std::unique_ptr<GRAPHICS::PlotDocument> document;   //!< the file
  GRAPHICS::PlotDocument::Path selected;   //!< selected element
  bool hasSelection = false;   //!< an element is selected
  int id = 0;   //!< identifier (for the ImGui tabs)
  };
  //! A canvas pane showing a canvas of the plot file
  struct PaneLink
  {
  CanvasPane * pane = nullptr;   //!< the pane
  GRAPHICS::PlotDocument::Path canvas;   //!< its Canvas element
  std::map<int, GRAPHICS::PlotDocument::Path> graphs;   //!< the Graph element of each cell
  XmlElement drawn;   //!< the canvas as drawn (or as last recorded), every setting
  std::string drawnText;   //!< its text
  int settle = 2;   //!< frames before the canvas as drawn is taken (the first frames complete it)
  };
  Mode _mode;   //!< starting point
  std::vector<String> _plotFiles;   //!< plot files of the command line
  std::vector<String> _inputFiles;   //!< files replacing the inputs of the first plot file
  Catalog _catalog;   //!< catalog of File > New from Data
  std::vector<Tab> _documents;   //!< open plot files
  int _current = -1;   //!< tab shown
  int _selectTab = -1;   //!< tab to bring to the front at the next frame
  int _nextId = 1;   //!< identifier of the next tab
  int _untitled = 0;   //!< number of the last new file
  std::string _clipboard;   //!< XML of the element copied
  bool _showTree = true;   //!< the Plot Tree is shown
  bool _showStyle = false;   //!< the elements of style are shown in the tree
  bool _showInspector = true;   //!< the inspector is shown below the tree
  bool _allWindows = false;   //!< every window of the plot file shown
  bool _showHelp = false;   //!< the help window is shown
  bool _treeFocused = false;   //!< the Plot Tree has the keyboard focus
  bool _grouping = false;   //!< changes made while a widget is active are one step of undo
  int  _groupTab = -1;   //!< tab of the grouped changes
  //! Closing: tabs to close once their changes are saved or discarded; then exit if asked
  std::vector<int> _closing;
  bool _exitAfterClosing = false;   //!< quit once the tabs are closed
  bool _closeDialog = false;   //!< the Save Changes dialog opens at the next frame
  //! Canvases: what is drawn
  std::unique_ptr<GRAPHICS::PlotSpec> _spec;   //!< specification of the canvases drawn (owns their functions)
  std::vector<PaneLink> _links;   //!< the panes showing canvases of the plot file
  int _drawnTab = -1;   //!< tab drawn
  unsigned long _drawnVersion = ~0ul;   //!< version of the document drawn (or recorded from the canvases)
  int _drawnWindow = -1;   //!< window drawn in the main window
  bool _drawnAll = false;   //!< all the windows drawn
  std::set<std::string> _missing;   //!< input files not found (reported once)
  long _frame = 0;   //!< frames drawn
  //! Shared selection: last seen on the canvases and in the tree
  CanvasPane * _seenPane = nullptr;
  int _seenGraph = -1;
  GRAPHICS::PlotDocument::Path _seenSelection;
  bool _seenHasSelection = false;
  //! Custom grids (Window menu)
  bool _gridDialog = false;   //!< the grid dialog opens at the next frame
  bool _gridOfGraphs = false;   //!< it sets the graphs of the canvas (otherwise the canvases of the window)
  int _gridColumns = 2, _gridRows = 2;   //!< grid chosen
  //! Plot Template window (File > New from Data)
  bool _templateOpen = false;
  GRAPHICS::PlotTemplate _template;
  GRAPHICS::PlotTemplate::Options _templateOptions;
  std::string _templateFile;   //!< file written by Write and Open ("": a new untitled plot file)
  std::string _templateFilter;   //!< filter of the names shown

  //! @brief The plot file shown. @return tab, or null
  Tab * tab() { return _current >= 0 && _current < int(_documents.size()) ? &_documents[std::size_t(_current)] : nullptr; }
  //! @brief Menus: File, Edit, View, Window, Help.
  void buildMenus() override;
  //! @brief Enable and check the items of the menus.
  void updateMenuStates() override;
  //! @brief Open the plot files of the command line, or a new one; dock the tree; connect the browsers.
  void onStartup() override;
  //! @brief Clicks and folders of a browser add to the plot file. @param browser browser
  void connectToPlot(FileBrowser & browser);
  //! @brief A new plot file in a new tab. @return its index
  int newDocument();
  //! @brief Ask to close a tab (asks to save its changes first). @param k index
  void requestClose(int k);
  //! @brief Close a tab without asking (a new untitled plot file if it was the last). @param k index
  void closeTab(int k);
  //! @brief Save a tab (asks for a name if it has none). @param k index @param ask always ask for a name
  //! @param after called once saved
  void save(int k, bool ask, const std::function<void()> & after = std::function<void()>());
  //! @brief Save the changed plot files one after the other (a file is asked for the untitled ones; Cancel stops).
  //! @param after called once they are all saved
  void saveChangedDocuments(const std::function<void()> & after = std::function<void()>());
  //! @brief Window of the plot file shown on the main window (otherwise that of the selection).
  //! @param window set to its path @return false if the plot file has no window
  bool currentWindow(GRAPHICS::PlotDocument::Path & window);
  //! @brief File > Duplicate: the entry selected in the browser if it was used last, otherwise the selection of the
  //! plot file (canvas, graph, histogram, function, window), otherwise the plot file.
  void duplicateCurrent();
  //! @brief Whether File > Save, Duplicate and Close act on the browser (used last, with an entry selected).
  //! @return true if so
  bool browserIsCurrent();
  //! @brief Draw the tree and the other windows; keep the canvases and the file in step (every frame).
  void renderFrame();
  //! @brief Draw the tree of a plot file. @param t tab
  void renderTree(Tab & t);
  //! @brief Draw an element of the tree and its children. @param t tab @param path path
  void renderNode(Tab & t, const GRAPHICS::PlotDocument::Path & path);
  //! @brief The pull-downs of a row of the tree. @param t tab @param path element
  void rowControls(Tab & t, const GRAPHICS::PlotDocument::Path & path);
  //! @brief The right-click menu of a row. @param t tab @param path element
  void rowMenu(Tab & t, const GRAPHICS::PlotDocument::Path & path);
  //! @brief Drag and drop on a row: elements of the tree moved, histograms and folders of the browser added.
  //! @param t tab @param path element
  void rowDragAndDrop(Tab & t, const GRAPHICS::PlotDocument::Path & path);
  //! @brief Draw the settings of the selected element. @param t tab
  void renderInspector(Tab & t);
  //! @brief The settings of an element (inspector and ... pull-down). @param t tab @param path element
  void editElement(Tab & t, const GRAPHICS::PlotDocument::Path & path);
  //! @brief Draw the dialogs: Save Changes, help, grid, Plot Template.
  void renderDialogs();
  //! @brief Draw the Plot Template window, if open.
  void renderTemplate();
  //! @brief A new plot file from the template (written to _templateFile if set). @return false if nothing selected
  bool createFromTemplate();
  //! @brief Draw the canvases again if the plot file, the window shown or the tab changed.
  void layoutCanvases();
  //! @brief Write to the plot file what was changed on the canvases.
  void recordCanvases();
  //! @brief Keep the selections of the canvases and of the tree in step.
  void syncSelection();
  //! @brief Whether a pane still exists. @param pane pane @return its index, or -1
  int paneIndexOf(const CanvasPane * pane);
  //! @brief Close panes (their x, Window > Close Canvas): the canvases of the plot file they show are removed from it
  //! (one step of undo). @param panes panes, in decreasing order @return the panes showing no canvas of the plot file
  std::vector<int> closeCanvases(const std::vector<int> & panes);
  //! @brief Where data shown from the browser go: the selected graph, or else its canvas, or else the top. @return path
  GRAPHICS::PlotDocument::Path dropTarget();
  //! @brief The canvas of the plot file shown by the current pane and the Graph element of its selected graph.
  //! @param canvas set to the canvas @param graph set to the graph (empty if the cell has none) @return false if the
  //! current pane shows no canvas of the plot file
  bool currentCanvas(GRAPHICS::PlotDocument::Path & canvas, GRAPHICS::PlotDocument::Path & graph);
  //! @brief Show an object of the browser (a click on it): in the graph selected on the current canvas, added to its
  //! contents or replacing them (Add / Replace of the canvas toolbar). @param object object
  void showObject(const ObjectReference & object);
  //! @brief Show the objects of a folder of the browser: a graph each, in the canvas of the selection or of the
  //! current pane (replacing its graphs in Replace mode), or a new canvas. @param object folder
  void showFolder(const ObjectReference & object);
  //! @brief Drop an object on a graph of a pane (added, or replacing the contents in Replace mode).
  //! @param target element dropped on @param object object @param pane pane @param cell its graph
  void dropOnPane(const GRAPHICS::PlotDocument::Path & target, const ObjectReference & object, CanvasPane & pane, int cell);
  //! @brief Remove the histograms and functions of a Graph element (and the shapes of the cell drawn, unless the pane
  //! keeps them). @param graph graph @param pane pane drawing it (or nullptr) @param cell its graph in the pane
  void clearGraph(const GRAPHICS::PlotDocument::Path & graph, CanvasPane * pane, int cell);
  //! @brief Replace the contents of a Graph element by an object: cleared, the object added, and the title of the graph
  //! following when it was the name of what it showed. @param graph graph @param object object @param pane pane
  //! drawing it (or nullptr) @param cell its graph in the pane
  void replaceGraph(const GRAPHICS::PlotDocument::Path & graph, const ObjectReference & object, CanvasPane * pane, int cell);
  //! @brief Paths of the objects of a folder of an open file (or the object itself). @param object reference
  //! @param folder whether it is a folder @param file set to the path of the file @return object paths
  std::vector<String> objectsOf(const ObjectReference & object, bool folder, String & file);
  //! @brief The canvases of the plot file shown, made from the file, for the outputs (File > Export).
  //! @param spec specification read @param windows windows made @return false if the file cannot be read
  bool makeOutputs(GRAPHICS::PlotSpec & spec, std::vector<GRAPHICS::PlotSpec::WindowState> & windows);
  //! @brief Set the grid of the window shown or of the current canvas in the plot file. @param graphs graphs of the
  //! canvas (otherwise canvases of the window) @param columns columns @param rows rows
  void setGrid(bool graphs, int columns, int rows);
  };

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__CapGuiApp
