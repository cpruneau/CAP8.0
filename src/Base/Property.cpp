/* **********************************************************************
 * Copyright (C) 2019-2024, Claude Pruneau, Victor Gonzalez     
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "Property.hpp"
#include "PrintHelpers.hpp"
#include <cerrno>
#include <climits>
#include <cstdlib>

ClassImp(CAP::Property);

namespace CAP
{

PropertyException::PropertyException(const String & keyword, const String & message, const String & source)
:
Exception(message,source,__FUNCTION__),
_keyword(keyword)
{   }

void PropertyException::print() const
{
  printCR();
  printLine();
  printValue("Exception Type",_exception);
  printLine();
  printValue("Source",_source);
  printValue("Message",_message);
  printValue("Keyword",_keyword);
  printLine();
}

Property::Property()
:
Object(),
_value(),
_type(kString)
{ /* no ops */}

Property::Property(const char * name, const char * value)
:
Object(name,"","Property"),
_value(value),
_type(kString)
{}

Property::Property(const char * name, const String & value)
:
Object(name,"","Property"),
_value(value),
_type(kString)
{}

Property::Property(const char * name, bool value)
:
Object(name,"","Property"),
_value(),
_type(kString)
{
  setValue(value);
}

Property::Property(const char * name, int value)
:
Object(name,"","Property"),
_value(),
_type(kString)
{
  setValue(value);
}

Property::Property(const char * name, long value)
:
Object(name,"","Property"),
_value(),
_type(kString)
{
  setValue(value);
}

Property::Property(const char * name, double value)
:
Object(name,"","Property"),
_value(),
_type(kString)
{
  setValue(value);
}

Property::Property(const char * name, const char * title, const char * value)
:
Object(name,title,"Property"),
_value(value),
_type(kString)
{ }

Property::Property(const char * name, const char * title, const String & value)
:
Object(name,title,"Property"),
_value(value),
_type(kString)
{ }

Property::Property(const char * name, const char * title, bool value)
:
Object(name,title,"Property"),
_value(),
_type(kString)
{
  setValue(value);
}

Property::Property(const char * name, const char * title, int value)
:
Object(name,title,"Property"),
_value(),
_type(kString)
{
  setValue(value);
}

Property::Property(const char * name, const char * title, long value)
:
Object(name,title,"Property"),
_value(),
_type(kString)
{
  setValue(value);
}

Property::Property(const char * name, const char * title, double value)
:
Object(name,title,"Property"),
_value(),
_type(kString)
{
  setValue(value);
}

Property::Property(const Property & property)
:
Object(property),
_value(property._value),
_type(property._type)
{ }

Property & Property::operator=(const Property & property)
{
  if (this!=&property)
    {
    Object::operator=(property);
    _value = property._value;
    _type  = property._type;
    }
  return *this;
}

void Property::setValue(const String & value)
{
  _value = value;
}

void Property::setValue(const char *  value)
{
  _value = value;
}

const String & Property::value() const
{
  return _value;
}

bool Property::valueContains(const char * keyword) const
{
  return _value.contains(keyword);
}

//!
//!Determines whethe this and the given object are equal
//!
//!option determine how stuct the equality is.
//!option=0 : same name
//!option=1 : same name and same value
//!option=2 : same name, same title, and same value
//!
bool Property::isEqualTo(const Property & rhs, int option) const
{
  switch (option)
    {
      case 0: return _name.equalTo(rhs._name);
      case 1: return _name.equalTo(rhs._name)  &&  _value.equalTo(rhs._value);
      case 2: return _name.equalTo(rhs._name)  &&  _title.equalTo(rhs._title)  &&  _value.equalTo(rhs._value);
    }
  return false; // should never get here...
}

void Property::set(const char * name, const char * value)
{
  _name = name;
  _title = name;
  _value  = value;
}

void Property::set(const char * name, const String & value)
{
  _name = name;
  _title = name;
  _value  = value;
}

void Property::set(const char * name, bool value)
{
  _name = name;
  _title = name;
  setValue(value);
}

void Property::set(const char * name, int value)
{
  _name = name;
  _title = name;
  setValue(value);
}

void Property::set(const char * name, long value)
{
  _name = name;
  _title = name;
  setValue(value);
}

void Property::set(const char * name, double value)
{
  _name = name;
  _title = name;
  setValue(value);
}

void Property::set(const String & name, const String & title, const String & value)
{
  _name = name;
  _title = title;
  _value  = value;
}

void Property::set(const String & name, const String & title, bool value)
{
  _name = name;
  _title = title;
  setValue(value);
}

void Property::set(const String & name, const String & title, int  value)
{
  _name = name;
  _title = title;
  setValue(value);
}

void Property::set(const String & name, const String & title, long value)
{
  _name = name;
  _title = title;
  setValue(value);
}

void Property::set(const String & name, const String & title, double value)
{
  _name = name;
  _title = title;
  setValue(value);
}

void Property::reset()
{
  _name  = "";
  _title = "";
  _value = "";
  _type  = kString;
}

void Property::clear()
{
  _name  = "";
  _title = "";
  _value = "";
  _type  = kString;
}

void Property::erase()
{
  _name  = "";
  _title = "";
  _value = "";
  _type  = kString;
}

void Property::setValue(bool value)
{
  _type = kBool;
  _value = "";
  _value += value;
}

void Property::setValue(int value)
{
  _type = kInt;
  _value  = "";
  _value  += value;
}

