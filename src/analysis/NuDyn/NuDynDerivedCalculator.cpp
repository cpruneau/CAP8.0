/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
#include "NuDynDerivedCalculator.hpp"
#include "PrintHelpers.hpp"
#include "NameHelpers.hpp"

namespace CAP
{

NuDynDerivedCalculator::NuDynDerivedCalculator()
:
EventProcessorHistos<NuDynHistos,2>(),
_sourceTask("NuDynAnalyzer")
{
  appendClassName("NuDynDerivedCalculator");
  setMinimumReportLevel(Object::kInfo);
  setName("NuDynDerivedCalculator");
  setTitle("NuDynDerivedCalculator");
}

NuDynDerivedCalculator::NuDynDerivedCalculator(const NuDynDerivedCalculator & source)
:
EventProcessorHistos<NuDynHistos,2>(source),
_sourceTask(source._sourceTask)
{   }

NuDynDerivedCalculator & NuDynDerivedCalculator::operator=(const NuDynDerivedCalculator & rhs)
{
  if (this!=&rhs)
    {
    EventProcessorHistos<NuDynHistos,2>::operator=(rhs);
    _sourceTask = rhs._sourceTask;
    }
  return *this;
}

void NuDynDerivedCalculator::setDefaultConfiguration()
{
  EventProcessorHistos<NuDynHistos,2>::setDefaultConfiguration();
  NuDynHistos::addDefaultConfiguration(_configuration,name());
  _configuration.addProperty(createKey(name(),"SOURCE_TASK"),String("NuDynAnalyzer"));
}

void NuDynDerivedCalculator::configure(const Configuration & configuration)
{
  EventProcessorHistos<NuDynHistos,2>::configure(configuration);
  _sourceTask = configuration.valueString(createKey(name(),"SOURCE_TASK"));
  if (reportInfo(__FUNCTION__)) printValue(createKey(name(),"SOURCE_TASK"),_sourceTask);
}

void NuDynDerivedCalculator::execute()
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  _taskExecuted.increment();
  std::vector<NuDynHistos*> & folders = histograms();
  if (folders.size()<nCombinations())
    throw Exception("Fewer histogram folders than combinations of filters",__FUNCTION__);
  for (NuDynHistos * folder : folders)
    {
    if (!folder->hasDerived()) folder->createDerived();   // (already there if the input file has them)
    folder->calculateDerived();
    }
}

} // namespace CAP
