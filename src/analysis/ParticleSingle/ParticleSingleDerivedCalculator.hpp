/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
#ifndef CAP__ParticleSingleDerivedCalculator
#define CAP__ParticleSingleDerivedCalculator
#include "EventProcessorHistos.hpp"
#include "ParticleSingleHistos.hpp"

namespace CAP
{

//!
//! @brief Derived histograms of a single-particle analysis (second phase, no event processing): reads the histograms
//! filled by a ParticleSingleAnalyzer from the input file of the job, computes their projections and pT averages
//! (ParticleSingleHistos::calculateDerived()) and writes the completed folders to the output file of the job.
//!
//! It uses the same filters and histogram settings (keys \<name\>:HISTOGRAM:n1_nbins, ..., eta_fill, rapidity_fill,
//! p2_fill) as the analyzer, and reads and writes the folders of the analyzer: those of the folder named by the key
//! \<name\>:SOURCE_TASK (the name of the analyzer task).
//!
class ParticleSingleDerivedCalculator
:
public EventProcessorHistos<ParticleSingleHistos,1>
{
public:

  //! @brief Default constructor.
  ParticleSingleDerivedCalculator();
  //! @brief Copy constructor. @param source task to copy
  ParticleSingleDerivedCalculator(const ParticleSingleDerivedCalculator & source);
  //! @brief Copy assignment. @param rhs task to copy @return this task
  ParticleSingleDerivedCalculator & operator=(const ParticleSingleDerivedCalculator & rhs);
  //! @brief Destructor.
  virtual ~ParticleSingleDerivedCalculator() {}
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

#endif /* CAP__ParticleSingleDerivedCalculator */
