/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
//! @file TestJets.cpp
//! @brief Tests of the jet module: the variables of a particle relative to a jet; jet finding (two sprays found as two
//! jets with their constituents, the acceptance |eta| < etaMax - R, the rank counted among the jets accepted, several
//! radii); the clustering done once per event and particle filter; the jet filters (kinematics, rank); and the filling
//! of the histograms.
#include <cmath>
#include <random>
#include <cstdio>
#include <string>
#include <vector>
#include "Jet.hpp"
#include "JetClustering.hpp"
#include "JetFilter.hpp"
#include "JetHistos.hpp"
#include "JetPair.hpp"
#include "JetPairHistos.hpp"
#include "JetUnderlyingEvent.hpp"
#include "JetFluctuationHistos.hpp"
#include "Event.hpp"
#include "Particle.hpp"
#include "ParticleType.hpp"
#include "Configuration.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "NameHelpers.hpp"

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

bool near(double a, double b, double tolerance=1.0E-9) { return std::fabs(a-b)<=tolerance*std::max(1.0,std::fabs(b)); }

//! Sum of the bin values of a histogram.
double sum(const MATH::Histogram * h)
{
  double total = 0.0;
  for (unsigned int k=0; k<h->nBinsTotal(); k++) total += h->binValueAtGlobal(k);
  return total;
}

MATH::VectorLorentz momentumOf(double pt, double eta, double phi, double mass=0.14)
{
  double px = pt*std::cos(phi), py = pt*std::sin(phi), pz = pt*std::sinh(eta);
  MATH::VectorLorentz p;
  p.setTXYZ(std::sqrt(mass*mass+px*px+py*py+pz*pz),px,py,pz);
  return p;
}
} // namespace

