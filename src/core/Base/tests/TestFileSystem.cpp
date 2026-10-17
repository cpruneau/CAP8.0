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
//! @file TestFileSystem.cpp
//! @brief Test program of the FileSystem functions (run by ctest).
//
// TestFileSystem -- tests of the CAP::FileSystem functions and of File::exists() / File::fileSize().
//
// Builds a small tree in a temporary directory, checks every function (including the errors), then removes the tree.
// Prints each failed check and exits with the number of failures (0: all passed). Run with ctest, or directly.
//
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <functional>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <unistd.h>
#include "FileSystem.hpp"
#include "BinaryFile.hpp"

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

void checkEqual(const String & value, const String & expected, const std::string & what, int line)
{
  check(value.equalTo(expected), what + ": got \"" + value.data() + "\", expected \"" + expected.data() + "\"", line);
}

void checkThrows(const std::function<void()> & f, const std::string & what, int line)
{
  bool thrown = false;
  try { f(); } catch (const FileException &) { thrown = true; } catch (...) { }
  check(thrown, what + " should throw FileException", line);
}

void checkPaths(const std::vector<String> & paths, const std::vector<String> & expected, const std::string & what, int line)
{
  bool ok = paths.size()==expected.size();
  for (std::size_t k=0; ok && k<paths.size(); k++) ok = paths[k].equalTo(expected[k]);
  std::string got;
  for (const auto & p : paths) got += std::string(" ") + p.data();
  check(ok, what + ": got" + got, line);
}

std::vector<String> pathsOf(const std::vector<FS::EntryInfo> & entries, const String & prefix)
{
  std::vector<String> paths;
  for (const auto & e : entries) paths.push_back(String(std::string(e.path.data()).substr(prefix.length()+1)));
  return paths;
}

void writeFile(const String & path, const std::string & content)
{
  std::ofstream out(path.data(), std::ios::binary);
  out << content;
}
} // namespace

//! @brief Count a check, and report it if the condition is false.
#define CHECK(condition)            check((condition), #condition, __LINE__)
//! @brief Count a check, and report it with both texts if the value differs from the expected one.
#define CHECK_EQUAL(value, expected) checkEqual((value), (expected), #value, __LINE__)
//! @brief Count a check, and report it if the statement does not throw.
#define CHECK_THROWS(statement)     checkThrows([&]() { statement; }, #statement, __LINE__)
//! @brief Count a check, and report it if the paths differ from the expected ones.
#define CHECK_PATHS(paths, ...)     checkPaths((paths), std::vector<String>{__VA_ARGS__}, #paths, __LINE__)

