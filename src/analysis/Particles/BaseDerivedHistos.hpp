/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__BaseDerivedHistos
#define CAP__BaseDerivedHistos
#include "BaseHistos.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Base class of the groups of derived histograms: quantities computed at the end of the run from the
//! histograms of a BaseHistos (e.g., correlation functions computed from single and pair densities).
//!
class BaseDerivedHistos : public BaseHistos
{
public:
  //! @brief Default constructor.
  BaseDerivedHistos();
  //! @brief Copy constructor. @param group group to copy
  BaseDerivedHistos(const BaseDerivedHistos & group);
  //! @brief Copy assignment. @param group group to copy @return this group
  BaseDerivedHistos & operator=(const BaseDerivedHistos & group);
  //! @brief Destructor.
  virtual ~BaseDerivedHistos(){ }
  //! @brief Compute the derived histograms from base histograms (nothing in the base class). @param base base histograms
  virtual void calculateDerivedHistograms(const BaseHistos & base  __attribute__((unused))) {}

protected:

  //! @brief Copy the content of another group (nothing in the base class). @param group group to copy
  virtual void cloneD(const BaseDerivedHistos & group  __attribute__((unused))) {}

  ClassDef(BaseDerivedHistos,0)
};

} // namespace CAP

#endif /* CAP__BaseDerivedHistos  */
