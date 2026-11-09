/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "SpherocityAnalyzer.hpp"
#include "ParticleSelection.hpp"
#include "MathExceptions.hpp"
#include "MathConstants.hpp"
#include "MathBasicFunctions.hpp"
#include "PrintHelpers.hpp"
#include "NameHelpers.hpp"


using CAP::MATH::absolute;

namespace CAP
{
SpherocityAnalyzer::SpherocityAnalyzer()
:
EventProcessorHistos<SpherocityHistos,1>(),
setEvent(true),
fillS0(true),
fillS1(false),
fillS1VsS0(false),
nSteps(1000),
stepSize(CAP::MATH::twoPi()/1000.0),
minMultiplicity(3)
{
  appendClassName("SpherocityAnalyzer");
  setMinimumReportLevel(Object::kInfo);
  setName("Spherocity");
  setTitle("Spherocity");
}

SpherocityAnalyzer::SpherocityAnalyzer(const SpherocityAnalyzer & source)
:
EventProcessorHistos<SpherocityHistos,1>(source),
setEvent(source.setEvent),
fillS0(source.fillS0),
fillS1(source.fillS1),
fillS1VsS0(source.fillS1VsS0),
nSteps(source.nSteps),
stepSize(source.stepSize),
minMultiplicity(source.minMultiplicity)
{    }

SpherocityAnalyzer & SpherocityAnalyzer::operator=(const SpherocityAnalyzer & rhs)
{
  if (this!=&rhs)
    {
    EventProcessorHistos<SpherocityHistos,1>::operator=(rhs);
    setEvent   = rhs.setEvent;
    fillS0     = rhs.fillS0;
    fillS1     = rhs.fillS1;
    fillS1VsS0 = rhs.fillS1VsS0;
    nSteps     = rhs.nSteps;
    stepSize   = rhs.stepSize;
    minMultiplicity = rhs.minMultiplicity;
    }
  return *this;
}

void SpherocityAnalyzer::setDefaultConfiguration()
{
  EventProcessorHistos<SpherocityHistos,1>::setDefaultConfiguration();
  SpherocityHistos::addDefaultConfiguration(_configuration,name());
}

void SpherocityAnalyzer::configure(const Configuration & configuration)
{
  EventProcessorHistos<SpherocityHistos,1>::configure(configuration);
  const String & taskName = name();
  String objectType = "HISTOGRAM";
  setEvent   = _configuration.valueBool(createKey(taskName,objectType,"SetEvent"));
  fillS0     = _configuration.valueBool(createKey(taskName,objectType,"FillS0"));
  fillS1     = _configuration.valueBool(createKey(taskName,objectType,"FillS1"));
  fillS1VsS0 = _configuration.valueBool(createKey(taskName,objectType,"FillS1VsS0"));
  nSteps     = _configuration.valueInt( createKey(taskName,objectType,"nSteps"));
  if (nSteps<1) throw MathException("DivisionByZero",__FUNCTION__);
  stepSize   = MATH::twoPi()/nSteps;
  int minimum = _configuration.valueInt(createKey(taskName,objectType,"MinMultiplicity"));
  minMultiplicity = minimum < 1 ? 1u : (unsigned int) minimum;
  if (reportDebug(__FUNCTION__))
    {
    printCR();
    printValue(createKey(taskName,objectType,"SetEvent"),setEvent);
    printValue(createKey(taskName,objectType,"FillS0"),fillS0);
    printValue(createKey(taskName,objectType,"FillS1"),fillS1);
    printValue(createKey(taskName,objectType,"FillS1VsS0"),fillS1VsS0);
    printValue(createKey(taskName,objectType,"nSteps"),nSteps);
    printCR();
    }
}

void SpherocityAnalyzer::execute()
{
  _taskExecuted.increment();
  Event & theEvent = event();
  if (!filterEvent(theEvent)) return;
  if (!filterParticles(theEvent))  return;

  unsigned int nef = nEventFilters();
  unsigned int npf = nParticleFilters();
  std::vector<SpherocityHistos*> & folders = histograms();
  if (folders.size()<nCombinations()) throw Exception("Fewer histogram folders than combinations of filters",__FUNCTION__);
  for (unsigned int iParticleFilter=0; iParticleFilter < npf; iParticleFilter++ )
    {
    // computed once per selection, for all the event filters
    const std::vector<Particle*> & particles = acceptedParticles(iParticleFilter);
    if (particles.size() < minMultiplicity) continue;     // too few particles: no spherocity
    double s0 = (fillS0 || fillS1VsS0) ? ParticleSelection::spherocity(particles, true,  (unsigned int) nSteps, (unsigned int) nSteps) : 0.0;
    double s1 = (fillS1 || fillS1VsS0) ? ParticleSelection::spherocity(particles, false, (unsigned int) nSteps, (unsigned int) nSteps) : 0.0;
    for (unsigned int iEventFilter = 0; iEventFilter < nef; iEventFilter++)
      {
      if (!_acceptedEventFilters[iEventFilter]) continue;
      folders[iEventFilter*npf+iParticleFilter]->fill(s0,s1,1.0);
      }
    }
}

}  //namespace CAP
