/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ParticlePair3DAnalyzer
#define CAP__ParticlePair3DAnalyzer
#include "EventProcessorHistos.hpp"
#include "ParticlePair3DHistos.hpp"
#include "ParticleSingleHistos.hpp"

namespace CAP
{

//!
//! @brief Pair analyzer in relative momentum variables (event processing) of n event filters and m particle filters:
//! for each accepted event filter e, fills the single-particle histograms of each particle filter i (ParticleSingleHistos,
//! folder e*m + i) and the pair histograms (ParticlePair3DHistos: Qinv, q_side, q_out, q_long) of each ordered pair of
//! particle filters (i, j) (folder e*m*m + i*m + j). The histograms are scaled per accepted event at the end of the
//! event loop. The correlation functions are computed in a later job by ParticlePair3DDerivedCalculator.
//!
class ParticlePair3DAnalyzer
:
public EventProcessorHistos<ParticlePair3DHistos,2,ParticleSingleHistos>
{
public:

  //! @brief Default constructor.
  ParticlePair3DAnalyzer();
  //! @brief Copy constructor. @param task task to copy
  ParticlePair3DAnalyzer(const ParticlePair3DAnalyzer & task);
  //! @brief Copy assignment. @param task task to copy @return this task
  ParticlePair3DAnalyzer & operator=(const ParticlePair3DAnalyzer & task);
  //! @brief Destructor.
  virtual ~ParticlePair3DAnalyzer() {}
  //! @brief Add the default configuration: that of EventProcessorHistos and the histogram settings.
  virtual void setDefaultConfiguration() override;
  //! @brief Analyze the current event: for each accepted event filter, fill the single-particle histograms of each
  //! particle filter and the pair histograms of each ordered pair of particle filters.
  virtual void execute() override;
  //!
  //! @brief Finalize the task. After an event loop that filled the histograms (created by this task, not imported),
  //! they are first scaled by 1/(number of events accepted) of their event filter, so that they hold per-event
  //! quantities; the derived histograms are computed from them in a later job, without scaling.
  //!
  virtual void finalize() override;

protected:
  
  //! @brief Mass of the particles selected by a filter. @param pf filter @return mass
  //! @warning Currently always returns the mass of PDG code 211 (pi+).
  double getMassFor(ParticleFilter* pf);
  
};

} // namespace CAP

#endif /* CAP__ParticlePair3DAnalyzer */
