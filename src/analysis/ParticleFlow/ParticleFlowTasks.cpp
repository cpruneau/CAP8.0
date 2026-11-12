/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "ParticleFlowAnalyzer.hpp"
#include "TaskRegistration.hpp"

CAP_REFLECTION(ParticleFlowTasks)
{
  CAP::registerTask<CAP::ParticleFlowAnalyzer>("CAP::ParticleFlowAnalyzer");
  CAP::registerTask<CAP::ParticleFlowDerivedCalculator>("CAP::ParticleFlowDerivedCalculator");
}
