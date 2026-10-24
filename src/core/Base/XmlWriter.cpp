/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "XmlWriter.hpp"
#include "XmlElement.hpp"

namespace CAP
{

  XmlWriter::XmlWriter(std::ostream & out, unsigned int indent)
  :
  _out(out), _indent(indent), _open(), _startTagOpen(false), _wroteSomething(false), _rootClosed(false)
  { }

  String XmlWriter::escape(const String & text)
  {
  std::string r;
  r.reserve(text.length());
  for (int i=0; i<text.length(); i++)
    {
    char c = text[i];
    switch (c)
      {
      case '&':  r += "&amp;";  break;
      case '<':  r += "&lt;";   break;
      case '>':  r += "&gt;";   break;
      case '"':  r += "&quot;"; break;
      case '\'': r += "&apos;"; break;
      default:   r += c;
      }
    }
  return String(r);
  }

  void XmlWriter::closeStartTag()
  {
  if (_startTagOpen) { _out << '>'; _startTagOpen = false; }
  }

  void XmlWriter::newLine()
  {
  if (_indent==0) return;
  if (_wroteSomething) _out << '\n';
  _out << std::string(std::size_t(_indent)*_open.size(),' ');
  }

  XmlWriter & XmlWriter::declaration(const String & version, const String & encoding)
  {
  if (_wroteSomething) throw XmlException("The declaration must come first",__FUNCTION__);
  _out << "<?xml version=\"" << escape(version) << "\" encoding=\"" << escape(encoding) << "\"?>";
  _wroteSomething = true;
  return *this;
  }

  XmlWriter & XmlWriter::startElement(const String & name)
  {
  if (!XmlElement::isValidName(name)) throw XmlException(String("Invalid element name: ")+name,__FUNCTION__);
  if (_open.empty() && _rootClosed) throw XmlException("A document has a single root element",__FUNCTION__);
  closeStartTag();
  if (!_open.empty()) _open.back().hasChildren = true;
  newLine();
  _out << '<' << name;
  _open.push_back({name,false,false});
  _startTagOpen = true;
  _wroteSomething = true;
  return *this;
  }

  XmlWriter & XmlWriter::attribute(const String & name, const String & value)
  {
  if (!_startTagOpen) throw XmlException("Attributes must follow startElement()",__FUNCTION__);
  if (!XmlElement::isValidName(name)) throw XmlException(String("Invalid attribute name: ")+name,__FUNCTION__);
  _out << ' ' << name << "=\"" << escape(value) << '"';
  return *this;
  }

  XmlWriter & XmlWriter::attribute(const String & name, double value) { return attribute(name,XmlElement::formatNumber(value)); }

  XmlWriter & XmlWriter::attribute(const String & name, long value) { String v; v += value; return attribute(name,v); }

  XmlWriter & XmlWriter::text(const String & text)
  {
  if (_open.empty()) throw XmlException("Text must be written inside an element",__FUNCTION__);
  if (text.isNull()) return *this;
  closeStartTag();
  _out << escape(text);
  _open.back().hasText = true;
  return *this;
  }

  XmlWriter & XmlWriter::cdata(const String & text)
  {
  if (_open.empty()) throw XmlException("CDATA must be written inside an element",__FUNCTION__);
  closeStartTag();
  String t(text);
  t.replaceAll("]]>","]]]]><![CDATA[>");
  _out << "<![CDATA[" << t << "]]>";
  _open.back().hasText = true;
  return *this;
  }

  XmlWriter & XmlWriter::comment(const String & text)
  {
  if (text.contains("--")) throw XmlException("A comment cannot contain \"--\"",__FUNCTION__);
  closeStartTag();
  if (!_open.empty()) _open.back().hasChildren = true;
  newLine();
  _out << "<!--" << text << "-->";
  _wroteSomething = true;
  return *this;
  }

  XmlWriter & XmlWriter::element(const String & name, const String & text)
  {
  startElement(name);
  this->text(text);
  return endElement();
  }

  XmlWriter & XmlWriter::endElement()
  {
  if (_open.empty()) throw XmlException("No element to close",__FUNCTION__);
  Open e = _open.back();
  _open.pop_back();
  if (_startTagOpen)
    {
    _out << "/>";
    _startTagOpen = false;
    }
  else
    {
    // an element with child elements (and no text) ends on its own line; text stays on the line of the start tag
    if (e.hasChildren && !e.hasText) newLine();
    _out << "</" << e.name << '>';
    }
  if (_open.empty()) _rootClosed = true;
  return *this;
  }

  XmlWriter & XmlWriter::write(const XmlElement & element)
  {
  startElement(element.name());
  for (const auto & a : element.attributes()) attribute(a.first,a.second);
  if (!element.rawText().isNull()) text(element.hasChildren() ? element.text() : element.rawText());
  for (const XmlElement * c : element.children()) write(*c);
  return endElement();
  }

  XmlWriter & XmlWriter::finish()
  {
  while (!_open.empty()) endElement();
  if (_wroteSomething && _indent>0) _out << '\n';
  _out.flush();
  return *this;
  }

} // namespace CAP
