/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "XmlDocument.hpp"
#include "XmlReader.hpp"
#include "XmlWriter.hpp"
#include "FileSystem.hpp"
#include <fstream>
#include <sstream>

namespace CAP
{

  XmlDocument::XmlDocument(const String & rootName)
  :
  _version("1.0"), _encoding("UTF-8"), _root(rootName)
  { }

  XmlDocument XmlDocument::parse(const String & text)
  {
  XmlReader reader(text);
  return reader.read();
  }

  XmlDocument XmlDocument::read(const String & fileName)
  {
  std::ifstream in(fileName.data(),std::ios::in|std::ios::binary);
  if (!in) throw FileException(fileName,"Cannot open file",__FUNCTION__);
  std::ostringstream content;
  content << in.rdbuf();
  if (in.bad()) throw FileException(fileName,"Cannot read file",__FUNCTION__);
  try
    {
    return parse(String(content.str()));
    }
  catch (XmlException & e)
    {
    throw XmlException(String(e.what())+" in file "+fileName,__FUNCTION__,e.line(),e.column());
    }
  }

  XmlDocument XmlDocument::read(const String & path, const String & name)
  {
  return read(fileName(path,name));
  }

  String XmlDocument::toString(unsigned int indent) const
  {
  std::ostringstream out;
  XmlWriter writer(out,indent);
  writer.declaration(_version,_encoding);
  if (!isEmpty()) writer.write(_root);
  writer.finish();
  return String(out.str());
  }

  void XmlDocument::write(const String & fileName, unsigned int indent) const
  {
  std::ofstream out(fileName.data(),std::ios::out|std::ios::trunc|std::ios::binary);
  if (!out) throw FileException(fileName,"Cannot open file for writing",__FUNCTION__);
  XmlWriter writer(out,indent);
  writer.declaration(_version,_encoding);
  if (!isEmpty()) writer.write(_root);
  writer.finish();
  if (!out) throw FileException(fileName,"Cannot write file",__FUNCTION__);
  }

  void XmlDocument::write(const String & path, const String & name, unsigned int indent) const
  {
  write(fileName(path,name),indent);
  }

  String XmlDocument::fileName(const String & path, const String & name)
  {
  return FileSystem::join(path,name,".xml");
  }

} // namespace CAP
