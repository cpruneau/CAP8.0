/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file PerformanceTasks.cpp
//! @brief Task classes of the Performance library, registered for Task::instantiateTask() (see TaskRegistration.hpp).
#include "CalibrationProducer.hpp"
#include "ClosureCalculator.hpp"
#include "ClosureIterator.hpp"
#include "EventPlaneRandomizerTask.hpp"
#include "EventVertexRandomizerTask.hpp"
#include "MeasurementSimulator.hpp"
#include "PerformanceAnalyzer.hpp"
#include "ParticlePerformanceHistos.hpp"
#include "TaskRegistration.hpp"

DATUM_REFLECTION(PerformanceTasks)
{
  Datum::registerTask<Datum::CalibrationProducer>("Datum::CalibrationProducer");
  Datum::registerTask<Datum::ClosureCalculator>("Datum::ClosureCalculator");
  Datum::registerTask<Datum::ClosureIterator>("Datum::ClosureIterator");
  Datum::registerTask<Datum::EventPlaneRandomizerTask>("Datum::EventPlaneRandomizerTask");
  Datum::registerTask<Datum::EventVertexRandomizerTask>("Datum::EventVertexRandomizerTask");
  Datum::registerTask<Datum::MeasurementSimulator>("Datum::MeasurementSimulator");
  Datum::registerTask<Datum::PerformanceAnalyzer>("Datum::PerformanceAnalyzer");
  Datum::registerTask<Datum::PerformanceDerivedCalculator>("Datum::PerformanceDerivedCalculator");
}
