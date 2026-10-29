/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
#include "PtPtDerivedCalculator.hpp"
#include "PrintHelpers.hpp"
#include "NameHelpers.hpp"

namespace CAP
{

PtPtDerivedCalculator::PtPtDerivedCalculator()
:
EventProcessorHistos<PtPtHistos,1>(),
_sourceTask("PtPtAnalyzer")
{
  appendClassName("PtPtDerivedCalculator");
  setMinimumReportLevel(Object::kInfo);
  setName("PtPtDerivedCalculator");
  setTitle("PtPtDerivedCalculator");
}

PtPtDerivedCalculator::PtPtDerivedCalculator(const PtPtDerivedCalculator & source)
:
EventProcessorHistos<PtPtHistos,1>(source),
_sourceTask(source._sourceTask)
{   }

PtPtDerivedCalculator & PtPtDerivedCalculator::operator=(const PtPtDerivedCalculator & rhs)
{
  if (this!=&rhs)
    {
    EventProcessorHistos<PtPtHistos,1>::operator=(rhs);
    _sourceTask = rhs._sourceTask;
    }
  return *this;
}

void PtPtDerivedCalculator::setDefaultConfiguration()
{
  EventProcessorHistos<PtPtHistos,1>::setDefaultConfiguration();
  PtPtHistos::addDefaultConfiguration(_configuration,name());
  _configuration.addProperty(createKey(name(),"SOURCE_TASK"),String("PtPtAnalyzer"));
}

void PtPtDerivedCalculator::configure(const Configuration & configuration)
{
  EventProcessorHistos<PtPtHistos,1>::configure(configuration);
  _sourceTask = configuration.valueString(createKey(name(),"SOURCE_TASK"));
  if (reportInfo(__FUNCTION__)) printValue(createKey(name(),"SOURCE_TASK"),_sourceTask);
}

void PtPtDerivedCalculator::execute()
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  _taskExecuted.increment();
  std::vector<PtPtHistos*> & folders = histograms();
  if (folders.size()<nCombinations())
    throw Exception("Fewer histogram folders than combinations of filters",__FUNCTION__);
  for (PtPtHistos * folder : folders)
    {
    if (!folder->hasDerived()) folder->createDerived();   // (already there if the input file has them)
    folder->calculateDerived();
    }
}

} // namespace CAP
