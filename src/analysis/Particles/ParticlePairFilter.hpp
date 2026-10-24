/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ParticlePairFilter
#define CAP__ParticlePairFilter
#include "Filter.hpp"
#include "Particle.hpp"

namespace CAP
{

//!
//! @brief Filter of particle pairs on their relative kinematics (differences of pT, azimuth and pseudorapidity).
//!
class ParticlePairFilter
:
public Filter<Particle>
{
public:

   //! @brief Default constructor.
   ParticlePairFilter();
  //! @brief Destructor.
  virtual ~ParticlePairFilter(){}
  
  //!
  //!Accept/reject the given pair of particles
  //!
  virtual bool accept(const Particle & particle1, const Particle & particle2) const;

protected:
  
  double dpt_min;  ///< lower edge of the pT difference axis
  double dpt_max;  ///< upper edge of the pT difference axis
  double Dphi_min;  ///< lower edge of the Delta phi axis
  double Dphi_max;  ///< upper edge of the Delta phi axis
  double Deta_min;  ///< lower edge of the Delta eta axis
  double Deta_max;  ///< upper edge of the Delta eta axis

};

} // namespace CAP

#endif /* CAP__ParticlePairFilter */
