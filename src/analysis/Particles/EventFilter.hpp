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
#ifndef CAP__EventFilter
#define CAP__EventFilter
#include "Event.hpp"
#include "Filter.hpp"
#include "ManagedObject.hpp"
#include "ClassDef.hpp"

namespace CAP
{
  //!
  //! @brief Filter selecting events by global quantities: multiplicity, energy or spherocity (each computed for up to 6
  //! particle selections, see Event::mult(), Event::energy(), Event::spherocity()).
  //!
  //! An event is accepted when it satisfies all the conditions (none: all events accepted). The analysis tasks fill
  //! their histograms separately for each event filter (see EventProcessor::filterEvent()).
  //!
  //! Configuration (read by configure()), for filter \<index\> of owner \<owner\>:
  //! @code
  //!   <owner>:EVENT_FILTER_<index>:CONDITION:N = 1
  //!   <owner>:EVENT_FILTER_<index>:CONDITION_0:TYPE = DOUBLE_RANGE
  //!   <owner>:EVENT_FILTER_<index>:CONDITION_0:SUBTYPE = MULT_0     // MULT_k, ENERGY_k, SPHERO_k (k=0..5)
  //!   <owner>:EVENT_FILTER_<index>:CONDITION_0:MINIMUM = 0
  //!   <owner>:EVENT_FILTER_<index>:CONDITION_0:MAXIMUM = 100
  //! @endcode
  //! @note The charge, strangeness and baryon subtypes are declared but not tested by accept().
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
    kSphero0,kSphero1,kSphero2,kSphero3,kSphero4,kSphero5
  };
  
  ClassDef(EventFilter,0)
  };
  
} // namespace CAP

#endif /* CAP__EventFilter */
