/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "MeasurementSimulator.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "TRandom.h"
#include "Exceptions.hpp"
#include "EventEfficiency.hpp"
#include "EventSmearer.hpp"
#include "ParticleEfficiency.hpp"
#include "ParticleSmear.hpp"
#include <unordered_set>


namespace CAP
{

MeasurementSimulator::MeasurementSimulator()
:
EventProcessor()
{
  appendClassName("MeasurementSimulator");
  setName("MeasurementSimulator");
  setTitle("MeasurementSimulator");
}

MeasurementSimulator::MeasurementSimulator(const MeasurementSimulator & source)
:
EventProcessor(source),
_random(source._random)
{  }

MeasurementSimulator & MeasurementSimulator::operator=(const MeasurementSimulator & rhs)
{
  if (this!=&rhs)
    {
    EventProcessor::operator=(rhs);
    _random = rhs._random;
    }
  return *this;
}

void MeasurementSimulator::setDefaultConfiguration()
{
  EventProcessor::setDefaultConfiguration();
  // events: the generator-level event, read, and the detector-level event, written (FastSimEvent); both are objects of
  // the store (made by FilterCreator in RunCAP workflows), reset by the event iterator before each event
  _configuration.addProperty(createKey(name(),"EVENT:USE"),true);
  _configuration.addProperty(createKey(name(),"EVENT:N"),2);
  _configuration.addProperty(createKey(name(),"EVENT_0:NAME"),"Event");
  _configuration.addProperty(createKey(name(),"EVENT_0:OWNER"),false);
  _configuration.addProperty(createKey(name(),"EVENT_1:NAME"),"FastSimEvent");
  _configuration.addProperty(createKey(name(),"EVENT_1:OWNER"),false);
  _configuration.addProperty(createKey(name(),"Random:SetSeed"),true);
  _configuration.addProperty(createKey(name(),"Random:Seed"),long(271828));
}

void MeasurementSimulator::initialize()
{
  EventProcessor::initialize();
  bool setSeed = _configuration.valueBool(createKey(name(),"Random:SetSeed"),true,true);
  long seed    = _configuration.valueLong(createKey(name(),"Random:Seed"),true,271828);
  _random.setSeed(setSeed ? std::uint64_t(seed) : 0);
}

void MeasurementSimulator::execute()
{
  _taskExecuted.increment();
  std::vector<Event*> & theEvents = events();
  if (theEvents.size()!=2 || !theEvents[0] || !theEvents[1] || theEvents[0]==theEvents[1])
    throw ConfigurationException(createKey(name(),"EVENT"),String("MeasurementSimulator needs two different events: "
                                 "EVENT_0, the generator-level event it reads, and EVENT_1, the detector-level event it "
                                 "writes (by default FastSimEvent); it has ")+String(std::to_string(theEvents.size()).c_str()),__FUNCTION__);
  Event & genEvent  = *(theEvents[0]);
  Event & recoEvent = *(theEvents[1]);
  recoEvent.reset();
  recoEvent.setEventIndex(genEvent.eventIndex());
  recoEvent.setEventNumber(genEvent.eventNumber());
  recoEvent.projectileA() = genEvent.projectileA();
  recoEvent.projectileB() = genEvent.projectileB();
  if (!filterEvent(genEvent)) return;
  std::vector<EventEfficiency*> & eventEff = eventEfficiencies();
  if (!eventEff.empty() && !eventEff[0]->accept(genEvent)) return;
  if (!filterParticles(genEvent)) return;

  unsigned int npf = nParticleFilters();
  // the event class: the first event filter accepting the event (for efficiencies and smearers per event filter)
  const std::vector<bool> & acceptedEF = acceptedEventFilters();
  unsigned int nef = (unsigned int) acceptedEF.size(), iEventFilter = 0;
  while (iEventFilter<nef && !acceptedEF[iEventFilter]) iEventFilter++;
  if (iEventFilter==nef) iEventFilter = 0;
  std::vector<ParticleEfficiency*> & particleEff = particleEfficiencies();
  std::vector<ParticleSmear*>    & smearers    = particleSmearers();
  std::unordered_set<const Particle*> measured;     // each generator-level particle is measured once
  for (unsigned int iParticleFilter=0; iParticleFilter<npf; iParticleFilter++)
    {
    if (!_acceptedParticleFilters[iParticleFilter]) continue;
    ParticleEfficiency * efficiency = objectFor(particleEff,iEventFilter,nef,iParticleFilter,npf,"particle efficiencies");
    ParticleSmear    * smearer    = objectFor(smearers,   iEventFilter,nef,iParticleFilter,npf,"particle smearers");
    for (Particle * genParticle : acceptedParticles(iParticleFilter))
      {
      if (!measured.insert(genParticle).second) continue;          // already measured (or lost) for another filter
      if (efficiency && !efficiency->accept(*genParticle,_random)) continue; // lost: not detected
      Particle & recoParticle = Particle::factory().nextObject();
      if (smearer)
        smearer->smear(*genParticle,recoParticle,_random);
      else
        {
        recoParticle = *genParticle;
        recoParticle.setParent(nullptr);
        recoParticle.clearChildren();
        recoParticle.setTruth(genParticle);
        }
      recoEvent.addParticle(&recoParticle);
      }
    }
  std::vector<EventSmearer*> & eventSmearer = eventSmearers();
  if (!eventSmearer.empty()) eventSmearer[0]->smear(genEvent,recoEvent);
}

} // namespace CAP
