/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Directory.hpp
//! @brief Directory: listing, searching, creating and removing directories of the file system.
#ifndef CAP__Directory
#define CAP__Directory
#include <iostream>
#include <vector>
#include "FileSystemEntry.hpp"

namespace CAP
{

//!
//! @brief A directory of the file system: listing, searching, creating and removing.
//!
//! A Directory is a path; nothing is read until a listing is asked for, and each listing asks the operating system
//! again. Listings use FileSystem::list() and its options (depth, files and/or directories, hidden entries,
//! extensions, include and exclude patterns, order). Paths in the results are built from path() (relative if path()
//! is relative).
//!
//! Not to be confused with Folder, the container of CAP objects held in memory (e.g. in a CapFile).
//!
//! Example:
//! @code
//!   CAP::Directory data("$CAP_HISTOS_IMPORT_PATH/PbPb");     // expanded at construction
//!   for (const auto & file : data.files(".cap")) ...          // the CAP files of the directory
//!   std::vector<CAP::String> runs = data.find("run*.cap");    // the whole tree
//!   data.tree();                                               // print the tree, with sizes
//! @endcode
//!
class Directory : public FileSystemEntry
{
public:

  //! @brief Constructor. @param path path of the directory ("~" and environment variables are expanded, see
  //! FileSystem::expand())
  Directory(const String & path=".");
  //! @brief Destructor.
  virtual ~Directory() {}

  //! @name Listing
  //! @{

  //! @brief Entries of the directory (or of its tree). @param options what to list (see FileSystem::ListOptions)
  //! @return entries @throws FileException if the directory does not exist or cannot be read
  std::vector<FileSystem::EntryInfo> list(const FileSystem::ListOptions & options = FileSystem::ListOptions()) const;
  //! @brief Names of the entries of the directory (directories first). @param options what to list @return names
  //! (relative to this directory for a tree, e.g. "PbPb/run1.cap")
  std::vector<String> names(const FileSystem::ListOptions & options = FileSystem::ListOptions()) const;
  //! @brief Paths of the files of the directory. @param extension extension, e.g. ".cap" (empty: all) @param depth
  //! 1: this directory only; -1: the whole tree @return paths
  std::vector<String> files(const String & extension="", int depth=1) const;
  //! @brief Sub-directories (not hidden ones). @return directories
  std::vector<Directory> subdirectories() const;
  //! @brief Paths of the files of the tree whose name matches a glob pattern (*, ?, [abc]). @param pattern pattern of
  //! the names @param depth levels searched (-1: the whole tree) @return paths
  std::vector<String> find(const String & pattern, int depth=-1) const;
  //! @brief Whether the directory has an entry of that name. @param name name @return true if it exists
  bool contains(const String & name) const { return FileSystem::exists(join(name)); }
  //! @brief A sub-directory (it need not exist). @param name name, or relative path @return directory
  Directory directory(const String & name) const { return Directory(join(name)); }
  //! @brief Path of an entry of the directory. @param name name, or relative path @return path
  String join(const String & name) const { return FileSystem::join(_path,name); }

  //! @}

  //! @name Printing
  //! @{

  //!
  //! @brief Print the entries, one per line: directories first, with a final '/', then the files with their sizes.
  //! @param out stream @param options what to list
  //!
  void ls(std::ostream & out = std::cout, const FileSystem::ListOptions & options = FileSystem::ListOptions()) const;
  //!
  //! @brief Print the tree of the directory, with the sizes of the files and, on the first line, the number of files
  //! and directories.
  //! @param out stream @param depth levels shown (-1: all) @param hidden show hidden entries
  //!
  void tree(std::ostream & out = std::cout, int depth=-1, bool hidden=false) const;
  //! @brief Print the path and the number of entries.
  void print() const override;

  //! @}

  //! @name Changes
  //! @{

  //! @brief Create the directory (nothing if it exists). @param parents also create missing parent directories
  //! @throws FileException if it cannot be created
  void create(bool parents=true) const { FileSystem::createDirectory(_path,parents); }
  //!
  //! @brief Remove the directory. @param recursive remove its content too (otherwise it must be empty)
  //! @return number of entries removed @throws FileException if it cannot be removed
  //!
  std::uintmax_t remove(bool recursive=false) const;

  //! @}

  //! @brief Size in a short readable form: "512 B", "12.4 kB", "3.1 MB", "2.0 GB". @param bytes size @return text
  static String readableSize(std::uintmax_t bytes);

};

} // namespace CAP

#endif
