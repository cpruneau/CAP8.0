/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file JetUnderlyingEvent.hpp
//! @brief Multiplicities by charge sign of the constituents of a jet or of the particles of a cone, and the cones of the
//! underlying event, transverse to the leading jet.
#ifndef CAP__JetUnderlyingEvent
#define CAP__JetUnderlyingEvent
#include <vector>
#include "Jet.hpp"

namespace CAP
{

//!
//! @brief Numbers of particles in total and by charge sign (positive, negative, neutral).
//!
struct Multiplicities
{
  double n    = 0.0;   //!< all
  double nPos = 0.0;   //!< positive
  double nNeg = 0.0;   //!< negative
  double n0   = 0.0;   //!< neutral

  //! @brief Count a particle of a charge. @param charge charge
  void add(double charge) { n += 1.0; if (charge>0.0) nPos += 1.0; else if (charge<0.0) nNeg += 1.0; else n0 += 1.0; }
};

//! @brief Multiplicities of the constituents of a jet. @param jet jet @return multiplicities
Multiplicities constituentMultiplicities(const Jet & jet);

//!
//! @brief A cone of the underlying event: its axis in pseudorapidity and azimuth (its radius: that of the jets).
//!
struct Cone
{
  double eta = 0.0;   //!< pseudorapidity of the axis
  double phi = 0.0;   //!< azimuth of the axis, [0, 2 pi)
};

//!
//! @brief Cones of the underlying event of an event, from its jets (found with one particle filter and settings).
//!
//! The axis is the leading jet (rank 1). Mode Transverse: two cones at phi_lead +- pi/2 and eta_lead. Mode
//! TransverseShifted: four cones at phi_lead +- pi/2 and eta_lead +- 2R. A cone is kept if it lies within the acceptance
//! of the particles, |eta| < particleEtaMax - R, and does not overlap any jet of the event (its distance to the axis of
//! every jet, in eta and phi, at least 2R). None without jets.
//!
class UnderlyingEventCones
{
public:

  //! @brief Placement of the cones.
  enum Mode { Transverse, TransverseShifted };

  //! @brief Mode from its name (TRANSVERSE, TRANSVERSE_SHIFTED). @param name name @return mode
  //! @throws Exception if unknown
  static Mode modeOf(const String & name);
  //! @brief Cones of an event. @param jets jets of the event, by rank @param settings settings of the jets
  //! @param mode placement @return cones kept
  static std::vector<Cone> cones(const std::vector<Jet> & jets, const JetSettings & settings, Mode mode);
  //! @brief Multiplicities of the particles within a distance R (eta, phi) of the axis of a cone. @param particles
  //! particles @param cone cone @param radius R @return multiplicities
  static Multiplicities count(const std::vector<Particle*> & particles, const Cone & cone, double radius);
};

} // namespace CAP

#endif /* CAP__JetUnderlyingEvent */
