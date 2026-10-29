/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
#include "ParticlePairDerivedCalculator.hpp"
#include "PrintHelpers.hpp"
#include "NameHelpers.hpp"

namespace CAP
{

ParticlePairDerivedCalculator::ParticlePairDerivedCalculator()
:
EventProcessorHistos<ParticlePairHistos,2,ParticleSingleHistos>(),
_sourceTask("ParticlePairAnalyzer")
{
  appendClassName("ParticlePairDerivedCalculator");
  setMinimumReportLevel(Object::kInfo);
  setName("ParticlePairDerivedCalculator");
  setTitle("ParticlePairDerivedCalculator");
}

ParticlePairDerivedCalculator::ParticlePairDerivedCalculator(const ParticlePairDerivedCalculator & source)
:
EventProcessorHistos<ParticlePairHistos,2,ParticleSingleHistos>(source),
_sourceTask(source._sourceTask)
{   }

ParticlePairDerivedCalculator & ParticlePairDerivedCalculator::operator=(const ParticlePairDerivedCalculator & rhs)
{
  if (this!=&rhs)
    {
    EventProcessorHistos<ParticlePairHistos,2,ParticleSingleHistos>::operator=(rhs);
    _sourceTask = rhs._sourceTask;
    }
  return *this;
}

void ParticlePairDerivedCalculator::setDefaultConfiguration()
{
  EventProcessorHistos<ParticlePairHistos,2,ParticleSingleHistos>::setDefaultConfiguration();
  ParticlePairHistos::addDefaultConfiguration(_configuration,name());
  _configuration.addProperty(createKey(name(),"SOURCE_TASK"),String("ParticlePairAnalyzer"));
}

void ParticlePairDerivedCalculator::configure(const Configuration & configuration)
{
  EventProcessorHistos<ParticlePairHistos,2,ParticleSingleHistos>::configure(configuration);
  _sourceTask = configuration.valueString(createKey(name(),"SOURCE_TASK"));
  if (reportInfo(__FUNCTION__)) printValue(createKey(name(),"SOURCE_TASK"),_sourceTask);
}

void ParticlePairDerivedCalculator::execute()
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  _taskExecuted.increment();
  const unsigned int nPF   = nParticleFilters();
  const unsigned int nPFsq = nPF*nPF;
  std::vector<ParticlePairHistos*>   & pairFolders   = histograms();
  std::vector<ParticleSingleHistos*> & singleFolders = singleHistograms();
  if (pairFolders.size()<nCombinations())
    throw Exception("Fewer pair folders than combinations of filters",__FUNCTION__);
  if (singleFolders.size()<nEventFilters()*nPF)
    throw Exception("Fewer single folders than nEventFilters()*nParticleFilters()",__FUNCTION__);
  for (ParticleSingleHistos * single : singleFolders)
    {
    if (!single->h_n1_phi) single->createDerived();   // (already there if the input file has them)
    single->calculateDerived();
    }
  for (unsigned int k=0; k<pairFolders.size(); k++)
    {
    unsigned int iEventFilter     = k/nPFsq;
    unsigned int iParticleFilter1 = (k/nPF)%nPF;
    unsigned int iParticleFilter2 = k%nPF;
    ParticlePairHistos & pair = *pairFolders[k];
    if (!pair.h_n1n1_phiphi) pair.createDerived();
    pair.calculateDerived(*singleFolders[iEventFilter*nPF+iParticleFilter1],
                          *singleFolders[iEventFilter*nPF+iParticleFilter2]);
    }
}

} // namespace CAP
