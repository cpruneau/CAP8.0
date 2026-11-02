/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
//! @file DerivedCalculator.hpp
//! @brief DerivedCalculator: the derived-histogram phase of an analysis whose folders are computed one by one.
#ifndef CAP__DerivedCalculator
#define CAP__DerivedCalculator
#include <vector>
#include "AnalysisHistos.hpp"
#include "Exceptions.hpp"
#include "PrintHelpers.hpp"

namespace CAP
{

//!
//! @brief Derived histograms of an analysis (second phase, no event processing) whose folders are each computed from
//! their own base histograms: single-particle, global, nu-dynamic and pT-pT analyses. (The pair analyses, whose pair
//! folders also need the single-particle folders, use PairDerivedCalculator.)
//!
//! The task reads the folders filled by the analyzer named by \<name\>:SOURCE_TASK from the input file of the job, with
//! the same filters and histogram settings as the analyzer, computes the derived histograms of every folder, and writes
//! the completed folders to the output file of the job, in the folder of the analyzer.
//!
//! There is no class per analysis: the task of an analysis is an instance of this template, registered under its own
//! name (e.g. GlobalDerivedCalculator = DerivedCalculator<GlobalHistos,1>, declared in GlobalHistos.hpp and registered
//! as CAP::GlobalDerivedCalculator in GlobalTasks.cpp).
//!
//! @tparam HISTOS histogram folder: static addDefaultConfiguration(configuration, taskName), derivedCalculatorName()
//! (name of the task class, without namespace) and analyzerName() (default name of the analyzer task); hasDerived(),
//! createDerived(), calculateDerived()
//! @tparam RANK number of particle filters per folder
//!
template <class HISTOS, unsigned int RANK>
class DerivedCalculator : public CalculatorHistos<HISTOS,RANK>
{
public:

  //! Base class.
  typedef CalculatorHistos<HISTOS,RANK> Base;

  //! @brief Default constructor: the name of the task and the default SOURCE_TASK come from HISTOS
  //! (HISTOS::derivedCalculatorName(), HISTOS::analyzerName()).
  DerivedCalculator()
  :
  Base(HISTOS::analyzerName())
  {
  this->appendClassName(HISTOS::derivedCalculatorName());
  this->setMinimumReportLevel(Object::kInfo);
  this->setName(HISTOS::derivedCalculatorName());
  this->setTitle(HISTOS::derivedCalculatorName());
  }

  //! @brief Copy constructor. @param source task to copy
  DerivedCalculator(const DerivedCalculator & source) = default;
  //! @brief Copy assignment. @param rhs task to copy @return this task
  DerivedCalculator & operator=(const DerivedCalculator & rhs) = default;
  //! @brief Destructor.
  virtual ~DerivedCalculator() {}
  //! @brief Copy of this task. @return new task, owned by the caller
  virtual Task * clone() override { return new DerivedCalculator(*this); }

  //! @brief Add the default configuration: that of CalculatorHistos (SOURCE_TASK, folders imported), and the
  //! histogram settings of the analysis.
  virtual void setDefaultConfiguration() override
  {
  Base::setDefaultConfiguration();
  HISTOS::addDefaultConfiguration(this->_configuration,this->name());
  }

  //!
  //! @brief Compute the derived histograms of every folder from its base histograms (created if the input file does
  //! not have them).
  //! @throws Exception if there are fewer folders than combinations of filters
  //!
  virtual void execute() override
  {
  if (this->reportDebug(__FUNCTION__)) { printCR(); }
  this->_taskExecuted.increment();
  std::vector<HISTOS*> & folders = this->histograms();
  if (folders.size()<this->nCombinations())
    throw Exception("Fewer histogram folders than combinations of filters",__FUNCTION__);
  for (HISTOS * folder : folders)
    {
    if (!folder->hasDerived()) folder->createDerived();
    folder->calculateDerived();
    }
  }

};

} // namespace CAP

#endif /* CAP__DerivedCalculator */
