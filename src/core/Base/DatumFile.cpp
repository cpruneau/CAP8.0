/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "DatumFile.hpp"
#include "LongParameter.hpp"
#include "PrintHelpers.hpp"
#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <sstream>
#include <utility>


namespace Datum
{

  DatumFile::DatumFile(const String & path, Access access)
  :
  BinaryFile(path,access),
  _folder(""),
  _source()
  {
  setClassName("DatumFile");
  }

  DatumFile & DatumFile::operator=(const Folder & folder)
  {
  if (&_folder!=&folder) _folder = folder;
  return *this;
  }

  bool DatumFile::readFolder(const String * folderName, Folder & folder)
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

  void DatumFile::read()
  {
  // decoded into a separate folder first, so that the folder held is unchanged if anything fails
  Folder loaded;
  if (!readFolder(nullptr,loaded)) throw FileException(_path,"No folder in the file","DatumFile::read()");
  _folder = std::move(loaded);
  _source.reset();
  _lazy = false;
  _loaded = true;
  }

  void DatumFile::read(const String & folderName)
  {
  Folder loaded;
  if (!readFolder(&folderName,loaded))
    throw FileException(_path,String("No folder named ")+folderName+" in the file","DatumFile::read()");
  _folder = std::move(loaded);
  _source.reset();
  _lazy = false;
  _loaded = true;
  }

  void DatumFile::load(Loading loading)
  {
  if (loading==Loading::kEager) { read(); return; }
  if (!FileSystem::isFile(_path)) throw FileException(_path,"File does not exist","DatumFile::load()");
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
  if (!found) throw FileException(_path,"No folder in the file","DatumFile::load()");
  _folder = std::move(loaded);
  _source = source;
  _lazy = true;
  _loaded = true;
  }

  void DatumFile::unload()
  {
  _folder = Folder();
  _source.reset();
  _lazy = false;
  _loaded = false;
  }

  void DatumFile::loadAllBeforeWriting()
  {
  if (_source && _source->path.equalTo(FileSystem::normalize(_path)) && _folder.nNotLoaded()>0) _folder.loadAll();
  }

  void DatumFile::writeFolder(Mode mode, bool skipUnsavable)
  {
  // Encode the whole folder first: the file is not touched if an object cannot be saved.
  loadAllBeforeWriting();
  std::ostringstream out;
  _folder.writeBinary(out,skipUnsavable);
  if (mode==kWrite) writeAll(out.str());
  else              appendAll(out.str());
  }

  void DatumFile::write(bool skipUnsavable)
  {
  writeFolder(kWrite,skipUnsavable);
  _loaded = true;
  }

  void DatumFile::append(bool skipUnsavable)
  {
  writeFolder(kAppend,skipUnsavable);
  }

  void DatumFile::print() const
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
  throw FileException(fileName,String("Unknown ioOption: ")+ioOption,"DatumFile::openDatumFile()");
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

  bool DatumFile::isDatumFileName(const String & name)
  {
  return FileSystem::hasExtension(name,extension()) || FileSystem::hasExtension(name,formerExtension());
  }

  String DatumFile::makeFileName(const String & path, const String & name)
  {
  return isDatumFileName(name) ? FileSystem::join(path,name) : FileSystem::join(path,name,extension());
  }

  String DatumFile::existingFileName(const String & path, const String & name)
  {
  String file = makeFileName(path,name);
  if (FileSystem::isFile(FileSystem::expand(file))) return file;
  String other = FileSystem::withExtension(file, FileSystem::hasExtension(file,extension()) ? formerExtension() : extension());
  return FileSystem::isFile(FileSystem::expand(other)) ? other : file;
  }

  DatumFile * DatumFile::openDatumFile(const String & path, const String & name, const String & ioOption, bool verbose)
  {
  String fileName = makeFileName(path,name);
  OpenOption option = parseOpenOption(ioOption,fileName);
  if (option==OpenOption::kOld || option==OpenOption::kUpdate) fileName = existingFileName(path,name);
  if (verbose)
    {
    printCR();
    printValue("DatumFile::openDatumFile(...) Opening file",fileName.data());
    printValue("ioOption",ioOption.data());
    }
  std::unique_ptr<DatumFile> file(new DatumFile(fileName, option==OpenOption::kOld ? File::kReadOnly : File::kReadWrite));
  bool exists = file->exists();
  switch (option)
    {
    case OpenOption::kOld:
      if (!exists) throw FileException(fileName,"File not found","DatumFile::openDatumFile()");
      file->read();
      break;
    case OpenOption::kNew:
      if (exists) throw FileException(fileName,"File already exists","DatumFile::openDatumFile()");
      break;
    case OpenOption::kRecreate:
      break;
    case OpenOption::kUpdate:
      if (exists) file->read();
      break;
    }
  if (verbose) printString("DatumFile::openDatumFile(...) File opened successfully.");
  return file.release();
  }

