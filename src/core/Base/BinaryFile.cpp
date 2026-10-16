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
#include "BinaryFile.hpp"
#include <iterator>

ClassImp(CAP::BinaryFile);

namespace CAP
{

  BinaryFile::BinaryFile(const String & path, Access access)
  :
  File(path,kBinary,access,"BinaryFile")
  { }

  std::string BinaryFile::readAll()
  {
  open(kRead);
  std::string bytes((std::istreambuf_iterator<char>(_stream)),std::istreambuf_iterator<char>());
  bool failed = _stream.bad();
  close();
  if (failed) throw FileException(_path,"Error while reading the file","BinaryFile::readAll()");
  return bytes;
  }

  void BinaryFile::writeBytes(const std::string & bytes, const char * source)
  {
  _stream.write(bytes.data(),std::streamsize(bytes.size()));
  _stream.flush();
  bool failed = !_stream;
  close();
  if (failed) throw FileException(_path,"Error while writing the file",source);
  }

  void BinaryFile::writeAll(const std::string & bytes)
  {
  open(kWrite);
  writeBytes(bytes,"BinaryFile::writeAll()");
  }

  void BinaryFile::appendAll(const std::string & bytes)
  {
  open(kAppend);
  writeBytes(bytes,"BinaryFile::appendAll()");
  }

} // namespace CAP
