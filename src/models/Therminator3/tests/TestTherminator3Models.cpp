/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestTherminator3Models.cpp
//! @brief Tests of the Therminator3 freeze-out models against Therminator 2 (version 2.0.3).
//!
//! - BlastWave with the parameters of Therminator 2's fomodel/blastwave.ini: the average multiplicities of the
//!   primordial pi+, K+, p and Lambda (Model::calculateMultiplicities()) against Therminator 2's fmultiplicity file,
//!   and the multiplicities and <pT> of generated events (Model::generateEvent()) against 1000 Therminator 2 events.
//! - Hydro2D on a hypersurface built to be the BlastWave one (tau = const, rho in [0,rhoMax], constant vT): the
//!   multiplicities must be those of BlastWave, which checks the dSigma.p of the hydro model.
//! - The integrated hadron gas written by saveToAscii() and read back by loadFromAscii().
//! - Hydro3D on a hypersurface built to be a boost-invariant BlastWave one: same multiplicities as BlastWave.
//! - Hydro2D on Therminator 2's RHIC 20-30% hypersurface (when $THERMINATOR2_DIR is set): multiplicities and <pT>
//!   against a Therminator 2 run.
//! - Copies of the Therminator3 task (copy constructor, assignment) own a copy of the model.
//! - BWA with formation (subtype 6): every particle must stream back, during its delay, onto the hypersurface.
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include "TRandom.h"
#include "Event.hpp"
#include "ParticleDb.hpp"
#include "ParticleFilter.hpp"
#include "PhysicsConstants.hpp"
#include "ModelBlastWave.hpp"
#include "ModelBWA.hpp"
#include "ModelHydro2D.hpp"
#include "ModelHydro3D.hpp"
#include "Therminator3.hpp"

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

//! Primordial species compared with Therminator 2: PDG code, name, and the Therminator 2 results for BlastWave
//! (blastwave.ini, IntegrateSamples = 2000000, 1000 events with Randomize = 0): average multiplicity from the
//! integration, and <pT> (GeV/c) of the generated primordial particles with its statistical error.
struct Species { int pdg; const char * name; double t2Multiplicity; double t2MeanPt; double t2MeanPtError; };
const std::vector<Species> species =
{
  {  211, "pi+",    417.589, 0.44993, 0.00050 },
  {  321, "K+",     130.503, 0.58944, 0.00102 },
  { 2212, "p",       41.0796, 0.75472, 0.00217 },
  { 3122, "Lambda",  17.0576, 0.82132, 0.00354 },
};

//! Therminator 2 fomodel/blastwave.ini, in GeV and GeV^-1 (the units the models work in; configure() takes fm).
const double hbarc       = Physics::hBarC();
const double tau0        = 8.17/hbarc;
const double rhoMax      = 8.21/hbarc;
const double vT          = 0.341;
const double temperature = 0.1656;
const double muB         = 0.0285;
const double muI         = -0.0009;
const double muS         = 0.0069;

