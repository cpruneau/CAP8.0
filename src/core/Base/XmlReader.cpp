/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "XmlReader.hpp"
#include <cstring>

namespace CAP
{

  namespace
  {
  bool isSpace(char c) { return c==' ' || c=='\t' || c=='\n' || c=='\r'; }
  bool isNameStart(unsigned char c) { return (c>='A' && c<='Z') || (c>='a' && c<='z') || c=='_' || c==':' || c>=0x80; }
  bool isNameChar(unsigned char c)  { return isNameStart(c) || (c>='0' && c<='9') || c=='.' || c=='-'; }

  void appendUtf8(std::string & out, unsigned long cp)
  {
  if (cp<0x80)        { out += char(cp); }
  else if (cp<0x800)  { out += char(0xC0|(cp>>6));  out += char(0x80|(cp&0x3F)); }
  else if (cp<0x10000){ out += char(0xE0|(cp>>12)); out += char(0x80|((cp>>6)&0x3F)); out += char(0x80|(cp&0x3F)); }
  else                { out += char(0xF0|(cp>>18)); out += char(0x80|((cp>>12)&0x3F)); out += char(0x80|((cp>>6)&0x3F)); out += char(0x80|(cp&0x3F)); }
  }

  bool isWhiteOnly(const std::string & s)
  {
  for (char c : s) if (!isSpace(c)) return false;
  return true;
  }
  }

  XmlReader::XmlReader(const String & text)
  :
  _text(text.str()), _pos(0), _line(1), _column(1), _maximumDepth(10000)
  { }

  void XmlReader::error(const String & message) const
  {
  throw XmlException(message,"XmlReader::read()",_line,_column);
  }

  bool XmlReader::startsWith(const char * s) const
  {
  return _text.compare(_pos,std::strlen(s),s)==0;
  }

  void XmlReader::advance(std::size_t n)
  {
  for (std::size_t i=0; i<n && _pos<_text.size(); i++, _pos++)
    {
    if (_text[_pos]=='\n') { _line++; _column = 1; }
    else                   _column++;
    }
  }

  void XmlReader::skipSpace()
  {
  while (_pos<_text.size() && isSpace(_text[_pos])) advance();
  }

  String XmlReader::readName()
  {
  if (_pos>=_text.size() || !isNameStart((unsigned char)_text[_pos])) error("Name expected");
  std::size_t start = _pos;
  while (_pos<_text.size() && isNameChar((unsigned char)_text[_pos])) advance();
  return String(_text.substr(start,_pos-start));
  }

  std::string XmlReader::readUntil(const char * terminator, const char * what)
  {
  std::size_t end = _text.find(terminator,_pos);
  if (end==std::string::npos) error(String("Unterminated ")+what);
  std::string content = _text.substr(_pos,end-_pos);
  advance(end-_pos+std::strlen(terminator));
  return content;
  }

  void XmlReader::readReference(std::string & out)
  {
  // current character is '&'
  std::size_t end = _text.find(';',_pos);
  if (end==std::string::npos || end-_pos>12) error("Invalid entity reference (missing ';')");
  std::string ref = _text.substr(_pos+1,end-_pos-1);
  if      (ref=="amp")  out += '&';
  else if (ref=="lt")   out += '<';
  else if (ref=="gt")   out += '>';
  else if (ref=="quot") out += '"';
  else if (ref=="apos") out += '\'';
  else if (ref.size()>1 && ref[0]=='#')
    {
    bool hex = ref[1]=='x' || ref[1]=='X';
    std::string digits = ref.substr(hex ? 2 : 1);
    if (digits.empty()) error("Invalid character reference");
    unsigned long cp = 0;
    for (char c : digits)
      {
      int d = 0;
      if (c>='0' && c<='9') d = c-'0';
      else if (hex && c>='a' && c<='f') d = c-'a'+10;
      else if (hex && c>='A' && c<='F') d = c-'A'+10;
      else { error("Invalid character reference"); }
      cp = cp*(hex ? 16 : 10) + d;
      if (cp>0x10FFFF) error("Character reference out of range");
      }
    if (cp==0) error("Character reference to the null character");
    appendUtf8(out,cp);
    }
  else error(String("Unknown entity &")+ref.c_str()+";");
  advance(end-_pos+1);
  }

  String XmlReader::readAttributeValue()
  {
  if (_pos>=_text.size() || (_text[_pos]!='"' && _text[_pos]!='\'')) error("Quoted attribute value expected");
  char quote = _text[_pos];
  advance();
  std::string value;
  while (true)
    {
    if (_pos>=_text.size()) error("Unterminated attribute value");
    char c = _text[_pos];
    if (c==quote) { advance(); break; }
    if (c=='<') error("'<' in an attribute value");
    if (c=='&') { readReference(value); continue; }
    value += c;
    advance();
    }
  return String(value);
  }

  void XmlReader::readDeclaration(XmlDocument & document)
  {
  // current position after "<?xml"
  String version = "1.0", encoding = "UTF-8";
  while (true)
    {
    skipSpace();
    if (startsWith("?>")) { advance(2); break; }
    if (_pos>=_text.size()) error("Unterminated XML declaration");
    String name = readName();
    skipSpace();
    if (_pos>=_text.size() || _text[_pos]!='=') error("'=' expected in the XML declaration");
    advance();
    skipSpace();
    String value = readAttributeValue();
    if      (name=="version")  version  = value;
    else if (name=="encoding") encoding = value;
    }
  document.setDeclaration(version,encoding);
  }

