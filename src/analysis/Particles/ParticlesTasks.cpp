/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file ParticlesTasks.cpp
//! @brief Task classes of the Particles library, registered for Task::instantiateTask() (see TaskRegistration.hpp).
#include "EventIterator.hpp"
#include "EventMerger.hpp"
#include "EventProcessor.hpp"
#include "FileIterator.hpp"
#include "ObjectCreator.hpp"
#include "ParticleDecayerTask.hpp"
#include "TaskRegistration.hpp"

DATUM_REFLECTION(ParticlesTasks)
{
  Datum::registerTask<Datum::EventIterator>("Datum::EventIterator");
  Datum::registerTask<Datum::EventMerger>("Datum::EventMerger");
  Datum::registerTask<Datum::EventProcessor>("Datum::EventProcessor");
  Datum::registerTask<Datum::FileIterator>("Datum::FileIterator");
  Datum::registerTask<Datum::ParticleDecayerTask>("Datum::ParticleDecayerTask");
  Datum::registerTask<Datum::FilterCreator>("Datum::FilterCreator");
  Datum::registerTask<Datum::EventFilterCreator>("Datum::EventFilterCreator");
  Datum::registerTask<Datum::ParticleFilterCreator>("Datum::ParticleFilterCreator");
  Datum::registerTask<Datum::EventEfficiencyCreator>("Datum::EventEfficiencyCreator");
  Datum::registerTask<Datum::ParticleEfficiencyCreator>("Datum::ParticleEfficiencyCreator");
  Datum::registerTask<Datum::EventSmearerCreator>("Datum::EventSmearerCreator");
  Datum::registerTask<Datum::ParticleSmearerCreator>("Datum::ParticleSmearerCreator");
  Datum::registerTask<Datum::EventCreator>("Datum::EventCreator");
  Datum::registerTask<Datum::ParticleDbCreator>("Datum::ParticleDbCreator");
}
