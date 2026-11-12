/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
//! @file TestParticleEfficiency.cpp
//! @brief Tests of ParticleEfficiency: constant, function and histogram sources, correction weights, export and import, order of the variables, phi range,
//! clamping, validation, species, accepted fractions and reproducibility of the random numbers.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <vector>
#include "ParticleEfficiency.hpp"
#include "Histogram.hpp"
#include "Configuration.hpp"
#include "Folder.hpp"
#include "MersenneTwister.hpp"
#include "Particle.hpp"
#include "ParticleType.hpp"

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

//! A function of rank 1, 2 or 3 given by a lambda.
class Lambda : public MATH::Function
{
public:
  typedef double (*Body)(const std::vector<double> &);
  Lambda(unsigned int rank, Body body) : MATH::Function(rank), _body(body) {}
  double evaluateAt(const std::vector<double> & x) const override { checkRank(x.size(),__FUNCTION__); return _body(x); }
  using MATH::Function::evaluateAt;
private:
  Body _body;
};

//! Whether a call throws.
template <class F> bool throws(F f) { try { f(); } catch (const std::exception &) { return true; } return false; }

//! Fraction of n draws accepted for a particle.
double acceptedFraction(ParticleEfficiency & eff, const Particle & p, int n)
{
  int k = 0;
  for (int i=0; i<n; i++) if (eff.accept(p)) k++;
  return double(k)/n;
}
} // namespace

