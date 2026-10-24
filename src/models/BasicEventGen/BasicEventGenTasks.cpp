/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file BasicEventGenTasks.cpp
//! @brief Task classes of the BasicEventGen library, registered for Task::instantiateTask() (see TaskRegistration.hpp).
#include "BasicEventGen.hpp"
#include "TaskRegistration.hpp"

CAP_REFLECTION(BasicEventGenTasks)
{
  CAP::registerTask<CAP::BasicEventGen>("CAP::BasicEventGen");
}
