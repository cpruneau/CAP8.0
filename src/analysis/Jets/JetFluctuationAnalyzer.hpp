/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file JetFluctuationAnalyzer.hpp
//! @brief JetFluctuationAnalyzer: fluctuations and correlations of the multiplicities within jets, with the underlying
//! event measured in cones transverse to the leading jet.
#ifndef CAP__JetFluctuationAnalyzer
#define CAP__JetFluctuationAnalyzer
#include "JetTask.hpp"
#include "JetFluctuationHistos.hpp"

namespace CAP
{

//!
//! @brief Analysis of the multiplicities within jets: for each event filter, particle filter, radius and jet filter (see
//! JetTask), each jet accepted fills the moments of its multiplicities by charge sign (JetFluctuationHistos), with those
//! of the cones of the underlying event of its event: cones of the radius of the jets, transverse to the leading jet of
//! the same particle filter and radius (UnderlyingEventCones), kept if within the acceptance and away from every jet.
//!
//! Settings: UE:MODE, TRANSVERSE (two cones, phi_lead +- pi/2 at eta_lead, default) or TRANSVERSE_SHIFTED (four cones,
//! phi_lead +- pi/2 at eta_lead +- 2R). The derived quantities (variances, R2, nu_dyn, and the same corrected for the
//! underlying event) are computed at the end of the run, and again by calculateDerived() in a job reading the folders
//! (HISTOGRAM:IMPORT, e.g. after the sub-samples are merged).
//!
class JetFluctuationAnalyzer : public JetTask<JetFluctuationHistos>
{
public:

  //! @brief Default constructor.
  JetFluctuationAnalyzer();
  //! @brief Copy constructor. @param source task
  JetFluctuationAnalyzer(const JetFluctuationAnalyzer & source) = default;
  //! @brief Copy assignment. @param rhs task @return this task
  JetFluctuationAnalyzer & operator=(const JetFluctuationAnalyzer & rhs) = default;
  //! @brief Destructor.
  virtual ~JetFluctuationAnalyzer() { }
  //! @brief Copy of the task. @return new task
  virtual Task * clone() override { return new JetFluctuationAnalyzer(*this); }
  //! @brief Add the default configuration: that of JetTask and UE:MODE.
  virtual void setDefaultConfiguration() override;
  //! @brief Read the configuration. @param configuration configuration
  virtual void configure(const Configuration & configuration) override;
  //! @brief Compute the derived histograms, then normalize (JetTask::finalize()).
  virtual void finalize() override;
  //! @brief Compute the derived histograms of the folders.
  virtual void calculateDerived() override;
  //! @brief Placement of the cones. @return mode
  UnderlyingEventCones::Mode mode() const { return _mode; }

protected:

  //! @brief Fill a folder with the jets accepted and the cones of the event. @param first folder @param jets all the jets
  //! @param accepted jets accepted @param particles particles @param settings settings
  virtual void fill(unsigned int first, const std::vector<Jet> & jets, const std::vector<const Jet*> & accepted,
                    const std::vector<Particle*> & particles, const JetSettings & settings) override;

  UnderlyingEventCones::Mode _mode = UnderlyingEventCones::Transverse;   //!< placement of the cones
  std::vector<Multiplicities> _cones;                                   //!< work: multiplicities of the cones
};

} // namespace CAP

#endif /* CAP__JetFluctuationAnalyzer */
