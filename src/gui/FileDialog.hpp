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
//! @file FileDialog.hpp
//! @brief FileDialog: dialog to choose a file to open, a folder, or a file to save to.
#ifndef CAP__GUI__FileDialog
#define CAP__GUI__FileDialog
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include "String.hpp"

namespace CAP
{
  namespace GUI
  {

  //!
  //! @brief Modal dialog to choose a file to open, a folder, or a file to save to, drawn with ImGui (no native or
  //! ROOT dialog needed).
  //!
  //! open() shows the dialog; render() must be called every frame; when the user confirms, the callback receives
  //! the chosen path. Double-click a folder to enter it, a file to choose it; the path field accepts a typed path.
  //!
  class FileDialog
  {
  public:

  enum Mode { kOpenFile, kOpenFolder, kSaveFile };   //!< What the dialog chooses: a file to open, a folder, or a file to save to
  using Callback = std::function<void(const String & path)>;   //!< Function receiving the chosen path

  //! @brief Constructor: a closed dialog starting in the current folder.
  FileDialog();

  //! @brief Show the dialog. @param mode what is chosen @param title window title @param directory starting folder
  //! @param extensions file name endings shown (e.g. {".cap", ".root"}; empty: all files)
  //! @param fileName initial file name (kSaveFile) @param callback called with the chosen path
  void open(Mode mode, const String & title, const String & directory, const std::vector<String> & extensions,
            const String & fileName, const Callback & callback);
  bool isOpen() const { return _open || _requested; }   //!< Whether the dialog is shown (or about to be)
  //! @brief Draw the dialog if open; call once per frame.
  void render();
  //! @brief Folder shown last (to start the next dialog there).
  const std::string & directory() const { return _directory; }

  protected:

  struct Entry { std::string name; bool folder; std::uintmax_t size; };   //!< Entry of the folder shown: name, whether it is a folder, size in bytes

  Mode _mode;   //!< what the dialog chooses
  std::string _title;   //!< window title
  std::string _directory;   //!< folder shown
  std::vector<std::string> _extensions;   //!< file name endings listed (empty: all)
  char _fileName[512];   //!< file name typed (kSaveFile)
  char _pathField[1024];   //!< editable path of the folder
  Callback _callback;   //!< called with the chosen path
  bool _open;   //!< the dialog is shown
  bool _requested;   //!< open() was called: the popup opens at the next frame
  bool _showHidden;   //!< hidden entries listed
  int  _selected;   //!< index of the selected entry, or -1
  std::vector<Entry> _entries;   //!< entries of the folder shown
  std::string _error;   //!< message shown when a folder cannot be listed

  //! @brief List the entries of the folder shown (see FileSystem::list()).
  void list();
  //! @brief Show another folder. @param directory folder
  void enter(const std::string & directory);
  //! @brief Close the dialog and pass the chosen path to the callback. @param path chosen path
  void confirm(const std::string & path);
  };

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__FileDialog
