/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "GlobalAnalyzer.hpp"
#include "PrintHelpers.hpp"
#include "NameHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"


namespace CAP
{

GlobalAnalyzer::GlobalAnalyzer()
:
  EventProcessorHistos<GlobalHistos,1>()
{
  appendClassName("GlobalAnalyzer");
  setMinimumReportLevel(Object::kInfo);
  setName("GlobalAnalyzer");
  setTitle("GlobalAnalyzer");
}

GlobalAnalyzer::GlobalAnalyzer(const GlobalAnalyzer & source)
:
  EventProcessorHistos<GlobalHistos,1>(source)
{   }

GlobalAnalyzer & GlobalAnalyzer::operator=(const GlobalAnalyzer & rhs)
{
  if (this!=&rhs)
    {
    EventProcessorHistos<GlobalHistos,1>::operator=(rhs);
    }
  return *this;
}

void GlobalAnalyzer::setDefaultConfiguration()
{
  EventProcessorHistos<GlobalHistos,1>::setDefaultConfiguration();
  GlobalHistos::addDefaultConfiguration(_configuration,name());
}

void GlobalAnalyzer::execute()
{
  Event & theEvent = event();
  if (!filterEvent(theEvent)) return;
  if (!filterParticles(theEvent.particles()))  return;

  double n ,e, q, s, b, ptSum;
  unsigned int baseIndex = 0;
  unsigned int index = 0;
  theEvent.resetMult();
  theEvent.resetEnergy();
  theEvent.resetCharge();
  theEvent.resetStrangeness();
  theEvent.resetBaryon();
  theEvent.resetPtSum();
  std::vector<GlobalHistos*> & folders = histograms();
  const bool fillHistograms = !folders.empty();
  if (fillHistograms && folders.size()<nEventFilters()*nParticleFilters())
    throw Exception("Fewer histogram folders than combinations of filters",__FUNCTION__);
  //printValue("GlobalAnalyzer::execute()",4);
  for (unsigned int iEventFilter = 0;iEventFilter<nEventFilters();iEventFilter++)
    {
    //printValue("GlobalAnalyzer::execute() iEventFilter",iEventFilter);
    baseIndex = iEventFilter*nParticleFilters();
    if (_acceptedEventFilters[iEventFilter])
      {
      for (unsigned int iParticleFilter = 0; iParticleFilter<nParticleFilters(); iParticleFilter++)
        {
        //printValue("GlobalAnalyzer::execute() iParticleFilter",iParticleFilter);
        if (_acceptedParticleFilters[iParticleFilter])
          {
          index = baseIndex+iParticleFilter;
         // std::vector<Particle*> & particlesSelected = acceptedParticles(iParticleFilter);
          std::vector<Particle*> & particlesSelected = acceptedParticles(iParticleFilter);
          n = 0;
          e = 0;
          q = 0;
          s = 0;
          b = 0;
          ptSum = 0;
          for (auto & particle : particlesSelected)
            {
            ParticleType & type = particle->type();
            VectorLorentz & momentum = particle->momentum();
            n++;
            e += momentum.t();
            q += type.charge();
            s += type.strangeness();
            b += type.baryonNumber();
            ptSum += momentum.perp();
            }
          theEvent.setMult(iEventFilter,n);
          theEvent.setEnergy(iEventFilter,e);
          theEvent.setCharge(iEventFilter,q);
          theEvent.setStrangeness(iEventFilter,s);
          theEvent.setBaryon(iEventFilter,b);
          theEvent.setPtSum(iEventFilter,ptSum);
          if (fillHistograms) folders[index]->fill(n,ptSum,e,q,s,b,1.0);
          }
        }
      }
    }
  _taskExecuted.increment();;
}


} // namespace CAP
