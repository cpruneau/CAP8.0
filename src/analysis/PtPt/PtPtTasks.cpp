/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file PtPtTasks.cpp
//! @brief Task classes of the PtPt library, registered for Task::instantiateTask() (see TaskRegistration.hpp).
#include "PtPtAnalyzer.hpp"
#include "TaskRegistration.hpp"

CAP_REFLECTION(PtPtTasks)
{
  CAP::registerTask<CAP::PtPtAnalyzer>("CAP::PtPtAnalyzer");
}
