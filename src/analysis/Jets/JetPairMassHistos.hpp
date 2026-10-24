/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani, Akash Raj
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau, Akash Raj,  Nov 2024
 *
 * *********************************************************************/
#ifndef CAP__JetPairMassHistos
#define CAP__JetPairMassHistos
#include "HistogramFolder.hpp"
#include "fastjet/ClusterSequence.hh"
#include "MathConstants.hpp"
#include "ParticleTypeList.hpp"

using namespace fastjet;

namespace CAP
{

//!
//! @brief Histograms of jet pairs: (pT1, pT2), (phi1, phi2), (eta1, eta2), (Delta eta, Delta phi) and di-jet mass.
//! @warning setParticleTypeList() assigns to the type name ParticleTypeList instead of the member particleTypeList.
//!
class JetPairMassHistos : public HistogramFolder
{
public:

  friend class JetPairMassHistosDerived;


  //! @brief Default constructor.
  JetPairMassHistos();
  //! @brief Copy constructor. @param source object to copy
  JetPairMassHistos(const JetPairMassHistos & source);
  //! @brief Copy assignment. @param rhs object to copy @return this object
  JetPairMassHistos & operator=(const JetPairMassHistos & rhs);
  //! @brief Destructor.
  ~JetPairMassHistos(){}
  //! @brief Configure the object from keys \<taskName\>:\<objectType\>:... (binning, options).
  //! @param taskName name of the task @param objectType object type @param configuration configuration @param index index
  virtual void configure(const String & taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Create the histograms.
  virtual void create();
  //! @brief Load the histograms from a file. @param inputFile file
  virtual void loadFrom(const Folder & inputFile);

  //! @brief Fill the histograms with a jet pair. @param J1 first jet @param J2 second jet
  virtual void fill(PseudoJet& J1, PseudoJet& J2);
  //! @brief Scale the histograms. @param scale factor
  virtual void scaleHistograms(double scale);
  //! @brief Set the particle types. @param selectedParticleTypeList types
  virtual void setParticleTypeList(ParticleTypeList * selectedParticleTypeList)
  {
  ParticleTypeList = selectedParticleTypeList;
  }

protected:

  //! @brief Copy the content of another group. @param source group to copy
  virtual void cloneB(const JetPairMassHistos & source);

  //! Particle types (not owned).
  ParticleTypeList * particleTypeList;
  
  Histogram *h_jetpair_n2_ptpt;  ///< jet1 pt vs. jet2 pt
  Histogram *h_jetpair_n2_phiphi;  ///< jet1 phi vs. jet2 phi
  Histogram *h_jetpair_n2_etaeta;  ///< jet1 eta vs. jet2 eta
  Histogram *h_jetpair_n2_DetaDphi;  ///< jet1, jet2 Deta, Dphi
  Histogram *h_jetpair_n2_mass;  ///< di-jet mass.

  int pt_nbins;  ///< number of bins of the pT axis
  int phi_nbins;  ///< number of bins of the azimuth axis
  int eta_nbins;  ///< number of bins of the pseudorapidity axis
  int Deta_nbins;  ///< number of bins of the Delta eta axis
  int Dphi_nbins;  ///< number of bins of the Delta phi axis
  int mass_nbins;  ///< number of bins of the mass axis

  double pt_min;  ///< lower edge of the pT axis
  double pt_max;  ///< upper edge of the pT axis
  double phi_min;  ///< lower edge of the azimuth axis
  double phi_max;  ///< upper edge of the azimuth axis
  double eta_min;  ///< lower edge of the pseudorapidity axis
  double eta_max;  ///< upper edge of the pseudorapidity axis
  double Deta_min;  ///< lower edge of the Delta eta axis
  double Deta_max;  ///< upper edge of the Delta eta axis
  double Dphi_min;  ///< lower edge of the Delta phi axis
  double Dphi_max;  ///< upper edge of the Delta phi axis

  double mass_min;  ///< lower edge of the mass axis
  double mass_max;  ///< upper edge of the mass axis


};

} //  namespace CAP

#endif
