/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file ParticlePair3DTasks.cpp
//! @brief Task classes of the ParticlePair3D library, registered for Task::instantiateTask() (see TaskRegistration.hpp).
#include "ParticlePair3DAnalyzer.hpp"
#include "ParticlePair3DDerivedCalculator.hpp"
#include "TaskRegistration.hpp"

CAP_REFLECTION(ParticlePair3DTasks)
{
  CAP::registerTask<CAP::ParticlePair3DAnalyzer>("CAP::ParticlePair3DAnalyzer");
  CAP::registerTask<CAP::ParticlePair3DDerivedCalculator>("CAP::ParticlePair3DDerivedCalculator");
}
