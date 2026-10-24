/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestLazyLoading.cpp
//! @brief Test program of the lazy loading of CAP files (run by ctest).
//
// TestLazyLoading -- tests of CapFile::load(File::Loading::kLazy): the tree read at once with placeholders, objects
// decoded on first use, loadAll(), checksums checked when the file is opened, changes of the file on disk, copies,
// and writing a lazily loaded folder.
//
// Works in a temporary directory, removed at the end. Prints each failed check and exits with the number of failures.
//
#include <cstdio>
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <algorithm>
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
  try { f(); } catch (const FileException &) { thrown = true; } catch (...) { }
  check(thrown, what + " should throw FileException", line);
}

long value(const Folder & folder, const char * path)
{
  const LongParameter * p = folder.get<LongParameter>(path);
  return p ? p->value() : -999;
}

std::string fileBytes(const String & path)
{
  std::ifstream in(path.data(), std::ios::binary);
  return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

void writeBytes(const String & path, const std::string & bytes)
{
  std::ofstream out(path.data(), std::ios::binary);
  out << bytes;
}
} // namespace

//! @brief Count a check, and report it if the condition is false.
#define CHECK(condition)        check((condition), #condition, __LINE__)
//! @brief Count a check, and report it if the statement does not throw.
#define CHECK_THROWS(statement) checkThrows([&]() { statement; }, #statement, __LINE__)

