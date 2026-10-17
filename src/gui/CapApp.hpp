#ifndef CAP__GUI__CapApp
#define CAP__GUI__CapApp
#include <deque>
#include <memory>
#include <vector>
#include "String.hpp"
#include "Action.hpp"
#include "CapGui.hpp"
#include "Graph.hpp"

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
  //!     <Graphs columns="1" rows="1" legend="true" logX="false" logY="false" mode="add"/>  <!-- per new canvas -->
  //!     <Files directory="" extensions=".cap .root .xml"/>
  //!     <Browser visible="true" docked="true"/>                   <!-- docked: in the main window -->
  //!     <Demo enabled="true"/>
  //!   </CapGui>
  //! @endcode
  //! Missing elements or attributes keep their default values.
  //!
  struct AppConfiguration
  {
  GuiSettings gui;
  int    graphColumns = 1;
  int    graphRows    = 1;
  bool   legend       = true;
  bool   logX         = false;
  bool   logY         = false;
  String mode         = "add";
  String directory;
  std::vector<String> extensions{".cap"};   //!< types listed by the Open dialog (only those CapGui can open are used)
  bool   demo         = true;

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
  //! Without --config, the configuration is $HOME/.CapGui.xml if it exists, otherwise <CAP>/config/CapGui.xml.
  //!
  class CapApp : public ActionListener
  {
  public:

  CapApp(int argc, char ** argv);
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
  //! @brief Take ownership of data points and show them in the current graph. @return the item shown
  GRAPHICS::GraphItem & show(MATH::DataPoints * data);
  //! @}

  //! @name Files
  //! @{
  //! @brief Open a CAP file and show its contents in the browser (if already open, reveal it).
  //! @param fileName file @return id of the open file, or -1 if it cannot be read
  int  openFile(const String & fileName);
  //! @brief Show a folder of the file system in the browser. @param folder path
  void openFolder(const String & folder);
  //! @brief Close an open file (its objects stay in the graphs that show them). @param id file id
  void closeFile(int id);
  //! @brief Read an open file again; the graphs showing its objects show the new versions. @param id file id
  void reloadFile(int id);
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
  //! @brief Show the objects of a folder, one per graph of the current canvas (divided as needed).
  //! @param file file id @param folderPath folder in the file ("" for the top)
  void showFolder(int file, const String & folderPath);
  void save(const String & fileName);
  //! @}

  //! @name Undo
  //! @{
  //! @brief Record the state of the canvases before a change.
  void recordUndo();
  void undo();
  void redo();
  //! @}

  AppConfiguration & configuration() { return _configuration; }
  CapGui & gui() { return _gui; }

  protected:

  //! State of all the canvases, for undo/redo.
  struct Snapshot
  {
  int columns, rows, current;
  std::vector<GRAPHICS::Canvas> canvases;
  std::vector<int> selected;
  };

  std::vector<String> _arguments;
  String _configurationFile;
  AppConfiguration _configuration;
  CapGui _gui;
  std::vector<std::unique_ptr<MATH::Function>>   _functions;
  std::vector<std::unique_ptr<MATH::DataPoints>> _data;
  std::vector<BrowserFile> _files;        //!< open files, shown in the browser
  std::vector<BrowserFile> _closedFiles;  //!< closed or replaced files: kept, since graphs and undo states may show their objects
  int _nextFileId;
  String _saveFile;
  std::deque<Snapshot> _undo;
  std::deque<Snapshot> _redo;
  std::vector<GRAPHICS::GraphItem> _clipboard;
  bool _showPreferences;
  bool _showAbout;
  bool _showShortcuts;
  bool _showImGuiDemo;
  int  _customColumns, _customRows;
  bool _customLayoutRequested, _customGraphsRequested;

  String findConfigurationFile() const;
  void buildMenus();
  void setupPane(CanvasPane & pane);
  void createDemo();
  void setLayout(int columns, int rows);
  void setGraphs(int columns, int rows);
  Snapshot snapshot() const;
  void restore(const Snapshot & state);
  void updateMenuStates();
  void renderWindows();
  void renderPreferences();
  void renderAbout();
  void renderShortcuts();
  void renderCustomLayout();
  BrowserFile * file(int id);
  //! Style of the k-th item of a graph (colors and markers cycle).
  static void styleItem(GRAPHICS::GraphItem & item, int k);
  };

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__CapApp
