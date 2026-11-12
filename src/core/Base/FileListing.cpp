/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FileListing.cpp
//! @brief listDirsIn(), listFilesInDir() (formerly in Helpers/RootFileHelpers).
#include "FileListing.hpp"
#include "FileSystem.hpp"
#include "PrintHelpers.hpp"

namespace CAP
{

std::vector<String> listDirsIn(const String & pathname,
                                bool verbose)
{
  if (verbose) printValue("listDirsIn() Search path",pathname);
  FileSystem::ListOptions options;
  options.files = false;
  options.order = FileSystem::ListOptions::kName;
  std::vector<String> subdirectories;
  for (const auto & entry : FileSystem::list(pathname,options)) subdirectories.push_back(entry.name);
  return subdirectories;
}

namespace
{
//! Files of directory whose names end with ext (sorted by name), then those of its sub-directories down to
//! maximumDepth; names relative to top, or full paths.
void collectFiles(const String & top, const String & directory, const String & ext, bool prependPath,
                  int maximumDepth, int depth, std::vector<String> & fileNames)
{
  FileSystem::ListOptions options;
  options.order = FileSystem::ListOptions::kName;
  std::vector<String> subdirectories;
  std::size_t prefix = top.length() + (top.endsWith("/") ? 0 : 1);
  for (const auto & entry : FileSystem::list(directory,options))
    {
    if (entry.isDirectory()) { subdirectories.push_back(entry.path); continue; }
    if (!entry.name.endsWith(ext)) continue;
    fileNames.push_back(prependPath ? entry.path : entry.path.subString(int(prefix)));
    }
  if (depth+1 > maximumDepth) return;
  for (const auto & subdirectory : subdirectories)
    collectFiles(top,subdirectory,ext,prependPath,maximumDepth,depth+1,fileNames);
}
} // namespace

std::vector<String> listFilesInDir(const String & pathname,
                                   const String & ext,
                                   bool prependPath,
                                   bool verbose,
                                   int  maximumDepth,
                                   int  currentDepth)
{
  if (verbose)
    {
    printCR();
    printString("listFilesInDir() Searching files");
    printValue("listFilesInDir() MaximumDepth:",maximumDepth);
    printValue("listFilesInDir() CurrentDepth:",currentDepth);
    }
  std::vector<String> fileNames;
  collectFiles(pathname,pathname,ext,prependPath,maximumDepth,currentDepth,fileNames);
  if (verbose)
    {
    printCR();
    printValue("listFilesInDir() Number of files found",fileNames.size());
    for (unsigned int k=0; k<fileNames.size();k++)
      {
      String s = "File "; s += k;
      printValue(s,fileNames[k]);
      }
    }
  return fileNames;
}

//!
//! Get file names at the given location that contain all the patterns in includePatterns and none of those in
//! excludePatterns
//!
std::vector<String>  listFilesInDir(const String & pathName,
                                    const std::vector<String> & includePatterns,
                                    const std::vector<String> & excludePatterns,
                                    bool prependPath,
                                    bool verbose,
                                    int  maximumDepth,
                                    int  currentDepth,
                                    const String & ext)
{
  std::vector<String> outputList;
  std::vector<String> fileList = listFilesInDir(pathName,ext,prependPath,verbose,maximumDepth,currentDepth);
  if (verbose)
    {
    printCR();
    printValue("nNames",fileList.size());
    printValue("nIncludes",includePatterns.size());
    printValue("nExcludes",excludePatterns.size());
    }
  for (const auto & name : fileList)
    {
    bool include = true;
    for (const auto & pattern : includePatterns) if (!name.contains(pattern)) { include = false; break; }
    for (const auto & pattern : excludePatterns) if (include && name.contains(pattern)) { include = false; break; }
    if (include) outputList.push_back(name);
    }
  return outputList;
}

} // namespace CAP
