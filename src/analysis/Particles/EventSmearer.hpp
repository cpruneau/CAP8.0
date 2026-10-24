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
#ifndef CAP__EventSmearer
#define CAP__EventSmearer
#include "HistogramFolder.hpp"
#include "Event.hpp"
#include "MathConstants.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Smearer of events: produces a smeared copy of an event (e.g., to simulate detector effects on global
//! quantities). The base implementation of smear() does nothing; the histograms it may use are held as a
//! HistogramFolder.
//!
class EventSmearer : public HistogramFolder
{
public:
  //! @brief Default constructor.
  EventSmearer();
  //! @brief Copy constructor. @param group smearer to copy
  EventSmearer(const EventSmearer & group);
  //! @brief Copy assignment. @param group smearer to copy @return this smearer
  EventSmearer & operator=(const EventSmearer & group);
  //! @brief Destructor.
  virtual ~EventSmearer(){ }
  //! @brief Configure the smearer (see HistogramFolder::configure()).
  //! @param ownerName name of the owner (task) @param typeName object type (EVENT_SMEARER)
  //! @param configuration configuration @param index index of the smearer
  virtual void configure(const String & ownerName,
                               const String & typeName,
                               const Configuration & configuration,
                               unsigned int index);
  //! @brief Fill target with the smeared version of source (nothing in the base class). @param source event @param target smeared event
  virtual void smear(const Event & source, Event & target);

protected:

  //! @brief Copy the content of another smearer. @param source smearer to copy
  virtual void cloneB(const EventSmearer & source);

  ClassDef(EventSmearer,0)

};

} // namespace CAP

#endif /* CAP__EventSmearer  */



