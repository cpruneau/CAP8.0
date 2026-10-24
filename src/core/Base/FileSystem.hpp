/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FileSystem.hpp
//! @brief FileSystem: functions asking the operating system about files and directories, and changing them.
#ifndef CAP__FileSystem
#define CAP__FileSystem
#include <cstdint>
#include <ctime>
#include <vector>
#include "Aliases.hpp"
#include "Exceptions.hpp"

namespace CAP
{

//!
//! @brief Functions asking the operating system about files and directories, and changing them.
//!
//! One implementation, shared by File, Directory, the graphical interface and the analysis code, for what was done in
//! several places with different rules: types, sizes, permissions and dates of entries; building and splitting paths;
//! expanding "~" and environment variables; listing directories; globbing; creating, removing, renaming and copying.
//!
//! Errors: the questions (exists(), isFile(), isReadable(), ...) never throw, they answer false. Functions that must
//! return something about an entry (size(), modified(), info(), list()) and all the changes throw FileException with
//! the path and the reason given by the system.
//!
//! Paths are kept as given (relative paths stay relative); use normalize() for an absolute path without "." and "..".
//!
//! Example:
//! @code
//!   using namespace CAP;
//!   String file = FileSystem::join("$CAP_HISTOS_EXPORT_PATH/PbPb", "run42", ".cap");   // ".../PbPb/run42.cap"
//!   if (FileSystem::isFile(FileSystem::expand(file))) ...
//!   FileSystem::ListOptions options;
//!   options.depth = -1;                        // the whole tree
//!   options.extensions = {".cap"};
//!   for (const auto & entry : FileSystem::list("data", options)) std::cout << entry.path << std::endl;
//!   std::vector<String> runs = FileSystem::glob("data/**/run*.cap");
//! @endcode
//!
namespace FileSystem
{

  //! @brief Type of an entry of the file system (of the target, for a symbolic link).
  enum class EntryType { kNone, kFile, kDirectory, kSymlink, kOther };

  //!
  //! @brief What is known about one entry of the file system: a snapshot, filled by info() and by list().
  //!
  struct EntryInfo
  {
    String         path;                  //!< path as given, or as built by the listing
    String         name;                  //!< last component of the path
    EntryType      type = EntryType::kNone;  //!< type (of the target, for a symbolic link; kSymlink if it is broken)
    bool           isLink     = false;    //!< the entry itself is a symbolic link
    std::uintmax_t size       = 0;        //!< size in bytes (0 for a directory)
    std::time_t    modified   = 0;        //!< time of the last modification
    bool           readable   = false;    //!< the current user may read it
    bool           writable   = false;    //!< the current user may write it
    bool           executable = false;    //!< the current user may execute it (enter it, for a directory)
    bool           hidden     = false;    //!< the name starts with '.'

    //! @brief Whether the entry is a regular file. @return true for kFile
    bool isFile() const { return type==EntryType::kFile; }
    //! @brief Whether the entry is a directory. @return true for kDirectory
    bool isDirectory() const { return type==EntryType::kDirectory; }
  };

  //!
  //! @brief What list() returns and in what order.
  //!
  //! extensions and include select files only (directories are listed and descended whatever their name); exclude
  //! applies to files and directories, and an excluded directory is not descended. Hidden entries (names starting
  //! with '.') are left out, and hidden directories not descended, unless hidden is true.
  //!
  struct ListOptions
  {
    //! @brief Order of the entries.
    enum Order { kName, kDirectoriesFirst, kModified, kSize };

    int                 depth       = 1;      //!< 1: the directory only; n: n levels; -1: the whole tree
    bool                files       = true;   //!< list the files
    bool                directories = true;   //!< list the directories
    bool                hidden      = false;  //!< list (and descend into) hidden entries
    std::vector<String> extensions;           //!< file extensions, e.g. {".cap"} (case-insensitive); empty: all
    std::vector<String> include;              //!< glob patterns of file names; empty: all
    std::vector<String> exclude;              //!< glob patterns of names left out (files and directories)
    bool                followLinks = false;  //!< descend into directories reached through symbolic links
    //! In a tree, kName and kDirectoriesFirst sort the entries of each directory (case-insensitive) and give each
    //! directory before its content; kModified (newest first) and kSize (largest first) sort the whole list.
    Order               order       = kDirectoriesFirst;
  };

  //! @name Information
  //! @{

  //! @brief Whether an entry exists (a broken symbolic link counts). @param path path @return true if it exists
  bool exists(const String & path);
  //! @brief Whether the path is a regular file (following symbolic links). @param path path @return true if so
  bool isFile(const String & path);
  //! @brief Whether the path is a directory (following symbolic links). @param path path @return true if so
  bool isDirectory(const String & path);
  //! @brief Whether the path is a symbolic link. @param path path @return true if so
  bool isLink(const String & path);
  //! @brief Whether the current user may read the entry. @param path path @return false if it does not exist
  bool isReadable(const String & path);
  //! @brief Whether the current user may write the entry, or create it (for a missing entry: whether its directory
  //! is writable). @param path path @return true if it may be written
  bool isWritable(const String & path);
  //! @brief Whether the current user may execute the entry (enter it, for a directory). @param path path @return true if so
  bool isExecutable(const String & path);
  //! @brief Size of a regular file. @param path path @return size in bytes @throws FileException if not a regular file
  std::uintmax_t size(const String & path);
  //! @brief Time of the last modification. @param path path @return time @throws FileException if the entry does not exist
  std::time_t modified(const String & path);
  //! @brief Everything known about an entry. @param path path @return information @throws FileException if it does not exist
  EntryInfo info(const String & path);

