/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani, Akash Raj
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau, Akash Raj,  Nov 2024
 *
 * *********************************************************************/
#ifndef CAP__JetSingleHistos
#define CAP__JetSingleHistos
#include "HistogramFolder.hpp"
#include "fastjet/ClusterSequence.hh"
#include "MathConstants.hpp"
#include "ParticleTypeList.hpp"

using namespace fastjet;

namespace CAP
{

//!
//! @brief Single-particle histograms of the jet constituents (all, positive, negative): pT, azimuth, pseudorapidity,
//! angle to the jet axis, momentum perpendicular to the axis (jt), momentum fraction z.
//!
class JetSingleHistos : public HistogramFolder
{
public:

  friend class JetSingleHistosDerived;
  friend class JetPairHistosDerived;

	//! @brief Default constructor.
	JetSingleHistos();
  //! @brief Copy constructor. @param source object to copy
  JetSingleHistos(const JetSingleHistos & source);
  //! @brief Copy assignment. @param rhs object to copy @return this object
  JetSingleHistos & operator=(const JetSingleHistos & rhs);
	//! @brief Destructor.
	~JetSingleHistos(){}
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
  //! @brief Fill the histograms with the constituents of a jet. @param J jet
  virtual void fill(PseudoJet& J);

  protected:

  //! @brief Copy the content of another group. @param group group to copy
  virtual void  cloneB(const JetSingleHistos & group);

  //! Particle types used to find the charges of the constituents (not owned).
  ParticleTypeList * particleTypeList;

  Histogram *h_jet_n1_pt;  ///< density of the constituents, all charges, vs pT
  Histogram *h_jet_n1_phi;  ///< density of the constituents, all charges, vs azimuth
  Histogram *h_jet_n1_eta;  ///< density of the constituents, all charges, vs pseudorapidity
  Histogram *h_jet_n1_th;  ///< density of the constituents, all charges, vs angle to the jet axis
  Histogram *h_jet_n1_jt;  ///< density of the constituents, all charges, vs momentum perpendicular to the jet axis
  Histogram *h_jet_n1_z;  ///< density of the constituents, all charges, vs momentum fraction p/p_jet

  Histogram *h_jet_n1p_pt;  ///< density of the constituents, positive charges, vs pT
  Histogram *h_jet_n1p_phi;  ///< density of the constituents, positive charges, vs azimuth
  Histogram *h_jet_n1p_eta;  ///< density of the constituents, positive charges, vs pseudorapidity
  Histogram *h_jet_n1p_th;  ///< density of the constituents, positive charges, vs angle to the jet axis
  Histogram *h_jet_n1p_jt;  ///< density of the constituents, positive charges, vs momentum perpendicular to the jet axis
  Histogram *h_jet_n1p_z;  ///< density of the constituents, positive charges, vs momentum fraction p/p_jet

  Histogram *h_jet_n1m_pt;  ///< density of the constituents, negative charges, vs pT
  Histogram *h_jet_n1m_phi;  ///< density of the constituents, negative charges, vs azimuth
  Histogram *h_jet_n1m_eta;  ///< density of the constituents, negative charges, vs pseudorapidity
  Histogram *h_jet_n1m_th;  ///< density of the constituents, negative charges, vs angle to the jet axis
  Histogram *h_jet_n1m_jt;  ///< density of the constituents, negative charges, vs momentum perpendicular to the jet axis
  Histogram *h_jet_n1m_z;  ///< density of the constituents, negative charges, vs momentum fraction p/p_jet

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

} // namespace CAP
#endif // JetSingleHistos_HPP
