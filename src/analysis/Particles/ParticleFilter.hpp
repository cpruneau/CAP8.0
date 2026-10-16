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
#ifndef CAP__ParticleFilter
#define CAP__ParticleFilter
#include "Particle.hpp"
#include "Filter.hpp"
#include "ManagedObject.hpp"
#include "Configuration.hpp"

namespace CAP
{

  //!
  //! @brief Filter selecting particles by kinematics (pT, px, py, pz, energy, pseudorapidity, rapidity, azimuth) and by
  //! properties of their type (PDG code, charge, strangeness, baryon number, charm) or state (live, enabled type).
  //!
  //! A particle is accepted when it satisfies all the conditions (none: all accepted). The analysis tasks use one
  //! ParticleFilter per particle species or selection (e.g. h+, h-, pi+, ...): see EventProcessor::filterParticles().
  //!
  //! Configuration (read by configure()), for filter \<index\> of owner \<owner\>:
  //! @code
  //!   <owner>:PARTICLE_FILTER_<index>:CONDITION:N = 2
  //!   <owner>:PARTICLE_FILTER_<index>:CONDITION_0:TYPE = DOUBLE_RANGE      // DOUBLE_RANGE, DOUBLE_DUAL_RANGE, INTEGER, ...
  //!   <owner>:PARTICLE_FILTER_<index>:CONDITION_0:SUBTYPE = PT             // PT, PX, PY, PZ, ENERGY, ETA, RAPIDITY, PHI,
  //!                                                                       // PDG, CHARGE, STRANGE, BARYON, CHARM, LIVE, ENABLED
  //!   <owner>:PARTICLE_FILTER_<index>:CONDITION_0:MINIMUM = 0.2
  //!   <owner>:PARTICLE_FILTER_<index>:CONDITION_0:MAXIMUM = 2.0
  //! @endcode
  //!
  class ParticleFilter
  :
  public Filter<Particle>,
  public ManagedObject
  {
  public:
  //! @brief Quantities a condition can test.
  enum ParticleFilterType   { kNone, kPt, kPx, kPy, kPz, kEnergy, kEta, kRapidity, kPhi, kPdg,  kCharge, kStrange, kBaryon, kCharm, kBeauty, kLive, kEnabled, kEfficiency};

  //! @brief Default constructor: no conditions (all particles accepted).
  ParticleFilter();
  //! @brief Copy constructor (the conditions are cloned). @param source filter to copy
  ParticleFilter(const ParticleFilter & source);
  //! @brief Copy assignment (the conditions are cloned). @param rhs filter to copy @return this filter
  ParticleFilter & operator=(const ParticleFilter & rhs);
  //! @brief Destructor.
  virtual ~ParticleFilter() {}
  //! @brief Whether a particle satisfies all the conditions. @param particle particle @return true if accepted
  virtual bool accept(const Particle & particle) const;
  //! @brief Read the conditions from a configuration (see the class description).
  //! @param ownerName name of the owner (task) @param typeName object type (PARTICLE_FILTER)
  //! @param configuration configuration @param index index of the filter
  virtual void configure(const String & ownerName,
                         const String & typeName,
                         const Configuration & configuration,
                         unsigned int index);

  ClassDef(ParticleFilter,0)
  };

}

#endif /* CAP__ParticleFilter */

