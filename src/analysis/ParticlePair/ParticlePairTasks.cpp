/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file ParticlePairTasks.cpp
//! @brief Task classes of the ParticlePair library, registered for Task::instantiateTask() (see TaskRegistration.hpp).
#include "ParticlePairAnalyzer.hpp"
#include "TaskRegistration.hpp"

CAP_REFLECTION(ParticlePairTasks)
{
  CAP::registerTask<CAP::ParticlePairAnalyzer>("CAP::ParticlePairAnalyzer");
}
