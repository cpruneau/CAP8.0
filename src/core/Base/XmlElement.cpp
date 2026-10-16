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
#include "XmlElement.hpp"
#include "XmlWriter.hpp"
#include "PrintHelpers.hpp"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <sstream>

namespace CAP
{

  XmlException::XmlException(const String & message, const String & source, int line, int column)
  :
  Exception(line>0 ? message+" (line "+line+", column "+column+")" : message, source, "XmlException"),
  _line(line),
  _column(column)
  { }

  void XmlException::print() const
  {
  printCR();
  printLine();
  printValue("Exception Type",_exception);
  printValue("Source",_source);
  printValue("Message",_message);
  printLine();
  }

  namespace
  {
  String trimmed(const String & s)
  {
  String r(s);
  r.strip();
  return r;
  }

  double toDouble(const String & text, const String & what, const char * source)
  {
  String t = trimmed(text);
  if (t.isNull()) throw XmlException(what+" is empty, not a number",source);
  errno = 0;
  char * end = nullptr;
  double v = std::strtod(t.data(),&end);
  if (end==t.data() || *end!='\0' || errno==ERANGE) throw XmlException(what+" is not a number: "+t,source);
  return v;
  }

  long toLong(const String & text, const String & what, const char * source)
  {
  String t = trimmed(text);
  if (!t.isInteger()) throw XmlException(what+" is not an integer: "+t,source);
  errno = 0;
  long v = std::strtol(t.data(),nullptr,10);
  if (errno==ERANGE) throw XmlException(what+" is out of range: "+t,source);
  return v;
  }

  bool toBool(const String & text, const String & what, const char * source)
  {
  String t = trimmed(text);
  t.toLower();
  if (t=="true"  || t=="yes" || t=="1") return true;
  if (t=="false" || t=="no"  || t=="0") return false;
  throw XmlException(what+" is not a boolean: "+text,source);
  }
  }

  //////////////////////////////////////////////////////////////////////////
  // Construction
  //////////////////////////////////////////////////////////////////////////

  XmlElement::XmlElement(const String & name)
  :
  _name(name), _attributes(), _text(), _children()
  { }

  XmlElement::XmlElement(const XmlElement & source)
  :
  _name(source._name), _attributes(source._attributes), _text(source._text), _children()
  {
  _children.reserve(source._children.size());
  for (const auto & c : source._children) _children.emplace_back(new XmlElement(*c));
  }

  XmlElement & XmlElement::operator=(const XmlElement & rhs)
  {
  if (this!=&rhs)
    {
    XmlElement copy(rhs);       // copy first: rhs may be a descendant of this element
    *this = std::move(copy);
    }
  return *this;
  }

  //////////////////////////////////////////////////////////////////////////
  // Attributes
  //////////////////////////////////////////////////////////////////////////

  bool XmlElement::hasAttribute(const String & name) const
  {
  for (const auto & a : _attributes) if (a.first==name) return true;
  return false;
  }

  const String & XmlElement::attribute(const String & name) const
  {
  for (const auto & a : _attributes) if (a.first==name) return a.second;
  throw XmlException(String("Element <")+_name+"> has no attribute "+name,__FUNCTION__);
  }

  String XmlElement::attribute(const String & name, const String & defaultValue) const
  {
  for (const auto & a : _attributes) if (a.first==name) return a.second;
  return defaultValue;
  }

  double XmlElement::attributeDouble(const String & name) const
  { return toDouble(attribute(name),String("Attribute ")+name+" of <"+_name+">",__FUNCTION__); }

  double XmlElement::attributeDouble(const String & name, double defaultValue) const
  { return hasAttribute(name) ? attributeDouble(name) : defaultValue; }

  long XmlElement::attributeInt(const String & name) const
  { return toLong(attribute(name),String("Attribute ")+name+" of <"+_name+">",__FUNCTION__); }

  long XmlElement::attributeInt(const String & name, long defaultValue) const
  { return hasAttribute(name) ? attributeInt(name) : defaultValue; }

  bool XmlElement::attributeBool(const String & name) const
  { return toBool(attribute(name),String("Attribute ")+name+" of <"+_name+">",__FUNCTION__); }

  bool XmlElement::attributeBool(const String & name, bool defaultValue) const
  { return hasAttribute(name) ? attributeBool(name) : defaultValue; }

