/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
//! @file TestParticleSmear.cpp
//! @brief Tests of ParticleSmear: no smearing by default, constant, function and histogram resolutions, biases, the
//! distributions of the deviates (Gaussian, uniform, Laplace, Cauchy, custom), pT kept positive, phi range, the copy
//! of the generated particle, seeds, copies, configuration by keys, export and import.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <vector>
#include "Configuration.hpp"
#include "Folder.hpp"
#include "Histogram.hpp"
#include "MersenneTwister.hpp"
#include "Particle.hpp"
#include "ParticleSmear.hpp"
#include "ParticleType.hpp"
#include "Polynomial.hpp"

using namespace CAP;

namespace
{
int nChecks = 0, nFailures = 0;

void check(bool ok, const char * what, int line)
{
  nChecks++;
  if (ok) return;
  nFailures++;
  std::printf("FAILED (line %d): %s\n", line, what);
}
#define CHECK(x) check((x), #x, __LINE__)

bool near(double a, double b, double tolerance) { return std::fabs(a-b) <= tolerance; }
template <class F> bool throws(F f) { try { f(); } catch (const std::exception &) { return true; } return false; }

//! A function of rank 1 given by a lambda.
class Lambda : public MATH::Function
{
public:
  typedef double (*Body)(const std::vector<double> &);
  Lambda(Body body) : MATH::Function(1), _body(body) {}
  double evaluateAt(const std::vector<double> & x) const override { checkRank(x.size(),__FUNCTION__); return _body(x); }
  using MATH::Function::evaluateAt;
private:
  Body _body;
};

const double mass = 0.13957;

Particle makeParticle(ParticleType * type, double pt, double eta, double phi)
{
  Particle p;
  double px = pt*std::cos(phi), py = pt*std::sin(phi), pz = pt*std::sinh(eta);
  p.set(type,px,py,pz,std::sqrt(px*px+py*py+pz*pz+mass*mass),0.1,0.2,0.3,0.0);
  return p;
}

//! Statistics of reco - gen of an observable over n smearings.
struct Stats { double mean=0, rms=0, kurtosis=0, maxAbs=0, medianAbs=0, minPt=1e9; };
Stats stats(ParticleSmear & s, Particle & gen, int observable, int n)
{
  std::vector<double> d(n);
  Particle reco;
  Stats r;
  for (int i=0; i<n; i++)
    {
    s.smear(gen,reco);
    const VectorLorentz & a = reco.momentum(), & b = gen.momentum();
    double v = observable==0 ? a.perp()-b.perp() : observable==1 ? a.pseudorapidity()-b.pseudorapidity() : a.phi()-b.phi();
    d[i] = v;
    r.minPt = std::min(r.minPt,a.perp());
    }
  for (double v : d) r.mean += v;
  r.mean /= n;
  double m2 = 0.0, m4 = 0.0;
  for (double v : d) { double x = v-r.mean; m2 += x*x; m4 += x*x*x*x; r.maxAbs = std::max(r.maxAbs,std::fabs(v)); }
  r.rms = std::sqrt(m2/n);
  r.kurtosis = (m4/n)/((m2/n)*(m2/n));
  std::vector<double> a(n);
  for (int i=0; i<n; i++) a[i] = std::fabs(d[i]);               // about 0 (a Cauchy sample has no stable mean)
  std::nth_element(a.begin(),a.begin()+n/2,a.end());
  r.medianAbs = a[n/2];
  return r;
}
} // namespace

