/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file JetGroomingAnalyzer.hpp
//! @brief JetGroomingAnalyzer: the jets of a jet filter groomed by each jet groomer, and the histograms of the groomed
//! jets and of the splittings the groomers select.
#ifndef DATUM__JetGroomingAnalyzer
#define DATUM__JetGroomingAnalyzer
#include "JetTask.hpp"
#include "JetGroomer.hpp"
#include "JetGroomingHistos.hpp"

namespace Datum
{

//!
//! @brief Analysis of groomed jets: for each event filter, particle filter, jet settings and jet filter (see JetTask),
//! each jet accepted by the jet filter (on its kinematics before grooming) is groomed by each jet groomer (JET_GROOMER:
//! Soft Drop / modified mass drop, trimming, filtering, pruning, dynamical grooming, see JetGroomer) and fills the
//! folder of JetGroomingHistos of that groomer.
//!
//! Folders: \<event filter\>_\<particle filter\>_\<jet settings\>_\<jet filter\>_\<jet groomer\>. The groomers: keys of
//! a list of managed objects (\<task\>:JET_GROOMER:N, JET_GROOMER_\<k\>:NAME, ...), made by JetGroomerCreator. At the
//! end of the run, the histograms of a folder are divided by the number of jets it groomed.
//!
class JetGroomingAnalyzer : public JetTask<JetGroomingHistos>
{
public:

  //! @brief Default constructor.
  JetGroomingAnalyzer();
  //! @brief Copy constructor. @param source task
  JetGroomingAnalyzer(const JetGroomingAnalyzer & source);
  //! @brief Copy assignment. @param rhs task @return this task
  JetGroomingAnalyzer & operator=(const JetGroomingAnalyzer & rhs);
  //! @brief Destructor.
  virtual ~JetGroomingAnalyzer() { }
  //! @brief Copy of the task. @return new task
  virtual Task * clone() override { return new JetGroomingAnalyzer(*this); }
  //! @brief Add the default configuration: that of JetTask and the jet groomers (JET_GROOMER).
  virtual void setDefaultConfiguration() override;
  //! @brief Number of jet groomers. @return number
  virtual unsigned int nSelections() const override { return _managedGroomers.size(); }
  //! @brief Name of a jet groomer. @param x groomer @return name
  virtual String selectionName(unsigned int x) const override { return _managedGroomers.getObjects()[x]->name(); }
  //! @brief Jet groomers. @return groomers
  const std::vector<JetGroomer*> & groomers() const { return _managedGroomers.getObjects(); }

protected:

  //! @brief Groom the jets accepted with each groomer and fill its folder. @param first folder of the first groomer
  //! @param jets all the jets @param accepted jets accepted @param particles particles @param settings settings
  virtual void fill(unsigned int first, const std::vector<Jet> & jets, const std::vector<const Jet*> & accepted,
                    const std::vector<Particle*> & particles, const JetSettings & settings) override;
  //! @brief Read the jet groomers. @param configuration configuration @throws ConfigurationException if none
  virtual void configureSelections(const Configuration & configuration) override;
  //! @brief Initialize the jet groomers.
  virtual void initializeSelections() override;
  //! @brief Finalize the jet groomers.
  virtual void finalizeSelections() override;

  ManagedObjects<JetGroomer> _managedGroomers;   //!< jet groomers
};

} // namespace Datum

#endif /* DATUM__JetGroomingAnalyzer */
