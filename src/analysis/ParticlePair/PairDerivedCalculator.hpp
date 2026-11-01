/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file PairDerivedCalculator.hpp
//! @brief PairDerivedCalculator: the derived-histogram phase of a pair analysis, for any kind of pair histograms.
#ifndef CAP__PairDerivedCalculator
#define CAP__PairDerivedCalculator
#include <vector>
#include "AnalysisHistos.hpp"
#include "ParticleSingleHistos.hpp"
#include "Exceptions.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"

namespace CAP
{

//!
//! @brief Compute the derived histograms of the single folders (e, p) and of the pair folders (e, p1, p2) of a pair
//! analysis: each single folder from its base histograms, then each pair folder from its base histograms and the single
//! folders (e, p1) and (e, p2). The derived histograms are created where they are missing (they are there if the input
//! file has them; they are recomputed either way). Used by PairDerivedCalculator and PairBFCalculator.
//! @tparam PAIR pair folder (see PairDerivedCalculator)
//! @param pairFolders pair folders, at index e*nPF*nPF + p1*nPF + p2 @param singleFolders single folders, at e*nPF + p
//! @param nPF number of particle filters
//!
template <class PAIR>
void calculatePairDerived(std::vector<PAIR*> & pairFolders, std::vector<ParticleSingleHistos*> & singleFolders,
                          unsigned int nPF)
{
  const unsigned int nPFsq = nPF*nPF;
  for (ParticleSingleHistos * single : singleFolders)
    {
    if (!single->h_n1_phi) single->createDerived();
    single->calculateDerived();
    }
  for (unsigned int k=0; k<pairFolders.size(); k++)
    {
    unsigned int e  = k/nPFsq;
    unsigned int p1 = (k/nPF)%nPF;
    unsigned int p2 = k%nPF;
    PAIR & pair = *pairFolders[k];
    if (!pair.hasDerived()) pair.createDerived();
    pair.calculateDerived(*singleFolders[e*nPF+p1],*singleFolders[e*nPF+p2],p1==p2);
    }
}

//!
//! @brief Derived histograms of a pair analysis (second phase, no event processing), common to the pair analyses: the
//! analyses differ only by their histograms (PAIR), not by how they are called.
//!
//! The task reads the histograms filled by the analyzer named by \<name\>:SOURCE_TASK from the input file of the job
//! (its single folders (e, p) and pair folders (e, p1, p2)), with the same filters and histogram settings as the
//! analyzer, computes the derived histograms of every folder (calculatePairDerived()), and writes the completed folders
//! to the output file of the job, in the folder of the analyzer.
//!
//! There is no class per analysis: the task of an analysis is an instance of this template, registered under its own
//! name (e.g. ParticlePairDerivedCalculator = PairDerivedCalculator<ParticlePairHistos>, registered as
//! CAP::ParticlePairDerivedCalculator in ParticlePairTasks.cpp).
//!
//! @tparam PAIR pair folder: static addDefaultConfiguration(configuration, taskName), derivedCalculatorName() (name of
//! the task class, without namespace) and analyzerName() (default name of the analyzer task); hasDerived(),
//! createDerived(), calculateDerived(single1, single2, same)
//!
template <class PAIR>
class PairDerivedCalculator : public CalculatorHistos<PAIR,2,ParticleSingleHistos>
{
public:

  //! Base class.
  typedef CalculatorHistos<PAIR,2,ParticleSingleHistos> Base;

  //! @brief Default constructor: the name of the task and the default SOURCE_TASK come from PAIR
  //! (PAIR::derivedCalculatorName(), PAIR::analyzerName()).
  PairDerivedCalculator()
  :
  Base(),
  _sourceTask(PAIR::analyzerName())
  {
  this->appendClassName(PAIR::derivedCalculatorName());
  this->setMinimumReportLevel(Object::kInfo);
  this->setName(PAIR::derivedCalculatorName());
  this->setTitle(PAIR::derivedCalculatorName());
  }

  //! @brief Copy constructor. @param source task to copy
  PairDerivedCalculator(const PairDerivedCalculator & source) = default;
  //! @brief Copy assignment. @param rhs task to copy @return this task
  PairDerivedCalculator & operator=(const PairDerivedCalculator & rhs) = default;
  //! @brief Destructor.
  virtual ~PairDerivedCalculator() {}
  //! @brief Copy of this task. @return new task, owned by the caller
  virtual Task * clone() override { return new PairDerivedCalculator(*this); }

  //! @brief Add the default configuration: that of CalculatorHistos, the histogram settings and SOURCE_TASK.
  virtual void setDefaultConfiguration() override
  {
  Base::setDefaultConfiguration();
  PAIR::addDefaultConfiguration(this->_configuration,this->name());
  this->_configuration.addProperty(createKey(this->name(),"SOURCE_TASK"),String(PAIR::analyzerName()));
  }

  //! @brief Configure the task (and read SOURCE_TASK). @param configuration configuration
  virtual void configure(const Configuration & configuration) override
  {
  Base::configure(configuration);
  _sourceTask = configuration.valueString(createKey(this->name(),"SOURCE_TASK"));
  if (this->reportInfo(__FUNCTION__)) printValue(createKey(this->name(),"SOURCE_TASK"),_sourceTask);
  }

  //!
  //! @brief Compute the derived histograms of every folder from its base histograms.
  //! @throws Exception if there are fewer folders than combinations of filters
  //!
  virtual void execute() override
  {
  if (this->reportDebug(__FUNCTION__)) { printCR(); }
  this->_taskExecuted.increment();
  const unsigned int nPF = this->nParticleFilters();
  std::vector<PAIR*>                 & pairFolders   = this->histograms();
  std::vector<ParticleSingleHistos*> & singleFolders = this->singleHistograms();
  if (pairFolders.size()<this->nCombinations())
    throw Exception("Fewer pair folders than combinations of filters",__FUNCTION__);
  if (singleFolders.size()<this->nEventFilters()*nPF)
    throw Exception("Fewer single folders than nEventFilters()*nParticleFilters()",__FUNCTION__);
  calculatePairDerived(pairFolders,singleFolders,nPF);
  }

  //! @brief Name of the folder of the job files holding the histograms: that of the analyzer (SOURCE_TASK). @return name
  virtual String histogramTaskName() const override { return _sourceTask; }

protected:

  //! Name of the analyzer task whose histograms are completed.
  String _sourceTask;

};

} // namespace CAP

#endif /* CAP__PairDerivedCalculator */
