/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__Object
#define CAP__Object
#include "Aliases.hpp"
#include "Exceptions.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Base class of most CAP classes: a named object with message reporting.
//!
//! The object class defines the notion of object from which many constructs inherit.
//! An object has a name and a title, a class name (used in messages and, for tasks, to create objects by name) and a
//! version index. A property inherits from object and adds a value.
//!
//! <b>Reporting.</b> Each object has a minimum report level (a Severity, kInfo by default). report() and its shortcuts
//! (reportDebug(), reportInfo(), reportWarning(), ...) return whether a message of a given severity should be issued
//! and, if so, print its header "<SEVERITY> className[name]::function() "; the caller then prints the message itself:
//! @code
//!   if (reportWarning(__FUNCTION__)) std::cout << "no events found" << std::endl;
//! @endcode
//! Severity names ("TRACE", "DEBUG0", "INFO", "WARNING", ...) are used in messages and configurations; they can be
//! changed with setSeverityName().
//!
class Object
{
public:

  //! @brief Severity levels of messages, in increasing order.
  enum Severity
  {
  kUnknown,
  kTrace,
  kEnd,
  kStart,
  kDebug0,
  kDebug1,
  kDebug2,
  kInfo,
  kWarning,
  kError,
  kFatal
  };

  //! @brief Default constructor: name "Object", report level kInfo.
  Object();
  //! @brief Constructor. @param name name @param title title @param className class name
  Object(const String & name, const String & title, const String & className);
  //! @brief Copy constructor. @param object object to copy
  Object(const Object & object);
  //! @brief Copy assignment. @param object object to copy @return this object
  Object & operator=(const Object & object);
  //! @brief Destructor.
  virtual ~Object();
  //! @brief Set the name. @param name new name
  virtual void setName(const String & name) { _name = name; }
  //! @brief Name of the object. @return name
  virtual const String & name() const {  return _name; }
  //! @brief Whether this object has a non-empty name. @return true if the name is not empty
  virtual bool isNamed() const {  return _name.length()>0; }
  //! @brief Whether this object has the given name. @param name name to compare with @return true if equal
  virtual bool isNamed(const String & name) const { return _name.equalTo(name); }
  //! @brief Whether another object has the same name. @param object object @return true if so
  virtual bool sameNameAs(const Object & object) const { return _name.equalTo(object._name); }
  //! @brief Set the title. @param title new title
  virtual void setTitle(const String & title) { _title = title; }
  //! @brief Whether another object has the same title. @param object object @return true if so
  virtual bool sameTitleAs(const Object & object) const {return _title.equalTo(object._title); }
  //! @brief Title of the object. @return title
  virtual const String & title() const { return _title; }
  //! @brief Whether the name begins with a string. @param aKeyword string @return true if so
  virtual bool nameBeginsWith(const char * aKeyword) const {  return _name.beginsWith(aKeyword); }
  //! @brief Whether the name ends with a string. @param aKeyword string @return true if so
  virtual bool nameEndsWith(const char * aKeyword) const {  return _name.endsWith(aKeyword); }
  //! @brief Whether the name contains a string. @param aKeyword string @return true if so
  virtual bool nameContains(const char * aKeyword) const {return _name.contains(aKeyword); }
  //! @brief Whether the name ends with a colon. @return true if so
  virtual bool nameHasEndColon() const;
  //! @brief Set the class name. @param className new class name
  virtual void setClassName(const String & className) { _className = className; }
  //! @brief Class name of the object (possibly a sequence "Base/Derived", see appendClassName()). @return class name
  virtual const String & className() const { return _className; }
  //! @brief Version index of this object (0 by default). @return index
  unsigned int index() const { return _index; }
  //! @brief Set the version index of this object. @param index new index
  void setIndex(unsigned int index) { _index = index; }
  //! @brief Print the class name, name and title.
  virtual void print(  ) const;
  //! @brief Reset the content of the object. Does nothing here; derived classes override it.
  virtual void reset() {}
  //! @brief Clear the content of the object. Does nothing here: the name, title, class name and report level
  //! identify the object and are kept. Derived classes override it to clear their own data.
  virtual void clear() {}

  //!
  //! Set the desired reported level used by instances (objects) of this class or subclasses.
  //!
  inline void setMinimumReportLevel(Severity minimumReportLevel)
  {
  _minimumReportLevel = minimumReportLevel;
  }

  //! @brief Set the minimum report level from its name (see severityName()). An unknown name leaves the level unchanged,
  //! with a warning unless the name is empty or "NONE". @param severityName name of the level
  void setMinimumReportLevel(const String & severityName);

  //!
  //! Get the desired reported level used by instances (objects) of this class or subclasses.
  //!
  inline Severity minimumReportLevel() const
  {
  return _minimumReportLevel;
  }

  //! @brief Name of the minimum report level. @return name
  inline String  minimumReportName() const
  {
  return severityName(_minimumReportLevel);
  }

