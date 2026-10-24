/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani, Akash Raj
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau, Akash Raj,  Nov 2024
 *
 * *********************************************************************/
#ifndef CAP__JetPairHistos
#define CAP__JetPairHistos
#include "HistogramFolder.hpp"
#include "fastjet/ClusterSequence.hh"
#include "MathConstants.hpp"
#include "ParticleTypeList.hpp"

using namespace fastjet;

namespace CAP
{


//!
//! @brief Pair histograms of the jet constituents (all, ++, +-, --) vs (pT1, pT2), (phi1, phi2), (eta1, eta2),
//! (theta1, theta2), (jt1, jt2), (z1, z2).
//!
class JetPairHistos : public HistogramFolder
{
public:

  friend class JetPairHistosDerived;

  //! @brief Default constructor.
  JetPairHistos();
  //! @brief Copy constructor. @param source object to copy
  JetPairHistos(const JetPairHistos & source);
  //! @brief Copy assignment. @param rhs object to copy @return this object
  JetPairHistos & operator=(const JetPairHistos & rhs);
  //! @brief Destructor.
  ~JetPairHistos(){}
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
  //! @brief Fill the histograms with the constituent pairs of a jet. @param J jet
  virtual void fill(PseudoJet& J);
  //! @brief Scale the histograms. @param scale factor
  virtual void scaleHistograms(double scale);

protected:

  //! @brief Copy the content of another group. @param source group to copy
  virtual void cloneB(const JetPairHistos & source);

  //! Particle types used to find the charges of the constituents (not owned).
  ParticleTypeList * particleTypeList;
  
  Histogram *h_jet_n2_ptpt;  ///< pair density of the constituents, all charges, vs (pT1, pT2)
  Histogram *h_jet_n2_phiphi;  ///< pair density of the constituents, all charges, vs (phi1, phi2)
  Histogram *h_jet_n2_etaeta;  ///< pair density of the constituents, all charges, vs (eta1, eta2)
  Histogram *h_jet_n2_thth;  ///< pair density of the constituents, all charges, vs (theta1, theta2)
  Histogram *h_jet_n2_jtjt;  ///< pair density of the constituents, all charges, vs (jt1, jt2)
  Histogram *h_jet_n2_zz;  ///< pair density of the constituents, all charges, vs (z1, z2)

  Histogram *h_jet_n2pp_ptpt;  ///< pair density of the constituents, (+,+) charges, vs (pT1, pT2)
  Histogram *h_jet_n2pp_phiphi;  ///< pair density of the constituents, (+,+) charges, vs (phi1, phi2)
  Histogram *h_jet_n2pp_etaeta;  ///< pair density of the constituents, (+,+) charges, vs (eta1, eta2)
  Histogram *h_jet_n2pp_thth;  ///< pair density of the constituents, (+,+) charges, vs (theta1, theta2)
  Histogram *h_jet_n2pp_jtjt;  ///< pair density of the constituents, (+,+) charges, vs (jt1, jt2)
  Histogram *h_jet_n2pp_zz;  ///< pair density of the constituents, (+,+) charges, vs (z1, z2)

  Histogram *h_jet_n2pm_ptpt;  ///< pair density of the constituents, (+,-) charges, vs (pT1, pT2)
  Histogram *h_jet_n2pm_phiphi;  ///< pair density of the constituents, (+,-) charges, vs (phi1, phi2)
  Histogram *h_jet_n2pm_etaeta;  ///< pair density of the constituents, (+,-) charges, vs (eta1, eta2)
  Histogram *h_jet_n2pm_thth;  ///< pair density of the constituents, (+,-) charges, vs (theta1, theta2)
  Histogram *h_jet_n2pm_jtjt;  ///< pair density of the constituents, (+,-) charges, vs (jt1, jt2)
  Histogram *h_jet_n2pm_zz;  ///< pair density of the constituents, (+,-) charges, vs (z1, z2)

  Histogram *h_jet_n2mm_ptpt;  ///< pair density of the constituents, (-,-) charges, vs (pT1, pT2)
  Histogram *h_jet_n2mm_phiphi;  ///< pair density of the constituents, (-,-) charges, vs (phi1, phi2)
  Histogram *h_jet_n2mm_etaeta;  ///< pair density of the constituents, (-,-) charges, vs (eta1, eta2)
  Histogram *h_jet_n2mm_thth;  ///< pair density of the constituents, (-,-) charges, vs (theta1, theta2)
  Histogram *h_jet_n2mm_jtjt;  ///< pair density of the constituents, (-,-) charges, vs (jt1, jt2)
  Histogram *h_jet_n2mm_zz;  ///< pair density of the constituents, (-,-) charges, vs (z1, z2)

  int pt_nbins;  ///< number of bins of the pT axis
  int phi_nbins;  ///< number of bins of the azimuth axis
  int eta_nbins;  ///< number of bins of the pseudorapidity axis
  int jt_nbins;  ///< number of bins of the jt axis
  int th_nbins;  ///< number of bins of the angle to the jet axis axis
  int z_nbins;  ///< number of bins of the z axis
  double pt_min;  ///< lower edge of the pT axis
  double pt_max;  ///< upper edge of the pT axis
  double phi_min;  ///< lower edge of the azimuth axis
  double phi_max;  ///< upper edge of the azimuth axis
  double eta_min;  ///< lower edge of the pseudorapidity axis
  double eta_max;  ///< upper edge of the pseudorapidity axis
  double jt_min;  ///< lower edge of the jt axis
  double jt_max;  ///< upper edge of the jt axis
  double th_min;  ///< lower edge of the angle to the jet axis axis
  double th_max;  ///< upper edge of the angle to the jet axis axis
  double z_min;  ///< lower edge of the z axis
  double z_max;  ///< upper edge of the z axis

};

} //  namespace CAP

#endif
