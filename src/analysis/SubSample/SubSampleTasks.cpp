/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file SubSampleTasks.cpp
//! @brief Task classes of the SubSample library, registered for Task::instantiateTask() (see TaskRegistration.hpp).
#include "SubSampleStatCalculator.hpp"
#include "TaskRegistration.hpp"

CAP_REFLECTION(SubSampleTasks)
{
  CAP::registerTask<CAP::SubSampleStatCalculator>("CAP::SubSampleStatCalculator");
}
