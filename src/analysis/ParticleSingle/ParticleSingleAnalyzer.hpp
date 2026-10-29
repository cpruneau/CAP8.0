/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ParticleSingleAnalyzer
#define CAP__ParticleSingleAnalyzer
#include "EventProcessorHistos.hpp"
#include "ParticleSingleHistos.hpp"

namespace CAP
{

//!
//! @brief Single-particle analyzer (event processing): fills, for each accepted (event filter, particle filter)
//! combination, the multiplicity, energy, pT, (eta, phi), (y, phi) and (y, pT) distributions of the accepted particles
//! (the base histograms of ParticleSingleHistos), and scales them per accepted event at the end of the event loop.
//! The derived histograms are computed in a later job by ParticleSingleDerivedCalculator.
//!
//! Particles are weighted by the inverse of their efficiency when particle efficiencies are configured (one for all
//! particle filters or one per filter). The histogram binning is set by the keys \<name\>:HISTOGRAM:n1_nbins, n1_min,
//! n1_max, pt_nbins, pt_min, pt_max, phi_nbins, ..., eta_fill, rapidity_fill, p2_fill, FillPtvsY, FillPid.
//!
class ParticleSingleAnalyzer
:
public EventProcessorHistos<ParticleSingleHistos,1>
{
public:
  
  //! @brief Default constructor.
  ParticleSingleAnalyzer();
  //! @brief Copy constructor. @param analyzer analyzer to copy
  ParticleSingleAnalyzer(const ParticleSingleAnalyzer & analyzer);
  //! @brief Copy assignment. @param analyzer analyzer to copy @return this analyzer
  ParticleSingleAnalyzer & operator=(const ParticleSingleAnalyzer & analyzer);
  //! @brief Destructor.
  virtual ~ParticleSingleAnalyzer() {}
  //! @brief Add the default configuration: that of EventProcessorHistos and the histogram binning (see the class description).
  virtual void setDefaultConfiguration() override;
  //! @brief Analyze the current event: apply the filters and fill the histograms of each accepted combination.
  //! @throws Exception if there are no histograms or if the number of particle efficiencies is neither 1 nor the number
  //! of particle filters
  virtual void execute() override;
  //!
  //! @brief Finalize the task. After an event loop that filled the histograms (created by this task, not imported),
  //! they are first scaled by 1/(number of events accepted) of their event filter, so that they hold per-event
  //! quantities; the derived histograms are computed from them in a later job, without scaling.
  //!
  virtual void finalize() override;

protected:

};

} // namespace CAP

#endif /* CAP__ParticleSingleAnalyzer */
