/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__SubSampleStatCalculator
#define CAP__SubSampleStatCalculator
#include "Task.hpp"
#include "ClassDef.hpp"

using namespace std;

namespace CAP
{

//!
//! @brief Task combining the histograms of several sub-samples: average and variance (uncertainty) of each histogram.
//!
//! Each subdirectory of \<name\>:HISTOGRAM:IMPORT:PATH holds the file \<name\>:HISTOGRAM:IMPORT:FILE_NAME of one
//! sub-sample. The sub-samples are averaged, weighted by their numbers of executed tasks (read from their accountants;
//! 1 if absent), with their spread as uncertainty; the result and the summed accountants are written to
//! \<name\>:HISTOGRAM:EXPORT:PATH/EXPORT:FILE_NAME.
//!
class SubSampleStatCalculator
:
public Task
{
public:
  
  //! @brief Default constructor.
  SubSampleStatCalculator();
  //! @brief Copy constructor. @param src task to copy
  SubSampleStatCalculator(const SubSampleStatCalculator & src);
  //! @brief Copy assignment. @param rhs task to copy @return this task
  SubSampleStatCalculator & operator=(const SubSampleStatCalculator & rhs);
  //! @brief Destructor.
  virtual ~SubSampleStatCalculator() {}
  //! @brief Add the default configuration: <name>:HISTOGRAM:IMPORT:PATH, IMPORT:FILE_NAME, EXPORT:PATH, EXPORT:FILE_NAME.
  virtual void setDefaultConfiguration();
  //! @brief Combine the sub-samples (see the class description). @throws Exception for fewer than 2 sub-samples
  virtual void execute();

protected:

  //! Sum of the numbers of executed tasks of the sub-samples.
  Accountant _totalTaskExecuted;
  //! Numbers of accepted events of the current sub-sample.
  Accountant _eventAccepted;
  //! Sum of the numbers of accepted events of the sub-samples.
  Accountant _totalEventAccepted;
  //! Directory holding one subdirectory per sub-sample.
  String _importPath;
  //! Name of the file of each sub-sample.
  String _importFile;
  //! Output directory.
  String _exportPath;
  //! Output file name.
  String _exportFile;

  ClassDef(SubSampleStatCalculator,0)
};

} // namespace CAP

#endif /* CAP__SubSampleStatCalculator */
