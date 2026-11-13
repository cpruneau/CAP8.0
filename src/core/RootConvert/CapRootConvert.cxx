/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file CapRootConvert.cxx
//! @brief Convert the histograms of a ROOT file to a CAP file, or those of a CAP file to a ROOT file.
//!
//! Usage: CapRootConvert input.root output.cap   or   CapRootConvert input.cap output.root
//! Directories and folders are kept; objects other than histograms and profiles are listed as skipped.
#include <cstdio>
#include <vector>
#include "CapFile.hpp"
#include "FileSystem.hpp"
#include "RootFileConversion.hpp"

using namespace CAP;

int main(int argc, char ** argv)
{
  if (argc != 3)
    {
    std::printf("Usage: CapRootConvert input.root output.cap\n"
                "       CapRootConvert input.cap output.root\n");
    return 1;
    }
  String input(argv[1]), output(argv[2]);
  std::vector<String> skipped;
  int count = 0;
  try
    {
    if (FileSystem::hasExtension(input, ".root") && FileSystem::hasExtension(output, ".cap"))
      {
      Folder tree(FileSystem::stem(input));
      count = importRootFile(input, tree, &skipped);
      CapFile file(output, File::kWriteOnly);
      file = tree;
      file.write();
      }
    else if (FileSystem::hasExtension(input, ".cap") && FileSystem::hasExtension(output, ".root"))
      {
      CapFile file(input, File::kReadOnly);
      file.read();
      count = exportRootFile(file, output, &skipped);
      }
    else
      {
      std::printf("CapRootConvert: convert a .root file to a .cap file, or a .cap file to a .root file\n");
      return 1;
      }
    }
  catch (const std::exception & e)
    {
    std::printf("CapRootConvert: %s\n", e.what());
    return 2;
    }
  for (const auto & s : skipped) std::printf("skipped %s\n", s.data());
  std::printf("%d histogram%s written to %s\n", count, count == 1 ? "" : "s", output.data());
  return 0;
}
