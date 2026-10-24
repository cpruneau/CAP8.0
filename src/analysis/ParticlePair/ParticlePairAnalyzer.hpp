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
#include "EventProcessorPair.hpp"
#include "ParticleSingleDerivedHistos.hpp"
#include "ParticlePairHistos.hpp"
#include "ParticlePairDerivedHistos.hpp"
#include "ParticlePairBfHistos.hpp"
#include "ClassDef.hpp"

namespace CAP
{
  //! @brief Pair analyzer: fills pair histograms for each (event filter, particle filter 1, particle filter 2)
  //! combination and computes correlation functions and balance functions at the end of the run.
  //!
  //! Histogram groups (see EventProcessorPair): HISTOGRAM_1 ParticlePairHistos, HISTOGRAM_2 ParticlePairDerivedHistos,
  //! HISTOGRAM_3 ParticlePairBfHistos, HISTOGRAM_4 ParticleSingleDerivedHistos (imported from a single-particle
  //! analysis). For identical filters, a particle is not paired with itself.
  //!
  //! Pairs are not weighted by the particle efficiencies.
  //!
  //! Task used for the analysis of particle  pair distributions and correlations. As for other tasks classes of this package, use event filters and particle filters to determine the
  //! event selection and particle types and kinematic ranges across which  particle pair  distributions are studied. Particle particle distributions are computed
  //! (histogrammed) with instances of the ParticlePairHistos class. The behavior of the class, i.e., what type and the range of filled histograms are determined by
  //! class configuration provided at task (object) construction.
  //!
  //! The following configuration parameters determine what types of histograms are filled (default values in brackets):
  //! - eta_fill [true]: whether to fill histograms  vs. pseudorapidity "eta"
  //! - rapidity_fill [false]: whether to fill histograms  vs. rapidity "y"
  //! - p2_fill [false]: whether to fill histograms used in the determination of P2 and G2 pT correlators
  //!
  //! The following parameters specify  the configuration of histograms filled by this task (default values in brackets):
  //!
  //! - Multiplicity of pair histogram
  //!  + n2_nbins [100]: Number of bins
  //!  + n2_min [0.0]: Minimum multiplicity
  //!  + n2_max [100]: Maximum multiplicity
  //! - Particle transverse momemtum
  //!  + pt_nbins [100]: Number of bins
  //!  + pt_min [0.0]: Minimum value
  //!  + pt_max [100.0]: Maximum value
  //! - Particle pseudorapidity
  //!  + eta_nbins [20]: Number of bins
  //!  + eta_min [-1.0]: Minimum value
  //!  + eta_max [1.0]: Maximum value
  //! - Particle rapidity
  //!  + rapidity_nbins [20]: Number of bins
  //!  + rapidity_minimum [-1.0]: Minimum value
  //!  + rapidity_maximum [1.0]: Maximum value
  //! - Particle azimuthal angle
  //!  + phi_nbins [36]: Number of bins
  //!  + phi_min [0.0]: Minimum value
  //!  + phi_max [2pi]: Maximum value
  //!

class ParticlePairAnalyzer
:
public EventProcessorPair<ParticlePairHistos,ParticlePairDerivedHistos,ParticlePairBfHistos,ParticleSingleDerivedHistos>
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
  //! @brief Add the default configuration: that of EventProcessorPair and the histogram binning and fill options.
  virtual void setDefaultConfiguration();
  //! @brief Analyze the current event: apply the filters and fill the pair histograms of each accepted combination.
  virtual void execute();

  ClassDef(ParticlePairAnalyzer,0)
};

} // namespace CAP

#endif /* CAP__ParticlePairAnalyzer */