  XmlElement & XmlElement::setAttribute(const String & name, const String & value)
  {
  if (!isValidName(name)) throw XmlException(String("Invalid attribute name: ")+name,__FUNCTION__);
  for (auto & a : _attributes) if (a.first==name) { a.second = value; return *this; }
  _attributes.emplace_back(name,value);
  return *this;
  }

  XmlElement & XmlElement::setAttribute(const String & name, double value) { return setAttribute(name,formatNumber(value)); }

  XmlElement & XmlElement::setAttribute(const String & name, long value) { String v; v += value; return setAttribute(name,v); }

  XmlElement & XmlElement::setAttribute(const String & name, bool value) { return setAttribute(name,String(value ? "true" : "false")); }

  bool XmlElement::removeAttribute(const String & name)
  {
  for (auto it=_attributes.begin(); it!=_attributes.end(); ++it)
    if (it->first==name) { _attributes.erase(it); return true; }
  return false;
  }

  //////////////////////////////////////////////////////////////////////////
  // Text
  //////////////////////////////////////////////////////////////////////////

  String XmlElement::text() const { return trimmed(_text); }

  double XmlElement::textDouble() const { return toDouble(_text,String("Text of <")+_name+">",__FUNCTION__); }

  long XmlElement::textInt() const { return toLong(_text,String("Text of <")+_name+">",__FUNCTION__); }

  bool XmlElement::textBool() const { return toBool(_text,String("Text of <")+_name+">",__FUNCTION__); }

  std::vector<double> XmlElement::textDoubles() const
  {
  std::vector<double> values;
  const char * p = _text.data();
  while (true)
    {
    while (*p==' ' || *p=='\t' || *p=='\n' || *p=='\r') p++;
    if (*p=='\0') break;
    char * end = nullptr;
    double v = std::strtod(p,&end);
    if (end==p || !(*end=='\0' || *end==' ' || *end=='\t' || *end=='\n' || *end=='\r'))
      throw XmlException(String("Text of <")+_name+"> contains a word that is not a number",__FUNCTION__);
    values.push_back(v);
    p = end;
    }
  return values;
  }

  XmlElement & XmlElement::setText(double value) { _text = formatNumber(value); return *this; }

  XmlElement & XmlElement::setText(long value) { _text.clear(); _text += value; return *this; }

  XmlElement & XmlElement::setText(const std::vector<double> & values, unsigned int perLine)
  {
  std::string t;
  for (std::size_t k=0; k<values.size(); k++)
    {
    if (k>0) t += (perLine>0 && k%perLine==0) ? '\n' : ' ';
    t += formatNumber(values[k]).str();
    }
  _text = String(t);
  return *this;
  }

  //////////////////////////////////////////////////////////////////////////
  // Children
  //////////////////////////////////////////////////////////////////////////

  XmlElement & XmlElement::child(unsigned int index)
  {
  if (index>=_children.size()) throw IndexOutOfBoundException(index,_children.size(),"Invalid child index",__FUNCTION__);
  return *_children[index];
  }

  const XmlElement & XmlElement::child(unsigned int index) const
  {
  if (index>=_children.size()) throw IndexOutOfBoundException(index,_children.size(),"Invalid child index",__FUNCTION__);
  return *_children[index];
  }

  XmlElement * XmlElement::child(const String & name)
  {
  for (auto & c : _children) if (c->_name==name) return c.get();
  return nullptr;
  }

  const XmlElement * XmlElement::child(const String & name) const
  {
  for (const auto & c : _children) if (c->_name==name) return c.get();
  return nullptr;
  }

  const XmlElement & XmlElement::requiredChild(const String & name) const
  {
  const XmlElement * c = child(name);
  if (!c) throw XmlException(String("Element <")+_name+"> has no child <"+name+">",__FUNCTION__);
  return *c;
  }

  std::vector<XmlElement *> XmlElement::children()
  {
  std::vector<XmlElement *> list;
  for (auto & c : _children) list.push_back(c.get());
  return list;
  }

  std::vector<const XmlElement *> XmlElement::children() const
  {
  std::vector<const XmlElement *> list;
  for (const auto & c : _children) list.push_back(c.get());
  return list;
  }

  std::vector<const XmlElement *> XmlElement::children(const String & name) const
  {
  std::vector<const XmlElement *> list;
  for (const auto & c : _children) if (c->_name==name) list.push_back(c.get());
  return list;
  }

