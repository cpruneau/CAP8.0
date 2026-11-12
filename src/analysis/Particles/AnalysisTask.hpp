/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file AnalysisTask.hpp
//! @brief AnalysisTask: base class of the tasks working with event and particle filters (analyzers and calculators).
#ifndef CAP__AnalysisTask
#define CAP__AnalysisTask
#include <vector>
#include "Task.hpp"
#include "Exceptions.hpp"
#include "ManagedObjects.hpp"
#include "EventFilter.hpp"
#include "ParticleFilter.hpp"

namespace CAP
{

  //!
  //! @brief Base class of the tasks whose histograms are organized by combinations of event and particle filters: the
  //! analyzers, which process events (through EventProcessor), and the calculators of the later phases of an analysis
  //! (derived histograms, balance functions), which only read and complete the histograms of an analyzer.
  //!
  //! It holds what both need and nothing more: the event and particle filters of the task, each a ManagedObjects list
  //! configured by the keys \<name\>:EVENT_FILTER... and \<name\>:PARTICLE_FILTER... (filters are usually shared, taken
  //! by name from the store where the filter creators put them), their names, and the naming of the combinations of
  //! filters (combinationName()), which is how the folders of histograms are found in the files of the jobs. Events,
  //! efficiencies, smearers, particle databases and the filtering of events and particles belong to EventProcessor.
  //!
  class AnalysisTask
  :
  public Task
  {
  protected:

  //! Event filters.
  ManagedObjects<EventFilter>        _managedEventFilters;
  //! Particle filters.
  ManagedObjects<ParticleFilter>     _managedParticleFilters;
  //! Names of the event and particle filters, kept from the configuration for the folders of the histograms in the
  //! files of the job (written after the filter lists are finalized, see combinationName()).
  std::vector<String> _eventFilterNames;      //!
  std::vector<String> _particleFilterNames;   //!

  public:

  //! @brief Default constructor.
  AnalysisTask();
  //! @brief Copy constructor. @param source task to copy
  AnalysisTask(const AnalysisTask & source);
  //! @brief Copy assignment. @param rhs task to copy @return this task
  AnalysisTask & operator=(const AnalysisTask & rhs);
  //! @brief Destructor.
  virtual ~AnalysisTask() {}

  //! @brief Add the default configuration of the task and of its filters (EVENT_FILTER, PARTICLE_FILTER).
  virtual void setDefaultConfiguration();
  //! @brief Configure the task and its filters, and record the names of the filters. @param configuration configuration
  virtual void configure(const Configuration & configuration);
  //! @brief Start the timer, initialize the filters and the subtasks.
  virtual void initialize();
  //! @brief Finalize the subtasks and the filters.
  virtual void finalize();
  //! @brief Reset the filters.
  virtual void reset();
  //! @brief Clear the filters.
  virtual void clear();
  //! @brief Print the filters.
  virtual void print() const;

  //! @name Filters
  //! @{
  //! @brief Number of event filters. @return number
  unsigned int nEventFilters() const  { return _managedEventFilters.size();  }
  //! @brief Number of particle filters. @return number
  unsigned int nParticleFilters() const  { return _managedParticleFilters.size();  }
  //! @brief Event filters. @return filters
  const std::vector<EventFilter*>    & eventFilters() const    { return _managedEventFilters.getObjects(); }
  //! @brief Particle filters. @return filters
  const std::vector<ParticleFilter*> & particleFilters() const { return _managedParticleFilters.getObjects(); }
  //! @brief Event filters. @return filters
  std::vector<EventFilter*>          & eventFilters()          { return _managedEventFilters.getObjects(); }
  //! @brief Particle filters. @return filters
  std::vector<ParticleFilter*>       & particleFilters()       { return _managedParticleFilters.getObjects(); }
  //! @}

  //!
  //! @brief Name of the folder of the histograms of a combination of filters, in the files of the job:
  //! \<event filter\>_\<particle filter\> (or \<event filter\>_\<particle filter 1\>_\<particle filter 2\> for pairs), the
  //! combinations being indexed e*nP + p (or e*nP*nP + p1*nP + p2).
  //! @param index index of the combination @param nParticles number of particle filters of a combination (1 or 2)
  //! @return name (Folder_\<index\> beyond the combinations of the filters)
  //!
  String combinationName(unsigned int index, unsigned int nParticles) const;

  //!
  //! @brief Add to a folder the histogram folders this task writes to the output file of its job, made empty without
  //! processing anything (the histograms the configuration asks for, created but not filled; for the calculators,
  //! with their derived histograms): what a job of the task would write, e.g. to make a plotting template from a
  //! workflow (see WorkflowHistograms). The subtasks are not visited. Nothing here (a task without histograms).
  //! @param top folder of the job
  //!
  virtual void makeEmptyHistograms(Folder & top) { (void)top; }

  //! @brief Scale the histograms by a factor (implemented by derived classes). @param factor factor
  //! @throws NoImplementationException here
  virtual void scaleHistograms(double factor);
  //! @brief Scale the histograms of each event filter by its factor (implemented by derived classes).
  //! @param factors factors @throws NoImplementationException here
  virtual void scaleHistograms(const std::vector<double> & factors);

  protected:

  //! @brief Record the names of the filters (after they are configured).
  void recordFilterNames();

  };

} // namespace CAP

#endif /* CAP__AnalysisTask */
