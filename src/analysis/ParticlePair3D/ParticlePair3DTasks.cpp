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
#include "ParticlePair3DBFHistos.hpp"
#include "TaskRegistration.hpp"

DATUM_REFLECTION(ParticlePair3DTasks)
{
  Datum::registerTask<Datum::ParticlePair3DAnalyzer>("Datum::ParticlePair3DAnalyzer");
  Datum::registerTask<Datum::ParticlePair3DDerivedCalculator>("Datum::ParticlePair3DDerivedCalculator");
  Datum::registerTask<Datum::ParticlePair3DBFCalculator>("Datum::ParticlePair3DBFCalculator");
}
