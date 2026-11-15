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
#include "JetFluctuationAnalyzer.hpp"
#include "JetGroomer.hpp"
#include "JetGroomingAnalyzer.hpp"
#include "TaskRegistration.hpp"

CAP_REFLECTION(JetTasks)
{
  CAP::registerTask<CAP::JetAnalyzer>("CAP::JetAnalyzer");
  CAP::registerTask<CAP::JetSettingsCreator>("CAP::JetSettingsCreator");
  CAP::registerTask<CAP::JetFilterCreator>("CAP::JetFilterCreator");
  CAP::registerTask<CAP::JetPairAnalyzer>("CAP::JetPairAnalyzer");
  CAP::registerTask<CAP::JetPairFilterCreator>("CAP::JetPairFilterCreator");
  CAP::registerTask<CAP::JetFluctuationAnalyzer>("CAP::JetFluctuationAnalyzer");
  CAP::registerTask<CAP::JetGroomerCreator>("CAP::JetGroomerCreator");
  CAP::registerTask<CAP::JetGroomingAnalyzer>("CAP::JetGroomingAnalyzer");
}
