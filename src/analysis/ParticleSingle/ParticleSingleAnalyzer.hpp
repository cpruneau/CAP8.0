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
#ifndef CAP__ParticleSingleAnalyzer
#define CAP__ParticleSingleAnalyzer
#include "EventProcessorSingle.hpp"
#include "ParticleSingleHistos.hpp"
#include "ParticleSingleDerivedHistos.hpp"

namespace CAP
{

//!
//! @brief Single-particle analyzer: fills, for each accepted (event filter, particle filter) combination, the
//! multiplicity, energy, pT, (eta, phi), (y, phi) and (y, pT) distributions of the accepted particles
//! (ParticleSingleHistos), and computes their projections and pT averages at the end (ParticleSingleDerivedHistos).
//!
//! Particles are weighted by the inverse of their efficiency when particle efficiencies are configured (one for all
//! particle filters or one per filter). The histogram binning is set by the keys \<name\>:HISTOGRAM:n1_nbins, n1_min,
//! n1_max, pt_nbins, pt_min, pt_max, phi_nbins, ..., eta_fill, rapidity_fill, p2_fill, FillPtvsY, FillPid.
//!
class ParticleSingleAnalyzer
:
public EventProcessorSingle<ParticleSingleHistos,ParticleSingleDerivedHistos>
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
  //! @brief Add the default configuration: that of EventProcessorSingle and the histogram binning (see the class description).
  virtual void setDefaultConfiguration();
  //! @brief Analyze the current event: apply the filters and fill the histograms of each accepted combination.
  //! @throws Exception if there are no histograms or if the number of particle efficiencies is neither 1 nor the number
  //! of particle filters
  virtual void execute();

protected:

  ClassDef(ParticleSingleAnalyzer,0)
};

} // namespace CAP

#endif /* CAP__ParticleSingleAnalyzer */
