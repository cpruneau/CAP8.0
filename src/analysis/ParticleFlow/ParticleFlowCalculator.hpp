/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ParticleFlowCalculator
#define CAP__ParticleFlowCalculator
#include "Calculator.hpp"
#include "ParticleFlowHistos.hpp"
#include "ParticleFlowDerivedHistos.hpp"
#include "ClassDef.hpp"

namespace CAP
{
//!
//! @brief Calculator of the particle flow derived histograms from previously filled histograms.
//! @warning Not built: its base class Calculator (Calculator.hpp) does not exist.
//!
class ParticleFlowCalculator
:
public Calculator<ParticleFlowHistos,ParticleFlowDerivedHistos,1>
{
public:
  
  //! @brief Default constructor.
  ParticleFlowCalculator();
  //! @brief Copy constructor. @param calculator calculator to copy
  ParticleFlowCalculator(const ParticleFlowCalculator & calculator);
  //! @brief Copy assignment. @param calculator calculator to copy @return this calculator
  ParticleFlowCalculator & operator=(const ParticleFlowCalculator & calculator);
  //! @brief Destructor.
  virtual ~ParticleFlowCalculator() {}
  //! @brief Add the default configuration.
  virtual void setDefaultConfiguration();

  ClassDef(ParticleFlowCalculator,0)
};

} // namespace CAP

#endif /* CAP__ParticleFlowCalculator */
