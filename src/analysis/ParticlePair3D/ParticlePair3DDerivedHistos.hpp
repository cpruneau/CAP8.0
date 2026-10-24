/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ParticlePair3DDerivedHistos
#define CAP__ParticlePair3DDerivedHistos
#include "Profile.hpp"
#include "ParticleSingleDerivedHistos.hpp"
#include "ParticlePair3DHistos.hpp"

namespace CAP
{

//!
//! @brief Two-particle correlation functions in relative momentum variables (Qinv, q_side, q_out, q_long), computed at
//! the end of the run: products of single densities, cumulant C2, normalized cumulant R2, associated yields A(1|2) and
//! A(2|1); also single-particle pT and rapidity densities.
//!
class ParticlePair3DDerivedHistos : public  ParticlePair3DHistos
{
public:

  //! @brief Default constructor.
  ParticlePair3DDerivedHistos();
  //! @brief Copy constructor. @param group group to copy
  ParticlePair3DDerivedHistos(const ParticlePair3DDerivedHistos & group);
  //! @brief Copy assignment. @param group group to copy @return this group
  ParticlePair3DDerivedHistos & operator=(const ParticlePair3DDerivedHistos & group);
  //! @brief Destructor.
  virtual ~ParticlePair3DDerivedHistos() {}
  //! @brief Copy the content of another group. @param group group to copy
  virtual void cloneD(const ParticlePair3DDerivedHistos & group);
  //! @brief Read the binning. @param taskName name of the task @param objectType object type
  //! @param configuration configuration @param index index
  virtual void configure(const String & taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Load the histograms from a file. @param inputFile file
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Create the derived histograms.
  virtual void create();
  //! @brief Compute the correlation functions. @param part1DerivedHistos derived single histograms of particle 1
  //! @param part2DerivedHistos same for particle 2 @param pairHistos pair histograms @param bincorrection bin correction factor
  virtual void calculateDerivedHistograms(ParticleSingleDerivedHistos & part1DerivedHistos,
                                          ParticleSingleDerivedHistos & part2DerivedHistos,
                                          ParticlePair3DHistos        & pairHistos,
                                          double bincorrection=1.0);
  //! @brief Set the masses of the two particles. @param m1 mass of particle 1 @param m2 mass of particle 2
  virtual void setMasses(double m1, double m2);

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

  //! @brief Project a rank 3 histogram onto an axis into the histogram of this group named targetName.
  //! @param source rank 3 histogram @param axis axis kept (0, 1 or 2) @param targetName name of the target histogram
  void projectInto(const Histogram * source, unsigned int axis, const String & targetName);



};

} // namespace CAP

#endif /* CAP__ParticlePair3DDerivedHistos */
