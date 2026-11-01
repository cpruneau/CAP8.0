/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file ParticlePairBFHistos.hpp
//! @brief ParticlePairBFHistos: balance functions and charge combinations of two species and their antiparticles.
#ifndef CAP__ParticlePairBFHistos
#define CAP__ParticlePairBFHistos
#include "HistogramFolder.hpp"
#include "PairBFCalculator.hpp"
#include "ParticleSingleHistos.hpp"
#include "ParticlePairHistos.hpp"

namespace CAP
{

//!
//! @brief Balance functions of a species a and a species b (and of their antiparticles ā and b̄), with the charge
//! independent and charge dependent combinations of R2, computed from the pair folders of the four combinations
//! (a,b), (a,b̄), (ā,b), (ā,b̄) and the single-particle folders of a, ā, b and b̄ (see PairBFCalculator).
//!
//! For each pair of variables v filled by the pair analysis (ptpt and phiphi always; etaeta and DetaDphi if eta_fill;
//! yY and DyDphi if rapidity_fill), with R^{xy} the normalized cumulant R2 of the pairs (x at x1, y at x2):
//!  - R2CI_v  = (R^{ab} + R^{ab̄} + R^{āb} + R^{āb̄})/4, the charge independent combination;
//!  - R2CD_v  = (R^{ab̄} + R^{āb} - R^{ab} - R^{āb̄})/2, the charge dependent combination;
//!  - B_a_bB_v = ρ1^{b̄}(x2) (R^{ab̄} - R^{āb̄}), the balance function B^{a|b̄}: the density of b̄ at x2 associated with an a
//!    at x1, minus that associated with an ā (equal to ρ2^{ab̄}/ρ1^{a} - ρ2^{āb̄}/ρ1^{ā});
//!  - B_aB_b_v = ρ1^{b}(x2) (R^{āb} - R^{ab}), the balance function B^{ā|b};
//!  - B_v = (B_a_bB_v + B_aB_b_v)/2.
//!
//! ρ1(x2) is the single-particle density of the associated species per event in each bin of x2 (dN/dpT, dN/dphi,
//! dN/deta, dN/dy times the bin width), so that the sum of B over the bins of x2 is the number of balancing partners.
//! For the histograms of (Delta eta, Delta phi) and (Delta y, Delta phi), ρ1 is the average density of the associated
//! species over the acceptance, per bin of the histogram: N̄ w_Delta w_phi / (A_eta A_phi), with N̄ the mean number of
//! associated particles per event in the acceptance A_eta x A_phi and w the bin widths (R2 corrects the pair density for
//! the acceptance); the Delta phi shifted versions (_shft) are also made. Statistical uncertainties are propagated
//! assuming the histograms combined are uncorrelated.
//!
//! The histograms have the binning of the R2 histograms of the pair folders (createFrom()); their names are short
//! (e.g. B_DetaDphi): the folder of the job names the species and the event filter (e.g. ALL_PiP_KP_BF).
//!
class ParticlePairBFHistos : public HistogramFolder
{
public:

  //! @brief Pairs of variables of the pair histograms.
  enum Variables { kPtPt, kPhiPhi, kEtaEta, kYY, kDetaDphi, kDyDphi, kNVariables };
  //! @brief Quantities computed for each pair of variables (see the class description).
  enum Quantities { kR2CI, kR2CD, kBabB, kBaBb, kB, kNQuantities };

  //! @brief Constructor. @param name name of the folder
  ParticlePairBFHistos(const String & name="ParticlePairBFHistos");
  //! @brief Copy constructor: copies the histograms. @param source folder to copy
  ParticlePairBFHistos(const ParticlePairBFHistos & source);
  //! @brief Assignment: replaces the histograms by copies of those of rhs. @param rhs folder to copy @return this folder
  ParticlePairBFHistos & operator=(const ParticlePairBFHistos & rhs);
  //! @brief Destructor.
  virtual ~ParticlePairBFHistos() {}
  //! @brief Name of the task computing these folders (see PairBFCalculator). @return name
  static const char * calculatorName() { return "ParticlePairBFCalculator"; }
  //! @brief Default name of the analyzer task whose folders the calculator reads. @return name
  static const char * defaultSourceTask() { return "ParticlePairAnalyzer"; }
  //! @brief Copy of this folder. @return new folder, owned by the caller
  virtual HistogramFolder * clone() const override { return new ParticlePairBFHistos(*this); }

