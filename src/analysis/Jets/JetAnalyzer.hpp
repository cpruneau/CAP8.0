/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani, Akash Raj
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau, Akash Raj,  Nov 2024
 *
 * *********************************************************************/
#ifndef CAP__JetAnalyzer
#define CAP__JetAnalyzer
#include "EventProcessor.hpp"
#include "JetFilter.hpp"

namespace CAP
{

//!
//! @brief Jet analyzer: clusters the accepted particles of each event into jets with FastJet (radius jetRadius,
//! minimum pT jetPtMin) and fills jet and constituent histograms for each jet filter.
//! @warning Work in progress: the Jets library is built only with FastJet, and this header does not compile as is
//! (ManagedList is not defined; clusteredJets is initialized from an undeclared clusterSequence).
//!
class JetAnalyzer
:
public EventProcessorSingle<JetSingleHistos,JetSingleDerivedHistos>,
public ManagedList<JetFilter>
{
public:

  //! @brief Default constructor.
  JetAnalyzer();
  //! @brief Copy constructor. @param soure object to copy
  JetAnalyzer(const JetAnalyzer & soure);
  //! @brief Copy assignment. @param rhs object to copy @return this object
  JetAnalyzer & operator=(const JetAnalyzer & rhs);
  //! @brief Destructor.
  virtual ~JetAnalyzer() {}
  //! @brief Add the default configuration of the task.
  virtual void setDefaultConfiguration();
  //! @brief Initialize the task.
  virtual void initialize();
  //! @brief Execute the task (process the current event).
  virtual void execute();

protected:

  //! Particles of the event given to the jet clustering.
  std::vector<PseudoJet> pseudoJetsInput;
  //! Jets found, sorted by decreasing pT.
  std::vector<PseudoJet> clusteredJets = sorted_by_pt( clusterSequence->inclusive_jets(jetPtMin) );

  //! Jet radius parameter R.
  double jetRadius;
  //! Minimum jet pT.
  double jetPtMin;


};

} // namespace CAP

#endif /* CAP__JetAnalyzer */
