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
#ifndef CAP__JetFilter
#define CAP__JetFilter
#include "Filter.hpp"
#include "fastjet/PseudoJet.hh"
#include "Particle.hpp"

namespace CAP
{

//!
//! @brief Filter selecting jets (FastJet PseudoJet) on their momentum, pT, pseudorapidity or constituent multiplicity.
//!
class JetFilter : public Filter<fastjet::PseudoJet>
{
public:

  //! @brief Default constructor.
  JetFilter();
  //! @brief Copy constructor. @param otherFilter object to copy
  JetFilter(const JetFilter & otherFilter);
  //! @brief Copy assignment. @param otherFilter object to copy @return this object
  JetFilter & operator=(const JetFilter & otherFilter);
  //! @brief Destructor.
  virtual ~JetFilter() {}
  //! @brief Whether a jet satisfies all the conditions. @param pseudoJet jet @return true if accepted
  virtual bool accept(const fastjet::PseudoJet & pseudoJet);

  //! @brief Quantities a condition can test.
  enum JetFilterType   { kNone, kP, kPt, kEta, kMult };

  ClassDef(JetFilter,0)
};

}

#endif /* CAP__JetFilter */

