/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__EventMerger
#define CAP__EventMerger
#include "EventProcessor.hpp"

namespace CAP
{

//!
//! @brief Task merging events from several input streams into one event.
//!
//! Each subtask is an input stream (e.g., an event generator). For each call of execute(), the particle factory is
//! reset and subtask k is executed \<name\>:INPUT_STREAM_\<k\>:MULT times, so that the event receives the particles of
//! that many events of stream k (e.g., signal plus pile-up).
//!
class EventMerger
:
public EventProcessor
{
public:
  
  //! @brief Default constructor.
  EventMerger();
  //! @brief Copy constructor. @param task task to copy
  EventMerger(const EventMerger & task);
  //! @brief Copy assignment. @param task task to copy @return copy
  EventMerger operator=(const EventMerger & task);
  //! @brief Destructor.
  virtual ~EventMerger() {}
  //! @brief Add the default configuration (that of EventProcessor).
  virtual void setDefaultConfiguration();
  //! @brief Configure the task: read the multiplicity of each input stream (\<name\>:INPUT_STREAM_\<k\>:MULT).
  //! @param configuration configuration
  virtual void configure(const Configuration & configuration);
  //! @brief Reset the particle factory and execute each input stream the configured number of times.
  virtual void execute();

  //! @brief Number of events taken from each input stream per merged event. @return numbers of events
  const std::vector<unsigned int> & nEventsPerStream() { return _nEventsPerStream; }

protected:

  //! Number of events taken from each input stream (subtask) per merged event.
  std::vector<unsigned int> _nEventsPerStream;

};

}
#endif /* CAP__EventMerger */
