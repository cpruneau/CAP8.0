/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__RunSubSampleSum
#define CAP__RunSubSampleSum
#include "String.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Helper running a SubSampleStatCalculator on the sub-sample directories of a path.
//! The task is named SubSample and configured with SubSample:HISTOGRAM:IMPORT:PATH, IMPORT:FILE_NAME, EXPORT:PATH,
//! EXPORT:FILE_NAME and SubSample:TASK:SEVERITY.
//!
class RunSubSampleSum 
{
public:

  //! @brief Default constructor.
  RunSubSampleSum();
  //! @brief Destructor.
  virtual ~RunSubSampleSum();

  //! @brief Combine the sub-samples. @param histogramImportPathName directory of the sub-sample subdirectories
  //! @param histogramImportFileName file name of each sub-sample @param histogramExportPathName output directory
  //! @param histogramExportFileName output file name @param verbose print what is done
  void run(String & histogramImportPathName,
           String & histogramImportFileName,
           String & histogramExportPathName,
           String & histogramExportFileName,
           bool verbose=true);

  ClassDef(RunSubSampleSum,0)
};

}    // namespace CAP

#endif /* CAP_RunSubSampleSum */


