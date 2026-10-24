/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file NuDynTasks.cpp
//! @brief Task classes of the NuDyn library, registered for Task::instantiateTask() (see TaskRegistration.hpp).
#include "NuDynAnalyzer.hpp"
#include "TaskRegistration.hpp"

CAP_REFLECTION(NuDynTasks)
{
  CAP::registerTask<CAP::NuDynAnalyzer>("CAP::NuDynAnalyzer");
}