int main()
{
  ParticleType pion;
  pion.setName("pi+"); pion.setPdgCode(211); pion.setMass(mass);
  Particle gen = makeParticle(&pion,5.0,0.5,1.0);
  gen.setPid(7);
  const int n = 200000;

  // no resolution: no smearing; the smeared particle is a full copy with the generated one as truth particle
  ParticleSmear none;
  CHECK(!none.smears(ParticleSmear::kPt) && !none.smears(ParticleSmear::kEta) && !none.smears(ParticleSmear::kPhi));
  Particle stale;
  Particle other;                                                   // stale links of a reused factory object
  stale.setParent(&other); stale.addChild(&other); stale._ixEtaPhi = 5;
  none.smear(gen,stale);
  CHECK(stale.truth() == &gen && !stale.hasParent() && !stale.hasChildren() && stale._ixEtaPhi == -1);
  CHECK(stale.typePtr() == &pion && stale.pid() == 7 && near(stale.position().z(),0.3,1e-12));
  CHECK(near(stale.momentum().perp(),5.0,1e-12) && near(stale.momentum().pseudorapidity(),0.5,1e-12));
  CHECK(near(stale.momentum().phi(),1.0,1e-12) && near(stale.momentum().modulus(),mass,1e-9));

  // constant Gaussian resolutions and a pT bias
  ParticleSmear gauss;
  gauss.setResolution(ParticleSmear::kPt,0.10);
  gauss.setBias(ParticleSmear::kPt,0.05);
  gauss.setResolution(ParticleSmear::kEta,0.02);
  gauss.setResolution(ParticleSmear::kPhi,0.01);
  gauss.setSeed(1);
  CHECK(gauss.distribution(ParticleSmear::kPt) == ParticleSmear::kGaussian);
  Stats sPt = stats(gauss,gen,0,n), sEta = stats(gauss,gen,1,n), sPhi = stats(gauss,gen,2,n);
  CHECK(near(sPt.mean,0.05,0.001) && near(sPt.rms,0.10,0.001) && near(sPt.kurtosis,3.0,0.05));
  CHECK(near(sEta.mean,0.0,0.0002) && near(sEta.rms,0.02,0.0002));
  CHECK(near(sPhi.mean,0.0,0.0001) && near(sPhi.rms,0.01,0.0001));
  Particle reco;
  gauss.smear(gen,reco);
  CHECK(near(reco.momentum().modulus(),mass,1e-9));                // mass kept
  CHECK(throws([&]{ gauss.setResolution(ParticleSmear::kPt,-0.1); }));

  // only eta smeared: pT and phi unchanged
  ParticleSmear etaOnly;
  etaOnly.setResolution(ParticleSmear::kEta,0.05);
  etaOnly.smear(gen,reco);
  CHECK(near(reco.momentum().perp(),5.0,1e-12) && near(reco.momentum().phi(),1.0,1e-12));
  CHECK(!near(reco.momentum().pseudorapidity(),0.5,1e-12));

  // function: relative pT resolution 2%
  ParticleSmear relative;
  relative.setResolution(ParticleSmear::kPt,std::make_shared<Lambda>([](const std::vector<double> & x){ return 0.02*x[0]; }));
  relative.setSeed(2);
  CHECK(near(relative.resolution(ParticleSmear::kPt,5.0,0.0,0.0),0.10,1e-12));
  Particle gen1 = makeParticle(&pion,1.0,0.5,1.0);
  CHECK(near(stats(relative,gen,0,n).rms,0.10,0.001) && near(stats(relative,gen1,0,n).rms,0.02,0.0002));

  // histogram of eta: resolution by bin; outside the histogram: no smearing; negative values refused
  MATH::Histogram h("sigmaEta","sigmaEta",2,0.0,1.0);
  h.setBinValueAt(0u,0.01); h.setBinValueAt(1u,0.03);
  ParticleSmear fromHistogram;
  fromHistogram.setResolution(ParticleSmear::kEta,h,{KinematicMap::kEta});
  fromHistogram.setSeed(3);
  CHECK(near(stats(fromHistogram,gen,1,n).rms,0.03,0.0003));        // eta 0.5: second bin
  Particle forward = makeParticle(&pion,5.0,2.0,1.0);
  fromHistogram.smear(forward,reco);
  CHECK(near(reco.momentum().pseudorapidity(),2.0,1e-12));          // eta 2: outside, not smeared
  MATH::Histogram negative("neg","neg",1,0.0,1.0);
  negative.setBinValueAt(0u,-0.1);
  CHECK(throws([&]{ fromHistogram.setResolution(ParticleSmear::kEta,negative); }));

  // distributions (deviates in units of sigma = 0.01 on eta)
  ParticleSmear d;
  d.setResolution(ParticleSmear::kEta,0.01);
  d.setSeed(4);
  d.setDistribution(ParticleSmear::kEta,ParticleSmear::kUniform);
  Stats u = stats(d,gen,1,n);
  CHECK(near(u.rms,0.01,0.0001) && u.maxAbs <= std::sqrt(3.0)*0.01*(1+1e-9) && near(u.kurtosis,1.8,0.02));
  d.setDistribution(ParticleSmear::kEta,ParticleSmear::kLaplace);
  Stats l = stats(d,gen,1,n);
  CHECK(near(l.rms,0.01,0.0002) && near(l.kurtosis,6.0,0.4));
  d.setDistribution(ParticleSmear::kEta,ParticleSmear::kCauchy);
  Stats c = stats(d,gen,1,n);
  CHECK(near(c.medianAbs,0.01,0.0002));                              // half width at half maximum: median of |deviate|
  d.setDeviate(ParticleSmear::kEta,[](MATH::RandomGenerator &){ return 2.0; });
  CHECK(d.distribution(ParticleSmear::kEta) == ParticleSmear::kCustom);
  d.smear(gen,reco);
  CHECK(near(reco.momentum().pseudorapidity(),0.5+2.0*0.01,1e-12));
  CHECK(throws([&]{ d.setDistribution(ParticleSmear::kEta,ParticleSmear::kCustom); }));
  CHECK(throws([&]{ d.setDeviate(ParticleSmear::kEta,ParticleSmear::Deviate()); }));

  // pT kept positive; phi in (-pi, pi]
  ParticleSmear wide;
  wide.setResolution(ParticleSmear::kPt,1.0);
  wide.setResolution(ParticleSmear::kPhi,0.5);
  Particle soft = makeParticle(&pion,0.1,0.0,3.1);
  Stats w = stats(wide,soft,0,20000);
  CHECK(w.minPt > 0.0);
  bool phiOk = true;
  for (int i=0; i<20000; i++) { wide.smear(soft,reco); double p = reco.momentum().phi(); phiOk = phiOk && p > -M_PI-1e-12 && p <= M_PI+1e-12; }
  CHECK(phiOk);

  // seeds: same seed, same smearing; a generator can be given
  ParticleSmear a(gauss), b(gauss);
  a.setSeed(9); b.setSeed(9);
  Particle ra, rb;
  bool same = true;
  for (int i=0; i<1000; i++) { a.smear(gen,ra); b.smear(gen,rb); same = same && ra.momentum().perp()==rb.momentum().perp(); }
  CHECK(same);
  MATH::MersenneTwister g1(5), g2(5);
  a.smear(gen,ra,g1); b.smear(gen,rb,g2);
  CHECK(ra.momentum().pseudorapidity() == rb.momentum().pseudorapidity());

  // copies: the histogram copied, the map of the copy pointing to its own
  ParticleSmear copy(fromHistogram);
  CHECK(copy.resolutionMap(ParticleSmear::kEta).histogram() != fromHistogram.resolutionMap(ParticleSmear::kEta).histogram());
  CHECK(copy.resolutionMap(ParticleSmear::kEta).histogram() == copy.histogram(copy.mapHistogramName(1)));
  CHECK(near(copy.resolution(ParticleSmear::kEta,5.0,0.5,1.0),0.03,1e-6));

  // configuration by keys, histograms created, exported and imported
  Configuration conf;
  conf.addProperty("Task:PARTICLE_SMEARER_0:PT:MODEL","CONSTANT");
  conf.addProperty("Task:PARTICLE_SMEARER_0:PT:VALUE",0.2);
  conf.addProperty("Task:PARTICLE_SMEARER_0:PT:DISTRIBUTION","UNIFORM");
  conf.addProperty("Task:PARTICLE_SMEARER_0:ETA:MODEL","HISTOGRAM");
  conf.addProperty("Task:PARTICLE_SMEARER_0:ETA:VALUE",0.04);
  conf.addProperty("Task:PARTICLE_SMEARER_0:ETA:VARIABLES","pt,eta");
  conf.addProperty("Task:PARTICLE_SMEARER_0:PHI_BIAS:MODEL","CONSTANT");
  conf.addProperty("Task:PARTICLE_SMEARER_0:PHI_BIAS:VALUE",0.001);
  ParticleSmear managed;
  managed.configure("Task","PARTICLE_SMEARER",conf,0);
  CHECK(managed.smears(ParticleSmear::kPt) && managed.distribution(ParticleSmear::kPt) == ParticleSmear::kUniform);
  CHECK(!managed.smears(ParticleSmear::kPhi) && near(managed.bias(ParticleSmear::kPhi,1,0,0),0.001,1e-12));
  CHECK(throws([&]{ managed.resolution(ParticleSmear::kEta,5.0,0.5,1.0); }));       // HISTOGRAM: not there yet
  managed.create();
  CHECK(managed.resolutionMap(ParticleSmear::kEta).histogram()->rank() == 2);
  CHECK(near(managed.resolution(ParticleSmear::kEta,5.0,0.5,1.0),0.04,1e-6));
  Folder file("file");
  managed.saveTo(file);
  ParticleSmear imported;
  imported.configure("Task","PARTICLE_SMEARER",conf,0);
  imported.loadFrom(file);
  CHECK(near(imported.resolution(ParticleSmear::kEta,5.0,0.5,1.0),0.04,1e-6));
  // self-describing files: everything restored by an object without configuration
  ParticleSmear fresh;
  fresh.loadFrom(file);
  CHECK(fresh.smears(ParticleSmear::kPt) && fresh.distribution(ParticleSmear::kPt) == ParticleSmear::kUniform);
  CHECK(near(fresh.resolution(ParticleSmear::kPt,1,0,0),0.2,1e-15) && near(fresh.bias(ParticleSmear::kPhi,1,0,0),0.001,1e-15));
  CHECK(fresh.resolutionMap(ParticleSmear::kEta).histogram() != nullptr && fresh.resolutionMap(ParticleSmear::kEta).variables().size() == 2);
  CHECK(!fresh.smears(ParticleSmear::kPhi) && fresh.biasMap(ParticleSmear::kPt).isNone());
  ParticleSmear withPolynomial;
  auto poly = std::make_shared<MATH::Polynomial>(std::vector<double>{0.001,0.02});
  withPolynomial.setResolution(ParticleSmear::kPt,poly);
  withPolynomial.setDistribution(ParticleSmear::kPt,ParticleSmear::kLaplace);
  Folder polyFile("poly");
  withPolynomial.saveTo(polyFile);
  ParticleSmear freshPoly;
  freshPoly.loadFrom(polyFile);
  CHECK(freshPoly.resolutionMap(ParticleSmear::kPt).function() != nullptr);
  CHECK(near(freshPoly.resolution(ParticleSmear::kPt,5.0,0,0),0.001+0.02*5.0,1e-12));
  CHECK(freshPoly.distribution(ParticleSmear::kPt) == ParticleSmear::kLaplace);
  ParticleSmear custom;                                          // a function of the user cannot be saved: no source
  custom.setResolution(ParticleSmear::kEta,std::make_shared<Lambda>([](const std::vector<double> &){ return 0.1; }));
  Folder customFile("custom");
  custom.saveTo(customFile);
  ParticleSmear freshCustom;
  freshCustom.setResolution(ParticleSmear::kEta,0.5);
  freshCustom.loadFrom(customFile);
  CHECK(!freshCustom.smears(ParticleSmear::kEta));

  Configuration badDistribution;
  badDistribution.addProperty("Task:PARTICLE_SMEARER_0:ETA:DISTRIBUTION","TRIANGLE");
  CHECK(throws([&]{ ParticleSmear s; s.configure("Task","PARTICLE_SMEARER",badDistribution,0); }));
  ParticleSmear defaults;                                           // no keys: nothing smeared
  defaults.configure("Task","PARTICLE_SMEARER",Configuration(),0);
  CHECK(!defaults.smears(ParticleSmear::kPt) && !defaults.smears(ParticleSmear::kEta));

  std::printf("TestParticleSmear: %d checks, %d failures\n", nChecks, nFailures);
  return nFailures == 0 ? 0 : 1;
}
