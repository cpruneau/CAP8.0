/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "JetPair.hpp"
#include <cmath>
#include "NameHelpers.hpp"

namespace CAP
{

double JetPair::deltaPhi() const
{
  double d = CAP::deltaPhi(first->phi(),second->phi());     // [-pi, pi)
  return d<-0.5*M_PI ? d+2.0*M_PI : d;                       // [-pi/2, 3 pi/2)
}

double JetPair::absDeltaPhi() const
{
  return std::fabs(CAP::deltaPhi(first->phi(),second->phi()));
}

double JetPair::acoplanarity() const
{
  return M_PI-absDeltaPhi();
}

double JetPair::asymmetry() const
{
  double sum = first->pt()+second->pt();
  return sum>0.0 ? (first->pt()-second->pt())/sum : 0.0;
}

MATH::VectorLorentz JetPair::momentum() const
{
  const MATH::VectorLorentz & a = first->momentum, & b = second->momentum;
  MATH::VectorLorentz p;
  p.setTXYZ(a.t()+b.t(),a.x()+b.x(),a.y()+b.y(),a.z()+b.z());
  return p;
}

double JetPair::mass() const
{
  MATH::VectorLorentz p = momentum();
  double m2 = p.t()*p.t()-p.x()*p.x()-p.y()*p.y()-p.z()*p.z();
  return m2>0.0 ? std::sqrt(m2) : 0.0;
}

JetPairFilter & JetPairFilter::operator=(const JetPairFilter & rhs)
{
  if (this!=&rhs)
    {
    Filter<JetPair>::operator=(rhs);
    ManagedObject::operator=(rhs);
    }
  return *this;
}

bool JetPairFilter::accept(const JetPair & pair) const
{
  for (Condition * condition : conditions())
    {
    bool integer = condition->type()==Condition::kConditionInteger || condition->type()==Condition::kConditionIntegerRange;
    bool accepting = true;
    switch (condition->subtype())
      {
      case kRank1: accepting = integer ? condition->accept(int(pair.first->rank)) : condition->accept(double(pair.first->rank)); break;
      case kRank2: accepting = integer ? condition->accept(int(pair.second->rank)) : condition->accept(double(pair.second->rank)); break;
      case kDeltaPhi:     accepting = condition->accept(pair.absDeltaPhi()); break;
      case kAcoplanarity: accepting = condition->accept(pair.acoplanarity()); break;
      case kDeltaEta:     accepting = condition->accept(std::fabs(pair.deltaEta())); break;
      case kPtRatio:      accepting = condition->accept(pair.ptRatio()); break;
      case kMass:         accepting = condition->accept(pair.mass()); break;
      case kPt:           accepting = condition->accept(pair.pt()); break;
      default: break;
      }
    if (!accepting) return false;
    }
  return true;
}

JetPairFilter::JetPairFilterType JetPairFilter::typeOf(const String & subtype)
{
  if (subtype=="RANK1")        return kRank1;
  if (subtype=="RANK2")        return kRank2;
  if (subtype=="DPHI")         return kDeltaPhi;
  if (subtype=="ACOPLANARITY") return kAcoplanarity;
  if (subtype=="DETA")         return kDeltaEta;
  if (subtype=="PTRATIO")      return kPtRatio;
  if (subtype=="MASS")         return kMass;
  if (subtype=="PT")           return kPt;
  return kNone;
}

void JetPairFilter::configure(const String & ownerName, const String & typeName, const Configuration & configuration,
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
    JetPairFilterType quantity = typeOf(subtype);
    if (quantity==kNone) throw FilterException(String("Unknown jet pair quantity ")+subtype,__FUNCTION__);
    if (type.equalTo("DOUBLE_RANGE"))
      addCondition(int(quantity),minimum,maximum);
    else if (type.equalTo("DOUBLE_DUAL_RANGE"))
      addCondition(int(quantity),minimum,maximum,configuration.valueDouble(key("MINIMUM2")),
                   configuration.valueDouble(key("MAXIMUM2")));
    else if (type.equalTo("INTEGER") && (quantity==kRank1 || quantity==kRank2))
      addCondition(int(quantity),int(minimum));
    else
      throw FilterException(String("Condition ")+type+" not accepted for the jet pair quantity "+subtype,__FUNCTION__);
    }
}

} // namespace CAP
