/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
#include "GlobalDerivedCalculator.hpp"
#include "PrintHelpers.hpp"
#include "NameHelpers.hpp"

namespace CAP
{

GlobalDerivedCalculator::GlobalDerivedCalculator()
:
EventProcessorHistos<GlobalHistos,1>(),
_sourceTask("GlobalAnalyzer")
{
  appendClassName("GlobalDerivedCalculator");
  setMinimumReportLevel(Object::kInfo);
  setName("GlobalDerivedCalculator");
  setTitle("GlobalDerivedCalculator");
}

GlobalDerivedCalculator::GlobalDerivedCalculator(const GlobalDerivedCalculator & source)
:
EventProcessorHistos<GlobalHistos,1>(source),
_sourceTask(source._sourceTask)
{   }

GlobalDerivedCalculator & GlobalDerivedCalculator::operator=(const GlobalDerivedCalculator & rhs)
{
  if (this!=&rhs)
    {
    EventProcessorHistos<GlobalHistos,1>::operator=(rhs);
    _sourceTask = rhs._sourceTask;
    }
  return *this;
}

void GlobalDerivedCalculator::setDefaultConfiguration()
{
  EventProcessorHistos<GlobalHistos,1>::setDefaultConfiguration();
  GlobalHistos::addDefaultConfiguration(_configuration,name());
  _configuration.addProperty(createKey(name(),"SOURCE_TASK"),String("GlobalAnalyzer"));
}

void GlobalDerivedCalculator::configure(const Configuration & configuration)
{
  EventProcessorHistos<GlobalHistos,1>::configure(configuration);
  _sourceTask = configuration.valueString(createKey(name(),"SOURCE_TASK"));
  if (reportInfo(__FUNCTION__)) printValue(createKey(name(),"SOURCE_TASK"),_sourceTask);
}

void GlobalDerivedCalculator::execute()
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  _taskExecuted.increment();
  std::vector<GlobalHistos*> & folders = histograms();
  if (folders.size()<nCombinations())
    throw Exception("Fewer histogram folders than combinations of filters",__FUNCTION__);
  for (GlobalHistos * folder : folders)
    {
    if (!folder->hasDerived()) folder->createDerived();   // (already there if the input file has them)
    folder->calculateDerived();
    }
}

} // namespace CAP
