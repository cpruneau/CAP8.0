/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
#ifndef CAP__NuDynDerivedCalculator
#define CAP__NuDynDerivedCalculator
#include "EventProcessorHistos.hpp"
#include "NuDynHistos.hpp"

namespace CAP
{

//!
//! @brief Derived histograms of a NuDyn analysis (second phase, no event processing): reads the histograms
//! filled by a NuDynAnalyzer from the input file of the job, computes the factorial cumulants F2, the correlators R2 and nu_dyn (NuDynHistos::calculateDerived()) and writes the
//! completed folders to the output file of the job.
//!
//! It uses the same filters and histogram settings as the analyzer, and reads and writes the folders of the analyzer:
//! those of the folder named by the key \<name\>:SOURCE_TASK (the name of the analyzer task).
//!
class NuDynDerivedCalculator
:
public EventProcessorHistos<NuDynHistos,2>
{
public:

  //! @brief Default constructor.
  NuDynDerivedCalculator();
  //! @brief Copy constructor. @param source task to copy
  NuDynDerivedCalculator(const NuDynDerivedCalculator & source);
  //! @brief Copy assignment. @param rhs task to copy @return this task
  NuDynDerivedCalculator & operator=(const NuDynDerivedCalculator & rhs);
  //! @brief Destructor.
  virtual ~NuDynDerivedCalculator() {}
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

#endif /* CAP__NuDynDerivedCalculator */
