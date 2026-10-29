/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__PtPtAnalyzer
#define CAP__PtPtAnalyzer
#include "EventProcessorHistos.hpp"
#include "PtPtHistos.hpp"

namespace CAP
{
//!
//! @brief Task for the analysis of transverse momentum correlations from event-wise pT moments (sums of powers of pT
//! of the accepted particles, see EventProcessor::moments()), as a function of an event variable.
//!
//! @warning In development: execute() applies the filters but fills no histograms.
//!
//!Task used for the determination of multiplicity moments of second, third, and fourth order. These moments
//!can then be used, in a "derived" stage, to compute cumulants of 2nd, 3rd, and 4th order, as well as other
//!variables such as the nu-dynamic variable.
//!As for other task classes of this package, the computation can be carried out for one or  several event class types (selected by the
//!user supplied event filters) and for one or several particle filters (selected by the user as particle filters). It is then possible to calculate
//!moments, cumulants, and various combinations of these for specific kinematic ranges or specific particle types or both.
//!
class PtPtAnalyzer
:
public EventProcessorHistos<PtPtHistos,1>
{
public:
  
  //! @brief Default constructor.
  PtPtAnalyzer();
  //! @brief Copy constructor. @param task task to copy
  PtPtAnalyzer(const PtPtAnalyzer & task);
  //! @brief Destructor.
  virtual ~PtPtAnalyzer() {}
  //! @brief Copy assignment. @param task task to copy @return this task
  PtPtAnalyzer & operator=(const PtPtAnalyzer & task);
  //! @brief Add the default configuration: that of EventProcessorHistos and the histogram binning.
  virtual void setDefaultConfiguration() override;
  //! @brief Analyze the current event (applies the filters only, see the class description).
  virtual void execute() override;

};

} // namespace CAP

#endif /* CAP__PtPtAnalyzer */
