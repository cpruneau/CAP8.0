/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "CapFile.hpp"
#include "LongParameter.hpp"
#include "PrintHelpers.hpp"
#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <sstream>
#include <utility>

ClassImp(CAP::CapFile);

namespace CAP
{

  CapFile::CapFile(const String & path, Access access)
  :
  BinaryFile(path,access),
  _folder(""),
  _source()
  {
  setClassName("CapFile");
  }

  CapFile & CapFile::operator=(const Folder & folder)
  {
  if (&_folder!=&folder) _folder = folder;
  return *this;
  }

  bool CapFile::readFolder(const String * folderName, Folder & folder)
  {
  open(kRead);
  try
    {
    std::string tag, payload;
    unsigned int version;
    while (readBinaryRecord(_stream,tag,version,payload,_path))
      {
      if (tag!=Folder::binaryTag()) continue;
      if (folderName && !binaryPayloadName(payload).equalTo(*folderName)) continue;
      decodeBinaryPayload(payload,version,folder,_path);
      close();
      return true;
      }
    }
  catch (...)
    {
    close();
    throw;
    }
  close();
  return false;
  }

  void CapFile::read()
  {
  // decoded into a separate folder first, so that the folder held is unchanged if anything fails
  Folder loaded;
  if (!readFolder(nullptr,loaded)) throw FileException(_path,"No folder in the file","CapFile::read()");
  _folder = std::move(loaded);
  _source.reset();
  _lazy = false;
  _loaded = true;
  }

  void CapFile::read(const String & folderName)
  {
  Folder loaded;
  if (!readFolder(&folderName,loaded))
    throw FileException(_path,String("No folder named ")+folderName+" in the file","CapFile::read()");
  _folder = std::move(loaded);
  _source.reset();
  _lazy = false;
  _loaded = true;
  }

  void CapFile::load(Loading loading)
  {
  if (loading==Loading::kEager) { read(); return; }
  if (!FileSystem::isFile(_path)) throw FileException(_path,"File does not exist","CapFile::load()");
  auto source = std::make_shared<LazySource>();
  source->path     = FileSystem::normalize(_path);
  source->size     = FileSystem::size(_path);
  source->modified = FileSystem::modified(_path);
  Folder loaded;
  open(kRead);
  bool found;
  try { found = loaded.scanBinary(_stream,source); }
  catch (...) { close(); throw; }
  close();
  if (!found) throw FileException(_path,"No folder in the file","CapFile::load()");
  _folder = std::move(loaded);
  _source = source;
  _lazy = true;
  _loaded = true;
  }

  void CapFile::unload()
  {
  _folder = Folder();
  _source.reset();
  _lazy = false;
  _loaded = false;
  }

  void CapFile::loadAllBeforeWriting()
  {
  if (_source && _source->path.equalTo(FileSystem::normalize(_path)) && _folder.nNotLoaded()>0) _folder.loadAll();
  }

  void CapFile::writeFolder(Mode mode, bool skipUnsavable)
  {
  // Encode the whole folder first: the file is not touched if an object cannot be saved.
  loadAllBeforeWriting();
  std::ostringstream out;
  _folder.writeBinary(out,skipUnsavable);
  if (mode==kWrite) writeAll(out.str());
  else              appendAll(out.str());
  }

  void CapFile::write(bool skipUnsavable)
  {
  writeFolder(kWrite,skipUnsavable);
  _loaded = true;
  }

  void CapFile::append(bool skipUnsavable)
  {
  writeFolder(kAppend,skipUnsavable);
  }

  void CapFile::print() const
  {
  BinaryFile::print();
  _folder.print();
  }

  // ---------------------------------------------------------------- opening files

  namespace
  {
  enum class OpenOption { kOld, kNew, kRecreate, kUpdate };

  OpenOption parseOpenOption(const String & ioOption, const String & fileName)
  {
  std::string option(ioOption.data());
  std::transform(option.begin(),option.end(),option.begin(),[](unsigned char c){ return char(std::toupper(c)); });
  if (option=="READ" || option=="OLD" || option.empty()) return OpenOption::kOld;
  if (option=="NEW"  || option=="CREATE")                return OpenOption::kNew;
  if (option=="RECREATE")                                return OpenOption::kRecreate;
  if (option=="UPDATE")                                  return OpenOption::kUpdate;
  throw FileException(fileName,String("Unknown ioOption: ")+ioOption,"CapFile::openCapFile()");
  }

  String accountantEntryName(const Accountant & accountant, const std::string & suffix)
  {
  String entry = "Accountant_";
  entry += accountant.name();
  entry += "_";
  entry += suffix.c_str();
  return entry;
  }
  } // namespace

  String CapFile::makeFileName(const String & path, const String & name)
  {
  return FileSystem::join(path,name,".cap");
  }

