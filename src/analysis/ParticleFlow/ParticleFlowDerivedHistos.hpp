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
#ifndef CAP__ParticleFlowDerivedHistos
#define CAP__ParticleFlowDerivedHistos
#include "Profile.hpp"
#include "ParticleFlowHistos.hpp"

namespace CAP
{

//!
//! @brief Histograms derived from the particle flow histograms: projections of the densities and pT sums, average pT.
//! @warning Work in progress, not built (loadFrom() has a syntax error in its declaration).
//!
class ParticleFlowDerivedHistos : public HistogramFolder
{
public:

  //! @brief Default constructor.
  ParticleFlowDerivedHistos();
  //! @brief Copy constructor. @param group group to copy
  ParticleFlowDerivedHistos(const ParticleFlowDerivedHistos & group);
  //! @brief Copy assignment. @param group group to copy @return this group
  ParticleFlowDerivedHistos & operator=(const ParticleFlowDerivedHistos & group);
  //! @brief Destructor.
  virtual ~ParticleFlowDerivedHistos() {}
  //! @brief Create the derived histograms.
  virtual void create();
  //! @brief Load the derived histograms from a file. @param inputFile file
  virtual void loadFrom(const Folder & inputFile,);
  //! @brief Compute the derived histograms. @param baseHistos base histograms
  virtual void calculateDerivedHistograms(ParticleFlowHistos & baseHistos);
  //! @brief Copy the content of another group. @param group group to copy
  void cloneAll(const ParticleFlowDerivedHistos & group);


  ////////////////////////////////////////////////////////////////////////////
  // Data Members - HistogramFolder
  ////////////////////////////////////////////////////////////////////////////
  //! Whether the eta histograms are filled.
  bool eta_fill;
  //! Whether the rapidity histograms are filled.
  bool rapidity_fill;
  //! Whether the pT-weighted histograms are filled.
  bool p2_fill;
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

  Histogram * h_n1_phi;  ///< single-particle density vs phi
  Histogram * h_n1_eta;  ///< single-particle density vs eta
  Histogram * h_n1_y;  ///< single-particle density vs y

  Histogram * h_spt_phi;  ///< pT-weighted density vs phi
  Histogram * h_spt_eta;  ///< pT-weighted density vs eta
  Histogram * h_spt_y;  ///< pT-weighted density vs y
  Histogram * h_avg_pt_phi;  ///< average pT vs phi
  Histogram * h_avg_pt_eta;  ///< average pT vs eta
  Histogram * h_avg_pt_phiEta;  ///< average pT vs (eta, phi)
  Histogram * h_avg_pt_y;  ///< average pT vs y
  Histogram * h_avg_pt_phiY;  ///< average pT vs (y, phi)

  ClassDef(ParticleFlowDerivedHistos,0)

};

} // namespace CAP

#endif /* CAP__ParticleFlowDerivedHistos  */



