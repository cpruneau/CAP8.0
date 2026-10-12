/* **********************************************************************
 * Copyright (C) 2019-2024, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ParticlePerformanceHistos
#define CAP__ParticlePerformanceHistos
#include "HistogramFolder.hpp"
#include "Particle.hpp"
#include "Configuration.hpp"

namespace CAP
{


//!
//! @brief Detector performance histograms: differences between reconstructed and generated pT, eta, phi and rapidity,
//! and their averages vs pT and eta.
//!
class ParticlePerformanceHistos : public HistogramFolder
{
public:

  //! @brief Default constructor.
  //! @brief Default constructor.
  ParticlePerformanceHistos();
  //! @brief Destructor.
  virtual ~ParticlePerformanceHistos() {}
  //! @brief Configure the object from keys \<taskName\>:\<objectType\>:... (binning, options).
  //! @param taskName name of the task @param objectType object type @param configuration configuration @param index index
  virtual void configure(const String& taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Create the histograms.
  virtual void create();
  //! @brief Load the histograms from a file. @param inputFile file
  virtual void loadFrom(CapFile & inputFile);
  //! @brief Fill the histograms with a particle (compared with its truth particle). @param particle particle @param weight weight
  virtual void fill(Particle & particle, double weight);

protected:

//! Distribution of pT(reco) - pT(gen).
Histogram * h_n1_dpt;
//! Distribution of eta(reco) - eta(gen).
Histogram * h_n1_deta;
//! Distribution of phi(reco) - phi(gen).
Histogram * h_n1_dphi;
//! Distribution of y(reco) - y(gen).
Histogram * h_n1_dy;

//! Average pT difference vs pT.
Profile * p_n1_dptVsPt;
//! Average pT difference vs eta.
Profile * p_n1_dptVsEta;
//! Average eta difference vs pT.
Profile * p_n1_detaVsPt;
//! Average eta difference vs eta.
Profile * p_n1_detaVsEta;
//! Average rapidity difference vs pT.
Profile * p_n1_dyVsPt;
//! Average rapidity difference vs eta.
Profile * p_n1_dyVsEta;
//! Average phi difference vs pT.
Profile * p_n1_dphiVsPt;
//! Average phi difference vs eta.
Profile * p_n1_dphiVsEta;

int pt_nbins;  ///< number of bins of the pT axis
double pt_min;  ///< lower edge of the pT axis
double pt_max;  ///< upper edge of the pT axis

int dpt_nbins;  ///< number of bins of the pT difference axis
double dpt_min;  ///< lower edge of the pT difference axis
double dpt_max;  ///< upper edge of the pT difference axis

int Dphi_nbins;  ///< number of bins of the Delta phi axis
double Dphi_min;  ///< lower edge of the Delta phi axis
double Dphi_max;  ///< upper edge of the Delta phi axis

int eta_nbins;  ///< number of bins of the pseudorapidity axis
double eta_min;  ///< lower edge of the pseudorapidity axis
double eta_max;  ///< upper edge of the pseudorapidity axis

int Deta_nbins;  ///< number of bins of the Delta eta axis
double Deta_min;  ///< lower edge of the Delta eta axis
double Deta_max;  ///< upper edge of the Delta eta axis

int Drapidity_nbins;  ///< number of bins of the Delta y axis
double Drapidity_min;  ///< lower edge of the Delta y axis
double Drapidity_max;  ///< upper edge of the Delta y axis

ClassDef(ParticlePerformanceHistos,0)

};

} // namespace CAP

#endif /* CAP__ParticlePerformanceHistos*/



