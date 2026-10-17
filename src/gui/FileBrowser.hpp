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
//! @file FileBrowser.hpp
//! @brief FileBrowser: browser of the CAP files and folders of the CapGui window.
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

  //! @brief Constructor: no files and no folders.
  FileBrowser();

  //! @brief The files to show (owned by the application; the vector must outlive the browser). @param files files
  void setFiles(const std::vector<BrowserFile> * files) { _files = files; }
  //! @brief Add a folder of the file system (shown in the Folders section). @param folder path
  void addFolder(const String & folder);
  //! @brief Remove a folder from the Folders section. @param folder path
  void removeFolder(const String & folder);
  const std::vector<String> & folders() const { return _folders; }   //!< Folders of the file system shown (Folders section)
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

  //! Contents of a folder of the file system, as shown: sub-folders and CAP files, or an error
  struct Listing
  {
  std::vector<std::string> directories;   //!< names of the sub-folders
  std::vector<std::string> files;   //!< names of the CAP files
  std::string error;   //!< why the folder cannot be listed, or empty
  };

  const std::vector<BrowserFile> * _files;   //!< open files (owned by the application)
  std::vector<String> _folders;   //!< folders of the file system shown
  std::map<std::string, Listing> _listings;   //!< listings of the folders, read once (see refresh())
  std::set<std::string> _openFailed;     //!< files whose opening failed (not retried until refresh)
  std::string _reveal;   //!< file to expand and scroll to at the next render
  std::string _filter;   //!< text typed in the filter field (case-insensitive part of the object names)
  ObjectReference _selected;   //!< object selected (highlighted)

  //! @brief Open file of a path. @param path path @return file, or null if not open
  const BrowserFile * fileByPath(const std::string & path) const;
  //! @brief Open file of an identifier. @param id identifier @return file, or null
  const BrowserFile * fileById(int id) const;
  //! @brief Whether a path is below one of the folders shown. @param path path @return true if so
  bool insideFolders(const std::string & path) const;
  //! @brief Contents of a folder (read once, see refresh()). @param directory folder @return listing
  const Listing & listing(const std::string & directory);

  //! @brief Draw the toolbar: filter, Open, Folder, Refresh.
  void renderToolbar();
  //! @brief Draw an open file and, if expanded, its tree. @param file file @param label text shown
  void renderFile(const BrowserFile & file, const String & label);
  //! @brief Draw a folder of the file system and, if expanded, its contents. @param directory path @param label text shown @param isRoot whether it is one of the folders added
  void renderDirectory(const std::string & directory, const std::string & label, bool isRoot);
  //! @brief Draw a CAP file of a folder: opened when first expanded. @param path path @param label text shown
  void renderCapFileInTree(const std::string & path, const std::string & label);
  //! @brief Draw the entries of a folder of objects. @param fileId file @param folder folder @param prefix path of the folder in the file
  void renderFolder(int fileId, const Folder & folder, const std::string & prefix);
  //! @brief Draw one object: click to show, drag onto a graph, right-click for its menu. @param fileId file @param folder folder holding it @param name name @param path path in the file
  void renderObject(int fileId, const Folder & folder, const String & name, const std::string & path);
  //! @brief Whether a name passes the filter. @param name name @return true if so
  bool matches(const String & name) const;
  //! @brief Whether a folder holds an object passing the filter (at any depth). @param folder folder @return true if so
  bool folderMatches(const Folder & folder) const;
  };

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__FileBrowser