//! @brief Run the checks. @return number of failed checks (0: all passed)
int main()
{
  // ---- a temporary tree
  //   root/
  //     a.cap (10 bytes)  b.CAP (3)  notes.txt (5)  .hidden (1)
  //     data/  run1.cap  run2.cap  run10.cap  x.root
  //       deep/  run3.cap
  //     .config/  settings.xml
  //     link -> data
  char buffer[] = "/tmp/CapTestFileSystem.XXXXXX";
  String root(mkdtemp(buffer));
  auto at = [&](const char * relative) { return FS::join(root, relative); };
  writeFile(at("a.cap"), "0123456789");
  writeFile(at("b.CAP"), "abc");
  writeFile(at("notes.txt"), "notes");
  writeFile(at(".hidden"), "h");
  FS::createDirectory(at("data/deep"));
  FS::createDirectory(at(".config"));
  for (const char * f : {"data/run1.cap", "data/run2.cap", "data/run10.cap", "data/x.root", "data/deep/run3.cap", ".config/settings.xml"})
    writeFile(at(f), "x");
  CHECK(symlink("data", at("link").data())==0);

  // ---- information
  CHECK(FS::exists(at("a.cap")));
  CHECK(FS::exists(at("data")));
  CHECK(!FS::exists(at("missing")));
  CHECK(!FS::exists(""));
  CHECK(FS::isFile(at("a.cap")));
  CHECK(!FS::isFile(at("data")));
  CHECK(FS::isDirectory(at("data")));
  CHECK(FS::isDirectory(at("link")));        // follows the link
  CHECK(FS::isLink(at("link")));
  CHECK(!FS::isLink(at("data")));
  CHECK(FS::size(at("a.cap"))==10);
  CHECK_THROWS(FS::size(at("data")));
  CHECK_THROWS(FS::size(at("missing")));
  CHECK(FS::isReadable(at("a.cap")) && FS::isWritable(at("a.cap")));
  CHECK(!FS::isReadable(at("missing")));
  CHECK(FS::isWritable(at("new.cap")));      // missing, in a writable directory
  CHECK(!FS::isWritable(at("nodir/new.cap")));
  CHECK(FS::isExecutable(at("data")));
  CHECK(std::labs(long(FS::modified(at("a.cap")) - std::time(nullptr))) < 120);
  CHECK_THROWS(FS::modified(at("missing")));
  {
    FS::EntryInfo info = FS::info(at("link"));
    CHECK(info.isLink && info.isDirectory() && info.name.equalTo("link"));
    FS::EntryInfo file = FS::info(at("a.cap"));
    CHECK(file.isFile() && file.size==10 && file.readable && !file.hidden);
    CHECK(FS::info(at(".hidden")).hidden);
    CHECK_THROWS(FS::info(at("missing")));
  }
  if (getuid()!=0)   // permissions do not apply to root
    {
    writeFile(at("locked"), "secret");
    chmod(at("locked").data(), 0);
    CHECK(FS::exists(at("locked")) && !FS::isReadable(at("locked")) && !FS::isWritable(at("locked")));
    chmod(at("locked").data(), 0600);
    FS::remove(at("locked"));
    }

  // ---- paths
  CHECK_EQUAL(FS::join("a", "b"), "a/b");
  CHECK_EQUAL(FS::join("a/", "b"), "a/b");
  CHECK_EQUAL(FS::join("a", "/abs/b"), "/abs/b");
  CHECK_EQUAL(FS::join("", "b"), "b");
  CHECK_EQUAL(FS::join("a", ""), "a");
  CHECK_EQUAL(FS::join("/", "b"), "/b");
  CHECK_EQUAL(FS::join("dir", "run42", ".cap"), "dir/run42.cap");
  CHECK_EQUAL(FS::join("dir", "run42", "cap"), "dir/run42.cap");
  CHECK_EQUAL(FS::join("dir", "run42.CAP", ".cap"), "dir/run42.CAP");
  CHECK_EQUAL(FS::join("dir", "/abs/run42", ".cap"), "/abs/run42.cap");
  CHECK_EQUAL(FS::parent("a/b/c.cap"), "a/b");
  CHECK_EQUAL(FS::parent("a/b/"), "a");
  CHECK_EQUAL(FS::parent("c.cap"), "");
  CHECK_EQUAL(FS::parent("/c"), "/");
  CHECK_EQUAL(FS::name("a/b/c.cap"), "c.cap");
  CHECK_EQUAL(FS::name("a/b/c.cap/"), "c.cap");
  CHECK_EQUAL(FS::name("/"), "/");
  CHECK_EQUAL(FS::stem("a/b/c.tar.gz"), "c.tar");
  CHECK_EQUAL(FS::stem(".bashrc"), ".bashrc");
  CHECK_EQUAL(FS::extension("a/b/c.cap"), ".cap");
  CHECK_EQUAL(FS::extension(".bashrc"), "");
  CHECK_EQUAL(FS::extension("a.b/c"), "");
  CHECK(FS::hasExtension("run.CAP", ".cap") && FS::hasExtension("run.cap", "cap") && !FS::hasExtension("run.root", ".cap"));
  CHECK(!FS::hasExtension("run.cap", ""));
  CHECK_EQUAL(FS::withExtension("a/run.root", ".cap"), "a/run.cap");
  CHECK_EQUAL(FS::withExtension("a/run", "cap"), "a/run.cap");
  CHECK_EQUAL(FS::withExtension("a/run.cap", ""), "a/run");
  CHECK_EQUAL(FS::normalize(FS::join(root, "data/deep/../run1.cap")), FS::normalize(at("data/run1.cap")));
  CHECK(!FS::normalize(at("data/")).endsWith("/"));
  CHECK(FS::normalize("relative").beginsWith("/"));
  setenv("CAP_TEST_DIR", "/test/dir", 1);
  unsetenv("CAP_TEST_UNDEFINED");
  CHECK_EQUAL(FS::expand("$CAP_TEST_DIR/run.cap"), "/test/dir/run.cap");
  CHECK_EQUAL(FS::expand("${CAP_TEST_DIR}x/run.cap"), "/test/dirx/run.cap");
  CHECK_EQUAL(FS::expand("$CAP_TEST_UNDEFINED/run.cap"), "$CAP_TEST_UNDEFINED/run.cap");
  CHECK_EQUAL(FS::expand("~/x"), FS::join(FS::home(), "x"));
  CHECK_EQUAL(FS::expand("a~b"), "a~b");
  CHECK_EQUAL(FS::resolve("/abs/x", "CAP_TEST_DIR"), "/abs/x");
  CHECK_EQUAL(FS::resolve("sub/x", "CAP_TEST_DIR"), "/test/dir/sub/x");
  CHECK_EQUAL(FS::resolve("./x", "CAP_TEST_DIR"), FS::join(FS::current(), "x"));
  CHECK_EQUAL(FS::resolve("$CAP_TEST_DIR/y", "CAP_TEST_UNDEFINED"), "/test/dir/y");
  CHECK_THROWS(FS::resolve("sub/x", "CAP_TEST_UNDEFINED"));
  CHECK_EQUAL(FS::configuredPath("DEFAULT", "CAP_TEST_DIR"), "/test/dir");
  CHECK_EQUAL(FS::configuredPath("", "CAP_TEST_DIR"), "/test/dir");
  CHECK_EQUAL(FS::configuredPath("sub/x", "CAP_TEST_DIR"), "/test/dir/sub/x");
  CHECK_EQUAL(FS::configuredPath("/abs/x", "CAP_TEST_DIR"), "/abs/x");
  CHECK_EQUAL(FS::configuredPath("./x", "CAP_TEST_DIR"), FS::join(FS::current(), "x"));
  CHECK_EQUAL(FS::configuredPath("DEFAULT", "CAP_TEST_UNDEFINED"), "");            // never throws
  CHECK_EQUAL(FS::configuredPath("sub/x", "CAP_TEST_UNDEFINED"), "sub/x");
  CHECK_EQUAL(FS::firstExisting({"", at("missing"), at("notes.txt"), at("a.cap")}), at("notes.txt"));
  CHECK_EQUAL(FS::firstExisting({at("missing")}), "");

  // ---- listing
  {
    FS::ListOptions options;
    CHECK_PATHS(pathsOf(FS::list(root, options), root), "data", "link", "a.cap", "b.CAP", "notes.txt");
    options.hidden = true;
    CHECK_PATHS(pathsOf(FS::list(root, options), root), ".config", "data", "link", ".hidden", "a.cap", "b.CAP", "notes.txt");
    options = FS::ListOptions();
    options.directories = false;
    options.extensions = {".cap"};
    CHECK_PATHS(pathsOf(FS::list(root, options), root), "a.cap", "b.CAP");
    options.depth = -1;
    CHECK_PATHS(pathsOf(FS::list(root, options), root),
                "data/deep/run3.cap", "data/run1.cap", "data/run10.cap", "data/run2.cap", "a.cap", "b.CAP");   // links not followed
    options.followLinks = true;
    options.depth = 2;
    CHECK_PATHS(pathsOf(FS::list(root, options), root),
                "data/run1.cap", "data/run10.cap", "data/run2.cap", "link/run1.cap", "link/run10.cap", "link/run2.cap", "a.cap", "b.CAP");
    options = FS::ListOptions();
    options.depth = -1;
    options.files = false;
    CHECK_PATHS(pathsOf(FS::list(root, options), root), "data", "data/deep", "link");
    options = FS::ListOptions();
    options.depth = -1;
    options.include = {"run?.cap"};
    options.exclude = {"deep"};
    options.directories = false;
    CHECK_PATHS(pathsOf(FS::list(root, options), root), "data/run1.cap", "data/run2.cap");
    options = FS::ListOptions();
    options.order = FS::ListOptions::kName;
    CHECK_PATHS(pathsOf(FS::list(root, options), root), "a.cap", "b.CAP", "data", "link", "notes.txt");
    options.order = FS::ListOptions::kSize;
    options.directories = false;
    CHECK_PATHS(pathsOf(FS::list(root, options), root), "a.cap", "notes.txt", "b.CAP");
    std::vector<FS::EntryInfo> entries = FS::list(at("data"));
    CHECK(!entries.empty() && entries[0].path.equalTo(at("data/deep")) && entries[0].isDirectory());
    CHECK_THROWS(FS::list(at("a.cap")));
    CHECK_THROWS(FS::list(at("missing")));
  }

  // ---- globbing
  CHECK(FS::matches("run1.cap", "run*.cap") && FS::matches("run1.cap", "run?.cap") && !FS::matches("run10.cap", "run?.cap"));
  CHECK(FS::matches("run1.cap", "run[0-9].cap") && !FS::matches("runx.cap", "run[0-9].cap"));
  CHECK(FS::matches("runx.cap", "run[!0-9].cap") && FS::matches("a", "*") && FS::matches("", "*") && !FS::matches("a", ""));
  CHECK(FS::matches("ab", "[ab]b") && FS::matches("[x", "[x") && FS::matches("a]", "a]"));
  CHECK_PATHS(FS::glob(FS::join(root, "data/run*.cap")), at("data/run1.cap"), at("data/run10.cap"), at("data/run2.cap"));
  CHECK_PATHS(FS::glob(FS::join(root, "data/run?.cap")), at("data/run1.cap"), at("data/run2.cap"));
  CHECK_PATHS(FS::glob(FS::join(root, "*/run1.cap")), at("data/run1.cap"), at("link/run1.cap"));
  CHECK_PATHS(FS::glob(FS::join(root, "**/run3.cap")), at("data/deep/run3.cap"));
  CHECK_PATHS(FS::glob(FS::join(root, "data/**/*.cap")),
              at("data/deep/run3.cap"), at("data/run1.cap"), at("data/run10.cap"), at("data/run2.cap"));
  CHECK_PATHS(FS::glob(FS::join(root, "*.xml")));
  CHECK_PATHS(FS::glob(FS::join(root, ".*/*.xml")), at(".config/settings.xml"));
  CHECK_PATHS(FS::glob(at("notes.txt")), at("notes.txt"));
  CHECK_PATHS(FS::glob(at("missing")));
  setenv("CAP_TEST_ROOT", root.data(), 1);
  CHECK_PATHS(FS::glob("$CAP_TEST_ROOT/data/deep/*"), at("data/deep/run3.cap"));
  {
    String here = FS::current();
    CHECK(chdir(root.data())==0);
    CHECK_PATHS(FS::glob("data/run1*"), "data/run1.cap", "data/run10.cap");
    CHECK_PATHS(FS::glob("*.txt"), "notes.txt");
    CHECK(chdir(here.data())==0);
  }

  // ---- changes
  FS::createDirectory(at("made/a/b"));
  CHECK(FS::isDirectory(at("made/a/b")));
  FS::createDirectory(at("made/a/b"));                      // exists: nothing
  CHECK_THROWS(FS::createDirectory(at("a.cap")));
  CHECK_THROWS(FS::createDirectory(at("nothere/x"), false));
  CHECK_THROWS(FS::remove(at("made")));                     // not empty
  FS::remove(at("made/a/b"));
  CHECK(!FS::exists(at("made/a/b")));
  CHECK_THROWS(FS::remove(at("missing")));
  FS::rename(at("notes.txt"), at("made/notes.txt"));
  CHECK(!FS::exists(at("notes.txt")) && FS::isFile(at("made/notes.txt")));
  CHECK_THROWS(FS::rename(at("missing"), at("x")));
  FS::copy(at("a.cap"), at("copy.cap"));
  CHECK(FS::size(at("copy.cap"))==10);
  CHECK_THROWS(FS::copy(at("b.CAP"), at("copy.cap")));      // exists
  FS::copy(at("b.CAP"), at("copy.cap"), true);
  CHECK(FS::size(at("copy.cap"))==3);
  FS::copy(at("data"), at("dataCopy"));
  CHECK(FS::isFile(at("dataCopy/deep/run3.cap")));
  CHECK_THROWS(FS::copy(at("missing"), at("x")));
  CHECK(FS::removeTree(at("dataCopy"))==7);                 // the directory, deep/, 5 files
  CHECK(!FS::exists(at("dataCopy")));
  CHECK_THROWS(FS::removeTree("/"));
  CHECK_THROWS(FS::removeTree(FS::home()));
  CHECK_THROWS(FS::removeTree(at("missing")));

  // ---- File uses the functions
  {
    BinaryFile file(at("a.cap"));
    CHECK(file.exists() && file.fileSize()==10);
    BinaryFile missing(at("missing.cap"));
    CHECK(!missing.exists());
    CHECK_THROWS(missing.fileSize());
    CHECK(!File::exists(at("data")));
  }

  FS::removeTree(root);
  CHECK(!FS::exists(root));
  std::printf("TestFileSystem: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
