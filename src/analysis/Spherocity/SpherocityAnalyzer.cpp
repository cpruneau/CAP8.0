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
nSteps(360),
stepSize(CAP::MATH::twoPi()/360.0)
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
nSteps(source.nSteps),
stepSize(source.stepSize)
{    }

SpherocityAnalyzer & SpherocityAnalyzer::operator=(const SpherocityAnalyzer & rhs)
{
  if (this!=&rhs)
    {
    EventProcessorHistos<SpherocityHistos,1>::operator=(rhs);
    setEvent   = rhs.setEvent;
    fillS0     = rhs.fillS0;
    fillS1     = rhs.fillS1;
    nSteps     = rhs.nSteps;
    stepSize   = rhs.stepSize;
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
  if (!filterParticles(theEvent.particles()))  return;

  double factor = 1.0;
  unsigned int nef = nEventFilters();
  unsigned int npf = nParticleFilters();
  std::vector<SpherocityHistos*> & folders = histograms();
  if (folders.size()<nCombinations()) throw Exception("Fewer histogram folders than combinations of filters",__FUNCTION__);
  for (unsigned int iEventFilter = 0; iEventFilter < nef; iEventFilter++)
    {
    if (!_acceptedEventFilters[iEventFilter]) continue;
    //String evfName = eventFilters[iEventFilter]->name();
    unsigned int  baseSingle = iEventFilter*npf;

    for (unsigned int iParticleFilter=0; iParticleFilter < npf; iParticleFilter++ )
      {
      std::vector<Particle*> & particlesSelected = acceptedParticles(iParticleFilter);
      double  s0 = 1.0E10;
      double  s1 = 1.0E10;
      double  num0, num1, nx, ny, px, py, pt;
      double  denom0 = 0;
      double  denom1 = 0;
      double  refPhi  = 0.0;
      for(int k = 0; k < nSteps; k++)
        {
        nx = std::cos(refPhi); // x component of a unitary vector n
        ny = std::sin(refPhi); // y component of a unitary vector n
        num0 = 0;
        num1 = 0;
        for (auto particleSelected : particlesSelected)
          {
          if (!particleSelected) continue;
          VectorLorentz & momentum = particleSelected->momentum();
          pt = momentum.perp();
          px = momentum.x();
          py = momentum.y();
          if (fillS0)
            {
            num0 += absolute(ny*px - nx*py);
            if(k==0) denom0 += pt;
            }
          if (fillS1)
            {
            double  ax = px/pt;
            double  ay = py/pt;
            num1 += absolute(ny*ax - nx*ay);
            if(k==0) denom1 += 1;
            }
          }
        if (fillS0)
          {
          double ratio = num0/denom0;
          double r2 = ratio*ratio;
          if (r2 < s0) s0 = r2;
          }
        if (fillS1)
          {
          double ratio = num1/denom1;
          double r2 = ratio*ratio;
          if (r2 < s1) s1 = r2;
          }
        refPhi += stepSize;
        }
      unsigned int index = baseSingle+iParticleFilter;
      SpherocityHistos * histos = folders[index];
      histos->fill(s0*factor,s1*factor,1.0);
      }
    }
}

}  //namespace CAP
