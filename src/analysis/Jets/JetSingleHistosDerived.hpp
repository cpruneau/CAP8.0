/* **********************************************************************
 * Copyright (C) 2019-2024, Claude Pruneau, Akash Raj
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau, Akash Raj,  Nov 2024
 *
 * *********************************************************************/
#ifndef CAP__JetSingleHistosDerived
#define CAP__JetSingleHistosDerived
#include "Profile.hpp"
#include "JetSingleHistos.hpp"
#include "MathConstants.hpp"
#include "ClassDef.hpp"

using namespace fastjet;

namespace CAP
{

//!
//! @brief Quantities derived from the constituent histograms: difference and ratio of the positive and negative
//! constituent densities.
//!
class JetSingleHistosDerived : public JetSingleHistos
{
public:
  //! @brief Default constructor.
  JetSingleHistosDerived();
  //! @brief Copy constructor. @param group object to copy
  JetSingleHistosDerived(const JetSingleHistosDerived & group);
  //! @brief Copy assignment. @param group object to copy @return this object
  JetSingleHistosDerived & operator=(const JetSingleHistosDerived & group);
	//! @brief Destructor.
	~JetSingleHistosDerived(){}
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
  //! @brief Compute the derived histograms. @param jetSingleHistos constituent histograms
  virtual void calculateDerivedHistograms(const JetSingleHistos & jetSingleHistos);

protected:

  //! @brief Copy the content of another group. @param source group to copy
  virtual void cloneD(const JetSingleHistosDerived & source);

  Histogram * h_jet_n1d_pt;  ///< difference of the densities of the positive and negative constituents, vs pT
  Histogram * h_jet_n1d_phi;  ///< difference of the densities of the positive and negative constituents, vs azimuth
  Histogram * h_jet_n1d_eta;  ///< difference of the densities of the positive and negative constituents, vs pseudorapidity
  Histogram * h_jet_n1d_th;  ///< difference of the densities of the positive and negative constituents, vs angle to the jet axis
  Histogram * h_jet_n1d_jt;  ///< difference of the densities of the positive and negative constituents, vs momentum perpendicular to the jet axis
  Histogram * h_jet_n1d_z;  ///< difference of the densities of the positive and negative constituents, vs momentum fraction p/p_jet

  Histogram * h_jet_n1r_pt;  ///< ratio of the densities of the positive and negative constituents, vs pT
  Histogram * h_jet_n1r_phi;  ///< ratio of the densities of the positive and negative constituents, vs azimuth
  Histogram * h_jet_n1r_eta;  ///< ratio of the densities of the positive and negative constituents, vs pseudorapidity
  Histogram * h_jet_n1r_th;  ///< ratio of the densities of the positive and negative constituents, vs angle to the jet axis
  Histogram * h_jet_n1r_jt;  ///< ratio of the densities of the positive and negative constituents, vs momentum perpendicular to the jet axis
  Histogram * h_jet_n1r_z;  ///< ratio of the densities of the positive and negative constituents, vs momentum fraction p/p_jet

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

  ClassDef(JetSingleHistosDerived,0)

};

} // namespace CAP
#endif // JetSingleHistos_HPP