  CapFile * CapFile::openCapFile(const String & path, const String & name, const String & ioOption, bool verbose)
  {
  String fileName = makeFileName(path,name);
  OpenOption option = parseOpenOption(ioOption,fileName);
  if (verbose)
    {
    printCR();
    printValue("CapFile::openCapFile(...) Opening file",fileName.data());
    printValue("ioOption",ioOption.data());
    }
  std::unique_ptr<CapFile> file(new CapFile(fileName, option==OpenOption::kOld ? File::kReadOnly : File::kReadWrite));
  bool exists = file->exists();
  switch (option)
    {
    case OpenOption::kOld:
      if (!exists) throw FileException(fileName,"File not found","CapFile::openCapFile()");
      file->read();
      break;
    case OpenOption::kNew:
      if (exists) throw FileException(fileName,"File already exists","CapFile::openCapFile()");
      break;
    case OpenOption::kRecreate:
      break;
    case OpenOption::kUpdate:
      if (exists) file->read();
      break;
    }
  if (verbose) printString("CapFile::openCapFile(...) File opened successfully.");
  return file.release();
  }

  CapFile * CapFile::openCapFile(const String & name, const String & ioOption, bool verbose)
  { return openCapFile("",name,ioOption,verbose); }
  CapFile * CapFile::openOldCapFile(const String & path, const String & name, bool verbose)
  { return openCapFile(path,name,"OLD",verbose); }
  CapFile * CapFile::openOldCapFile(const String & name, bool verbose)
  { return openCapFile("",name,"OLD",verbose); }
  CapFile * CapFile::openNewCapFile(const String & path, const String & name, bool verbose)
  { return openCapFile(path,name,"NEW",verbose); }
  CapFile * CapFile::openNewCapFile(const String & name, bool verbose)
  { return openCapFile("",name,"NEW",verbose); }
  CapFile * CapFile::openRecreateCapFile(const String & path, const String & name, bool verbose)
  { return openCapFile(path,name,"RECREATE",verbose); }
  CapFile * CapFile::openRecreateCapFile(const String & name, bool verbose)
  { return openCapFile("",name,"RECREATE",verbose); }

  std::vector<CapFile*> CapFile::openCapFiles(const std::vector<String> & names, const String & ioOption, bool verbose)
  { return openCapFiles("",names,ioOption,verbose); }

  std::vector<CapFile*> CapFile::openCapFiles(const String & path, const std::vector<String> & names, const String & ioOption, bool verbose)
  {
  if (names.empty()) throw FileException("Empty file list","File not found/opened","CapFile::openCapFiles()");
  if (verbose)
    {
    printCR();
    printValue("CapFile::openCapFiles(...) Opening N files",names.size());
    printValue("CapFile::openCapFiles(...) Path",path.data());
    }
  std::vector<CapFile*> files;
  files.reserve(names.size());
  try
    {
    for (const auto & name : names) files.push_back(openCapFile(path,name,ioOption,verbose));
    }
  catch (...)
    {
    for (auto * file : files) delete file;
    throw;
    }
  if (verbose) printString("CapFile::openCapFiles(...) Completed successfully");
  return files;
  }

  // ---------------------------------------------------------------- parameters and accountants

  void CapFile::exportParameter(const String & name, long value, bool verbose)
  {
  if (verbose)
    {
    printCR();
    printValue("CapFile::exportParameter() file",path().data());
    printValue("CapFile::exportParameter() parameterName",name.data());
    printValue("CapFile::exportParameter() value",value);
    }
  if (LongParameter * existing = get<LongParameter>(name))
    {
    existing->setValue(value);
    return;
    }
  add(new LongParameter(name,value),name);
  }

  long CapFile::importParameter(const String & name, bool verbose) const
  {
  const LongParameter * parameter = get<LongParameter>(name);
  if (!parameter)
    {
    if (verbose)
      {
      printCR();
      printValue("CapFile::importParameter() file",path().data());
      printValue("CapFile::importParameter() Parameter not found",name.data());
      }
    throw FileException(name,"Parameter not found","CapFile::importParameter()");
    }
  if (verbose)
    {
    printCR();
    printValue("CapFile::importParameter() parameterName",name.data());
    printValue("CapFile::importParameter() value",parameter->value());
    }
  return parameter->value();
  }

  void CapFile::exportAccountant(const Accountant & accountant, bool verbose)
  {
  if (verbose) printString("CapFile::exportAccountant(...)");
  exportParameter(accountantEntryName(accountant,"N"),long(accountant.size()),verbose);
  unsigned int k = 0;
  for (long value : accountant.values())
    exportParameter(accountantEntryName(accountant,std::to_string(k++)),value,verbose);
  }

  void CapFile::importAccountant(Accountant & accountant, bool verbose) const
  {
  if (verbose) printString("CapFile::importAccountant(...)");
  long n = importParameter(accountantEntryName(accountant,"N"),verbose);
  if (n<0) throw FileException(accountant.name(),"Invalid number of counts","CapFile::importAccountant()");
  if (accountant.size()!=0 && long(accountant.size())!=n)
    throw FileException(accountant.name(),"Stored number of counts differs from the accountant size","CapFile::importAccountant()");
  std::vector<long> values;
  values.reserve(std::size_t(n));
  for (long k=0; k<n; k++)
    values.push_back(importParameter(accountantEntryName(accountant,std::to_string(k)),verbose));
  accountant.values() = values;
  }

} // namespace CAP
