/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "GlobalAnalyzer.hpp"
#include "PrintHelpers.hpp"
#include "NameHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"


namespace CAP
{

GlobalAnalyzer::GlobalAnalyzer()
:
  EventProcessorHistos<GlobalHistos,1>()
{
  appendClassName("GlobalAnalyzer");
  setMinimumReportLevel(Object::kInfo);
  setName("GlobalAnalyzer");
  setTitle("GlobalAnalyzer");
}

GlobalAnalyzer::GlobalAnalyzer(const GlobalAnalyzer & source)
:
  EventProcessorHistos<GlobalHistos,1>(source)
{   }

GlobalAnalyzer & GlobalAnalyzer::operator=(const GlobalAnalyzer & rhs)
{
  if (this!=&rhs)
    {
    EventProcessorHistos<GlobalHistos,1>::operator=(rhs);
    }
  return *this;
}

void GlobalAnalyzer::setDefaultConfiguration()
{
  EventProcessorHistos<GlobalHistos,1>::setDefaultConfiguration();
  GlobalHistos::addDefaultConfiguration(_configuration,name());
}

void GlobalAnalyzer::execute()
{
  Event & theEvent = event();
  if (!filterEvent(theEvent)) return;
  if (!filterParticles(theEvent))  return;
  theEvent.resetMult();
  theEvent.resetEnergy();
  theEvent.resetCharge();
  theEvent.resetStrangeness();
  theEvent.resetBaryon();
  theEvent.resetPtSum();
  std::vector<GlobalHistos*> & folders = histograms();
  const bool fillHistograms = !folders.empty();
  const unsigned int nef = nEventFilters();
  const unsigned int npf = nParticleFilters();
  if (fillHistograms && folders.size()<nef*npf)
    throw Exception("Fewer histogram folders than combinations of filters",__FUNCTION__);
  for (unsigned int iEventFilter = 0;iEventFilter<nef;iEventFilter++)
    {
    if (!_acceptedEventFilters[iEventFilter]) continue;
    unsigned int baseIndex = iEventFilter*npf;
    for (unsigned int iParticleFilter = 0; iParticleFilter<npf; iParticleFilter++)
      {
      if (!_acceptedParticleFilters[iParticleFilter]) continue;
      // the global quantities of the selection, computed once per event with the selection itself (shared)
      const ParticleSelection & s = selection(iParticleFilter);
      double n = s.multiplicity(), e = s.energy(), q = s.charge(), st = s.strangeness(), b = s.baryon();
      double ptSum = s.ptSum();
      // quantities indexed by number, for the event filters of earlier versions (MULT_k: particle filter k of this
      // task; they were stored by the index of the event filter, each particle filter overwriting the previous one)
      if (iParticleFilter < 6)
        {
        theEvent.setMult(iParticleFilter,n);
        theEvent.setEnergy(iParticleFilter,e);
        theEvent.setCharge(iParticleFilter,q);
        theEvent.setStrangeness(iParticleFilter,st);
        theEvent.setBaryon(iParticleFilter,b);
        theEvent.setPtSum(iParticleFilter,ptSum);
        }
      if (fillHistograms) folders[baseIndex+iParticleFilter]->fill(n,ptSum,e,q,st,b,1.0);
      }
    }
  _taskExecuted.increment();
}


} // namespace CAP
