/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "TxtFile.hpp"
#include "PrintHelpers.hpp"
#include <string>

ClassImp(CAP::TxtFile);

namespace CAP
{

  namespace
  {
  // Split a text at "\n", removing a trailing "\r" from each line.
  std::vector<String> splitLines(const std::string & text)
  {
  std::vector<String> lines;
  std::size_t start = 0;
  while (start<=text.size())
    {
    std::size_t end = text.find('\n',start);
    if (end==std::string::npos) end = text.size();
    std::string line = text.substr(start,end-start);
    if (!line.empty() && line.back()=='\r') line.pop_back();
    lines.push_back(String(line.c_str()));
    start = end+1;
    }
  return lines;
  }
  }

  TxtFile::TxtFile(const String & path, Access access)
  :
  File(path,kText,access,"TxtFile"),
  _lines()
  { }

  const String & TxtFile::line(unsigned int index) const
  {
  if (index>=_lines.size()) throw FileException(_path,"Line index out of range (index>=nLines())","TxtFile::line()");
  return _lines[index];
  }

  void TxtFile::setLine(unsigned int index, const String & line)
  {
  if (index>=_lines.size()) throw FileException(_path,"Line index out of range (index>=nLines())","TxtFile::setLine()");
  _lines[index] = line;
  changed();
  }

  String TxtFile::text() const
  {
  std::string text;
  for (std::size_t k=0; k<_lines.size(); k++)
    {
    if (k>0) text += '\n';
    text += _lines[k].data();
    }
  return String(text.c_str());
  }

  void TxtFile::setText(const String & text)
  {
  if (text.length()==0) _lines.clear();
  else _lines = splitLines(std::string(text.data(),std::size_t(text.length())));
  changed();
  }

  void TxtFile::read()
  {
  open(kRead);
  std::vector<String> lines;
  std::string line;
  while (std::getline(_stream,line))
    {
    if (!line.empty() && line.back()=='\r') line.pop_back();
    lines.push_back(String(line.c_str()));
    }
  bool failed = _stream.bad();
  close();
  if (failed) throw FileException(_path,"Error while reading the file","TxtFile::read()");
  _lines.swap(lines);
  _loaded = true;
  changed();
  }

  void TxtFile::load(Loading /*loading*/)
  {
  read();
  }

  void TxtFile::unload()
  {
  std::vector<String>().swap(_lines);   // frees the memory
  _loaded = false;
  changed();
  }

  void TxtFile::writeLines(const char * source)
  {
  for (const auto & line : _lines) _stream << line.data() << '\n';
  _stream.flush();
  bool failed = !_stream;
  close();
  if (failed) throw FileException(_path,"Error while writing the file",source);
  }

  void TxtFile::write()
  {
  open(kWrite);
  writeLines("TxtFile::write()");
  _loaded = true;
  }

  void TxtFile::append()
  {
  open(kAppend);
  writeLines("TxtFile::append()");
  }

  void TxtFile::print() const
  {
  File::print();
  printValue("Lines",nLines());
  }

} // namespace CAP
