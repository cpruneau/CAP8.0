/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "XmlFile.hpp"
#include <cctype>
#include <string>
#include <vector>


namespace CAP
{

  namespace
  {
  bool isBlank(const std::string & s)
  {
  for (char c : s) if (!std::isspace(static_cast<unsigned char>(c))) return false;
  return true;
  }

  std::string trimLeft(const std::string & s)
  {
  std::size_t k = 0;
  while (k<s.size() && std::isspace(static_cast<unsigned char>(s[k]))) k++;
  return s.substr(k);
  }
  }

  XmlFile::XmlFile(const String & path, Access access)
  :
  TxtFile(path,access),
  _document(),
  _documentValid(false)
  {
  setClassName("XmlFile");
  }

  bool XmlFile::hasDeclaration() const
  {
  for (const auto & line : _lines)
    {
    std::string s(line.data());
    if (isBlank(s)) continue;
    return trimLeft(s).compare(0,5,"<?xml")==0;
    }
  return false;
  }

  void XmlFile::setDeclaration(const String & version, const String & encoding)
  {
  String declaration = String("<?xml version=\"")+version+"\" encoding=\""+encoding+"\"?>";
  for (auto & line : _lines)
    {
    std::string s(line.data());
    if (isBlank(s)) continue;
    if (trimLeft(s).compare(0,5,"<?xml")==0) { line = declaration; return; }
    break;
    }
  _lines.insert(_lines.begin(),declaration);
  }

  void XmlFile::addElement(const String & name, const String & content, unsigned int depth)
  {
  std::string indent(2*depth,' ');
  addLine(String(indent.c_str())+"<"+name+">"+escape(content)+"</"+name+">");
  }

  bool XmlFile::isWellFormed(String * reason) const
  {
  try
    {
    XmlDocument::parse(text());
    }
  catch (XmlException & e)
    {
    if (reason) *reason = e.what();
    return false;
    }
  if (reason) *reason = "";
  return true;
  }

  const XmlDocument & XmlFile::document() const
  {
  if (!_documentValid)
    {
    _document = XmlDocument::parse(text());
    _documentValid = true;
    }
  return _document;
  }

  void XmlFile::setDocument(const XmlDocument & document, unsigned int indent)
  {
  setText(document.toString(indent));
  }

  void XmlFile::load(Loading loading)
  {
  TxtFile::load(loading);
  document();
  }

  void XmlFile::unload()
  {
  TxtFile::unload();
  _document = XmlDocument();
  }

  String XmlFile::escape(const String & text)
  {
  std::string out;
  for (const char * p=text.data(); *p; p++)
    {
    switch (*p)
      {
      case '&':  out += "&amp;";  break;
      case '<':  out += "&lt;";   break;
      case '>':  out += "&gt;";   break;
      case '"':  out += "&quot;"; break;
      case '\'': out += "&apos;"; break;
      default:   out += *p;
      }
    }
  return String(out.c_str());
  }

  String XmlFile::unescape(const String & text)
  {
  static const char * entities[5][2] = { {"&lt;","<"}, {"&gt;",">"}, {"&quot;","\""}, {"&apos;","'"}, {"&amp;","&"} };
  std::string s(text.data()), out;
  std::size_t k = 0;
  while (k<s.size())
    {
    bool replaced = false;
    if (s[k]=='&')
      for (auto & e : entities)
        {
        std::size_t len = std::string(e[0]).size();
        if (s.compare(k,len,e[0])==0) { out += e[1]; k += len; replaced = true; break; }
        }
    if (!replaced) out += s[k++];
    }
  return String(out.c_str());
  }

} // namespace CAP
