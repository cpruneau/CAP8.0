/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
//! @file TestPerformance.cpp
//! @brief Tests of the detector simulation and performance chain on a toy sample: gen -> smear (ParticleSmear) ->
//! performance histograms (ParticlePerformanceHistos: biases, resolutions, efficiencies, Delta phi wrapping, file round
//! trip, copies, scaling, fill options) -> closure (ClosureCalculator::calculateClosureFor, difference and ratio of folder trees).
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <unistd.h>
#include <vector>
#include "MersenneTwister.hpp"
#include "ClosureCalculator.hpp"
#include "ClosureIterator.hpp"
#include "CapFile.hpp"
#include "FileSystem.hpp"
#include "Folder.hpp"
#include "Histogram.hpp"
#include "StringParameter.hpp"
#include "Particle.hpp"
#include "ParticlePerformanceHistos.hpp"
#include "CalibrationProducer.hpp"
#include "ParticleSmear.hpp"
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

//! Smearing with constant resolutions and a constant pT bias.
ParticleSmear toySmear(double ptBias, double ptRms, double etaRms, double phiRms)
{
  ParticleSmear s;
  s.setBias(ParticleSmear::kPt,ptBias);
  s.setResolution(ParticleSmear::kPt,ptRms);
  s.setResolution(ParticleSmear::kEta,etaRms);
  s.setResolution(ParticleSmear::kPhi,phiRms);
  s.setSeed(4357);
  return s;
}

//! Performance histograms with a fixed binning (the configuration done by hand).
class ToyHistos : public ParticlePerformanceHistos
{
public:
  ToyHistos(bool phi=false, bool rapidity=true)
  {
  setHistogramBaseName("Toy");
  eta_fill = true; rapidity_fill = rapidity; phi_fill = phi;
  ptEta_fill = true; ptEta_nbins[0] = 5; ptEta_nbins[1] = 2;    // maps: pT bins of 2, eta bins of 2
  setAxis(kPt,10,0.0,10.0);  setAxis(kEta,4,-2.0,2.0);  setAxis(kPhi,4,0.0,2.0*M_PI);  setAxis(kY,4,-2.0,2.0);
  setDiff(kDpt,100,-1.0,1.0); setDiff(kDeta,100,-0.5,0.5); setDiff(kDphi,100,-3.2,3.2); setDiff(kDy,100,-0.5,0.5);
  }
  void setAxis(Axis a, int n, double lo, double hi) { axis_nbins[a] = n; axis_min[a] = lo; axis_max[a] = hi; }
  void setDiff(Quantity q, int n, double lo, double hi) { diff_nbins[q] = n; diff_min[q] = lo; diff_max[q] = hi; }
};

//! A rank 1 histogram with given bin values.
Histogram * histogram(const char * name, const std::vector<double> & values)
{
  Histogram * h = new Histogram(name,name,values.size(),0.0,double(values.size()));
  for (unsigned int k=0; k<values.size(); k++) { h->setBinValueAt(k,values[k]); h->setBinUncertaintyAt(k,0.1); }
  return h;
}
} // namespace

