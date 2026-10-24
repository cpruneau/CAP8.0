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
#ifndef CAP__ParticlePair3DBfHistos
#define CAP__ParticlePair3DBfHistos
#include "Profile.hpp"
#include "ParticleSingleHistos.hpp"
#include "ParticleSingleDerivedHistos.hpp"
#include "ParticlePair3DHistos.hpp"
#include "ParticlePair3DDerivedHistos.hpp"
#include "ClassDef.hpp"

namespace CAP
{
//!
//! @brief Balance functions in relative momentum variables (Qinv, q_side, q_out, q_long), computed from the correlation
//! functions of the four combinations of a species and its antiparticle (1 2, 1 2bar, 1bar 2, 1bar 2bar).
//!
class ParticlePair3DBfHistos : public  ParticlePair3DDerivedHistos
{
public:

  //! @brief Default constructor.
  ParticlePair3DBfHistos();
  //! @brief Copy constructor. @param group group to copy
  ParticlePair3DBfHistos(const ParticlePair3DBfHistos & group);
  //! @brief Copy assignment. @param group group to copy @return this group
  ParticlePair3DBfHistos & operator=(const ParticlePair3DBfHistos & group);
  //! @brief Destructor.
  virtual ~ParticlePair3DBfHistos() {}
  //! @brief Read the binning. @param taskName name of the task @param objectType object type
  //! @param configuration configuration @param index index
  virtual void configure(const String & taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Load the histograms from a file. @param inputFile file
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Create the balance function histograms.
  virtual void create();
  //! @brief Compute the balance functions. @param pair12B pairs (1, 2bar) @param pair1B2B pairs (1bar, 2bar)
  //! @param pair1B2 pairs (1bar, 2) @param pair12 pairs (1, 2)
  virtual void calculateBfHistograms(ParticlePair3DDerivedHistos & pair12B,
                                     ParticlePair3DDerivedHistos & pair1B2B,
                                     ParticlePair3DDerivedHistos & pair1B2,
                                     ParticlePair3DDerivedHistos & pair12);
  //! @brief Set the masses of the two particles. @param m1 mass of particle 1 @param m2 mass of particle 2
  virtual void setMasses(double m1, double m2);
  //! @brief Copy the content of another group. @param source group to copy
  virtual void cloneBF(const ParticlePair3DBfHistos & source);

protected:

  Histogram * h_B12_Qinv;  ///< balance function B(1|2) vs Qinv
  Histogram * h_B12_DeltaPs;  ///< balance function B(1|2) vs q_side
  Histogram * h_B12_DeltaPo;  ///< balance function B(1|2) vs q_out
  Histogram * h_B12_DeltaPl;  ///< balance function B(1|2) vs q_long

  Histogram * h_B21_Qinv;  ///< balance function B(2|1) vs Qinv
  Histogram * h_B21_DeltaPs;  ///< balance function B(2|1) vs q_side
  Histogram * h_B21_DeltaPo;  ///< balance function B(2|1) vs q_out
  Histogram * h_B21_DeltaPl;  ///< balance function B(2|1) vs q_long

  Histogram * h_Bs_Qinv;  ///< sum of the balance functions vs Qinv
  Histogram * h_Bs_DeltaPs;  ///< sum of the balance functions vs q_side
  Histogram * h_Bs_DeltaPo;  ///< sum of the balance functions vs q_out
  Histogram * h_Bs_DeltaPl;  ///< sum of the balance functions vs q_long

  //! Mass of particle 1.
  double mass1;
  //! Mass of particle 2.
  double mass2;

  ClassDef(ParticlePair3DBfHistos,1)

};

} // namespace CAP

#endif /* CAP__ParticlePair3DDerivedHistos */
