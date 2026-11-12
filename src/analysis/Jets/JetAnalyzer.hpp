/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file JetAnalyzer.hpp
//! @brief JetAnalyzer: jets found with FastJet for several radii in one run, selected by jet filters, and their
//! per-event, single-jet and constituent histograms.
#ifndef CAP__JetAnalyzer
#define CAP__JetAnalyzer
#include "JetTask.hpp"
#include "JetHistos.hpp"

namespace CAP
{

//!
//! @brief Analysis of jets: for each event filter, particle filter (the particles clustered), radius and jet filter
//! (see JetTask), the jets accepted fill a folder of JetHistos: per event (number of jets, pT sum), per jet and per
//! constituent.
//!
//! Folders: \<event filter\>_\<particle filter\>_\<R label\>_\<jet filter\> (R label: R0p4 for 0.4). At the end of the
//! run, the per-event histograms are divided by the number of events accepted by the event filter, and the per-jet and
//! per-constituent histograms by the number of jets of their folder; the profiles are averages per jet.
//!
class JetAnalyzer : public JetTask<JetHistos>
{
public:

  //! @brief Default constructor.
  JetAnalyzer();
  //! @brief Copy constructor. @param source task
  JetAnalyzer(const JetAnalyzer & source) = default;
  //! @brief Copy assignment. @param rhs task @return this task
  JetAnalyzer & operator=(const JetAnalyzer & rhs) = default;
  //! @brief Destructor.
  virtual ~JetAnalyzer() { }
  //! @brief Copy of the task. @return new task
  virtual Task * clone() override { return new JetAnalyzer(*this); }

protected:

  //! @brief Fill the folder with the jets accepted. @param first folder @param jets all the jets
  //! @param accepted jets accepted @param particles particles @param settings settings
  virtual void fill(unsigned int first, const std::vector<Jet> & jets, const std::vector<const Jet*> & accepted,
                    const std::vector<Particle*> & particles, const JetSettings & settings) override;
};

} // namespace CAP

#endif /* CAP__JetAnalyzer */
