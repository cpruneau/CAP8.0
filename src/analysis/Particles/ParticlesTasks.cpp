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

CAP_REFLECTION(ParticlesTasks)
{
  CAP::registerTask<CAP::EventIterator>("CAP::EventIterator");
  CAP::registerTask<CAP::EventMerger>("CAP::EventMerger");
  CAP::registerTask<CAP::EventProcessor>("CAP::EventProcessor");
  CAP::registerTask<CAP::FileIterator>("CAP::FileIterator");
  CAP::registerTask<CAP::ParticleDecayerTask>("CAP::ParticleDecayerTask");
  CAP::registerTask<CAP::FilterCreator>("CAP::FilterCreator");
  CAP::registerTask<CAP::EventFilterCreator>("CAP::EventFilterCreator");
  CAP::registerTask<CAP::ParticleFilterCreator>("CAP::ParticleFilterCreator");
  CAP::registerTask<CAP::EventEfficiencyCreator>("CAP::EventEfficiencyCreator");
  CAP::registerTask<CAP::ParticleEfficiencyCreator>("CAP::ParticleEfficiencyCreator");
  CAP::registerTask<CAP::EventSmearerCreator>("CAP::EventSmearerCreator");
  CAP::registerTask<CAP::ParticleSmearerCreator>("CAP::ParticleSmearerCreator");
  CAP::registerTask<CAP::EventCreator>("CAP::EventCreator");
  CAP::registerTask<CAP::ParticleDbCreator>("CAP::ParticleDbCreator");
}
