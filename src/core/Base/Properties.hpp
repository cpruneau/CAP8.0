/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__Properties
#define CAP__Properties
#include "Aliases.hpp"
#include "Property.hpp"
#include "ClassDef.hpp"

namespace CAP
{
  
  //!
  //! @brief Ordered list of named properties (key, value pairs).
  //!
  //! Keys are paths whose components are separated by colons, e.g. "Analysis:PtPt:nBins". Values are stored as strings
  //! (see Property) and converted on request by valueBool(), valueInt(), valueLong(), valueDouble() and valueString().
  //!
  //! <b>Lookup.</b> A key matches a property whose name is equal to it; failing that, a property whose name ends with
  //! ":key" (the key being one or more complete trailing path components) matches. Leading colons of the key are ignored.
  //! The first matching property is used.
  //!
  //! <b>Adding.</b> addProperty() replaces the value of a property with the same name, or appends a new one. A typed
  //! value (bool, int, long, double) also sets the type of the property; a string value keeps it (see Property). The
  //! (path, keyword) overloads build the name with standardize().
  //!
  //! <b>Files.</b> loadFrom() and saveTo() read and write "name = value" text files with PropertiesParser.
  //!
  //! Example:
  //! @code
  //!   CAP::Properties p;
  //!   p.addProperty("Analysis:PtPt","nBins",100);
  //!   int n = p.valueInt("PtPt:nBins");               // 100 (trailing path components)
  //!   double x = p.valueDouble("missing",true,2.5);   // 2.5 (default)
  //! @endcode
  //!
  class Properties
  {
  public:
  //! @brief Default constructor: no properties.
  Properties();
  //! @brief Copy constructor. @param source properties to copy
  Properties(const Properties & source);
  //! @brief Destructor.
  virtual ~Properties();
  //! @brief Copy assignment. @param rhs properties to copy @return these properties
  Properties & operator=(const Properties & rhs);
  
  //! @brief Property matching a key (see the lookup rules above). @param aKeyword key @return property
  //! @throws PropertyException if no property matches
  const Property & property(const char* aKeyword) const;
  //! @brief Property matching a key (see the lookup rules above). @param aKeyword key @return property
  //! @throws PropertyException if no property matches
  Property & property(const char* aKeyword);
  
  //! @brief All properties, in insertion order (modifiable). @return properties
  std::vector<Property> & properties() { return _properties; }
  //! @brief All properties, in insertion order. @return properties
  const std::vector<Property> & properties() const  { return _properties; }
  
  //! @name Value types
  //! Whether the value of the property matching a key can be read as a given type (see Property).
  //! These functions throw PropertyException if no property matches, except isFound().
  //! @{
  //! @brief Whether the value is a boolean ("0", "1", "true", "false", "yes", "no", any case). @param name key @return true if so
  virtual bool isBool(const char * name) const;
  //! @brief Whether the value is a decimal integer. @param name key @return true if so
  virtual bool isInt(const char * name) const;
  //! @brief Whether the value is a decimal integer. @param name key @return true if so
  virtual bool isLong(const char * name) const;
  //! @brief Whether the value is a floating-point number. @param name key @return true if so
  virtual bool isDouble(const char * name) const;
  //! @brief Whether the value is a string (always true). @param name key @return true
  virtual bool isString(const char * name) const;
  //! @brief Whether a property matches the key. @param name key @return true if found
  virtual bool isFound(const char * name) const;
  //! @}
  
  
  //! @name Values
  //! @{
  //! @brief Value as a boolean. @param aKeyword key @param useDefault return defaultValue if no property matches
  //! @param defaultValue value returned when no property matches and useDefault is true @return value
  //! @throws PropertyException if no property matches and useDefault is false
  virtual bool    valueBool(const char* aKeyword, bool useDefault=1, bool defaultValue=false) const;
  //! @brief Value as an int (-9999 if it is not a decimal integer). @param aKeyword key @param useDefault see valueBool()
  //! @param defaultValue see valueBool() @return value @throws PropertyException see valueBool()
  virtual int     valueInt(const char* aKeyword,  bool useDefault=1, int defaultValue=0) const;
  //! @brief Value as a long (-99999 if it is not a decimal integer). @param aKeyword key @param useDefault see valueBool()
  //! @param defaultValue see valueBool() @return value @throws PropertyException see valueBool()
  virtual long    valueLong(const char* aKeyword,  bool useDefault=1, long defaultValue=0) const;
  //! @brief Value as a double (-1e100 if it is not a number). @param aKeyword key @param useDefault see valueBool()
  //! @param defaultValue see valueBool() @return value @throws PropertyException see valueBool()
  virtual double  valueDouble(const char* aKeyword,  bool useDefault=1, double defaultValue=0) const;
  //! @brief Value as a string. @param aKeyword key @param useDefault see valueBool()
  //! @param defaultValue see valueBool() @return value @throws PropertyException see valueBool()
  virtual String  valueString(const char* aKeyword,  bool useDefault=1, const String defaultValue="NONE") const;
  //! @}
  
