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
#include "PythiaEventReader.hpp"
#include "TaskRegistration.hpp"

CAP_REFLECTION(PythiaTasks)
{
  CAP::registerTask<CAP::PythiaEventGenerator>("CAP::PythiaEventGenerator");
  CAP::registerTask<CAP::PythiaEventReader>("CAP::PythiaEventReader");
}
