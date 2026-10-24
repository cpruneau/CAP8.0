/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "EventPlaneRandomizerTask.hpp"
#include "PrintHelpers.hpp"
#include "MathConstants.hpp"

ClassImp(CAP::EventPlaneRandomizerTask);

namespace CAP
{

EventPlaneRandomizerTask::EventPlaneRandomizerTask()
:
EventProcessor()
{
  appendClassName("EventPlaneRandomizerTask");
  setMinimumReportLevel(Object::kInfo);
  setName("EventPlaneRandomizerTask");
  setTitle("EventPlaneRandomizerTask");
}

void EventPlaneRandomizerTask::execute()
{
  double eventAngle= CAP::MATH::twoPi() * gRandom->Rndm();
  Event & theEvent = event();
  for (auto & particle : theEvent.particles())
    {
    particle->position().rotateZ(eventAngle);
    particle->momentum().rotateZ(eventAngle);
    }
}

} // namespace CAP
