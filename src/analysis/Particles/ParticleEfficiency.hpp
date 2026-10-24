/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ParticleEfficiency
#define CAP__ParticleEfficiency
#include "Aliases.hpp"
#include "HistogramFolder.hpp"
#include "Particle.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Particle detection efficiency as a function of pseudorapidity and pT, used to simulate detection losses
//! (accept()) and to correct for them (weightAt()).
//!
//! The efficiency histogram (eta, pT) is created by create() or loaded by loadFrom(). Outside its range the efficiency
//! and the weight are 0.
//!
class ParticleEfficiency
:
public HistogramFolder
{
public:

  //! @brief Default constructor.
  ParticleEfficiency();
  //! @brief Copy constructor. @param source efficiency to copy
  ParticleEfficiency(const ParticleEfficiency & source);
  //! @brief Copy assignment. @param rhs efficiency to copy @return this efficiency
  ParticleEfficiency & operator=(const ParticleEfficiency & rhs);
  //! @brief Destructor.
  virtual ~ParticleEfficiency() { delete h_weight_etaPt; }
  //! @brief Configure the efficiency (binning in pT, eta, phi).
  //! @param ownerName name of the owner (task) @param typeName object type (PARTICLE_EFFICIENCY)
  //! @param configuration configuration @param index index of the efficiency
  virtual void configure(const String & ownerName,
                               const String & typeName,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Create the efficiency histogram.
  virtual void create();
  //! @brief Load the efficiency histogram from a file. @param inputFile file
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Whether a particle is detected: random draw against efficiencyAt(). @param particle particle @return true if detected
  virtual bool accept(const Particle & particle);
  //! @brief Whether a particle of a given momentum is detected: random draw against efficiencyAt().
  //! @param momentum momentum @return true if detected
  virtual bool accept(const VectorLorentz & momentum);
  //! @brief Efficiency at a momentum (0 outside the histogram range). @param momentum momentum @return efficiency
  virtual double efficiencyAt(const VectorLorentz & momentum);
  //! @brief Weight (inverse efficiency) at a momentum, computed on first use (0 outside the histogram range or where
  //! the efficiency is not in (0,1]). @param momentum momentum @return weight
  virtual double weightAt(const VectorLorentz & momentum);
  //! @brief Compute the weight histogram: 1/efficiency in the bins where the efficiency is in (0,1], 0 elsewhere.
  virtual void   calculateWeights();

protected:

  //! @brief Copy the content of another efficiency. @param source efficiency to copy
  void cloneB(const ParticleEfficiency & source);

  //! Efficiency vs (eta, pT).
  Histogram * h_efficiency_etaPt;
  //! Weight (inverse efficiency) vs (eta, pT).
  Histogram * h_weight_etaPt;

  int pt_nbins;  ///< number of bins of the pT axis
  int eta_nbins;  ///< number of bins of the pseudorapidity axis
  int phi_nbins;  ///< number of bins of the azimuth axis
  double pt_min;  ///< lower edge of the pT axis
  double pt_max;  ///< upper edge of the pT axis
  double eta_min;  ///< lower edge of the pseudorapidity axis
  double eta_max;  ///< upper edge of the pseudorapidity axis
  double phi_min;  ///< lower edge of the azimuth axis
  double phi_max;  ///< upper edge of the azimuth axis

  ClassDef(ParticleEfficiency,0)
};

}

#endif /* CAP__ParticleEfficiency */

