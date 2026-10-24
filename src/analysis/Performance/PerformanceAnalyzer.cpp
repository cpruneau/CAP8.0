/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "PerformanceAnalyzer.hpp"
#include "ParticlePerformanceHistos.hpp"
#include "PrintHelpers.hpp"
#include "NameHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"

ClassImp(CAP::PerformanceAnalyzer);

namespace CAP
{

PerformanceAnalyzer::PerformanceAnalyzer()
:
EventProcessorSingle<ParticlePerformanceHistos,ParticlePerformanceDerivedHistos>()
{
  appendClassName("PerformanceAnalyzer");
  setMinimumReportLevel(Object::kInfo);
  setName("PerformanceAnalyzer");
  setTitle("PerformanceAnalyzer");
}

PerformanceAnalyzer::PerformanceAnalyzer(const PerformanceAnalyzer & source)
:
EventProcessorSingle<ParticlePerformanceHistos,ParticlePerformanceDerivedHistos>(source)
{   }

PerformanceAnalyzer & PerformanceAnalyzer::operator=(const PerformanceAnalyzer & rhs)
{
  if (this!=&rhs)
    {
    EventProcessorSingle<ParticlePerformanceHistos,ParticlePerformanceDerivedHistos>::operator=(rhs);
    }
  return *this;
}

void PerformanceAnalyzer::setDefaultConfiguration()
{
  EventProcessorSingle<ParticlePerformanceHistos,ParticlePerformanceDerivedHistos>::setDefaultConfiguration();
  String taskName = name();
  String objectType = "HISTOGRAM";
  _configuration.addProperty(createKey(taskName,objectType,"pt_nbins"),100);
  _configuration.addProperty(createKey(taskName,objectType,"pt_min"), 0.0);
  _configuration.addProperty(createKey(taskName,objectType,"pt_max"), 5.0);
  _configuration.addProperty(createKey(taskName,objectType,"dpt_nbins"), 100);
  _configuration.addProperty(createKey(taskName,objectType,"dpt_min"),  -1.0);
  _configuration.addProperty(createKey(taskName,objectType,"dpt_max"),   1.0);
  _configuration.addProperty(createKey(taskName,objectType,"phi_nbins"), 100);
  _configuration.addProperty(createKey(taskName,objectType,"phi_min"),  -1.0);
  _configuration.addProperty(createKey(taskName,objectType,"phi_max"),   1.0);
  _configuration.addProperty(createKey(taskName,objectType,"Dphi_nbins"), 100);
  _configuration.addProperty(createKey(taskName,objectType,"Dphi_min"),  -1.0);
  _configuration.addProperty(createKey(taskName,objectType,"Dphi_max"),   1.0);
  _configuration.addProperty(createKey(taskName,objectType,"eta_nbins"), 100);
  _configuration.addProperty(createKey(taskName,objectType,"eta_min"),  -1.0);
  _configuration.addProperty(createKey(taskName,objectType,"eta_max"),   1.0);
  _configuration.addProperty(createKey(taskName,objectType,"Deta_nbins"), 100);
  _configuration.addProperty(createKey(taskName,objectType,"Deta_min"),  -1.0);
  _configuration.addProperty(createKey(taskName,objectType,"Deta_max"),   1.0);
  _configuration.addProperty(createKey(taskName,objectType,"rapidity_nbins"), 100);
  _configuration.addProperty(createKey(taskName,objectType,"rapidity_min"),  -1.0);
  _configuration.addProperty(createKey(taskName,objectType,"rapidity_max"),  -1.0);
  _configuration.addProperty(createKey(taskName,objectType,"Drapidity_nbins"), 100);
  _configuration.addProperty(createKey(taskName,objectType,"Drapidity_min"),  -1.0);
  _configuration.addProperty(createKey(taskName,objectType,"Drapidity_max"),   1.0);
}

void PerformanceAnalyzer::execute()
{
  _taskExecuted.increment();
  Event & theEvent = event();
  if (!filterEvent(theEvent)) return;
  if (!filterParticles(theEvent.particles()))  return;
  unsigned int npf = nParticleFilters();
  std::vector<ParticlePerformanceHistos*> & histograms = histograms_1();
  if (histograms.size()<_acceptedEventFilters.size()*npf)
    throw Exception("histograms_1().size()<nEventFilters()*nParticleFilters()",__FUNCTION__);
  // loop over the event filters (was over the particle filter flags), groups at e*nPF + p
  for (unsigned int iEventFilter=0; iEventFilter<_acceptedEventFilters.size(); iEventFilter++)
    {
    if (!_acceptedEventFilters[iEventFilter]) continue;
    unsigned int  baseSingle   = iEventFilter*npf;
    for (unsigned int iParticleFilter=0; iParticleFilter<npf;iParticleFilter++)
      {
      if (!_acceptedParticleFilters[iParticleFilter]) continue;
      ParticlePerformanceHistos * histos = histograms[baseSingle+iParticleFilter];
      std::vector<Particle*> & particles = acceptedParticles(iParticleFilter);
      for (auto & particle : particles) histos->fill(*particle,1.0);
      }
    }
}

} // namespace CAP
