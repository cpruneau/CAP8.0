/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file ParticleSingleTasks.cpp
//! @brief Task classes of the ParticleSingle library, registered for Task::instantiateTask() (see TaskRegistration.hpp).
#include "ParticleSingleAnalyzer.hpp"
#include "TaskRegistration.hpp"

CAP_REFLECTION(ParticleSingleTasks)
{
  CAP::registerTask<CAP::ParticleSingleAnalyzer>("CAP::ParticleSingleAnalyzer");
}
