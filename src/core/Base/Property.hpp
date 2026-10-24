/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__Property
#define CAP__Property
#include "Object.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Exception thrown when a property is missing or invalid (e.g., Properties::property() with an unknown key).
//!
class PropertyException : public Exception
{
public:
  //! @brief Constructor. @param name key of the property @param message description of the problem
  //! @param source function where the problem was found
  PropertyException(const String & name, const String & message, const String & source);
  //! @brief Destructor.
  virtual ~PropertyException()  {}
  //! @brief Print the exception type, source, message and key.
  virtual void print() const;
protected:
  const String  _keyword; //!< key of the property
};


//!
//! @brief Named value of a configuration (see Properties and Configuration).
//!
//! A property features three elements:
//! - name: a name or key to refer to the property (a colon-separated path, e.g. "PtPt:nBins");
//! - title: a readable name or title;
//! - value: the value of the property.
//!
//! The value is stored as a String but can be set from and converted to other types: booleans ("0", "1", "true",
//! "false", "yes", "no", any case), integers and floating-point numbers. Conversions of values of the wrong type return
//! sentinel values (see integer(), longInteger() and doubleValue()); check the type with isBool(), isInt(), ... first.
//!
//! Example:
//! @code
//!   CAP::Property p("PtPt:nBins",100);
//!   if (p.isInt()) n = p.integer();
//! @endcode
//!
class Property : public Object
{
public:
  //! @brief Type of the value, set by the typed constructors and setters (kString for string values).
  //! Setting the value from a string keeps the type.
  enum Type { kString=0, kBool, kInt, kLong, kDouble };

  //! @brief Default constructor: empty name and value.
  Property();
  //! @name Constructors from a name and a value (the title is empty)
  //! @{
  //! @brief Constructor. @param name name @param value value
  Property(const char * name, const char * value);
  //! @brief Constructor. @param name name @param value value
  Property(const char * name, const String & value);
  //! @brief Constructor. @param name name @param value value
  Property(const char * name, bool value);
  //! @brief Constructor. @param name name @param value value
  Property(const char * name, int value);
  //! @brief Constructor. @param name name @param value value
  Property(const char * name, long value);
  //! @brief Constructor. @param name name @param value value
  Property(const char * name, double value);
  //! @}
  
  //! @name Constructors from a name, a title and a value
  //! @{
  //! @brief Constructor. @param name name @param title title @param value value
  Property(const char * name, const char * title, const char * value);
  //! @brief Constructor. @param name name @param title title @param value value
  Property(const char * name, const char * title, const String & value);
  //! @brief Constructor. @param name name @param title title @param value value
  Property(const char * name, const char * title, bool value);
  //! @brief Constructor. @param name name @param title title @param value value
  Property(const char * name, const char * title, int value);
  //! @brief Constructor. @param name name @param title title @param value value
  Property(const char * name, const char * title, long value);
  //! @brief Constructor. @param name name @param title title @param value value
  Property(const char * name, const char * title, double value);
  //! @}
  
  //! @brief Copy constructor. @param property property to copy
  Property(const Property & property);
  //! @brief Copy assignment. @param property property to copy @return this property
  Property & operator=(const Property & property);
  //! @brief Destructor.
  virtual ~Property(){}
  
  //! @brief Set the value. @param title new value
  virtual void setValue(const String & title);
  //! @brief Set the value. @param value new value
  virtual void setValue(const char * value);
  //! @brief Value, as stored. @return value
  const String & value() const;
  
  //! @brief Whether the value contains a string. @param aKeyword string to look for @return true if found
  virtual bool valueContains(const char * aKeyword) const;
  //! @brief Compare with another property.
  //! @param rhs property to compare with
  //! @param option 0: names; 1: names and values; 2: names, titles and values
  //! @return true if the compared elements are equal (false for any other option)
  virtual bool isEqualTo(const Property & rhs, int option=1) const;

  //! @name Setting name, title and value
  //! @{
  //! @brief Set name, title and value. @param name name @param title title @param value value
  virtual void set(const String & name,const String & title,const String & value="");
  //! @brief Set name, title and value. @param name name @param title title @param value value
  virtual void set(const String & name,const String & title,bool value);
  //! @brief Set name, title and value. @param name name @param title title @param value value
  virtual void set(const String & name,const String & title,int value);
  //! @brief Set name, title and value. @param name name @param title title @param value value
  virtual void set(const String & name,const String & title,long value);
  //! @brief Set name, title and value. @param name name @param title title @param value value
  virtual void set(const String & name,const String & title,double value);
  //! @}

