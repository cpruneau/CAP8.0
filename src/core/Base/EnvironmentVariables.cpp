/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "EnvironmentVariables.hpp"
#include "FileSystem.hpp"
#include "PrintHelpers.hpp"


namespace Datum
{

EnvironmentVariables::EnvironmentVariables()
:
Properties()
{
  initialize();
}


void EnvironmentVariables::initialize()
{
  // fetch required env variables from the system
  // store them for easy later retrieval
  addVariable("DATUM_DATABASE_PATH",      FileSystem::environment("DATUM_DATABASE_PATH"));
  addVariable("DATUM_PROJECTS_PATH",      FileSystem::environment("DATUM_PROJECTS_PATH"));
  addVariable("DATUM_DATA_IMPORT_PATH",   FileSystem::environment("DATUM_DATA_IMPORT_PATH"));
  addVariable("DATUM_DATA_EXPORT_PATH",   FileSystem::environment("DATUM_DATA_EXPORT_PATH"));
  addVariable("DATUM_HISTOS_IMPORT_PATH", FileSystem::environment("DATUM_HISTOS_IMPORT_PATH"));
  addVariable("DATUM_HISTOS_EXPORT_PATH", FileSystem::environment("DATUM_HISTOS_EXPORT_PATH"));
  addVariable("DATUM_CALIB_IMPORT_PATH",  FileSystem::environment("DATUM_CALIB_IMPORT_PATH"));
  addVariable("DATUM_CALIB_EXPORT_PATH",  FileSystem::environment("DATUM_CALIB_EXPORT_PATH"));
}

void EnvironmentVariables::print() const
{
  printValue("DATUM_DATABASE_PATH",      valueString("DATUM_DATABASE_PATH"));
  printValue("DATUM_PROJECTS_PATH",      valueString("DATUM_PROJECTS_PATH"));
  printValue("DATUM_DATA_IMPORT_PATH",   valueString("DATUM_DATA_IMPORT_PATH"));
  printValue("DATUM_DATA_EXPORT_PATH",   valueString("DATUM_DATA_EXPORT_PATH"));
  printValue("DATUM_HISTOS_IMPORT_PATH", valueString("DATUM_HISTOS_IMPORT_PATH"));
  printValue("DATUM_HISTOS_EXPORT_PATH", valueString("DATUM_HISTOS_EXPORT_PATH"));
  printValue("DATUM_CALIB_IMPORT_PATH",  valueString("DATUM_CALIB_IMPORT_PATH"));
  printValue("DATUM_CALIB_EXPORT_PATH",  valueString("DATUM_CALIB_EXPORT_PATH"));
}

void  EnvironmentVariables::addVariable(const char* aKeyword, const char* aValue)
{
  addProperty(aKeyword, aValue);
}


String EnvironmentVariables::variable(const char* aKeyword) const
{
  return valueString(aKeyword);
}

EnvironmentVariables * EnvironmentVariables::_envVariables = nullptr;

EnvironmentVariables * EnvironmentVariables::environmentVariables()
{
  if (_envVariables==nullptr)
    {
    _envVariables = new EnvironmentVariables();
    }
  return _envVariables;
}

} // namespace Datum

