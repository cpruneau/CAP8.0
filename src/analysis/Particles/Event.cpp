/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include <algorithm>
#include "Event.hpp"
#include "EventFilter.hpp"
#include "PrintHelpers.hpp"


namespace CAP
{

  Event::Event()
  :
  ManagedObject(),
  _eventIndex(0),
  _eventNumber(0),
  _projectileA(),
  _projectileB(),
  _particles(),
  _multiplicity(),
  _energy(),
  _charge(),
  _strangeness(),
  _baryon(),
  _ptSum(),
  _spherocity(),
  _epoch(1),
  _globalsVersion(0),
  _selections(),
  _selectionEpochs(),
  _filterResults(),
  _filterEpochs(),
  _filterVersions()
  {
  initializeMult(6);
  initializeEnergy(6);
  initializeCharge(6);
  initializeStrangeness(6);
  initializeBaryon(6);
  initializePtSum(6);
  initializeSpherocity(6);
  }

  Event::Event(const Event & source)
  :
  ManagedObject(source),
  _eventIndex(source._eventIndex),
  _eventNumber(source._eventNumber),
  _projectileA(source._projectileA),
  _projectileB(source._projectileB),
  _particles(source._particles),
  _multiplicity(source._multiplicity),
  _energy(source._energy),
  _charge(source._charge),
  _strangeness(source._strangeness),
  _baryon(source._baryon),
  _ptSum(source._ptSum),
  _spherocity(source._spherocity),
  _epoch(1),
  _globalsVersion(0),
  _selections(),
  _selectionEpochs(),
  _filterResults(),
  _filterEpochs(),
  _filterVersions()
  { }

  Event & Event::operator=(const Event & source)
  {
  if (this!=&source)
    {
    ManagedObject::operator=(source);
    _eventIndex   = source._eventIndex;
    _eventNumber  = source._eventNumber;
    _projectileA  = source._projectileA;
    _projectileB  = source._projectileB;
    _particles    = source._particles;
    _multiplicity = source._multiplicity;
    _energy       = source._energy;
    _charge       = source._charge;
    _strangeness  = source._strangeness;
    _baryon       = source._baryon;
    _ptSum        = source._ptSum;
    _spherocity   = source._spherocity;
    _epoch++;
    _globalsVersion++;
    }
  return *this;
  }

  Event::~Event()
  {
  _particles.clear();
  }

  //!
  // Call before (re)starting simulation
  //!
  void Event::clear()
  {
  _eventIndex  = 0;
  _eventNumber = 0;
  _particles.clear();
  _epoch++;
  }

  //!
  // Call before generating new event
  //!
  void Event::reset()
  {
  _eventIndex++;
  _eventNumber  = 0;
  _particles.clear();
  _epoch++;
  //  resetMult();
  //  resetEnergy();
  //  resetCharge();
  //  resetStrangeness();
  //  resetBaryon();
  //  resetPtSum();
  //  resetpherocity();
  }

  void Event::addParticle(Particle * particle)
  {
  _particles.push_back(particle);
  _epoch++;
  }

  ParticleSelection & Event::selection(unsigned int slot) const
  {
  if (slot >= _selections.size())
    {
    unsigned int n = std::max(slot+1, ParticleSelection::nSlots());
    for (unsigned int k=(unsigned int)_selections.size(); k<n; k++)
      _selections.emplace_back(new ParticleSelection(ParticleSelection::filterAt(k), k));
    _selectionEpochs.resize(n, 0);
    }
  ParticleSelection & s = *_selections[slot];
  if (_selectionEpochs[slot] != _epoch)
    {
    s.fill(_particles, ParticleSelection::ptOrder());
    _selectionEpochs[slot] = _epoch;
    }
  return s;
  }

  bool Event::accepted(const EventFilter & filter, unsigned int slot) const
  {
  if (slot >= _filterResults.size())
    {
    unsigned int n = std::max(slot+1, ParticleSelection::nEventSlots());
    _filterResults.resize(n, 0);
    _filterEpochs.resize(n, 0);
    _filterVersions.resize(n, 0);
    }
  if (_filterEpochs[slot] != _epoch || _filterVersions[slot] != _globalsVersion)
    {
    _filterResults[slot]  = filter.accept(*this) ? 1 : 0;
    _filterEpochs[slot]   = _epoch;
    _filterVersions[slot] = _globalsVersion;
    }
  return _filterResults[slot] != 0;
  }

  Particle * Event::particleAt(unsigned long index)
  {
  if (index>=_particles.size() )
    throw Exception("index out of bounds","Event::particleAt(unsigned long index)");
  return _particles[index];
  }

  void Event::printParticles() const
  {
  unsigned int n = _particles.size();
  for (unsigned int k=0; k<n; k++)
    {
    printValue("k",k);
    Particle::factory().objectAt(k).print();
    //particles[k]->print();
    }

  //  for (auto & particle : particles) particle->print();
  }

  void Event::print() const
  {
  printCR();
  printLine();
  printValue("Event Name",name());
  printValue("Event Index",_eventIndex);
  printValue("Event Number",_eventNumber);
  //  printString("Projectile A");
  //  projectileA.print();
  //  printString("Projectile B");
  //  projectileB.print();
  printParticles();
  printLine();
  printCR();
  }

} // namespace CAP
