/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ClosureCalculator
#define CAP__ClosureCalculator
#include "Task.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Task computing a closure test: the difference (METHOD 0) or ratio (METHOD 1) of the histograms of a detector
//! level (reconstructed) analysis and of the generator level analysis, histogram by histogram.
//!
//! Keys: \<name\>:HISTOGRAM:METHOD, IMPORT:PATH, IMPORT:GEN_FILE_NAME, IMPORT:REC_FILE_NAME, EXPORT:PATH,
//! EXPORT:CLOSURE_FILE_NAME.
//!
class ClosureCalculator
:
public Task
{
  
public:
  
  //! @brief Default constructor.
  //! @brief Default constructor.
  ClosureCalculator();
  //! @brief Destructor.
  virtual ~ClosureCalculator() {}
  //! @brief Add the default configuration of the task.
  virtual void setDefaultConfiguration();
  //! @brief Execute the task (process the current event).
  virtual void execute();

  ClassDef(ClosureCalculator,0)
};

} // namespace CAP

#endif /* CAP__ClosureCalculator */
