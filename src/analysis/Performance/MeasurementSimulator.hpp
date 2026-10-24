/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__MeasurementSimulator
#define CAP__MeasurementSimulator
#include "EventProcessor.hpp"

namespace CAP
{
//!
//! @brief Task simulating a measurement: the particles of the generator-level event (events()[0]) accepted by the
//! filters are smeared (ParticleSmearer of each event filter and particle filter combination) and added to the
//! detector-level event (events()[1]).
//!
class MeasurementSimulator
:
public EventProcessor
{
public:
  
  //! @brief Default constructor.
  MeasurementSimulator();
  //! @brief Copy constructor. @param source object to copy
  MeasurementSimulator(const MeasurementSimulator & source);
  //! @brief Copy assignment. @param rhs object to copy @return this object
  MeasurementSimulator & operator=(const MeasurementSimulator & rhs);
  //! @brief Destructor.
  virtual ~MeasurementSimulator() {}
  //! @brief Add the default configuration of the task.
  virtual void setDefaultConfiguration();
  //! @brief Execute the task (process the current event).
  virtual void execute();

};

} // namespace CAP


#endif /* CAP__MeasurementSimulator */
