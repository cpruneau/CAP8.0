/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file SpreadsheetTasks.cpp
//! @brief Task classes of the Spreadsheet library, registered for Task::instantiateTask() (see TaskRegistration.hpp).
#include "SpreadsheetAnalyzer.hpp"
#include "TaskRegistration.hpp"

DATUM_REFLECTION(SpreadsheetTasks)
{
  Datum::registerTask<Datum::SpreadsheetAnalyzer>("Datum::SpreadsheetAnalyzer");
}
