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
#ifndef CAP__ParticlePairBfCalculator
#define CAP__ParticlePairBfCalculator
#include "Calculator.hpp"
#include "ParticlePairDerivedHistos.hpp"
#include "ParticlePairBfHistos.hpp"

namespace CAP
{
//!
//! @brief Task computing the pair balance functions (ParticlePairBfHistos) from the derived pair histograms
//! (ParticlePairDerivedHistos) read from files.
//!
//! @warning Not part of the build: its base class CalculatorBF (Calculator.hpp) does not exist.
//!
class ParticlePairBfCalculator
:
public CalculatorBF<ParticlePairDerivedHistos,ParticlePairBfHistos,2>
{
public:

  //! @brief Default constructor.
  ParticlePairBfCalculator();
  //! @brief Copy constructor. @param task task to copy
  ParticlePairBfCalculator(const ParticlePairBfCalculator & task);
  //! @brief Destructor.
  virtual ~ParticlePairBfCalculator() {}
  //! @brief Copy assignment. @param task task to copy @return this task
  ParticlePairBfCalculator & operator=(const ParticlePairBfCalculator & task);
  //! @brief Add the default configuration.
  virtual void setDefaultConfiguration();
  //! @brief Compute the balance functions.
  virtual void execute();

  ClassDef(ParticlePairBfCalculator,0)
};

} // namespace CAP

#endif /* CAP__ParticlePairCalculator */