//! Models with the parameters set directly (the members are protected).
template <class M>
class TestModel : public M
{
public:
  void setUp(const ParticleDb & db, unsigned int nSamples)
  {
  this->_pRapidityRange = 4.0;
  this->_sRapidityRange = 8.0;
  this->_rhoMax         = rhoMax;
  this->_tauI           = tau0;
  this->_tVelocity      = vT;
  this->_tGamma         = 1.0/std::sqrt(1.0-vT*vT);
  this->setTemperature(temperature);
  this->setMuB(muB);
  this->setMuI(muI);
  this->setMuS(muS);
  this->setMuC(0.0);
  this->setNSamples(nSamples);
  for (auto & s : species)
    {
    ThermalHadron hadron;
    hadron.setType(db.findPdgCode(s.pdg));
    this->thermalHadronGas().appendHadron(hadron);
    }
  }
  void setSubtype(unsigned int subtype, double amplitude, double delay)
  { this->_modelSubtype = subtype; this->_amp = amplitude; this->_delay = delay; }
  //! Hypersurface tau = tau0, rho = D cos(zeta) in [0,rhoMax] with tauI = 0: D(zeta) = tau0/sin(zeta),
  //! zeta from atan(tau0/rhoMax) to pi/2, and a constant transverse velocity vT in the radial direction.
  void setBlastWaveHypersurface(unsigned int nZeta)
  {
  this->_tauI = 0.0;
  const double zetaMin = std::atan(tau0/rhoMax), zetaMax = M_PI/2;
  std::vector<double> minima = { zetaMin, 0.0, 0.0 }, maxima = { zetaMax, 2*M_PI, 0.0 };
  std::vector<unsigned int> nPoints = { nZeta, 2, 1 };
  this->_distanceField       = ScalarField("Distance",      minima,maxima,nPoints);
  this->_distanceFieldDZeta  = ScalarField("DistanceDZeta", minima,maxima,nPoints);
  this->_distanceFieldDPhi   = ScalarField("DistanceDPhi",  minima,maxima,nPoints,0.0);
  this->_vtFluidField        = ScalarField("FluidVt",       minima,maxima,nPoints,vT);
  this->_phiFluidField       = ScalarField("FluidPhi",      minima,maxima,nPoints,0.0);
  for (unsigned int i=0; i<nZeta; i++)
    {
    double zeta = zetaMin + i*(zetaMax-zetaMin)/(nZeta-1);
    for (unsigned int j=0; j<2; j++)
      {
      this->_distanceField(i,j,0)      =  tau0/std::sin(zeta);
      this->_distanceFieldDZeta(i,j,0) = -tau0*std::cos(zeta)/(std::sin(zeta)*std::sin(zeta));
      }
    }
  }
  //! 3+1 hypersurface of a boost-invariant BlastWave: tau = tauI + D sin(theta) sin(zeta) = tau0 (tauI = 0),
  //! rho = D sin(theta) cos(zeta) = tau0 cot(zeta) in [0,rhoMax], eta = D cos(theta)/Lambda, with Lambda = tau0/6 and
  //! theta in [pi/4,3pi/4], so |eta| reaches 6/sin(zeta) >= 6; radial flow u_T = gamma vT, fluid rapidity eta.
  void setBlastWave3DHypersurface(unsigned int nZeta, unsigned int nPhi, unsigned int nTheta)
  {
  this->_tauI   = 0.0;
  this->_lambda = tau0/6.0;
  const double zetaMin = std::atan(tau0/rhoMax), zetaMax = M_PI/2, thetaMin = M_PI/4, thetaMax = 3*M_PI/4;
  std::vector<double> minima = { zetaMin, 0.0, thetaMin }, maxima = { zetaMax, 2*M_PI, thetaMax };
  std::vector<unsigned int> nPoints = { nZeta, nPhi, nTheta };
  this->_distanceField        = ScalarField("Distance",      minima,maxima,nPoints);
  this->_distanceFieldDZeta   = ScalarField("DistanceDZeta", minima,maxima,nPoints);
  this->_distanceFieldDPhi    = ScalarField("DistanceDPhi",  minima,maxima,nPoints,0.0);
  this->_distanceFieldDTheta  = ScalarField("DistanceDTheta",minima,maxima,nPoints);
  this->uxFluidVelocityField  = ScalarField("FluidUx",       minima,maxima,nPoints);
  this->uyFluidVelocityField  = ScalarField("FluidUy",       minima,maxima,nPoints);
  this->rapidityFluidField    = ScalarField("FluidRap",      minima,maxima,nPoints);
  const double uT = vT/std::sqrt(1.0-vT*vT);
  for (unsigned int i=0; i<nZeta; i++)
    for (unsigned int j=0; j<nPhi; j++)
      for (unsigned int k=0; k<nTheta; k++)
        {
        double zeta  = zetaMin  + i*(zetaMax-zetaMin)/(nZeta-1);
        double phi   = j*2*M_PI/(nPhi-1);
        double theta = thetaMin + k*(thetaMax-thetaMin)/(nTheta-1);
        double d     = tau0/(std::sin(theta)*std::sin(zeta));
        this->_distanceField(i,j,k)       = d;
        this->_distanceFieldDZeta(i,j,k)  = -d*std::cos(zeta)/std::sin(zeta);
        this->_distanceFieldDTheta(i,j,k) = -d*std::cos(theta)/std::sin(theta);
        this->uxFluidVelocityField(i,j,k) = uT*std::cos(phi);
        this->uyFluidVelocityField(i,j,k) = uT*std::sin(phi);
        this->rapidityFluidField(i,j,k)   = d*std::cos(theta)/this->_lambda;
        }
  }
  unsigned long overflows() const { return this->nOverflows(); }
};

