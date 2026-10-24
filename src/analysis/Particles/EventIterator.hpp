/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__EventIterator
#define CAP__EventIterator
#include "EventProcessor.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Task running its subtasks on a sequence of events (event loop).
//!
//! This class implements a task iterator task, i.e., a task that repeated calls other tasks (subtasks) to carry out the same operation(s) on a sequence of events (event stream).
//!  Use the 'run' method to execute the job on all selected events. Internally, 'run' calls the 'execute' method repeatedly on each event in the event stream.
//!  Do not call the 'execute' method of this class directly unless you know what you are doing..
//!
//!  Optionally, the iterator may call a subsampleAnalysis method to carry out a sub-sample analysis of all the sub tasks operated by this iterator.
//!
class EventIterator
:
public EventProcessor
{
public:
  
  //! @brief Default constructor.
  EventIterator();
  //! @brief Copy constructor. @param task task to copy
  EventIterator(const EventIterator & task);
  //! @brief Copy assignment. @param task task to copy @return copy
  EventIterator operator=(const EventIterator & task);
  //! @brief Destructor.
  virtual ~EventIterator() {}
  //! @brief Add the default configuration: \<name\>:EVENT:REQUESTED:N (number of events) and \<name\>:EVENT:REPORT:N
  //! (report period, in events).
  virtual void setDefaultConfiguration();
  //! @brief Configure the task (number of events and report period). @param configuration configuration
  virtual void configure(const Configuration & configuration);
  //! @brief Run the event loop: for each requested event, execute the subtasks, and report progress periodically.
  virtual void execute();
  //! @brief Number of events to process. @return number of events
  long nEventsRequested() const { return _nEventsRequested; }
  //! @brief Report period (a progress message is printed every nEventsReport() events). @return period
  long nEventsReport() const { return _nEventsReport; }

protected:

  //! Number of events to process.
  long    _nEventsRequested;
  //! Report period, in events.
  long    _nEventsReport;

  ClassDef(EventIterator,0)
};

}
#endif /* CAP__EventIterator */
