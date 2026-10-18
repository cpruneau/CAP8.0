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
//! @file TestDirectory.cpp
//! @brief Test program of FileSystemEntry, Directory, and File as a FileSystemEntry (run by ctest).
//
// TestDirectory -- tests of CAP::FileSystemEntry, CAP::Directory, and of File as a FileSystemEntry.
//
// Builds a small tree in a temporary directory, checks the classes (including the printed listings and errors), then
// removes the tree. Prints each failed check and exits with the number of failures (0: all passed).
//
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include "Directory.hpp"
#include "BinaryFile.hpp"
#include "TxtFile.hpp"

using namespace CAP;
namespace FS = CAP::FileSystem;

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

void checkEqual(const std::string & value, const std::string & expected, const std::string & what, int line)
{
  check(value==expected, what + ":\n--- got:\n" + value + "\n--- expected:\n" + expected, line);
}

void checkThrows(const std::function<void()> & f, const std::string & what, int line)
{
  bool thrown = false;
  try { f(); } catch (const FileException &) { thrown = true; } catch (...) { }
  check(thrown, what + " should throw FileException", line);
}

std::string joined(const std::vector<String> & values)
{
  std::string s;
  for (const auto & v : values) s += (s.empty() ? "" : " ") + std::string(v.data());
  return s;
}

void writeFile(const String & path, std::size_t bytes)
{
  std::ofstream out(path.data(), std::ios::binary);
  out << std::string(bytes, 'x');
}
} // namespace

//! @brief Count a check, and report it if the condition is false.
#define CHECK(condition)             check((condition), #condition, __LINE__)
//! @brief Count a check, and report it with both texts if the value differs from the expected one.
#define CHECK_EQUAL(value, expected) checkEqual((value), (expected), #value, __LINE__)
//! @brief Count a check, and report it if the statement does not throw.
#define CHECK_THROWS(statement)      checkThrows([&]() { statement; }, #statement, __LINE__)

