/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "JetFilter.hpp"
#include "NameHelpers.hpp"

namespace CAP
{

JetFilter::JetFilter()
:
Filter<Jet>(),
ManagedObject()
{ }

JetFilter::JetFilter(const JetFilter & source)
:
Filter<Jet>(source),
ManagedObject(source)
{ }

JetFilter & JetFilter::operator=(const JetFilter & rhs)
{
  if (this!=&rhs)
    {
    Filter<Jet>::operator=(rhs);
    ManagedObject::operator=(rhs);
    }
  return *this;
}

bool JetFilter::accept(const Jet & jet) const
{
  for (Condition * condition : conditions())
    {
    bool accepting = true;
    bool integer = condition->type()==Condition::kConditionInteger || condition->type()==Condition::kConditionIntegerRange;
    switch (condition->subtype())
      {
      case kPt:   accepting = condition->accept(jet.pt());   break;
      case kEta:  accepting = condition->accept(jet.eta());  break;
      case kPhi:  accepting = condition->accept(jet.phi());  break;
      case kMass: accepting = condition->accept(jet.mass()); break;
      case kRank: accepting = integer ? condition->accept(int(jet.rank)) : condition->accept(double(jet.rank)); break;
      case kMult: accepting = integer ? condition->accept(int(jet.multiplicity()))
                                      : condition->accept(double(jet.multiplicity())); break;
      default: break;
      }
    if (!accepting) return false;
    }
  return true;
}

JetFilter::JetFilterType JetFilter::typeOf(const String & subtype)
{
  if (subtype=="PT")   return kPt;
  if (subtype=="ETA")  return kEta;
  if (subtype=="PHI")  return kPhi;
  if (subtype=="RANK") return kRank;
  if (subtype=="MULT") return kMult;
  if (subtype=="MASS") return kMass;
  return kNone;
}

void JetFilter::configure(const String & ownerName, const String & typeName, const Configuration & configuration,
                          unsigned int index)
{
  int nConditions = configuration.valueInt(createKey(ownerName,typeName,index,"CONDITION:N"));
  for (int i=0; i<nConditions; i++)
    {
    auto key = [&](const char * name) { return createKey(ownerName,typeName,index,"CONDITION",i,name); };
    String type    = configuration.valueString(key("TYPE"));
    String subtype = configuration.valueString(key("SUBTYPE"));
    double minimum = configuration.valueDouble(key("MINIMUM"));
    double maximum = configuration.valueDouble(key("MAXIMUM"));
    JetFilterType quantity = typeOf(subtype);
    if (quantity==kNone) throw FilterException(String("Unknown jet quantity ")+subtype,__FUNCTION__);
    if (type.equalTo("DOUBLE_RANGE"))
      addCondition(int(quantity),minimum,maximum);
    else if (type.equalTo("DOUBLE_DUAL_RANGE"))
      addCondition(int(quantity),minimum,maximum,configuration.valueDouble(key("MINIMUM2")),
                   configuration.valueDouble(key("MAXIMUM2")));
    else if (type.equalTo("INTEGER") && (quantity==kRank || quantity==kMult))
      addCondition(int(quantity),int(minimum));
    else
      throw FilterException(String("Condition ")+type+" not accepted for the jet quantity "+subtype,__FUNCTION__);
    }
}

} // namespace CAP
