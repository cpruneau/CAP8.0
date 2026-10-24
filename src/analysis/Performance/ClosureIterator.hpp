/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ClosureIterator
#define CAP__ClosureIterator
#include "Task.hpp"

namespace CAP
{

//!
//! @brief Task running closure tests (see ClosureCalculator) on all the generator-level files of a directory: for each
//! file *_Gen*, the detector-level file *_Reco* is compared and the result written to *_Closure*.
//!
//! Files are selected in \<name\>:HISTOGRAM:IMPORT:PATH with the patterns \<name\>:HISTOGRAM:IncludedPattern_k and
//! ExcludedPattern_k (files containing "Reco" are excluded).
//!Task performs a closure test
//!
class ClosureIterator
:
public Task
{
public:
  //! @brief Default constructor.
  //! @brief Default constructor.
  ClosureIterator();
  //! @brief Destructor.
  virtual ~ClosureIterator() {}
  //! @brief Add the default configuration of the task.
  virtual void setDefaultConfiguration();
  //! @brief Execute the task (process the current event).
  virtual void execute();
};

} // namespace CAP

#endif /* CAP__ClosureIterator */
