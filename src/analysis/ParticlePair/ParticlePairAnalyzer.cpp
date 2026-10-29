/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "ParticlePairAnalyzer.hpp"
#include "PrintHelpers.hpp"
#include "NameHelpers.hpp"
#include "Exceptions.hpp"

namespace CAP
{

ParticlePairAnalyzer::ParticlePairAnalyzer()
:
EventProcessorHistos<ParticlePairHistos,2,ParticleSingleHistos>()
{
  appendClassName("ParticlePairAnalyzer");
  setMinimumReportLevel(Object::kInfo);
  setName("ParticlePairAnalyzer");
  setTitle("ParticlePairAnalyzer");
}

ParticlePairAnalyzer::ParticlePairAnalyzer(const ParticlePairAnalyzer & source)
:
EventProcessorHistos<ParticlePairHistos,2,ParticleSingleHistos>(source)
{   }

ParticlePairAnalyzer & ParticlePairAnalyzer::operator=(const ParticlePairAnalyzer & rhs)
{
  if (this!=&rhs)
    {
    EventProcessorHistos<ParticlePairHistos,2,ParticleSingleHistos>::operator=(rhs);
    }
  return *this;
}

void ParticlePairAnalyzer::setDefaultConfiguration()
{
  EventProcessorHistos<ParticlePairHistos,2,ParticleSingleHistos>::setDefaultConfiguration();
  ParticlePairHistos::addDefaultConfiguration(_configuration,name());
}

//!
//! For each accepted event filter e: the particles of each particle filter i (folder e*m + i): n1, energy, pT,
//! (eta or y, phi), ... (ParticleSingleHistos::fill()) and their number per event (if the filter accepted particles); the pairs of each ordered pair of
//! particle filters (i, j) (folder e*m*m + i*m + j): all pairs of a particle of i and a particle of j, distinct when
//! i = j, and their number per event.
//!
void ParticlePairAnalyzer::execute()
{
  _taskExecuted.increment();
  Event & theEvent = event();
  if (!filterEvent(theEvent)) return;
  if (!filterParticles(theEvent.particles()))  return;
  const unsigned int nPF   = nParticleFilters();
  const unsigned int nPFsq = nPF*nPF;
  std::vector<ParticlePairHistos*>   & pairFolders   = histograms();
  std::vector<ParticleSingleHistos*> & singleFolders = singleHistograms();
  const std::vector<bool> & acceptedEF = acceptedEventFilters();
  const std::vector<bool> & acceptedPF = acceptedParticleFilters();
  if (pairFolders.size()<acceptedEF.size()*nPFsq)
    throw Exception("Fewer pair folders than nEventFilters()*nParticleFilters()^2",__FUNCTION__);
  if (singleFolders.size()<acceptedEF.size()*nPF)
    throw Exception("Fewer single folders than nEventFilters()*nParticleFilters()",__FUNCTION__);
  const double weight = 1.0;
  for (unsigned int iEventFilter=0; iEventFilter<acceptedEF.size(); iEventFilter++)
    {
    if (!acceptedEF[iEventFilter]) continue;
    for (unsigned int iParticleFilter=0; iParticleFilter<nPF; iParticleFilter++)
      {
      if (!acceptedPF[iParticleFilter]) continue;   // as ParticleSingleAnalyzer
      std::vector<Particle*> & particles = acceptedParticles(iParticleFilter);
      ParticleSingleHistos & single = *singleFolders[iEventFilter*nPF+iParticleFilter];
      double energy = 0.0;
      for (Particle * particle : particles)
        {
        energy += particle->momentum().t();
        single.fill(*particle,weight);
        }
      single.fillMultiplicity(double(particles.size()),energy,1.0);
      }
    for (unsigned int iParticleFilter1=0; iParticleFilter1<nPF; iParticleFilter1++)
      {
      std::vector<Particle*> & list1 = acceptedParticles(iParticleFilter1);
      const unsigned int n1 = list1.size();
      for (unsigned int iParticleFilter2=0; iParticleFilter2<nPF; iParticleFilter2++)
        {
        std::vector<Particle*> & list2 = acceptedParticles(iParticleFilter2);
        const unsigned int n2 = list2.size();
        const bool same = iParticleFilter1==iParticleFilter2;
        ParticlePairHistos & pair = *pairFolders[iEventFilter*nPFsq + iParticleFilter1*nPF + iParticleFilter2];
        for (unsigned int i1=0; i1<n1; i1++)
          for (unsigned int i2=0; i2<n2; i2++)
            if (!same || i1!=i2) pair.fill(*list1[i1],*list2[i2],weight);
        pair.fillMultiplicity(same ? double(n1)*double(n1>0 ? n1-1 : 0) : double(n1)*double(n2));
        }
      }
    }
}

void ParticlePairAnalyzer::finalize()
{
  // before EventProcessor::finalize(), which clears the counts of accepted events
  const auto & folderList = _managedHistograms;
  if (folderList.create() && !folderList.imports()) EventProcessor::scaleHistograms();
  EventProcessorHistos<ParticlePairHistos,2,ParticleSingleHistos>::finalize();
}

} // namespace CAP
