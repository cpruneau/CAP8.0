/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file PythiaTasks.cpp
//! @brief Task classes of the Pythia library, registered for Task::instantiateTask() (see TaskRegistration.hpp).
#include "PythiaEventGenerator.hpp"
#ifdef DATUM_WITH_ROOT
#include "PythiaEventReader.hpp"
#endif
#include "TaskRegistration.hpp"

DATUM_REFLECTION(PythiaTasks)
{
  Datum::registerTask<Datum::PythiaEventGenerator>("Datum::PythiaEventGenerator");
#ifdef DATUM_WITH_ROOT
  Datum::registerTask<Datum::PythiaEventReader>("Datum::PythiaEventReader");
#endif
}
