/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "Object.hpp"
#include "PrintHelpers.hpp"

using std::cout;
using std::endl;

ClassImp(CAP::Object);

namespace CAP
{

  String Object::kUnknownName   = "UNKNOWN";
  String Object::kTraceName     = "TRACE";
  String Object::kEndName       = "ENDF";
  String Object::kStartName     = "STARTF";
  String Object::kDebug0Name    = "DEBUG0";
  String Object::kDebug1Name    = "DEBUG1";
  String Object::kDebug2Name    = "DEBUG2";
  String Object::kInfoName      = "INFO";
  String Object::kWarningName   = "WARNING";
  String Object::kErrorName     = "ERROR";
  String Object::kFatalName     = "FATAL";

  Object::Object()
  :
  _name("Object"),
  _title("Object"),
  _className("Object"),
  _minimumReportLevel(kInfo),
  _holdMinimumReportLevel(kInfo),
  _loggerStyle(0),
  _index(0)
  { /* no ops */}


  Object::Object(const String & name, const String & title, const String & className)
  :
  _name(name),
  _title(title),
  _className(className),
  _minimumReportLevel(kInfo),
  _holdMinimumReportLevel(kInfo),
  _loggerStyle(0),
  _index(0)
  {}

  Object::Object(const Object & source)
  :
  _name(source._name),
  _title(source._title),
  _className(source._className),
  _minimumReportLevel(source._minimumReportLevel),
  _holdMinimumReportLevel(source._holdMinimumReportLevel),
  _loggerStyle(source._loggerStyle),
  _index(source._index)
  { /* no ops */}

  Object & Object::operator=(const Object & rhs)
  {
  if (this!=&rhs)
    {
    _name = rhs._name;
    _title = rhs._title;
    _className = rhs._className;
    _minimumReportLevel = rhs._minimumReportLevel;
    _holdMinimumReportLevel = rhs._holdMinimumReportLevel;
    _loggerStyle = rhs._loggerStyle;
    _index = rhs._index;
    }
  return *this;
  }

  Object::~Object()
  { }

  bool Object::nameHasEndColon() const
  {
  return _name.endsWith(":");
  }

  void Object::print()  const
  {
  printValue("Name",_name);
  printValue("Title",_title);
  printValue("Class",_className);
  }

  bool Object::report(Severity severity, const String & fctName) const
  {
  if (severity<_minimumReportLevel) return false;
  cout << "<" << severityName(severity) << "> " << _className << "[" << name() << "]::" << fctName << "() ";
  return true;
  }

  void Object::setSeverityName(Severity level, const String name)
  {
  switch (level)
    {
      case kUnknown: kUnknownName = name; break;
      case kTrace:   kTraceName   = name; break;
      case kEnd:     kEndName     = name; break;
      case kStart:   kStartName   = name; break;
      case kDebug0:  kDebug0Name  = name; break;
      case kDebug1:  kDebug1Name  = name; break;
      case kDebug2:  kDebug2Name  = name; break;
      case kInfo:    kInfoName    = name; break;
      case kWarning: kWarningName = name; break;
      case kError:   kErrorName   = name; break;
      case kFatal:   kFatalName   = name; break;
    }
  }

  void Object::setMinimumReportLevel(const String & severityName)
  {
  if (severityName.equalTo(kTraceName))
    setMinimumReportLevel(kTrace);
  else if (severityName.equalTo(kEndName))
    setMinimumReportLevel(kEnd);
  else if (severityName.equalTo(kStartName))
    setMinimumReportLevel(kStart);
  else if (severityName.equalTo(kDebug0Name))
    setMinimumReportLevel(kDebug0);
  else if (severityName.equalTo(kDebug1Name))
    setMinimumReportLevel(kDebug1);
  else if (severityName.equalTo(kDebug2Name))
    setMinimumReportLevel(kDebug2);
  else if (severityName.equalTo(kInfoName))
    setMinimumReportLevel(kInfo);
  else if (severityName.equalTo(kWarningName))
    setMinimumReportLevel(kWarning);
  else if (severityName.equalTo(kErrorName))
    setMinimumReportLevel(kError);
  else if (severityName.equalTo(kFatalName))
    setMinimumReportLevel(kFatal);
  else if (severityName.length()>0 && !severityName.equalTo("NONE"))
    {
    // Unrecognized name: keep the current level but say so ("NONE" is the value returned
    // by Properties::valueString() for a missing key, so it stays silent).
    if (report(kWarning,__FUNCTION__))
      std::cout << "Unknown severity name: " << severityName << " -- report level left at " << minimumReportName() << endl;
    }
  }



} // namespace CAP
