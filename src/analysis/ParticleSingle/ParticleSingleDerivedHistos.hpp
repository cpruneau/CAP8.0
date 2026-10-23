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
#ifndef CAP__ParticleSingleDerivedHistos
#define CAP__ParticleSingleDerivedHistos
#include "Profile.hpp"
#include "ParticleSingleHistos.hpp"

namespace CAP
{

//!
//! @brief Histograms derived from the single-particle histograms at the end of the run: projections on phi, eta and y
//! of the densities and pT sums, and average pT vs phi, eta, y, (phi, eta), (phi, y).
//!
class ParticleSingleDerivedHistos
:
public ParticleSingleHistos
{
public:

  //! @brief Default constructor.
  ParticleSingleDerivedHistos();
  //! @brief Copy constructor. @param group group to copy
  ParticleSingleDerivedHistos(const ParticleSingleDerivedHistos & group);
  //! @brief Copy assignment. @param group group to copy @return this group
  ParticleSingleDerivedHistos & operator=(const ParticleSingleDerivedHistos & group);
  //! @brief Destructor.
  virtual ~ParticleSingleDerivedHistos() {}
  //! @brief Read the binning and fill options (see ParticleSingleHistos::configure()).
  //! @param taskName name of the task @param objectType object type @param configuration configuration @param index index
  virtual void configure(const String & taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Create the derived histograms.
  virtual void create();
  //! @brief Load the derived histograms from a file. @param inputFile file
  virtual void loadFrom(const Folder & inputFile);
  //! @brief Compute the projections and pT averages from base histograms. @param baseHistos base histograms
  virtual void calculateDerivedHistograms(ParticleSingleHistos & baseHistos);
  friend class ParticlePairDerivedHistos;
  friend class ParticlePairBfHistos;

protected:
  //! @brief Copy the content of another group. @param group group to copy
  virtual void cloneD(const ParticleSingleDerivedHistos & group);

  //! dN/dphi.
  Histogram * h_n1_phi;
  //! dN/deta.
  Histogram * h_n1_eta;
  //! dN/dy.
  Histogram * h_n1_y;
  //! Sum of pT vs phi.
  Histogram * h_spt_phi;
  //! Sum of pT vs eta.
  Histogram * h_spt_eta;
  //! Sum of pT vs y.
  Histogram * h_spt_y;
  //! Average pT vs phi.
  Histogram * h_avg_pt_phi;
  //! Average pT vs eta.
  Histogram * h_avg_pt_eta;
  //! Average pT vs (eta, phi).
  Histogram * h_avg_pt_phiEta;
  //! Average pT vs y.
  Histogram * h_avg_pt_y;
  //! Average pT vs (y, phi).
  Histogram * h_avg_pt_phiY;

  ClassDef(ParticleSingleDerivedHistos,0)

};

} // namespace CAP

#endif /* CAP__ParticleSingleDerivedHistos  */