void Property::setValue(long value)
{
  _type = kLong;
  _value  = "";
  _value  += value;
}

void Property::setValue(double value)
{
  _type = kDouble;
  _value  = "";
  _value  += value;
}

// name & value

//void Property::set(const char * name, const char * value)
//{
//  name    = name;
//  title   = name;
//  value   = "";
//  value   += value;
//}
//
//void Property::set(const char * name, const String & value)
//{
//  name    = name;
//  title   = name;
//  value   = "";
//  value   += value;
//}
//
//void Property::set(const char * name, bool value)
//{
//  name    = name;
//  title   = name;
//  value   = "";
//  value   += value;
//}
//
//void Property::set(const char * name, int value)
//{
//  name    = name;
//  title   = name;
//  value   = "";
//  value   += value;
//}
//
//void Property::set(const char * name, long value)
//{
//  name    = name;
//  title   = name;
//  value   = "";
//  value   += value;
//}
//
//void Property::set(const char * name, double value)
//{
//  name    = name;
//  title   = name;
//  value   = "";
//  value   += value;
//}

// extract bool, int, long, double
// v.toUpper();

bool  Property::isBool() const
{
  String v = _value;
  v.toUpper(); // same (case-insensitive) rules as getBool()
  return v.equalTo("0") || v.equalTo("FALSE") || v.equalTo("NO")
      || v.equalTo("1") || v.equalTo("TRUE")  || v.equalTo("YES");
}

// not quite right... should return false if integer is too large
bool Property::isInt() const
{
  if (!_value.isDec()) return false;
  return true;
}

// not quite right...
bool Property::isLong() const
{
  if (!_value.isDec()) return false;
  return true;
}

bool Property::isDouble() const
{
  if (_value.isFloat()) return true;
  return false;
}

// always can be considered a string... not good?
bool Property::isString() const
{
  return true;
}

bool  Property::getBool() const
{
  String v = _value;
  v.toUpper();
  if (v.equalTo("0") || v.equalTo("FALSE") || v.equalTo("NO") ) return false;
  else if (v.equalTo("1") || v.equalTo("TRUE")  || v.equalTo("YES")  ) return true;
  else if (v.isDec())
    {
    int vv = v.atoi();
    return vv>0;
    }
  return false;
}

int Property::integer() const
{
  String v = _value;
  if (v.isDec()) return v.atoi();
  return -9999;
  //throw Exception("Property value not an integer","Property::integer()");
}

long Property::longInteger() const
{
  String v = _value;
  if (!v.isDec())
    {
    return -99999;
    }
  return v.atoll();
}

double Property::doubleValue() const
{
  String v = _value;
  if (!v.isFloat())
    {
    return -1.0E100;
    }
  return v.atof();
}

const String Property::string() const
{
  return _value;
}

const String Property::upperString() const
{
  String v = _value; v.toUpper();
  return v;
}

const String Property::lowerString() const
{
  String v = _value; v.toLower();
  return v;
}

bool Property::nameHasEndColon() const
{
  int last = _name.last(':');
  int size = _name.length() - 1;
  return (last>0  &&  last == size);
}

void Property::print()  const
{
  printValue(_name,_value);
}

void Property::saveTo(std::ostream & output)  const
{
  output << _name << " : ";
  if (isBool())
    output << "BOOL";
  else if (isInt())
    output << "INTEGER";
  else if (isLong())
    output << "LONG";
  else if (isDouble())
    output << "DOUBLE";
  else if (isString())
    output << "STRING";
  else
    output << "UNKNOWN";
  output << " : " << _value << std::endl;
}

const char * Property::typeName(Type type)
{
  switch (type)
    {
    case kBool:   return "bool";
    case kInt:    return "int";
    case kLong:   return "long";
    case kDouble: return "double";
    default:      return "string";
    }
}

bool Property::typeFromName(const String & name, Type & type)
{
  String n = name; n.toLower();
  if      (n=="bool"   || n=="boolean")             type = kBool;
  else if (n=="int"    || n=="integer")             type = kInt;
  else if (n=="long")                               type = kLong;
  else if (n=="double" || n=="float" || n=="real")  type = kDouble;
  else if (n=="string" || n=="text")                type = kString;
  else return false;
  return true;
}

bool Property::isValidValue(const String & value, Type type)
{
  String v = value; v.strip();
  const char * c = v.data();
  char * end = nullptr;
  switch (type)
    {
    case kBool:
      {
      String u = v; u.toUpper();
      return u=="0" || u=="1" || u=="TRUE" || u=="FALSE" || u=="YES" || u=="NO";
      }
    case kInt:
      {
      if (v.isNull()) return false;
      errno = 0;
      long x = std::strtol(c,&end,10);
      return errno==0 && *end=='\0' && x>=INT_MIN && x<=INT_MAX;
      }
    case kLong:
      if (v.isNull()) return false;
      errno = 0;
      std::strtol(c,&end,10);
      return errno==0 && *end=='\0';
    case kDouble:
      if (v.isNull()) return false;
      std::strtod(c,&end);
      return *end=='\0';
    default:
      return true;
    }
}

String Property::normalizedValue(const String & value, Type type)
{
  if (type==kString) return value;
  String v = value; v.strip();
  if (type==kBool)
    {
    String u = v; u.toUpper();
    return (u=="1" || u=="TRUE" || u=="YES") ? String("1") : String("0");
    }
  return v;
}

} // namespace CAP
