/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file DatumRootConvert.cxx
//! @brief Convert the histograms of a ROOT file to a Datum file, or those of a Datum file to a ROOT file.
//!
//! Usage: DatumRootConvert input.root output.datum   or   DatumRootConvert input.datum output.root (or .cap, the files of CAP)
//! Directories and folders are kept; objects other than histograms and profiles are listed as skipped.
#include <cstdio>
#include <vector>
#include "DatumFile.hpp"
#include "FileSystem.hpp"
#include "RootFileConversion.hpp"

using namespace Datum;

int main(int argc, char ** argv)
{
  if (argc != 3)
    {
    std::printf("Usage: DatumRootConvert input.root output.datum\n"
                "       DatumRootConvert input.datum output.root\n");
    return 1;
    }
  String input(argv[1]), output(argv[2]);
  std::vector<String> skipped;
  int count = 0;
  try
    {
    if (FileSystem::hasExtension(input, ".root") && DatumFile::isDatumFileName(output))
      {
      Folder tree(FileSystem::stem(input));
      count = importRootFile(input, tree, &skipped);
      DatumFile file(output, File::kWriteOnly);
      file = tree;
      file.write();
      }
    else if (DatumFile::isDatumFileName(input) && FileSystem::hasExtension(output, ".root"))
      {
      DatumFile file(input, File::kReadOnly);
      file.read();
      count = exportRootFile(file, output, &skipped);
      }
    else
      {
      std::printf("DatumRootConvert: convert a .root file to a Datum file (.datum, .cap), or a Datum file to a .root file\n");
      return 1;
      }
    }
  catch (const std::exception & e)
    {
    std::printf("DatumRootConvert: %s\n", e.what());
    return 2;
    }
  for (const auto & s : skipped) std::printf("skipped %s\n", s.data());
  std::printf("%d histogram%s written to %s\n", count, count == 1 ? "" : "s", output.data());
  return 0;
}
