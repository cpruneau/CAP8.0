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
//! @file TestRootFileHelpers.cpp
//! @brief Test program of the file listing and file name helpers (run by ctest).
//
// TestRootFileHelpers -- tests of the file listing and file name helpers (listDirsIn, listFilesInDir, makeFileName),
// now built on CAP::FileSystem, and of CapFile::makeFileName and XmlDocument::fileName.
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
#include "CapFile.hpp"
#include "Directory.hpp"
#include "XmlDocument.hpp"

using namespace CAP;

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

#define CHECK(condition) check((condition), #condition, __LINE__)
#define CHECK_LIST(values, expected) check(joined(values)==(expected), std::string(#values) + ": got \"" + joined(values) + "\"", __LINE__)

int main()
{
  // root/ b_Gen.cap  a_Gen.cap  a_Reco.cap  notes.txt  sub/ c_Gen.cap  sub/deep/ d_Gen.cap  other/
  char buffer[] = "/tmp/CapTestRootFileHelpers.XXXXXX";
  Directory top{String(mkdtemp(buffer))};
  top.directory("sub/deep").create();
  top.directory("other").create();
  for (const char * f : {"b_Gen.cap", "a_Gen.cap", "a_Reco.cap", "notes.txt", "sub/c_Gen.cap", "sub/deep/d_Gen.cap"})
    std::ofstream(top.join(f).data()) << "x";
  String root = top.path();

  CHECK_LIST(listDirsIn(root), "other sub");
  // depth counted from 0: maximumDepth 0 is the directory only, 1 (default) adds one level of sub-directories
  CHECK_LIST(listFilesInDir(root, ".cap", false, false, 0), "a_Gen.cap a_Reco.cap b_Gen.cap");
  CHECK_LIST(listFilesInDir(root, ".cap", false), "a_Gen.cap a_Reco.cap b_Gen.cap sub/c_Gen.cap");
  CHECK_LIST(listFilesInDir(root, ".cap", false, false, 5), "a_Gen.cap a_Reco.cap b_Gen.cap sub/c_Gen.cap sub/deep/d_Gen.cap");
  std::vector<String> full = listFilesInDir(root, ".cap", true);
  CHECK(full.size()==4 && full[3].equalTo(top.join("sub/c_Gen.cap")));        // no "//"
  CHECK_LIST(listFilesInDir(root, ".txt", false), "notes.txt");
  CHECK_LIST(listFilesInDir(root, "", false, false, 0), "a_Gen.cap a_Reco.cap b_Gen.cap notes.txt");
  // include: all the patterns; exclude: none of them (the exclusion was inverted)
  using Patterns = std::vector<String>;
  CHECK_LIST(listFilesInDir(root, Patterns{"_Gen"}, Patterns{}, false), "a_Gen.cap b_Gen.cap sub/c_Gen.cap");
  CHECK_LIST(listFilesInDir(root, Patterns{"_Gen"}, Patterns{"b_"}, false), "a_Gen.cap sub/c_Gen.cap");
  CHECK_LIST(listFilesInDir(root, Patterns{"a_", ".cap"}, Patterns{"Reco"}, false), "a_Gen.cap");
  CHECK_LIST(listFilesInDir(root, Patterns{}, Patterns{}, false, false, 0, 0, ".txt"), "notes.txt");
  bool thrown = false;
  try { listFilesInDir(top.join("missing"), ".cap"); } catch (const FileException &) { thrown = true; }
  CHECK(thrown);

  // file names: an absolute name is used as it is; the extension is appended if missing
  CHECK(makeFileName("dir", "run", ".root").equalTo("dir/run.root"));
  CHECK(makeFileName("dir/", "run.root", ".root").equalTo("dir/run.root"));
  CHECK(makeFileName("dir", "/abs/run", ".root").equalTo("/abs/run.root"));
  CHECK(makeFileName("", "run", ".root").equalTo("run.root"));
  CHECK(CapFile::makeFileName("dir", "run").equalTo("dir/run.cap"));
  CHECK(CapFile::makeFileName("dir", "/abs/run.cap").equalTo("/abs/run.cap"));
  CHECK(CapFile::makeFileName("", "run").equalTo("run.cap"));
  CHECK(XmlDocument::fileName("dir", "config").equalTo("dir/config.xml"));
  CHECK(XmlDocument::fileName("dir", "/abs/config.xml").equalTo("/abs/config.xml"));

  top.remove(true);
  std::printf("TestRootFileHelpers: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
