/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
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
#include "ParticlePairBFHistos.hpp"
#include "TaskRegistration.hpp"

DATUM_REFLECTION(ParticlePairTasks)
{
  Datum::registerTask<Datum::ParticlePairAnalyzer>("Datum::ParticlePairAnalyzer");
  Datum::registerTask<Datum::ParticlePairDerivedCalculator>("Datum::ParticlePairDerivedCalculator");
  Datum::registerTask<Datum::ParticlePairBFCalculator>("Datum::ParticlePairBFCalculator");
}
