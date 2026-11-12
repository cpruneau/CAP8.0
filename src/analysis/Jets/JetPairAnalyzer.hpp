/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file JetPairAnalyzer.hpp
//! @brief JetPairAnalyzer: pairs of jets (e.g. leading and subleading), selected by jet pair filters, and their
//! histograms: azimuthal separation, acoplanarity, momentum balance, mass.
#ifndef CAP__JetPairAnalyzer
#define CAP__JetPairAnalyzer
#include "JetTask.hpp"
#include "JetPairHistos.hpp"

namespace CAP
{

//!
//! @brief Analysis of jet pairs: for each event filter, particle filter, radius and jet filter (see JetTask), the pairs
//! of the jets accepted by the jet filter (each pair once, the first jet of higher rank) accepted by each jet pair
//! filter (JET_PAIR_FILTER: ranks of the two jets, |delta phi|, acoplanarity, |delta eta|, pT ratio, mass and pT of the
//! pair) fill a folder of JetPairHistos.
//!
//! Folders: \<event filter\>_\<particle filter\>_\<R label\>_\<jet filter\>_\<pair filter\>. The pair filters: keys of a
//! list of managed objects (\<task\>:JET_PAIR_FILTER:N, JET_PAIR_FILTER_\<k\>:NAME, ...), made by JetPairFilterCreator.
//! At the end of the run, nPairs is divided by the number of events accepted by the event filter, the pair histograms by
//! the number of pairs of their folder.
//!
class JetPairAnalyzer : public JetTask<JetPairHistos>
{
public:

  //! @brief Default constructor.
  JetPairAnalyzer();
  //! @brief Copy constructor. @param source task
  JetPairAnalyzer(const JetPairAnalyzer & source);
  //! @brief Copy assignment. @param rhs task @return this task
  JetPairAnalyzer & operator=(const JetPairAnalyzer & rhs);
  //! @brief Destructor.
  virtual ~JetPairAnalyzer() { }
  //! @brief Copy of the task. @return new task
  virtual Task * clone() override { return new JetPairAnalyzer(*this); }
  //! @brief Add the default configuration: that of JetTask and the pair filters (JET_PAIR_FILTER).
  virtual void setDefaultConfiguration() override;
  //! @brief Number of pair filters. @return number
  virtual unsigned int nSelections() const override { return _managedPairFilters.size(); }
  //! @brief Name of a pair filter. @param x filter @return name
  virtual String selectionName(unsigned int x) const override { return _managedPairFilters.getObjects()[x]->name(); }
  //! @brief Pair filters. @return filters
  const std::vector<JetPairFilter*> & pairFilters() const { return _managedPairFilters.getObjects(); }

protected:

  //! @brief Fill the folders of the pair filters with the pairs of the jets accepted. @param first folder of the first
  //! pair filter @param jets all the jets @param accepted jets accepted @param particles particles @param settings settings
  virtual void fill(unsigned int first, const std::vector<Jet> & jets, const std::vector<const Jet*> & accepted,
                    const std::vector<Particle*> & particles, const JetSettings & settings) override;
  //! @brief Read the pair filters. @param configuration configuration
  virtual void configureSelections(const Configuration & configuration) override;
  //! @brief Initialize the pair filters.
  virtual void initializeSelections() override;
  //! @brief Finalize the pair filters.
  virtual void finalizeSelections() override;

  ManagedObjects<JetPairFilter> _managedPairFilters;   //!< pair filters
  std::vector<JetPair> _pairs;                         //!< work: the pairs of the jets accepted
  std::vector<JetPair> _selected;                      //!< work: the pairs accepted by a pair filter
};

} // namespace CAP

#endif /* CAP__JetPairAnalyzer */
