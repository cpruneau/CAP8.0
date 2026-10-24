/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "FileIterator.hpp"
#include "PrintHelpers.hpp"
#include "Exceptions.hpp"


namespace CAP
{

  FileIterator::FileIterator()
  :
  EventProcessor()
  {
  appendClassName("FileIterator");
  setMinimumReportLevel(Object::kInfo);
  setName("FileIterator");
  setTitle("FileIterator");
  }

  FileIterator::FileIterator(const FileIterator & source)
  :
  EventProcessor(source)
  {    }

  FileIterator FileIterator::operator=(const FileIterator & rhs)
  {
  if (this!=&rhs) EventProcessor::operator=(rhs);
  return *this;
  }

  void FileIterator::execute()
  {
  if (reportDebug(__FUNCTION__))  { printCR(); };
  _timer.start();
  _taskExecuted.increment();
  for (auto & subtask : _subTasks)
    {
    if (reportDebug(__FUNCTION__)) printValue("SubtaskName",subtask->name());
    subtask->postProcess();
    }
  _timer.stop();
  printCR();
  printLine();
  _timer.print();
  printLine();
  }

} // namespace CAP
