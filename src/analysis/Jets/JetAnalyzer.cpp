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
JetTask<JetHistos>(),
_mode(UnderlyingEventCones::Transverse),
_coneEtaMax(0.9),
_cones()
{
  appendClassName("JetAnalyzer");
  setMinimumReportLevel(Object::kInfo);
  setName("JetAnalyzer");
  setTitle("JetAnalyzer");
}

void JetAnalyzer::setDefaultConfiguration()
{
  JetTask<JetHistos>::setDefaultConfiguration();
  _configuration.addProperty(createKey(name(),"UE","MODE"),String("TRANSVERSE"));
  _configuration.addProperty(createKey(name(),"UE","ETA_MAX"),0.9);
}

void JetAnalyzer::configure(const Configuration & configuration)
{
  JetTask<JetHistos>::configure(configuration);
  _mode = UnderlyingEventCones::modeOf(configuration.valueString(createKey(name(),"UE","MODE")));
  _coneEtaMax = configuration.valueDouble(createKey(name(),"UE","ETA_MAX"));
  for (const JetSettings * s : jetSettings())
    if (!(s->radius<_coneEtaMax) && reportWarning(__FUNCTION__))
      printValue(String("No cone of the underlying event for the jet settings ")+s->name()
                 +String(" (R not below UE:ETA_MAX)"),_coneEtaMax);
}

void JetAnalyzer::fill(unsigned int first, const std::vector<Jet> & jets, const std::vector<const Jet*> & accepted,
                       const std::vector<Particle*> & particles, const JetSettings & settings)
{
  _cones.clear();
  if (settings.radius<_coneEtaMax)
    for (const Cone & cone : UnderlyingEventCones::cones(jets,settings.radius,_coneEtaMax,_mode))
      _cones.push_back(UnderlyingEventCones::count(particles,cone,settings.radius));
  _folders[first]->fill(accepted,_cones);
}

} // namespace CAP
