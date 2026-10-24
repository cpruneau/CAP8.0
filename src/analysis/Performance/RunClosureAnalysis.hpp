/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__RunClosure
#define CAP__RunClosure
#include "String.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Helper running a ClosureCalculator on a pair of generator and detector level files.
//!
class RunClosureAnalysis
{
public:

  //! @brief Default constructor.
  //! @brief Default constructor.
  RunClosureAnalysis();
  //! @brief Destructor.
  //! @brief Destructor.
  virtual ~RunClosureAnalysis();

  //! @brief Run the closure test. @param histogramImportPathName input directory
  //! @param histogramImportGenFileName generator-level file @param histogramImportRecoFileName detector-level file
  //! @param histogramExportPathName output directory @param histogramExportFileName output file @param verbose print what is done
  void run(String & histogramImportPathName,
           String & histogramImportGenFileName,
           String & histogramImportRecoFileName,
           String & histogramExportPathName,
           String & histogramExportFileName,
           bool verbose=true);

  ClassDef(RunClosureAnalysis,0)
};

}    // namespace CAP

#endif /* CAP_RunClosure */


