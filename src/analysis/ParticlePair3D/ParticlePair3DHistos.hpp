/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ParticlePair3DHistos
#define CAP__ParticlePair3DHistos
#include "HistogramFolder.hpp"
#include "ParticleSingleHistos.hpp"
#include "Particle.hpp"
#include "ParticleDigit.hpp"



namespace CAP
{
//!
//! @brief Pair histograms in relative momentum (femtoscopy-like) variables for one (event filter, particle filter 1,
//! particle filter 2) combination: pair density vs Qinv and vs (q_side, q_out, q_long) in the longitudinally
//! comoving frame (Bertsch-Pratt), and number of pairs per event.
//!
class ParticlePair3DHistos : public HistogramFolder
{
public:


  //! @brief Default constructor.
  ParticlePair3DHistos();
  //! @brief Copy constructor. @param group group to copy
  ParticlePair3DHistos(const ParticlePair3DHistos & group);
  //! @brief Copy assignment. @param group group to copy @return this group
  ParticlePair3DHistos & operator=(const ParticlePair3DHistos & group);
  //! @brief Destructor.
  virtual ~ParticlePair3DHistos() {}
  //!
  //! @brief Add the default settings (keys \<taskName\>:HISTOGRAM:...) to a configuration: those of the
  //! single-particle histograms (ParticleSingleHistos::addDefaultConfiguration(), with rapidity_fill and FillPtvsY on),
  //! the binning of the pairs (n2, Qinv, DeltaPs, DeltaPo, DeltaPl) and nMcSimEvents (number of Monte Carlo events of
  //! the n1n1 calculation): those of every task working on these histograms (ParticlePair3DAnalyzer,
  //! ParticlePair3DDerivedCalculator). @param configuration configuration @param taskName name of the task
  //!
  static void addDefaultConfiguration(Configuration & configuration, const String & taskName);
  //! @brief Read the binning (n2, Qinv, DeltaPs, DeltaPo, DeltaPl, pt, rapidity) and nMcSimEvents. @param taskName name of the task
  //! @param objectType object type @param configuration configuration @param index index
  virtual void configure(const String & taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Create the histograms.
  virtual void create();
  //! @brief Load the base histograms from a folder of a file, and the derived histograms if the folder has them.
  //! @param inputFile folder
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Create the derived histograms, empty; see calculateDerived().
  virtual void createDerived();
  //! @brief Whether the derived histograms exist (created or loaded). @return true if they do
  bool hasDerived() const { return h_n1n1_Qinv!=nullptr; }
  //! @brief Load the derived histograms from a folder of a file. @param inputFile folder
  virtual void loadDerivedFrom(const Folder & inputFile);
  //!
  //! @brief Compute the derived histograms (n1n1 by Monte Carlo folding of the (y, pT) densities of the two particles,
  //! C2, R2, associated yields, projections) from the pair histograms of this folder and the single-particle histograms
  //! of the two particle filters (createDerived() first). The histograms must hold per-event quantities.
  //! @param part1Histos single-particle histograms of particle filter 1 @param part2Histos same for particle filter 2
  //! @param same whether the two particle filters are the same
  //! @throws Exception if the derived histograms were not created or the single-particle histograms have no (y, pT)
  //! histogram
  //!
  virtual void calculateDerived(const ParticleSingleHistos & part1Histos, const ParticleSingleHistos & part2Histos, bool same);
  //! @brief Set the masses of the two particles (Qinv of the Monte Carlo pairs). @param m1 mass of particle 1
  //! @param m2 mass of particle 2
  virtual void setMasses(double m1, double m2);
  //! @brief Fill the histograms with a pair: compute Qinv and (q_side, q_out, q_long).
  //! @param particle1 first particle @param particle2 second particle @param weight weight
  virtual void fill(Particle & particle1, Particle & particle2, double weight);
  //! @brief Fill the number of pairs of the event. @param nPairs number of pairs @param weigh weight
  virtual void fillMultiplicity(double nPairs, double weigh=1.0);
  //! @brief Copy the content of another group. @param source group to copy
  virtual void cloneB(const ParticlePair3DHistos & source);

protected:

  int    n2_nbins;  ///< number of bins of the pair multiplicity axis
  int    Qinv_nbins;  ///< number of bins of the Qinv axis
  int    DeltaPs_nbins;  ///< number of bins of the q_side axis
  int    DeltaPo_nbins;  ///< number of bins of the q_out axis
  int    DeltaPl_nbins;  ///< number of bins of the q_long axis
  int    pt_nbins;  ///< number of bins of the pT axis
  int    rapidity_nbins;  ///< number of bins of the rapidity axis
  double n2_min;  ///< lower edge of the pair multiplicity axis
  double n2_max;  ///< upper edge of the pair multiplicity axis
  double Qinv_min;  ///< lower edge of the Qinv axis
  double Qinv_max;  ///< upper edge of the Qinv axis
  double DeltaPs_min;  ///< lower edge of the q_side axis
  double DeltaPs_max;  ///< upper edge of the q_side axis
  double DeltaPo_min;  ///< lower edge of the q_out axis
  double DeltaPo_max;  ///< upper edge of the q_out axis
  double DeltaPl_min;  ///< lower edge of the q_long axis
  double DeltaPl_max;  ///< upper edge of the q_long axis
  double pt_min;  ///< lower edge of the pT axis
  double pt_max;  ///< upper edge of the pT axis
  double rapidity_minimum;  ///< lower edge of the rapidity axis
  double rapidity_maximum;  ///< upper edge of the rapidity axis

  Histogram * h_n2;  ///< number of pairs per event
  Histogram * h_n2_Qinv;  ///< pair density vs Qinv
  Histogram * h_n2_DeltaP;  ///< pair density vs (q_side, q_out, q_long)


public:

  // Derived histograms (createDerived(), calculateDerived(); computed by ParticlePair3DDerivedCalculator)
  Histogram * h_n1n1_Qinv;  ///< product of the single-particle densities vs Qinv
  Histogram * h_n1n1_DeltaP;  ///< product of the single-particle densities vs (q_side, q_out, q_long)
  Histogram * h_n1n1_DeltaPs;  ///< product of the single-particle densities vs q_side
  Histogram * h_n1n1_DeltaPo;  ///< product of the single-particle densities vs q_out
  Histogram * h_n1n1_DeltaPl;  ///< product of the single-particle densities vs q_long

  Histogram * h_n2_DeltaPs;  ///< pair density vs q_side
  Histogram * h_n2_DeltaPo;  ///< pair density vs q_out
  Histogram * h_n2_DeltaPl;  ///< pair density vs q_long

  
  Histogram * h_c2_Qinv;  ///< cumulant C2 = n2 - n1n1 vs Qinv
  Histogram * h_c2_DeltaP;  ///< cumulant C2 = n2 - n1n1 vs (q_side, q_out, q_long)
  Histogram * h_c2_DeltaPs;  ///< cumulant C2 = n2 - n1n1 vs q_side
  Histogram * h_c2_DeltaPo;  ///< cumulant C2 = n2 - n1n1 vs q_out
  Histogram * h_c2_DeltaPl;  ///< cumulant C2 = n2 - n1n1 vs q_long

  
  Histogram * h_r2_Qinv;  ///< normalized cumulant R2 = n2/n1n1 - 1 vs Qinv
  Histogram * h_r2_DeltaP;  ///< normalized cumulant R2 = n2/n1n1 - 1 vs (q_side, q_out, q_long)
  Histogram * h_r2_DeltaPs;  ///< normalized cumulant R2 = n2/n1n1 - 1 vs q_side
  Histogram * h_r2_DeltaPo;  ///< normalized cumulant R2 = n2/n1n1 - 1 vs q_out
  Histogram * h_r2_DeltaPl;  ///< normalized cumulant R2 = n2/n1n1 - 1 vs q_long

  Histogram * h_a12_Qinv;  ///< associated yield A(1|2) vs Qinv
  Histogram * h_a12_DeltaPs;  ///< associated yield A(1|2) vs q_side
  Histogram * h_a12_DeltaPo;  ///< associated yield A(1|2) vs q_out
  Histogram * h_a12_DeltaPl;  ///< associated yield A(1|2) vs q_long

  Histogram * h_a21_Qinv;  ///< associated yield A(2|1) vs Qinv
  Histogram * h_a21_DeltaPs;  ///< associated yield A(2|1) vs q_side
  Histogram * h_a21_DeltaPo;  ///< associated yield A(2|1) vs q_out
  Histogram * h_a21_DeltaPl;  ///< associated yield A(2|1) vs q_long

  Histogram * h_n1_1_pt;  ///< pT density of particle 1
  Histogram * h_n1_2_pt;  ///< pT density of particle 2
  Histogram * h_n1_1_y;  ///< rapidity density of particle 1
  Histogram * h_n1_2_y;  ///< rapidity density of particle 2

  Histogram * h_n1_1_pt_mc;  ///< density of particle 1 vs pT, Monte Carlo correction
  Histogram * h_n1_2_pt_mc;  ///< density of particle 2 vs pT, Monte Carlo correction
  Histogram * h_n1_1_y_mc;  ///< density of particle 1 vs y, Monte Carlo correction
  Histogram * h_n1_2_y_mc;  ///< density of particle 2 vs y, Monte Carlo correction

  Histogram * h_n1r_1_pt_mc;  ///< reweighted density of particle 1 vs pT, Monte Carlo correction
  Histogram * h_n1r_2_pt_mc;  ///< reweighted density of particle 2 vs pT, Monte Carlo correction
  Histogram * h_n1r_1_y_mc;  ///< reweighted density of particle 1 vs y, Monte Carlo correction
  Histogram * h_n1r_2_y_mc;  ///< reweighted density of particle 2 vs y, Monte Carlo correction

  Histogram * h_n1_1_ptY_mc;  ///< density of particle 1 vs (y, pT), Monte Carlo correction
  Histogram * h_n1_2_ptY_mc;  ///< density of particle 2 vs (y, pT), Monte Carlo correction
  Histogram * h_n1r_1_ptY_mc;  ///< reweighted density of particle 1 vs (y, pT), Monte Carlo correction
  Histogram * h_n1r_2_ptY_mc;  ///< reweighted density of particle 2 vs (y, pT), Monte Carlo correction

  //! Mass of particle 1.
  double mass1;
  //! Mass of particle 2.
  double mass2;

protected:

  //! Number of Monte Carlo events of the n1n1 calculation (key \<taskName\>:HISTOGRAM:nMcSimEvents).
  long nMcSimEvents;
  //! @brief Project a rank 3 histogram onto an axis into the histogram of this group named targetName.
  //! @param source rank 3 histogram @param axis axis kept (0, 1 or 2) @param targetName name of the target histogram
  void projectInto(const Histogram * source, unsigned int axis, const String & targetName);

};

} // namespace CAP

#endif /* CAP__ParticlePair3DHistos  */

