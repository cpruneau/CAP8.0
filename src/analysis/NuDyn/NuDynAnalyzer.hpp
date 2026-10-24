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
#ifndef CAP__NuDynAnalyzer
#define CAP__NuDynAnalyzer
#include "EventProcessorSingle.hpp"
#include "NuDynHistos.hpp"
#include "NuDynDerivedHistos.hpp"
#include "ClassDef.hpp"

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
//! For each accepted event filter and each pair of particle filters (i, j) with j < i, execute() fills a NuDynHistos
//! (group index e*nPF*nPF + i*nPF + j) with the multiplicities n_i, n_j and the factorial moments n_i(n_i-1), n_i n_j,
//! n_j(n_j-1), as a function of the event class variable (see EventProcessor::getEvtClassValue()).
//!
class NuDynAnalyzer
:
public EventProcessorSingle<NuDynHistos,NuDynDerivedHistos>
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
  //! @brief Add the default configuration: that of EventProcessorSingle and the histogram binning.
  virtual void setDefaultConfiguration();
  //! @brief Analyze the current event (see the class description).
  virtual void execute();
 
  ClassDef(NuDynAnalyzer,0)
};

} // namespace CAP

#endif /* CAP__NuDynAnalyzer */
