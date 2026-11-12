/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file JetPair.hpp
//! @brief JetPair: two jets of an event and the variables of the pair; JetPairFilter: selection of pairs;
//! JetPairFilterCreator: the task creating the jet pair filters (JET_PAIR_FILTER).
#ifndef CAP__JetPair
#define CAP__JetPair
#include "Filter.hpp"
#include "ManagedObject.hpp"
#include "ObjectCreator.hpp"
#include "Configuration.hpp"
#include "Jet.hpp"

namespace CAP
{

//!
//! @brief Two jets of an event, the first of higher rank (higher pT), and the variables of the pair:
//!  - deltaPhi = phi1 - phi2 in [-pi/2, 3 pi/2); absDeltaPhi = |phi1 - phi2| in [0, pi]; acoplanarity = pi - absDeltaPhi;
//!  - deltaEta = eta1 - eta2;
//!  - ptRatio x_J = pT2/pT1 (<= 1); asymmetry A_J = (pT1 - pT2)/(pT1 + pT2);
//!  - mass, pt and rapidity of the pair (the sum of the four-momenta).
//!
struct JetPair
{
  const Jet * first = nullptr;    //!< jet of higher rank
  const Jet * second = nullptr;   //!< jet of lower rank

  //! @brief Pair, ordered by rank. @param a jet @param b jet
  JetPair(const Jet * a, const Jet * b) : first(a->rank<=b->rank ? a : b), second(a->rank<=b->rank ? b : a) { }
  //! @brief phi1 - phi2 in [-pi/2, 3 pi/2) (back-to-back at pi). @return deltaPhi
  double deltaPhi() const;
  //! @brief |phi1 - phi2| in [0, pi]. @return |deltaPhi|
  double absDeltaPhi() const;
  //! @brief pi - |phi1 - phi2| (0: back to back). @return acoplanarity
  double acoplanarity() const;
  //! @brief eta1 - eta2. @return deltaEta
  double deltaEta() const { return first->eta()-second->eta(); }
  //! @brief pT2/pT1. @return x_J
  double ptRatio() const { return first->pt()>0.0 ? second->pt()/first->pt() : 0.0; }
  //! @brief (pT1 - pT2)/(pT1 + pT2). @return A_J
  double asymmetry() const;
  //! @brief Four-momentum of the pair. @return momentum
  MATH::VectorLorentz momentum() const;
  //! @brief Invariant mass of the pair. @return mass
  double mass() const;
  //! @brief Transverse momentum of the pair. @return pT
  double pt() const { return momentum().perp(); }
  //! @brief Rapidity of the pair. @return y
  double rapidity() const { return momentum().rapidity(); }
};

//!
//! @brief Filter of jet pairs: all its conditions must hold.
//!
//! Quantities (condition SUBTYPE), DOUBLE_RANGE [minimum, maximum) or DOUBLE_DUAL_RANGE, and INTEGER (the value,
//! MINIMUM) for the ranks: RANK1, RANK2 (the ranks of the two jets: e.g. INTEGER 1 and 2 for the leading and
//! subleading jets), DPHI (|phi1 - phi2| in [0, pi]), ACOPLANARITY (pi - |phi1 - phi2|), DETA (|eta1 - eta2|), PTRATIO
//! (pT2/pT1, e.g. [0.5, 1.01) for a minimum ratio of 0.5), MASS (of the pair), PT (of the pair).
//!
class JetPairFilter
:
public Filter<JetPair>,
public ManagedObject
{
public:

  //! @brief Quantities tested.
  enum JetPairFilterType { kNone, kRank1, kRank2, kDeltaPhi, kAcoplanarity, kDeltaEta, kPtRatio, kMass, kPt };

  //! @brief Filter accepting all pairs.
  JetPairFilter() : Filter<JetPair>(), ManagedObject() { }
  //! @brief Copy constructor. @param source filter
  JetPairFilter(const JetPairFilter & source) : Filter<JetPair>(source), ManagedObject(source) { }
  //! @brief Copy assignment. @param rhs filter @return this filter
  JetPairFilter & operator=(const JetPairFilter & rhs);
  //! @brief Destructor.
  virtual ~JetPairFilter() {}
  //! @brief Whether a pair passes all the conditions. @param pair pair @return true if accepted
  virtual bool accept(const JetPair & pair) const override;
  //! @brief Read the conditions. @param ownerName owner @param typeName JET_PAIR_FILTER @param configuration
  //! configuration @param index index of the filter @throws FilterException for an unknown type or quantity
  virtual void configure(const String & ownerName, const String & typeName, const Configuration & configuration,
                         unsigned int index);
  //! @brief Quantity of a SUBTYPE (kNone if unknown). @param subtype name @return quantity
  static JetPairFilterType typeOf(const String & subtype);
};

//!
//! @brief Task creating the jet pair filters of a job (JET_PAIR_FILTER), see ObjectCreator; a subtask of FilterCreator.
//!
class JetPairFilterCreator : public ObjectCreator<JetPairFilter>
{
public:
  //! @brief Constructor.
  JetPairFilterCreator() : ObjectCreator<JetPairFilter>("JET_PAIR_FILTER")
  { appendClassName("JetPairFilterCreator"); setName("JetPairFilterCreator"); }
  //! @brief Copy. @return new task
  Task * clone() override { return new JetPairFilterCreator(*this); }
};

} // namespace CAP

#endif /* CAP__JetPair */