  //! @brief Key made of a path and a keyword: "path:keyword" (a single colon), or keyword alone if the path is empty.
  //! @param path path @param aKeyword keyword @return key
  String  standardize(   const char * path, const char* aKeyword) const;
  //! @name Values by path and keyword
  //! Values of the property standardize(path,keyword), with the defaults of the single-key versions (false, 0, 0, 0.0,
  //! "NONE") when no property matches.
  //! @{
  //! @brief Value as a boolean. @param path path @param aKeyword keyword @return value
  virtual bool    valueBool(const char * path, const char* aKeyword) const;
  //! @brief Value as an int. @param path path @param aKeyword keyword @return value
  virtual int     valueInt (const char * path, const char* aKeyword) const;
  //! @brief Value as a long. @param path path @param aKeyword keyword @return value
  virtual long    valueLong(const char * path, const char* aKeyword) const;
  //! @brief Value as a double. @param path path @param aKeyword keyword @return value
  virtual double  valueDouble(const char * path, const char* aKeyword) const;
  //! @brief Value as a string. @param path path @param aKeyword keyword @return value
  virtual String  valueString(const char * path, const char* aKeyword) const;
  //! @}
  
  //!
  //! @brief Add the properties of source that belong to a given (task) name: those whose name begins with "keyword:" or
  //! contains ":keyword:". The keyword must be a complete path component.
  //! @param source properties to select from @param keyword name to select
  //!
  virtual void addSelectedProperties(const Properties & source,
                                     const String keyword);
  //  virtual void addSelectedProperty(const Property & _property,
  //                                   const String keyword);
  
  
  //! @name Adding properties
  //! Each function replaces the value of the property of the same name, or appends a new property.
  //! @{
  //! @brief Add all the properties of source. @param source properties to add
  virtual void addProperties(const Properties & source);
  //! @brief Add a property. @param _property property to add
  virtual void addProperty(const Property & _property);
  //! @brief Add a boolean property. @param name name @param value value
  virtual void addProperty(const char * name, bool value);
  //! @brief Add an int property. @param name name @param value value
  virtual void addProperty(const char * name, int value);
  //! @brief Add a long property. @param name name @param value value
  virtual void addProperty(const char * name, long value);
  //! @brief Add a double property. @param name name @param value value
  virtual void addProperty(const char * name, double value);
  //! @brief Add a string property. @param name name @param value value
  virtual void addProperty(const char * name, const char * value);
  //! @brief Add a string property. @param name name @param value value
  virtual void addProperty(const char * name, const String & value);
  //! @brief Add a boolean property named standardize(path,name). @param path path @param name keyword @param value value
  virtual void addProperty(const char * path, const char * name, bool value);
  //! @brief Add an int property named standardize(path,name). @param path path @param name keyword @param value value
  virtual void addProperty(const char * path, const char * name, int value);
  //! @brief Add a long property named standardize(path,name). @param path path @param name keyword @param value value
  virtual void addProperty(const char * path, const char * name, long value);
  //! @brief Add a double property named standardize(path,name). @param path path @param name keyword @param value value
  virtual void addProperty(const char * path, const char * name, double value);
  //! @brief Add a string property named standardize(path,name). @param path path @param name keyword @param value value
  virtual void addProperty(const char * path, const char * name, const char * value);
  //! @brief Add a string property named standardize(path,name). @param path path @param name keyword @param value value
  virtual void addProperty(const char * path, const char * name, const String & value);
  //! @}
  
  //! @brief Whether a path ends with a colon (a colon at position 0 alone does not count). @param path path @return true if so
  bool hasEndColon(const char * path) const;
  //! @brief Index of the property matching a key (see the lookup rules above). @param aKeyword key
  //! @return index, or -1 if no property matches
  int  findIndex(const char * aKeyword) const;
  
  //! @brief Add the properties read from a "name = value" text file (see PropertiesParser::loadFrom()).
  //! @param _path directory @param _fileName file name @param _extension extension of the file
  //! @throws FileException if the file cannot be opened
  void loadFrom(const char * _path,
                const char * _fileName,
                const char * _extension=".ini");
  //! @brief Write the properties to a "name = value" text file (see PropertiesParser::saveTo()).
  //! @param _path directory @param _fileName file name @param _extension extension of the file
  //! @throws FileException if the file cannot be opened
  void saveTo(const char * _path,
              const char * _fileName,
              const char * _extension=".ini");
  //! @brief Write the properties to a stream as "name : TYPE : value" lines (see Property::saveTo()). @param os stream
  void saveTo(std::ostream & os) const;
  //! @brief Print all properties.
  void print() const;
  //! @brief Remove all properties.
  void clear();
  
  
  protected:
  
  std::vector<Property> _properties; //!< properties, in insertion order
  
  ClassDef(Properties,0)
  
  };
  
} // namespace CAP

#endif /* Properties_hpp */
