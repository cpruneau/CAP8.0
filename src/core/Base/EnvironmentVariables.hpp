/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef DATUM__EnvironmentVariables
#define DATUM__EnvironmentVariables
#include "String.hpp"
#include "Properties.hpp"

namespace Datum
{

//!
//! @brief Singleton giving access to the environment variables used by Datum.
//!
//! The single instance, created on the first call to environmentVariables(), reads the following variables of the
//! operating system and stores them as properties: DATUM_DATABASE_PATH, DATUM_PROJECTS_PATH, DATUM_DATA_IMPORT_PATH,
//! DATUM_DATA_EXPORT_PATH, DATUM_HISTOS_IMPORT_PATH, DATUM_HISTOS_EXPORT_PATH, DATUM_CALIB_IMPORT_PATH and
//! DATUM_CALIB_EXPORT_PATH. Values are read once: later changes of the environment are not seen.
//!
//! The constructor is protected to implement this class as a singleton.
//!
//! Example:
//! @code
//!   String path = Datum::EnvironmentVariables::environmentVariables()->variable("DATUM_HISTOS_IMPORT_PATH");
//! @endcode
//!
class EnvironmentVariables : public Properties
{
public:

  //! @brief Destructor.
  virtual ~EnvironmentVariables() {}
  EnvironmentVariables(const EnvironmentVariables &) = delete;             // singleton: no copies
  EnvironmentVariables & operator=(const EnvironmentVariables &) = delete;
  //! @brief Value of a stored variable. @param aKeyword name of the variable @return value, or "NONE" if not stored
  String variable(const char* aKeyword) const;
  //! @brief Store a variable (replacing any previous value). @param aKeyword name of the variable @param aValue value
  void  addVariable(const char* aKeyword, const char* aValue);
  //! @brief Print the Datum environment variables.
  void print() const;

  //! @brief The single instance (created on the first call). @return instance
  static EnvironmentVariables * environmentVariables();

protected:

  //! @brief Constructor: reads the environment (see initialize()).
  EnvironmentVariables();
  static EnvironmentVariables * _envVariables; //!< the single instance

  //! @brief Read the Datum environment variables from the operating system.
  void initialize();

};

} // namespace Datum

#endif /* DATUM__Task */
