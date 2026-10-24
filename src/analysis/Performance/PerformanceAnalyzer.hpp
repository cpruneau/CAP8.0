/* **********************************************************************
 * Copyright (C) 2019-2024, Claude Pruneau, Victor Gonzalez   
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__PerformanceAnalyzer
#define CAP__PerformanceAnalyzer
#include "EventProcessorSingle.hpp"
#include "ParticlePerformanceHistos.hpp"
#include "ParticlePerformanceDerivedHistos.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Analyzer filling detector performance histograms (ParticlePerformanceHistos) for each (event filter, particle
//! filter) combination.
//!
class PerformanceAnalyzer
:
public EventProcessorSingle<ParticlePerformanceHistos,ParticlePerformanceDerivedHistos>
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
  virtual void setDefaultConfiguration();
  //! @brief Execute the task (process the current event).
  virtual void execute();

protected:

  ClassDef(PerformanceAnalyzer,0)
};

} // namespace CAP

#endif /* CAP__PerformanceAnalyzer */
