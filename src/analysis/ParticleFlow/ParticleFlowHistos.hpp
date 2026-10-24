/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ParticleFlowHistos
#define CAP__ParticleFlowHistos
#include "HistogramFolder.hpp"
#include "Particle.hpp"
#include "MathConstants.hpp"

namespace CAP
{

//!
//! @brief Single-particle histograms of the particle flow analysis for one (event filter, particle filter) combination.
//! @warning Work in progress, not built (see ParticleFlowAnalyzer).
//!
class ParticleFlowHistos : public HistogramFolder
{
public:
  //! @brief Default constructor.
  ParticleFlowHistos();
  //! @brief Copy constructor. @param source group to copy
  ParticleFlowHistos(const ParticleFlowHistos & source);
  //! @brief Copy assignment. @param rhs group to copy @return this group
  ParticleFlowHistos & operator=(const ParticleFlowHistos & rhs);
  //! @brief Destructor.
  virtual ~ParticleFlowHistos(){ }

  //! @brief Read the binning. @param taskName name of the task @param objectType object type
  //! @param configuration configuration @param index index
  virtual void configure(const String & taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Create the histograms.
  virtual void create();
  //! @brief Load the histograms from a file. @param inputFile file
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Fill the histograms with a particle. @param particle particle @param weight weight
  virtual void fill(Particle & particle, double weight);

protected:

  //! @brief Copy the content of another group. @param group group to copy
  virtual void cloneB(const ParticleFlowHistos & group);

  unsigned int n1_nbins;  ///< number of bins of the multiplicity axis
  float        n1_min;  ///< lower edge of the multiplicity axis
  float        n1_max;  ///< upper edge of the multiplicity axis
  unsigned int pt_nbins;  ///< number of bins of the pT axis
  float        pt_min;  ///< lower edge of the pT axis
  float        pt_max;  ///< upper edge of the pT axis
  float        pt_scale;  ///< inverse bin width of the pT axis
  unsigned int phi_nbins;  ///< number of bins of the azimuth axis
  float        phi_min;  ///< lower edge of the azimuth axis
  float        phi_max;  ///< upper edge of the azimuth axis
  float        phi_scale;  ///< inverse bin width of the azimuth axis
  unsigned int eta_nbins;  ///< number of bins of the pseudorapidity axis
  float        eta_min;  ///< lower edge of the pseudorapidity axis
  float        eta_max;  ///< upper edge of the pseudorapidity axis
  float        eta_range;
  unsigned int rapidity_nbins;  ///< number of bins of the rapidity axis
  float        rapidity_minimum;  ///< lower edge of the rapidity axis
  float        rapidity_maximum;  ///< upper edge of the rapidity axis
  float        range_y;
  unsigned int phiEta_nbins;  ///< number of bins of the phiEta axis
  unsigned int phiEtaPt_nbins;  ///< number of bins of the phiEtaPt axis
  unsigned int phiY_nbins;  ///< number of bins of the phiY axis
  unsigned int phiYPt_nbins;  ///< number of bins of the phiYPt axis
  
  //! Multiplicity of accepted particles per event.
  Histogram * h_n1;


};

} // namespace CAP

#endif /* CAP__ParticleFlowHistos  */