  void XmlReader::skipDoctype()
  {
  advance(9); // "<!DOCTYPE"
  int brackets = 0;
  while (_pos<_text.size())
    {
    char c = _text[_pos];
    if (c=='"' || c=='\'')
      {
      std::size_t end = _text.find(c,_pos+1);
      if (end==std::string::npos) error("Unterminated DOCTYPE");
      advance(end-_pos+1);
      continue;
      }
    if (c=='[') brackets++;
    if (c==']') brackets--;
    if (c=='>' && brackets<=0) { advance(); return; }
    advance();
    }
  error("Unterminated DOCTYPE");
  }

  XmlDocument XmlReader::read()
  {
  XmlDocument document;
  _pos = 0; _line = 1; _column = 1;
  if (startsWith("\xEF\xBB\xBF")) { _pos = 3; }                // UTF-8 byte order mark
  std::vector<XmlElement *> open;                               // elements being read, outermost first
  bool rootDone = false;
  std::string text;                                             // character data of the innermost open element
  while (_pos<_text.size())
    {
    char c = _text[_pos];
    if (c!='<')
      {
      if (c=='&')
        {
        if (open.empty()) error("Text outside the root element");
        readReference(text);
        continue;
        }
      if (open.empty() && !isSpace(c)) error("Text outside the root element");
      if (!open.empty()) text += c;
      advance();
      continue;
      }
    // markup
    if (startsWith("<?xml") && _pos+5<_text.size() && (isSpace(_text[_pos+5]) || _text[_pos+5]=='?'))
      {
      if (rootDone || !open.empty() || !document.isEmpty()) error("XML declaration not at the start of the document");
      advance(5);
      readDeclaration(document);
      continue;
      }
    if (startsWith("<?"))   { advance(2); readUntil("?>","processing instruction"); continue; }
    if (startsWith("<!--"))
      {
      advance(4);
      std::string comment = readUntil("-->","comment");
      if (comment.find("--")!=std::string::npos) error("\"--\" inside a comment");
      continue;
      }
    if (startsWith("<![CDATA["))
      {
      if (open.empty()) error("CDATA section outside the root element");
      advance(9);
      text += readUntil("]]>","CDATA section");
      continue;
      }
    if (startsWith("<!DOCTYPE"))
      {
      if (!open.empty() || rootDone) error("DOCTYPE inside or after the root element");
      skipDoctype();
      continue;
      }
    if (startsWith("</"))
      {
      int tagLine = _line, tagColumn = _column;
      advance(2);
      String name = readName();
      skipSpace();
      if (_pos>=_text.size() || _text[_pos]!='>') error("'>' expected at the end of an end tag");
      if (open.empty())
        throw XmlException(String("End tag </")+name+"> without a start tag","XmlReader::read()",tagLine,tagColumn);
      if (open.back()->name()!=name)
        throw XmlException(String("End tag </")+name+"> does not match <"+open.back()->name()+">","XmlReader::read()",tagLine,tagColumn);
      advance();
      // white space between child elements and the end tag is formatting, not content
      if (!(open.back()->hasChildren() && isWhiteOnly(text))) open.back()->appendText(String(text));
      text.clear();
      open.pop_back();
      if (open.empty()) rootDone = true;
      continue;
      }
    // start tag
    advance(1);
    String name = readName();
    if (rootDone) error(String("Second root element <")+name+">");
    if (open.size()>=_maximumDepth) error("Elements nested too deeply");
    XmlElement * element;
    if (open.empty())
      {
      document.root() = XmlElement(name);
      element = &document.root();
      }
    else
      {
      // text read so far belongs to the parent; keep it (white space between child elements is dropped)
      if (!isWhiteOnly(text)) open.back()->appendText(String(text));
      text.clear();
      element = &open.back()->addChild(name);
      }
    bool empty = false;
    while (true)
      {
      bool hadSpace = _pos<_text.size() && isSpace(_text[_pos]);
      skipSpace();
      if (_pos>=_text.size()) error(String("Unterminated start tag <")+name+">");
      if (startsWith("/>")) { advance(2); empty = true; break; }
      if (_text[_pos]=='>') { advance(); break; }
      if (!hadSpace) error("White space expected between attributes");
      String attributeName = readName();
      skipSpace();
      if (_pos>=_text.size() || _text[_pos]!='=') error(String("'=' expected after attribute ")+attributeName);
      advance();
      skipSpace();
      String value = readAttributeValue();
      if (element->hasAttribute(attributeName)) error(String("Duplicate attribute ")+attributeName);
      element->setAttribute(attributeName,value);
      }
    if (empty)
      {
      if (open.empty()) rootDone = true;
      }
    else
      open.push_back(element);
    }
  if (!open.empty()) error(String("Element <")+open.back()->name()+"> is not closed");
  if (!rootDone) error("No root element");
  return document;
  }

} // namespace CAP
