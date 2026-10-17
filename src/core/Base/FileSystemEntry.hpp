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
#ifndef CAP__FileSystemEntry
#define CAP__FileSystemEntry
#include "FileSystem.hpp"
#include "Object.hpp"

namespace CAP
{

//!
//! @brief An entry of the file system, given by its path: the common base of File and Directory.
//!
//! It answers the questions about the entry (see FileSystem): whether it exists, its type, size, date and the
//! permissions of the current user, and the parts of its path. Each question asks the operating system again: the
//! answers follow changes made to the file system. info() takes a snapshot of everything at once.
//!
//! The name of the object (Object::name()) is the path given at construction; the last component of the path is
//! baseName().
//!
//!     FileSystemEntry ─┬─ File ─┬─ TxtFile ──── XmlFile
//!                      │        └─ BinaryFile ── CapFile
//!                      └─ Directory
//!
class FileSystemEntry : public Object
{
protected:

  //! @brief Constructor for derived classes. @param path path of the entry @param className name of the derived class
  FileSystemEntry(const String & path, const String & className);

public:

  //! @brief Destructor.
  virtual ~FileSystemEntry() {}

  //! @name Path
  //! @{

  //! @brief Path of the entry. @return path
  const String & path() const { return _path; }
  //! @brief Set the path (the name of the object is unchanged). @param path new path
  virtual void setPath(const String & path) { _path = path; }
  //! @brief Last component of the path: "a/b/c.cap" gives "c.cap". @return name
  String baseName() const { return FileSystem::name(_path); }
  //! @brief Directory part of the path: "a/b/c.cap" gives "a/b". @return directory
  String parentPath() const { return FileSystem::parent(_path); }
  //! @brief Last component without its extension: "a/b/c.cap" gives "c". @return stem
  String stem() const { return FileSystem::stem(_path); }
  //! @brief Extension, with its '.': "a/b/c.cap" gives ".cap". @return extension, or ""
  String extension() const { return FileSystem::extension(_path); }
  //! @brief Absolute path, without "." and "..". @return path
  String absolutePath() const { return FileSystem::normalize(_path); }

  //! @}

  //! @name Questions to the operating system
  //! @{

  //! @brief Whether the entry exists (of any type). @return true if it exists
  virtual bool exists() const { return FileSystem::exists(_path); }
  //! @brief Type of the entry. @return type (kNone if it does not exist)
  FileSystem::EntryType type() const;
  //! @brief Whether the entry is a regular file. @return true if so
  bool isFile() const { return FileSystem::isFile(_path); }
  //! @brief Whether the entry is a directory. @return true if so
  bool isDirectory() const { return FileSystem::isDirectory(_path); }
  //! @brief Whether the entry is a symbolic link. @return true if so
  bool isLink() const { return FileSystem::isLink(_path); }
  //! @brief Size of the file in bytes. @return size @throws FileException if it is not a regular file
  std::uintmax_t fileSize() const;
  //! @brief Time of the last modification. @return time @throws FileException if it does not exist
  std::time_t modified() const { return FileSystem::modified(_path); }
  //! @brief Whether the current user may read the entry. @return true if so
  bool isReadable() const { return FileSystem::isReadable(_path); }
  //! @brief Whether the current user may write the entry (or create it, if it does not exist). @return true if so
  bool isWritable() const { return FileSystem::isWritable(_path); }
  //! @brief Everything known about the entry. @return information @throws FileException if it does not exist
  FileSystem::EntryInfo info() const { return FileSystem::info(_path); }

  //! @}

  //! @brief Print the path and, if the entry exists, its type, size and permissions.
  void print() const override;

protected:

  String _path;  //!< path of the entry

  ClassDefOverride(FileSystemEntry,0)
};

} // namespace CAP

#endif
