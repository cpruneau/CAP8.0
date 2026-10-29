/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__PerformanceAnalyzer
#define CAP__PerformanceAnalyzer
#include "EventProcessorHistos.hpp"
#include "ParticlePerformanceHistos.hpp"

namespace CAP
{

//!
//! @brief Analyzer filling detector performance histograms (ParticlePerformanceHistos) for each (event filter, particle
//! filter) combination.
//!
class PerformanceAnalyzer
:
public EventProcessorHistos<ParticlePerformanceHistos,1>
{
public:
  
  //! @brief Default constructor.
  PerformanceAnalyzer();
  //! @brief Copy constructor. @param analyzer object to copy
  PerformanceAnalyzer(const PerformanceAnalyzer & analyzer);
  //! @brief Copy assignment. @param analyzer object to copy @return this object
  PerformanceAnalyzer & operator=(const PerformanceAnalyzer & analyzer);
  //! @brief Destructor.
  virtual ~PerformanceAnalyzer() {}
  //! @brief Add the default configuration of the task.
  virtual void setDefaultConfiguration() override;
  //! @brief Execute the task (process the current event).
  virtual void execute() override;

protected:

};

} // namespace CAP

#endif /* CAP__PerformanceAnalyzer */
