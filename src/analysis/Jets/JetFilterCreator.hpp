
/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__JetFilterCreator
#define CAP__JetFilterCreator
#include "FilterCreator.hpp"
#include "PseudoJet.hpp"
#include "ClassDef.hpp"

namespace CAP
{
//!
//! @brief Helper creating jet filters from lists of conditions.
//!
class JetFilterCreator
:
public FilterCreator<JetFilter>
{

public:
  
  //! @brief Default constructor.
  JetFilterCreator();
  //! @brief Copy constructor. @param task object to copy
  JetFilterCreator(const JetFilterCreator & task);
  //! @brief Copy assignment. @param task object to copy @return this object
  JetFilterCreator & operator=(const JetFilterCreator & task);
  //! @brief Destructor.
  virtual ~JetFilterCreator() {}

  //! @brief Create a jet filter. @param name name @param title title @param conditionTypes types of the conditions
  //! @param conditionSubypes subtypes (quantities tested) @param conditionMinima minima @param conditionMaxima maxima
  //! @param conditionMinima2 second minima (dual ranges) @param conditionMaxima2 second maxima @param verbose print what is done
  void createFilter(const String & name,
                    const String & title,
                    const vector<String> & conditionTypes,
                    const vector<String> & conditionSubypes,
                    const vector<double> & conditionMinima,
                    const vector<double> & conditionMaxima,
                    const vector<double> & conditionMinima2,
                    const vector<double> & conditionMaxima2,
                    bool verbose=true);

  ClassDef(JetFilterCreator,0)
};

}

#endif /* CAP_FilterCreator */


