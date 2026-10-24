/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file GlobalTasks.cpp
//! @brief Task classes of the Global library, registered for Task::instantiateTask() (see TaskRegistration.hpp).
#include "GlobalAnalyzer.hpp"
#include "TaskRegistration.hpp"

CAP_REFLECTION(GlobalTasks)
{
  CAP::registerTask<CAP::GlobalAnalyzer>("CAP::GlobalAnalyzer");
}