  //! @brief Name of a pair of variables, as in the names of the histograms (ptpt, phiphi, etaeta, yY, DetaDphi, DyDphi).
  //! @param v pair of variables @return name
  static const char * variablesName(Variables v);
  //! @brief Name of a quantity, as in the names of the histograms (R2CI, R2CD, B_a_bB, B_aB_b, B). @param q quantity
  //! @return name
  static const char * quantityName(Quantities q);

  //!
  //! @brief Create the histograms (empty) with the binning of the R2 histograms of a pair folder: for each pair of
  //! variables whose R2 histogram exists in it (derived histograms computed). Existing histograms are deleted first.
  //! @param pair pair folder of the analysis
  //!
  void createFrom(const ParticlePairHistos & pair);
  //!
  //! @brief Compute the histograms from the pair folders of the four combinations of a, ā, b, b̄ and the single-particle
  //! folders of a, ā, b and b̄, all with their derived histograms computed (see ParticlePairHistos::calculateDerived()).
  //! @param ab pairs (a,b) @param abB pairs (a,b̄) @param aBb pairs (ā,b) @param aBbB pairs (ā,b̄)
  //! @param a singles of a (not used) @param aB singles of ā (not used) @param b singles of b @param bB singles of b̄
  //! @throws HistogramException if a histogram is missing or the binnings differ
  //!
  void calculate(const ParticlePairHistos & ab,  const ParticlePairHistos & abB,
                 const ParticlePairHistos & aBb, const ParticlePairHistos & aBbB,
                 const ParticleSingleHistos & a, const ParticleSingleHistos & aB,
                 const ParticleSingleHistos & b, const ParticleSingleHistos & bB);
  //! @brief Histogram of a quantity for a pair of variables. @param q quantity @param v variables
  //! @return histogram, or nullptr if not made
  Histogram * histogram(Quantities q, Variables v) const { return _h[v][q]; }
  //! @brief Histogram of a quantity for a pair of variables with the Delta phi range shifted (Delta variables only).
  //! @param q quantity @param v variables (kDetaDphi or kDyDphi) @return histogram, or nullptr if not made
  Histogram * shiftedHistogram(Quantities q, Variables v) const { return _hShifted[v][q]; }

  using HistogramFolder::histogram;

protected:

  //! @brief Set the pointers to the histograms from their names (after a copy).
  void relink();
  //! @brief Multiply each column x2 of h by the density in bin x2 of rho1, propagating the uncertainties.
  //! @param h histogram (rank 2) @param rho1 density (rank 1, as many bins as the second axis of h)
  static void multiplyByAssociatedDensity(Histogram & h, const Histogram & rho1);
  //! @brief R2 histogram of a pair folder for a pair of variables (nullptr if none). @param pair folder @param v variables
  static const Histogram * r2Of(const ParticlePairHistos & pair, Variables v);
  //! @brief R2 histogram of a pair folder with the Delta phi range shifted (nullptr if none). @param pair folder
  //! @param v variables (kDetaDphi or kDyDphi)
  static const Histogram * shiftedR2Of(const ParticlePairHistos & pair, Variables v);

  //! Histograms, by pair of variables and quantity (nullptr if not made; owned by the folder).
  Histogram * _h[kNVariables][kNQuantities];
  //! Histograms with the Delta phi range shifted (kDetaDphi and kDyDphi only).
  Histogram * _hShifted[kNVariables][kNQuantities];
  //! Number of Delta phi bins of the shift (see shiftY()), 0 if not made.
  int _shiftBins;

};

//!
//! @brief Balance functions of the pair analysis (third phase): PairBFCalculator with the pair folders ParticlePairHistos and
//! the balance-function folders ParticlePairBFHistos, registered as the task class CAP::ParticlePairBFCalculator.
//!
using ParticlePairBFCalculator = PairBFCalculator<ParticlePairHistos,ParticlePairBFHistos>;

} // namespace CAP

#endif /* CAP__ParticlePairBFHistos */
