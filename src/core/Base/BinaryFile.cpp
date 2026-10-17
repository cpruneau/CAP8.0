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
#include <algorithm>
#include <iterator>

ClassImp(CAP::BinaryFile);

namespace CAP
{

  BinaryFile::BinaryFile(const String & path, Access access)
  :
  File(path,kBinary,access,"BinaryFile"),
  _bytes(),
  _lazy(false)
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

  std::string BinaryFile::readRange(std::uintmax_t offset, std::size_t length)
  {
  open(kRead);
  _stream.seekg(0,std::ios::end);
  std::uintmax_t size = std::uintmax_t(_stream.tellg());
  if (offset>size) { close(); throw FileException(_path,"Offset beyond the end of the file","BinaryFile::readRange()"); }
  std::size_t n = std::size_t(std::min<std::uintmax_t>(length,size-offset));
  std::string bytes(n,'\0');
  _stream.seekg(std::streamoff(offset));
  if (n>0) _stream.read(&bytes[0],std::streamsize(n));
  bool failed = !_stream;
  close();
  if (failed) throw FileException(_path,"Error while reading the file","BinaryFile::readRange()");
  return bytes;
  }

  void BinaryFile::load(Loading loading)
  {
  if (loading==Loading::kLazy)
    {
    if (!FileSystem::isFile(_path) || !FileSystem::isReadable(_path))
      throw FileException(_path,"File does not exist or cannot be read","BinaryFile::load()");
    std::string().swap(_bytes);
    _lazy = true;
    }
  else
    {
    _bytes = readAll();
    _lazy = false;
    }
  _loaded = true;
  }

  void BinaryFile::unload()
  {
  std::string().swap(_bytes);   // frees the memory
  _lazy = false;
  _loaded = false;
  }

  void BinaryFile::save()
  {
  if (_lazy) throw FileException(_path,"Loaded lazily: no bytes held to save","BinaryFile::save()");
  writeAll(_bytes);
  _loaded = true;
  }

} // namespace CAP
