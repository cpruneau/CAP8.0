/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__EventProcessor
#define CAP__EventProcessor
#include "TObject.h"
#include "AnalysisTask.hpp"
#include "Exceptions.hpp"
#include "ManagedObjects.hpp"
#include "Event.hpp"
#include "EventFilter.hpp"
#include "EventEfficiency.hpp"
#include "EventSmearer.hpp"
#include "ParticleDb.hpp"
#include "Particle.hpp"
#include "ParticleFilter.hpp"
#include "ParticleEfficiency.hpp"
#include "ParticleSmearer.hpp"
#include "ParticleSelection.hpp"
#include <memory>

namespace CAP
{

  //!
  //! @brief Base class of the tasks that process events: event generators and readers, analyzers, decayers, ...
  //!
  //! An EventProcessor manages the objects it works with, each kind as a ManagedObjects list configured by keys
  //! \<name\>:\<TYPE\>_\<k\>:... (see setDefaultConfiguration()):
  //! - EVENT: the events (event() is the first one);
  //! - EVENT_FILTER, PARTICLE_FILTER: event and particle selections (held by the base class AnalysisTask, which it
  //!   shares with the calculators of the later phases); histograms are filled separately for each accepted event filter
  //!   and each particle filter;
  //! - EVENT_EFFICIENCY, PARTICLE_EFFICIENCY, EVENT_SMEARER, PARTICLE_SMEARER: detector simulation;
  //! - PARTICLE_DB: particle databases (db() is the first one).
  //!
  //! For each event, filterEvent() records which event filters accept it and filterParticles() gives, for each particle
  //! filter, the list of accepted particles and their pT moments (sum of pT^k, k=0..ptOrder()). Both are shared by the
  //! tasks: the event keeps the selection of each filter and the result of each event filter, made once per event by
  //! the first task asking (see Event::selection() and ParticleSelection), so that a filter used by several analyzers is
  //! applied once. Derived classes implement execute() (the base version throws).
  //!
  class EventProcessor
  :
  public AnalysisTask
  {
  protected:

  //! Events.
  ManagedObjects<Event>              _managedEvents;
  //! Event efficiencies.
  ManagedObjects<EventEfficiency>    _managedEventEfficiencies;
  //! Event smearers.
  ManagedObjects<EventSmearer>       _managedEventSmearers;
  //! Particle databases.
  ManagedObjects<ParticleDb>         _managedParticleDbs;
  //! Particle efficiencies.
  ManagedObjects<ParticleEfficiency> _managedParticleEfficiencies;
  //! Particle smearers.
  ManagedObjects<ParticleSmearer>    _managedParticleSmearers;

  //! Whether each event filter accepted the current event (see filterEvent()).
  std::vector<bool>   _acceptedEventFilters;
  //! Whether each particle filter accepted at least one particle of the current event (see filterParticles()).
  std::vector<bool>   _acceptedParticleFilters;
  //! Slot of each particle filter in the selections of the events (ParticleSelection::slot()).
  std::vector<unsigned int> _particleSlots;
  //! Slot of each event filter in the results kept by the events.
  std::vector<unsigned int> _eventSlots;
  //! Selection of each particle filter for the current event: in the event (shared, filterParticles(Event &)) or of
  //! this task (filterParticles(std::vector<Particle*> &)).
  std::vector<ParticleSelection*> _selections;
  //! Selections of this task, for particles that are not those of an event.
  std::vector<std::unique_ptr<ParticleSelection>> _ownSelections;

  //! Highest order of the pT moments computed by filterParticles().
  unsigned int _ptMomentOrder;
  //! Number of events accepted by each event filter.
  Accountant _eventAccepted;
  //! Quantity used to classify the events (an EventFilter::EventFilterType, see getEvtClassValue()).
  int _evtType;


  public:

  //! @brief Default constructor.
  EventProcessor();
  //! @brief Copy constructor. @param source task to copy
  EventProcessor(const EventProcessor & source);
  //! @brief Copy assignment. @param rhs task to copy @return this task
  EventProcessor & operator=(const EventProcessor & rhs);
  //! @brief Destructor.
  virtual ~EventProcessor() {}
  //! @brief Add the default configuration of the task and of its managed objects (EVENT, EVENT_FILTER, EVENT_EFFICIENCY,
  //! EVENT_SMEARER, PARTICLE_DB, PARTICLE_FILTER, PARTICLE_EFFICIENCY, PARTICLE_SMEARER).
  virtual void setDefaultConfiguration();
  //! @brief Configure the task and its managed objects. @param configuration configuration
  virtual void configure(const Configuration & configuration);
  //! @brief Start the timer, initialize the managed objects, the filter flags and pT moments, and the subtasks.
  virtual void initialize();
  //! @brief Process the current event (implemented by derived classes). @throws NoImplementationException here
  virtual void execute();
  //! @brief Finalize the task and its subtasks.
  virtual void finalize();
  //! @brief Reset the task (counters, histograms) and its subtasks.
  virtual void reset();
  //! @brief Clear the task and its subtasks.
  virtual void clear();
  //! @brief Print the task.
  virtual void print() const;
  //! @name Managed objects
  //! @{

  //! @brief Number of events. @return number
  unsigned int nEvents() const  { return _managedEvents.size();  }
  //! @brief Whether this task makes its events (creates or imports them: a generator, a reader, a model).
  //! Their reset before each event is the event iterator's (see EventIterator). @return true if so
  bool makesEvents() const { return _managedEvents.create() || _managedEvents.imports(); }
  //! @brief Number of event efficiencies. @return number
  unsigned int nEventEfficiencies() const  { return _managedEventEfficiencies.size();  }
  //! @brief Number of event smearers. @return number
  unsigned int nEventSmearers() const  { return _managedEventSmearers.size();  }
  //! @brief Number of particle databases. @return number
  unsigned int nParticleDbs() const  { return _managedParticleDbs.size();  }
  //! @brief Number of particle efficiencies. @return number
  unsigned int nParticleEfficiencies() const  { return _managedParticleEfficiencies.size();  }
  //! @brief Number of particle smearers. @return number
  unsigned int nParticlesmearers() const  { return _managedParticleSmearers.size();  }
  //! @brief First particle database. @return database
  const ParticleDb & db() const                                         { return *_managedParticleDbs.objectAt(0); }
  //! @brief First (current) event. @return event
  const Event & event() const                                           { return *_managedEvents.objectAt(0); }
  //! @brief Events. @return events
  const std::vector<Event*>             & events() const               { return _managedEvents.getObjects(); }
  //! @brief Event efficiencies. @return efficiencies
  const std::vector<EventEfficiency*>   & eventEfficiencies() const    { return _managedEventEfficiencies.getObjects(); }
  //! @brief Event smearers. @return smearers
  const std::vector<EventSmearer*>      & eventSmearers() const        { return _managedEventSmearers.getObjects(); }
  //! @brief Particle databases. @return databases
  const std::vector<ParticleDb*>        & particleDbs() const          { return _managedParticleDbs.getObjects(); }
  //! @brief Particle efficiencies. @return efficiencies
  const std::vector<ParticleEfficiency*> & particleEfficiencies() const { return _managedParticleEfficiencies.getObjects(); }
  //! @brief Particle smearers. @return smearers
  const std::vector<ParticleSmearer*>   & particleSmearers() const     { return _managedParticleSmearers.getObjects(); }

  //! @brief First particle database. @return database
  ParticleDb & db()                                                     { return *_managedParticleDbs.objectAt(0); }
  //! @brief First (current) event. @return event
  Event & event()                                                       { return *_managedEvents.objectAt(0); }
  //! @brief Events. @return events
  std::vector<Event*> & events()                                        { return _managedEvents.getObjects(); }
  //! @brief Event efficiencies. @return efficiencies
  std::vector<EventEfficiency*>   & eventEfficiencies()                { return _managedEventEfficiencies.getObjects(); }
  //! @brief Event smearers. @return smearers
  std::vector<EventSmearer*>      & eventSmearers()                    { return _managedEventSmearers.getObjects(); }
  //! @brief Particle databases. @return databases
  std::vector<ParticleDb*>        & particleDbs()                      { return _managedParticleDbs.getObjects(); }
  //! @brief Particle efficiencies. @return efficiencies
  std::vector<ParticleEfficiency*> & particleEfficiencies()             { return _managedParticleEfficiencies.getObjects(); }
  //! @brief Particle smearers. @return smearers
  //! @}
  std::vector<ParticleSmearer*>   & particleSmearers()                 { return _managedParticleSmearers.getObjects(); }

  //! @brief Set the highest order of the pT moments computed by filterParticles(). @param order order
  void setPtOrder(unsigned int order) { _ptMomentOrder = order; }
  //! @brief Highest order of the pT moments computed by filterParticles(). @return order
  unsigned int ptOrder() const { return _ptMomentOrder; }
  //! @brief Number of events accepted by each event filter. @return counter
  Accountant & eventAccepted() { return _eventAccepted; }
  //! @brief Number of events accepted by each event filter. @return counter
  const Accountant & eventAccepted() const { return _eventAccepted; }
  //! @brief Whether each event filter accepted the current event. @return flags
  const std::vector<bool> & acceptedEventFilters() const { return _acceptedEventFilters; }
  //! @brief Whether each particle filter accepted at least one particle of the current event. @return flags
  const std::vector<bool> & acceptedParticleFilters() const { return _acceptedParticleFilters;}
  //! @brief Particles of the current event accepted by a particle filter (after filterParticles()).
  //! @param index filter index @return particles @throws MemoryException for an index without selection
  const std::vector<Particle*> & acceptedParticles(unsigned int index) const;
  //! @brief Particles of the current event accepted by a particle filter (shared with the other tasks: not to be
  //! changed). @param index filter index @return particles @throws MemoryException for an index without selection
  std::vector<Particle*> & acceptedParticles(unsigned int index);
  //! @brief pT moments (sum of pT^k, k=0..ptOrder() at least) of the particles accepted by a particle filter.
  //! @param index filter index @return moments @throws MemoryException for an index without selection
  std::vector<double> & moments(unsigned int index);
  //! @brief Selection of a particle filter for the current event (after filterParticles()): its particles, moments
  //! and global quantities. @param index filter index @return selection @throws MemoryException for an index
  //! without selection
  const ParticleSelection & selection(unsigned int index) const;
  //! @brief Value of the quantity used to classify the current event (multiplicity, energy or spherocity, according
  //! to the event class type). @return value @throws Exception for an unknown event class type
  double getEvtClassValue() const;
  //! @brief Divide the histograms of each event filter by the number of events it accepted (see eventAccepted()).
  virtual void scaleHistograms();
  using AnalysisTask::scaleHistograms;

  //! @brief Post-processing after the run: calculateDerived().
  virtual void postProcess();
  //! @brief Compute the derived histograms (implemented by derived classes). @throws NoImplementationException here
  virtual void calculateDerived();
  //! @brief Compute the balance functions (implemented by derived classes). @throws NoImplementationException here
  virtual void calculateBf();
  //! @brief Apply the event filters to an event: update acceptedEventFilters() and eventAccepted().
  //! @param event event @return true if at least one filter accepts the event
  virtual bool filterEvent(Event & event);
  //! @brief The selections of the particle filters in an event (made once per event, shared by the tasks):
  //! acceptedParticles(k), moments(k), selection(k) for each filter k, and acceptedParticleFilters().
  //! @param event event @return true if at least one filter accepts a particle
  virtual bool filterParticles(Event & event);
  //! @brief Apply the particle filters to particles that are not those of an event (this task only): as
  //! filterParticles(Event &). @param particles particles @return true if at least one filter accepts a particle
  virtual bool filterParticles(std::vector<Particle*> & particles);
  //! @brief Give the filters their slots (ParticleSelection::slot()); called by initialize().
  void assignSlots();

  };

} // namespace CAP

#endif /* CAP__EventProcessor */
