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
//! @file FileSystemEntry.cpp
//! @brief Implementation of FileSystemEntry.
#include "FileSystemEntry.hpp"
#include "PrintHelpers.hpp"

ClassImp(CAP::FileSystemEntry);

namespace CAP
{

  FileSystemEntry::FileSystemEntry(const String & path, const String & className)
  :
  Object(path,path,className),
  _path(path)
  { }

  FileSystem::EntryType FileSystemEntry::type() const
  {
  return FileSystem::exists(_path) ? FileSystem::info(_path).type : FileSystem::EntryType::kNone;
  }

  std::uintmax_t FileSystemEntry::fileSize() const
  {
  if (!FileSystem::isFile(_path)) throw FileException(_path,"File does not exist","FileSystemEntry::fileSize()");
  return FileSystem::size(_path);
  }

  void FileSystemEntry::print() const
  {
  Object::print();
  printValue("Path",_path);
  if (!FileSystem::exists(_path)) { printValue("Exists",String("no")); return; }
  FileSystem::EntryInfo entry = FileSystem::info(_path);
  const char * type = entry.isFile() ? "file" : (entry.isDirectory() ? "directory" : "other");
  printValue("Type",String(entry.isLink ? (std::string(type)+" (symbolic link)").c_str() : type));
  if (entry.isFile()) printValue("Size (bytes)",long(entry.size));
  printValue("Permissions",String((std::string(entry.readable ? "r" : "-")+(entry.writable ? "w" : "-")+(entry.executable ? "x" : "-")).c_str()));
  }

} // namespace CAP
