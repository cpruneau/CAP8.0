/* **********************************************************************
 * Copyright (C) 2019-2024, Claude Pruneau, Victor Gonzalez   
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__EventProcessor
#define CAP__EventProcessor
#include "TObject.h"
#include "Task.hpp"
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
#include "ClassDef.hpp"

namespace CAP
{

  //!
  //! @brief Base class of the tasks that process events: event generators and readers, analyzers, decayers, ...
  //!
  //! An EventProcessor manages the objects it works with, each kind as a ManagedObjects list configured by keys
  //! \<name\>:\<TYPE\>_\<k\>:... (see setDefaultConfiguration()):
  //! - EVENT: the events (event() is the first one);
  //! - EVENT_FILTER, PARTICLE_FILTER: event and particle selections; histograms are filled separately for each
  //!   accepted event filter and each particle filter;
  //! - EVENT_EFFICIENCY, PARTICLE_EFFICIENCY, EVENT_SMEARER, PARTICLE_SMEARER: detector simulation;
  //! - PARTICLE_DB: particle databases (db() is the first one).
  //!
  //! For each event, filterEvent() records which event filters accept it and filterParticles() builds, for each particle
  //! filter, the list of accepted particles and their pT moments (sum of pT^k, k=0..ptOrder()). Derived classes
  //! implement execute() (the base version throws).
  //!
  class EventProcessor
  :
  public Task
  {
  protected:

  //! Events.
  ManagedObjects<Event>              _managedEvents;
  //! Event filters.
  ManagedObjects<EventFilter>        _managedEventFilters;
  //! Event efficiencies.
  ManagedObjects<EventEfficiency>    _managedEventEfficiencies;
  //! Event smearers.
  ManagedObjects<EventSmearer>       _managedEventSmearers;
  //! Particle databases.
  ManagedObjects<ParticleDb>         _managedParticleDbs;
  //! Particle filters.
  ManagedObjects<ParticleFilter>     _managedParticleFilters;
  //! Particle efficiencies.
  ManagedObjects<ParticleEfficiency> _managedParticleEfficiencies;
  //! Particle smearers.
  ManagedObjects<ParticleSmearer>    _managedParticleSmearers;

  //! Whether each event filter accepted the current event (see filterEvent()).
  std::vector<bool>   _acceptedEventFilters;
  //! Whether each particle filter accepted at least one particle of the current event (see filterParticles()).
  std::vector<bool>   _acceptedParticleFilters;
  //! pT moments (sum of pT^k, k=0..ptOrder()) of the particles accepted by particle filters 0 to 7 (see moments()).
  std::vector<double> _particleMomentsFilter0;
  std::vector<double> _particleMomentsFilter1;  ///< pT moments of the particles accepted by particle filter 1
  std::vector<double> _particleMomentsFilter2;  ///< pT moments of the particles accepted by particle filter 2
  std::vector<double> _particleMomentsFilter3;  ///< pT moments of the particles accepted by particle filter 3
  std::vector<double> _particleMomentsFilter4;  ///< pT moments of the particles accepted by particle filter 4
  std::vector<double> _particleMomentsFilter5;  ///< pT moments of the particles accepted by particle filter 5
  std::vector<double> _particleMomentsFilter6;  ///< pT moments of the particles accepted by particle filter 6
  std::vector<double> _particleMomentsFilter7;  ///< pT moments of the particles accepted by particle filter 7
  //! Particles of the current event accepted by particle filters 0 to 7 (see acceptedParticles()).
  std::vector<Particle*> _allAcceptedParticlesFilter0;
  std::vector<Particle*> _allAcceptedParticlesFilter1;  ///< particles accepted by particle filter 1
  std::vector<Particle*> _allAcceptedParticlesFilter2;  ///< particles accepted by particle filter 2
  std::vector<Particle*> _allAcceptedParticlesFilter3;  ///< particles accepted by particle filter 3
  std::vector<Particle*> _allAcceptedParticlesFilter4;  ///< particles accepted by particle filter 4
  std::vector<Particle*> _allAcceptedParticlesFilter5;  ///< particles accepted by particle filter 5
  std::vector<Particle*> _allAcceptedParticlesFilter6;  ///< particles accepted by particle filter 6
  std::vector<Particle*> _allAcceptedParticlesFilter7;  ///< particles accepted by particle filter 7

  //! Highest order of the pT moments computed by filterParticles().
  unsigned int _ptMomentOrder;
  //! Names of the event and particle filters, kept from the configuration for the folders of the histograms in the
  //! files of the job (written after the filter lists are finalized, see combinationName()).
  std::vector<String> _eventFilterNames;      //!
  std::vector<String> _particleFilterNames;   //!
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
  //! @brief Number of event filters. @return number
  unsigned int nEventFilters() const  { return _managedEventFilters.size();  }
  //! @brief Number of event efficiencies. @return number
  unsigned int nEventEfficiencies() const  { return _managedEventEfficiencies.size();  }
  //! @brief Number of event smearers. @return number
  unsigned int nEventSmearers() const  { return _managedEventSmearers.size();  }
  //! @brief Number of particle databases. @return number
  unsigned int nParticleDbs() const  { return _managedParticleDbs.size();  }
  //! @brief Number of particle filters. @return number
  unsigned int nParticleFilters() const  { return _managedParticleFilters.size();  }
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
  //! @brief Event filters. @return filters
  const std::vector<EventFilter*>       & eventFilters() const         { return _managedEventFilters.getObjects(); }
  //! @brief Event efficiencies. @return efficiencies
  const std::vector<EventEfficiency*>   & eventEfficiencies() const    { return _managedEventEfficiencies.getObjects(); }
  //! @brief Event smearers. @return smearers
  const std::vector<EventSmearer*>      & eventSmearers() const        { return _managedEventSmearers.getObjects(); }
  //! @brief Particle databases. @return databases
  const std::vector<ParticleDb*>        & particleDbs() const          { return _managedParticleDbs.getObjects(); }
  //! @brief Particle filters. @return filters
  const std::vector<ParticleFilter*>    & particleFilters() const      { return _managedParticleFilters.getObjects(); }
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
  //! @brief Event filters. @return filters
  std::vector<EventFilter*>       & eventFilters()                     { return _managedEventFilters.getObjects(); }
  //! @brief Event efficiencies. @return efficiencies
  std::vector<EventEfficiency*>   & eventEfficiencies()                { return _managedEventEfficiencies.getObjects(); }
  //! @brief Event smearers. @return smearers
  std::vector<EventSmearer*>      & eventSmearers()                    { return _managedEventSmearers.getObjects(); }
  //! @brief Particle databases. @return databases
  std::vector<ParticleDb*>        & particleDbs()                      { return _managedParticleDbs.getObjects(); }
  //! @brief Particle filters. @return filters
  std::vector<ParticleFilter*>    & particleFilters()                  { return _managedParticleFilters.getObjects(); }
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
  //! @brief Particles of the current event accepted by a particle filter. @param index filter index (0 to 7)
  //! @return particles @throws MemoryException if index>7
  const std::vector<Particle*> & acceptedParticles(unsigned int index) const;
  //! @brief Particles of the current event accepted by a particle filter. @param index filter index (0 to 7)
  //! @return particles @throws MemoryException if index>7
  std::vector<Particle*> & acceptedParticles(unsigned int index);
  //! @brief pT moments (sum of pT^k, k=0..ptOrder()) of the particles accepted by a particle filter.
  //! @param index filter index (0 to 7) @return moments @throws MemoryException if index>7
  std::vector<double> & moments(unsigned int index);
  //! @brief Value of the quantity used to classify the current event (multiplicity, energy or spherocity, according
  //! to the event class type). @return value @throws Exception for an unknown event class type
  double getEvtClassValue() const;
  //! @brief Divide the histograms of each event filter by the number of events it accepted (see eventAccepted()).
  virtual void scaleHistograms();
  //! @brief Scale the histograms by a factor. @param factor factor
  virtual void scaleHistograms(double factor);
  //! @brief Scale the histograms of each event filter by its factor. @param factors factors
  virtual void scaleHistograms(const std::vector<double> & factors);

  //! @brief Post-processing after the run: calculateDerived().
  virtual void postProcess();
  //! @brief Compute the derived histograms (implemented by derived classes). @throws NoImplementationException here
  virtual void calculateDerived();
  //!
  //! @brief Name of the folder of the histograms of a combination of filters, in the files of the job:
  //! \<event filter\>_\<particle filter\> (or \<event filter\>_\<particle filter 1\>_\<particle filter 2\> for pairs), the
  //! combinations being indexed e*nP + p (or e*nP*nP + p1*nP + p2).
  //! @param index index of the combination @param nParticles number of particle filters of a combination (1 or 2)
  //! @return name (Folder_\<index\> beyond the combinations of the filters)
  //!
  String combinationName(unsigned int index, unsigned int nParticles) const;
  //! @brief Compute the balance functions (implemented by derived classes). @throws NoImplementationException here
  virtual void calculateBf();
  //! @brief Apply the event filters to an event: update acceptedEventFilters() and eventAccepted().
  //! @param event event @return true if at least one filter accepts the event
  virtual bool filterEvent(Event & event);
  //! @brief Apply the particle filters to particles: fill acceptedParticles(k) and moments(k) for each filter k, and
  //! acceptedParticleFilters(). @param particles particles @return true if at least one filter accepts a particle
  virtual bool filterParticles(std::vector<Particle*> & particles);

  ClassDef(EventProcessor,1)
  };

} // namespace CAP

#endif /* CAP__EventProcessor */
