/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__FileIterator
#define CAP__FileIterator
#include "EventProcessor.hpp"

namespace CAP
{

  //!
  //! @brief Task post-processing histograms read from the input file of the job, e.g. computing the derived histograms
  //! of the analyzers it holds from the histograms of an earlier run (no event loop).
  //!
  //! The job reads the histograms its subtasks import (IMPORT) from its input file after initialization (see
  //! Task::loadHistograms()); execute() then calls postProcess() of each subtask (EventProcessor::postProcess() computes
  //! the derived histograms); the job writes them, with those imported, to its output file.
  //!
  class FileIterator
  :
  public EventProcessor
  {
  public:

  //! @brief Default constructor.
  FileIterator();
  //! @brief Copy constructor. @param task task to copy
  FileIterator(const FileIterator & task);
  //! @brief Copy assignment. @param task task to copy @return copy
  FileIterator operator=(const FileIterator & task);
  //! @brief Destructor.
  virtual ~FileIterator() {}
  //! @brief Post-process the subtasks (see the class description).
  virtual void execute();

  };

} // namespace CAP

#endif /* CAP__FileIterator */
