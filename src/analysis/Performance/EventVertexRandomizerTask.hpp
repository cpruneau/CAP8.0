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
#ifndef CAP__EventVertexRandomizerTask
#define CAP__EventVertexRandomizerTask
#include "EventProcessor.hpp"

namespace CAP
{

//!
//! @brief Task shifting all the particles of each event by a random vertex position drawn from Gaussian distributions
//! (mean and rms per axis, in configuration units converted with rConversion and tConversion).
//!
class EventVertexRandomizerTask
:
public EventProcessor
{
public:
  
  //! @brief Default constructor.
  //! @brief Default constructor.
  EventVertexRandomizerTask();
  //! @brief Destructor.
  virtual ~EventVertexRandomizerTask() {}
  //! @brief Add the default configuration of the task.
  virtual void setDefaultConfiguration();
  //! @brief Initialize the task.
  virtual void initialize();
  //! @brief Execute the task (process the current event).
  virtual void execute();
  
protected:
  
  //! Conversion factor of the configured positions (default 1e9).
  double rConversion;
  //! Conversion factor of the configured time (default 1e9).
  double tConversion;
  double xAvg; //!< mean vertex position along the x-axis
  double yAvg; //!< mean vertex position along the y-axis
  double zAvg; //!< mean vertex position along the z-axis
  double tAvg; //!< mean vertex position along the t-axis
  double xRms; //!< rms of the vertex distribution along the x-axis
  double yRms; //!< rms of the vertex distribution along the y-axis
  double zRms; //!< rms of the vertex distribution along the z-axis
  double tRms; //!< rms of the vertex distribution along the t-axis
  
  ClassDef(EventVertexRandomizerTask,0)
};

} // namespace CAP

#endif /* CAP__EventVertexRandomizerTask */