  //! @brief Name of a severity level (as set with setSeverityName()). @param level severity level @return its name
  static String severityName(Severity level)
  {
  String s;
  switch (level)
    {
      case kFatal:    s = kFatalName;   break;
      case kError:    s = kErrorName;   break;
      case kWarning:  s = kWarningName; break;
      case kInfo:     s = kInfoName;    break;
      case kDebug2:   s = kDebug2Name;  break;
      case kDebug1:   s = kDebug1Name;  break;
      case kDebug0:   s = kDebug0Name;  break;
      case kStart:    s = kStartName;   break;
      case kEnd:      s = kEndName;     break;
      case kTrace:    s = kTraceName;   break;
      default:
      case kUnknown:  s = kUnknownName; break;
    }
  return s;
  }

  //!
  //! Temporarily set the report level to requiredLevel; restoreSeverityLevel() returns to the previous level.
  //! Only one level is saved: holds do not nest (a second hold before a restore loses the first saved level).
  //!
  inline void holdSeverityLevel(Severity requiredLevel)
  {
  _holdMinimumReportLevel = _minimumReportLevel;
  _minimumReportLevel     = requiredLevel;
  }

  //!
  //!Restore the nominal report level requested by the user.
  //!
  inline void restoreSeverityLevel()
  {
  _minimumReportLevel = _holdMinimumReportLevel;
  }

  //! @brief Set the logging style (stored, not used yet). @param s style
  inline void setLoggingStyle(unsigned int s)
  {
  _loggerStyle = s;
  }

  //! @brief Logging style. @return style
  inline unsigned int  loggingStyle() const
  {
  return _loggerStyle;
  }

  //!
  //!Append the given string to the class name sequence of this object instance.
  //!
  inline void appendClassName(const String & className)
  {
  _className += "/";
  _className += className;
  }

  //!
  //! @brief Whether to issue a message of severity kTrace (prints the header if so). @param fctName function name
  //! @return true if so
  //!
  bool reportTrace(const String & fctName) const
  {
  return report(kTrace,fctName);
  }

  //! @brief Whether to issue a message of severity kEnd (prints the header if so). @param fctName function name @return true if so
  bool reportEnd(const String & fctName) const
  {
  return report(kEnd,fctName);
  }

  //! @brief Whether to issue a message of severity kStart (prints the header if so). @param fctName function name @return true if so
  bool reportStart(const String & fctName) const
  {
  return report(kStart,fctName);
  }

  //! @brief Whether to issue a message of severity kDebug0 (prints the header if so). @param fctName function name @return true if so
  inline bool reportDebug0(const String & fctName) const
  {
  return report(kDebug0,fctName);
  }

  //! @brief Whether to issue a message of severity kDebug1 (prints the header if so). @param fctName function name @return true if so
  inline bool reportDebug1(const String & fctName) const
  {
  return report(kDebug1,fctName);
  }

  //! @brief Whether to issue a message of severity kDebug2 (prints the header if so). @param fctName function name @return true if so
  inline bool reportDebug2(const String & fctName) const
  {
  return report(kDebug2,fctName);
  }

  //! @brief Same as reportDebug0(). @param fctName function name @return true if the message is issued
  inline bool reportDebug(const String & fctName) const
  {
  return report(kDebug0,fctName);
  }

  //! @brief Whether to issue a message of severity kInfo (prints the header if so). @param fctName function name @return true if so
  inline bool reportInfo(const String & fctName) const
  {
  return report(kInfo,fctName);
  }

  //! @brief Whether to issue a message of severity kWarning (prints the header if so). @param fctName function name @return true if so
  bool reportWarning(const String & fctName) const
  {
  return report(kWarning,fctName);
  }

  //! @brief Whether to issue a message of severity kError (prints the header if so). @param fctName function name @return true if so
  bool reportError(const String & fctName) const
  {
  return report(kError,fctName);
  }

  //! @brief Whether to issue a message of severity kFatal (prints the header if so). @param fctName function name @return true if so
  bool reportFatal(const String & fctName) const
  {
  return report(kFatal,fctName);
  }

  //!
  //! @brief Whether a message of the given severity is issued (severity not below the minimum report level); if so, its
  //! header "<SEVERITY> className[name]::fctName() " is printed to std::cout.
  //! @param severity severity of the message @param fctName name of the calling function @return true if issued
  //!
  bool report(Severity severity, const String & fctName) const;


  //! @name Severity names ("UNKNOWN", "TRACE", "STARTF", "ENDF", "DEBUG0", "DEBUG1", "DEBUG2", "INFO", "WARNING", "ERROR", "FATAL")
  //! @{
  static String kUnknownName;
  static String kTraceName;
  static String kStartName;
  static String kEndName;
  static String kDebug0Name;
  static String kDebug1Name;
  static String kDebug2Name;
  static String kInfoName;
  static String kWarningName;
  static String kErrorName;
  static String kFatalName;
  //! @}
  //! @brief Change the name of a severity level. @param level level @param name new name
  static void setSeverityName(Severity level, const String name);

protected:
  String _name;  //!< name
  String _title;  //!< title
  String _className;  //!< class name
  Severity  _minimumReportLevel;      //!< Minimal error level reported by object instances inheriting from this class.
  Severity  _holdMinimumReportLevel;  //!< Internal variable used internally to set/reset the nominal report level selected by the user.
  unsigned int _loggerStyle;  //!< logging style (not used yet)
  unsigned int _index;                //!< Version index of this object instance.

  ClassDef(Object,0)
};

} // namespace CAP


#endif // !CAP__Object

