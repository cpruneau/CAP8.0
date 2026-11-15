/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "JetFluctuationAnalyzer.hpp"

namespace CAP
{

JetFluctuationAnalyzer::JetFluctuationAnalyzer()
:
JetTask<JetFluctuationHistos>(),
_mode(UnderlyingEventCones::Transverse),
_coneEtaMax(0.9),
_cones()
{
  appendClassName("JetFluctuationAnalyzer");
  setMinimumReportLevel(Object::kInfo);
  setName("JetFluctuationAnalyzer");
  setTitle("JetFluctuationAnalyzer");
}

void JetFluctuationAnalyzer::setDefaultConfiguration()
{
  JetTask<JetFluctuationHistos>::setDefaultConfiguration();
  _configuration.addProperty(createKey(name(),"UE","MODE"),String("TRANSVERSE"));
  _configuration.addProperty(createKey(name(),"UE","ETA_MAX"),0.9);
}

void JetFluctuationAnalyzer::configure(const Configuration & configuration)
{
  JetTask<JetFluctuationHistos>::configure(configuration);
  _mode = UnderlyingEventCones::modeOf(configuration.valueString(createKey(name(),"UE","MODE")));
  _coneEtaMax = configuration.valueDouble(createKey(name(),"UE","ETA_MAX"));
  for (const JetSettings * s : jetSettings())
    if (!(s->radius<_coneEtaMax))
      throw ConfigurationException(createKey(name(),"UE","ETA_MAX"),String("UE:ETA_MAX not above the radius of the jet ")
                                   +"settings "+s->name()+": no cone of the underlying event would be kept",__FUNCTION__);
}

void JetFluctuationAnalyzer::fill(unsigned int first, const std::vector<Jet> & jets,
                                  const std::vector<const Jet*> & accepted, const std::vector<Particle*> & particles,
                                  const JetSettings & settings)
{
  _cones.clear();
  for (const Cone & cone : UnderlyingEventCones::cones(jets,settings.radius,_coneEtaMax,_mode))
    _cones.push_back(UnderlyingEventCones::count(particles,cone,settings.radius));
  JetFluctuationHistos * folder = _folders[first];
  folder->fillEvent(_cones.size());
  for (const Jet * jet : accepted) folder->fill(*jet,_cones);
}

void JetFluctuationAnalyzer::calculateDerived()
{
  for (JetFluctuationHistos * folder : _folders) folder->calculateDerived();
}

void JetFluctuationAnalyzer::finalize()
{
  calculateDerived();
  JetTask<JetFluctuationHistos>::finalize();
}

} // namespace CAP
