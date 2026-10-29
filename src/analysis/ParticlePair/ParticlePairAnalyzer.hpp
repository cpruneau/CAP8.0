/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ParticlePairAnalyzer
#define CAP__ParticlePairAnalyzer
#include "EventProcessorHistos.hpp"
#include "ParticleSingleHistos.hpp"
#include "ParticlePairHistos.hpp"

namespace CAP
{

//!
//! @brief Pair analyzer (event processing) of n event filters and m particle filters. For each accepted event filter e,
//! it calls ParticleSingleHistos::fill() for the particles of each particle filter i (single folder e*m + i) and
//! ParticlePairHistos::fill() for the pairs of each ordered pair of particle filters (i, j) (pair folder
//! e*m*m + i*m + j; a particle is not paired with itself when i = j). The histograms are scaled per accepted event at
//! the end of the event loop. The derived histograms (densities, R2, C2, G2, ...) are computed in a later job by
//! ParticlePairDerivedCalculator.
//!
//! The n*m*m pair folders are the list of histogram folders of the task (HISTOGRAM) and the n*m single folders its
//! single-particle folders (see EventProcessorHistos). In the output file, all are in the folder of the task: \<event\>_\<particle\>
//! (e.g. ALL_PiP) and \<event\>_\<particle i\>_\<particle j\> (e.g. ALL_PiP_PiM).
//!
//! The binning and fill options are those of the HISTOGRAM group (ParticlePairHistos::addDefaultConfiguration()):
//! eta_fill, rapidity_fill, p2_fill, n1_*, pt_*, phi_*, eta_*, rapidity_*, n2_*. Particles and pairs are not weighted
//! by the particle efficiencies.
//!
class ParticlePairAnalyzer
:
public EventProcessorHistos<ParticlePairHistos,2,ParticleSingleHistos>
{
public:
  //! @brief Default constructor.
  ParticlePairAnalyzer();
  //! @brief Copy constructor. @param task task to copy
  ParticlePairAnalyzer(const ParticlePairAnalyzer & task);
  //! @brief Copy assignment. @param task task to copy @return this task
  ParticlePairAnalyzer & operator=(const ParticlePairAnalyzer & task);
  //! @brief Destructor.
  virtual ~ParticlePairAnalyzer() {}
  //! @brief Add the default configuration: that of EventProcessorHistos and the binning and fill options.
  virtual void setDefaultConfiguration() override;
  //! @brief Analyze the current event: fill the histograms of the particles and of the pairs of each accepted event filter.
  virtual void execute() override;
  //!
  //! @brief Finalize the task. After an event loop that filled the histograms (created by this task, not imported),
  //! they are first scaled by 1/(number of events accepted) of their event filter, so that they hold per-event
  //! quantities; the derived histograms are computed from them in a later job, without scaling.
  //!
  virtual void finalize() override;
};

} // namespace CAP

#endif /* CAP__ParticlePairAnalyzer */
