/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Directory.cpp
//! @brief Implementation of Directory.
#include <algorithm>
#include <cstdio>
#include <sstream>
#include <string>
#include "Directory.hpp"
#include "PrintHelpers.hpp"


namespace CAP
{

  namespace
  {
  //! Entries of one level, as tree() and ls() show them: directories first, then files, by name.
  std::vector<FileSystem::EntryInfo> level(const String & path, bool hidden)
  {
  FileSystem::ListOptions options;
  options.hidden = hidden;
  return FileSystem::list(path,options);
  }

  void treeLevel(std::ostream & out, const String & path, const std::string & prefix, int depth, int current, bool hidden,
                 unsigned int & nFiles, unsigned int & nDirectories)
  {
  std::vector<FileSystem::EntryInfo> entries;
  try { entries = level(path,hidden); } catch (const FileException &) { return; }   // unreadable: shown empty
  for (std::size_t k=0; k<entries.size(); k++)
    {
    const FileSystem::EntryInfo & entry = entries[k];
    bool last = (k+1==entries.size());
    out << prefix << (last ? "└── " : "├── ") << entry.name.data();
    if (entry.isDirectory())
      {
      nDirectories++;
      out << "/" << (entry.isLink ? " (link)" : "") << "\n";
      if (!entry.isLink && (depth<0 || current<depth))
        treeLevel(out,entry.path,prefix + (last ? "    " : "│   "),depth,current+1,hidden,nFiles,nDirectories);
      }
    else
      {
      nFiles++;
      out << "  " << Directory::readableSize(entry.size).data() << "\n";
      }
    }
  }
  } // namespace

  Directory::Directory(const String & path)
  :
  FileSystemEntry(FileSystem::expand(path),"Directory")
  { }

  std::vector<FileSystem::EntryInfo> Directory::list(const FileSystem::ListOptions & options) const
  {
  return FileSystem::list(_path,options);
  }

  std::vector<String> Directory::names(const FileSystem::ListOptions & options) const
  {
  std::size_t prefix = _path.length() + (_path.endsWith("/") ? 0 : 1);
  std::vector<String> result;
  for (const auto & entry : list(options))
    result.push_back(entry.path.length()>int(prefix) ? entry.path.subString(int(prefix)) : entry.name);
  return result;
  }

  std::vector<String> Directory::files(const String & extension, int depth) const
  {
  FileSystem::ListOptions options;
  options.directories = false;
  options.depth = depth;
  if (!extension.isNull()) options.extensions = { extension };
  std::vector<String> result;
  for (const auto & entry : list(options)) result.push_back(entry.path);
  return result;
  }

  std::vector<Directory> Directory::subdirectories() const
  {
  FileSystem::ListOptions options;
  options.files = false;
  std::vector<Directory> result;
  for (const auto & entry : list(options)) result.push_back(Directory(entry.path));
  return result;
  }

  std::vector<String> Directory::find(const String & pattern, int depth) const
  {
  FileSystem::ListOptions options;
  options.directories = false;
  options.depth = depth;
  options.include = { pattern };
  std::vector<String> result;
  for (const auto & entry : list(options)) result.push_back(entry.path);
  return result;
  }

  void Directory::ls(std::ostream & out, const FileSystem::ListOptions & options) const
  {
  std::vector<String> shown = names(options);
  std::vector<FileSystem::EntryInfo> entries = list(options);
  std::size_t width = 0;
  for (const auto & name : shown) width = std::max(width, std::size_t(name.length()));   // sizes two spaces after the longest name
  for (std::size_t k=0; k<entries.size(); k++)
    {
    std::string name = shown[k].data();
    if (entries[k].isDirectory()) { out << name << "/\n"; continue; }
    out << name << std::string(width + 2 - name.size(), ' ') << readableSize(entries[k].size).data() << "\n";
    }
  }

  void Directory::tree(std::ostream & out, int depth, bool hidden) const
  {
  if (!isDirectory()) throw FileException(_path,"Not a directory","Directory::tree()");
  std::ostringstream body;
  unsigned int nFiles = 0, nDirectories = 0;
  treeLevel(body,_path,"",depth,1,hidden,nFiles,nDirectories);
  std::string head = std::string(_path.data()) + (_path.endsWith("/") ? "" : "/");
  out << head << "   " << nFiles << (nFiles==1 ? " file, " : " files, ")
      << nDirectories << (nDirectories==1 ? " directory" : " directories") << "\n" << body.str();
  }

  void Directory::print() const
  {
  FileSystemEntry::print();
  if (isDirectory() && isReadable()) printValue("Entries",long(list().size()));
  }

  std::uintmax_t Directory::remove(bool recursive) const
  {
  if (recursive) return FileSystem::removeTree(_path);
  FileSystem::remove(_path);
  return 1;
  }

  String Directory::readableSize(std::uintmax_t bytes)
  {
  static const char * units[] = { "kB", "MB", "GB", "TB" };
  char text[32];
  if (bytes < 1000) { std::snprintf(text, sizeof(text), "%llu B", (unsigned long long)bytes); return String(text); }
  double value = double(bytes)/1000.0;
  int unit = 0;
  while (value >= 999.95 && unit < 3) { value /= 1000.0; unit++; }
  std::snprintf(text, sizeof(text), "%.1f %s", value, units[unit]);
  return String(text);
  }

} // namespace CAP
