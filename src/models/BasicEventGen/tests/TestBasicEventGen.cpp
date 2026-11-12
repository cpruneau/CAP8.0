/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
//! @file TestBasicEventGen.cpp
//! @brief Tests of the momentum generation of BasicEventGen: the cylindrical (y, pT, phi) and spherical (p, theta,
//! phi) styles, the transverse-momentum and momentum ranges, the exponential, Gaussian and user-function
//! distributions, the list of particle types, and the errors on invalid settings.
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include "BasicEventGen.hpp"
#include "Exceptions.hpp"
#include "MathConstants.hpp"
#include "ParticleDb.hpp"

using namespace CAP;

namespace
{
int nChecks = 0, nFailures = 0;

void check(bool ok, const std::string & what, int line)
{
  nChecks++;
  if (ok) return;
  nFailures++;
  std::printf("FAILED (line %d): %s\n", line, what.c_str());
}

//! The generator with its protected steps made public.
class TestGen : public BasicEventGen
{
public:
  TestGen() { setDefaultConfiguration(); }
  void set(const char * k, const String & v) { _configuration.addProperty(createKey(name(),k),v); }
  void set(const char * k, double v)         { _configuration.addProperty(createKey(name(),k),v); }
  void set(const char * k, int v)            { _configuration.addProperty(createKey(name(),k),v); }
  void setup(const ParticleDb & db)          { readSettings(); selectTypes(db); }
  MATH::VectorLorentz cylindrical(double m)  { MATH::VectorLorentz v; generateCylindrical(m,v); return v; }
  MATH::VectorLorentz spherical(double m)    { MATH::VectorLorentz v; generateSpherical(m,v); return v; }
  const std::vector<ParticleType*> & types() const { return _types; }
  void setPsi(double psi)                    { _psi = psi; }
};

//! Mean and standard error of a sample.
struct Moments
{
  double n=0, s=0, s2=0;
  void add(double x) { n++; s+=x; s2+=x*x; }
  double mean() const { return s/n; }
  double error() const { double m=mean(); return std::sqrt(std::max(0.0,s2/n-m*m)/n); }
};

double pt(const MATH::VectorLorentz & v) { return std::sqrt(v[1]*v[1]+v[2]*v[2]); }
double p(const MATH::VectorLorentz & v)  { return std::sqrt(v[1]*v[1]+v[2]*v[2]+v[3]*v[3]); }
double rapidity(const MATH::VectorLorentz & v) { return 0.5*std::log((v[0]+v[3])/(v[0]-v[3])); }

bool throws(TestGen & gen, const ParticleDb & db)
{
  try { gen.setup(db); }
  catch (const ConfigurationException &) { return true; }
  return false;
}
} // namespace

