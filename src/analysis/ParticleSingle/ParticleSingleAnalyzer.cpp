/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "ParticleSingleAnalyzer.hpp"
#include "PrintHelpers.hpp"
#include "NameHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"


namespace CAP
{

ParticleSingleAnalyzer::ParticleSingleAnalyzer()
:
EventProcessorHistos<ParticleSingleHistos,1>()
{
  appendClassName("ParticleSingleAnalyzer");
  setMinimumReportLevel(Object::kInfo);
  setName("ParticleSingleAnalyzer");
  setTitle("ParticleSingleAnalyzer");
}

ParticleSingleAnalyzer::ParticleSingleAnalyzer(const ParticleSingleAnalyzer & source)
:
EventProcessorHistos<ParticleSingleHistos,1>(source)
{   }

ParticleSingleAnalyzer & ParticleSingleAnalyzer::operator=(const ParticleSingleAnalyzer & rhs)
{
  if (this!=&rhs)
    {
    EventProcessorHistos<ParticleSingleHistos,1>::operator=(rhs);
    }
  return *this;
}

void ParticleSingleAnalyzer::setDefaultConfiguration()
{
  if (reportDebug(__FUNCTION__)) { printCR();}
  EventProcessorHistos<ParticleSingleHistos,1>::setDefaultConfiguration();
  ParticleSingleHistos::addDefaultConfiguration(_configuration,name());
}

void ParticleSingleAnalyzer::execute()
{
  _taskExecuted.increment();
  Event & theEvent = event();
  if (!filterEvent(theEvent)) return;
  if (!filterParticles(theEvent))  return;
  unsigned int baseSingle;
  unsigned int index;
  double weight;
  bool usePartEff = particleEfficiencies().size()>0;
  const std::vector<bool> & acceptedEF = acceptedEventFilters();
  const std::vector<bool> & acceptedPF = acceptedParticleFilters();
  std::vector<ParticleSingleHistos*> & folders = histograms();
  if (folders.size() < 1)
    throw Exception("no histograms",__FUNCTION__);
  std::vector<ParticleEfficiency*> & particleEff = particleEfficiencies();
  unsigned int nef = acceptedEF.size();
  unsigned int npf = acceptedPF.size();
  // Particle efficiencies: one for all, one per particle filter, or one per event filter and particle filter
  for (unsigned int iEventFilter = 0;iEventFilter<nef;iEventFilter++)
    {
    if (!acceptedEF[iEventFilter]) continue;
    baseSingle = iEventFilter*nParticleFilters();
    for (unsigned int iParticleFilter=0; iParticleFilter<npf;iParticleFilter++)
      {
      if (!acceptedPF[iParticleFilter]) continue;
      index = baseSingle + iParticleFilter;
      ParticleSingleHistos * histos = folders[index];
      std::vector<Particle*> & accParticles = acceptedParticles(iParticleFilter);
      ParticleEfficiency * efficiency = usePartEff ? objectFor(particleEff,iEventFilter,nef,iParticleFilter,npf,
                                                               "particle efficiencies") : nullptr;
      double multiplicityAccepted = 0.0;
      double energyAccepted = 0.0;
      for (unsigned int k=0; k<accParticles.size(); k++)
        {
        Particle * particle = accParticles[k];
        multiplicityAccepted++;
        VectorLorentz mm = particle->momentum();
        energyAccepted += mm.t();
        weight = efficiency ? efficiency->weight(*particle) : 1.0;
        histos->fill(*particle,weight);
        }
      histos->fillMultiplicity(multiplicityAccepted,energyAccepted,1.0);
      }
    }
}

void ParticleSingleAnalyzer::finalize()
{
  // before EventProcessor::finalize(), which clears the counts of accepted events
  const auto & folderList = _managedHistograms;
  if (folderList.create() && !folderList.imports()) EventProcessor::scaleHistograms();
  EventProcessorHistos<ParticleSingleHistos,1>::finalize();
}

} // namespace CAP
