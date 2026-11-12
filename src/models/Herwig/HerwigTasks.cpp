/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file HerwigTasks.cpp
//! @brief Task classes of the Herwig library, registered for Task::instantiateTask() (see TaskRegistration.hpp).
#include "HerwigEventGenerator.hpp"
#include "TaskRegistration.hpp"

CAP_REFLECTION(HerwigTasks)
{
  CAP::registerTask<CAP::HerwigEventGenerator>("CAP::HerwigEventGenerator");
}