//! Generated primordial particles of each species: number per event and <pT>.
struct Generated { double n = 0, sumPt = 0, sumPt2 = 0; };

template <class M>
std::vector<Generated> generate(TestModel<M> & model, int nEvents)
{
  std::vector<Generated> result(species.size());
  Event event;
  ParticleFilter all;
  for (int iEvent=0; iEvent<nEvents; iEvent++)
    {
    event.clear();
    Particle::factory().reset();
    model.generateEvent(event,all);
    for (Particle * particle : event.particles())
      for (unsigned int k=0; k<species.size(); k++)
        if (particle->type().pdgCode()==species[k].pdg)
          {
          double pt = particle->momentum().perp();
          result[k].n += 1; result[k].sumPt += pt; result[k].sumPt2 += pt*pt;
          }
    }
  return result;
}
} // namespace

int main()
{
  gRandom->SetSeed(4357);
  ParticleDb db;
  db.loadFromXml(CAP_PARTICLE_DB);

  // ---------------------------------------------------------------- BlastWave against Therminator 2
  TestModel<ModelBlastWave> blastWave;
  blastWave.setUp(db,2000000);
  blastWave.calculateMultiplicities();
  std::vector<double> bwMultiplicities;
  std::printf("BlastWave: average multiplicity (Therminator 2)\n");
  for (unsigned int k=0; k<species.size(); k++)
    {
    double n = blastWave.thermalHadronGas().hadrons()[k].avgMultiplicity();
    bwMultiplicities.push_back(n);
    double ratio = n/species[k].t2Multiplicity;
    std::printf("  %-7s %10.3f (%10.3f)  ratio %.4f\n", species[k].name, n, species[k].t2Multiplicity, ratio);
    // the Monte Carlo integrations (2e6 draws here and in Therminator 2) each fluctuate by about 1%
    check(std::fabs(ratio-1.0)<0.025, std::string("BlastWave multiplicity of ")+species[k].name, __LINE__);
    }

  // the integrated gas written by saveToAscii() reads back with loadFromAscii()
  {
  const char * gasFile = "TestTherminator3Models_gas.txt";
  { std::ofstream out(gasFile); blastWave.saveToAscii(out); }
  CAP::ThermalHadronGas gas;
  { std::ifstream in(gasFile); gas.loadFromAscii(in,db); }
  std::remove(gasFile);
  bool same = gas.hadrons().size()==species.size();
  for (unsigned int k=0; same && k<species.size(); k++)
    {
    const ThermalHadron & a = gas.hadrons()[k], & b = blastWave.thermalHadronGas().hadrons()[k];
    same = a.type().pdgCode()==b.type().pdgCode()
        && std::fabs(a.avgMultiplicity()-b.avgMultiplicity()) <= 1e-8*b.avgMultiplicity()
        && std::fabs(a.integrand()-b.integrand()) <= 1e-8*b.integrand();
    }
  check(same, "hadron gas saved and loaded back", __LINE__);
  }

  const int nEvents = 1000;
  std::vector<Generated> bw = generate(blastWave,nEvents);
  std::printf("BlastWave: %d events, primordials per event and <pT> (Therminator 2)\n", nEvents);
  for (unsigned int k=0; k<species.size(); k++)
    {
    const Generated & g = bw[k];
    double perEvent = g.n/nEvents;
    double meanPt   = g.sumPt/g.n;
    double error    = std::sqrt((g.sumPt2/g.n - meanPt*meanPt)/g.n);
    double combined = std::sqrt(error*error + species[k].t2MeanPtError*species[k].t2MeanPtError);
    std::printf("  %-7s %9.3f +- %6.3f   <pT> %.5f +- %.5f (%.5f +- %.5f)  pull %+.2f\n", species[k].name, perEvent,
                std::sqrt(g.n)/nEvents, meanPt, error, species[k].t2MeanPt, species[k].t2MeanPtError,
                (meanPt-species[k].t2MeanPt)/combined);
    check(std::fabs(perEvent-bwMultiplicities[k]) < 5*std::sqrt(g.n)/nEvents + 0.002*bwMultiplicities[k],
          std::string("BlastWave generated multiplicity of ")+species[k].name, __LINE__);
    check(std::fabs(meanPt-species[k].t2MeanPt) < 5*combined, std::string("BlastWave <pT> of ")+species[k].name, __LINE__);
    }
  std::printf("  draws above the accept/reject maximum: %lu\n", blastWave.overflows());

  // ---------------------------------------------------------------- Hydro2D on the BlastWave hypersurface
  TestModel<ModelHydro2D> hydro;
  hydro.setUp(db,2000000);
  hydro.setBlastWaveHypersurface(2001);
  hydro.calculateMultiplicities();
  std::printf("Hydro2D on the BlastWave hypersurface: average multiplicity (BlastWave)\n");
  for (unsigned int k=0; k<species.size(); k++)
    {
    double n = hydro.thermalHadronGas().hadrons()[k].avgMultiplicity();
    double ratio = n/bwMultiplicities[k];
    std::printf("  %-7s %10.3f (%10.3f)  ratio %.4f\n", species[k].name, n, bwMultiplicities[k], ratio);
    check(std::fabs(ratio-1.0)<0.025, std::string("Hydro2D multiplicity of ")+species[k].name, __LINE__);
    }

  // ---------------------------------------------------------------- Hydro3D on a boost-invariant BlastWave hypersurface
  // compared with BlastWave over 12 units of spatial rapidity (the 3D surface reaches |eta| >= 6 everywhere)
  {
  TestModel<ModelBlastWave> wide;
  wide.setUp(db,2000000);
  wide._sRapidityRange = 12.0;
  wide.calculateMultiplicities();
  TestModel<ModelHydro3D> hydro3D;
  hydro3D.setUp(db,2000000);
  hydro3D.setBlastWave3DHypersurface(101,91,101);
  hydro3D.calculateMultiplicities();
  std::printf("Hydro3D on a BlastWave hypersurface: average multiplicity (BlastWave, 12 units of eta)\n");
  for (unsigned int k=0; k<species.size(); k++)
    {
    double n = hydro3D.thermalHadronGas().hadrons()[k].avgMultiplicity();
    double reference = wide.thermalHadronGas().hadrons()[k].avgMultiplicity();
    std::printf("  %-7s %10.3f (%10.3f)  ratio %.4f\n", species[k].name, n, reference, n/reference);
    check(std::fabs(n/reference-1.0)<0.025, std::string("Hydro3D multiplicity of ")+species[k].name, __LINE__);
    }
  }

  // ---------------------------------------------------------------- Hydro2D on a Therminator 2 hypersurface
  // Therminator 2's fomodel/lhyquid2dbi/RHICAuAu200c2030Ti455ti025Tf145.xml (lhyquid2dbi.ini: RapPRange 4,
  // RapSRange 8), read with readFromXmlFile(); the directory of Therminator 2 is given by $THERMINATOR2_DIR
  // (the check is skipped without it). Therminator 2 results: integration with 2e6 samples, 500 events.
  const char * t2Dir = std::getenv("THERMINATOR2_DIR");
  if (!t2Dir)
    std::printf("Hydro2D on the Therminator 2 RHIC hypersurface: skipped (THERMINATOR2_DIR is not set)\n");
  else
    {
    struct Reference { double multiplicity, perEvent, perEventError, meanPt, meanPtError; };
    const std::vector<Reference> t2 =
      {
      { 195.559, 195.876, 0.626, 0.49337, 0.00110 },
      {  48.1203, 47.864, 0.309, 0.67327, 0.00260 },
      {  10.7583, 10.928, 0.148, 0.88912, 0.00666 },
      {   3.82855, 3.924, 0.089, 0.96253, 0.01196 },
      };
    TestModel<ModelHydro2D> rhic;
    rhic.setUp(db,2000000);
    rhic.readFromXmlFile((String(t2Dir)+"/fomodel/lhyquid2dbi").c_str(),"RHICAuAu200c2030Ti455ti025Tf145.xml");
    rhic.calculateMultiplicities();
    const int nRhicEvents = 500;
    std::vector<Generated> g = generate(rhic,nRhicEvents);
    std::printf("Hydro2D on the Therminator 2 RHIC hypersurface: average multiplicity, per event and <pT> (Therminator 2)\n");
    for (unsigned int k=0; k<species.size(); k++)
      {
      double n        = rhic.thermalHadronGas().hadrons()[k].avgMultiplicity();
      double perEvent = g[k].n/nRhicEvents;
      double perEventError = std::sqrt(g[k].n)/nRhicEvents;
      double meanPt   = g[k].sumPt/g[k].n;
      double error    = std::sqrt((g[k].sumPt2/g[k].n - meanPt*meanPt)/g[k].n);
      double combined = std::sqrt(error*error + t2[k].meanPtError*t2[k].meanPtError);
      double nPull    = (perEvent-t2[k].perEvent)/std::sqrt(perEventError*perEventError+t2[k].perEventError*t2[k].perEventError);
      std::printf("  %-7s %9.3f (%9.3f) ratio %.4f   %8.3f +- %5.3f (%8.3f +- %5.3f) pull %+.2f   <pT> %.5f +- %.5f (%.5f +- %.5f) pull %+.2f\n",
                  species[k].name, n, t2[k].multiplicity, n/t2[k].multiplicity, perEvent, perEventError,
                  t2[k].perEvent, t2[k].perEventError, nPull, meanPt, error, t2[k].meanPt, t2[k].meanPtError,
                  (meanPt-t2[k].meanPt)/combined);
      check(std::fabs(n/t2[k].multiplicity-1.0)<0.025, std::string("RHIC Hydro2D multiplicity of ")+species[k].name, __LINE__);
      check(std::fabs(nPull)<5, std::string("RHIC Hydro2D generated multiplicity of ")+species[k].name, __LINE__);
      check(std::fabs(meanPt-t2[k].meanPt) < 5*combined, std::string("RHIC Hydro2D <pT> of ")+species[k].name, __LINE__);
      }
    std::printf("  draws above the accept/reject maximum: %lu\n", rhic.overflows());
    }

  // ---------------------------------------------------------------- BWA formation: back onto the hypersurface
  TestModel<ModelBWA> bwa;
  bwa.setUp(db,200000);
  bwa.setSubtype(6,0.0,2.0/hbarc);
  bwa.calculateMultiplicities();
  Event event;
  ParticleFilter all;
  Particle::factory().reset();
  bwa.generateEvent(event,all);
  unsigned int nOff = 0;
  for (Particle * particle : event.particles())
    {
    // delay d with (t-d)^2 - (z-vz d)^2 = tau0^2 and |r_T - v_T d| <= rhoMax, d >= 0
    VectorLorentz x;  // positions are in fm, the hypersurface in GeV^-1
    x.setTXYZ(particle->position().t()/hbarc,particle->position().x()/hbarc,particle->position().y()/hbarc,particle->position().z()/hbarc);
    const VectorLorentz & p = particle->momentum();
    double vx = p.x()/p.t(), vy = p.y()/p.t(), vz = p.z()/p.t();
    double a = 1.0 - vz*vz, b = -2.0*(x.t() - x.z()*vz), c = x.t()*x.t() - x.z()*x.z() - tau0*tau0;
    double discriminant = b*b - 4*a*c;
    bool onSurface = false;
    if (discriminant>=0)
      for (double d : { (-b-std::sqrt(discriminant))/(2*a), (-b+std::sqrt(discriminant))/(2*a) })
        if (d>=-1e-9 && std::hypot(x.x()-vx*d, x.y()-vy*d) <= rhoMax*(1+1e-9)) onSurface = true;
    if (!onSurface) nOff++;
    }
  std::printf("BWA formation: %zu particles, %u not streaming back onto the hypersurface\n", event.particles().size(), nOff);
  check(event.particles().size()>100 && nOff==0, "BWA formation positions", __LINE__);

  // ---------------------------------------------------------------- copies of the task: each its own model
  {
  Therminator3 * task = new Therminator3();
  task->setDefaultConfiguration();
  task->configure(task->configuration());
  Therminator3 * copy = new Therminator3(*task);
  Therminator3 assigned;
  assigned = *task;
  bool own = task->model() && copy->model() && assigned.model()
          && copy->model()!=task->model() && assigned.model()!=task->model()
          && dynamic_cast<ModelBlastWave*>(copy->model()) && dynamic_cast<ModelBlastWave*>(assigned.model())
          && std::fabs(copy->model()->rhoMax()-task->model()->rhoMax()) < 1e-12;
  delete task;   // the copies keep their models
  own = own && std::fabs(copy->model()->rhoMax()-8.21/hbarc) < 1e-12;
  delete copy;
  check(own, "copies of Therminator3 own a copy of the model", __LINE__);
  }

  std::printf("%d checks, %d failures\n", nChecks, nFailures);
  return nFailures ? 1 : 0;
}
