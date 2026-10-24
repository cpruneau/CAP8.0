/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file PerformanceTasks.cpp
//! @brief Task classes of the Performance library, registered for Task::instantiateTask() (see TaskRegistration.hpp).
#include "ClosureCalculator.hpp"
#include "ClosureIterator.hpp"
#include "EventPlaneRandomizerTask.hpp"
#include "EventVertexRandomizerTask.hpp"
#include "MeasurementSimulator.hpp"
#include "PerformanceAnalyzer.hpp"
#include "TaskRegistration.hpp"

CAP_REFLECTION(PerformanceTasks)
{
  CAP::registerTask<CAP::ClosureCalculator>("CAP::ClosureCalculator");
  CAP::registerTask<CAP::ClosureIterator>("CAP::ClosureIterator");
  CAP::registerTask<CAP::EventPlaneRandomizerTask>("CAP::EventPlaneRandomizerTask");
  CAP::registerTask<CAP::EventVertexRandomizerTask>("CAP::EventVertexRandomizerTask");
  CAP::registerTask<CAP::MeasurementSimulator>("CAP::MeasurementSimulator");
  CAP::registerTask<CAP::PerformanceAnalyzer>("CAP::PerformanceAnalyzer");
}
