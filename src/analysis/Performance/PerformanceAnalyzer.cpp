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


namespace CAP
{

PerformanceAnalyzer::PerformanceAnalyzer()
:
EventProcessorHistos<ParticlePerformanceHistos,1>()
{
  appendClassName("PerformanceAnalyzer");
  setMinimumReportLevel(Object::kInfo);
  setName("PerformanceAnalyzer");
  setTitle("PerformanceAnalyzer");
}

PerformanceAnalyzer::PerformanceAnalyzer(const PerformanceAnalyzer & source)
:
EventProcessorHistos<ParticlePerformanceHistos,1>(source)
{   }

PerformanceAnalyzer & PerformanceAnalyzer::operator=(const PerformanceAnalyzer & rhs)
{
  if (this!=&rhs)
    {
    EventProcessorHistos<ParticlePerformanceHistos,1>::operator=(rhs);
    }
  return *this;
}

void PerformanceAnalyzer::setDefaultConfiguration()
{
  EventProcessorHistos<ParticlePerformanceHistos,1>::setDefaultConfiguration();
  ParticlePerformanceHistos::addDefaultConfiguration(_configuration,name());
}

void PerformanceAnalyzer::execute()
{
  _taskExecuted.increment();
  Event & theEvent = event();                     // detector level
  if (!filterEvent(theEvent)) return;
  unsigned int npf = nParticleFilters();
  std::vector<ParticlePerformanceHistos*> & folders = histograms();
  if (folders.size()<_acceptedEventFilters.size()*npf)
    throw Exception("Fewer histogram folders than combinations of filters",__FUNCTION__);
  std::vector<Event*> & theEvents = events();
  Event * genEvent = theEvents.size()>1 ? theEvents[1] : nullptr;

  // generator level (efficiency denominator): before the detector level, whose selections replace these
  if (genEvent && filterParticles(*genEvent))
    for (unsigned int iEventFilter=0; iEventFilter<_acceptedEventFilters.size(); iEventFilter++)
      {
      if (!_acceptedEventFilters[iEventFilter]) continue;
      for (unsigned int iParticleFilter=0; iParticleFilter<npf;iParticleFilter++)
        {
        if (!_acceptedParticleFilters[iParticleFilter]) continue;
        ParticlePerformanceHistos * histos = folders[iEventFilter*npf+iParticleFilter];
        for (auto & particle : acceptedParticles(iParticleFilter)) histos->fillGenerated(*particle,1.0);
        }
      }

  // detector level: reco - gen, and the efficiency numerator
  if (!filterParticles(theEvent))  return;
  for (unsigned int iEventFilter=0; iEventFilter<_acceptedEventFilters.size(); iEventFilter++)
    {
    if (!_acceptedEventFilters[iEventFilter]) continue;
    unsigned int  baseSingle   = iEventFilter*npf;
    for (unsigned int iParticleFilter=0; iParticleFilter<npf;iParticleFilter++)
      {
      if (!_acceptedParticleFilters[iParticleFilter]) continue;
      ParticlePerformanceHistos * histos = folders[baseSingle+iParticleFilter];
      std::vector<Particle*> & particles = acceptedParticles(iParticleFilter);
      for (auto & particle : particles) histos->fill(*particle,1.0);
      }
    }
}
} // namespace CAP
