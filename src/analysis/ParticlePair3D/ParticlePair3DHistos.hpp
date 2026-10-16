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
#ifndef CAP__ParticlePair3DHistos
#define CAP__ParticlePair3DHistos
#include "HistogramFolder.hpp"
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

  friend class ParticlePair3DDerivedHistos;

  //! @brief Default constructor.
  ParticlePair3DHistos();
  //! @brief Copy constructor. @param group group to copy
  ParticlePair3DHistos(const ParticlePair3DHistos & group);
  //! @brief Copy assignment. @param group group to copy @return this group
  ParticlePair3DHistos & operator=(const ParticlePair3DHistos & group);
  //! @brief Destructor.
  virtual ~ParticlePair3DHistos() {}
  //! @brief Read the binning (n2, Qinv, DeltaPs, DeltaPo, DeltaPl, pt, rapidity). @param taskName name of the task
  //! @param objectType object type @param configuration configuration @param index index
  virtual void configure(const String & taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Create the histograms.
  virtual void create();
  //! @brief Load the histograms from a file. @param inputFile file
  virtual void loadFrom(CapFile & inputFile);
  //! @brief Fill the histograms with a pair: compute Qinv and (q_side, q_out, q_long).
  //! @param particle1 first particle @param particle2 second particle @param weight weight
  virtual void fill(Particle & particle1, Particle & particle2, double weight);
  //! @brief Fill the number of pairs of the event. @param nPairs number of pairs @param weigh weight
  virtual void fillMultiplicity(double nPairs, double weigh=1.0);
  //! @brief Copy the content of another group. @param source group to copy
  virtual void cloneB(const ParticlePair3DHistos & source);
  friend class ParticlePair3DDerivedHistos;
  friend class ParticlePair3DBFHistos;

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

  ClassDef(ParticlePair3DHistos,0)
};

} // namespace CAP

#endif /* CAP__ParticlePair3DHistos  */

