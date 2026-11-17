/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Jet.hpp
//! @brief Jet: a jet found by the jet finder (JetClustering), with its constituents (particles of the event), the
//! settings it was found with and its rank among the jets found with them; and the variables of a constituent relative
//! to a jet.
#ifndef DATUM__Jet
#define DATUM__Jet
#include <vector>
#include "VectorLorentz.hpp"
#include "String.hpp"
#include "JetSettings.hpp"

namespace Datum
{

class Particle;

//!
//! @brief A jet: its four-momentum, its constituents (the particles of the event clustered into it), the jet settings it
//! was found with (algorithm, R, recombination, pT floor) and its rank among all the jets of the event found with the
//! same particle filter and settings (1: leading, highest pT; 2: subleading; ...). A rank is meaningful only with its
//! settings: the jets found with other settings are ranked separately.
//!
struct Jet
{
  MATH::VectorLorentz momentum;          //!< four-momentum (E, px, py, pz)
  std::vector<Particle*> constituents;   //!< constituents (particles of the event)
  unsigned int rank = 0;                 //!< rank in pT among the jets of its settings (1: leading)
  const JetSettings * settings = nullptr; //!< settings the jet was found with (not owned)

  //! @brief Transverse momentum. @return pT
  double pt() const { return momentum.perp(); }
  //! @brief Pseudorapidity. @return eta
  double eta() const { return momentum.pseudorapidity(); }
  //! @brief Rapidity. @return y
  double rapidity() const { return momentum.rapidity(); }
  //! @brief Azimuth in [0, 2 pi). @return phi
  double phi() const;
  //! @brief Mass (0 if the four-momentum is space-like by rounding). @return mass
  double mass() const;
  //! @brief Number of constituents. @return number
  unsigned int multiplicity() const { return constituents.size(); }
};

//! @name Variables of a particle relative to a jet
//! @{
//! @brief Azimuth in [0, 2 pi) of a four-momentum. @param p momentum @return phi
double azimuth(const MATH::VectorLorentz & p);
//! @brief Azimuthal difference wrapped to [-pi, pi). @param phi1 azimuth @param phi2 azimuth @return phi1 - phi2
double deltaPhi(double phi1, double phi2);
//! @brief Distance in rapidity and azimuth, sqrt(dy^2 + dphi^2) (FastJet's distance). @param p particle momentum
//! @param jet jet momentum @return distance
double deltaR(const MATH::VectorLorentz & p, const MATH::VectorLorentz & jet);
//! @brief Momentum fraction z = pT/pT_jet. @param p particle momentum @param jet jet momentum @return z
double momentumFraction(const MATH::VectorLorentz & p, const MATH::VectorLorentz & jet);
//! @brief Momentum perpendicular to the jet axis, jT = |p x j|/|j|. @param p particle momentum @param jet jet momentum
//! @return jT
double transverseToJet(const MATH::VectorLorentz & p, const MATH::VectorLorentz & jet);
//! @brief Angle between the momentum of a particle and the jet axis. @param p particle momentum @param jet jet momentum
//! @return angle (rad)
double angleToJet(const MATH::VectorLorentz & p, const MATH::VectorLorentz & jet);
//! @}

} // namespace Datum

#endif /* DATUM__Jet */