//! @brief Run the checks. @return number of failed checks (0: all passed)
int main()
{
  // ---- a temporary tree
  //   root/
  //     notes.txt (1500 bytes)
  //     data/  run1.cap (10)  run2.cap (2 000 000)  x.root (3)
  //       deep/  run3.cap (0)
  //     empty/
  char buffer[] = "/tmp/CapTestDirectory.XXXXXX";
  String root(mkdtemp(buffer));
  Directory top(root);
  top.directory("data/deep").create();
  top.directory("empty").create();
  writeFile(top.join("notes.txt"), 1500);
  writeFile(top.join("data/run1.cap"), 10);
  writeFile(top.join("data/run2.cap"), 2000000);
  writeFile(top.join("data/x.root"), 3);
  writeFile(top.join("data/deep/run3.cap"), 0);

  // ---- FileSystemEntry, through Directory
  CHECK(top.exists() && top.isDirectory() && !top.isFile() && !top.isLink());
  CHECK(top.type()==FS::EntryType::kDirectory);
  CHECK(top.name().equalTo(root));                 // the object is named by its path
  CHECK(top.baseName().equalTo(FS::name(root)));
  CHECK(top.parentPath().equalTo("/tmp"));
  CHECK(top.isReadable() && top.isWritable());
  CHECK(top.info().isDirectory());
  CHECK_THROWS(top.fileSize());
  Directory missing(top.join("missing"));
  CHECK(!missing.exists() && missing.type()==FS::EntryType::kNone);
  CHECK_THROWS(missing.info());
  CHECK_THROWS(missing.list());
  setenv("CAP_TEST_TOP", root.data(), 1);
  CHECK(Directory("$CAP_TEST_TOP/data").path().equalTo(top.join("data")));   // expanded at construction

  // ---- Directory listings
  Directory data = top.directory("data");
  CHECK_EQUAL(joined(top.names()), "data empty notes.txt");
  FS::ListOptions tree;
  tree.depth = -1;
  CHECK_EQUAL(joined(top.names(tree)), "data data/deep data/deep/run3.cap data/run1.cap data/run2.cap data/x.root empty notes.txt");
  CHECK_EQUAL(joined(data.files(".cap")), joined({data.join("run1.cap"), data.join("run2.cap")}));
  CHECK_EQUAL(joined(data.files(".cap", -1)), joined({data.join("deep/run3.cap"), data.join("run1.cap"), data.join("run2.cap")}));
  CHECK_EQUAL(joined(data.files()), joined({data.join("run1.cap"), data.join("run2.cap"), data.join("x.root")}));
  {
    std::vector<Directory> subdirectories = top.subdirectories();
    CHECK(subdirectories.size()==2 && subdirectories[0].baseName().equalTo("data") && subdirectories[1].baseName().equalTo("empty"));
  }
  CHECK_EQUAL(joined(top.find("run?.cap")), joined({data.join("deep/run3.cap"), data.join("run1.cap"), data.join("run2.cap")}));
  CHECK_EQUAL(joined(top.find("run?.cap", 2)), joined({data.join("run1.cap"), data.join("run2.cap")}));
  CHECK(top.contains("notes.txt") && top.contains("data/deep") && !top.contains("nothing"));
  CHECK(top.directory("empty").list().empty());

  // ---- printing
  {
    std::ostringstream out;
    data.ls(out);
    CHECK_EQUAL(out.str(), "deep/\nrun1.cap  10 B\nrun2.cap  2.0 MB\nx.root    3 B\n");
  }
  {
    std::ostringstream out;
    top.tree(out);
    std::string expected = std::string(root.data()) + "/   5 files, 3 directories\n"
      "├── data/\n"
      "│   ├── deep/\n"
      "│   │   └── run3.cap  0 B\n"
      "│   ├── run1.cap  10 B\n"
      "│   ├── run2.cap  2.0 MB\n"
      "│   └── x.root  3 B\n"
      "├── empty/\n"
      "└── notes.txt  1.5 kB\n";
    CHECK_EQUAL(out.str(), expected);
    std::ostringstream shallow;
    top.tree(shallow, 1);
    CHECK(shallow.str().find("run1.cap")==std::string::npos && shallow.str().find("1 file, 2 directories")!=std::string::npos);
    CHECK_THROWS(missing.tree());
  }
  CHECK(Directory::readableSize(999).equalTo("999 B"));
  CHECK(Directory::readableSize(1500).equalTo("1.5 kB"));
  CHECK(Directory::readableSize(999999).equalTo("1.0 MB"));
  CHECK(Directory::readableSize(3200000000ULL).equalTo("3.2 GB"));

  // ---- changes
  Directory made = top.directory("made/a/b");
  made.create();
  CHECK(made.isDirectory());
  CHECK_THROWS(top.directory("made").remove());              // not empty
  CHECK(top.directory("made").remove(true)==3);
  CHECK(!top.contains("made"));
  CHECK(top.directory("empty").remove()==1);
  CHECK_THROWS(missing.remove());

  // ---- File as a FileSystemEntry
  {
    BinaryFile run1(data.join("run1.cap"));
    CHECK(run1.exists() && run1.isFile() && run1.fileSize()==10);
    CHECK(run1.baseName().equalTo("run1.cap") && run1.stem().equalTo("run1") && run1.extension().equalTo(".cap"));
    CHECK(run1.parentPath().equalTo(data.path()));
    CHECK(run1.absolutePath().equalTo(FS::normalize(data.join("run1.cap"))));
    CHECK(run1.info().size==10 && run1.type()==FS::EntryType::kFile);
    BinaryFile directory(data.path());
    CHECK(!directory.exists());                                // a File exists only as a regular file
    CHECK(directory.isDirectory());
    TxtFile notes(top.join("notes.txt"), File::kReadOnly);
    notes.open(File::kRead);
    CHECK_THROWS(notes.setPath(top.join("other.txt")));        // open: the path cannot change
    notes.close();
    notes.setPath(top.join("other.txt"));
    CHECK(notes.path().equalTo(top.join("other.txt")) && !notes.exists());
    FileSystemEntry & entry = run1;
    CHECK(entry.exists() && entry.className().equalTo("BinaryFile"));
  }

  top.remove(true);
  CHECK(!top.exists());
  std::printf("TestDirectory: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