  XmlElement & XmlElement::addChild(const String & name)
  {
  if (!isValidName(name)) throw XmlException(String("Invalid element name: ")+name,__FUNCTION__);
  _children.emplace_back(new XmlElement(name));
  return *_children.back();
  }

  XmlElement & XmlElement::addChild(const XmlElement & element)
  {
  _children.emplace_back(new XmlElement(element));
  return *_children.back();
  }

  void XmlElement::removeChild(unsigned int index)
  {
  if (index>=_children.size()) throw IndexOutOfBoundException(index,_children.size(),"Invalid child index",__FUNCTION__);
  _children.erase(_children.begin()+index);
  }

  void XmlElement::clear()
  {
  _attributes.clear();
  _text.clear();
  _children.clear();
  }

  //////////////////////////////////////////////////////////////////////////
  // Searching
  //////////////////////////////////////////////////////////////////////////

  const XmlElement * XmlElement::find(const String & path) const
  {
  const XmlElement * e = this;
  std::string p = path.str();
  std::size_t start = 0;
  while (e && start<=p.size())
    {
    std::size_t end = p.find('/',start);
    if (end==std::string::npos) end = p.size();
    if (end>start) e = e->child(String(p.substr(start,end-start)));
    start = end+1;
    }
  return e;
  }

  XmlElement * XmlElement::find(const String & path)
  {
  return const_cast<XmlElement *>(static_cast<const XmlElement *>(this)->find(path));
  }

  bool XmlElement::matches(const String & name, const String & attributeName, const String & attributeValue) const
  {
  if (_name!=name) return false;
  if (attributeName.isNull()) return true;
  for (const auto & a : _attributes) if (a.first==attributeName) return a.second==attributeValue;
  return false;
  }

  const XmlElement * XmlElement::findFirst(const String & name, const String & attributeName, const String & attributeValue) const
  {
  // depth-first, document order, without recursion
  std::vector<const XmlElement *> stack(1,this);
  while (!stack.empty())
    {
    const XmlElement * e = stack.back();
    stack.pop_back();
    if (e->matches(name,attributeName,attributeValue)) return e;
    for (auto it=e->_children.rbegin(); it!=e->_children.rend(); ++it) stack.push_back(it->get());
    }
  return nullptr;
  }

  XmlElement * XmlElement::findFirst(const String & name, const String & attributeName, const String & attributeValue)
  {
  return const_cast<XmlElement *>(static_cast<const XmlElement *>(this)->findFirst(name,attributeName,attributeValue));
  }

  std::vector<const XmlElement *> XmlElement::findAll(const String & name, const String & attributeName, const String & attributeValue) const
  {
  std::vector<const XmlElement *> found;
  std::vector<const XmlElement *> stack(1,this);
  while (!stack.empty())
    {
    const XmlElement * e = stack.back();
    stack.pop_back();
    if (e->matches(name,attributeName,attributeValue)) found.push_back(e);
    for (auto it=e->_children.rbegin(); it!=e->_children.rend(); ++it) stack.push_back(it->get());
    }
  return found;
  }

  //////////////////////////////////////////////////////////////////////////
  // Text form and helpers
  //////////////////////////////////////////////////////////////////////////

  String XmlElement::toString(unsigned int indent) const
  {
  std::ostringstream out;
  XmlWriter writer(out,indent);
  writer.write(*this);
  writer.finish();
  return String(out.str());
  }

  bool XmlElement::isValidName(const String & name)
  {
  if (name.isNull()) return false;
  for (int i=0; i<name.length(); i++)
    {
    unsigned char c = (unsigned char) name[i];
    bool letter = (c>='A' && c<='Z') || (c>='a' && c<='z') || c=='_' || c==':' || c>=0x80;
    bool other  = (c>='0' && c<='9') || c=='.' || c=='-';
    if (!(letter || (i>0 && other))) return false;
    }
  return true;
  }

  String XmlElement::formatNumber(double value)
  {
  char buffer[40];
  for (int precision=1; precision<=17; precision++)
    {
    std::snprintf(buffer,sizeof(buffer),"%.*g",precision,value);
    if (std::strtod(buffer,nullptr)==value) break;
    }
  return String(buffer);
  }

} // namespace CAP
