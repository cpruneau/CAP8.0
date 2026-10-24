/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ParticleFlowAnalyzer
#define CAP__ParticleFlowAnalyzer
#include "EventProcessorSingle.hpp"
#include "ParticleFlowHistos.hpp"
#include "ParticleFlowDerivedHistos.hpp"

namespace CAP
{

//!
//! @brief Analyzer of particle flow (azimuthal anisotropy) per (event filter, particle filter) combination.
//!
//! @warning Work in progress, not built (ParticleFlow is not in src/CMakeLists.txt): the class is declared as
//! NuDynAnalyzer while its members are named ParticleFlowAnalyzer, and the description below is copied from
//! NuDynAnalyzer.
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
class NuDynAnalyzer
:
public EventProcessorSingle<ParticleFlowHistos,ParticleFlowDerivedHistos>
{
public:
  
  //! @brief Default constructor.
  ParticleFlowAnalyzer();
  //! @brief Copy constructor. @param analyzer analyzer to copy
  ParticleFlowAnalyzer(const ParticleFlowAnalyzer & analyzer);
  //! @brief Copy assignment. @param analyzer analyzer to copy @return this analyzer
  ParticleFlowAnalyzer & operator=(const ParticleFlowAnalyzer & analyzer);
  //! @brief Destructor.
  virtual ~ParticleFlowAnalyzer() {}
  //! @brief Add the default configuration.
  virtual void setDefaultConfiguration();
  //! @brief Analyze the current event.
  virtual void execute();

};

} // namespace CAP

#endif /* CAP__ParticleFlowAnalyzer */
