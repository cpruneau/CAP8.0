/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FileListing.hpp
//! @brief listDirsIn(), listFilesInDir(): the files and sub-directories of a directory, by name (built on FileSystem).
#ifndef CAP__FileListing
#define CAP__FileListing
#include <vector>
#include "String.hpp"

namespace CAP
{
//! @brief Names of the sub-directories of pathname (not hidden ones), by name. @throws FileException if it is not a directory
std::vector<String> listDirsIn(const String & pathname,bool verbose=false);
//! @brief Files of pathname whose names end with ext, by name, then those of its sub-directories while
//! currentDepth < maximumDepth (maximumDepth 0: pathname only). With prependPath, the full paths; otherwise the paths
//! relative to pathname (e.g. "sub/file.cap"). @throws FileException if pathname is not a directory
std::vector<String> listFilesInDir(const String & pathname,
                                   const String & ext,
                                   bool prependPath=1,
                                   bool verbose=0,
                                   int  maximumDepth=1,
                                   int  currentDepth=0);

//! @brief Files of pathName (and of its sub-directories, see the other listFilesInDir()) whose names end with ext,
//! contain every pattern of includePatterns and none of excludePatterns (plain substrings).
std::vector<String>  listFilesInDir(const String & pathName,
                                    const std::vector<String> & includePatterns,
                                    const std::vector<String> & excludePatterns,
                                    bool prependPath=1,
                                    bool verbose=0,
                                    int  maximumDepth=1,
                                    int  currentDepth=0,
                                    const String & ext=".cap");

} // namespace CAP

#endif // !CAP__FileListing
