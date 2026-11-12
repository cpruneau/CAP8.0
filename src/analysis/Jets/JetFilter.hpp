/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file JetFilter.hpp
//! @brief JetFilter: selection of jets on their kinematics and their rank in the event; JetFilterCreator: the task
//! creating the jet filters of a job (JET_FILTER).
#ifndef CAP__JetFilter
#define CAP__JetFilter
#include "Filter.hpp"
#include "ManagedObject.hpp"
#include "ObjectCreator.hpp"
#include "Configuration.hpp"
#include "Jet.hpp"

namespace CAP
{

//!
//! @brief Filter of jets: all its conditions must hold.
//!
//! Quantities (condition SUBTYPE), with the forms DOUBLE_RANGE [minimum, maximum) and DOUBLE_DUAL_RANGE (PT, ETA, PHI,
//! RANK, MULT, MASS) and INTEGER (RANK, MULT: the value, MINIMUM):
//!  - PT, ETA, PHI (in [0, 2 pi)), MASS: of the jet;
//!  - RANK: its rank in pT among the jets of the event found with the same particle filter and settings (1: leading,
//!    2: subleading, ...; e.g. INTEGER 1 for the leading jet, DOUBLE_RANGE [1, 3) for the two leading jets);
//!  - MULT: its number of constituents.
//!
//! The filters are defined in the configuration by the keys \<owner\>:JET_FILTER_\<k\>:NAME, CONDITION:N and
//! CONDITION_\<i\>:TYPE, SUBTYPE, MINIMUM, MAXIMUM (MINIMUM2, MAXIMUM2), made by JetFilterCreator (OWNER true) and
//! used by name by the jet analyzers (OWNER false), as the particle filters.
//!
class JetFilter
:
public Filter<Jet>,
public ManagedObject
{
public:

  //! @brief Quantities tested.
  enum JetFilterType { kNone, kPt, kEta, kPhi, kRank, kMult, kMass };

  //! @brief Filter accepting all jets.
  JetFilter();
  //! @brief Copy constructor. @param source filter
  JetFilter(const JetFilter & source);
  //! @brief Copy assignment. @param rhs filter @return this filter
  JetFilter & operator=(const JetFilter & rhs);
  //! @brief Destructor.
  virtual ~JetFilter() {}
  //! @brief Whether a jet passes all the conditions. @param jet jet @return true if accepted
  virtual bool accept(const Jet & jet) const override;
  //! @brief Read the conditions. @param ownerName owner @param typeName JET_FILTER @param configuration configuration
  //! @param index index of the filter @throws FilterException for an unknown type or quantity
  virtual void configure(const String & ownerName, const String & typeName, const Configuration & configuration,
                         unsigned int index);
  //! @brief Quantity of a SUBTYPE (kNone if unknown). @param subtype name @return quantity
  static JetFilterType typeOf(const String & subtype);
};

//!
//! @brief Task creating the jet filters of a job (JET_FILTER), see ObjectCreator; a subtask of FilterCreator.
//!
class JetFilterCreator : public ObjectCreator<JetFilter>
{
public:
  //! @brief Constructor.
  JetFilterCreator() : ObjectCreator<JetFilter>("JET_FILTER") { appendClassName("JetFilterCreator"); setName("JetFilterCreator"); }
  //! @brief Copy. @return new task
  Task * clone() override { return new JetFilterCreator(*this); }
};

} // namespace CAP

#endif /* CAP__JetFilter */
