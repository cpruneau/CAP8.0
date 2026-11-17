/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
//! @file TestJets.cpp
//! @brief Tests of the jet module: the variables of a particle relative to a jet; the jet settings (configuration);
//! jet finding (three sprays found as three jets with their constituents, no selection but the pT floor, the rank
//! among all the jets of the settings, the settings carried by the jets, several radii); the clustering done once per
//! event, particle filter and settings; the jet filters (kinematics, rank before the conditions); and the filling of
//! the histograms.
#include <cmath>
#include <random>
#include <cstdio>
#include <string>
#include <vector>
#include "Jet.hpp"
#include "JetSettings.hpp"
#include "JetClustering.hpp"
#include "JetFilter.hpp"
#include "JetHistos.hpp"
#include "JetPair.hpp"
#include "JetPairHistos.hpp"
#include "JetUnderlyingEvent.hpp"
#include "JetGroomer.hpp"
#include "JetGroomingHistos.hpp"
#include "Event.hpp"
#include "Particle.hpp"
#include "ParticleType.hpp"
#include "Configuration.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "NameHelpers.hpp"

using namespace Datum;

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

  // ----- jet settings: configuration
  {
  Configuration configuration;
  auto set = [&](unsigned int k, const char * name, auto value)
    { configuration.addProperty(createKey("T","JET_SETTINGS",k,name),value); };
  set(0,"ALGORITHM",String("KT"));  set(0,"R",0.6);  set(0,"RECOMBINATION",String("PT"));  set(0,"PT_FLOOR",2.0);
  JetSettings kt;
  kt.configure("T","JET_SETTINGS",configuration,0);
  check(kt.algorithm==JetSettings::Kt && near(kt.radius,0.6) && kt.recombination==JetSettings::PtScheme
        && near(kt.ptFloor,2.0), "ALGORITHM, R, RECOMBINATION, PT_FLOOR read", __LINE__);
  JetSettings defaults;
  defaults.configure("T","JET_SETTINGS",configuration,1);          // no keys: the defaults
  check(defaults.algorithm==JetSettings::AntiKt && near(defaults.radius,0.4) && defaults.recombination==JetSettings::EScheme
        && near(defaults.ptFloor,1.0), "defaults: anti-kt, R = 0.4, E scheme, pT floor 1", __LINE__);
  check(!kt.sameDefinition(defaults) && defaults.sameDefinition(JetSettings()), "same definition", __LINE__);
  set(2,"ALGORITHM",String("SISCONE"));
  set(3,"R",0.0);
  set(4,"PT_FLOOR",-1.0);
  for (unsigned int k : {2u, 3u, 4u})
    {
    JetSettings bad;
    bool threw = false;
    try { bad.configure("T","JET_SETTINGS",configuration,k); } catch (std::exception &) { threw = true; }
    check(threw, "invalid algorithm, radius or pT floor: exception", __LINE__);
    }
  }

  // ----- jet settings: made by JetSettingsCreator (OWNER true), used by name by the lists of the tasks (OWNER false)
  {
  Configuration configuration;
  JetSettingsCreator creator;
  creator.setName("JetSettings");
  creator.setDefaultConfiguration();
  auto key = [](const char * owner, unsigned int k, const char * name) { return createKey(owner,"JET_SETTINGS",k,name); };
  configuration.addProperty(createKey("JetSettings","JET_SETTINGS","N"),2);
  configuration.addProperty(createKey("Analyzer","JET_SETTINGS","N"),1);
  configuration.addProperty(key("JetSettings",0,"NAME"),String("AntiKtR0p4"));
  configuration.addProperty(key("JetSettings",0,"OWNER"),true);
  configuration.addProperty(key("JetSettings",1,"NAME"),String("KtR0p6"));
  configuration.addProperty(key("JetSettings",1,"OWNER"),true);
  configuration.addProperty(key("JetSettings",1,"ALGORITHM"),String("KT"));
  configuration.addProperty(key("JetSettings",1,"R"),0.6);
  configuration.addProperty(key("Analyzer",0,"NAME"),String("KtR0p6"));
  configuration.addProperty(key("Analyzer",0,"OWNER"),false);
  creator.configure(configuration);
  check(creator.nObjects()==2 && creator.objects()[1]->isNamed("KtR0p6") && near(creator.objects()[1]->radius,0.6),
        "the creator makes the jet settings of the job", __LINE__);
  ManagedObjects<JetSettings> used;
  Configuration defaults;                                     // (the defaults of a task: before the keys of the job)
  used.setDefaultObjectConfiguration("Analyzer","JET_SETTINGS",defaults);
  used.configureObjects(configuration);
  check(used.size()==1 && used.getObjects()[0]==creator.objects()[1], "a task uses them by name (the same object)", __LINE__);
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
  // A: 35 GeV at (0, 1); B: 20 GeV at (0.3, 1+pi); C: 50 GeV at (0.62, 5.5)
  add(&plus,20.0,0.00,1.00);  add(&minus,10.0,0.05,1.05);  add(&neutral,5.0,-0.05,0.95);
  add(&plus,15.0,0.30,1.0+M_PI); add(&plus,5.0,0.32,1.02+M_PI);
  add(&minus,30.0,0.62,5.50);  add(&plus,20.0,0.63,5.52);
  JetSettings settings;  settings.radius = 0.4;  settings.ptFloor = 5.0;
  std::vector<Jet> jets = JetClustering::cluster(particles,settings);
  check(jets.size()==3, "R = 0.4: three jets (no selection in pseudorapidity: that is the jet filters')", __LINE__);
  if (jets.size()==3)
    {
    check(jets[0].rank==1 && jets[1].rank==2 && jets[2].rank==3, "ranks 1, 2, 3 among all the jets", __LINE__);
    check(near(jets[0].pt(),50.0,0.01) && near(jets[0].eta(),0.62,0.01), "leading jet: 50 GeV at eta 0.62", __LINE__);
    check(near(jets[1].pt(),35.0,0.01) && jets[1].multiplicity()==3, "second jet: 35 GeV, 3 constituents", __LINE__);
    check(near(jets[2].pt(),20.0,0.01) && jets[2].multiplicity()==2, "third jet: 20 GeV, 2 constituents", __LINE__);
    double pt = 0;  for (Particle * p : jets[1].constituents) pt += p->momentum().perp();
    check(near(pt,35.0), "constituents of the second jet: the particles of spray A", __LINE__);
    check(jets[0].settings==&settings && jets[2].settings==&settings, "each jet carries its settings", __LINE__);
    }
  settings.radius = 0.2;
  jets = JetClustering::cluster(particles,settings);
  check(jets.size()==3 && near(jets[0].pt(),50.0,0.01) && jets[0].rank==1, "R = 0.2: three jets, 50 GeV leading", __LINE__);
  settings.ptFloor = 25.0;
  jets = JetClustering::cluster(particles,settings);
  check(jets.size()==2 && jets[1].rank==2, "pT floor 25: two jets, the ranks of the two hardest unchanged", __LINE__);
  settings.algorithm = JetSettings::Kt;  settings.ptFloor = 5.0;
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
  const std::vector<Jet> & other = JetClustering::jets(event,nullptr,particles,s2);
  check(JetClustering::nClusterings()==before+2, "other settings: found again", __LINE__);
  check(!other.empty() && other.front().settings==&s2, "the jets of the other settings carry them", __LINE__);
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
  // the rank is that before the conditions: RANK 1 with |eta| < 0.5 keeps the leading jet only if within |eta| < 0.5
  JetSettings wide;  wide.radius = 0.4;
  std::vector<Jet> all = JetClustering::cluster(particles,wide);       // 50 (eta 0.62, rank 1), 35 (eta 0, 2), 20 (3)
  configuration.addProperty(createKey("T","JET_FILTER",3,"CONDITION:N"),1);
  configuration.addProperty(createKey("T","JET_FILTER",3,"CONDITION",0,"TYPE"),String("DOUBLE_RANGE"));
  configuration.addProperty(createKey("T","JET_FILTER",3,"CONDITION",0,"SUBTYPE"),String("ETA"));
  configuration.addProperty(createKey("T","JET_FILTER",3,"CONDITION",0,"MINIMUM"),-0.5);
  configuration.addProperty(createKey("T","JET_FILTER",3,"CONDITION",0,"MAXIMUM"),0.5);
  JetFilter central;
  central.configure("T","JET_FILTER",configuration,3);
  std::vector<unsigned int> ranks;
  for (const Jet & jet : all) if (central.accept(jet)) ranks.push_back(jet.rank);
  check(ranks==std::vector<unsigned int>({2,3}), "ETA in [-0.5, 0.5): the jets of ranks 2 and 3 (ranks kept)", __LINE__);
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
  std::vector<Jet> two(2);
  two[0].momentum = momentumOf(40.0,0.0,1.0,0.0);              two[0].rank = 1;
  two[1].momentum = momentumOf(20.0,0.3,1.0+0.5*M_PI+0.1,0.0); two[1].rank = 2;
  std::vector<Cone> t = UnderlyingEventCones::cones(two,0.4,2.0,UnderlyingEventCones::Transverse);
  check(t.size()==1 && near(t[0].phi,1.0-0.5*M_PI+2*M_PI) && near(t[0].eta,0.0), "transverse: the cone away from the jet",
        __LINE__);
  std::vector<Cone> shifted = UnderlyingEventCones::cones(two,0.4,2.0,UnderlyingEventCones::TransverseShifted);
  check(shifted.size()==3, "shifted by +-2R: three of four cones (one overlaps the jet)", __LINE__);
  check(UnderlyingEventCones::cones(two,0.4,0.9,UnderlyingEventCones::TransverseShifted).empty(), "cones outside the acceptance",
        __LINE__);
  check(UnderlyingEventCones::cones(std::vector<Jet>(),0.4,2.0,UnderlyingEventCones::Transverse).empty(), "no jet: no cone",
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

  // ----- grooming: prongs A (30 + 20 GeV, Delta R 0.02) at (0, 1) and B (30 GeV) at (0.2, 1), a soft particle C
  // (1 GeV) at (0, 1.35): one anti-kt jet of R = 0.4; C/A: C joins last (Delta R ~ 0.35, z ~ 1/81), then A-B
  // (Delta R ~ 0.2, z = 0.375), then the two particles of A (Delta R 0.02, z = 0.4)
  {
  std::vector<Particle> ps(4);
  std::vector<Particle*> in;
  const double pts[] = {30.0, 20.0, 30.0, 1.0}, etas[] = {0.0, 0.0, 0.2, 0.0}, phis[] = {1.0, 1.02, 1.0, 1.35};
  for (unsigned int k=0; k<4; k++)
    { ps[k].setType(&plus);  ps[k].setMomentum(momentumOf(pts[k],etas[k],phis[k]));  in.push_back(&ps[k]); }
  JetSettings antiKt;  antiKt.radius = 0.4;
  std::vector<Jet> one = JetClustering::cluster(in,antiKt);
  check(one.size()==1 && one[0].multiplicity()==4, "grooming: one jet of four constituents", __LINE__);
  const Jet & jet = one.front();
  auto groomer = [](JetGroomer::Method method) { JetGroomer g;  g.method = method;  return g; };
  JetGroomer sd = groomer(JetGroomer::SoftDrop);                       // zcut 0.1, beta 0: modified mass drop
  GroomedJet g = sd.groom(jet);
  check(g.status==GroomedJet::Groomed && near(g.zg,0.375,0.02) && near(g.rg,0.2,0.02) && g.constituents.size()==3,
        "Soft Drop: C dropped, the A-B splitting kept (zg 0.375, Rg 0.2)", __LINE__);
  check(near(g.pt(),80.0,0.1) && near(g.ktg,30.0*g.rg,1.0E-6) && g.nSD==2, "Soft Drop: pT 80, kTg = pT2 Rg, nSD 2",
        __LINE__);
  sd.zCut = 0.5;                                                       // no splitting passes
  g = sd.groom(jet);
  check(g.status==GroomedJet::NoSplitting && g.constituents.size()==1 && near(g.pt(),30.0,0.1) && !g.hasSplitting()
        && g.nSD==0, "Soft Drop, grooming mode: the hardest constituent kept", __LINE__);
  sd.mode = JetGroomer::Tagging;
  check(sd.groom(jet).status==GroomedJet::Failed, "Soft Drop, tagging mode: the jet fails", __LINE__);
  sd.zCut = 0.1;  sd.mode = JetGroomer::Grooming;  sd.beta = 2.0;    // zcut (dR/R0)^2: 0.1/16 at Delta R 0.1
  check(near(sd.groom(jet).zg,0.375,0.02), "Soft Drop beta 2: the same splitting", __LINE__);
  sd.beta = 0.0;  sd.mu = 0.5;                                         // mass drop: m_A << m_AB, passes
  check(sd.groom(jet).status==GroomedJet::Groomed, "mass drop mu 0.5: passes", __LINE__);
  JetGroomer trim = groomer(JetGroomer::Trimming);                     // R_SUB 0.2 f 0.03: C (1 < 2.4) dropped
  trim.recluster = JetSettings::Kt;  trim.rSub = 0.1;
  g = trim.groom(jet);
  check(g.status==GroomedJet::Groomed && g.constituents.size()==3 && near(g.pt(),80.0,0.1) && !g.hasSplitting(),
        "trimming: the soft subjet dropped", __LINE__);
  trim.fCut = 0.9;
  check(trim.groom(jet).status==GroomedJet::Failed, "trimming above every subjet: fails", __LINE__);
  JetGroomer filter = groomer(JetGroomer::Filtering);
  filter.rFilt = 0.1;  filter.nKeep = 1;
  g = filter.groom(jet);
  check(g.constituents.size()==2 && near(g.pt(),50.0,0.1), "filtering, N_KEEP 1: prong A only", __LINE__);
  filter.nKeep = 3;
  check(filter.groom(jet).constituents.size()==4, "filtering, N_KEEP 3: everything", __LINE__);
  JetGroomer prune = groomer(JetGroomer::Pruning);                    // D_cut = m/pT ~ 0.1: C pruned, B kept
  g = prune.groom(jet);
  check(g.status==GroomedJet::Groomed && g.constituents.size()==3 && near(g.pt(),80.0,0.1),
        "pruning: the soft wide-angle C pruned", __LINE__);
  prune.zCut = 0.45;                                                   // B (z 0.375) pruned too
  check(prune.groom(jet).constituents.size()==2, "pruning, z_cut 0.45: B pruned too", __LINE__);
  JetGroomer dyg = groomer(JetGroomer::Dynamical);                    // kTDrop: kappa largest at A-B
  g = dyg.groom(jet);
  check(g.status==GroomedJet::Groomed && near(g.zg,0.375,0.02) && g.constituents.size()==3 && g.nSD==-1,
        "dynamical grooming a = 1: the A-B splitting", __LINE__);
  // zDrop: kappa = z (1 - z) pT_parent (dR/R)^0.1: A-B 0.234 x 80 x 0.93 = 17.5 against A 0.24 x 50 x 0.74 = 8.9
  dyg.a = 0.1;
  check(near(dyg.groom(jet).zg,0.375,0.02), "zDrop: kappa weighted by the pT of the parent, A-B still", __LINE__);
  // without B: A-C (z 1/51, dR 0.35) and A (z 0.4, dR 0.02): kTDrop kappa 0.86 against 0.60 (the wide splitting),
  // zDrop 0.97 against 8.9 (the symmetric one)
  Jet noB;  noB.constituents = {&ps[0], &ps[1], &ps[3]};
  for (Particle * p : noB.constituents) noB.momentum += p->momentum();
  dyg.a = 1.0;
  check(near(dyg.groom(noB).zg,1.0/51.0,0.01) && dyg.groom(noB).constituents.size()==3, "kTDrop without B: A-C",
        __LINE__);
  dyg.a = 0.1;
  check(near(dyg.groom(noB).zg,0.4,0.01) && dyg.groom(noB).constituents.size()==2, "zDrop without B: the splitting of A",
        __LINE__);
  Jet single;  single.constituents = {&ps[0]};  single.momentum = ps[0].momentum();
  check(dyg.groom(single).status==GroomedJet::NoSplitting, "one constituent: no splitting", __LINE__);
  check(groomer(JetGroomer::SoftDrop).groom(Jet()).status==GroomedJet::Failed, "no constituent: fails", __LINE__);
  // configuration
  Configuration configuration;
  auto set = [&](const char * name, auto value) { configuration.addProperty(createKey("T","JET_GROOMER",0,name),value); };
  set("METHOD",String("PRUNING"));  set("RECLUSTER",String("KT"));  set("Z_CUT",0.15);  set("R_CUT_FACTOR",0.4);
  JetGroomer read;
  read.configure("T","JET_GROOMER",configuration,0);
  check(read.method==JetGroomer::Pruning && read.recluster==JetSettings::Kt && near(read.zCut,0.15)
        && near(read.rCutFactor,0.4) && near(read.beta,0.0), "groomer read from the configuration", __LINE__);
  configuration.addProperty(createKey("T","JET_GROOMER",1,"METHOD"),String("RECURSIVE"));
  bool threw = false;
  try { read.configure("T","JET_GROOMER",configuration,1); } catch (std::exception &) { threw = true; }
  check(threw, "unknown grooming method: exception", __LINE__);
  // histograms: three jets groomed (Soft Drop, no splitting, failed)
  Configuration binning;
  JetGroomingHistos::addDefaultConfiguration(binning,"T");
  JetGroomingHistos histos;
  histos.configure("T","HISTOGRAM",binning,0);
  histos.create();
  JetGroomer sd2 = groomer(JetGroomer::SoftDrop);
  histos.fill(sd2.groom(jet));
  sd2.zCut = 0.5;  histos.fill(sd2.groom(jet));
  sd2.mode = JetGroomer::Tagging;  histos.fill(sd2.groom(jet));
  auto h = [&](const char * name) { return histos.histogram(createName("JetGroomed",name)); };
  check(near(h("status")->integral(),3.0) && near(h("zg")->integral(),1.0) && near(h("pt")->integral(),2.0)
        && near(h("nSD")->integral(),3.0), "status of 3 jets, zg of 1, pT of the 2 kept, nSD of 3", __LINE__);
  histos.normalize(1.0);
  check(near(sum(h("zg")),1.0/3.0,1.0E-6) && near(sum(h("status")),1.0,1.0E-6), "per jet groomed", __LINE__);
  }

  // ----- moments within jets and of the underlying event (JetHistos): jet A (N+ 2, N- 1) at 20.2 GeV, jet B (N+ 1,
  //       N- 2, N0 1) at 20.7 GeV, in one event with cones (1, 0, 0) and (0, 0, 1) of N+, N-, N0; jet C (N+ 3) at 40 GeV in
  //       an event without a cone
  {
  std::vector<Particle> ps(3);
  ps[0].setType(&plus);  ps[1].setType(&minus);  ps[2].setType(&neutral);
  for (Particle & p : ps) p.setMomentum(momentumOf(5.0,0.0,1.0));
  Jet a, b, c;
  a.momentum = momentumOf(20.2,0.0,1.0,0.0);  a.constituents = {&ps[0], &ps[0], &ps[1]};
  b.momentum = momentumOf(20.7,0.0,4.0,0.0);  b.constituents = {&ps[0], &ps[1], &ps[1], &ps[2]};
  c.momentum = momentumOf(40.5,0.0,2.0,0.0);  c.constituents = {&ps[0], &ps[0], &ps[0]};
  Multiplicities coneP, coneN0;
  coneP.add(1.0);  coneN0.add(0.0);
  Configuration binning;
  JetHistos::addDefaultConfiguration(binning,"T");
  JetHistos histos;
  histos.configure("T","HISTOGRAM",binning,0);
  histos.create();
  histos.fill({&a, &b},{coneP, coneN0});
  histos.fill({&c},{});
  using H = JetHistos;
  auto pt = [&](unsigned int m, unsigned int bin) { return histos.profile(m,H::kVsPt)->binValueAtGlobal(bin); };
  auto mult = [&](unsigned int m, unsigned int bin) { return histos.profile(m,H::kVsMult)->binValueAtGlobal(bin); };
  // versus pT: A and B in the bin of 20 GeV, C in that of 40 GeV
  check(near(pt(H::kMultPos,20),1.5,1.0E-6) && near(pt(H::kMultNeg,20),1.5,1.0E-6) && near(pt(H::kMult0,20),0.5,1.0E-6)
        && near(pt(H::kMult,20),3.5,1.0E-6), "<N+>, <N->, <N0>, <N> vs pT", __LINE__);
  check(near(pt(H::kF2Pos,20),1.0,1.0E-6) && near(pt(H::kF2Neg,20),1.0,1.0E-6) && near(pt(H::kF20,20),0.0,1.0E-6)
        && near(pt(H::kF2,20),9.0,1.0E-6), "second factorial moments vs pT: (2 + 0)/2, (0 + 2)/2, 0, (6 + 12)/2", __LINE__);
  check(near(pt(H::kPosNeg,20),(2.0+2.0)/2.0,1.0E-6) && near(mult(H::kPosNeg,4),2.0,1.0E-6) && near(pt(H::kPosNeg,40),0.0,1.0E-6),
        "<N+ N-> vs pT and vs N", __LINE__);
  check(near(pt(H::kNetQ,20),0.0,1.0E-6) && near(pt(H::kNetQF2,20),1.0,1.0E-6), "<Q> = (1 - 1)/2, <Q(Q-1)> = (0 + 2)/2",
        __LINE__);
  check(near(pt(H::kMultPos,40),3.0,1.0E-6) && near(pt(H::kF2Pos,40),6.0,1.0E-6) && near(pt(H::kNetQF2,40),6.0,1.0E-6),
        "jet C at 40 GeV", __LINE__);
  // versus N: A (N 3), B and C... C has N 3 too: bin 3 holds A and C, bin 4 holds B
  check(near(mult(H::kMultPos,3),2.5,1.0E-6) && near(mult(H::kF2Pos,3),4.0,1.0E-6) && near(mult(H::kMultNeg,4),2.0,1.0E-6)
        && near(mult(H::kF2Neg,4),2.0,1.0E-6) && near(mult(H::kMult0,4),1.0,1.0E-6), "moments vs N", __LINE__);
  check(histos.profile(H::kMult,H::kVsMult)==nullptr && histos.profile(H::kF2,H::kVsMult)==nullptr
        && histos.profile(H::kMass,H::kVsMult)==nullptr && histos.profile(H::kMass,H::kVsPt)!=nullptr,
        "no trivial profiles vs N", __LINE__);
  // the jet charge of jet C alone at 40 GeV: the second moment is the square of the first
  double q1 = pt(H::kJetCharge,40), q2 = pt(H::kJetCharge2,40);
  check(q1>0.0 && near(q2,q1*q1,1.0E-6), "<Q_kappa^2> = <Q_kappa>^2 for one jet", __LINE__);
  // underlying event: the two cones, for each of the jets A and B; none for C
  auto ue = [&](unsigned int m, unsigned int axis, unsigned int bin) { return histos.ueProfile(m,axis)->binValueAtGlobal(bin); };
  check(near(ue(H::kUeMult,H::kVsPt,20),1.0,1.0E-6) && near(ue(H::kUeMultPos,H::kVsPt,20),0.5,1.0E-6)
        && near(ue(H::kUeMult0,H::kVsPt,20),0.5,1.0E-6) && near(ue(H::kUeMultNeg,H::kVsPt,20),0.0,1.0E-6),
        "UE multiplicities per cone vs pT", __LINE__);
  check(histos.ueProfile(H::kUeMult,H::kVsPt)->binEntriesAtGlobal(40)==0, "no cone: the bin of C empty", __LINE__);
  // second factorial moments of the cones: add a cone with N+ 2, N- 1 to an event of jet A: F2 (3 x 2, 2 x 1, 0)
  {
  JetHistos one;
  one.configure("T","HISTOGRAM",binning,0);
  one.create();
  Multiplicities big;
  big.add(1.0);  big.add(1.0);  big.add(-1.0);
  one.fill({&a},{big, coneP});
  auto u = [&](unsigned int m) { return one.ueProfile(m,H::kVsPt)->binValueAtGlobal(20); };
  check(near(u(H::kUeF2),(6.0+0.0)/2.0,1.0E-6) && near(u(H::kUeF2Pos),(2.0+0.0)/2.0,1.0E-6) && near(u(H::kUeF2Neg),0.0,1.0E-6)
        && near(one.ueProfile(H::kUeF2Pos,H::kVsMult)->binValueAtGlobal(3),1.0,1.0E-6),
        "UE second factorial moments per cone, vs pT and vs N", __LINE__);
  check(near(u(H::kUePosNeg),(2.0+0.0)/2.0,1.0E-6), "UE <N+ N-> per cone", __LINE__);
  }
  check(near(ue(H::kUeMultPos,H::kVsMult,4),0.5,1.0E-6) && near(ue(H::kUeMultPos,H::kVsMult,3),0.5,1.0E-6),
        "UE vs N: the cones of A (N 3) and B (N 4)", __LINE__);
  auto h = [&](const char * name) { return histos.histogram(createName("Jet",name)); };
  check(near(h("nCones")->binValueAtGlobal(2),1.0) && near(h("nCones")->binValueAtGlobal(0),1.0), "cones: 2 and 0",
        __LINE__);
  histos.normalize(0.5);
  check(near(sum(h("nCones")),1.0,1.0E-6) && near(pt(H::kMultPos,20),1.5,1.0E-6), "nCones per event, profiles not scaled",
        __LINE__);
  check(histos.histogram(createName("Jet","f2Pos_vsMult"))!=nullptr && histos.histogram(createName("Jet","ue_mult0_vsPt"))!=nullptr
        && histos.histogram(createName("Jet","netQf2_vsPt"))!=nullptr, "names", __LINE__);
  JetHistos copy(histos);
  check(near(copy.profile(H::kF2Pos,H::kVsPt)->binValueAtGlobal(20),1.0,1.0E-6) && copy.ueProfile(H::kUeMult,H::kVsMult)!=nullptr,
        "copy", __LINE__);
  }

  std::printf("TestJets: %d checks, %d failures\n", nChecks, nFailures);
  return nFailures==0 ? 0 : 1;
}
