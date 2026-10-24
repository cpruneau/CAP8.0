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
#include "EventProcessorPair.hpp"
#include "ParticleSingleDerivedHistos.hpp"
#include "ParticlePair3DHistos.hpp"
#include "ParticlePair3DDerivedHistos.hpp"
#include "ParticlePair3DBfHistos.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Pair analyzer in relative momentum variables: fills ParticlePair3DHistos (Qinv, q_side, q_out, q_long) for
//! each (event filter, particle filter 1, particle filter 2) combination and computes correlation functions
//! (ParticlePair3DDerivedHistos) and balance functions (ParticlePair3DBfHistos) at the end of the run.
//!
class ParticlePair3DAnalyzer
:
public EventProcessorPair<ParticlePair3DHistos,ParticlePair3DDerivedHistos,ParticlePair3DBfHistos,ParticleSingleDerivedHistos>
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
  //! @brief Add the default configuration: that of EventProcessorPair and the histogram binning.
  virtual void setDefaultConfiguration();
  //! @brief Analyze the current event: apply the filters and fill the pair histograms of each accepted combination.
  virtual void execute();

protected:
  
  //! @brief Mass of the particles selected by a filter. @param pf filter @return mass
  //! @warning Currently always returns the mass of PDG code 211 (pi+).
  double getMassFor(ParticleFilter* pf);
  
  ClassDef(ParticlePair3DAnalyzer,0)
};

} // namespace CAP

#endif /* CAP__ParticlePair3DAnalyzer */
