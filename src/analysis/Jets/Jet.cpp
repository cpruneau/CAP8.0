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
#include <string>
#include <tuple>
#include "Exceptions.hpp"

namespace CAP
{

namespace
{
const double kTwoPi = 2.0*M_PI;
} // namespace

bool JetSettings::operator<(const JetSettings & other) const
{
  return std::make_tuple(int(algorithm),radius,int(recombination),ptMin,particleEtaMax) <
         std::make_tuple(int(other.algorithm),other.radius,int(other.recombination),other.ptMin,other.particleEtaMax);
}

JetSettings::Algorithm JetSettings::algorithmOf(const String & name)
{
  String n = name;
  n.toUpper();
  if (n=="ANTIKT" || n=="ANTI_KT" || n=="ANTI-KT") return AntiKt;
  if (n=="KT")                                      return Kt;
  if (n=="CA" || n=="CAMBRIDGE" || n=="CAMBRIDGEAACHEN" || n=="CAMBRIDGE_AACHEN") return CambridgeAachen;
  throw Exception(String("Unknown jet algorithm: ")+name+" (ANTIKT, KT or CA)","JetSettings::algorithmOf()");
}

JetSettings::Recombination JetSettings::recombinationOf(const String & name)
{
  String n = name;
  n.toUpper();
  if (n=="E" || n=="E_SCHEME")    return EScheme;
  if (n=="PT" || n=="PT_SCHEME")  return PtScheme;
  throw Exception(String("Unknown recombination scheme: ")+name+" (E or PT)","JetSettings::recombinationOf()");
}

String JetSettings::radiusLabel(double radius)
{
  char text[32];
  std::snprintf(text,sizeof(text),"%g",radius);
  std::string s(text);
  for (char & c : s) if (c=='.') c = 'p';
  return String((std::string("R")+s).c_str());
}

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

} // namespace CAP
