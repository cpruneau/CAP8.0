/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef DATUM__EventPlaneRandomizerTask
#define DATUM__EventPlaneRandomizerTask
#include "EventProcessor.hpp"

namespace Datum
{

//!
//! @brief Task rotating each event by a random azimuthal angle (uniform in [0, 2pi)): momenta and positions of all
//! particles are rotated about the z axis.
//!
class EventPlaneRandomizerTask
:
public EventProcessor
{
public:
  //! @brief Default constructor.
  //! @brief Default constructor.
  EventPlaneRandomizerTask();
  //! @brief Destructor.
  virtual ~EventPlaneRandomizerTask() {}
  //! @brief Execute the task (process the current event).
  virtual void execute();
};

} // namespace Datum

#endif /* DATUM__EventPlaneRandomizerTask */
