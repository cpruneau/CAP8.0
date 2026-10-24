/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__CalibrationProducer
#define CAP__CalibrationProducer
#include "HistogramTask.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Task producing particle efficiency histograms from the ratios of reconstructed to generated single-particle
//! yields of a closure test.
//! @note Not part of the build (not listed in Performance/CMakeLists.txt).
//!
//! Task loads ratio histograms of single particle yields obtain for reconstructed events and generator level events (from a closure test w/o corrections) and
//! produces a new file root file containing efficiency histograms. Nominally, there will be one histogram for each collision class present in the input file.
//!  The name of the histograms should contain the event filter class they were calculated for and the specific single particle histogram
//! used for (track) efficiency correction. Efficiency corrections can be done based on
//! - Case 0: pt spectra only
//! - Case 1: pt vs. eta spectra
//! - Case 2: pt vs. y (rapidity) spectra
//! - Case 3: pt vs phi vs eta spectra
//! - Case 3: pt vs phi vs y (rapidity)  spectra
//! based on the integer value of the configuration variable "efficiencyOpt".
//!
//!  The name of the histograms will be changed according to the following pattern:
//!    Part_Ratio_EFILTER_n1_pt  --> Part_Efficiency_EFILTER_pt
//!   where EFILTER is the name of the event filter used to produce the ratio histogram.
//!
//!  This task reads data from a specific  input path and file and outputs to a specific output path and distinct file file name auto-generated based
//!  on the name of the input file or as given as input parameter.name
//!
class CalibrationProducer
:
public Task,

public ManagedList<Event>, public Analyzer 


{
  
public:
  
  //! @brief Default constructor.
  CalibrationProducer();
  //! @brief Destructor.
  virtual ~CalibrationProducer() {}
  //! @brief Add the default configuration of the task.
  virtual void setDefaultConfiguration();
  //! @brief Configure the task. @param configuration configuration
  virtual void configure(const Configuration & configuration);
  //! @brief Initialize the task.
  virtual void initialize();
  //! @brief Execute the task (process the current event).
  virtual void execute();
  
  ClassDef(CalibrationProducer,0)
};

} // namespace CAP

#endif /* CAP__CalibrationProducer */
