/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "JetAnalyzer.hpp"
#include "JetFilter.hpp"
#include "JetSettings.hpp"
#include "JetPairAnalyzer.hpp"
#include "JetGroomer.hpp"
#include "JetGroomingAnalyzer.hpp"
#include "TaskRegistration.hpp"

DATUM_REFLECTION(JetTasks)
{
  Datum::registerTask<Datum::JetAnalyzer>("Datum::JetAnalyzer");
  Datum::registerTask<Datum::JetSettingsCreator>("Datum::JetSettingsCreator");
  Datum::registerTask<Datum::JetFilterCreator>("Datum::JetFilterCreator");
  Datum::registerTask<Datum::JetPairAnalyzer>("Datum::JetPairAnalyzer");
  Datum::registerTask<Datum::JetPairFilterCreator>("Datum::JetPairFilterCreator");
  Datum::registerTask<Datum::JetGroomerCreator>("Datum::JetGroomerCreator");
  Datum::registerTask<Datum::JetGroomingAnalyzer>("Datum::JetGroomingAnalyzer");
}
