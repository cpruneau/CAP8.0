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
#ifndef CAP__ParticleSmearer
#define CAP__ParticleSmearer
#include "HistogramFolder.hpp"
#include "Particle.hpp"
#include "MathConstants.hpp"
#include "Configuration.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Smearer of the particle momenta, simulating the detector resolution.
//!
//! smear() draws the pT, pseudorapidity and azimuth of the target from Gaussian distributions centered on the source
//! values plus a bias, with a width (rms); bias and rms are read from histograms vs (eta, pT). Outside the histogram
//! range the values are not smeared.
//!
class ParticleSmearer : public HistogramFolder
{
public:
  //! @brief Default constructor.
  ParticleSmearer();
  //! @brief Copy constructor. @param group smearer to copy
  ParticleSmearer(const ParticleSmearer & group);
  //! @brief Copy assignment. @param group smearer to copy @return this smearer
  ParticleSmearer & operator=(const ParticleSmearer & group);
  //! @brief Destructor.
  virtual ~ParticleSmearer(){ }
  //! @brief Configure the smearer (binning in pT, eta, phi).
  //! @param ownerName name of the owner (task) @param typeName object type (PARTICLE_SMEARER)
  //! @param configuration configuration @param index index of the smearer
  virtual void configure(const String & ownerName,
                               const String & typeName,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Create the bias and rms histograms.
  virtual void create();
  //! @brief Load the bias and rms histograms from a file. @param inputFile file
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Set the momentum of target to the smeared momentum of source (see the class description).
  //! @param source particle @param target smeared particle
  virtual void smear(const Particle & source, Particle & target);
  //! @brief Copy the content of another smearer. @param source smearer to copy
  virtual void cloneB(const ParticleSmearer & source);

protected:

  int    n_nbins;  ///< number of bins of the multiplicity axis
  int    pt_nbins;  ///< number of bins of the pT axis
  int    phi_nbins;  ///< number of bins of the azimuth axis
  int    eta_nbins;  ///< number of bins of the pseudorapidity axis
  double pt_min;  ///< lower edge of the pT axis
  double pt_max;  ///< upper edge of the pT axis
  double phi_min;  ///< lower edge of the azimuth axis
  double phi_max;  ///< upper edge of the azimuth axis
  double eta_min;  ///< lower edge of the pseudorapidity axis
  double eta_max;  ///< upper edge of the pseudorapidity axis

  //! Bias of pT vs (eta, pT).
  Histogram * h_pt_bias;
  //! Resolution (rms) of pT vs (eta, pT).
  Histogram * h_pt_rms;
  //! Bias of eta vs (eta, pT).
  Histogram * h_eta_bias;
  //! Resolution (rms) of eta vs (eta, pT).
  Histogram * h_eta_rms;
  //! Bias of phi vs (eta, pT).
  Histogram * h_phi_bias;
  //! Resolution (rms) of phi vs (eta, pT).
  Histogram * h_phi_rms;
  //! PDG codes of the species.
  std::vector<int> pids;
  //! Identification probability of each species.
  std::vector<Histogram*> h_prob_species;
  //! Number of species.
  unsigned int nSpecies;

  ClassDef(ParticleSmearer,0)

};

} // namespace CAP

#endif /* CAP__ParticleSmearer  */



