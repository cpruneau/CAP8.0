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
//! @brief Jet: a jet found by the jet finder (JetClustering), with its constituents (particles of the event) and its
//! rank in the event; JetSettings: the settings of a jet finder; and the variables of a constituent relative to a jet.
#ifndef CAP__Jet
#define CAP__Jet
#include <vector>
#include "VectorLorentz.hpp"
#include "String.hpp"

namespace CAP
{

class Particle;

//!
//! @brief Settings of a jet finder: algorithm, radius, recombination scheme, minimum pT, and the pseudorapidity range of
//! the particles clustered (|eta| < particleEtaMax), from which the jets kept follow: |eta_jet| < particleEtaMax - R.
//!
struct JetSettings
{
  //! @brief Algorithms of FastJet.
  enum Algorithm { AntiKt, Kt, CambridgeAachen };
  //! @brief Recombination schemes: E (four-momenta added), Pt (pT-weighted, massless jets).
  enum Recombination { EScheme, PtScheme };

  Algorithm algorithm = AntiKt;           //!< algorithm
  double radius = 0.4;                    //!< radius R
  Recombination recombination = EScheme;  //!< recombination scheme
  double ptMin = 5.0;                     //!< minimum jet pT (GeV/c)
  double particleEtaMax = 0.9;            //!< pseudorapidity range of the particles clustered

  //! @brief Maximum |eta| of the jets kept (particleEtaMax - R). @return maximum
  double jetEtaMax() const { return particleEtaMax - radius; }
  //! @brief Whether two settings are the same. @param other settings @return true if so
  bool operator==(const JetSettings & other) const
  {
  return algorithm==other.algorithm && radius==other.radius && recombination==other.recombination &&
         ptMin==other.ptMin && particleEtaMax==other.particleEtaMax;
  }
  //! @brief Strict order (for maps). @param other settings @return true if before
  bool operator<(const JetSettings & other) const;
  //! @brief Algorithm from its name (ANTIKT, KT, CA). @param name name @return algorithm
  //! @throws Exception if the name is unknown
  static Algorithm algorithmOf(const String & name);
  //! @brief Recombination scheme from its name (E, PT). @param name name @return scheme @throws Exception if unknown
  static Recombination recombinationOf(const String & name);
  //! @brief Label of a radius in the names of the folders: R0p4 for 0.4. @param radius radius @return label
  static String radiusLabel(double radius);
};

//!
//! @brief A jet: its four-momentum, its constituents (the particles of the event clustered into it) and its rank among
//! the jets of the event found with the same particle filter and settings (1: leading, highest pT; 2: subleading; ...),
//! counted among the jets within the acceptance (|eta| < particleEtaMax - R) and above the minimum pT.
//!
struct Jet
{
  MATH::VectorLorentz momentum;          //!< four-momentum (E, px, py, pz)
  std::vector<Particle*> constituents;   //!< constituents (particles of the event)
  unsigned int rank = 0;                 //!< rank in pT in the event (1: leading)

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

} // namespace CAP

#endif /* CAP__Jet */
