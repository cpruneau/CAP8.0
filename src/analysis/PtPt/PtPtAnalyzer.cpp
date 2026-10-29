/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "PtPtAnalyzer.hpp"
#include "PrintHelpers.hpp"
#include "NameHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"


namespace CAP
{

PtPtAnalyzer::PtPtAnalyzer()
:
EventProcessorHistos<PtPtHistos,1>()
{
  appendClassName("PtPtAnalyzer");
  setMinimumReportLevel(Object::kInfo);
  setName("PtPtAnalyzer");
  setTitle("PtPtAnalyzer");
}

PtPtAnalyzer::PtPtAnalyzer(const PtPtAnalyzer & source)
:
EventProcessorHistos<PtPtHistos,1>(source)
{   }


void PtPtAnalyzer::setDefaultConfiguration()
{
  EventProcessorHistos<PtPtHistos,1>::setDefaultConfiguration();
  PtPtHistos::addDefaultConfiguration(_configuration,name());
}

void PtPtAnalyzer::execute()
{
  _taskExecuted.increment();
  Event & theEvent = event();
  if (!filterEvent(theEvent)) return;
  if (!filterParticles(theEvent.particles()))  return;
//  double mult = getEvtClassValue();
//  for (unsigned int iEventFilter=0; iEventFilter<nEventFilters();iEventFilter++)
//    {
//    if (!_acceptedParticleFilters[iEventFilter]) continue;
//    PtPtHistos * histos = static_cast<PtPtHistos *>(listAt(iEventFilter));
//    std::vector<double> & moments = particleMoments[iEventFilter];
//    histos->fill(mult,moments);
//    }
}

} // namespace CAP

