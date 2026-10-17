//
// TestFileLoading -- tests of load(), unload(), isLoaded() and save() for TxtFile, XmlFile, BinaryFile and CapFile,
// of BinaryFile::readRange(), and of the moves of Folder used when reading a CapFile.
//
// Works in a temporary directory, removed at the end. Prints each failed check and exits with the number of failures.
//
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <string>
#include <utility>
#include <unistd.h>
#include "BinaryFile.hpp"
#include "CapFile.hpp"
#include "Directory.hpp"
#include "LongParameter.hpp"
#include "TxtFile.hpp"
#include "XmlFile.hpp"

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

template <typename E>
void checkThrows(const std::function<void()> & f, const std::string & what, int line)
{
  bool thrown = false;
  try { f(); } catch (const E &) { thrown = true; } catch (...) { }
  check(thrown, what + " should throw", line);
}

long parameter(const CapFile & file, const char * path)
{
  const LongParameter * p = file.get<LongParameter>(path);
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

#define CHECK(condition)                  check((condition), #condition, __LINE__)
#define CHECK_THROWS(Exception, statement) checkThrows<Exception>([&]() { statement; }, #statement, __LINE__)

int main()
{
  char buffer[] = "/tmp/CapTestFileLoading.XXXXXX";
  Directory top{String(mkdtemp(buffer))};

  // ---- TxtFile
  {
    TxtFile out(top.join("notes.txt"));
    CHECK(!out.isLoaded());
    out.addLine("first");
    out.addLine("second");
    out.save();
    CHECK(out.isLoaded() && FS::size(out.path())==13);
    TxtFile in(top.join("notes.txt"), File::kReadOnly);
    in.load();
    CHECK(in.isLoaded() && in.nLines()==2 && in.line(1).equalTo("second"));
    in.unload();
    CHECK(!in.isLoaded() && in.nLines()==0);
    in.load(File::Loading::kLazy);                     // no lazy mode for text: everything
    CHECK(in.isLoaded() && in.nLines()==2);
    in.setPath(top.join("other.txt"));
    CHECK(!in.isLoaded() && in.nLines()==2);           // lines kept, no longer the content of the file
    CHECK_THROWS(FileException, in.load());            // missing
    TxtFile readOnly(top.join("notes.txt"), File::kReadOnly);
    CHECK_THROWS(FileException, readOnly.save());
  }

  // ---- XmlFile
  {
    XmlFile out(top.join("run.xml"));
    out.setDeclaration();
    out.addLine("<run>");
    out.addElement("energy", "13.6 TeV", 1);
    out.addLine("</run>");
    out.save();
    XmlFile in(top.join("run.xml"), File::kReadOnly);
    in.load();
    CHECK(in.isLoaded());
    const XmlDocument & first = in.document();
    const XmlDocument & second = in.document();
    CHECK(&first==&second);                            // parsed once
    CHECK(first.root().isNamed("run"));
    in.setLine(1, "<events>");                         // the lines change: parsed again
    in.setLine(3, "</events>");
    CHECK(in.document().root().isNamed("events"));
    in.unload();
    CHECK(!in.isLoaded() && in.nLines()==0);
    writeBytes(top.join("bad.xml"), "<run>\n<energy>\n</run>\n");
    XmlFile bad(top.join("bad.xml"), File::kReadOnly);
    CHECK_THROWS(XmlException, bad.load());
    CHECK(bad.isLoaded() && bad.nLines()==3);          // the lines are loaded, to be corrected
    CHECK(!bad.isWellFormed());
  }

  // ---- BinaryFile
  {
    std::string content;
    for (int k=0; k<1000; k++) content += char(k % 251);
    writeBytes(top.join("data.bin"), content);
    BinaryFile file(top.join("data.bin"));
    file.load();
    CHECK(file.isLoaded() && !file.isLazy() && file.bytes()==content);
    file.unload();
    CHECK(!file.isLoaded() && file.bytes().empty());
    file.load(File::Loading::kLazy);
    CHECK(file.isLoaded() && file.isLazy() && file.bytes().empty());
    CHECK(file.readRange(10, 5)==content.substr(10, 5));
    CHECK(file.readRange(995, 100)==content.substr(995));   // fewer at the end
    CHECK(file.readRange(1000, 10).empty());
    CHECK_THROWS(FileException, file.readRange(1001, 1));
    CHECK_THROWS(FileException, file.save());               // lazy: nothing held
    file.setBytes("abc");
    file.save();
    CHECK(fileBytes(file.path())=="abc" && file.isLoaded() && !file.isLazy());
    BinaryFile missing(top.join("missing.bin"));
    CHECK_THROWS(FileException, missing.load());
    CHECK_THROWS(FileException, missing.load(File::Loading::kLazy));
  }

  // ---- Folder moves
  {
    Folder source("source");
    source.addFolder("sub")->add(new LongParameter("n", 7), "n");
    Folder target(std::move(source));
    CHECK(source.empty() && target.size()==1 && target.name().equalTo("source"));
    CHECK(target.folder("sub")->parent()==&target);
    CHECK(target.get<LongParameter>("sub/n")->value()==7);
    Folder outer("outer");
    Folder * inner = outer.addFolder("inner");
    *inner = std::move(target);
    CHECK(inner->parent()==&outer);                    // the parent of the target is kept
    CHECK(inner->folder("sub")->parent()==inner);
    CHECK(outer.get<LongParameter>("inner/sub/n")->value()==7);
    CHECK(target.empty());
  }

  // ---- CapFile
  {
    String path = top.join("run42.cap");
    {
      CapFile out(path, File::kWriteOnly);
      out.setFolderName("run42");
      out.addFolder("singles")->add(new LongParameter("n", 7), "n");
      out.exportParameter("events", 1000);
      CHECK(!out.isLoaded());
      out.save();
      CHECK(out.isLoaded());
      out.setFolderName("run43");
      out.exportParameter("events", 2000);
      out.append();                                     // a second folder in the same file
    }
    CapFile in(path, File::kReadOnly);
    in.load();
    CHECK(in.isLoaded() && in.folderName().equalTo("run42") && parameter(in, "events")==1000 && parameter(in, "singles/n")==7);
    CHECK(in.folder("singles")->parent()==&in.folder());
    in.read("run43");
    CHECK(in.folderName().equalTo("run43") && parameter(in, "events")==2000);
    CHECK_THROWS(FileException, in.read("run44"));
    CHECK(in.folderName().equalTo("run43"));            // unchanged after a failure
    in.unload();
    CHECK(!in.isLoaded() && in.empty() && in.folderName().equalTo(""));
    in.load(File::Loading::kLazy);                      // not available yet: everything
    CHECK(in.isLoaded() && parameter(in, "singles/n")==7);

    // corrupted, truncated, not a CAP file, no folder: refused, and the folder held is unchanged
    std::string bytes = fileBytes(path);
    std::string corrupted = bytes;
    corrupted[bytes.size()/4] ^= 0x55;
    writeBytes(top.join("corrupted.cap"), corrupted);
    writeBytes(top.join("truncated.cap"), bytes.substr(0, 20));          // inside the first record
    writeBytes(top.join("text.cap"), "not a CAP file");
    writeBytes(top.join("empty.cap"), "");
    for (const char * name : {"corrupted.cap", "truncated.cap", "text.cap", "empty.cap"})
      {
      CapFile file(top.join(name), File::kReadOnly);
      file.setFolderName("kept");
      bool thrown = false;
      try { file.load(); } catch (const FileException &) { thrown = true; }
      check(thrown && file.folderName().equalTo("kept") && !file.isLoaded(), std::string(name) + " refused, folder unchanged", __LINE__);
      }
  }

  top.remove(true);
  std::printf("TestFileLoading: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
