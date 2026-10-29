/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
#include "ParticleSingleDerivedCalculator.hpp"
#include "PrintHelpers.hpp"
#include "NameHelpers.hpp"

namespace CAP
{

ParticleSingleDerivedCalculator::ParticleSingleDerivedCalculator()
:
EventProcessorHistos<ParticleSingleHistos,1>(),
_sourceTask("ParticleSingleAnalyzer")
{
  appendClassName("ParticleSingleDerivedCalculator");
  setMinimumReportLevel(Object::kInfo);
  setName("ParticleSingleDerivedCalculator");
  setTitle("ParticleSingleDerivedCalculator");
}

ParticleSingleDerivedCalculator::ParticleSingleDerivedCalculator(const ParticleSingleDerivedCalculator & source)
:
EventProcessorHistos<ParticleSingleHistos,1>(source),
_sourceTask(source._sourceTask)
{   }

ParticleSingleDerivedCalculator & ParticleSingleDerivedCalculator::operator=(const ParticleSingleDerivedCalculator & rhs)
{
  if (this!=&rhs)
    {
    EventProcessorHistos<ParticleSingleHistos,1>::operator=(rhs);
    _sourceTask = rhs._sourceTask;
    }
  return *this;
}

void ParticleSingleDerivedCalculator::setDefaultConfiguration()
{
  EventProcessorHistos<ParticleSingleHistos,1>::setDefaultConfiguration();
  ParticleSingleHistos::addDefaultConfiguration(_configuration,name());
  _configuration.addProperty(createKey(name(),"SOURCE_TASK"),String("ParticleSingleAnalyzer"));
}

void ParticleSingleDerivedCalculator::configure(const Configuration & configuration)
{
  EventProcessorHistos<ParticleSingleHistos,1>::configure(configuration);
  _sourceTask = configuration.valueString(createKey(name(),"SOURCE_TASK"));
  if (reportInfo(__FUNCTION__)) printValue(createKey(name(),"SOURCE_TASK"),_sourceTask);
}

void ParticleSingleDerivedCalculator::execute()
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  _taskExecuted.increment();
  std::vector<ParticleSingleHistos*> & folders = histograms();
  if (folders.size()<nCombinations())
    throw Exception("Fewer histogram folders than combinations of filters",__FUNCTION__);
  for (ParticleSingleHistos * folder : folders)
    {
    if (!folder->h_n1_phi) folder->createDerived();   // (already there if the input file has them)
    folder->calculateDerived();
    }
}

} // namespace CAP
