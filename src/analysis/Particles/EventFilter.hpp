/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 
 * *********************************************************************/
#ifndef CAP__EventFilter
#define CAP__EventFilter
#include "Event.hpp"
#include "Filter.hpp"
#include "ManagedObject.hpp"

namespace CAP
{
  //!
  //! @brief Filter selecting events by global quantities of particle selections: multiplicity, energy, net charge, net
  //! strangeness, net baryon number, pT sum or spherocity of the particles accepted by a particle filter.
  //!
  //! An event is accepted when it satisfies all the conditions (none: all events accepted). The analysis tasks fill
  //! their histograms separately for each event filter (see EventProcessor::filterEvent()).
  //!
  //! Configuration (read by configure()), for filter \<index\> of owner \<owner\>:
  //! @code
  //!   <owner>:EVENT_FILTER_<index>:CONDITION:N = 1
  //!   <owner>:EVENT_FILTER_<index>:CONDITION_0:TYPE = DOUBLE_RANGE
  //!   <owner>:EVENT_FILTER_<index>:CONDITION_0:SUBTYPE = MULT        // MULT, ENERGY, CHARGE, STRANGENESS, BARYON,
  //!                                                                  // PTSUM, SPHERO
  //!   <owner>:EVENT_FILTER_<index>:CONDITION_0:SELECTION = Charged   // name of the particle filter
  //!   <owner>:EVENT_FILTER_<index>:CONDITION_0:MINIMUM = 0
  //!   <owner>:EVENT_FILTER_<index>:CONDITION_0:MAXIMUM = 100
  //! @endcode
  //! The quantity is that of the selection of the particle filter named SELECTION (a filter of the store of the
  //! ManagedObjects, e.g. made by FilterCreator) in the event, made once per event and shared with the tasks using the
  //! filter (Event::selection()): it does not depend on the order of the tasks. The filter is found at the first event.
  //!
  //! Earlier versions: the subtypes MULT_k, ENERGY_k, SPHERO_k (k=0..5) test the quantities set in the event by
  //! GlobalAnalyzer for its particle filter k (Event::mult(k), ...), which must run before the tasks using the filter.
  //!
  class EventFilter
  :
  public Filter<Event>,
  public ManagedObject
  {
  public:
  //! @brief Default constructor: no conditions (all events accepted).
  EventFilter();
  //! @brief Copy constructor (the conditions are cloned). @param source filter to copy
  EventFilter(const EventFilter & source);
  //! @brief Copy assignment (the conditions are cloned). @param source filter to copy @return this filter
  EventFilter & operator=(const EventFilter & source);
  //! @brief Destructor.
  virtual ~EventFilter();
  //! @brief Whether an event satisfies all the conditions. @param event event @return true if accepted
  virtual bool accept(const Event & event) const;
  //! @brief Read the conditions from a configuration (see the class description).
  //! @param ownerName name of the owner (task) @param typeName object type (EVENT_FILTER)
  //! @param configuration configuration @param index index of the filter
  virtual void configure(const String & ownerName,
                         const String & typeName,
                         const Configuration & configuration,
                         unsigned int index);
  
  //! @brief Quantities a condition can test (k = index of the particle selection used to compute them).
  enum EventFilterType
  { kAll,
    kMult0, kMult1, kMult2, kMult3, kMult4, kMult5,
    kEnergy0,kEnergy1,kEnergy2,kEnergy3,kEnergy4,kEnergy5,
    kCharge0,kCharge1,kCharge2,kCharge3,kCharge4,kCharge5,
    kStrange0,kStrange1,kStrange2,kStrange3,kStrange4,kStrange5,
    kBaryon0,kBaryon1,kBaryon2,kBaryon3,kBaryon4,kBaryon5,
    kSphero0,kSphero1,kSphero2,kSphero3,kSphero4,kSphero5,
    kMult, kEnergy, kCharge, kStrangeness, kBaryon, kPtSum, kSphero   // of the selection of a named particle filter
  };

  //! @brief Subtype of a name (MULT_0, ..., MULT, ENERGY, ...). @param name name @return subtype, -1 if unknown
  static int subtypeOf(const String & name);
  //! @brief Whether a subtype tests the selection of a named particle filter. @param subtype subtype
  //! @return true for kMult ... kSphero
  static bool isSelectionSubtype(int subtype) { return subtype >= kMult && subtype <= kSphero; }
  //! @brief Name of the particle filter of a condition (empty for the subtypes of earlier versions).
  //! @param index condition @return name
  const String & selectionName(unsigned int index) const { return _selectionNames[index]; }

  protected:

  //! @brief Value of the quantity of a condition in an event. @param index condition @param event event @return value
  double value(unsigned int index, const Event & event) const;

  //! Name of the particle filter of each condition (empty: subtype of earlier versions).
  std::vector<String> _selectionNames;
  //! Slot of the particle filter of each condition (-1 until found).
  mutable std::vector<int> _selectionSlots;
  
  };
  
} // namespace CAP

#endif /* CAP__EventFilter */
