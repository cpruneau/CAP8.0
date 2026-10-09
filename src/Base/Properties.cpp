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
#include "Properties.hpp"
#include "PropertiesParser.hpp"

ClassImp(CAP::Properties);

namespace CAP
{

  Properties::Properties()
  {  }

  Properties::Properties(const Properties & source)
  {
  _properties = source._properties;
  }


  Properties & Properties::operator=(const Properties & rhs)
  {
  if (this!= & rhs)
    {
    _properties = rhs._properties;
    }
  return *this;
  }

  Properties::~Properties()
  {
  clear();
  }


  String  Properties::standardize(const char * path, const char* keyword) const
  {
  String aKeyword = path;
  if (aKeyword.Length()==0) return String(keyword); // no path: avoid producing ":keyword"
  if (aKeyword.EndsWith(":")) aKeyword += keyword;
  else
    {
    aKeyword += ":";
    aKeyword += keyword;
    }
  return aKeyword;
  }


  //!
  //! Locate a property by name. An exact match always wins. Failing that, a property whose name
  //! ends with ":<keyword>" (i.e., the keyword is a complete trailing path component) is accepted.
  //! Returns -1 if there is no match.
  //!
  int Properties::findIndex(const char* keyword) const
  {
  String key = keyword;
  while (key.BeginsWith(":")) key = String(key.Data()+1);
  if (key.Length()==0) return -1;
  for (unsigned int k=0; k<_properties.size(); k++)
    {
    if (_properties[k].isNamed(key)) return k;
    }
  String suffix = ":";
  suffix += key;
  for (unsigned int k=0; k<_properties.size(); k++)
    {
    if (_properties[k].name().EndsWith(suffix)) return k;
    }
  return -1;
  }

  const Property & Properties::property(const char* keyword)  const
  {
  int index = findIndex(keyword);
  if (index<0) throw PropertyException(keyword,"Property not found","Properties::property(const char* keyword) ");
  return _properties[index];
  }

  Property & Properties::property(const char* keyword)
  {
  int index = findIndex(keyword);
  if (index<0) throw PropertyException(keyword,"Property not found",__FUNCTION__);
  return _properties[index];
  }

  bool  Properties::valueBool(const char* keyword, bool useDefault, bool defaultValue) const
  {
  try
    {
    const Property & p = property(keyword);
    return p.getBool();
    }
  catch (PropertyException & e )
    {
    if (useDefault)
      return defaultValue;
    else
      throw;   // rethrow the original exception (throw e; threw a copy)
    }
  }


  int Properties::valueInt(const char* keyword,  bool useDefault, int defaultValue) const
  {
  try
    {
    const Property & p = property(keyword);
    return p.integer();
    }
  catch (PropertyException & e )
    {
    if (useDefault)
      return defaultValue;
    else
      throw;   // rethrow the original exception (throw e; threw a copy)
    }
  }

  long Properties::valueLong(const char* keyword,  bool useDefault, long defaultValue) const
  {
  try
    {
    const Property & p = property(keyword);
    return p.longInteger();
    }
  catch (PropertyException & e )
    {
    if (useDefault)
      return defaultValue;
    else
      throw;   // rethrow the original exception (throw e; threw a copy)
    }
  }

  double Properties::valueDouble (const char* keyword,  bool useDefault, double defaultValue) const
  {
  try
    {
    const Property & p = property(keyword);
    return p.doubleValue();
    }
  catch (PropertyException & e )
    {
    if (useDefault)
      return defaultValue;
    else
      throw;   // rethrow the original exception (throw e; threw a copy)
    }
  }

  String Properties::valueString (const char* keyword,  bool useDefault, const String defaultValue) const
  {
  try
    {
    const Property & p = property(keyword);
    return p.string();
    }
  catch (PropertyException & e )
    {
    if (useDefault)
      return defaultValue;
    else
      throw;   // rethrow the original exception (throw e; threw a copy)
    }
  }

  bool Properties::valueBool(const char * path, const char* keyword) const
  {
  return Properties::valueBool(standardize(path,keyword));
  }

  int Properties::valueInt (const char * path, const char* keyword) const
  {
  return Properties::valueInt(standardize(path,keyword));
  }

  long    Properties::valueLong(const char * path, const char* keyword) const
  {
  return Properties::valueLong(standardize(path,keyword));
  }

  double  Properties::valueDouble(const char * path, const char* keyword) const
  {
  return Properties::valueDouble(standardize(path,keyword));
  }

  String  Properties::valueString(const char * path, const char* keyword) const
  {
  return Properties::valueString(standardize(path,keyword));
  }


  //!
  //! Add a bool property to the configuration with the given name and value
  //!
  void Properties::addProperty(const char * path, const char * keyword, bool value)
  {
  Properties::addProperty(standardize(path,keyword),value);
  }