int main()
{
  CAP::MATH::randomGenerator().setSeed(4357);
  ParticleType pion;
  pion.setName("pi+"); pion.setCharge(1.0); pion.setPdgCode(211); pion.setMass(0.13957);
  const double mass = 0.13957, genPt = 1.5, genEta = 0.5;

  // Delta phi in [-pi, pi)
  CHECK(near(ParticlePerformanceHistos::deltaPhi(3.1,-3.1), 6.2-2.0*M_PI, 1e-12));
  CHECK(near(ParticlePerformanceHistos::deltaPhi(-3.1,3.1), 2.0*M_PI-6.2, 1e-12));
  CHECK(near(ParticlePerformanceHistos::deltaPhi(0.3,0.1), 0.2, 1e-12));
  CHECK(ParticlePerformanceHistos::deltaPhi(M_PI,0.0) >= -M_PI && ParticlePerformanceHistos::deltaPhi(M_PI,0.0) < M_PI);

  // generator-level sample: n particles at pT = 1.5, eta = 0.5, phi spread over [-pi, pi)
  const unsigned int n = 40000;
  std::vector<Particle> gen(n), reco(n);
  for (unsigned int k=0; k<n; k++)
    {
    double phi = -M_PI + 2.0*M_PI*(k+0.5)/n;
    double px = genPt*std::cos(phi), py = genPt*std::sin(phi), pz = genPt*std::sinh(genEta);
    gen[k].set(&pion, px, py, pz, std::sqrt(px*px+py*py+pz*pz+mass*mass), 0.1, 0.2, 0.3, 0.0);
    gen[k].setPid(k);
    }

  // smearing: every property of the target set, the truth particle recorded
  ParticleSmear smearer = toySmear(0.05,0.10,0.02,0.01);
  Particle stale;                                         // a reused factory object: parent, child, truth set
  stale.setParent(&gen[1]); stale.addChild(&gen[2]); stale.setTruth(&gen[3]);
  smearer.smear(gen[0],stale);
  CHECK(stale.truth() == &gen[0]);
  CHECK(!stale.hasParent() && !stale.hasChildren());
  CHECK(stale.typePtr() == &pion && stale.pid() == 0);
  CHECK(near(stale.position().x(),0.1,1e-12) && near(stale.position().z(),0.3,1e-12));
  double sumPt = 0.0, sumPt2 = 0.0, sumEta = 0.0, sumMass = 0.0;
  for (unsigned int k=0; k<n; k++)
    {
    smearer.smear(gen[k],reco[k]);
    double dpt = reco[k].momentum().perp() - genPt;
    sumPt += dpt; sumPt2 += dpt*dpt;
    sumEta += reco[k].momentum().pseudorapidity();
    sumMass += reco[k].momentum().modulus();
    }
  double biasPt = sumPt/n, rmsPt = std::sqrt(sumPt2/n - biasPt*biasPt);
  CHECK(near(biasPt,0.05,0.003));                         // bias of the map
  CHECK(near(rmsPt,0.10,0.003));                          // resolution of the map
  CHECK(near(sumEta/n,genEta,0.001));                     // eta smeared as a pseudorapidity (no shift for a massive particle)
  CHECK(near(sumMass/n,mass,1e-9));                       // mass kept

  // performance histograms: an 80% efficient detector
  ToyHistos histos;
  histos.create();
  unsigned int detected = 0;
  for (unsigned int k=0; k<n; k++)
    {
    histos.fillGenerated(gen[k],1.0);
    if (CAP::MATH::randomGenerator().random() > 0.8) continue;
    histos.fill(reco[k],1.0);
    detected++;
    }
  bool threw = false;
  try { histos.fill(gen[0],1.0); } catch (const std::exception &) { threw = true; }
  CHECK(threw);                                           // a particle without truth particle is refused
  CHECK(histos.difference(ParticlePerformanceHistos::kDphi)->binValueAt(0) == 0.0 && histos.difference(ParticlePerformanceHistos::kDphi)->binValueAt(99) == 0.0);  // no +-2pi entries
  histos.createDerived();
  CHECK(histos.hasDerived());
  histos.calculateDerived();
  int kPt = 1;                                            // bin [1, 2) of pT, holding 1.5
  CHECK(near(histos.profile(ParticlePerformanceHistos::kDpt,ParticlePerformanceHistos::kPt)->binValueAt(kPt),0.05,0.004));        // bias
  CHECK(near(histos.resolution(ParticlePerformanceHistos::kDpt,ParticlePerformanceHistos::kPt)->binValueAt(kPt),0.10,0.004));       // resolution
  CHECK(near(histos.resolution(ParticlePerformanceHistos::kDeta,ParticlePerformanceHistos::kPt)->binValueAt(kPt),0.02,0.001));
  CHECK(histos.resolution(ParticlePerformanceHistos::kDpt,ParticlePerformanceHistos::kPt)->binValueAt(0) == 0.0);                   // no entries: empty
  CHECK(near(histos.efficiency(ParticlePerformanceHistos::kPt)->binValueAt(kPt),double(detected)/n,1e-6));   // stored as float
  CHECK(near(histos.efficiency(ParticlePerformanceHistos::kPt)->binValueAt(kPt),0.8,0.01));
  CHECK(near(histos.efficiency(ParticlePerformanceHistos::kEta)->binValueAt(2),0.8,0.01));              // eta bin [0, 1)
  CHECK(histos.efficiency(ParticlePerformanceHistos::kPt)->binValueAt(5) == 0.0);                       // no generated particle: empty
  // maps vs (pT, eta): pT 1.5 and eta 0.5 in the bin (0, 1)
  bool mapsOk = histos.profilePtEta(ParticlePerformanceHistos::kDpt) && histos.resolutionPtEta(ParticlePerformanceHistos::kDpt)
                && histos.efficiencyPtEta() && histos.generatedPtEta() && histos.detectedPtEta();
  CHECK(mapsOk);
  if (mapsOk)
    {
    CHECK(histos.efficiencyPtEta()->rank() == 2 && histos.efficiencyPtEta()->nBins(0) == 5 && histos.efficiencyPtEta()->nBins(1) == 2);
    CHECK(near(histos.generatedPtEta()->binValueAt(0,1),double(n),1e-6) && histos.generatedPtEta()->binValueAt(0,0) == 0.0);
    CHECK(near(histos.profilePtEta(ParticlePerformanceHistos::kDpt)->binValueAt(0,1),0.05,0.004));      // bias map
    CHECK(near(histos.resolutionPtEta(ParticlePerformanceHistos::kDpt)->binValueAt(0,1),0.10,0.004));   // resolution map
    CHECK(near(histos.resolutionPtEta(ParticlePerformanceHistos::kDeta)->binValueAt(0,1),0.02,0.001));
    CHECK(near(histos.efficiencyPtEta()->binValueAt(0,1),double(detected)/n,1e-6));                    // efficiency map
    CHECK(histos.efficiencyPtEta()->binValueAt(1,1) == 0.0 && histos.resolutionPtEta(ParticlePerformanceHistos::kDpt)->binValueAt(0,0) == 0.0);
    }

  // detector maps (CalibrationProducer): efficiency and smearer of the folder, vs (pT, eta) or vs pT
  bool calibrated = true;
  try
    {
    ParticleEfficiency mapEff = CalibrationProducer::makeEfficiency(histos);
    ParticleSmear mapSmear = CalibrationProducer::makeSmearer(histos);
    CHECK(mapEff.name() == "Toy_Efficiency" && mapSmear.name() == "Toy_Smearer");
    CHECK(mapEff.histogram() && mapEff.histogram()->rank() == 2 && mapEff.variables().size() == 2);
    CHECK(near(mapEff.efficiency(1.5,0.5,0.0),double(detected)/n,1e-6));
    CHECK(mapEff.efficiency(3.0,0.5,0.0) == 0.0);                               // nothing generated there
    CHECK(near(mapSmear.mapValue(mapSmear.mapIndex("PT"),1.5,0.5,0.0),0.10,0.004));
    CHECK(near(mapSmear.mapValue(mapSmear.mapIndex("ETA"),1.5,0.5,0.0),0.02,0.001));
    CHECK(near(mapSmear.mapValue(mapSmear.mapIndex("PT_BIAS"),1.5,0.5,0.0),0.05,0.004));
    ParticleEfficiency effPt = CalibrationProducer::makeEfficiency(histos,CalibrationProducer::kPtOnly);
    CHECK(effPt.histogram() && effPt.histogram()->rank() == 1 && near(effPt.efficiency(1.5,0.5,0.0),double(detected)/n,1e-6));
    ParticleSmear noBias = CalibrationProducer::makeSmearer(histos,CalibrationProducer::kPtOnly,false);
    CHECK(noBias.map(noBias.mapIndex("PT_BIAS")).model() == KinematicMap::kNone);
    Folder maps("maps");                                                        // one map file: both objects, by kind
    mapEff.saveTo(maps); mapSmear.saveTo(maps);
    CHECK(ParticleEfficiency::objectNames(maps,"PARTICLE_EFFICIENCY").size() == 1
          && ParticleSmear::objectNames(maps,"PARTICLE_SMEARER").size() == 1);
    ParticleSmear readSmear; readSmear.setName("Toy_Smearer"); readSmear.loadFrom(maps);
    CHECK(near(readSmear.mapValue(readSmear.mapIndex("PT"),1.5,0.5,0.0),mapSmear.mapValue(mapSmear.mapIndex("PT"),1.5,0.5,0.0),1e-12));
    }
  catch (const std::exception & e) { calibrated = false; std::printf("  %s\n", e.what()); }
  CHECK(calibrated);
  CHECK(CalibrationProducer::parseMapVariables("pt_eta") == CalibrationProducer::kPtEta);

  // file round trip: base and derived histograms written and read back under their names
  Folder file("file");
  histos.saveTo(file);
  ToyHistos loaded;
  bool loadedOk = true;
  try { loaded.loadFrom(file); } catch (const std::exception & e) { loadedOk = false; std::printf("  %s\n", e.what()); }
  CHECK(loadedOk);
  CHECK(loaded.hasDerived());
  CHECK(loadedOk && near(loaded.efficiency(ParticlePerformanceHistos::kPt)->binValueAt(kPt),histos.efficiency(ParticlePerformanceHistos::kPt)->binValueAt(kPt),1e-12));
  CHECK(loadedOk && loaded.efficiencyPtEta() && near(loaded.efficiencyPtEta()->binValueAt(0,1),histos.efficiencyPtEta()->binValueAt(0,1),1e-12));
  CHECK(loadedOk && loaded.profilePtEta(ParticlePerformanceHistos::kDpt)
        && near(loaded.profilePtEta(ParticlePerformanceHistos::kDpt)->binValueAt(0,1),histos.profilePtEta(ParticlePerformanceHistos::kDpt)->binValueAt(0,1),1e-6));

  // copy: the members point to the histograms of the copy, with the same content
  ToyHistos copy(histos);
  CHECK(copy.efficiency(ParticlePerformanceHistos::kPt) != histos.efficiency(ParticlePerformanceHistos::kPt));
  CHECK(copy.efficiency(ParticlePerformanceHistos::kPt) == copy.histogram(histos.efficiency(ParticlePerformanceHistos::kPt)->name()));
  CHECK(near(copy.efficiency(ParticlePerformanceHistos::kPt)->binValueAt(kPt),histos.efficiency(ParticlePerformanceHistos::kPt)->binValueAt(kPt),1e-12));
  ToyHistos assigned;
  assigned = histos;
  CHECK(assigned.generated(ParticlePerformanceHistos::kPt) != histos.generated(ParticlePerformanceHistos::kPt));
  CHECK(near(assigned.generated(ParticlePerformanceHistos::kPt)->binValueAt(kPt),double(n),1e-6));
  CHECK(copy.efficiencyPtEta() && copy.efficiencyPtEta() != histos.efficiencyPtEta()
        && copy.efficiencyPtEta() == copy.histogram(histos.efficiencyPtEta()->name()));

  // scaling: counts scaled, averages and resolutions not
  double bias = copy.profile(ParticlePerformanceHistos::kDpt,ParticlePerformanceHistos::kPt)->binValueAt(kPt);
  double sigma = copy.resolution(ParticlePerformanceHistos::kDpt,ParticlePerformanceHistos::kPt)->binValueAt(kPt);
  copy.scaleObject(0.5);
  CHECK(near(copy.generated(ParticlePerformanceHistos::kPt)->binValueAt(kPt),0.5*n,1e-3));
  CHECK(near(copy.generatedPtEta()->binValueAt(0,1),0.5*n,1e-3));
  CHECK(near(copy.profile(ParticlePerformanceHistos::kDpt,ParticlePerformanceHistos::kPt)->binValueAt(kPt),bias,1e-12));
  CHECK(near(copy.resolution(ParticlePerformanceHistos::kDpt,ParticlePerformanceHistos::kPt)->binValueAt(kPt),sigma,1e-12));

  // fill options: vs phi on, rapidity off
  ToyHistos options(true,false);
  options.ptEta_fill = false;
  options.create();
  for (unsigned int k=0; k<n; k++) { options.fillGenerated(gen[k],1.0); if (k%5) options.fill(reco[k],1.0); }
  options.createDerived();
  options.calculateDerived();
  CHECK(options.difference(ParticlePerformanceHistos::kDy) == nullptr);
  CHECK(options.efficiencyPtEta() == nullptr && options.profilePtEta(ParticlePerformanceHistos::kDpt) == nullptr && options.generatedPtEta() == nullptr);
  CHECK(options.profile(ParticlePerformanceHistos::kDy,ParticlePerformanceHistos::kPt) == nullptr);
  CHECK(options.generated(ParticlePerformanceHistos::kY) == nullptr && options.efficiency(ParticlePerformanceHistos::kY) == nullptr);
  CHECK(options.profile(ParticlePerformanceHistos::kDpt,ParticlePerformanceHistos::kPhi) != nullptr);
  CHECK(options.resolution(ParticlePerformanceHistos::kDphi,ParticlePerformanceHistos::kPhi) != nullptr);
  bool phiOk = options.efficiency(ParticlePerformanceHistos::kPhi) != nullptr;
  CHECK(phiOk);
  double phiCounts = 0.0;
  if (phiOk)
    for (int k=0; k<4; k++)
      {
      CHECK(near(options.efficiency(ParticlePerformanceHistos::kPhi)->binValueAt(k),0.8,0.001));   // 4 of 5 detected
      phiCounts += options.generated(ParticlePerformanceHistos::kPhi)->binValueAt(k);
      }
  CHECK(near(phiCounts,double(n),1e-6));                  // phi mapped onto [0, 2pi): no particle lost
  CHECK(options.histogram(histos.difference(ParticlePerformanceHistos::kDy)->name()) == nullptr);   // not created

  // closure of folder trees: reco - gen and reco / gen, place by place; histograms without counterpart skipped
  Folder genTop("gen"), recoTop("reco");
  Folder * g = genTop.addFolder("Ana")->addFolder("EF0_PF0");
  Folder * r = recoTop.addFolder("Ana")->addFolder("EF0_PF0");
  g->add(histogram("n1_pt",{2.0,4.0,8.0}),"n1_pt");
  r->add(histogram("n1_pt",{1.0,3.0,8.0}),"n1_pt");
  g->add(histogram("n1_eta",{1.0,1.0,1.0}),"n1_eta");           // no detector-level counterpart
  genTop.addFolder("Other");                                      // no detector-level counterpart
  for (int method : {0, 1})
    {
    Folder out("closure");
    std::vector<String> skipped;
    unsigned int compared = ClosureCalculator::calculateClosureFor(genTop,recoTop,out,method,ClosureCalculator::kNoTest,
                                                                   skipped);
    CHECK(compared == 1);
    CHECK(skipped.size() == 2);
    const Histogram * c = out.get<Histogram>("Ana/EF0_PF0/n1_pt");
    CHECK(c != nullptr);
    if (!c) continue;
    if (method == 0) CHECK(near(c->binValueAt(0),-1.0,1e-12) && near(c->binValueAt(1),-1.0,1e-12) && near(c->binValueAt(2),0.0,1e-12));
    else             CHECK(near(c->binValueAt(0),0.5,1e-12) && near(c->binValueAt(1),0.75,1e-12) && near(c->binValueAt(2),1.0,1e-12));
    }

  // statistical tests of the closure (chi2 of the values, Kolmogorov-Smirnov, user defined), written next to it
  {
  Histogram * g2 = new Histogram("n2_ptpt","n2_ptpt",2,0.0,2.0,2,0.0,2.0);   // rank 2: no Kolmogorov test
  Histogram * r2 = new Histogram("n2_ptpt","n2_ptpt",2,0.0,2.0,2,0.0,2.0);
  for (unsigned int k=0; k<4; k++) { g2->setBinValueAtGlobal(k,1.0); g2->setBinUncertaintyAtGlobal(k,0.1); r2->setBinValueAtGlobal(k,1.1); r2->setBinUncertaintyAtGlobal(k,0.1); }
  g->add(g2,"n2_ptpt");
  r->add(r2,"n2_ptpt");
  // chi2 = ((1-2)^2 + (3-4)^2 + 0)/(0.1^2 + 0.1^2) = 100 (uncertainties kept as floats), 3 degrees of freedom
  {
  Folder out("closure");
  std::vector<String> skipped;
  std::vector<ClosureCalculator::TestRecord> results;
  ClosureCalculator::calculateClosureFor(genTop,recoTop,out,1,ClosureCalculator::kChi2Test,skipped,"",&results);
  CHECK(results.size() == 2);
  const ClosureCalculator::TestRecord * pt = nullptr;
  for (const auto & record : results) if (record.first == "Ana/EF0_PF0/n1_pt") pt = &record;
  CHECK(pt != nullptr && near(pt->second.statistic,100.0,1e-4) && pt->second.nDegreesOfFreedom == 3.0 && pt->second.rejected());
  const StringParameter * written = out.get<StringParameter>("Ana/EF0_PF0/n1_pt_TEST");
  CHECK(written != nullptr && written->value() == pt->second.summary());
  }
  // Kolmogorov-Smirnov: D = max |C_reco - C_gen| = |4/12 - 6/14|; no test of the histogram of rank 2
  {
  Folder out("closure");
  std::vector<String> skipped;
  std::vector<ClosureCalculator::TestRecord> results;
  ClosureCalculator::calculateClosureFor(genTop,recoTop,out,0,ClosureCalculator::kKolmogorovTest,skipped,"",&results);
  CHECK(results.size() == 2);
  for (const auto & record : results)
    if (record.first == "Ana/EF0_PF0/n1_pt")
      CHECK(record.second.statisticName == "D" && near(record.second.statistic,6.0/14.0-4.0/12.0,1e-12));
    else
      CHECK(record.second.test.isNull());                                       // rank 2: not tested
  const StringParameter * written = out.get<StringParameter>("Ana/EF0_PF0/n2_ptpt_TEST");
  CHECK(written != nullptr && written->value().beginsWith("not tested"));
  }
  // user defined test
  {
  ClosureCalculator::setUserTest([](const Histogram & gen, const Histogram & reco)
    {
    MATH::TestResult result;
    result.test = "difference of sums";
    result.statisticName = "d";
    result.statistic = reco.integral() - gen.integral();
    return result;
    });
  Folder out("closure");
  std::vector<String> skipped;
  std::vector<ClosureCalculator::TestRecord> results;
  ClosureCalculator::calculateClosureFor(genTop,recoTop,out,0,ClosureCalculator::kUserTest,skipped,"",&results);
  CHECK(results.size() == 2);
  for (const auto & record : results)
    if (record.first == "Ana/EF0_PF0/n1_pt") CHECK(near(record.second.statistic,-2.0,1e-12));
  ClosureCalculator::setUserTest(ClosureCalculator::UserTest());
  bool thrown = false;
  try { ClosureCalculator::runTest(ClosureCalculator::kUserTest,*g2,*r2); } catch (const Exception &) { thrown = true; }
  CHECK(thrown);
  }
  }

  // ClosureIterator: the generator-level files of a directory, by their tags; a file that cannot be compared skipped
  {
  CHECK(ClosureIterator::counterpart("pp_Gen.cap","_Reco") == "pp_Reco.cap");
  CHECK(ClosureIterator::counterpart("pp_Gen_run1.cap","_Closure") == "pp_Closure_run1.cap");
  CHECK(ClosureIterator::counterpart("pp_General_Gen.cap","_Reco") == "pp_General_Reco.cap");
  CHECK(ClosureIterator::counterpart("pp_General.cap","_Reco") == "");
  CHECK(ClosureIterator::counterpart("pp_Gen_Gen.cap","_Reco") == "");
  char pattern[] = "/tmp/TestClosureIteratorXXXXXX";
  if (mkdtemp(pattern))
    {
    String in(pattern), out = FileSystem::join(in,"out");
    FileSystem::createDirectory(out);
    auto write = [&](const char * name, const char * folder, const std::vector<double> & values)
      {
      std::unique_ptr<CapFile> file(CapFile::openRecreateCapFile(in,name));
      file->folder().addFolder(folder)->add(histogram("n1_pt",values),"n1_pt");
      file->write();
      };
    write("a_Gen.cap","Ana",{2.0,4.0,8.0});            write("a_Reco.cap","Ana",{1.0,3.0,8.0});            // compared
    write("b_Gen.cap","Ana",{2.0,4.0,8.0});                                                                // no reco: skipped
    write("c_General_Gen.cap","Ana",{2.0,4.0,8.0});    write("c_General_Reco.cap","Ana",{2.0,4.0,8.0});    // compared
    write("d_Gen_Gen.cap","Ana",{2.0,4.0,8.0});        write("d_Reco_Reco.cap","Ana",{2.0,4.0,8.0});      // ambiguous: left out
    write("e_General.cap","Ana",{2.0,4.0,8.0});                                                            // not a gen file
    write("f_Gen.cap","Ana",{2.0,4.0,8.0});            write("f_Reco.cap","Other",{2.0,4.0,8.0});          // no folder: skipped
    ClosureIterator iterator;
    iterator.setMinimumReportLevel(Object::kError);
    ClosureIterator::Summary summary = iterator.run(0,ClosureCalculator::kChi2Test,in,"Ana","Ana",out,true);
    CHECK(summary.compared.size() == 2 && summary.compared[0] == "a_Gen.cap" && summary.compared[1] == "c_General_Gen.cap");
    CHECK(summary.skipped.size() == 2 && String(summary.skipped[0]).beginsWith("b_Gen.cap: no detector-level file")
          && String(summary.skipped[1]).beginsWith("f_Gen.cap:"));
    CHECK(summary.nHistograms == 2 && summary.nTested == 2 && summary.nRejected == 1);   // a differs, c is identical
    CHECK(FileSystem::isFile(FileSystem::join(out,"a_Closure.cap")) && FileSystem::isFile(FileSystem::join(out,"c_General_Closure.cap")));
    std::vector<String> included = { "a_" };
    CHECK(iterator.run(1,ClosureCalculator::kNoTest,in,"Ana","Ana",out,true,included).compared.size() == 1);
    bool thrown = false;                                  // closure files kept (no rewrite): every file skipped
    try { iterator.run(0,ClosureCalculator::kNoTest,in,"Ana","Ana",out,false); } catch (const Exception &) { thrown = true; }
    CHECK(thrown);
    FileSystem::removeTree(in);
    }
  else
    CHECK(false);
  }

  std::printf("TestPerformance: %d checks, %d failures\n", nChecks, nFailures);
  return nFailures == 0 ? 0 : 1;
}
