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
#ifndef CAP__ParticlePairBfHistos
#define CAP__ParticlePairBfHistos
#include "ParticlePairDerivedHistos.hpp"
#include "HistogramFolder.hpp"

namespace CAP
{

//!
//! @brief Two-particle balance functions and charge-dependent/independent combinations, computed from the correlation
//! functions of the four combinations of a particle species and its antiparticle (1 2, 1 2bar, 1bar 2, 1bar 2bar).
//!
//! For each pair of variables: C2_CD and C2_CI (charge-dependent and -independent combinations of C2), and the balance
//! functions B(1|2), B(2|1) and their sum, from A2 (C2N_) and from R2 (R2_).
//!
class ParticlePairBfHistos : public  ParticlePairDerivedHistos
{
public:

  //! @brief Default constructor.
  ParticlePairBfHistos();
  //! @brief Copy constructor. @param source group to copy
  ParticlePairBfHistos(const ParticlePairBfHistos & source);
  //! @brief Copy assignment. @param rhs group to copy @return this group
  ParticlePairBfHistos & operator=(const ParticlePairBfHistos & rhs);
  //! @brief Destructor.
  virtual ~ParticlePairBfHistos() {}
  //! @brief Read the binning and fill options. @param taskName name of the task @param objectType object type
  //! @param configuration configuration @param index index
  virtual void configure(const String & taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Create the balance function histograms.
  void create();
  //! @brief Load the histograms from a file. @param inputFile file
  void loadFrom(const Folder & inputFile);
  
  //! @brief Compute the balance functions and combinations from the correlation functions of the four combinations.
  //! @param pair12B pairs (1, 2bar) @param pair1B2B pairs (1bar, 2bar) @param pair1B2 pairs (1bar, 2) @param pair12 pairs (1, 2)
  virtual void calculateBfHistograms(ParticlePairDerivedHistos & pair12B,
                                     ParticlePairDerivedHistos & pair1B2B,
                                     ParticlePairDerivedHistos & pair1B2,
                                     ParticlePairDerivedHistos & pair12);

protected:

  //! @brief Copy the content of another group. @param source group to copy
  virtual void cloneBF(const ParticlePairBfHistos & source);

  Histogram* h_C2_CD_ptpt;  ///< charge-dependent combination of C2 vs (pT1, pT2)
  Histogram* h_C2_CI_ptpt;  ///< charge-independent combination of C2 vs (pT1, pT2)
  Histogram* h_C2N_B12_ptpt;  ///< balance function B(1|2) from A2 vs (pT1, pT2)
  Histogram* h_C2N_B21_ptpt;  ///< balance function B(2|1) from A2 vs (pT1, pT2)
  Histogram* h_C2N_Bs_ptpt;  ///< sum of the balance functions from A2 vs (pT1, pT2)
  Histogram* h_R2_B12_ptpt;  ///< balance function B(1|2) from R2 vs (pT1, pT2)
  Histogram* h_R2_B21_ptpt;  ///< balance function B(2|1) from R2 vs (pT1, pT2)
  Histogram* h_R2_Bs_ptpt;  ///< sum of the balance functions from R2 vs (pT1, pT2)

  Histogram* h_C2_CD_phiphi;  ///< charge-dependent combination of C2 vs (phi1, phi2)
  Histogram* h_C2_CI_phiphi;  ///< charge-independent combination of C2 vs (phi1, phi2)
  Histogram* h_C2N_B12_phiphi;  ///< balance function B(1|2) from A2 vs (phi1, phi2)
  Histogram* h_C2N_B21_phiphi;  ///< balance function B(2|1) from A2 vs (phi1, phi2)
  Histogram* h_C2N_Bs_phiphi;  ///< sum of the balance functions from A2 vs (phi1, phi2)
  Histogram* h_R2_B12_phiphi;  ///< balance function B(1|2) from R2 vs (phi1, phi2)
  Histogram* h_R2_B21_phiphi;  ///< balance function B(2|1) from R2 vs (phi1, phi2)
  Histogram* h_R2_Bs_phiphi;  ///< sum of the balance functions from R2 vs (phi1, phi2)

  Histogram* h_C2_CD_etaeta;  ///< charge-dependent combination of C2 vs (eta1, eta2)
  Histogram* h_C2_CI_etaeta;  ///< charge-independent combination of C2 vs (eta1, eta2)
  Histogram* h_C2N_B12_etaeta;  ///< balance function B(1|2) from A2 vs (eta1, eta2)
  Histogram* h_C2N_B21_etaeta;  ///< balance function B(2|1) from A2 vs (eta1, eta2)
  Histogram* h_C2N_Bs_etaeta;  ///< sum of the balance functions from A2 vs (eta1, eta2)
  Histogram* h_R2_B12_etaeta;  ///< balance function B(1|2) from R2 vs (eta1, eta2)
  Histogram* h_R2_B21_etaeta;  ///< balance function B(2|1) from R2 vs (eta1, eta2)
  Histogram* h_R2_Bs_etaeta;  ///< sum of the balance functions from R2 vs (eta1, eta2)

  Histogram* h_C2_CD_yY;  ///< charge-dependent combination of C2 vs (y1, y2)
  Histogram* h_C2_CI_yY;  ///< charge-independent combination of C2 vs (y1, y2)
  Histogram* h_C2N_B12_yY;  ///< balance function B(1|2) from A2 vs (y1, y2)
  Histogram* h_C2N_B21_yY;  ///< balance function B(2|1) from A2 vs (y1, y2)
  Histogram* h_C2N_Bs_yY;  ///< sum of the balance functions from A2 vs (y1, y2)
  Histogram* h_R2_B12_yY;  ///< balance function B(1|2) from R2 vs (y1, y2)
  Histogram* h_R2_B21_yY;  ///< balance function B(2|1) from R2 vs (y1, y2)
  Histogram* h_R2_Bs_yY;  ///< sum of the balance functions from R2 vs (y1, y2)

  Histogram* h_C2_CD_DetaDphi;  ///< charge-dependent combination of C2 vs (Delta eta, Delta phi)
  Histogram* h_C2_CI_DetaDphi;  ///< charge-independent combination of C2 vs (Delta eta, Delta phi)
  Histogram* h_C2N_B12_DetaDphi;  ///< balance function B(1|2) from A2 vs (Delta eta, Delta phi)
  Histogram* h_C2N_B21_DetaDphi;  ///< balance function B(2|1) from A2 vs (Delta eta, Delta phi)
  Histogram* h_C2N_Bs_DetaDphi;  ///< sum of the balance functions from A2 vs (Delta eta, Delta phi)
  Histogram* h_R2_B12_DetaDphi;  ///< balance function B(1|2) from R2 vs (Delta eta, Delta phi)
  Histogram* h_R2_B21_DetaDphi;  ///< balance function B(2|1) from R2 vs (Delta eta, Delta phi)
  Histogram* h_R2_Bs_DetaDphi;  ///< sum of the balance functions from R2 vs (Delta eta, Delta phi)

  Histogram* h_C2_CD_DyDphi;  ///< charge-dependent combination of C2 vs (Delta y, Delta phi)
  Histogram* h_C2_CI_DyDphi;  ///< charge-independent combination of C2 vs (Delta y, Delta phi)
  Histogram* h_C2N_B12_DyDphi;  ///< balance function B(1|2) from A2 vs (Delta y, Delta phi)
  Histogram* h_C2N_B21_DyDphi;  ///< balance function B(2|1) from A2 vs (Delta y, Delta phi)
  Histogram* h_C2N_Bs_DyDphi;  ///< sum of the balance functions from A2 vs (Delta y, Delta phi)
  Histogram* h_R2_B12_DyDphi;  ///< balance function B(1|2) from R2 vs (Delta y, Delta phi)
  Histogram* h_R2_B21_DyDphi;  ///< balance function B(2|1) from R2 vs (Delta y, Delta phi)
  Histogram* h_R2_Bs_DyDphi;  ///< sum of the balance functions from R2 vs (Delta y, Delta phi)

  ClassDef(ParticlePairBfHistos,1)

};

} // namespace CAP

#endif /* CAP__ParticlePairBfHistos */
