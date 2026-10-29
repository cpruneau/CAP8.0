/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__NuDynAnalyzer
#define CAP__NuDynAnalyzer
#include "EventProcessorHistos.hpp"
#include "NuDynHistos.hpp"

namespace CAP
{

//!
//! @brief Task computing multiplicity moments and the nu_dyn fluctuation variable for pairs of particle filters.
//!
//!Task used for the determination of multiplicity moments of second, third, and fourth order. These moments
//!can then be used, in a "derived" stage, to compute cumulants of 2nd, 3rd, and 4th order, as well as other
//!variables such as the nu-dynamic variable.
//!As for other task classes of this package, the computation can be carried out for one or  several event class types (selected by the
//!user supplied event filters) and for one or several particle filters (selected by the user as particle filters). It is then possible to calculate
//!moments, cumulants, and various combinations of these for specific kinematic ranges or specific particle types or both.
//!The moments are computed as integrated averages and as function of some global event characteristic or property. The specific
//!property used as independent variable to compute (and eventually plot) the moments, cumulants, etc, is selected based on the  inputType
//!parameter selected at construction of an instance of this class.
//!
//! The task has a folder (NuDynHistos) per event filter e and ordered pair of particle filters (i, j), at index
//! e*nPF*nPF + i*nPF + j (named \<event\>_\<particle i\>_\<particle j\>). For each accepted event filter and each pair
//! with j < i, execute() fills the folder (e, i, j) with the multiplicities n_i, n_j and the factorial moments
//! n_i(n_i-1), n_i n_j, n_j(n_j-1), as a function of the event class variable (see EventProcessor::getEvtClassValue());
//! the folders with j >= i stay empty. The histograms are profiles: they are not scaled. The derived histograms (F2,
//! R2, nu_dyn) are computed in a later job by NuDynDerivedCalculator.
//!
class NuDynAnalyzer
:
public EventProcessorHistos<NuDynHistos,2>
{
public:
  
  //! @brief Default constructor.
  NuDynAnalyzer();
  //! @brief Copy constructor. @param task task to copy
  NuDynAnalyzer(const NuDynAnalyzer & task);
  //! @brief Destructor.
  virtual ~NuDynAnalyzer() {}
  //! @brief Copy assignment. @param task task to copy @return this task
  NuDynAnalyzer & operator=(const NuDynAnalyzer & task);
  //! @brief Add the default configuration: that of EventProcessorHistos and the histogram binning.
  virtual void setDefaultConfiguration() override;
  //! @brief Analyze the current event (see the class description).
  //! @throws Exception if there are fewer folders than combinations of filters
  virtual void execute() override;
 
};

} // namespace CAP

#endif /* CAP__NuDynAnalyzer */
