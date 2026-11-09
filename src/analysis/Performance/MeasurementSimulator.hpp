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
#include "MersenneTwister.hpp"

namespace CAP
{
//!
//! @brief Task simulating a measurement: the generator-level event (events()[0]) is turned into a detector-level event
//! (events()[1], by default FastSimEvent), particle by particle. Both events are taken by name from the store (OWNER
//! false): in a RunCAP workflow they are objects of the library, and the event iterator uses both (it resets them
//! before each event); the analyzers of the detector-level particles choose FastSimEvent.
//!
//! For each event accepted by at least one event filter (and by the event efficiency, if any):
//! - the detector-level event is cleared, then given the index, number and projectiles of the generator-level event;
//! - each generator-level particle accepted by at least one particle filter is considered ONCE (a particle accepted by
//!   several filters is not duplicated): it is kept with the probability given by the particle efficiency of the first
//!   filter accepting it (no loss without efficiency), then smeared by the particle smearer of that filter (copied
//!   unchanged without smearer). The detector-level particle keeps the generator-level particle as its truth particle
//!   (Particle::truth()), which PerformanceAnalyzer uses;
//! - the event smearer (if any) is then applied to the whole event.
//!
//! Particle efficiencies and smearers: none, one for all the particle filters, one per particle filter, or one per
//! event filter and particle filter (EventProcessor::objectFor(); the event filter is the first accepting the event). An event
//! rejected by the event filters or the event efficiency gives an empty detector-level event.
//!
//! Random numbers (detection and smearing): from the generator of the task (CAP's MersenneTwister), seeded at
//! initialization with \<name\>:Random:Seed when \<name\>:Random:SetSeed is true (the default), with a seed of its own
//! otherwise. The sub-samples of RunCAP increase the seed.
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
  //! @brief Seed the generator (see the class description).
  virtual void initialize();
  //! @brief Execute the task (process the current event).
  virtual void execute();
protected:
  //! Generator of the detection and smearing.
  MATH::MersenneTwister _random;

};

} // namespace CAP


#endif /* CAP__MeasurementSimulator */
