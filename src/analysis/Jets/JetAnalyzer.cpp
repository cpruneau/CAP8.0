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

namespace CAP
{

JetAnalyzer::JetAnalyzer()
:
JetTask<JetHistos>()
{
  appendClassName("JetAnalyzer");
  setMinimumReportLevel(Object::kInfo);
  setName("JetAnalyzer");
  setTitle("JetAnalyzer");
}

void JetAnalyzer::fill(unsigned int first, const std::vector<Jet> & jets __attribute__((unused)),
                       const std::vector<const Jet*> & accepted,
                       const std::vector<Particle*> & particles __attribute__((unused)),
                       const JetSettings & settings __attribute__((unused)))
{
  _folders[first]->fill(accepted);
}

} // namespace CAP
