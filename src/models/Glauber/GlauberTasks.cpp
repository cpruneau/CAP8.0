/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file GlauberTasks.cpp
//! @brief Task classes of the Glauber library, registered for Task::instantiateTask() (see TaskRegistration.hpp).
#include "GlauberAnalyzer.hpp"
#include "GlauberGenerator.hpp"
#include "TaskRegistration.hpp"

CAP_REFLECTION(GlauberTasks)
{
  CAP::registerTask<CAP::GlauberAnalyzer>("CAP::GlauberAnalyzer");
  CAP::registerTask<CAP::GlauberGenerator>("CAP::GlauberGenerator");
}
