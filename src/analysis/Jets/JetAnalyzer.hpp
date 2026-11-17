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
//! @brief JetAnalyzer: jets found with FastJet for several jet settings in one run, selected by jet filters, and their
//! per-event, single-jet and constituent histograms.
#ifndef DATUM__JetAnalyzer
#define DATUM__JetAnalyzer
#include "JetTask.hpp"
#include "JetHistos.hpp"
#include "JetUnderlyingEvent.hpp"

namespace Datum
{

//!
//! @brief Analysis of jets: for each event filter, particle filter (the particles clustered), jet settings and jet filter
//! (see JetTask), the jets accepted fill a folder of JetHistos: per event (number of jets, pT sum, number of cones of the
//! underlying event), per jet, the moments of the multiplicities within the jets and of those of the underlying event
//! versus the jet pT and the jet multiplicity, and per constituent.
//!
//! The underlying event of an event is measured, for each particle filter and jet settings, in cones of the radius of
//! the jets transverse to the leading jet (UnderlyingEventCones), kept if within the acceptance of the particles and
//! away from every jet. Settings: UE:MODE, TRANSVERSE (two cones, phi_lead +- pi/2 at eta_lead, default) or
//! TRANSVERSE_SHIFTED (four cones, phi_lead +- pi/2 at eta_lead +- 2R); UE:ETA_MAX, the |eta| range of the particles in
//! which the cones must lie entirely (|eta_cone| < UE:ETA_MAX - R; 0.9 by default; set it to the pseudorapidity range
//! of the particle filters). Jet settings whose radius is not below UE:ETA_MAX have no cone (reported at the start).
//!
//! Folders: \<event filter\>_\<particle filter\>_\<jet settings\>_\<jet filter\> (names of the objects). At the end of the
//! run, the per-event histograms are divided by the number of events accepted by the event filter, and the per-jet and
//! per-constituent histograms by the number of jets of their folder; the profiles are averages per jet (per cone for
//! the underlying event).
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
  //! @brief Add the default configuration: that of JetTask, UE:MODE and UE:ETA_MAX.
  virtual void setDefaultConfiguration() override;
  //! @brief Read the configuration. @param configuration configuration
  virtual void configure(const Configuration & configuration) override;
  //! @brief Placement of the cones of the underlying event. @return mode
  UnderlyingEventCones::Mode mode() const { return _mode; }
  //! @brief |eta| range of the particles for the cones (UE:ETA_MAX). @return range
  double coneEtaMax() const { return _coneEtaMax; }

protected:

  //! @brief Fill the folder with the jets accepted and the cones of the underlying event of the event. @param first
  //! folder @param jets all the jets @param accepted jets accepted @param particles particles @param settings settings
  virtual void fill(unsigned int first, const std::vector<Jet> & jets, const std::vector<const Jet*> & accepted,
                    const std::vector<Particle*> & particles, const JetSettings & settings) override;

  UnderlyingEventCones::Mode _mode = UnderlyingEventCones::Transverse;   //!< placement of the cones
  double _coneEtaMax = 0.9;                                             //!< |eta| range of the particles (UE:ETA_MAX)
  std::vector<Multiplicities> _cones;                                   //!< work: multiplicities of the cones
};

} // namespace Datum

#endif /* DATUM__JetAnalyzer */