  DatumFile * DatumFile::openDatumFile(const String & name, const String & ioOption, bool verbose)
  { return openDatumFile("",name,ioOption,verbose); }
  DatumFile * DatumFile::openOldDatumFile(const String & path, const String & name, bool verbose)
  { return openDatumFile(path,name,"OLD",verbose); }
  DatumFile * DatumFile::openOldDatumFile(const String & name, bool verbose)
  { return openDatumFile("",name,"OLD",verbose); }
  DatumFile * DatumFile::openNewDatumFile(const String & path, const String & name, bool verbose)
  { return openDatumFile(path,name,"NEW",verbose); }
  DatumFile * DatumFile::openNewDatumFile(const String & name, bool verbose)
  { return openDatumFile("",name,"NEW",verbose); }
  DatumFile * DatumFile::openRecreateDatumFile(const String & path, const String & name, bool verbose)
  { return openDatumFile(path,name,"RECREATE",verbose); }
  DatumFile * DatumFile::openRecreateDatumFile(const String & name, bool verbose)
  { return openDatumFile("",name,"RECREATE",verbose); }

  std::vector<DatumFile*> DatumFile::openDatumFiles(const std::vector<String> & names, const String & ioOption, bool verbose)
  { return openDatumFiles("",names,ioOption,verbose); }

  std::vector<DatumFile*> DatumFile::openDatumFiles(const String & path, const std::vector<String> & names, const String & ioOption, bool verbose)
  {
  if (names.empty()) throw FileException("Empty file list","File not found/opened","DatumFile::openDatumFiles()");
  if (verbose)
    {
    printCR();
    printValue("DatumFile::openDatumFiles(...) Opening N files",names.size());
    printValue("DatumFile::openDatumFiles(...) Path",path.data());
    }
  std::vector<DatumFile*> files;
  files.reserve(names.size());
  try
    {
    for (const auto & name : names) files.push_back(openDatumFile(path,name,ioOption,verbose));
    }
  catch (...)
    {
    for (auto * file : files) delete file;
    throw;
    }
  if (verbose) printString("DatumFile::openDatumFiles(...) Completed successfully");
  return files;
  }

  // ---------------------------------------------------------------- parameters and accountants

  void DatumFile::exportParameter(const String & name, long value, bool verbose)
  {
  if (verbose)
    {
    printCR();
    printValue("DatumFile::exportParameter() file",path().data());
    printValue("DatumFile::exportParameter() parameterName",name.data());
    printValue("DatumFile::exportParameter() value",value);
    }
  if (LongParameter * existing = get<LongParameter>(name))
    {
    existing->setValue(value);
    return;
    }
  add(new LongParameter(name,value),name);
  }

  long DatumFile::importParameter(const String & name, bool verbose) const
  {
  const LongParameter * parameter = get<LongParameter>(name);
  if (!parameter)
    {
    if (verbose)
      {
      printCR();
      printValue("DatumFile::importParameter() file",path().data());
      printValue("DatumFile::importParameter() Parameter not found",name.data());
      }
    throw FileException(name,"Parameter not found","DatumFile::importParameter()");
    }
  if (verbose)
    {
    printCR();
    printValue("DatumFile::importParameter() parameterName",name.data());
    printValue("DatumFile::importParameter() value",parameter->value());
    }
  return parameter->value();
  }

  void DatumFile::exportAccountant(const Accountant & accountant, bool verbose)
  {
  if (verbose) printString("DatumFile::exportAccountant(...)");
  exportParameter(accountantEntryName(accountant,"N"),long(accountant.size()),verbose);
  unsigned int k = 0;
  for (long value : accountant.values())
    exportParameter(accountantEntryName(accountant,std::to_string(k++)),value,verbose);
  }

  void DatumFile::importAccountant(Accountant & accountant, bool verbose) const
  {
  if (verbose) printString("DatumFile::importAccountant(...)");
  long n = importParameter(accountantEntryName(accountant,"N"),verbose);
  if (n<0) throw FileException(accountant.name(),"Invalid number of counts","DatumFile::importAccountant()");
  if (accountant.size()!=0 && long(accountant.size())!=n)
    throw FileException(accountant.name(),"Stored number of counts differs from the accountant size","DatumFile::importAccountant()");
  std::vector<long> values;
  values.reserve(std::size_t(n));
  for (long k=0; k<n; k++)
    values.push_back(importParameter(accountantEntryName(accountant,std::to_string(k)),verbose));
  accountant.values() = values;
  }

} // namespace Datum
