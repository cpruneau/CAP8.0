/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
//! @file TestParticleDecays.cpp
//! @brief Decays of DB/ParticleData/ParticleDb.xml by ParticleDecayerTask: K0 -> K0S or K0L (one-product decay mode,
//! same momentum and position) and their decays, Lambda -> p pi-, n pi0 at the decay length of its lifetime,
//! conservation of the charge and of the energy-momentum, and the branching ratios of these types summing to 1.
#include <cmath>
#include <cstdio>
#include <map>
#include "Event.hpp"
#include "MersenneTwister.hpp"
#include "Particle.hpp"
#include "ParticleDb.hpp"
#include "ParticleDecayerTask.hpp"

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

//! Decays n particles of the given type with momentum pz along z, from the origin; returns the final particles by
//! PDG code (per decayed particle), the mean distance of the first decay from the origin (fm), and whether the
//! charge, the energy and the momentum were conserved in every event.
struct Result { std::map<int,double> perParticle; double meanDistance = 0; bool conserved = true; };

Result decayMany(ParticleType & type, double pz, int n)
{
  Result result;
  ParticleDecayerTask decayer;
  Event event;
  double sumDistance = 0;
  for (int i=0; i<n; i++)
    {
    event.clear();
    Particle::factory().reset();
    Particle & parent = Particle::factory().nextObject();
    parent.setType(&type);
    parent.setLive(true);
    double m = type.mass();
    parent.momentum().setTXYZ(std::sqrt(m*m+pz*pz),0.0,0.0,pz);
    parent.position().setTXYZ(0.0,0.0,0.0,0.0);
    event.addParticle(&parent);
    decayer.decayParticle(parent,event);
    double q = 0, e = 0, p = 0;
    bool first = true;
    for (Particle * particle : event.particles())
      {
      if (particle==&parent) continue;
      if (first)   // the products of the first decay start where the parent decayed
        {
        const VectorLorentz & r = particle->position();
        sumDistance += std::sqrt(r.x()*r.x()+r.y()*r.y()+r.z()*r.z());
        first = false;
        }
      if (particle->isDecayed()) continue;
      result.perParticle[particle->type().pdgCode()] += 1.0/n;
      q += particle->type().charge();
      e += particle->momentum().t();
      p += particle->momentum().z();
      if (!std::isfinite(particle->position().t())) result.conserved = false;
      }
    if (std::fabs(q-type.charge())>1e-6 || std::fabs(e-parent.momentum().t())>1e-6*e || std::fabs(p-pz)>1e-6*(1+std::fabs(pz)))
      result.conserved = false;
    }
  result.meanDistance = sumDistance/n;
  return result;
}
} // namespace

int main()
{
  CAP::MATH::randomGenerator().setSeed(4357);
  ParticleDb db;
  db.loadFromXml(CAP_PARTICLE_DB);

  // the branching ratios of the weak decays and of the rho(770) (once listed twice) sum to 1,
  // and of Sigma(1940), Delta(1920), Delta(1930) (rescaled after the PDG 2026 update)
  for (int pdg : { 311, -311, 310, 130, 3122, -3122, 3222, 3112, 3312, 3322, 3334, 113, 213, -213, 23224, 23114, 23214, 22224, 12126 })
    {
    ParticleType * type = db.findPdgCode(pdg);
    double sum = 0;
    for (auto & mode : type->decayModes()) sum += mode.branchingRatio();
    if (std::fabs(sum-1.0)>0.01) std::printf("  branching ratios of %s sum to %.4f\n", type->name().c_str(), sum);
    CHECK(std::fabs(sum-1.0)<=0.01);
    }

  const int n = 100000;
  // K0 at rest: K0S (pi+ pi- 0.692, pi0 pi0 0.307) or K0L (0.2028+0.1352 pi+ semileptonic, 0.1254 pi+ pi- pi0)
  ParticleType & k0 = *db.findPdgCode(311);
  CHECK(!k0.isStable());
  Result rk = decayMany(k0,0.0,n);
  double expectedPiPlus = 0.5*0.692 + 0.5*(0.20275+0.13520+0.12540);
  std::printf("K0: pi+ %.4f (%.4f), pi0 %.4f, K0 left %.4f\n", rk.perParticle[211], expectedPiPlus, rk.perParticle[111], rk.perParticle[311]);
  CHECK(std::fabs(rk.perParticle[211]-expectedPiPlus) < 0.01);
  CHECK(std::fabs(rk.perParticle[211]-rk.perParticle[-211]) < 0.01);
  CHECK(rk.perParticle[311]==0 && rk.perParticle[310]==0);
  CHECK(rk.conserved);

  // Lambda with p = 1 GeV/c: p pi- 0.641, n pi0 0.359, decay length beta gamma c tau = (p/m) 7.89 cm
  ParticleType & lambda = *db.findPdgCode(3122);
  CHECK(!lambda.isStable());
  Result rl = decayMany(lambda,1.0,n);
  double length = 1.0/lambda.mass()*2.617e-10*2.99792458e23;   // fm
  std::printf("Lambda: p %.4f (0.641), n %.4f (0.359), decay length %.4g fm (%.4g)\n", rl.perParticle[2212], rl.perParticle[2112],
              rl.meanDistance, length);
  CHECK(std::fabs(rl.perParticle[2212]-0.641) < 0.006);
  CHECK(std::fabs(rl.perParticle[2112]-0.359) < 0.006);
  CHECK(std::fabs(rl.meanDistance/length-1.0) < 0.02);
  CHECK(rl.conserved);

  // with the weak decays disabled, K0 and Lambda stay
  db.disableWeakDecays();
  CHECK(k0.isStable() && lambda.isStable());

  std::printf("TestParticleDecays: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures==0 ? 0 : 1;
}
