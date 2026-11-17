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

DATUM_REFLECTION(ParticleFlowTasks)
{
  Datum::registerTask<Datum::ParticleFlowAnalyzer>("Datum::ParticleFlowAnalyzer");
  Datum::registerTask<Datum::ParticleFlowDerivedCalculator>("Datum::ParticleFlowDerivedCalculator");
}
