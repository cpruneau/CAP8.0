/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file SpherocityTasks.cpp
//! @brief Task classes of the Spherocity library, registered for Task::instantiateTask() (see TaskRegistration.hpp).
#include "SpherocityAnalyzer.hpp"
#include "TaskRegistration.hpp"

CAP_REFLECTION(SpherocityTasks)
{
  CAP::registerTask<CAP::SpherocityAnalyzer>("CAP::SpherocityAnalyzer");
}