  //!
  //! Add an int property to the configuration with the given name and value
  //!
  void Properties::addProperty(const char * path, const char * keyword, int value)
  {
  Properties::addProperty(standardize(path,keyword),value);
  }

  //!
  //! Add an int property to the configuration with the given name and value
  //!
  void Properties::addProperty(const char * path, const char * keyword, long value)
  {
  Properties::addProperty(standardize(path,keyword),value);
  }

  //!
  //! Add a double property to the configuration with the given name and value
  //!
  void Properties::addProperty(const char * path, const char * keyword, double value)
  {
  Properties::addProperty(standardize(path,keyword),value);
  }

  //!
  //! Add a const char *  property to the configuration with the given name and value
  //!
  void Properties::addProperty(const char * path, const char * keyword, const char * value)
  {
  Properties::addProperty(standardize(path,keyword),value);
  }

  //!
  //! Add a string property to the configuration with the given name and value
  //!
  void Properties::addProperty(const char * path, const char * keyword, const String & value)
  {
  Properties::addProperty(standardize(path,keyword),value);
  }


  bool Properties::hasEndColon(const char * path) const
  {
  String s = path;
  int last = s.Last(':');
  int size = s.Length() - 1;
  return (last>0  &&  last == size);
  }


  void Properties::addSelectedProperties(const Properties & source,
                                         const String keyword)
  {
  // Select properties that belong to the given (task) name: the name must be a complete path
  // component, not merely a substring (so that "Ana" does not pick up "Analysis:..." keys).
  String prefix = keyword;   prefix += ":";
  String inner  = ":";       inner  += keyword;  inner += ":";
  for (auto & p : source.properties())
    {
    if (p.name().BeginsWith(prefix) || p.name().Contains(inner))  addProperty(p);
    }
  }

  void Properties::addProperties(const Properties & source)
  {
  // Go through addProperty() so that a property already present is overridden by the new value
  // (a plain push_back left the old value first in line, so property() kept returning it).
  for (auto & p : source._properties) addProperty(p);
  }

  void Properties::addProperty(const Property& newProperty)
  {
  for (auto & p : _properties)
    {
    if (p.sameNameAs(newProperty))
      {
      p.setValue(newProperty.string());
      return;
      }
    }
  _properties.push_back(newProperty);
  }

  void Properties::addProperty(const char * name, bool value)
  {
  Property p(name,value);
  addProperty(p);
  }

  //!
  //! Add an int property to the Properties with the given name and value
  //!
  void Properties::addProperty(const char * name, int value)
  {
  Property p(name,value);
  addProperty(p);
  }


  //!
  //! Add an int property to the Properties with the given name and value
  //!
  void Properties::addProperty(const char * name, long value)
  {
  Property p(name,value);
  addProperty(p);
  }

  //!
  //! Add a double property to the Properties with the given name and value
  //!
  void Properties::addProperty(const char * name, double value)
  {
  Property p(name,value);
  addProperty(p);
  }

  //!
  //! Add a string property to the Properties with the given name and value
  //!
  void Properties::addProperty(const char * keyword, const char * value)
  {
  Property p(keyword,value);
  addProperty(p);
  }

  //!
  //! Add a string property to the Properties with the given name and value
  //!
  void Properties::addProperty(const char * keyword, const String & value)
  {
  Property p(keyword,value);
  addProperty(p);
  }

  void Properties::print() const
  {
  for (auto & p : properties()) p.print();
  }

  void Properties::saveTo(std::ostream & os) const
  {
  for (auto & p : properties()) p.saveTo(os);
  }

  void Properties::clear()
  {
  _properties.clear();
  }

  bool Properties::isBool(const char * keyword) const
  {
  const Property & p = property(keyword);
  return p.isBool();
  }


  bool Properties::isInt(const char * keyword) const
  {
  const Property & p = property(keyword);
  return p.isInt();
  }

  bool Properties::isLong(const char * keyword) const
  {
  const Property & p = property(keyword);
  return p.isLong();
  }

  bool Properties::isDouble(const char * keyword) const
  {
  const Property & p = property(keyword);
  return p.isDouble();
  }


  bool Properties::isString(const char * keyword) const
  {
  const Property & p = property(keyword);
  return p.isString();
  }

  bool Properties::isFound(const char * keyword) const
  {
  return findIndex(keyword)>=0;
  }

  void Properties::loadFrom(const char * _path,
                            const char * _fileName,
                            const char * _extension)
  {
  PropertiesParser parser;
  parser.loadFrom(*this,_path,_fileName,_extension);
  }

  //void Properties::saveTo(const char * _path      __attribute__((unused)),
  //                          const char * _fileName  __attribute__((unused)),
  //                          const char * _extension __attribute__((unused)) )
  void Properties::saveTo(const char * _path,
                          const char * _fileName,
                          const char * _extension)
  {
  PropertiesParser parser;
  parser.saveTo(*this,_path,_fileName,_extension);
  }

} // namespace CAP
