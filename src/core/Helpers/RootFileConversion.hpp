/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#ifndef CAP__RootFileConversion
#define CAP__RootFileConversion
#include <vector>
#include "Folder.hpp"

class TDirectory;

namespace CAP
{

//!
//! @file RootFileConversion.hpp
//! @brief Import of the histograms of ROOT files into CAP folders, and export of CAP folders to ROOT files.
//!
//! The directories of a ROOT file become folders, and folders become directories. Histograms (TH1, TH2, TH3) and
//! profiles (TProfile, TProfile2D) are converted (see RootConversion.hpp); the other objects are skipped and reported.
//!
//! Example:
//! @code
//!   CAP::Folder tree("run");
//!   std::vector<CAP::String> skipped;
//!   int n = CAP::importRootFile("old.root", tree, &skipped);   // histograms of old.root, as CAP histograms
//!   CAP::exportRootFile(tree, "copy.root");                     // and back
//! @endcode
//!

//!
//! @brief Add the histograms of a ROOT directory (and its sub-directories, as folders) to a folder.
//! @param directory ROOT directory @param folder folder receiving the histograms
//! @param skipped if not null, receives the paths of the objects not converted, with the reason
//! @return number of histograms converted
//!
int importRootDirectory(TDirectory & directory, Folder & folder, std::vector<String> * skipped=nullptr);

//!
//! @brief Add the histograms of a ROOT file (and its directories, as folders) to a folder.
//! @param path ROOT file @param folder folder receiving the histograms
//! @param skipped if not null, receives the paths of the objects not converted, with the reason
//! @return number of histograms converted
//! @throws Exception if the file cannot be opened
//!
int importRootFile(const String & path, Folder & folder, std::vector<String> * skipped=nullptr);

//!
//! @brief Write the histograms and profiles of a folder (and its sub-folders, as directories) to a ROOT directory.
//! @param folder folder @param directory ROOT directory
//! @param skipped if not null, receives the paths of the entries not converted, with the reason
//! @return number of histograms written
//!
int exportRootDirectory(const Folder & folder, TDirectory & directory, std::vector<String> * skipped=nullptr);

//!
//! @brief Write the histograms and profiles of a folder (and its sub-folders, as directories) to a new ROOT file
//! (replaced if it exists).
//! @param folder folder @param path ROOT file
//! @param skipped if not null, receives the paths of the entries not converted, with the reason
//! @return number of histograms written
//! @throws Exception if the file cannot be created
//!
int exportRootFile(const Folder & folder, const String & path, std::vector<String> * skipped=nullptr);

} // namespace CAP

#endif
