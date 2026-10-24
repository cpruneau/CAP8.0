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
#ifndef CAP__EventPlaneRandomizerTask
#define CAP__EventPlaneRandomizerTask
#include "EventProcessor.hpp"
#include "ClassDef.hpp"

namespace CAP
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
  ClassDef(EventPlaneRandomizerTask,0)
};

} // namespace CAP

#endif /* CAP__EventPlaneRandomizerTask */
