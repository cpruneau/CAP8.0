/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestFolderEditing.cpp
//! @brief Tests of the editing of folders: copies of entries (objects and folders, from memory and from lazily loaded
//! files), unique names, renaming, taking entries out, and saving an edited tree to a new file.
//
// Works in a temporary directory. Prints each failed check and exits with the number of failures.
#include <cstdio>
#include <functional>
#include <memory>
#include <string>
#include <unistd.h>
#include "CapFile.hpp"
#include "Directory.hpp"
#include "LongParameter.hpp"

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

void checkThrows(const std::function<void()> & f, const std::string & what, int line)
{
  bool thrown = false;
  try { f(); } catch (...) { thrown = true; }
  check(thrown, what + " should throw", line);
}

long value(const Folder & folder, const char * path)
{
  const LongParameter * p = folder.get<LongParameter>(path);
  return p ? p->value() : -999;
}
} // namespace

#define CHECK(condition)        check((condition), #condition, __LINE__)
#define CHECK_THROWS(statement) checkThrows([&]() { statement; }, #statement, __LINE__)

int main()
{
  // ---- copies in memory: objects and folders, deep
  {
    Folder source("source");
    source.add(new LongParameter("a", 1), "a");
    Folder * sub = source.addFolder("sub");
    sub->add(new LongParameter("n", 10), "n");
    sub->addFolder("deep")->add(new LongParameter("k", 12), "k");

    Folder target("target");
    CHECK(target.copyEntry(source, "a")=="a");
    CHECK(target.copyEntry(source, "sub/n", "n2")=="n2");
    CHECK(target.copyEntry(source, "sub")=="sub");
    CHECK(value(target, "a")==1 && value(target, "n2")==10 && value(target, "sub/deep/k")==12);
    CHECK(target.folder("sub")->parent()==&target);
    // deep: changing the source does not change the copies
    source.get<LongParameter>("sub/deep/k")->setValue(99);
    source.remove("a");
    CHECK(value(target, "sub/deep/k")==12 && value(target, "a")==1);
    // into the current folder
    target.cd("sub");
    CHECK(target.copyEntry(source, "sub/n", "again")=="again");
    target.cd("/");
    CHECK(value(target, "sub/again")==10);
    CHECK_THROWS(target.copyEntry(source, "a"));                    // no longer in the source
    CHECK_THROWS(target.copyEntry(source, "sub/n", "a"));           // name taken
    CHECK_THROWS(target.copyEntry(source, "sub/n", "x/y"));         // invalid name
  }

  // ---- unique names, renaming, taking entries out
  {
    Folder f("f");
    f.add(new LongParameter("h", 1), "h");
    f.add(new LongParameter("h_2", 2), "h_2");
    CHECK(f.uniqueName("g")=="g");
    CHECK(f.uniqueName("h")=="h_3");
    f.addFolder("dir")->add(new LongParameter("x", 5), "x");
    f.rename("h", "renamed");
    CHECK(!f.contains("h") && value(f, "renamed")==1);
    f.rename("dir", "folder");
    CHECK(f.folder("folder")!=nullptr && f.folder("folder")->name()=="folder" && value(f, "folder/x")==5);
    f.rename("folder/x", "y");
    CHECK(value(f, "folder/y")==5);
    CHECK_THROWS(f.rename("renamed", "h_2"));                        // taken
    CHECK_THROWS(f.rename("missing", "z"));
    CHECK_THROWS(f.rename("renamed", "a/b"));
    // taken out: the object stays alive, owned by the caller
    const LongParameter * shown = f.get<LongParameter>("renamed");
    std::unique_ptr<FolderItem> item = f.take("renamed");
    CHECK(item!=nullptr && !f.contains("renamed") && shown->value()==1);
    std::unique_ptr<FolderItem> folder = f.take("folder");
    CHECK(folder!=nullptr && folder->asFolder()->parent()==nullptr && value(*folder->asFolder(), "y")==5);
    CHECK(f.take("missing")==nullptr);
  }

  // ---- copies from a lazily loaded file, then saved to a new file
  char buffer[] = "/tmp/CapTestFolderEditing.XXXXXX";
  Directory top{String(mkdtemp(buffer))};
  String inputPath = top.join("input.cap");
  {
    CapFile out(inputPath, File::kWriteOnly);
    out.setFolderName("run");
    out.add(new LongParameter("a", 1), "a");
    Folder * singles = out.addFolder("singles");
    singles->add(new LongParameter("n", 10), "n");
    singles->addFolder("deep")->add(new LongParameter("k", 12), "k");
    out.save();
  }
  String outputPath = top.join("edited.cap");
  {
    CapFile input(inputPath, File::kReadOnly);
    input.load(File::Loading::kLazy);
    CHECK(!input.isLoaded("singles/n"));
    Folder tree("edited");
    tree.addFolder("copies");
    tree.cd("copies");
    tree.copyEntry(input, "singles");                                // a folder of entries not decoded yet
    tree.copyEntry(input, "a", "first");
    tree.cd("/");
    CHECK(tree.isLoaded("copies/singles/n") && tree.isLoaded("copies/first"));   // decoded: independent of the file
    CapFile output(outputPath, File::kWriteOnly);
    output = tree;
    output.write();
  }
  {
    CapFile reread(outputPath, File::kReadOnly);
    reread.read();
    CHECK(value(reread, "copies/first")==1);
    CHECK(value(reread, "copies/singles/n")==10);
    CHECK(value(reread, "copies/singles/deep/k")==12);
  }
  top.remove(true);

  std::printf("TestFolderEditing: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
