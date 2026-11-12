/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "EventFilter.hpp"
#include <string>
#include "ManagedObjects.hpp"
#include "ParticleFilter.hpp"
#include "ParticleSelection.hpp"
#include "Event.hpp"

namespace CAP
{
  
  EventFilter::EventFilter()
  :
  Filter<Event>(),
  ManagedObject(),
  _selectionNames(),
  _selectionSlots()
  { /* no ops */}

  EventFilter::EventFilter(const EventFilter & source)
  :
  Filter<Event>(source),
  ManagedObject(source),
  _selectionNames(source._selectionNames),
  _selectionSlots(source._selectionSlots)
  { /* no ops */}

  EventFilter & EventFilter::operator=(const EventFilter & source)
  {
  if (this!=&source)
    {
    Filter<Event>::operator=(source);
    _selectionNames = source._selectionNames;
    _selectionSlots = source._selectionSlots;
    }
  return *this;
  }

  EventFilter::~EventFilter()
  { /* no ops */}

  int EventFilter::subtypeOf(const String & name)
  {
  static const char * selection[] = { "MULT", "ENERGY", "CHARGE", "STRANGENESS", "BARYON", "PTSUM", "SPHERO" };
  for (int k=0; k<7; k++) if (name.equalTo(selection[k])) return kMult + k;
  static const char * legacy[] = { "MULT_", "ENERGY_", "SPHERO_" };
  static const int first[] = { kMult0, kEnergy0, kSphero0 };
  for (int k=0; k<3; k++)
    for (int i=0; i<6; i++)
      if (name.equalTo(String(legacy[k]) + String(std::to_string(i)))) return first[k] + i;
  return -1;
  }

  double EventFilter::value(unsigned int index, const Event & event) const
  {
  int subtype = _conditions[index]->subtype();
  if (isSelectionSubtype(subtype))
    {
    if (_selectionSlots[index] < 0)
      {
      const ParticleFilter * filter = nullptr;
      for (auto f : ManagedObjects<ParticleFilter>::getObjectStore())
        if (f->isNamed(_selectionNames[index])) { filter = f; break; }
      if (!filter)
        throw FilterException(String("Event filter ") + name() + String(": no particle filter named ")
                              + _selectionNames[index],__FUNCTION__);
      _selectionSlots[index] = int(ParticleSelection::slot(filter));
      }
    const ParticleSelection & s = event.selection((unsigned int) _selectionSlots[index]);
    switch (subtype)
      {
      case kMult:        return s.multiplicity();
      case kEnergy:      return s.energy();
      case kCharge:      return s.charge();
      case kStrangeness: return s.strangeness();
      case kBaryon:      return s.baryon();
      case kPtSum:       return s.ptSum();
      default:           return s.spherocity();
      }
    }
  if (subtype >= kMult0 && subtype <= kMult5)     return event.mult(subtype-kMult0);
  if (subtype >= kEnergy0 && subtype <= kEnergy5) return event.energy(subtype-kEnergy0);
  if (subtype >= kSphero0 && subtype <= kSphero5) return event.spherocity(subtype-kSphero0);
  throw FilterException(String("unknown subtype ") + String(std::to_string(subtype)),__FUNCTION__);
  }

  bool EventFilter::accept(const Event & event)  const
  {
  for (unsigned int k=0; k<_conditions.size(); k++)
    if (!_conditions[k]->accept(value(k,event))) return false;
  return true;
  }

  void EventFilter::configure(const String & ownerName,
                              const String & typeName,
                              const Configuration & configuration,
                              unsigned int index)
  {
  unsigned int nConditions = configuration.valueInt(createKey(ownerName,typeName,index,"CONDITION:N"));
  for (unsigned int iCondition=0; iCondition<nConditions; iCondition++)
    {
    String type     = configuration.valueString(createKey(ownerName,typeName,index,"CONDITION",iCondition,"TYPE"));
    String subtype  = configuration.valueString(createKey(ownerName,typeName,index,"CONDITION",iCondition,"SUBTYPE"));
    double minimum  = configuration.valueDouble(createKey(ownerName,typeName,index,"CONDITION",iCondition,"MINIMUM"));
    double maximum  = configuration.valueDouble(createKey(ownerName,typeName,index,"CONDITION",iCondition,"MAXIMUM"));
    if (!type.equalTo("DOUBLE_RANGE") && !type.equalTo("DOUBLE_DUAL_RANGE"))
      throw FilterException(type,__FUNCTION__);
    int subtypeIndex = subtypeOf(subtype);
    if (subtypeIndex < 0) throw FilterException(subtype,__FUNCTION__);
    String selectionName;
    if (isSelectionSubtype(subtypeIndex))
      {
      String key = createKey(ownerName,typeName,index,"CONDITION",iCondition,"SELECTION");
      selectionName = configuration.valueString(key,true,"NONE");
      if (selectionName.isEmpty() || selectionName=="NONE")
        throw FilterException(key + String(": the condition ") + subtype + String(" needs the name of a particle filter"),
                              __FUNCTION__);
      }
    addCondition(subtypeIndex,minimum,maximum);
    _selectionNames.push_back(selectionName);
    _selectionSlots.push_back(-1);
    }
  }

} // namespace CAP
