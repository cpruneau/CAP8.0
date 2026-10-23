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
#ifndef CAP__JetPairHistosDerived
#define CAP__JetPairHistosDerived
#include "Profile.hpp"
#include "JetSingleHistos.hpp"
#include "JetPairHistos.hpp"
#include "MathConstants.hpp"

//using namespace fastjet;

namespace CAP
{


//!
//! @brief Correlations derived from the constituent pair and single histograms: associated pair correlations A2
//! (++, -+, +-, --) and balance functions B2 (+-, -+, sum).
//!
class JetPairHistosDerived : public JetPairHistos
{
public:
  //! @brief Default constructor.
  JetPairHistosDerived();
  //! @brief Copy constructor. @param source object to copy
  JetPairHistosDerived(const JetPairHistosDerived & source);
  //! @brief Copy assignment. @param rhs object to copy @return this object
  JetPairHistosDerived & operator=(const JetPairHistosDerived & rhs);
  //! @brief Destructor.
  ~JetPairHistosDerived(){}
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
  //! @brief Compute the derived histograms. @param singleHistos constituent histograms @param pairHistos pair histograms
  virtual void calculateDerivedHistograms(const JetSingleHistos & singleHistos, const JetPairHistos & pairHistos);


protected:

  //! @brief Copy the content of another group. @param source group to copy
  virtual void cloneD(const JetPairHistosDerived & source);

  Histogram *  h_jet_a2pp_ptpt;  ///< associated pair correlation A2, (+,+) charges, vs (pT1, pT2)
  Histogram *  h_jet_a2pp_phiphi;  ///< associated pair correlation A2, (+,+) charges, vs (phi1, phi2)
  Histogram *  h_jet_a2pp_etaeta;  ///< associated pair correlation A2, (+,+) charges, vs (eta1, eta2)
  Histogram *  h_jet_a2pp_thth;  ///< associated pair correlation A2, (+,+) charges, vs (theta1, theta2)
  Histogram *  h_jet_a2pp_jtjt;  ///< associated pair correlation A2, (+,+) charges, vs (jt1, jt2)
  Histogram *  h_jet_a2pp_zz;  ///< associated pair correlation A2, (+,+) charges, vs (z1, z2)

  Histogram *  h_jet_a2mp_ptpt;  ///< associated pair correlation A2, (-,+) charges, vs (pT1, pT2)
  Histogram *  h_jet_a2mp_phiphi;  ///< associated pair correlation A2, (-,+) charges, vs (phi1, phi2)
  Histogram *  h_jet_a2mp_etaeta;  ///< associated pair correlation A2, (-,+) charges, vs (eta1, eta2)
  Histogram *  h_jet_a2mp_thth;  ///< associated pair correlation A2, (-,+) charges, vs (theta1, theta2)
  Histogram *  h_jet_a2mp_jtjt;  ///< associated pair correlation A2, (-,+) charges, vs (jt1, jt2)
  Histogram *  h_jet_a2mp_zz;  ///< associated pair correlation A2, (-,+) charges, vs (z1, z2)

  Histogram *  h_jet_a2pm_ptpt;  ///< associated pair correlation A2, (+,-) charges, vs (pT1, pT2)
  Histogram *  h_jet_a2pm_phiphi;  ///< associated pair correlation A2, (+,-) charges, vs (phi1, phi2)
  Histogram *  h_jet_a2pm_etaeta;  ///< associated pair correlation A2, (+,-) charges, vs (eta1, eta2)
  Histogram *  h_jet_a2pm_thth;  ///< associated pair correlation A2, (+,-) charges, vs (theta1, theta2)
  Histogram *  h_jet_a2pm_jtjt;  ///< associated pair correlation A2, (+,-) charges, vs (jt1, jt2)
  Histogram *  h_jet_a2pm_zz;  ///< associated pair correlation A2, (+,-) charges, vs (z1, z2)

  Histogram *  h_jet_a2mm_ptpt;  ///< associated pair correlation A2, (-,-) charges, vs (pT1, pT2)
  Histogram *  h_jet_a2mm_phiphi;  ///< associated pair correlation A2, (-,-) charges, vs (phi1, phi2)
  Histogram *  h_jet_a2mm_etaeta;  ///< associated pair correlation A2, (-,-) charges, vs (eta1, eta2)
  Histogram *  h_jet_a2mm_thth;  ///< associated pair correlation A2, (-,-) charges, vs (theta1, theta2)
  Histogram *  h_jet_a2mm_jtjt;  ///< associated pair correlation A2, (-,-) charges, vs (jt1, jt2)
  Histogram *  h_jet_a2mm_zz;  ///< associated pair correlation A2, (-,-) charges, vs (z1, z2)

  Histogram *  h_jet_b2pm_ptpt;  ///< balance function B2, (+,-) charges, vs (pT1, pT2)
  Histogram *  h_jet_b2pm_phiphi;  ///< balance function B2, (+,-) charges, vs (phi1, phi2)
  Histogram *  h_jet_b2pm_etaeta;  ///< balance function B2, (+,-) charges, vs (eta1, eta2)
  Histogram *  h_jet_b2pm_thth;  ///< balance function B2, (+,-) charges, vs (theta1, theta2)
  Histogram *  h_jet_b2pm_jtjt;  ///< balance function B2, (+,-) charges, vs (jt1, jt2)
  Histogram *  h_jet_b2pm_zz;  ///< balance function B2, (+,-) charges, vs (z1, z2)

  Histogram *  h_jet_b2mp_ptpt;  ///< balance function B2, (-,+) charges, vs (pT1, pT2)
  Histogram *  h_jet_b2mp_phiphi;  ///< balance function B2, (-,+) charges, vs (phi1, phi2)
  Histogram *  h_jet_b2mp_etaeta;  ///< balance function B2, (-,+) charges, vs (eta1, eta2)
  Histogram *  h_jet_b2mp_thth;  ///< balance function B2, (-,+) charges, vs (theta1, theta2)
  Histogram *  h_jet_b2mp_jtjt;  ///< balance function B2, (-,+) charges, vs (jt1, jt2)
  Histogram *  h_jet_b2mp_zz;  ///< balance function B2, (-,+) charges, vs (z1, z2)

  Histogram *  h_jet_b2s_ptpt;  ///< balance function B2, summed charges, vs (pT1, pT2)
  Histogram *  h_jet_b2s_phiphi;  ///< balance function B2, summed charges, vs (phi1, phi2)
  Histogram *  h_jet_b2s_etaeta;  ///< balance function B2, summed charges, vs (eta1, eta2)
  Histogram *  h_jet_b2s_thth;  ///< balance function B2, summed charges, vs (theta1, theta2)
  Histogram *  h_jet_b2s_jtjt;  ///< balance function B2, summed charges, vs (jt1, jt2)
  Histogram *  h_jet_b2s_zz;  ///< balance function B2, summed charges, vs (z1, z2)

  ClassDef(JetPairHistosDerived,0)

};

} //  namespace CAP

#endif
