/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file JetClustering.hpp
//! @brief JetClustering: the jets of a set of particles found by FastJet, and the jets of the events, found once per
//! event, particle filter and jet settings and shared by the tasks.
#ifndef CAP__JetClustering
#define CAP__JetClustering
#include <vector>
#include "Jet.hpp"

namespace CAP
{

class Event;
class ParticleFilter;

//!
//! @brief Jet finding with FastJet (Cacciari, Salam, Soyez, Eur. Phys. J. C 72, 1896 (2012)).
//!
//! cluster() clusters a set of particles with the algorithm, radius and recombination scheme of the settings (anti-kt
//! by default), keeps the jets above the minimum pT whose pseudorapidity is within |eta| < particleEtaMax - R (fully
//! inside the acceptance of the particles), sorts them by decreasing pT and gives them their rank (1: leading).
//!
//! jets() does the same for the particles accepted by a particle filter in an event, once per event: the jets are kept
//! for the event (until it is reset or its particles change, Event::epoch()) and shared by all the tasks asking for the
//! same filter and settings, so that several analyzers (jets, jet pairs, fluctuations) do not cluster again.
//!
//! @code
//!   JetSettings settings;  settings.radius = 0.4;  settings.ptMin = 10.0;  settings.particleEtaMax = 0.9;
//!   const std::vector<Jet> & jets = JetClustering::jets(event, filter, particles, settings);
//!   for (const Jet & jet : jets) ... jet.pt(), jet.rank, jet.constituents ...
//! @endcode
//!
class JetClustering
{
public:

  //! @brief Jets of a set of particles. @param particles particles @param settings settings @return jets, by decreasing pT
  static std::vector<Jet> cluster(const std::vector<Particle*> & particles, const JetSettings & settings);
  //!
  //! @brief Jets of the particles accepted by a particle filter in an event, found once per event and settings.
  //! @param event event @param filter particle filter (identifies the particles) @param particles the particles it
  //! accepts in the event @param settings settings @return jets (valid until the next event)
  //!
  static const std::vector<Jet> & jets(const Event & event, const ParticleFilter * filter,
                                       const std::vector<Particle*> & particles, const JetSettings & settings);
  //! @brief Forget the jets kept for the events.
  static void clearCache();
  //! @brief Number of clusterings done (for tests). @return number
  static unsigned long nClusterings();
};

} // namespace CAP

#endif /* CAP__JetClustering */
