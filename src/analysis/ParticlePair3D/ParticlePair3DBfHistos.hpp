/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file ParticlePair3DBFHistos.hpp
//! @brief ParticlePair3DBFHistos: balance functions in relative momentum variables of two species and their
//! antiparticles.
#ifndef CAP__ParticlePair3DBFHistos
#define CAP__ParticlePair3DBFHistos
#include "HistogramFolder.hpp"
#include "PairBFCalculator.hpp"
#include "ParticleSingleHistos.hpp"
#include "ParticlePair3DHistos.hpp"

namespace CAP
{

//!
//! @brief Balance functions of a species a and a species b (and of their antiparticles ā and b̄) in relative momentum
//! variables, with the charge independent and charge dependent combinations of R2, computed from the 3D pair folders of
//! the four combinations (a,b), (a,b̄), (ā,b), (ā,b̄) and the single-particle folders of a and ā (see PairBFCalculator).
//! It is the counterpart of ParticlePairBFHistos for ParticlePair3DHistos: same quantities, other variables.
//!
//! For each variable v (Qinv, the 3D (q_side, q_out, q_long), and the projections q_side, q_out, q_long), with R^{xy}
//! the normalized cumulant r2 and n^{xy} the pair density per event of the pairs (x, y):
//!  - R2CI_v  = (R^{ab} + R^{ab̄} + R^{āb} + R^{āb̄})/4, the charge independent combination;
//!  - R2CD_v  = (R^{ab̄} + R^{āb} - R^{ab} - R^{āb̄})/2, the charge dependent combination;
//!  - B_a_bB_v = n^{ab̄}/N̄_a - n^{āb̄}/N̄_ā, the balance function B^{a|b̄} per trigger particle a (resp. ā);
//!  - B_aB_b_v = n^{āb}/N̄_ā - n^{ab}/N̄_a, the balance function B^{ā|b};
//!  - B_v = (B_a_bB_v + B_aB_b_v)/2;
//! with N̄ the mean number of trigger particles per event (integral of their (y, pT) density, as in
//! ParticlePair3DHistos::calculateDerived()). Statistical uncertainties are propagated assuming the histograms
//! combined are uncorrelated.
//!
//! The histograms have the binning of the r2 histograms of the pair folders (createFrom()); their names are short (e.g.
//! B_Qinv): the folder of the job names the species and the event filter (e.g. ALL_PiP_KP_BF).
//!
class ParticlePair3DBFHistos : public HistogramFolder
{
public:

  //! @brief Variables of the pair histograms.
  enum Variables { kQinv, kDeltaP, kDeltaPs, kDeltaPo, kDeltaPl, kNVariables };
  //! @brief Quantities computed for each variable (see the class description).
  enum Quantities { kR2CI, kR2CD, kBabB, kBaBb, kB, kNQuantities };

  //! @brief Constructor. @param name name of the folder
  ParticlePair3DBFHistos(const String & name="ParticlePair3DBFHistos");
  //! @brief Copy constructor: copies the histograms. @param source folder to copy
  ParticlePair3DBFHistos(const ParticlePair3DBFHistos & source);
  //! @brief Assignment: replaces the histograms by copies of those of rhs. @param rhs folder to copy @return this folder
  ParticlePair3DBFHistos & operator=(const ParticlePair3DBFHistos & rhs);
  //! @brief Destructor.
  virtual ~ParticlePair3DBFHistos() {}
  //! @brief Name of the task computing these folders (see PairBFCalculator). @return name
  static const char * calculatorName() { return "ParticlePair3DBFCalculator"; }
  //! @brief Default name of the analyzer task whose folders the calculator reads. @return name
  static const char * defaultSourceTask() { return "ParticlePair3DAnalyzer"; }
  //! @brief Copy of this folder. @return new folder, owned by the caller
  virtual HistogramFolder * clone() const override { return new ParticlePair3DBFHistos(*this); }

  //! @brief Name of a variable, as in the names of the histograms (Qinv, DeltaP, DeltaPs, DeltaPo, DeltaPl).
  //! @param v variable @return name
  static const char * variableName(Variables v);
  //! @brief Name of a quantity, as in the names of the histograms (R2CI, R2CD, B_a_bB, B_aB_b, B). @param q quantity
  //! @return name
  static const char * quantityName(Quantities q);

  //!
  //! @brief Create the histograms (empty) with the binning of the r2 histograms of a pair folder (derived histograms
  //! computed). Existing histograms are deleted first. @param pair pair folder of the analysis
  //!
  void createFrom(const ParticlePair3DHistos & pair);
  //!
  //! @brief Compute the histograms from the pair folders of the four combinations of a, ā, b, b̄ and the single-particle
  //! folders, all with their derived histograms computed (see ParticlePair3DHistos::calculateDerived()).
  //! @param ab pairs (a,b) @param abB pairs (a,b̄) @param aBb pairs (ā,b) @param aBbB pairs (ā,b̄)
  //! @param a singles of a @param aB singles of ā @param b singles of b (not used) @param bB singles of b̄ (not used)
  //! @throws HistogramException if a histogram is missing or the binnings differ
  //!
  void calculate(const ParticlePair3DHistos & ab,  const ParticlePair3DHistos & abB,
                 const ParticlePair3DHistos & aBb, const ParticlePair3DHistos & aBbB,
                 const ParticleSingleHistos & a, const ParticleSingleHistos & aB,
                 const ParticleSingleHistos & b, const ParticleSingleHistos & bB);
  //! @brief Histogram of a quantity for a variable. @param q quantity @param v variable @return histogram, or nullptr
  Histogram * histogram(Quantities q, Variables v) const { return _h[v][q]; }

  using HistogramFolder::histogram;

protected:

  //! @brief Set the pointers to the histograms from their names (after a copy).
  void relink();
  //! @brief Histogram of a pair folder for a variable: r2 (prefix "r2") or n2 (prefix "n2"), or nullptr.
  //! @param pair folder @param prefix "r2" or "n2" @param v variable
  static const Histogram * of(const ParticlePair3DHistos & pair, const char * prefix, Variables v);

  //! Histograms, by variable and quantity (nullptr if not made; owned by the folder).
  Histogram * _h[kNVariables][kNQuantities];

};

//!
//! @brief Balance functions of the 3D pair analysis (third phase): PairBFCalculator with the pair folders ParticlePair3DHistos and
//! the balance-function folders ParticlePair3DBFHistos, registered as the task class CAP::ParticlePair3DBFCalculator.
//!
using ParticlePair3DBFCalculator = PairBFCalculator<ParticlePair3DHistos,ParticlePair3DBFHistos>;

} // namespace CAP

#endif /* CAP__ParticlePair3DBFHistos */