  //! @}

  //! @name Paths
  //! @{

  //! @brief Path of an entry of a directory: "a" and "b" give "a/b". An absolute name is returned as is, an empty
  //! directory gives the name, an empty name the directory. @param directory directory @param name name @return path
  String join(const String & directory, const String & name);
  //! @brief Path of a file of a directory, with its extension: the extension (with or without its '.') is appended to
  //! the name unless the name already has it. @param directory directory @param name file name @param extension
  //! extension, e.g. ".cap" @return path
  String join(const String & directory, const String & name, const String & extension);
  //! @brief Absolute path, without "." and ".." components and without a final '/'. @param path path @return path
  String normalize(const String & path);
  //! @brief Directory part: "a/b/c.cap" gives "a/b", "c.cap" gives "", "/c" gives "/". @param path path @return directory
  String parent(const String & path);
  //! @brief Last component: "a/b/c.cap" and "a/b/c.cap/" give "c.cap". @param path path @return name
  String name(const String & path);
  //! @brief Last component without its extension: "a/b/c.cap" gives "c". @param path path @return stem
  String stem(const String & path);
  //! @brief Extension of the last component, with its '.': "a/b/c.cap" gives ".cap", ".bashrc" gives "". @param path
  //! path @return extension, or ""
  String extension(const String & path);
  //! @brief Whether the last component has an extension (case-insensitive; with or without its '.'). @param path path
  //! @param extension extension @return true if it has it
  bool hasExtension(const String & path, const String & extension);
  //! @brief The path with its extension replaced (or added). @param path path @param extension new extension (with or
  //! without '.'; empty to remove it) @return path
  String withExtension(const String & path, const String & extension);
  //! @brief Expand "~" (home directory) at the start, and environment variables written $NAME or ${NAME}. A variable
  //! that is not defined is left as written. @param path path @return expanded path
  String expand(const String & path);
  //! @brief Home directory of the user ($HOME). @return path, or "" if unknown
  String home();
  //! @brief Current working directory. @return path
  String current();
  //!
  //! @brief Resolve a path given in a configuration: expanded (see expand()); then an absolute path is returned as is,
  //! "." or a path starting with "./" is relative to the current directory, and any other path is relative to the
  //! directory held by the environment variable.
  //! @param path path @param variable name of the environment variable, e.g. "CAP_HISTOS_IMPORT_PATH"
  //! @return path @throws FileException if the path is relative to the variable and the variable is not defined
  //!
  String resolve(const String & path, const String & variable);
  //!
  //! @brief Path given in the configuration of a task, with the convention of the CAP tasks: "DEFAULT" (or an empty
  //! path) is the directory held by the environment variable, any other path is resolved as by resolve(). Unlike
  //! resolve(), it never throws, since tasks resolve paths they may not use: if the variable is not defined,
  //! "DEFAULT" gives "" and a relative path is returned as given.
  //! @param path path from the configuration @param variable environment variable, e.g. "CAP_HISTOS_IMPORT_PATH"
  //! @return path
  //!
  String configuredPath(const String & path, const String & variable);
  //! @brief First candidate that exists (empty candidates are skipped). @param candidates paths @return path, or ""
  String firstExisting(const std::vector<String> & candidates);

  //! @}

  //! @name Listing and searching
  //! @{

  //!
  //! @brief Entries of a directory, or of its tree (see ListOptions). Paths are join(directory, ...). Sub-directories
  //! that cannot be read are skipped.
  //! @param directory directory @param options what to list @return entries
  //! @throws FileException if directory is not a directory or cannot be read
  //!
  std::vector<EntryInfo> list(const String & directory, const ListOptions & options = ListOptions());
  //!
  //! @brief Paths matching a pattern (expanded first, see expand()). Each component of the pattern may use * (any
  //! characters), ? (one character) and [abc], [a-z], [!abc] (one character of, or not of, a set); a component **
  //! matches any number of directories. Hidden entries match only a component that starts with '.'.
  //! @param pattern pattern, e.g. "data/run*.cap", "data/\*\*/\*.cap" @return paths, sorted
  //!
  std::vector<String> glob(const String & pattern);
  //! @brief Whether a name matches a glob pattern (*, ?, [abc], [a-z], [!abc]; no '/'). @param name name
  //! @param pattern pattern @return true if it matches
  bool matches(const String & name, const String & pattern);

  //! @}

  //! @name Changes
  //! @{

  //! @brief Create a directory (nothing if it exists). @param path path @param parents also create the missing
  //! parent directories @throws FileException if it cannot be created, or exists and is not a directory
  void createDirectory(const String & path, bool parents = true);
  //! @brief Remove a file, a symbolic link or an empty directory. @param path path
  //! @throws FileException if it does not exist or cannot be removed (e.g. a directory that is not empty)
  void remove(const String & path);
  //! @brief Remove a directory and all its content (or a file). @param path path @return number of entries removed
  //! @throws FileException if it does not exist, cannot be removed, or is "/" or the home directory
  std::uintmax_t removeTree(const String & path);
  //! @brief Rename or move an entry. @param from current path @param to new path @throws FileException on failure
  void rename(const String & from, const String & to);
  //! @brief Copy a file, or a directory and its content. @param from source @param to destination @param overwrite
  //! replace existing files @throws FileException if the source is missing, the destination exists (and overwrite is
  //! false), or the copy fails
  void copy(const String & from, const String & to, bool overwrite = false);

  //! @}

} // namespace FileSystem

} // namespace CAP

#endif