  //! @name Setting name and value (the title is set to the name)
  //! @{
  //! @brief Set name and value. @param name name (also used as title) @param value value
  virtual void set(const char * name, const char * value);
  //! @brief Set name and value. @param name name (also used as title) @param value value
  virtual void set(const char * name, const String & value);
  //! @brief Set name and value. @param name name (also used as title) @param value value
  virtual void set(const char * name, bool value);
  //! @brief Set name and value. @param name name (also used as title) @param value value
  virtual void set(const char * name, int value);
  //! @brief Set name and value. @param name name (also used as title) @param value value
  virtual void set(const char * name, long value);
  //! @brief Set name and value. @param name name (also used as title) @param value value
  virtual void set(const char * name, double value);
  //! @}

  //! @name Setting the value from other types
  //! @{
  //! @brief Set the value. @param value new value
  virtual void setValue(bool value);
  //! @brief Set the value. @param value new value
  virtual void setValue(int value);
  //! @brief Set the value. @param value new value
  virtual void setValue(long value);
  //! @brief Set the value. @param value new value
  virtual void setValue(double value);
  //! @}


  //! @name Types and conversions
  //! @{
  //! @brief Whether the value is a boolean: "0", "1", "true", "false", "yes" or "no" (any case). @return true if so
  bool isBool() const;
  //! @brief Whether the value is a decimal integer. @return true if so
  bool isInt() const;
  //! @brief Whether the value is a decimal integer. @return true if so
  bool isLong() const;
  //! @brief Whether the value is a floating-point number. @return true if so
  bool isDouble() const;
  //! @brief Whether the value is a string (always true). @return true
  bool isString() const;
  //! @brief Value as a boolean: true for "1", "true", "yes" or a positive integer, false otherwise. @return value
  bool getBool() const;
  //! @brief Value as an int. @return value, or -9999 if it is not a decimal integer
  int  integer() const;
  //! @brief Value as a long. @return value, or -99999 if it is not a decimal integer
  long longInteger() const;
  //! @brief Value as a double. @return value, or -1e100 if it is not a number
  double doubleValue() const;
  //! @brief Value as a string. @return value
  const String string() const;
  //! @brief Value in upper case. @return value
  const String upperString() const;
  //! @brief Value in lower case. @return value
  const String lowerString() const;
  //! @}
  //! @name Value type
  //! @{
  //! @brief Type of the value. @return type
  Type type() const { return _type; }
  //! @brief Set the type of the value (the value is not changed). @param type type
  void setType(Type type) { _type = type; }
  //! @brief Name of the type of the value ("bool", "int", "long", "double" or "string"). @return name
  const char * typeName() const { return typeName(_type); }
  //! @brief Name of a type: "bool", "int", "long", "double" or "string". @param type type @return name
  static const char * typeName(Type type);
  //! @brief Type of a name ("bool"/"boolean", "int"/"integer", "long", "double"/"float"/"real", "string"/"text",
  //! any case). @param name name @param type type found @return false if the name is unknown
  static bool typeFromName(const String & name, Type & type);
  //! @brief Whether a string is a valid value of a type: a boolean ("0", "1", "true", "false", "yes", "no", any case),
  //! a decimal integer (within the int range for kInt), a number, or anything for kString. Surrounding blanks are
  //! ignored. @param value value @param type type @return true if valid
  static bool isValidValue(const String & value, Type type);
  //! @brief Value as stored for a type: trimmed, booleans as "1" or "0". @param value valid value @param type type
  //! @return stored value
  static String normalizedValue(const String & value, Type type);
  //! @}
  //! @brief Whether the name ends with a colon. @return true if so
  bool nameHasEndColon() const;
  //! @brief Print the name and value.
  void print() const;
  //! @brief Write the property as a "name : TYPE : value" line (TYPE is BOOL, INTEGER, DOUBLE or STRING). @param os stream
  virtual void saveTo(std::ostream & os) const;

  //! @brief Erase name, title and value.
  virtual void reset();
  //! @brief Erase name, title and value.
  virtual void clear();
  //! @brief Erase name, title and value.
  virtual void erase();

protected:
  String _value; //!< value, stored as a string
  Type   _type;  //!< type of the value

  ClassDef(Property,0)

};

} // namespace CAP

#endif /* CAP_PropertyGroup */


