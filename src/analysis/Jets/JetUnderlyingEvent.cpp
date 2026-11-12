/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "JetUnderlyingEvent.hpp"
#include <cmath>
#include "Exceptions.hpp"
#include "Particle.hpp"
#include "ParticleType.hpp"

namespace CAP
{

Multiplicities constituentMultiplicities(const Jet & jet)
{
  Multiplicities m;
  for (const Particle * particle : jet.constituents) m.add(particle->type().charge());
  return m;
}

UnderlyingEventCones::Mode UnderlyingEventCones::modeOf(const String & name)
{
  String n = name;
  n.toUpper();
  if (n=="TRANSVERSE")         return Transverse;
  if (n=="TRANSVERSE_SHIFTED") return TransverseShifted;
  throw Exception(String("Unknown placement of the underlying event cones: ")+name+" (TRANSVERSE or TRANSVERSE_SHIFTED)",
                  "UnderlyingEventCones::modeOf()");
}

std::vector<Cone> UnderlyingEventCones::cones(const std::vector<Jet> & jets, const JetSettings & settings, Mode mode)
{
  std::vector<Cone> kept;
  if (jets.empty()) return kept;
  const Jet & leading = jets.front();
  const double R = settings.radius, etaMax = settings.particleEtaMax-R;
  std::vector<double> etas;
  if (mode==Transverse) etas = {leading.eta()};
  else                  etas = {leading.eta()+2.0*R, leading.eta()-2.0*R};
  for (double sign : {1.0, -1.0})
    for (double eta : etas)
      {
      Cone cone;
      cone.eta = eta;
      cone.phi = std::fmod(leading.phi()+sign*0.5*M_PI+2.0*M_PI,2.0*M_PI);
      if (!(std::fabs(cone.eta)<etaMax)) continue;
      bool overlap = false;
      for (const Jet & jet : jets)
        {
        double dEta = jet.eta()-cone.eta, dPhi = deltaPhi(jet.phi(),cone.phi);
        if (dEta*dEta+dPhi*dPhi<4.0*R*R) { overlap = true; break; }
        }
      if (!overlap) kept.push_back(cone);
      }
  return kept;
}

Multiplicities UnderlyingEventCones::count(const std::vector<Particle*> & particles, const Cone & cone, double radius)
{
  Multiplicities m;
  const double r2 = radius*radius;
  for (const Particle * particle : particles)
    {
    const MATH::VectorLorentz & p = particle->momentum();
    double dEta = p.pseudorapidity()-cone.eta, dPhi = deltaPhi(azimuth(p),cone.phi);
    if (dEta*dEta+dPhi*dPhi<r2) m.add(particle->type().charge());
    }
  return m;
}

} // namespace CAP
