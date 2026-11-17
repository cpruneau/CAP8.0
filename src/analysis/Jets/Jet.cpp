/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "Jet.hpp"
#include <cmath>
#include <cstdio>

namespace Datum
{

namespace
{
const double kTwoPi = 2.0*M_PI;
} // namespace

double Jet::phi() const
{
  return azimuth(momentum);
}

double Jet::mass() const
{
  double m2 = momentum.t()*momentum.t() - momentum.x()*momentum.x() - momentum.y()*momentum.y() - momentum.z()*momentum.z();
  return m2>0.0 ? std::sqrt(m2) : 0.0;
}

double azimuth(const MATH::VectorLorentz & p)
{
  double phi = std::atan2(p.y(),p.x());
  if (phi<0.0) phi += kTwoPi;
  if (phi>=kTwoPi) phi -= kTwoPi;
  return phi;
}

double deltaPhi(double phi1, double phi2)
{
  double d = std::fmod(phi1-phi2+M_PI,kTwoPi);
  if (d<0.0) d += kTwoPi;
  return d-M_PI;
}

double deltaR(const MATH::VectorLorentz & p, const MATH::VectorLorentz & jet)
{
  double dy = p.rapidity()-jet.rapidity();
  double dphi = deltaPhi(azimuth(p),azimuth(jet));
  return std::sqrt(dy*dy+dphi*dphi);
}

double momentumFraction(const MATH::VectorLorentz & p, const MATH::VectorLorentz & jet)
{
  double ptJet = jet.perp();
  return ptJet>0.0 ? p.perp()/ptJet : 0.0;
}

double transverseToJet(const MATH::VectorLorentz & p, const MATH::VectorLorentz & jet)
{
  double jx = jet.x(), jy = jet.y(), jz = jet.z();
  double j = std::sqrt(jx*jx+jy*jy+jz*jz);
  if (!(j>0.0)) return 0.0;
  double cx = p.y()*jz - p.z()*jy;
  double cy = p.z()*jx - p.x()*jz;
  double cz = p.x()*jy - p.y()*jx;
  return std::sqrt(cx*cx+cy*cy+cz*cz)/j;
}

double angleToJet(const MATH::VectorLorentz & p, const MATH::VectorLorentz & jet)
{
  double jx = jet.x(), jy = jet.y(), jz = jet.z();
  double px = p.x(), py = p.y(), pz = p.z();
  double j = std::sqrt(jx*jx+jy*jy+jz*jz), q = std::sqrt(px*px+py*py+pz*pz);
  if (!(j>0.0) || !(q>0.0)) return 0.0;
  double c = (px*jx+py*jy+pz*jz)/(j*q);
  if (c>1.0) c = 1.0;
  if (c<-1.0) c = -1.0;
  return std::acos(c);
}

} // namespace Datum