//! @brief Run the checks. @return number of failed checks (0: all passed)
int main()
{
  // ---- the checksum: standard CRC-32 values, whole or in pieces of any size
  {
    CHECK(crc32("123456789", 9)==0xCBF43926u);
    CHECK(crc32("", 0)==0);
    std::string text;
    for (int k=0; k<1000; k++) text += char(k*31 % 256);
    uint32_t reference = 0xFFFFFFFFu;
    for (char c : text) reference = crc32Update(reference, &c, 1);                    // byte by byte
    uint32_t pieces = 0xFFFFFFFFu;
    for (std::size_t at=0, piece=1; at<text.size(); at+=piece, piece=piece%13+1)
      pieces = crc32Update(pieces, text.data()+at, std::min(piece, text.size()-at));
    CHECK(crc32(text.data(), text.size())==(reference ^ 0xFFFFFFFFu) && pieces==reference);
  }

  char buffer[] = "/tmp/CapTestLazyLoading.XXXXXX";
  Directory top{String(mkdtemp(buffer))};
  String path = top.join("run.cap");
  // run: a=1, b=2, singles/{n=10, m=11}, singles/deep/{k=12}, empty/
  {
    CapFile out(path, File::kWriteOnly);
    out.setFolderName("run");
    out.add(new LongParameter("a", 1), "a");
    out.add(new LongParameter("b", 2), "b");
    Folder * singles = out.addFolder("singles");
    singles->add(new LongParameter("n", 10), "n");
    singles->add(new LongParameter("m", 11), "m");
    singles->addFolder("deep")->add(new LongParameter("k", 12), "k");
    out.addFolder("empty");
    out.save();
  }

  // ---- the tree at once, nothing decoded
  {
    CapFile in(path, File::kReadOnly);
    in.load(File::Loading::kLazy);
    CHECK(in.isLoaded() && in.isLazy());
    CHECK(in.folderName().equalTo("run") && in.size()==4);
    CHECK(in.nNotLoaded()==5);
    CHECK(in.paths().size()==5);
    CHECK(in.folder("singles/deep")!=nullptr && in.folder("empty")!=nullptr);
    CHECK(in.folder("singles")->parent()==&in.folder());
    CHECK(in.isLoaded("singles") && !in.isLoaded("a") && !in.isLoaded("singles/deep/k") && !in.isLoaded("nothing"));
    CHECK(in.folder().typeName("singles/n")=="CAP::LongParameter");    // known from the index
    CHECK(in.nNotLoaded()==5);                                           // ... without decoding

    // ---- decoded on first use, then kept
    const LongParameter * n = in.get<LongParameter>("singles/n");
    CHECK(n && n->value()==10);
    CHECK(in.isLoaded("singles/n") && in.nNotLoaded()==4);
    CHECK(in.get<LongParameter>("singles/n")==n);                       // same object
    CHECK(value(in, "singles/deep/k")==12 && in.nNotLoaded()==3);

    // ---- copies stay lazy, and work
    Folder copy(in.folder());
    CHECK(copy.nNotLoaded()==3 && value(copy, "a")==1 && copy.nNotLoaded()==2);
    CHECK(in.nNotLoaded()==3);                                           // the original is not affected

    // ---- release
    LongParameter * b = in.folder().release<LongParameter>("b");
    CHECK(b && b->value()==2 && !in.contains("b") && in.nNotLoaded()==2);
    delete b;

    // ---- loadAll
    in.loadAll();
    CHECK(in.nNotLoaded()==0 && value(in, "a")==1 && value(in, "singles/m")==11);
  }

  // ---- written elsewhere without decoding: the records are copied as they are
  {
    CapFile in(path, File::kReadWrite);
    in.load(File::Loading::kLazy);
    in.setPath(top.join("copy.cap"));
    in.write();
    CHECK(in.nNotLoaded()==5);
    CHECK(fileBytes(top.join("copy.cap"))==fileBytes(path));            // identical file
    CapFile reread(top.join("copy.cap"), File::kReadOnly);
    reread.load();
    CHECK(value(reread, "singles/deep/k")==12 && reread.nNotLoaded()==0);
  }

  // ---- written to the same file: everything is decoded first
  {
    CapFile in(path, File::kReadWrite);
    in.load(File::Loading::kLazy);
    in.add(new LongParameter("c", 3), "c");
    in.write();
    CHECK(in.nNotLoaded()==0);
    CapFile reread(path, File::kReadOnly);
    reread.load(File::Loading::kLazy);
    CHECK(value(reread, "c")==3 && value(reread, "singles/n")==10 && value(reread, "a")==1);
  }

  // ---- the file changes on disk: objects not decoded can no longer be
  {
    CapFile in(path, File::kReadOnly);
    in.load(File::Loading::kLazy);
    CHECK(value(in, "a")==1);
    std::string bytes = fileBytes(path);
    writeBytes(path, bytes + bytes);                                    // the same folder, twice
    CHECK_THROWS(in.get<LongParameter>("singles/n"));
    CHECK(value(in, "a")==1);                                           // decoded objects stay
    writeBytes(path, bytes);
  }

  // ---- checksums checked when the file is opened; the folder held is unchanged on failure
  {
    std::string bytes = fileBytes(path);
    std::size_t at = bytes.find("singles");                            // inside the payload of a nested record
    std::string corrupted = bytes;
    corrupted[at + 20] ^= 0x01;
    writeBytes(top.join("corrupted.cap"), corrupted);
    writeBytes(top.join("truncated.cap"), bytes.substr(0, bytes.size() - 3));
    writeBytes(top.join("text.cap"), "not a CAP file");
    writeBytes(top.join("empty.cap"), "");
    for (const char * name : {"corrupted.cap", "truncated.cap", "text.cap", "empty.cap"})
      {
      CapFile file(top.join(name), File::kReadOnly);
      file.setFolderName("kept");
      bool thrown = false;
      try { file.load(File::Loading::kLazy); } catch (const FileException &) { thrown = true; }
      check(thrown && file.folderName().equalTo("kept") && !file.isLoaded(), std::string(name) + " refused, folder unchanged", __LINE__);
      }
    CapFile missing(top.join("missing.cap"), File::kReadOnly);
    CHECK_THROWS(missing.load(File::Loading::kLazy));
  }

  // ---- eager after lazy, unload
  {
    CapFile in(path, File::kReadOnly);
    in.load(File::Loading::kLazy);
    in.load();
    CHECK(!in.isLazy() && in.nNotLoaded()==0 && value(in, "singles/deep/k")==12);
    in.unload();
    CHECK(!in.isLoaded() && in.empty());
  }

  top.remove(true);
  std::printf("TestLazyLoading: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
