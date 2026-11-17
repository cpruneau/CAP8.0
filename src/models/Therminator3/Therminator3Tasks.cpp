/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Therminator3Tasks.cpp
//! @brief Task classes of the Therminator3 library, registered for Task::instantiateTask() (see TaskRegistration.hpp).
#include "ThermalModel.hpp"
#include "Therminator3.hpp"
#include "TaskRegistration.hpp"

DATUM_REFLECTION(Therminator3Tasks)
{
  Datum::registerTask<Datum::ThermalModel>("Datum::ThermalModel");
  Datum::registerTask<Datum::Therminator3>("Datum::Therminator3");
}
