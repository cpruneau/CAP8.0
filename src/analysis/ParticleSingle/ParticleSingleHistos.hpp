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
#ifndef CAP__ParticleSingleHistos
#define CAP__ParticleSingleHistos
#include "HistogramFolder.hpp"
#include "Particle.hpp"
#include "MathConstants.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Single-particle histograms of one (event filter, particle filter) combination.
//!
//! Always filled: n1 (multiplicity of accepted particles per event), n1_eTotal (their total energy), n1_pt and
//! n1_ptXS (1/pT dN/dpT). Optional: n1_phiEta and spt_phiEta (pT-weighted) if eta_fill (spt_ if p2_fill),
//! n1_phiY and spt_phiY if rapidity_fill, n1_ptY if FillPtvsY, n1_indexId (PDG codes) if FillPid.
//! Histogram names begin with the base name of the group.
//!
class ParticleSingleHistos : public HistogramFolder
{
public:
  //! @brief Default constructor.
  ParticleSingleHistos();
  //! @brief Copy constructor. @param source group to copy
  ParticleSingleHistos(const ParticleSingleHistos & source);
  //! @brief Copy assignment. @param rhs group to copy @return this group
  ParticleSingleHistos & operator=(const ParticleSingleHistos & rhs);
  //! @brief Destructor.
  virtual ~ParticleSingleHistos(){ }
  //! @brief Copy the content of another group. @param source group to copy
  virtual void cloneB(const ParticleSingleHistos & source);
  //! @brief Read the binning and fill options from keys \<taskName\>:HISTOGRAM:... (see the class description).
  //! @param taskName name of the task @param objectType object type @param configuration configuration @param index index
  virtual void configure(const String & taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Create the histograms (according to the fill options).
  virtual void create();
  //! @brief Load the histograms from a file. @param inputFile file
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Fill the particle histograms. @param particle particle @param weight weight (e.g., inverse efficiency)
  virtual void fill(Particle & particle, double weight);
  //! @brief Fill the event histograms. @param nAccepted number of accepted particles @param totalEnergy their total energy
  //! @param weight weight
  virtual void fillMultiplicity(double nAccepted, double totalEnergy, double weight);
  //! @brief Scale the histograms. @param factor factor
  virtual void scaleObject(double factor);

public:

  ////////////////////////////////////////////////////////////////////////////
  // Data Members - HistogramFolder
  ////////////////////////////////////////////////////////////////////////////
  //! Whether the (eta, phi) histograms are filled.
  bool eta_fill;
  //! Whether the (y, phi) histograms are filled.
  bool rapidity_fill;
  //! Whether the pT-weighted histograms (spt_) are filled.
  bool p2_fill;
  //! Whether the (y, pT) histogram is filled.
  bool ptVsRapidity_fill;
  //! Whether the PDG code histogram is filled.
  bool pid_fill;

  int n1_nbins;  ///< number of bins of the multiplicity axis
  int pt_nbins;  ///< number of bins of the pT axis
  int phi_nbins;  ///< number of bins of the azimuth axis
  int eta_nbins;  ///< number of bins of the pseudorapidity axis
  int rapidity_nbins;  ///< number of bins of the rapidity axis
  double n1_min;  ///< lower edge of the multiplicity axis
  double n1_max;  ///< upper edge of the multiplicity axis
  double pt_min;  ///< lower edge of the pT axis
  double pt_max;  ///< upper edge of the pT axis
  double phi_min;  ///< lower edge of the azimuth axis
  double phi_max;  ///< upper edge of the azimuth axis
  double eta_min;  ///< lower edge of the pseudorapidity axis
  double eta_max;  ///< upper edge of the pseudorapidity axis
  double rapidity_minimum;  ///< lower edge of the rapidity axis
  double rapidity_maximum;  ///< upper edge of the rapidity axis

  //! Multiplicity of accepted particles per event.
  Histogram * h_n1;
  //! Total energy of the accepted particles per event.
  Histogram * h_n1_eTotal;
  //! dN/dpT.
  Histogram * h_n1_pt;
  //! 1/pT dN/dpT.
  Histogram * h_n1_ptXS;  // 1/pt dN/dptdy

  //! dN/(deta dphi).
  Histogram * h_n1_phiEta;
  //! Sum of pT vs (eta, phi).
  Histogram * h_spt_phiEta;

  //! dN/(dy dphi).
  Histogram * h_n1_phiY;
  //! Sum of pT vs (y, phi).
  Histogram * h_spt_phiY;
  //! dN/(dy dpT).
  Histogram * h_n1_ptY;

  //! Distribution of the PDG codes.
  Histogram * h_pdgId;

  ClassDef(ParticleSingleHistos,0)

};

} // namespace CAP

#endif /* CAP__ParticleSingleHistos  */



