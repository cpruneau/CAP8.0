/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file ParticleFlowAnalyzer.hpp
//! @brief ParticleFlowAnalyzer: analysis of anisotropic flow by multi-particle cumulants (any harmonic and order, with
//! or without a rapidity gap) and by the scalar product method, integral and differential.
#ifndef CAP__ParticleFlowAnalyzer
#define CAP__ParticleFlowAnalyzer
#include "EventProcessorHistos.hpp"
#include "ParticleFlowHistos.hpp"

namespace CAP
{

//!
//! @brief Analysis of anisotropic flow: the multi-particle correlators of the reference particles (RF) and of the
//! particles of interest (POI), filled event by event into ParticleFlowHistos (one folder per event filter and RF
//! filter); the cumulants and flow coefficients are computed from them by ParticleFlowDerivedCalculator.
//!
//! Two sets of particle filters:
//! - PARTICLE_FILTER (as every analyzer): the reference particles, one histogram folder per filter (and per event
//!   filter);
//! - POI_FILTER: the particles of interest, a set of differential histograms per filter in every folder. Its keys are
//!   those of a list of managed objects (\<task\>:POI_FILTER:N, POI_FILTER_\<k\>:NAME, ...), the filters taken from the
//!   particle filters made by an earlier task (a FilterCreator) by name, as for PARTICLE_FILTER. A particle accepted by
//!   both an RF filter and a POI filter is one particle (its self-correlations are removed).
//!
//! The particles may be weighted by the inverse of their efficiency (PARTICLE_EFFICIENCY: one for all, one per RF
//! filter, or one per event filter and RF filter): the efficiency of the RF filter of a folder weights its RF and its
//! POI. The event class variable (the x axis of all the histograms) is the event filter quantity
//! (EventFilter type, see EventProcessor::getEvtClassValue()).
//!
//! Settings: see ParticleFlowHistos (nHarmonics, maxOrder, the gap and the scalar product, the binning).
//!
class ParticleFlowAnalyzer
:
public EventProcessorHistos<ParticleFlowHistos,1>
{
public:

  typedef EventProcessorHistos<ParticleFlowHistos,1> Base;

  //! @brief Default constructor.
  ParticleFlowAnalyzer();
  //! @brief Copy constructor. @param source task to copy
  ParticleFlowAnalyzer(const ParticleFlowAnalyzer & source);
  //! @brief Destructor.
  virtual ~ParticleFlowAnalyzer() {}
  //! @brief Copy assignment. @param rhs task to copy @return this task
  ParticleFlowAnalyzer & operator=(const ParticleFlowAnalyzer & rhs);
  //! @brief Copy of the task. @return new task
  virtual Task * clone() override { return new ParticleFlowAnalyzer(*this); }
  //! @brief Add the default configuration: that of the histograms and of the POI filters (POI_FILTER).
  virtual void setDefaultConfiguration() override;
  //! @brief Configure the task, the POI filters, and give the histogram folders the names of the POI filters.
  //! @param configuration configuration
  virtual void configure(const Configuration & configuration) override;
  //! @brief Initialize the task and the POI filters (their slots in the events).
  virtual void initialize() override;
  //! @brief Fill the correlators of the current event.
  virtual void execute() override;
  //! @brief Finalize the task and the POI filters.
  virtual void finalize() override;
  //! @brief Number of POI filters. @return number
  unsigned int nPoiFilters() const { return _managedPoiFilters.size(); }
  //! @brief POI filters. @return filters
  const std::vector<ParticleFilter*> & poiFilters() const { return _managedPoiFilters.getObjects(); }

protected:

  //! POI filters.
  ManagedObjects<ParticleFilter> _managedPoiFilters;
  //! Slot of each POI filter in the selections of the events.
  std::vector<unsigned int> _poiSlots;
};

} // namespace CAP

#endif /* CAP__ParticleFlowAnalyzer */