int main()
{
  ParticleDb db;
  db.loadFromXml(CAP_PARTICLE_DB);
  const int n = 200000;
  const double mPi = 0.13957;

  // cylindrical, exponential pT in [0, 10]: <pT> = slope (truncation negligible), y and phi uniform
  {
  TestGen gen;
  gen.set("Slope",0.4);
  gen.setup(db);
  Moments mpt, my, mphi;
  bool inRange = true;
  for (int i=0; i<n; i++)
    {
    MATH::VectorLorentz v = gen.cylindrical(mPi);
    double y = rapidity(v), phi = std::atan2(v[2],v[1]);
    if (phi<0) phi += MATH::twoPi();
    mpt.add(pt(v)); my.add(y); mphi.add(phi);
    inRange = inRange && pt(v)<=10.0 && y>=-2.0-1e-9 && y<=2.0+1e-9;
    }
  check(std::fabs(mpt.mean()-0.4)<5*mpt.error(), "cylindrical exponential: <pT> = slope", __LINE__);
  check(std::fabs(my.mean())<5*my.error(), "cylindrical: <y> = 0", __LINE__);
  check(std::fabs(mphi.mean()-MATH::pi())<5*mphi.error(), "cylindrical: <phi> = pi", __LINE__);
  check(inRange, "cylindrical: pT and y in their ranges", __LINE__);
  check(gen.types().size()==4, "default PdgSelected: four types", __LINE__);
  }

  // cylindrical, Gaussian pT in [0.2, 2] (tabulated): mean of the truncated half Gaussian
  {
  TestGen gen;
  gen.set("Distribution",String("GAUSSIAN"));
  gen.set("Sigma",0.5);
  gen.set("PtMin",0.2);
  gen.set("PtMax",2.0);
  gen.setup(db);
  Moments mpt;
  double lo=1e9, hi=-1e9;
  for (int i=0; i<n; i++) { double x = pt(gen.cylindrical(mPi)); mpt.add(x); lo=std::min(lo,x); hi=std::max(hi,x); }
  // expected mean: integral of x exp(-x^2/2s^2) / integral of exp(-x^2/2s^2) on [0.2, 2]
  const double s = 0.5;
  double num = s*s*(std::exp(-0.02/(s*s))-std::exp(-2.0/(s*s)));
  double den = s*std::sqrt(MATH::pi()/2.0)*(std::erf(2.0/(s*std::sqrt(2.0)))-std::erf(0.2/(s*std::sqrt(2.0))));
  check(std::fabs(mpt.mean()-num/den)<5*mpt.error(), "cylindrical Gaussian: <pT> of the truncated Gaussian", __LINE__);
  check(lo>=0.2 && hi<=2.0, "cylindrical Gaussian: pT in [PtMin, PtMax]", __LINE__);
  }

  // cylindrical with a momentum range: every particle has p in [PMin, PMax]
  {
  TestGen gen;
  gen.set("MomentumRange",String("P"));
  gen.set("PMin",0.5);
  gen.set("PMax",3.0);
  gen.setup(db);
  bool ok = true;
  for (int i=0; i<20000; i++) { double x = p(gen.cylindrical(mPi)); ok = ok && x>=0.5 && x<=3.0; }
  check(ok, "cylindrical with MomentumRange = P: p in [PMin, PMax]", __LINE__);
  }

  // spherical, exponential p in [0, 10]: <p> = slope, isotropic: <cos theta> = 0, <cos^2 theta> = 1/3
  {
  TestGen gen;
  gen.set("EventGenType",int(BasicEventGen::kSpherical));
  gen.set("Slope",0.6);
  gen.setup(db);
  Moments mp, mc, mc2;
  for (int i=0; i<n; i++)
    {
    MATH::VectorLorentz v = gen.spherical(mPi);
    double c = v[3]/p(v);
    mp.add(p(v)); mc.add(c); mc2.add(c*c);
    }
  check(std::fabs(mp.mean()-0.6)<5*mp.error(), "spherical exponential: <p> = slope", __LINE__);
  check(std::fabs(mc.mean())<5*mc.error(), "spherical: <cos theta> = 0", __LINE__);
  check(std::fabs(mc2.mean()-1.0/3.0)<5*mc2.error(), "spherical: isotropic, <cos^2 theta> = 1/3", __LINE__);
  }

  // spherical, user function: linear density f(p) = p on [0, 1] (Linear: a0 + a1 x), <p> = 2/3; theta in a range
  {
  TestGen gen;
  gen.set("EventGenType",int(BasicEventGen::kSpherical));
  gen.set("Distribution",String("FUNCTION"));
  gen.set("Function:Name",String("Linear"));
  gen.set("Function:Parameters",String("0, 1"));
  gen.set("PMin",0.0);
  gen.set("PMax",1.0);
  gen.set("ThetaMin",0.5);
  gen.set("ThetaMax",1.0);
  gen.setup(db);
  Moments mp;
  bool ok = true;
  for (int i=0; i<n; i++)
    {
    MATH::VectorLorentz v = gen.spherical(mPi);
    double theta = std::acos(v[3]/p(v));
    mp.add(p(v));
    ok = ok && theta>=0.5-1e-9 && theta<=1.0+1e-9;
    }
  check(std::fabs(mp.mean()-2.0/3.0)<5*mp.error(), "spherical user function f(p) = p: <p> = 2/3", __LINE__);
  check(ok, "spherical: theta in [ThetaMin, ThetaMax]", __LINE__);
  }

  // flow: <cos(n(phi - Psi))> = v_n, constant or rising linearly with pT up to PtScale
  {
  TestGen gen;
  gen.set("Flow:Vn",String("0, 0.1, 0.05"));
  gen.setup(db);
  gen.setPsi(1.0);
  Moments c1, c2, c3;
  for (int i=0; i<n; i++)
    {
    MATH::VectorLorentz v = gen.cylindrical(mPi);
    double d = std::atan2(v[2],v[1])-1.0;
    c1.add(std::cos(d)); c2.add(std::cos(2*d)); c3.add(std::cos(3*d));
    }
  check(std::fabs(c1.mean())<5*c1.error(), "flow: <cos(phi - Psi)> = v1 = 0", __LINE__);
  check(std::fabs(c2.mean()-0.1)<5*c2.error(), "flow: <cos 2(phi - Psi)> = v2", __LINE__);
  check(std::fabs(c3.mean()-0.05)<5*c3.error(), "flow: <cos 3(phi - Psi)> = v3", __LINE__);
  TestGen rising;
  rising.set("Flow:Vn",String("0, 0.2"));
  rising.set("Flow:PtScale",2.0);
  rising.set("Slope",1.0);
  rising.setup(db);
  rising.setPsi(0.0);
  Moments low, high;
  for (int i=0; i<n; i++)
    {
    MATH::VectorLorentz v = rising.cylindrical(mPi);
    double x = pt(v), c = std::cos(2*std::atan2(v[2],v[1]));
    if (x>0.4 && x<0.6) low.add(c/(x/2.0));        // v2(pT) = 0.2 pT/2 below 2
    if (x>2.0) high.add(c);                          // v2 = 0.2 above
    }
  check(std::fabs(low.mean()-0.2)<5*low.error(), "flow: v2(pT) = v2 pT/PtScale below PtScale", __LINE__);
  check(std::fabs(high.mean()-0.2)<5*high.error(), "flow: v2(pT) = v2 above PtScale", __LINE__);
  TestGen bad; bad.set("Flow:Vn",String("0.3, 0.3"));
  check(throws(bad,db), "flow: sum of |v_n| >= 1/2: ConfigurationException", __LINE__);
  }

  // particle types and invalid settings
  {
  TestGen gen;
  gen.set("PdgSelected",String("2212 -2212; 3122"));
  gen.setup(db);
  check(gen.types().size()==3 && gen.types()[2]->pdgCode()==3122, "PdgSelected: list of PDG codes", __LINE__);
  TestGen bad1; bad1.set("PdgSelected",String("211, 999999"));
  check(throws(bad1,db), "unknown PDG code: ConfigurationException", __LINE__);
  TestGen bad2; bad2.set("EventGenType",7);
  check(throws(bad2,db), "unknown EventGenType: ConfigurationException", __LINE__);
  TestGen bad3; bad3.set("MultMin",30); bad3.set("MultMax",10);
  check(throws(bad3,db), "MultMin > MultMax: ConfigurationException", __LINE__);
  TestGen bad4; bad4.set("Distribution",String("FUNCTION")); bad4.set("Function:Name",String("NoSuchFunction"));
  check(throws(bad4,db), "unknown function: ConfigurationException", __LINE__);
  TestGen bad5; bad5.set("MomentumRange",String("Q"));
  check(throws(bad5,db), "unknown MomentumRange: ConfigurationException", __LINE__);
  }

  std::printf("TestBasicEventGen: %d checks, %d failures\n", nChecks, nFailures);
  return nFailures==0 ? 0 : 1;
}
