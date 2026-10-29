/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__SpherocityAnalyzer
#define CAP__SpherocityAnalyzer
#include "EventProcessorHistos.hpp"
#include "SpherocityHistos.hpp"

namespace CAP
{
//!
//! @brief Task computing the transverse spherocity of the accepted particles of each event, per (event filter,
//! particle filter) combination.
//!
//! S0 = min over the unit vectors n of (sum |pT x n| / sum pT)^2 (pT-weighted), S1 = the same with unit pT vectors
//! (unweighted); the minimum is searched over nSteps azimuthal directions. The values are not multiplied by pi^2/4.
//!
//!Task performing a transverse spherocity analysis of events of the incoming event stream.
//!
class SpherocityAnalyzer
:
public EventProcessorHistos<SpherocityHistos,1>
{
public:
  
  //! @brief Default constructor.
  SpherocityAnalyzer();
  //! @brief Copy constructor. @param group task to copy
  SpherocityAnalyzer(const SpherocityAnalyzer & group);
  //! @brief Copy assignment. @param group task to copy @return this task
  SpherocityAnalyzer & operator=(const SpherocityAnalyzer & group);
  //! @brief Destructor.
  virtual ~SpherocityAnalyzer() {}
  //! @brief Add the default configuration: fill options (S0, S1, S1 vs S0), number of steps, binning.
  virtual void setDefaultConfiguration() override;
  //! @brief Configure the task. @param configuration configuration
  virtual void configure(const Configuration & configuration) override;
  //! @brief Compute S0 and S1 for each accepted combination and fill the histograms.
  virtual void execute() override;

protected:

  bool   setEvent;   //!< Whether this task instance sets spherocity properties stored in the EventProperty record of the current event.
  bool   fillS0;     //!< Whether the regular spherocity observable S0 should be analyzed and filled by this rhs.
  bool   fillS1;     //!< Whether the unit vector spherocity observable S1 should be analyzed and filled by this rhs.
  bool   fillS1VsS0; //!< Whether two-dimensional S1 vs S0 histograms should be filled.
  int    nSteps;   //!< Number of azimuthal steps used in the calculation of the transverse spherocity.
  double stepSize; //!< Two-pi/nSteps.

};

} //namespace CAP


#endif /* CAP__SpherocityAnalyzer */