int main()
{
  ParticleType piPlus, piMinus;
  piPlus.setName("pi+");  piPlus.setPdgCode(211);   piPlus.setMass(0.13957);
  piMinus.setName("pi-"); piMinus.setPdgCode(-211); piMinus.setMass(0.13957);
  auto makeParticle = [](ParticleType * type, double pt, double eta, double phi)
    {
    Particle p;
    double px = pt*std::cos(phi), py = pt*std::sin(phi), pz = pt*std::sinh(eta);
    p.set(type,px,py,pz,std::sqrt(px*px+py*py+pz*pz+0.13957*0.13957),0.0,0.0,0.0,0.0);
    return p;
    };
  Particle pion = makeParticle(&piPlus,2.5,0.5,-M_PI/2.0);       // phi = -pi/2, i.e. 3pi/2 in [0, 2pi)
  const int n = 200000;

  // constant (default 1)
  ParticleEfficiency perfect;
  CHECK(perfect.isConstant() && perfect.constant() == 1.0 && perfect.variables().empty());
  CHECK(acceptedFraction(perfect,pion,1000) == 1.0);
  ParticleEfficiency blind(0.0);
  CHECK(acceptedFraction(blind,pion,1000) == 0.0);
  ParticleEfficiency seventy(0.7);
  seventy.setSeed(1);
  CHECK(near(acceptedFraction(seventy,pion,n),0.7,0.004));
  CHECK(throws([]{ ParticleEfficiency e(1.2); }) && throws([]{ ParticleEfficiency e(-0.1); }));

  // function of pT: pT/5; clamped to [0, 1]
  auto linear = std::make_shared<Lambda>(1,[](const std::vector<double> & x){ return x[0]/5.0; });
  ParticleEfficiency ofPt(linear);
  CHECK(!ofPt.isConstant() && ofPt.function() == linear.get() && ofPt.variables().size() == 1);
  CHECK(ofPt.variables()[0] == ParticleEfficiency::kPt);
  CHECK(near(ofPt.efficiency(pion),0.5,1e-12));
  CHECK(near(ofPt.efficiency(7.5,0.0,0.0),1.0,1e-12));          // 1.5 clamped to 1
  CHECK(near(ofPt.efficiency(-1.0,0.0,0.0),0.0,1e-12));         // -0.2 clamped to 0
  ofPt.setSeed(2);
  CHECK(near(acceptedFraction(ofPt,pion,n),0.5,0.004));

  // function of (eta, pT): the order of the variables given
  auto product = std::make_shared<Lambda>(2,[](const std::vector<double> & x){ return 0.2*x[0] + 0.1*x[1]; });
  ParticleEfficiency etaPt(product,{ParticleEfficiency::kEta,ParticleEfficiency::kPt});
  CHECK(near(etaPt.efficiency(pion),0.2*0.5+0.1*2.5,1e-12));
  ParticleEfficiency ptEta(product);                             // default for rank 2: (pT, eta)
  CHECK(near(ptEta.efficiency(pion),0.2*2.5+0.1*0.5,1e-12));
  // function of (pT, eta, phi): phi in [0, 2pi) by default
  auto ofPhi = std::make_shared<Lambda>(3,[](const std::vector<double> & x){ return x[2]/(2.0*M_PI); });
  ParticleEfficiency withPhi(ofPhi);
  CHECK(near(withPhi.efficiency(pion),0.75,1e-12));
  withPhi.setPhiMinimum(-M_PI);                                  // phi in [-pi, pi): -pi/2
  CHECK(near(withPhi.efficiency(pion),0.0,1e-12));               // -0.25 clamped to 0
  // the variables must match the rank and not repeat
  CHECK(throws([&]{ ParticleEfficiency e(product,{ParticleEfficiency::kPt}); }));
  CHECK(throws([&]{ ParticleEfficiency e(product,{ParticleEfficiency::kPt,ParticleEfficiency::kPt}); }));
  CHECK(throws([]{ std::shared_ptr<const MATH::Function> none; ParticleEfficiency e(none); }));

  // histogram of (pT, eta): bin values; 0 outside its range
  MATH::Histogram h("eff","eff",5,0.0,5.0,4,-1.0,1.0);
  for (unsigned int i=0; i<5; i++)
    for (unsigned int j=0; j<4; j++) h.setBinValueAt(i,j,0.1*i + 0.05*j);
  ParticleEfficiency fromHistogram(h);
  CHECK(fromHistogram.histogram() != nullptr && fromHistogram.histogram() != &h);   // a copy
  CHECK(near(fromHistogram.efficiency(pion),0.1*2 + 0.05*3,1e-6));                  // pT bin 2; eta 0.5: lower edge of bin 3
  CHECK(fromHistogram.efficiency(6.0,0.0,0.0) == 0.0 && fromHistogram.efficiency(1.0,1.5,0.0) == 0.0);
  fromHistogram.setSeed(3);
  CHECK(near(acceptedFraction(fromHistogram,pion,n),0.35,0.004));
  ParticleEfficiency swapped(h,{ParticleEfficiency::kEta,ParticleEfficiency::kPt});  // axis 0 read as eta
  CHECK(swapped.efficiency(pion) == 0.0);                       // eta 0.5 -> bin 0; pT 2.5 outside [-1, 1)
  CHECK(near(swapped.efficiency(0.5,2.5,0.0),0.1*2 + 0.05*3,1e-6));
  // a histogram with a value outside [0, 1] is refused
  MATH::Histogram bad("bad","bad",2,0.0,1.0);
  bad.setBinValueAt(0u,0.5); bad.setBinValueAt(1u,1.2);
  CHECK(throws([&]{ ParticleEfficiency e(bad); }));
  // phi axis: phi taken from its lower edge
  MATH::Histogram hPhi("phi","phi",1,0.0,10.0,1,-5.0,5.0,4,-M_PI,M_PI);
  for (unsigned int k=0; k<4; k++) hPhi.setBinValueAt(0,0,k,0.2*(k+1));
  ParticleEfficiency fromPhi(hPhi);
  CHECK(near(fromPhi.phiMinimum(),-M_PI,1e-6));                  // the edges are stored as float
  CHECK(near(fromPhi.efficiency(pion),0.4,1e-6));                // -pi/2: second bin of [-pi, pi)

  // species: pi+ only
  ParticleEfficiency pions(linear);
  pions.setSpecies({211});
  Particle negative = makeParticle(&piMinus,2.5,0.5,0.0);
  Particle untyped  = makeParticle(nullptr,2.5,0.5,0.0);
  CHECK(pions.appliesTo(pion) && !pions.appliesTo(negative) && !pions.appliesTo(untyped));
  CHECK(near(pions.efficiency(pion),0.5,1e-12) && pions.efficiency(negative) == 0.0);
  CHECK(acceptedFraction(pions,negative,1000) == 0.0);
  CHECK(ofPt.appliesTo(untyped));                               // no species given: all particles

  // random numbers: same seed, same decisions; a generator can be given; copies share the source
  ParticleEfficiency a(0.5), b(0.5);
  a.setSeed(42); b.setSeed(42);
  bool same = true;
  for (int i=0; i<1000; i++) same = same && (a.accept(pion) == b.accept(pion));
  CHECK(same);
  MATH::MersenneTwister g1(7), g2(7);
  bool sameGiven = true;
  for (int i=0; i<1000; i++) sameGiven = sameGiven && (a.accept(pion,g1) == b.accept(pion,g2));
  CHECK(sameGiven);
  ParticleEfficiency copy(fromHistogram);
  CHECK(copy.histogram() != fromHistogram.histogram() && copy.efficiency(pion) == fromHistogram.efficiency(pion));   // its own copy
  copy.setConstant(0.25);                                        // the copy changed, not the original
  CHECK(copy.isConstant() && !fromHistogram.isConstant());

  // correction weight: 1/efficiency, 0 where nothing is detected
  CHECK(near(ofPt.weight(pion),2.0,1e-12) && blind.weight(pion) == 0.0 && pions.weight(negative) == 0.0);

  // managed object: configured by keys, histogram created, exported and imported, function made by name
  Configuration c;
  c.addProperty("Task:PARTICLE_EFFICIENCY_0:EFFICIENCY:MODEL","HISTOGRAM");
  c.addProperty("Task:PARTICLE_EFFICIENCY_0:EFFICIENCY:VALUE",0.6);
  c.addProperty("Task:PARTICLE_EFFICIENCY_0:EFFICIENCY:VARIABLES","eta,pt");
  c.addProperty("Task:PARTICLE_EFFICIENCY_0:eta_nbins",4);
  c.addProperty("Task:PARTICLE_EFFICIENCY_0:eta_min",-2.0);
  c.addProperty("Task:PARTICLE_EFFICIENCY_0:eta_max",2.0);
  c.addProperty("Task:PARTICLE_EFFICIENCY_0:SPECIES","211, -211");
  ParticleEfficiency managed;
  managed.configure("Task","PARTICLE_EFFICIENCY",c,0);
  CHECK(managed.species().size() == 2 && managed.appliesTo(negative));
  CHECK(throws([&]{ managed.efficiency(pion); }));                // HISTOGRAM: neither created nor imported yet
  managed.create();
  CHECK(managed.histogram() != nullptr && managed.histogram()->rank() == 2);
  CHECK(managed.variables().size() == 2 && managed.variables()[0] == ParticleEfficiency::kEta);
  CHECK(near(managed.efficiency(pion),0.6,1e-6) && managed.efficiency(0.5,3.0,0.0) == 0.0);   // eta 3: outside
  Folder exported("exported");
  managed.saveTo(exported);
  ParticleEfficiency imported;
  imported.configure("Task","PARTICLE_EFFICIENCY",c,0);
  imported.loadFrom(exported);
  CHECK(imported.histogram() != nullptr && near(imported.efficiency(pion),0.6,1e-6));
  Configuration cf;
  cf.addProperty("Task:PARTICLE_EFFICIENCY_0:EFFICIENCY:MODEL","FUNCTION");
  cf.addProperty("Task:PARTICLE_EFFICIENCY_0:EFFICIENCY:FUNCTION","Linear");
  cf.addProperty("Task:PARTICLE_EFFICIENCY_0:EFFICIENCY:PARAMETERS","0.5, 0.1");
  ParticleEfficiency byName;
  byName.configure("Task","PARTICLE_EFFICIENCY",cf,0);
  bool functionOk = byName.function() != nullptr;
  CHECK(functionOk);
  if (functionOk) CHECK(near(byName.efficiency(pion),std::min(1.0,std::max(0.0,byName.function()->evaluateAt(2.5))),1e-12));
  Folder withFunction("withFunction");
  byName.saveTo(withFunction);
  CHECK(withFunction.folder(byName.folderName()) && withFunction.folder(byName.folderName())->contains(byName.mapFunctionName(0)));
  // self-describing files: an object configured otherwise (or not at all) takes the settings of the file
  ParticleEfficiency fresh;                                      // constant 1, all species
  fresh.loadFrom(exported);
  CHECK(fresh.histogram() != nullptr && fresh.variables().size() == 2 && fresh.variables()[0] == ParticleEfficiency::kEta);
  CHECK(fresh.species().size() == 2 && !fresh.appliesTo(untyped) && near(fresh.efficiency(pion),0.6,1e-6));
  ParticleEfficiency freshFunction;
  freshFunction.loadFrom(withFunction);
  CHECK(freshFunction.function() != nullptr && near(freshFunction.efficiency(pion),byName.efficiency(pion),1e-12));
  Folder constantFile("constant");
  ParticleEfficiency(0.3).saveTo(constantFile);
  ParticleEfficiency fromConstant(managed);                     // a histogram, replaced by the constant of the file
  fromConstant.loadFrom(constantFile);
  CHECK(fromConstant.isConstant() && near(fromConstant.constant(),0.3,1e-15));

  // one file, several objects: each in its own folder, listed by kind
  Folder shared("shared");
  ParticleEfficiency first(0.4), second(0.7);
  first.setName("First"); second.setName("Second");
  first.saveTo(shared); second.saveTo(shared);
  std::vector<String> names = ParticleEfficiency::objectNames(shared,"PARTICLE_EFFICIENCY");
  CHECK(names.size() == 2 && ParticleEfficiency::objectNames(shared,"PARTICLE_SMEARER").empty());
  ParticleEfficiency readSecond;
  readSecond.setName("Second");
  readSecond.loadFrom(shared);
  CHECK(near(readSecond.constant(),0.7,1e-15));
  // the binning used to create histograms is saved with the object
  Folder binned("binned");
  ParticleEfficiency withBinning(0.5);
  withBinning.setBinning(ParticleEfficiency::kEta,8,-0.8,0.8);
  withBinning.saveTo(binned);
  ParticleEfficiency readBinning;
  readBinning.loadFrom(binned);
  CHECK(readBinning.nBins(ParticleEfficiency::kEta) == 8 && near(readBinning.binMinimum(ParticleEfficiency::kEta),-0.8,1e-15)
        && near(readBinning.binMaximum(ParticleEfficiency::kEta),0.8,1e-15) && readBinning.nBins(ParticleEfficiency::kPt) == 100);
  // a file of the old layout (histogram at the top, named <BASE_NAME>_<KEY>): read as configured
  Folder legacy("legacy");
  MATH::Histogram old("old","old",2,0.0,10.0);
  old.setBinValueAt(0u,0.2); old.setBinValueAt(1u,0.9);
  legacy.add(new MATH::Histogram(old),"Det_EFFICIENCY");
  Configuration cl;
  cl.addProperty("Task:PARTICLE_EFFICIENCY_0:BASE_NAME","Det");
  cl.addProperty("Task:PARTICLE_EFFICIENCY_0:EFFICIENCY:MODEL","HISTOGRAM");
  ParticleEfficiency fromLegacy;
  fromLegacy.configure("Task","PARTICLE_EFFICIENCY",cl,0);
  fromLegacy.loadFrom(legacy);
  CHECK(near(fromLegacy.efficiency(7.0,0.0,0.0),0.9,1e-6));

  Configuration bad1, bad2;
  bad1.addProperty("Task:PARTICLE_EFFICIENCY_0:EFFICIENCY:MODEL","SOMETHING");
  bad2.addProperty("Task:PARTICLE_EFFICIENCY_0:EFFICIENCY:MODEL","FUNCTION");
  bad2.addProperty("Task:PARTICLE_EFFICIENCY_0:EFFICIENCY:FUNCTION","NoSuchFunction");
  CHECK(throws([&]{ ParticleEfficiency e; e.configure("Task","PARTICLE_EFFICIENCY",bad1,0); }));
  CHECK(throws([&]{ ParticleEfficiency e; e.configure("Task","PARTICLE_EFFICIENCY",bad2,0); }));
  ParticleEfficiency defaults;                                  // no keys: constant 1
  defaults.configure("Task","PARTICLE_EFFICIENCY",Configuration(),0);
  CHECK(defaults.isConstant() && defaults.constant() == 1.0);

  std::printf("TestParticleEfficiency: %d checks, %d failures\n", nChecks, nFailures);
  return nFailures == 0 ? 0 : 1;
}
