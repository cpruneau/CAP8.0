/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file PHSDTasks.cpp
//! @brief Task classes of the PHSD library, registered for Task::instantiateTask() (see TaskRegistration.hpp).
#include "PHSDEventReader.hpp"
#include "TaskRegistration.hpp"

CAP_REFLECTION(PHSDTasks)
{
  CAP::registerTask<CAP::PHSDEventReader>("CAP::PHSDEventReader");
}
