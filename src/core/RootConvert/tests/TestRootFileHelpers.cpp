/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestRootFileHelpers.cpp
//! @brief Test program of the file listing and file name helpers (run by ctest).
//
// TestRootFileHelpers -- tests of the file listing and file name helpers (listDirsIn, listFilesInDir, makeFileName),
// now built on Datum::FileSystem, and of DatumFile::makeFileName and XmlDocument::fileName.
//
// Builds a small tree in a temporary directory, removed at the end. Prints each failed check and exits with the number
// of failures (0: all passed).
//
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include "RootFileHelpers.hpp"
#include "DatumFile.hpp"
#include "Directory.hpp"
#include "XmlDocument.hpp"

using namespace Datum;

namespace
{
int nChecks = 0, nFailures = 0;

void check(bool ok, const std::string & what, int line)
{
  nChecks++;
  if (ok) return;
  nFailures++;
  std::printf("FAILED (line %d): %s\n", line, what.c_str());
}

std::string joined(const std::vector<String> & values)
{
  std::string s;
  for (const auto & v : values) s += (s.empty() ? "" : " ") + std::string(v.data());
  return s;
}
} // namespace

//! @brief Count a check, and report it if the condition is false.
#define CHECK(condition) check((condition), #condition, __LINE__)
//! @brief Count a check, and report it if the joined values differ from the expected text.
#define CHECK_LIST(values, expected) check(joined(values)==(expected), std::string(#values) + ": got \"" + joined(values) + "\"", __LINE__)

//! @brief Run the checks. @return number of failed checks (0: all passed)
int main()
{
  // root/ b_Gen.datum  a_Gen.datum  a_Reco.datum  notes.txt  sub/ c_Gen.datum  sub/deep/ d_Gen.datum  other/
  char buffer[] = "/tmp/DatumTestRootFileHelpers.XXXXXX";
  Directory top{String(mkdtemp(buffer))};
  top.directory("sub/deep").create();
  top.directory("other").create();
  for (const char * f : {"b_Gen.datum", "a_Gen.datum", "a_Reco.datum", "notes.txt", "sub/c_Gen.datum", "sub/deep/d_Gen.datum"})
    std::ofstream(top.join(f).data()) << "x";
  String root = top.path();

  CHECK_LIST(listDirsIn(root), "other sub");
  // depth counted from 0: maximumDepth 0 is the directory only, 1 (default) adds one level of sub-directories
  CHECK_LIST(listFilesInDir(root, ".datum", false, false, 0), "a_Gen.datum a_Reco.datum b_Gen.datum");
  CHECK_LIST(listFilesInDir(root, ".datum", false), "a_Gen.datum a_Reco.datum b_Gen.datum sub/c_Gen.datum");
  CHECK_LIST(listFilesInDir(root, ".datum", false, false, 5), "a_Gen.datum a_Reco.datum b_Gen.datum sub/c_Gen.datum sub/deep/d_Gen.datum");
  std::vector<String> full = listFilesInDir(root, ".datum", true);
  CHECK(full.size()==4 && full[3].equalTo(top.join("sub/c_Gen.datum")));        // no "//"
  CHECK_LIST(listFilesInDir(root, ".txt", false), "notes.txt");
  CHECK_LIST(listFilesInDir(root, "", false, false, 0), "a_Gen.datum a_Reco.datum b_Gen.datum notes.txt");
  // include: all the patterns; exclude: none of them (the exclusion was inverted)
  using Patterns = std::vector<String>;
  CHECK_LIST(listFilesInDir(root, Patterns{"_Gen"}, Patterns{}, false), "a_Gen.datum b_Gen.datum sub/c_Gen.datum");
  CHECK_LIST(listFilesInDir(root, Patterns{"_Gen"}, Patterns{"b_"}, false), "a_Gen.datum sub/c_Gen.datum");
  CHECK_LIST(listFilesInDir(root, Patterns{"a_", ".datum"}, Patterns{"Reco"}, false), "a_Gen.datum");
  CHECK_LIST(listFilesInDir(root, Patterns{}, Patterns{}, false, false, 0, 0, ".txt"), "notes.txt");
  bool thrown = false;
  try { listFilesInDir(top.join("missing"), ".datum"); } catch (const FileException &) { thrown = true; }
  CHECK(thrown);

  // file names: an absolute name is used as it is; the extension is appended if missing
  CHECK(makeFileName("dir", "run", ".root").equalTo("dir/run.root"));
  CHECK(makeFileName("dir/", "run.root", ".root").equalTo("dir/run.root"));
  CHECK(makeFileName("dir", "/abs/run", ".root").equalTo("/abs/run.root"));
  CHECK(makeFileName("", "run", ".root").equalTo("run.root"));
  CHECK(DatumFile::makeFileName("dir", "run").equalTo("dir/run.datum"));
  CHECK(DatumFile::makeFileName("dir", "/abs/run.cap").equalTo("/abs/run.cap"));       // a file of CAP
  CHECK(DatumFile::makeFileName("dir", "/abs/run.datum").equalTo("/abs/run.datum"));
  CHECK(DatumFile::makeFileName("", "run").equalTo("run.datum"));
  CHECK(XmlDocument::fileName("dir", "config").equalTo("dir/config.xml"));
  CHECK(XmlDocument::fileName("dir", "/abs/config.xml").equalTo("/abs/config.xml"));

  top.remove(true);
  std::printf("TestRootFileHelpers: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
