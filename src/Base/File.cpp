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
#include "File.hpp"
#include "PrintHelpers.hpp"
#include <sys/stat.h>

ClassImp(CAP::File);

namespace CAP
{

  File::File(const String & path, Format format, Access access, const String & className)
  :
  Object(path,path,className),
  _path(path),
  _format(format),
  _access(access),
  _mode(kRead),
  _stream()
  { }

  File::~File()
  {
  if (_stream.is_open()) _stream.close();
  }

  void File::setPath(const String & path)
  {
  if (isOpen()) throw FileException(_path,"Cannot change the path of an open file","File::setPath()");
  _path = path;
  }

  bool File::allows(Mode mode) const
  {
  switch (mode)
    {
    case kRead:   return canRead();
    case kWrite:
    case kAppend: return canWrite();
    case kUpdate: return _access==kReadWrite;
    }
  return false;
  }

  bool File::exists(const String & path)
  {
  struct stat info;
  return path.length()>0 && ::stat(path.data(),&info)==0 && S_ISREG(info.st_mode);
  }

  std::uintmax_t File::fileSize() const
  {
  struct stat info;
  if (_path.length()==0 || ::stat(_path.data(),&info)!=0 || !S_ISREG(info.st_mode))
    throw FileException(_path,"File does not exist","File::fileSize()");
  return std::uintmax_t(info.st_size);
  }

  void File::open(Mode mode)
  {
  if (!allows(mode))
    throw FileException(_path,"Mode not allowed by the access of this file (read only, write only, or read/write)","File::open()");
  if (_path.length()==0) throw FileException(_path,"No file path given","File::open()");
  close();
  std::ios::openmode flags = std::ios::in;
  switch (mode)
    {
    case kRead:   flags = std::ios::in;                    break;
    case kWrite:  flags = std::ios::out | std::ios::trunc; break;
    case kAppend: flags = std::ios::out | std::ios::app;   break;
    case kUpdate: flags = std::ios::in  | std::ios::out;   break;
    }
  if (_format==kBinary) flags |= std::ios::binary;
  if (mode==kUpdate && !exists())
    {
    std::ofstream create(_path.data(), _format==kBinary ? std::ios::binary : std::ios::out);   // create it empty
    }
  _stream.clear();
  _stream.open(_path.data(),flags);
  if (!_stream.is_open()) throw FileException(_path,"Cannot open file","File::open()");
  _mode = mode;
  }

  void File::close()
  {
  if (_stream.is_open()) _stream.close();
  _stream.clear();
  }

  File::Mode File::mode() const
  {
  if (!isOpen()) throw FileException(_path,"File is not open","File::mode()");
  return _mode;
  }

  std::fstream & File::stream()
  {
  if (!isOpen()) throw FileException(_path,"File is not open","File::stream()");
  return _stream;
  }

  void File::requireReadable(const char * source)
  {
  if (!isOpen() || (_mode!=kRead && _mode!=kUpdate)) throw FileException(_path,"File is not open for reading",source);
  }

  void File::requireWritable(const char * source)
  {
  if (!isOpen() || _mode==kRead) throw FileException(_path,"File is not open for writing",source);
  }

  void File::checkStream(const char * source, const char * what)
  {
  if (!_stream) throw FileException(_path,String("Error while ")+what+" the file",source);
  }

  void File::print() const
  {
  Object::print();
  printValue("Path",_path);
  printValue("Format",String(_format==kText ? "text" : "binary"));
  printValue("Access",String(_access==kReadOnly ? "read only" : (_access==kWriteOnly ? "write only" : "read/write")));
  printValue("Open",String(isOpen() ? "yes" : "no"));
  }

} // namespace CAP
