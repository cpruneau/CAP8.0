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
#include "CapFile.hpp"
#include "LongParameter.hpp"
#include "PrintHelpers.hpp"
#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <sstream>

ClassImp(CAP::CapFile);

namespace CAP
{

  CapFile::CapFile(const String & path, File::Access access)
  :
  Folder(""),
  _file(path,access)
  {
  setClassName("CapFile");
  }

  CapFile & CapFile::operator=(const Folder & folder)
  {
  if (this!=&folder) adopt(folder);
  return *this;
  }

  void CapFile::adopt(const Folder & folder)
  {
  Folder::operator=(folder);   // copies the Object part too: restore the class name
  setClassName("CapFile");
  }

  void CapFile::read()
  {
  // Read the file into memory first, so that the folder is unchanged if anything fails.
  std::istringstream in(_file.readAll());
  Folder loaded;
  if (!loaded.readBinary(in)) throw FileException(_file.path(),"No folder in the file","CapFile::read()");
  adopt(loaded);
  }

  void CapFile::read(const String & folderName)
  {
  std::istringstream in(_file.readAll());
  std::string tag, payload;
  unsigned int version;
  while (readBinaryRecord(in,tag,version,payload,_file.path()))
    {
    if (tag!=Folder::binaryTag() || !binaryPayloadName(payload).equalTo(folderName)) continue;
    Folder loaded;
    decodeBinaryPayload(payload,version,loaded,_file.path());
    adopt(loaded);
    return;
    }
  throw FileException(_file.path(),String("No folder named ")+folderName+" in the file","CapFile::read()");
  }

  void CapFile::writeFolder(File::Mode mode, bool skipUnsavable)
  {
  // Encode the whole folder first: the file is not touched if an object cannot be saved.
  std::ostringstream out;
  writeBinary(out,skipUnsavable);
  if (mode==File::kWrite) _file.writeAll(out.str());
  else                    _file.appendAll(out.str());
  }

  void CapFile::write(bool skipUnsavable)
  {
  writeFolder(File::kWrite,skipUnsavable);
  }

  void CapFile::append(bool skipUnsavable)
  {
  writeFolder(File::kAppend,skipUnsavable);
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
  String fileName;
  if (name.beginsWith("/") || path.length()==0)
    fileName = name;
  else
    {
    fileName = path;
    if (!fileName.endsWith("/")) fileName += "/";
    fileName += name;
    }
  if (!fileName.endsWith(".cap")) fileName += ".cap";
  return fileName;
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
