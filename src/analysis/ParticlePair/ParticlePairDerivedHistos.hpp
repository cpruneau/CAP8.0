/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ParticlePairDerivedHistos
#define CAP__ParticlePairDerivedHistos
#include "MathBasicFunctions.hpp"
#include "ParticleSingleHistos.hpp"
#include "ParticleSingleDerivedHistos.hpp"
#include "ParticlePairHistos.hpp"
#include "HistogramFolder.hpp"

using CAP::MATH::absolute;

namespace CAP
{


//!
//! @brief Two-particle correlation functions computed at the end of the run from the pair histograms and the derived
//! single-particle histograms of the two particle filters.
//!
//! For each pair of variables (pT1 pT2, phi1 phi2, eta1 eta2, y1 y2, Delta eta Delta phi, Delta y Delta phi): the
//! product of single densities n1n1, the cumulant C2, the normalized cumulants R2 and A2 (C2N), and, with p2_fill, the
//! transverse momentum correlators P2 and G2. The _shft histograms have the Delta phi range shifted (e.g., centered on
//! the near side).
//!
class ParticlePairDerivedHistos : public  ParticlePairHistos
{
public:

  //! @brief Default constructor.
  ParticlePairDerivedHistos();
  //! @brief Copy constructor. @param source group to copy
  ParticlePairDerivedHistos(const ParticlePairDerivedHistos & source);
  //! @brief Copy assignment. @param rhs group to copy @return this group
  ParticlePairDerivedHistos & operator=(const ParticlePairDerivedHistos & rhs);
  //! @brief Destructor.
  virtual ~ParticlePairDerivedHistos() {}
  //! @brief Read the binning and fill options. @param taskName name of the task @param objectType object type
  //! @param configuration configuration @param index index
  virtual void configure(const String & taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Load the histograms from a file. @param inputFile file
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Create the derived histograms.
  virtual void create();
  //! @brief Compute the correlation functions.
  //! @param part1DerivedHistos derived single histograms of particle 1 @param part2DerivedHistos same for particle 2
  //! @param pairHistos pair histograms
  virtual void calculateDerivedHistograms(ParticleSingleDerivedHistos & part1DerivedHistos,
                                          ParticleSingleDerivedHistos & part2DerivedHistos,
                                          ParticlePairHistos          & pairHistos);
  //! @brief Acceptance factor of each Delta y bin: 1 - |Delta y|/(2 y_max).
  //! @param h_DyDphi histogram vs (Delta y, Delta phi) @param omegaFactor factors (set)
  void calculateOmegaFactor(Histogram * h_DyDphi, vector<double> & omegaFactor);
  //! @brief Average over ybar of a histogram vs (Delta y, Delta phi), using the acceptance factors.
  //! @param h_DyDphi histogram @param h_DyDphi_Avg average (set) @param omegaFactor acceptance factors
  void calculateAverageYbar(Histogram * h_DyDphi, Histogram * h_DyDphi_Avg, vector<double> & omegaFactor);
  friend class ParticlePairBfHistos;

protected:

  //! @brief Copy the content of another group. @param source group to copy
  virtual void cloneD(const ParticlePairDerivedHistos & source);

  Histogram* h_n1n1_ptpt;  ///< product of the single-particle densities n1(1) n1(2) vs (pT1, pT2)
  Histogram* h_C2_ptpt;  ///< pair correlation (cumulant) C2 = n2 - n1n1 vs (pT1, pT2)
  Histogram* h_C2N_ptpt;  ///< normalized cumulant A2 vs (pT1, pT2)
  Histogram* h_R2_ptpt;  ///< normalized cumulant R2 = n2/(n1n1) - 1 vs (pT1, pT2)

  Histogram* h_n1n1_phiphi;  ///< product of the single-particle densities n1(1) n1(2) vs (phi1, phi2)
  Histogram* h_C2_phiphi;  ///< pair correlation (cumulant) C2 = n2 - n1n1 vs (phi1, phi2)
  Histogram* h_C2N_phiphi;  ///< normalized cumulant A2 vs (phi1, phi2)
  Histogram* h_R2_phiphi;  ///< normalized cumulant R2 = n2/(n1n1) - 1 vs (phi1, phi2)
  Histogram* h_P2_phiphi;  ///< transverse momentum correlator P2 vs (phi1, phi2)
  Histogram* h_G2_phiphi;  ///< transverse momentum correlator G2 vs (phi1, phi2)
  Histogram* h_DptDpt_phiphi;  ///< pair density weighted by pT1 pT2 vs (phi1, phi2)
  Histogram* h_pt1pt1_phiphi;  ///< product of the single-particle pT densities vs (phi1, phi2)

  Histogram* h_n1n1_etaeta;  ///< product of the single-particle densities n1(1) n1(2) vs (eta1, eta2)
  Histogram* h_C2_etaeta;  ///< pair correlation (cumulant) C2 = n2 - n1n1 vs (eta1, eta2)
  Histogram* h_C2N_etaeta;  ///< normalized cumulant A2 vs (eta1, eta2)
  Histogram* h_R2_etaeta;  ///< normalized cumulant R2 = n2/(n1n1) - 1 vs (eta1, eta2)
  Histogram* h_G2_etaeta;  ///< transverse momentum correlator G2 vs (eta1, eta2)
  Histogram* h_P2_etaeta;  ///< transverse momentum correlator P2 vs (eta1, eta2)
  Histogram* h_DptDpt_etaeta;  ///< pair density weighted by pT1 pT2 vs (eta1, eta2)
  Histogram* h_pt1pt1_etaeta;  ///< product of the single-particle pT densities vs (eta1, eta2)

  Histogram* h_n1n1_yY;  ///< product of the single-particle densities n1(1) n1(2) vs (y1, y2)
  Histogram* h_C2_yY;  ///< pair correlation (cumulant) C2 = n2 - n1n1 vs (y1, y2)
  Histogram* h_C2N_yY;  ///< normalized cumulant A2 vs (y1, y2)
  Histogram* h_R2_yY;  ///< normalized cumulant R2 = n2/(n1n1) - 1 vs (y1, y2)
  Histogram* h_G2_yY;  ///< transverse momentum correlator G2 vs (y1, y2)
  Histogram* h_P2_yY;  ///< transverse momentum correlator P2 vs (y1, y2)
  Histogram* h_DptDpt_yY;  ///< pair density weighted by pT1 pT2 vs (y1, y2)
  Histogram* h_pt1pt1_yY;  ///< product of the single-particle pT densities vs (y1, y2)

  Histogram* h_n1n1_DetaDphi;  ///< product of the single-particle densities n1(1) n1(2) vs (Delta eta, Delta phi)
  Histogram* h_C2_DetaDphi;  ///< pair correlation (cumulant) C2 = n2 - n1n1 vs (Delta eta, Delta phi)
  Histogram* h_C2N_DetaDphi;  ///< normalized cumulant A2 vs (Delta eta, Delta phi)
  Histogram* h_C2O_DetaDphi;  ///< cumulant D2 (A2 averaged over the mean (pseudo)rapidity, see calculateAverageYbar()) vs (Delta eta, Delta phi)
  Histogram* h_R2_DetaDphi;  ///< normalized cumulant R2 = n2/(n1n1) - 1 vs (Delta eta, Delta phi)
  Histogram* h_P2_DetaDphi;  ///< transverse momentum correlator P2 vs (Delta eta, Delta phi)
  Histogram* h_G2_DetaDphi;  ///< transverse momentum correlator G2 vs (Delta eta, Delta phi)
  Histogram* h_DptDpt_DetaDphi;  ///< pair density weighted by pT1 pT2 vs (Delta eta, Delta phi)
  Histogram* h_pt1pt1_DetaDphi;  ///< product of the single-particle pT densities vs (Delta eta, Delta phi)

  Histogram* h_n1n1_DetaDphi_shft;  ///< product of the single-particle densities n1(1) n1(2) vs (Delta eta, Delta phi), Delta phi range shifted
  Histogram* h_C2_DetaDphi_shft;  ///< pair correlation (cumulant) C2 = n2 - n1n1 vs (Delta eta, Delta phi), Delta phi range shifted
  Histogram* h_C2N_DetaDphi_shft;  ///< normalized cumulant A2 vs (Delta eta, Delta phi), Delta phi range shifted
  Histogram* h_C2O_DetaDphi_shft;  ///< cumulant D2 (A2 averaged over the mean (pseudo)rapidity, see calculateAverageYbar()) vs (Delta eta, Delta phi), Delta phi range shifted
  Histogram* h_R2_DetaDphi_shft;  ///< normalized cumulant R2 = n2/(n1n1) - 1 vs (Delta eta, Delta phi), Delta phi range shifted
  Histogram* h_P2_DetaDphi_shft;  ///< transverse momentum correlator P2 vs (Delta eta, Delta phi), Delta phi range shifted
  Histogram* h_G2_DetaDphi_shft;  ///< transverse momentum correlator G2 vs (Delta eta, Delta phi), Delta phi range shifted
  Histogram* h_DptDpt_DetaDphi_shft;  ///< pair density weighted by pT1 pT2 vs (Delta eta, Delta phi), Delta phi range shifted

  Histogram* h_n1n1_DyDphi;  ///< product of the single-particle densities n1(1) n1(2) vs (Delta y, Delta phi)
  Histogram* h_C2_DyDphi;  ///< pair correlation (cumulant) C2 = n2 - n1n1 vs (Delta y, Delta phi)
  Histogram* h_C2N_DyDphi;  ///< normalized cumulant A2 vs (Delta y, Delta phi)
  Histogram* h_C2O_DyDphi;  ///< cumulant D2 (A2 averaged over the mean (pseudo)rapidity, see calculateAverageYbar()) vs (Delta y, Delta phi)
  Histogram* h_R2_DyDphi;  ///< normalized cumulant R2 = n2/(n1n1) - 1 vs (Delta y, Delta phi)
  Histogram* h_P2_DyDphi;  ///< transverse momentum correlator P2 vs (Delta y, Delta phi)
  Histogram* h_G2_DyDphi;  ///< transverse momentum correlator G2 vs (Delta y, Delta phi)
  Histogram* h_DptDpt_DyDphi;  ///< pair density weighted by pT1 pT2 vs (Delta y, Delta phi)
  Histogram* h_pt1pt1_DyDphi;  ///< product of the single-particle pT densities vs (Delta y, Delta phi)

  Histogram* h_n1n1_DyDphi_shft;  ///< product of the single-particle densities n1(1) n1(2) vs (Delta y, Delta phi), Delta phi range shifted
  Histogram* h_C2_DyDphi_shft;  ///< pair correlation (cumulant) C2 = n2 - n1n1 vs (Delta y, Delta phi), Delta phi range shifted
  Histogram* h_C2N_DyDphi_shft;  ///< normalized cumulant A2 vs (Delta y, Delta phi), Delta phi range shifted
  Histogram* h_C2O_DyDphi_shft;  ///< cumulant D2 (A2 averaged over the mean (pseudo)rapidity, see calculateAverageYbar()) vs (Delta y, Delta phi), Delta phi range shifted
  Histogram* h_R2_DyDphi_shft;  ///< normalized cumulant R2 = n2/(n1n1) - 1 vs (Delta y, Delta phi), Delta phi range shifted
  Histogram* h_P2_DyDphi_shft;  ///< transverse momentum correlator P2 vs (Delta y, Delta phi), Delta phi range shifted
  Histogram* h_G2_DyDphi_shft;  ///< transverse momentum correlator G2 vs (Delta y, Delta phi), Delta phi range shifted
  Histogram* h_DptDpt_DyDphi_shft;  ///< pair density weighted by pT1 pT2 vs (Delta y, Delta phi), Delta phi range shifted
  

};

} // namespace CAP

#endif /* CAP__ParticlePairDerivedHistos */
