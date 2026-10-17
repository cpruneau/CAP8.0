#ifndef CAP__GUI__FileBrowser
#define CAP__GUI__FileBrowser
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include "String.hpp"

namespace CAP
{
  class Folder;
  class CapFile;

  namespace GUI
  {

  //!
  //! @brief Reference to an object of an open CAP file: what the browser passes to the application when an object
  //! is clicked, and what a drag from the browser carries to a canvas.
  //!
  struct ObjectReference
  {
  int    file = -1;    //!< id of the open file (BrowserFile::id)
  String path;         //!< path of the object in the file, e.g. "singles/positive/pt"
  };

  //!
  //! @brief A CAP file opened by the application and shown in the browser.
  //!
  struct BrowserFile
  {
  int    id = -1;                       //!< unique id (not reused)
  String path;                          //!< path of the file on disk
  std::shared_ptr<CapFile> file;        //!< contents (the folder tree loaded from the file)
  };

  //!
  //! @brief File browser of CapGui: folders of the file system and the contents of the CAP files.
  //!
  //! Two sections:
  //! - Files: the CAP files opened with File > Open (or from the command line) that are not inside a folder below.
  //! - Folders: the folders opened with File > Open Folder, as trees of sub-folders and CAP files. Expanding a CAP
  //!   file opens it (onOpenFile) and shows its contents in place.
  //!
  //! The contents of a CAP file are its Folder tree: folders expand when clicked; clicking an object shows it in the
  //! selected graph (onShowObject); an object can also be dragged onto a graph (payload kPayloadType, decoded by
  //! decodePayload()). Right-click menus: on a file (Reload, Close), on a folder (Show All), on an object (Show).
  //! The filter field keeps the objects whose name contains the text (ignoring case), and the folders holding them.
  //!
  //! The browser does not own the files: the application does (setFiles()), and decides what opening, closing and
  //! showing mean. The browser only draws its contents in the current ImGui window (render()); CapGui owns the window,
  //! docked in the main window or standalone.
  //!
  class FileBrowser
  {
  public:

  //! Name of the browser window.
  static const char * windowName() { return "File Browser"; }
  //! Type of the drag-and-drop payload of an object.
  static const char * payloadType() { return "CAP_OBJECT"; }
  //! @brief Decode a drag-and-drop payload. @param data payload data @param size payload size
  //! @param reference set to the object dragged @return false if the payload is not an object reference
  static bool decodePayload(const void * data, int size, ObjectReference & reference);

  FileBrowser();

  //! @brief The files to show (owned by the application; the vector must outlive the browser). @param files files
  void setFiles(const std::vector<BrowserFile> * files) { _files = files; }
  //! @brief Add a folder of the file system (shown in the Folders section). @param folder path
  void addFolder(const String & folder);
  //! @brief Remove a folder from the Folders section. @param folder path
  void removeFolder(const String & folder);
  const std::vector<String> & folders() const { return _folders; }
  //! @brief Expand (and scroll to) a file at the next render. @param path path of the file
  void reveal(const String & path);
  //! @brief Forget the cached listings of the folders (they are read again when shown).
  void refresh() { _listings.clear(); }

  //! @name What the browser asks the application to do
  //! @{
  std::function<int(const String & path)>                      onOpenFile;    //!< open a CAP file: id, or -1
  std::function<void(const ObjectReference & object)>          onShowObject;  //!< show an object
  std::function<void(const ObjectReference & object)>          onShowData;    //!< show the values of an object as a table
  std::function<void(int file, const String & folderPath)>     onShowFolder;  //!< show all the objects of a folder
  std::function<void(int file)>                                onCloseFile;   //!< close a file
  std::function<void(int file)>                                onReloadFile;  //!< read a file again
  std::function<void(const String & command)>                  onCommand;     //!< a menu command ("file.open", ...)
  //! @}

  //! @brief Draw the browser in the current window.
  void render();

  //! @brief Short type name of an entry of a folder and whether it can be shown in a 2D graph.
  //! @param folder folder @param name entry name @param kind set to e.g. "H1", "H2", "Profile", "Points"
  //! @param details set to a description (title, bins, entries) @return true if it can be shown
  static bool describe(const Folder & folder, const String & name, String & kind, String & details);

  protected:

  struct Listing
  {
  std::vector<std::string> directories;
  std::vector<std::string> files;
  std::string error;
  };

  const std::vector<BrowserFile> * _files;
  std::vector<String> _folders;
  std::map<std::string, Listing> _listings;
  std::set<std::string> _openFailed;     //!< files whose opening failed (not retried until refresh)
  std::string _reveal;
  std::string _filter;
  ObjectReference _selected;

  const BrowserFile * fileByPath(const std::string & path) const;
  const BrowserFile * fileById(int id) const;
  bool insideFolders(const std::string & path) const;
  const Listing & listing(const std::string & directory);

  void renderToolbar();
  void renderFile(const BrowserFile & file, const String & label);
  void renderDirectory(const std::string & directory, const std::string & label, bool isRoot);
  void renderCapFileInTree(const std::string & path, const std::string & label);
  void renderFolder(int fileId, const Folder & folder, const std::string & prefix);
  void renderObject(int fileId, const Folder & folder, const String & name, const std::string & path);
  bool matches(const String & name) const;
  bool folderMatches(const Folder & folder) const;
  };

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__FileBrowser
