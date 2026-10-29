/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
#ifndef CAP__PtPtDerivedCalculator
#define CAP__PtPtDerivedCalculator
#include "EventProcessorHistos.hpp"
#include "PtPtHistos.hpp"

namespace CAP
{

//!
//! @brief Derived histograms of a PtPt analysis (second phase, no event processing): reads the histograms
//! filled by a PtPtAnalyzer from the input file of the job, computes the factorial cumulants F1-F4, the correlators R2-R4 and the pT moments (PtPtHistos::calculateDerived()) and writes the
//! completed folders to the output file of the job.
//!
//! It uses the same filters and histogram settings as the analyzer, and reads and writes the folders of the analyzer:
//! those of the folder named by the key \<name\>:SOURCE_TASK (the name of the analyzer task).
//!
class PtPtDerivedCalculator
:
public EventProcessorHistos<PtPtHistos,1>
{
public:

  //! @brief Default constructor.
  PtPtDerivedCalculator();
  //! @brief Copy constructor. @param source task to copy
  PtPtDerivedCalculator(const PtPtDerivedCalculator & source);
  //! @brief Copy assignment. @param rhs task to copy @return this task
  PtPtDerivedCalculator & operator=(const PtPtDerivedCalculator & rhs);
  //! @brief Destructor.
  virtual ~PtPtDerivedCalculator() {}
  //! @brief Add the default configuration: that of EventProcessorHistos, the histogram settings and SOURCE_TASK.
  virtual void setDefaultConfiguration() override;
  //! @brief Configure the task (and read SOURCE_TASK). @param configuration configuration
  virtual void configure(const Configuration & configuration) override;
  //!
  //! @brief Compute the derived histograms of every folder from its base histograms.
  //! @throws Exception if there are fewer folders than combinations of filters
  //!
  virtual void execute() override;
  //! @brief Name of the folder of the job files holding the histograms: that of the analyzer (SOURCE_TASK). @return name
  virtual String histogramTaskName() const override { return _sourceTask; }

protected:

  //! Name of the analyzer task whose histograms are completed.
  String _sourceTask;

};

} // namespace CAP

#endif /* CAP__PtPtDerivedCalculator */
