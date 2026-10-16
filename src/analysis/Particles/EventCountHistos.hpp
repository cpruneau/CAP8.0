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
#ifndef CAP__EventCountHistos
#define CAP__EventCountHistos
#include "HistogramFolder.hpp"
#include "Configuration.hpp"
#include "EventFilter.hpp"
#include "ParticleFilter.hpp"

namespace CAP
{

//!
//! @brief Histograms of the event and particle counts of a task: number of executions, number of events accepted by
//! each event filter, and number of particles accepted by each (event filter, particle filter) combination.
//!
class EventCountHistos : public HistogramFolder
{
public:

  //! @brief Default constructor.
  EventCountHistos();
  //! @brief Copy constructor. @param group group to copy
  EventCountHistos(const EventCountHistos & group);
  //! @brief Copy assignment. @param group group to copy @return this group
  EventCountHistos & operator=(const EventCountHistos & group);
  //! @brief Copy the content of another group. @param group group to copy
  void cloneAll(const EventCountHistos & group);
  //! @brief Destructor.
  virtual ~EventCountHistos(){}
  //! @brief Create the histograms nTaskExecuted, nEventAccepted and nParticleAccepted.
  virtual void create();
  //! @brief Load the histograms from a file. @param inputFile file
  virtual void loadFrom(CapFile & inputFile);
  //! @brief Add counts to the histograms.
  //! @param taskExecuted number of executions @param nEventsAccepted number of events accepted by each event filter
  //! @param nParticleAccepted number of particles accepted by each combination (index iEventFilter*nParticleFilters+iParticleFilter)
  virtual void fill(long taskExecuted,
                    vector<long> & nEventsAccepted,
                    vector<long> & nParticleAccepted);

protected:

  
  
  //! Number of (event filter, particle filter) combinations.
  unsigned int nFilterCombos;

  //! Number of executions of the task.
  Histogram * h_taskExecuted;
  //! Number of events accepted, per event filter.
  Histogram * h_eventAccepted;
  //! Number of particles accepted, per (event filter, particle filter) combination.
  Histogram * h_particleAccepted;

  //! Event filters of the task.
  vector<EventFilter*> eventFilters;
  //! Particle filters of the task.
  vector<ParticleFilter*> particleFilters;

  ClassDef(EventCountHistos,0)

};

} // namespace CAP

#endif /* CAP__EventCountHistos  */