int main()
{
  ParticleType plus, minus, neutral;
  plus.setCharge(1.0);  minus.setCharge(-1.0);  neutral.setCharge(0.0);
  plus.setPdgCode(211); minus.setPdgCode(-211); neutral.setPdgCode(111);

  // ----- variables of a particle relative to a jet
  {
  MATH::VectorLorentz jet;  jet.setTXYZ(10.0,0.0,0.0,10.0);      // along z
  MATH::VectorLorentz p;    p.setTXYZ(5.0,3.0,0.0,4.0);          // |p| = 5, 3 perpendicular, 4 along
  check(near(transverseToJet(p,jet),3.0), "jT = |p x j|/|j|", __LINE__);
  check(near(angleToJet(p,jet),std::atan2(3.0,4.0)), "angle to the jet axis", __LINE__);
  MATH::VectorLorentz a = momentumOf(2.0,0.5,1.0), b = momentumOf(8.0,0.2,0.6);
  check(near(momentumFraction(a,b),0.25), "z = pT/pT_jet", __LINE__);
  check(near(deltaR(a,b),std::sqrt(std::pow(a.rapidity()-b.rapidity(),2)+0.16)), "deltaR in rapidity and azimuth", __LINE__);
  check(near(deltaPhi(0.1,6.2),0.1+2*M_PI-6.2), "deltaPhi wrapped", __LINE__);
  check(near(azimuth(momentumOf(1.0,0.0,-0.5)),2*M_PI-0.5), "azimuth in [0, 2 pi)", __LINE__);
  check(JetSettings::radiusLabel(0.4)=="R0p4" && JetSettings::radiusLabel(1.0)=="R1", "radius labels", __LINE__);
  }

  // ----- jet finding: three sprays
  std::vector<Particle> store(12);
  std::vector<Particle*> particles;
  auto add = [&](ParticleType * type, double pt, double eta, double phi)
    {
    Particle & p = store[particles.size()];
    p.setType(type);
    p.setMomentum(momentumOf(pt,eta,phi));
    particles.push_back(&p);
    };
  // A: 35 GeV at (0, 1); B: 20 GeV at (0.3, 1+pi); C: 50 GeV at (0.62, 5.5), inside |eta| < 0.9-0.2 only
  add(&plus,20.0,0.00,1.00);  add(&minus,10.0,0.05,1.05);  add(&neutral,5.0,-0.05,0.95);
  add(&plus,15.0,0.30,1.0+M_PI); add(&plus,5.0,0.32,1.02+M_PI);
  add(&minus,30.0,0.62,5.50);  add(&plus,20.0,0.63,5.52);
  JetSettings settings;  settings.radius = 0.4;  settings.ptMin = 5.0;  settings.particleEtaMax = 0.9;
  std::vector<Jet> jets = JetClustering::cluster(particles,settings);
  check(jets.size()==2, "R = 0.4: two jets within |eta| < 0.5 (the 50 GeV one at eta 0.62 is out)", __LINE__);
  if (jets.size()==2)
    {
    check(jets[0].rank==1 && jets[1].rank==2, "ranks 1, 2 among the jets accepted", __LINE__);
    check(near(jets[0].pt(),35.0,0.01) && jets[0].multiplicity()==3, "leading jet: 35 GeV, 3 constituents", __LINE__);
    check(near(jets[1].pt(),20.0,0.01) && jets[1].multiplicity()==2, "subleading jet: 20 GeV, 2 constituents", __LINE__);
    double pt = 0;  for (Particle * p : jets[0].constituents) pt += p->momentum().perp();
    check(near(pt,35.0), "constituents of the leading jet: the particles of spray A", __LINE__);
    }
  settings.radius = 0.2;
  jets = JetClustering::cluster(particles,settings);
  check(jets.size()==3 && near(jets[0].pt(),50.0,0.01) && jets[0].rank==1, "R = 0.2: the 50 GeV jet in, leading", __LINE__);
  settings.ptMin = 25.0;
  jets = JetClustering::cluster(particles,settings);
  check(jets.size()==2, "minimum pT 25: two jets", __LINE__);
  settings.algorithm = JetSettings::Kt;  settings.ptMin = 5.0;
  check(JetClustering::cluster(particles,settings).size()==3, "kt algorithm: the same three jets", __LINE__);
  check(JetClustering::cluster(std::vector<Particle*>(),settings).empty(), "no particles: no jets", __LINE__);

  // ----- once per event and settings
  {
  JetClustering::clearCache();
  Event event;
  event.reset();
  JetSettings s;  s.radius = 0.4;
  unsigned long before = JetClustering::nClusterings();
  JetClustering::jets(event,nullptr,particles,s);
  JetClustering::jets(event,nullptr,particles,s);
  check(JetClustering::nClusterings()==before+1, "the jets of an event found once", __LINE__);
  JetSettings s2 = s;  s2.radius = 0.2;
  JetClustering::jets(event,nullptr,particles,s2);
  check(JetClustering::nClusterings()==before+2, "another radius: found again", __LINE__);
  event.reset();
  JetClustering::jets(event,nullptr,particles,s);
  check(JetClustering::nClusterings()==before+3, "next event: found again", __LINE__);
  }

  // ----- jet filters
  settings = JetSettings();  settings.radius = 0.2;
  jets = JetClustering::cluster(particles,settings);         // 50 (rank 1), 35 (2), 20 (3)
  {
  Configuration configuration;
  auto set = [&](unsigned int k, const char * name, auto value)
    { configuration.addProperty(createKey("T","JET_FILTER",k,"CONDITION",0,name),value); };
  configuration.addProperty(createKey("T","JET_FILTER",0,"CONDITION:N"),1);
  set(0,"TYPE",String("INTEGER"));  set(0,"SUBTYPE",String("RANK"));  set(0,"MINIMUM",2.0);  set(0,"MAXIMUM",2.0);
  JetFilter subleading;
  subleading.configure("T","JET_FILTER",configuration,0);
  unsigned int n = 0;  double pt = 0;
  for (const Jet & jet : jets) if (subleading.accept(jet)) { n++; pt = jet.pt(); }
  check(n==1 && near(pt,35.0,0.01), "RANK = 2: the subleading jet", __LINE__);
  configuration.addProperty(createKey("T","JET_FILTER",1,"CONDITION:N"),2);
  configuration.addProperty(createKey("T","JET_FILTER",1,"CONDITION",0,"TYPE"),String("DOUBLE_RANGE"));
  configuration.addProperty(createKey("T","JET_FILTER",1,"CONDITION",0,"SUBTYPE"),String("PT"));
  configuration.addProperty(createKey("T","JET_FILTER",1,"CONDITION",0,"MINIMUM"),30.0);
  configuration.addProperty(createKey("T","JET_FILTER",1,"CONDITION",0,"MAXIMUM"),100.0);
  configuration.addProperty(createKey("T","JET_FILTER",1,"CONDITION",1,"TYPE"),String("DOUBLE_RANGE"));
  configuration.addProperty(createKey("T","JET_FILTER",1,"CONDITION",1,"SUBTYPE"),String("PHI"));
  configuration.addProperty(createKey("T","JET_FILTER",1,"CONDITION",1,"MINIMUM"),0.0);
  configuration.addProperty(createKey("T","JET_FILTER",1,"CONDITION",1,"MAXIMUM"),3.0);
  JetFilter hardNear;
  hardNear.configure("T","JET_FILTER",configuration,1);
  n = 0;
  for (const Jet & jet : jets) if (hardNear.accept(jet)) { n++; pt = jet.pt(); }
  check(n==1 && near(pt,35.0,0.01), "PT in [30, 100) and PHI in [0, 3): the 35 GeV jet", __LINE__);
  configuration.addProperty(createKey("T","JET_FILTER",2,"CONDITION:N"),1);
  configuration.addProperty(createKey("T","JET_FILTER",2,"CONDITION",0,"TYPE"),String("DOUBLE_RANGE"));
  configuration.addProperty(createKey("T","JET_FILTER",2,"CONDITION",0,"SUBTYPE"),String("WIDTH"));
  configuration.addProperty(createKey("T","JET_FILTER",2,"CONDITION",0,"MINIMUM"),0.0);
  configuration.addProperty(createKey("T","JET_FILTER",2,"CONDITION",0,"MAXIMUM"),1.0);
  JetFilter bad;
  bool threw = false;
  try { bad.configure("T","JET_FILTER",configuration,2); } catch (std::exception &) { threw = true; }
  check(threw, "unknown jet quantity: exception", __LINE__);
  }

  // ----- histograms
  {
  Configuration configuration;
  JetHistos::addDefaultConfiguration(configuration,"T");
  JetHistos histos;
  histos.configure("T","HISTOGRAM",configuration,0);
  histos.create();
  std::vector<const Jet*> accepted;
  for (const Jet & jet : jets) accepted.push_back(&jet);
  histos.fill(accepted);                                // an event with 3 jets
  histos.fill(std::vector<const Jet*>());               // an event without
  auto h = [&](const char * name) { return histos.histogram(createName("Jet",name)); };
  check(near(h("nJets")->binValueAtGlobal(3),1.0) && near(h("nJets")->binValueAtGlobal(0),1.0), "nJets: 3 and 0", __LINE__);
  check(near(h("pt")->integral(),3.0), "three jets in pt", __LINE__);
  check(near(h("c_z")->integral(),7.0), "seven constituents", __LINE__);
  check(near(h("cPos_z")->integral(),4.0) && near(h("cNeg_z")->integral(),2.0), "4 positive, 2 negative", __LINE__);
  double sumZ = 0;
  const MATH::Histogram * rho = h("c_rho");
  for (unsigned int k=0; k<rho->nBinsTotal(); k++) sumZ += rho->binValueAtGlobal(k);
  check(near(sumZ,3.0,1.0E-3), "rho: the sum of z of each jet is 1 (three jets)", __LINE__);
  const MATH::Histogram * netQ = h("netQ");
  check(near(netQ->integral(),3.0), "net charge of each jet", __LINE__);
  check(near(histos.jetCount(),3.0), "three jets counted", __LINE__);
  histos.normalize(0.5);
  check(near(sum(h("nJets")),1.0,1.0E-6), "nJets per event (two events)", __LINE__);
  check(near(sum(h("pt")),1.0,1.0E-6), "jet histograms per jet (bins in single precision)", __LINE__);
  check(near(sum(h("c_z")),7.0/3.0,1.0E-6), "constituents per jet: 7/3", __LINE__);
  sumZ = 0;
  for (unsigned int k=0; k<rho->nBinsTotal(); k++) sumZ += rho->binValueAtGlobal(k);
  check(near(sumZ,1.0,1.0E-3), "rho per jet: integral 1", __LINE__);
  const MATH::Profile * p = dynamic_cast<const MATH::Profile*>(histos.histogram(createName("Jet","mult_vsPt")));
  unsigned int bin = (unsigned int)(jets[1].pt());          // 1 GeV bins from 0
  check(p!=nullptr && near(p->binValueAtGlobal(bin),3.0), "profile not scaled: <N> = 3 at 35 GeV", __LINE__);
  }

  // ----- jet pairs: three jets 40 GeV (phi 0, eta 0.2), 30 GeV (phi pi - 0.1, eta -0.3), 10 GeV (phi 1.5, eta 0)
  {
  std::vector<Jet> three(3);
  three[0].momentum = momentumOf(40.0,0.2,0.0,0.0);       three[0].rank = 1;
  three[1].momentum = momentumOf(30.0,-0.3,M_PI-0.1,0.0); three[1].rank = 2;
  three[2].momentum = momentumOf(10.0,0.0,1.5,0.0);       three[2].rank = 3;
  JetPair ls(&three[0],&three[1]), lt(&three[0],&three[2]), st(&three[1],&three[2]);
  check(near(ls.deltaPhi(),-(M_PI-0.1)+2*M_PI) && near(ls.absDeltaPhi(),M_PI-0.1), "dphi in [-pi/2, 3 pi/2)", __LINE__);
  check(near(ls.acoplanarity(),0.1), "acoplanarity pi - |dphi|", __LINE__);
  check(near(ls.deltaEta(),0.5) && near(ls.ptRatio(),0.75) && near(ls.asymmetry(),10.0/70.0), "deta, xJ, AJ", __LINE__);
  double m2 = 2*40.0*30.0*(std::cosh(0.5)-std::cos(M_PI-0.1));
  check(near(ls.mass(),std::sqrt(m2),1.0E-9), "dijet mass of massless jets", __LINE__);
  check(near(lt.deltaPhi(),-1.5), "dphi in (-pi/2, 0) kept", __LINE__);
  Jet far;  far.momentum = momentumOf(5.0,0.0,2.0,0.0);  far.rank = 4;
  check(near(JetPair(&three[0],&far).deltaPhi(),2*M_PI-2.0), "dphi below -pi/2 moved up by 2 pi", __LINE__);
  check(near(st.deltaPhi(),M_PI-0.1-1.5), "dphi positive unchanged", __LINE__);
  Configuration configuration;
  auto set = [&](unsigned int k, unsigned int c, const char * name, auto value)
    { configuration.addProperty(createKey("T","JET_PAIR_FILTER",k,"CONDITION",c,name),value); };
  // 0: leading-subleading; 1: back to back (|dphi| > pi - 0.3) with xJ > 0.5
  configuration.addProperty(createKey("T","JET_PAIR_FILTER",0,"CONDITION:N"),2);
  set(0,0,"TYPE",String("INTEGER"));  set(0,0,"SUBTYPE",String("RANK1"));  set(0,0,"MINIMUM",1.0);  set(0,0,"MAXIMUM",1.0);
  set(0,1,"TYPE",String("INTEGER"));  set(0,1,"SUBTYPE",String("RANK2"));  set(0,1,"MINIMUM",2.0);  set(0,1,"MAXIMUM",2.0);
  configuration.addProperty(createKey("T","JET_PAIR_FILTER",1,"CONDITION:N"),2);
  set(1,0,"TYPE",String("DOUBLE_RANGE"));  set(1,0,"SUBTYPE",String("DPHI"));  set(1,0,"MINIMUM",M_PI-0.3);  set(1,0,"MAXIMUM",4.0);
  set(1,1,"TYPE",String("DOUBLE_RANGE"));  set(1,1,"SUBTYPE",String("PTRATIO"));  set(1,1,"MINIMUM",0.5);  set(1,1,"MAXIMUM",1.1);
  JetPairFilter leadSub, backToBack;
  leadSub.configure("T","JET_PAIR_FILTER",configuration,0);
  backToBack.configure("T","JET_PAIR_FILTER",configuration,1);
  check(leadSub.accept(ls) && !leadSub.accept(lt) && !leadSub.accept(st), "RANK1 = 1, RANK2 = 2: leading-subleading only", __LINE__);
  check(backToBack.accept(ls) && !backToBack.accept(lt) && !backToBack.accept(st), "|dphi| and xJ: the back-to-back pair", __LINE__);
  configuration.addProperty(createKey("T","JET_PAIR_FILTER",2,"CONDITION:N"),1);
  set(2,0,"TYPE",String("INTEGER"));  set(2,0,"SUBTYPE",String("PTRATIO"));  set(2,0,"MINIMUM",1.0);  set(2,0,"MAXIMUM",1.0);
  JetPairFilter bad;
  bool threw = false;
  try { bad.configure("T","JET_PAIR_FILTER",configuration,2); } catch (std::exception &) { threw = true; }
  check(threw, "INTEGER condition on the pT ratio: exception", __LINE__);

  Configuration binning;
  JetPairHistos::addDefaultConfiguration(binning,"T");
  JetPairHistos histos;
  histos.configure("T","HISTOGRAM",binning,0);
  histos.create();
  histos.fill(std::vector<JetPair>{ls,lt,st});        // an event with three pairs
  histos.fill(std::vector<JetPair>{ls});              // one with one
  histos.fill(std::vector<JetPair>());                // one with none
  auto h = [&](const char * name) { return histos.histogram(createName("JetPair",name)); };
  check(near(histos.pairCount(),4.0) && near(h("dphi")->integral(),4.0), "four pairs", __LINE__);
  check(near(h("nPairs")->binValueAtGlobal(3),1.0) && near(h("nPairs")->binValueAtGlobal(1),1.0)
        && near(h("nPairs")->binValueAtGlobal(0),1.0), "nPairs: 3, 1, 0", __LINE__);
  histos.normalize(1.0/3.0);
  check(near(sum(h("nPairs")),1.0,1.0E-6), "nPairs per event", __LINE__);
  check(near(sum(h("xJ")),1.0,1.0E-6) && near(sum(h("dphiXJ")),1.0,1.0E-6), "pair histograms per pair", __LINE__);
  const MATH::Profile * p = dynamic_cast<const MATH::Profile*>(histos.histogram(createName("JetPair","xJ_vsPt1")));
  check(p!=nullptr && near(p->binValueAtGlobal(30),1.0/3.0,1.0E-6), "profile <xJ> at pT1 = 30: 1/3", __LINE__);
  }

  // ----- cones of the underlying event: leading jet at (0, 1); a jet at (0.3, 1 + pi/2 + 0.1) covers one side
  {
  JetSettings s;  s.radius = 0.4;  s.particleEtaMax = 2.0;
  std::vector<Jet> two(2);
  two[0].momentum = momentumOf(40.0,0.0,1.0,0.0);              two[0].rank = 1;
  two[1].momentum = momentumOf(20.0,0.3,1.0+0.5*M_PI+0.1,0.0); two[1].rank = 2;
  std::vector<Cone> t = UnderlyingEventCones::cones(two,s,UnderlyingEventCones::Transverse);
  check(t.size()==1 && near(t[0].phi,1.0-0.5*M_PI+2*M_PI) && near(t[0].eta,0.0), "transverse: the cone away from the jet",
        __LINE__);
  std::vector<Cone> shifted = UnderlyingEventCones::cones(two,s,UnderlyingEventCones::TransverseShifted);
  check(shifted.size()==3, "shifted by +-2R: three of four cones (one overlaps the jet)", __LINE__);
  s.particleEtaMax = 0.9;
  check(UnderlyingEventCones::cones(two,s,UnderlyingEventCones::TransverseShifted).empty(), "cones outside the acceptance",
        __LINE__);
  check(UnderlyingEventCones::cones(std::vector<Jet>(),s,UnderlyingEventCones::Transverse).empty(), "no jet: no cone",
        __LINE__);
  check(UnderlyingEventCones::modeOf("transverse_shifted")==UnderlyingEventCones::TransverseShifted, "mode by name", __LINE__);
  std::vector<Particle> ps(4);
  std::vector<Particle*> in;
  ParticleType * types[] = {&plus, &minus, &neutral, &plus};
  const double etas[] = {0.1, -0.2, 0.0, 0.5}, dphis[] = {0.1, 0.0, 0.3, 0.0};
  for (unsigned int k=0; k<4; k++)
    { ps[k].setType(types[k]);  ps[k].setMomentum(momentumOf(1.0,etas[k],t[0].phi+dphis[k]));  in.push_back(&ps[k]); }
  Multiplicities m = UnderlyingEventCones::count(in,t[0],0.4);
  check(m.n==3 && m.nPos==1 && m.nNeg==1 && m.n0==1, "particles within R of the cone axis", __LINE__);
  }

  // ----- moments within jets and the derived quantities: jet A (N+ 2, N- 1), jet B (N+ 1, N- 2, N0 1), both at 20 GeV
  {
  std::vector<Particle> ps(3);
  ps[0].setType(&plus);  ps[1].setType(&minus);  ps[2].setType(&neutral);
  Jet a, b;
  a.momentum = momentumOf(20.2,0.0,1.0,0.0);  a.constituents = {&ps[0], &ps[0], &ps[1]};
  b.momentum = momentumOf(20.7,0.0,4.0,0.0);  b.constituents = {&ps[0], &ps[1], &ps[1], &ps[2]};
  Multiplicities coneA, coneB;
  coneA.add(1.0);  coneB.add(1.0);  coneB.add(-1.0);
  Configuration binning;
  JetFluctuationHistos::addDefaultConfiguration(binning,"T");
  JetFluctuationHistos histos;
  histos.configure("T","HISTOGRAM",binning,0);
  histos.create();
  histos.fillEvent(1);  histos.fill(a,{coneA});
  histos.fillEvent(1);  histos.fill(b,{coneB});
  histos.calculateDerived();
  using H = JetFluctuationHistos;
  auto d = [&](unsigned int q, unsigned int axis, unsigned int bin) { return histos.derived(q,axis)->binValueAtGlobal(bin); };
  auto j = [&](unsigned int q) { return histos.jetProfile(q,H::kIntegrated)->binValueAtGlobal(0); };
  check(near(j(H::kN),3.5,1.0E-6) && near(j(H::kF2),9.0,1.0E-6) && near(j(H::kF2PosNeg),2.0,1.0E-6) && near(j(H::kQ2),1.0,1.0E-6),
        "averages per jet: N, N(N-1), N+N-, Q^2", __LINE__);
  check(near(d(H::kVarN,H::kIntegrated,0),0.25,1.0E-6) && near(d(H::kVarQ,H::kIntegrated,0),1.0,1.0E-6), "variances of N and Q",
        __LINE__);
  check(near(d(H::kR2PP,H::kIntegrated,0),1.0/2.25-1.0,1.0E-6) && near(d(H::kR2PM,H::kIntegrated,0),2.0/2.25-1.0,1.0E-6),
        "R2++ and R2+-", __LINE__);
  check(near(d(H::kNudyn,H::kIntegrated,0),2.0*(1.0/2.25)-2.0*(2.0/2.25),1.0E-6), "nu_dyn = R2++ + R2-- - 2 R2+-", __LINE__);
  check(near(d(H::kSubN,H::kIntegrated,0),2.0,1.0E-6) && near(d(H::kSubF2,H::kIntegrated,0),2.0,1.0E-6),
        "UE subtracted: <s> = <n> - <b>, F2(s) = F2(n) - 2<s><b> - F2(b)", __LINE__);
  check(near(d(H::kSubF2PosNeg,H::kIntegrated,0),2.0-0.5*0.5-1.0*1.0-0.5,1.0E-6), "UE subtracted pairs +-", __LINE__);
  check(near(d(H::kVarN,H::kVsPt,20),0.25,1.0E-6) && near(d(H::kVarN,H::kVsMult,3),0.0,1.0E-6) &&
        near(d(H::kVarN,H::kVsMult,4),0.0,1.0E-6) && histos.derived(H::kVarN,H::kVsMult)->binEntriesAtGlobal(5)==0,
        "versus pT and versus N (one jet per bin of N; empty bins left empty)", __LINE__);
  histos.normalize(0.5);
  check(near(sum(histos.histogram(createName("JetFluct","nCones"))),1.0,1.0E-6) && near(j(H::kN),3.5,1.0E-6),
        "nCones per event, profiles not scaled", __LINE__);
  JetFluctuationHistos copy(histos);
  check(copy.hasDerived() && near(copy.derived(H::kVarN,H::kIntegrated)->binValueAtGlobal(0),0.25,1.0E-6), "copy", __LINE__);
  }

  // ----- UE subtraction, statistically: s+ = c + a, s- = c + e (c, a, e Poisson 1, 1, 0.5), background b+, b- Poisson
  //       0.8, 0.7 in the jet and, independent, in a cone: R2+-(s) = 1/(2 x 1.5) = 1/3, var(s+) = 2
  {
  std::mt19937 random(12345);
  std::poisson_distribution<int> c(1.0), a(1.0), e(0.5), bp(0.8), bm(0.7);
  std::vector<Particle> ps(2);
  ps[0].setType(&plus);  ps[1].setType(&minus);
  Configuration binning;
  JetFluctuationHistos::addDefaultConfiguration(binning,"T");
  JetFluctuationHistos histos;
  histos.configure("T","HISTOGRAM",binning,0);
  histos.create();
  Jet jet;
  jet.momentum = momentumOf(30.0,0.0,1.0,0.0);
  for (unsigned int k=0; k<400000; k++)
    {
    int pairs = c(random), nPos = pairs+a(random)+bp(random), nNeg = pairs+e(random)+bm(random);
    jet.constituents.assign(nPos,&ps[0]);
    jet.constituents.insert(jet.constituents.end(),nNeg,&ps[1]);
    Multiplicities cone;
    for (int i=bp(random); i>0; i--) cone.add(1.0);
    for (int i=bm(random); i>0; i--) cone.add(-1.0);
    histos.fill(jet,{cone});
    }
  histos.calculateDerived();
  using H = JetFluctuationHistos;
  auto d = [&](unsigned int q) { return histos.derived(q,H::kIntegrated)->binValueAtGlobal(0); };
  check(near(d(H::kSubNPos),2.0,0.01) && near(d(H::kSubNNeg),1.5,0.01), "<s+> = 2, <s-> = 1.5", __LINE__);
  check(near(d(H::kSubR2PM),1.0/3.0,0.03) && std::fabs(d(H::kR2PM)-1.0/3.0)>0.1, "R2+- of the jet itself: 1/3", __LINE__);
  check(near(d(H::kSubVarNPos),2.0,0.02), "var(s+) = 2", __LINE__);
  check(std::fabs(d(H::kSubR2PP))<0.01, "R2++(s) = 0 (s+ Poisson)", __LINE__);
  check(near(d(H::kSubNudyn),-2.0/3.0,0.03), "nu_dyn(s) = 0 + 0 - 2/3", __LINE__);
  }

  std::printf("TestJets: %d checks, %d failures\n", nChecks, nFailures);
  return nFailures==0 ? 0 : 1;
}
