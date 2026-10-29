/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
#include "ParticlePair3DDerivedCalculator.hpp"
#include "PrintHelpers.hpp"
#include "NameHelpers.hpp"

namespace CAP
{

ParticlePair3DDerivedCalculator::ParticlePair3DDerivedCalculator()
:
EventProcessorHistos<ParticlePair3DHistos,2,ParticleSingleHistos>(),
_sourceTask("ParticlePair3DAnalyzer")
{
  appendClassName("ParticlePair3DDerivedCalculator");
  setMinimumReportLevel(Object::kInfo);
  setName("ParticlePair3DDerivedCalculator");
  setTitle("ParticlePair3DDerivedCalculator");
}

ParticlePair3DDerivedCalculator::ParticlePair3DDerivedCalculator(const ParticlePair3DDerivedCalculator & source)
:
EventProcessorHistos<ParticlePair3DHistos,2,ParticleSingleHistos>(source),
_sourceTask(source._sourceTask)
{   }

ParticlePair3DDerivedCalculator & ParticlePair3DDerivedCalculator::operator=(const ParticlePair3DDerivedCalculator & rhs)
{
  if (this!=&rhs)
    {
    EventProcessorHistos<ParticlePair3DHistos,2,ParticleSingleHistos>::operator=(rhs);
    _sourceTask = rhs._sourceTask;
    }
  return *this;
}

void ParticlePair3DDerivedCalculator::setDefaultConfiguration()
{
  EventProcessorHistos<ParticlePair3DHistos,2,ParticleSingleHistos>::setDefaultConfiguration();
  ParticlePair3DHistos::addDefaultConfiguration(_configuration,name());
  _configuration.addProperty(createKey(name(),"SOURCE_TASK"),String("ParticlePair3DAnalyzer"));
}

void ParticlePair3DDerivedCalculator::configure(const Configuration & configuration)
{
  EventProcessorHistos<ParticlePair3DHistos,2,ParticleSingleHistos>::configure(configuration);
  _sourceTask = configuration.valueString(createKey(name(),"SOURCE_TASK"));
  if (reportInfo(__FUNCTION__)) printValue(createKey(name(),"SOURCE_TASK"),_sourceTask);
}

void ParticlePair3DDerivedCalculator::execute()
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  _taskExecuted.increment();
  const unsigned int nPF   = nParticleFilters();
  const unsigned int nPFsq = nPF*nPF;
  std::vector<ParticlePair3DHistos*>   & pairFolders   = histograms();
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
    ParticlePair3DHistos & pair = *pairFolders[k];
    if (!pair.hasDerived()) pair.createDerived();
    pair.calculateDerived(*singleFolders[iEventFilter*nPF+iParticleFilter1],
                          *singleFolders[iEventFilter*nPF+iParticleFilter2],
                          iParticleFilter1==iParticleFilter2);
    }
}

} // namespace CAP
