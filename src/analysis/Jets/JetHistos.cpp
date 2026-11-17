/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "JetHistos.hpp"
#include <cmath>
#include "Exceptions.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "Particle.hpp"

namespace Datum
{

namespace
{
const char * kClass[3] = {"c", "cPos", "cNeg"};
const char * kClassTitle[3] = {"", "^{+}", "^{-}"};
const char * kMomentName[JetHistos::kMoments] = {"mult", "multPos", "multNeg", "mult0", "f2", "f2Pos", "f2Neg", "f20",
                                                  "posNeg", "netQ", "netQf2", "jetCharge", "jetCharge2", "mass"};
const char * kMomentTitle[JetHistos::kMoments] = {"<N>", "<N_{+}>", "<N_{-}>", "<N_{0}>", "<N(N-1)>",
                                                   "<N_{+}(N_{+}-1)>", "<N_{-}(N_{-}-1)>", "<N_{0}(N_{0}-1)>", "<N_{+}N_{-}>", "<Q>",
                                                   "<Q(Q-1)>", "<Q_{#kappa}>", "<Q_{#kappa}^{2}>", "<m_{jet}>"};
const char * kUeName[JetHistos::kUeMoments] = {"ue_mult", "ue_multPos", "ue_multNeg", "ue_mult0", "ue_f2",
                                                "ue_f2Pos", "ue_f2Neg", "ue_posNeg"};
const char * kUeTitle[JetHistos::kUeMoments] = {"<N>_{UE}", "<N_{+}>_{UE}", "<N_{-}>_{UE}", "<N_{0}>_{UE}",
                                                 "<N(N-1)>_{UE}", "<N_{+}(N_{+}-1)>_{UE}", "<N_{-}(N_{-}-1)>_{UE}",
                                                 "<N_{+}N_{-}>_{UE}"};
const char * kAxisSuffix[JetHistos::kAxes] = {"_vsPt", "_vsMult"};
} // namespace

JetHistos::JetHistos()
:
HistogramFolder(),
nJets_nbins(20), nJets_max(20.0),
sumPt_nbins(100), sumPt_max(200.0),
nCones_nbins(5), nCones_max(5.0),
pt_nbins(100), pt_min(0.0), pt_max(100.0),
eta_nbins(20), eta_min(-1.0), eta_max(1.0),
phi_nbins(36),
mass_nbins(50), mass_max(50.0),
mult_nbins(50), mult_max(50.0),
netQ_nbins(40), netQ_max(20.0),
jetCharge_nbins(80), jetCharge_max(9.0),
jetChargeKappa(0.5),
z_nbins(50),
jt_nbins(50), jt_max(5.0),
theta_nbins(50), theta_max(1.0),
r_nbins(20), r_max(1.0),
_jetCount(0.0),
h_nJets(nullptr), h_sumPt(nullptr), h_nCones(nullptr),
h_pt(nullptr), h_eta(nullptr), h_phi(nullptr), h_etaPhi(nullptr), h_mass(nullptr),
h_mult(nullptr), h_multPos(nullptr), h_multNeg(nullptr), h_mult0(nullptr),
h_netQ(nullptr), h_jetCharge(nullptr),
_profiles(kAxes*kMoments,nullptr), _ueProfiles(kAxes*kUeMoments,nullptr),
h_z(3,nullptr), h_jt(3,nullptr), h_theta(3,nullptr), h_r(3,nullptr), h_rho(3,nullptr)
{
  appendClassName("JetHistos");
  setName("JetHistos");
  setTitle("JetHistos");
  setHistogramBaseName("Jet");
}

JetHistos::JetHistos(const JetHistos & source)
:
HistogramFolder(source)
{
  *this = source;
}

JetHistos & JetHistos::operator=(const JetHistos & rhs)
{
  if (this!=&rhs)
    {
    HistogramFolder::operator=(rhs);
    nJets_nbins = rhs.nJets_nbins;  nJets_max = rhs.nJets_max;
    sumPt_nbins = rhs.sumPt_nbins;  sumPt_max = rhs.sumPt_max;
    nCones_nbins = rhs.nCones_nbins;  nCones_max = rhs.nCones_max;
    pt_nbins = rhs.pt_nbins;  pt_min = rhs.pt_min;  pt_max = rhs.pt_max;
    eta_nbins = rhs.eta_nbins;  eta_min = rhs.eta_min;  eta_max = rhs.eta_max;
    phi_nbins = rhs.phi_nbins;
    mass_nbins = rhs.mass_nbins;  mass_max = rhs.mass_max;
    mult_nbins = rhs.mult_nbins;  mult_max = rhs.mult_max;
    netQ_nbins = rhs.netQ_nbins;  netQ_max = rhs.netQ_max;
    jetCharge_nbins = rhs.jetCharge_nbins;  jetCharge_max = rhs.jetCharge_max;  jetChargeKappa = rhs.jetChargeKappa;
    z_nbins = rhs.z_nbins;
    jt_nbins = rhs.jt_nbins;  jt_max = rhs.jt_max;
    theta_nbins = rhs.theta_nbins;  theta_max = rhs.theta_max;
    r_nbins = rhs.r_nbins;  r_max = rhs.r_max;
    _jetCount = rhs._jetCount;
    cloneB(rhs);
    }
  return *this;
}

void JetHistos::cloneB(const JetHistos & source)
{
  h_nJets = histogramLike(source.h_nJets);   h_sumPt = histogramLike(source.h_sumPt);
  h_nCones = histogramLike(source.h_nCones);
  h_pt = histogramLike(source.h_pt);         h_eta = histogramLike(source.h_eta);
  h_phi = histogramLike(source.h_phi);       h_etaPhi = histogramLike(source.h_etaPhi);
  h_mass = histogramLike(source.h_mass);     h_mult = histogramLike(source.h_mult);
  h_multPos = histogramLike(source.h_multPos);  h_multNeg = histogramLike(source.h_multNeg);
  h_mult0 = histogramLike(source.h_mult0);   h_netQ = histogramLike(source.h_netQ);
  h_jetCharge = histogramLike(source.h_jetCharge);
  _profiles.assign(kAxes*kMoments,nullptr);
  _ueProfiles.assign(kAxes*kUeMoments,nullptr);
  for (unsigned int k=0; k<source._profiles.size() && k<_profiles.size(); k++) _profiles[k] = histogramLike(source._profiles[k]);
  for (unsigned int k=0; k<source._ueProfiles.size() && k<_ueProfiles.size(); k++)
    _ueProfiles[k] = histogramLike(source._ueProfiles[k]);
  h_z.resize(3); h_jt.resize(3); h_theta.resize(3); h_r.resize(3); h_rho.resize(3);
  for (unsigned int k=0; k<3; k++)
    {
    h_z[k] = histogramLike(source.h_z[k]);      h_jt[k] = histogramLike(source.h_jt[k]);
    h_theta[k] = histogramLike(source.h_theta[k]);  h_r[k] = histogramLike(source.h_r[k]);
    h_rho[k] = histogramLike(source.h_rho[k]);
    }
}

void JetHistos::addDefaultConfiguration(Configuration & configuration, const String & taskName)
{
  auto add = [&](const char * name, auto value) { configuration.addProperty(createKey(taskName,"HISTOGRAM",name),value); };
  add("nJets_nbins",20);      add("nJets_max",20.0);
  add("sumPt_nbins",100);     add("sumPt_max",200.0);
  add("nCones_nbins",5);      add("nCones_max",5.0);
  add("pt_nbins",100);        add("pt_min",0.0);       add("pt_max",100.0);
  add("eta_nbins",20);        add("eta_min",-1.0);     add("eta_max",1.0);
  add("phi_nbins",36);
  add("mass_nbins",50);       add("mass_max",50.0);
  add("mult_nbins",50);       add("mult_max",50.0);
  add("netQ_nbins",40);       add("netQ_max",20.0);
  add("jetCharge_nbins",40);  add("jetCharge_max",2.0);  add("jetChargeKappa",0.5);
  add("z_nbins",50);
  add("jt_nbins",50);         add("jt_max",5.0);
  add("theta_nbins",50);      add("theta_max",1.0);
  add("r_nbins",20);          add("r_max",1.0);
}

void JetHistos::configure(const String & taskName, const String & objectType, const Configuration & configuration,
                          unsigned int index __attribute__((unused)))
{
  String type = objectType.beginsWith("HISTOGRAM") ? String("HISTOGRAM") : objectType;
  auto i = [&](const char * name) { return configuration.valueInt(createKey(taskName,type,name)); };
  auto d = [&](const char * name) { return configuration.valueDouble(createKey(taskName,type,name)); };
  nJets_nbins = i("nJets_nbins");  nJets_max = d("nJets_max");
  sumPt_nbins = i("sumPt_nbins");  sumPt_max = d("sumPt_max");
  nCones_nbins = i("nCones_nbins");  nCones_max = d("nCones_max");
  pt_nbins = i("pt_nbins");  pt_min = d("pt_min");  pt_max = d("pt_max");
  eta_nbins = i("eta_nbins");  eta_min = d("eta_min");  eta_max = d("eta_max");
  phi_nbins = i("phi_nbins");
  mass_nbins = i("mass_nbins");  mass_max = d("mass_max");
  mult_nbins = i("mult_nbins");  mult_max = d("mult_max");
  netQ_nbins = i("netQ_nbins");  netQ_max = d("netQ_max");
  jetCharge_nbins = i("jetCharge_nbins");  jetCharge_max = d("jetCharge_max");  jetChargeKappa = d("jetChargeKappa");
  z_nbins = i("z_nbins");
  jt_nbins = i("jt_nbins");  jt_max = d("jt_max");
  theta_nbins = i("theta_nbins");  theta_max = d("theta_max");
  r_nbins = i("r_nbins");  r_max = d("r_max");
}

const char * JetHistos::momentName(unsigned int m)   { return m<kMoments ? kMomentName[m] : ""; }
const char * JetHistos::ueMomentName(unsigned int m) { return m<kUeMoments ? kUeName[m] : ""; }

String JetHistos::profileName(const char * name, unsigned int axis)
{
  return String(name)+String(kAxisSuffix[axis]);
}

void JetHistos::create()
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  const double twoPi = 2.0*M_PI;
  auto name = [this](const char * n) { return createName(_histogramBaseName,n); };
  h_nJets   = createHistogram(name("nJets"),nJets_nbins,0.0,nJets_max,"n_{jets}","Events");
  h_sumPt   = createHistogram(name("sumPt"),sumPt_nbins,0.0,sumPt_max,"#Sigma p_{T,jet} (GeV/c)","Events");
  h_nCones  = createHistogram(name("nCones"),nCones_nbins,0.0,nCones_max,"n_{cones}","Events");
  h_pt      = createHistogram(name("pt"),pt_nbins,pt_min,pt_max,"p_{T,jet} (GeV/c)","Jets");
  h_eta     = createHistogram(name("eta"),eta_nbins,eta_min,eta_max,"#eta_{jet}","Jets");
  h_phi     = createHistogram(name("phi"),phi_nbins,0.0,twoPi,"#varphi_{jet}","Jets");
  h_etaPhi  = createHistogram(name("etaPhi"),eta_nbins,eta_min,eta_max,phi_nbins,0.0,twoPi,"#eta_{jet}","#varphi_{jet}","Jets");
  h_mass    = createHistogram(name("mass"),mass_nbins,0.0,mass_max,"m_{jet} (GeV/c^{2})","Jets");
  h_mult    = createHistogram(name("mult"),mult_nbins,0.0,mult_max,"N","Jets");
  h_multPos = createHistogram(name("multPos"),mult_nbins,0.0,mult_max,"N_{+}","Jets");
  h_multNeg = createHistogram(name("multNeg"),mult_nbins,0.0,mult_max,"N_{-}","Jets");
  h_mult0   = createHistogram(name("mult0"),mult_nbins,0.0,mult_max,"N_{0}","Jets");
  h_netQ    = createHistogram(name("netQ"),netQ_nbins,-netQ_max,netQ_max,"Q","Jets");
  h_jetCharge = createHistogram(name("jetCharge"),jetCharge_nbins,-jetCharge_max,jetCharge_max,"Q_{#kappa}","Jets");
  for (unsigned int a=0; a<kAxes; a++)
    {
    const bool vsPt = a==kVsPt;
    const int nBins = vsPt ? pt_nbins : mult_nbins;
    const double low = vsPt ? pt_min : 0.0, high = vsPt ? pt_max : mult_max;
    const char * axisTitle = vsPt ? "p_{T,jet} (GeV/c)" : "N";
    for (unsigned int m=0; m<kMoments; m++)
      _profiles[a*kMoments+m] = hasProfile(m,a)
        ? createProfile(name(profileName(kMomentName[m],a).data()),nBins,low,high,axisTitle,kMomentTitle[m]) : nullptr;
    for (unsigned int m=0; m<kUeMoments; m++)
      _ueProfiles[a*kUeMoments+m] = createProfile(name(profileName(kUeName[m],a).data()),nBins,low,high,axisTitle,kUeTitle[m]);
    }
  for (unsigned int k=0; k<3; k++)
    {
    String c = kClass[k], t = kClassTitle[k];
    h_z[k]     = createHistogram(createName(_histogramBaseName,c+"_z"),z_nbins,0.0,1.0+1.0E-9,"z","N"+t);
    h_jt[k]    = createHistogram(createName(_histogramBaseName,c+"_jt"),jt_nbins,0.0,jt_max,"j_{T} (GeV/c)","N"+t);
    h_theta[k] = createHistogram(createName(_histogramBaseName,c+"_theta"),theta_nbins,0.0,theta_max,"#theta","N"+t);
    h_r[k]     = createHistogram(createName(_histogramBaseName,c+"_r"),r_nbins,0.0,r_max,"r","N"+t);
    h_rho[k]   = createHistogram(createName(_histogramBaseName,c+"_rho"),r_nbins,0.0,r_max,"r","#Sigma p_{T}/p_{T,jet}"+t);
    }
}

void JetHistos::loadFrom(const Folder & inputFile)
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  auto h = [&](const char * n) { return importHistogram(inputFile,createName(_histogramBaseName,n)); };
  auto p = [&](const String & n) { return importProfile(inputFile,createName(_histogramBaseName,n)); };
  h_nJets = h("nJets");  h_sumPt = h("sumPt");  h_nCones = h("nCones");
  h_pt = h("pt");  h_eta = h("eta");  h_phi = h("phi");  h_etaPhi = h("etaPhi");  h_mass = h("mass");
  h_mult = h("mult");  h_multPos = h("multPos");  h_multNeg = h("multNeg");  h_mult0 = h("mult0");
  h_netQ = h("netQ");  h_jetCharge = h("jetCharge");
  _profiles.assign(kAxes*kMoments,nullptr);
  _ueProfiles.assign(kAxes*kUeMoments,nullptr);
  for (unsigned int a=0; a<kAxes; a++)
    {
    for (unsigned int m=0; m<kMoments; m++)
      if (hasProfile(m,a)) _profiles[a*kMoments+m] = p(profileName(kMomentName[m],a));
    for (unsigned int m=0; m<kUeMoments; m++) _ueProfiles[a*kUeMoments+m] = p(profileName(kUeName[m],a));
    }
  for (unsigned int k=0; k<3; k++)
    {
    String c = kClass[k];
    h_z[k]     = importHistogram(inputFile,createName(_histogramBaseName,c+"_z"));
    h_jt[k]    = importHistogram(inputFile,createName(_histogramBaseName,c+"_jt"));
    h_theta[k] = importHistogram(inputFile,createName(_histogramBaseName,c+"_theta"));
    h_r[k]     = importHistogram(inputFile,createName(_histogramBaseName,c+"_r"));
    h_rho[k]   = importHistogram(inputFile,createName(_histogramBaseName,c+"_rho"));
    }
}

void JetHistos::fillConstituent(unsigned int k, double z, double jt, double theta, double r)
{
  h_z[k]->fill(z);
  h_jt[k]->fill(jt);
  h_theta[k]->fill(theta);
  h_r[k]->fill(r);
  h_rho[k]->fill(r,z);
}

void JetHistos::fill(const std::vector<const Jet*> & jets, const std::vector<Multiplicities> & cones)
{
  double sumPt = 0.0;
  for (const Jet * jet : jets) sumPt += jet->pt();
  h_nJets->fill(double(jets.size()));
  h_sumPt->fill(sumPt);
  h_nCones->fill(double(cones.size()));
  _jetCount += double(jets.size());
  for (const Jet * jet : jets)
    {
    const double pt = jet->pt(), eta = jet->eta(), phi = jet->phi(), mass = jet->mass();
    double nPos = 0, nNeg = 0, n0 = 0, netQ = 0, jetCharge = 0;
    for (const Particle * particle : jet->constituents)
      {
      const MATH::VectorLorentz & p = particle->momentum();
      double q = particle->type().charge();
      double z = momentumFraction(p,jet->momentum);
      double jt = transverseToJet(p,jet->momentum);
      double theta = angleToJet(p,jet->momentum);
      double r = deltaR(p,jet->momentum);
      fillConstituent(0,z,jt,theta,r);
      if (q>0)      { nPos += 1.0; fillConstituent(1,z,jt,theta,r); }
      else if (q<0) { nNeg += 1.0; fillConstituent(2,z,jt,theta,r); }
      else            n0 += 1.0;
      netQ += q;
      if (q!=0.0 && z>0.0) jetCharge += q*std::pow(z,jetChargeKappa);
      }
    const double n = jet->multiplicity();
    h_pt->fill(pt);
    h_eta->fill(eta);
    h_phi->fill(phi);
    h_etaPhi->fill(eta,phi,1.0);
    h_mass->fill(mass);
    h_mult->fill(n);
    h_multPos->fill(nPos);
    h_multNeg->fill(nNeg);
    h_mult0->fill(n0);
    h_netQ->fill(netQ);
    h_jetCharge->fill(jetCharge);
    double v[kMoments];
    v[kMult] = n;           v[kMultPos] = nPos;              v[kMultNeg] = nNeg;              v[kMult0] = n0;
    v[kF2] = n*(n-1.0);     v[kF2Pos] = nPos*(nPos-1.0);     v[kF2Neg] = nNeg*(nNeg-1.0);     v[kF20] = n0*(n0-1.0);
    v[kPosNeg] = nPos*nNeg;
    v[kNetQ] = netQ;        v[kNetQF2] = netQ*(netQ-1.0);    v[kJetCharge] = jetCharge;       v[kJetCharge2] = jetCharge*jetCharge;
    v[kMass] = mass;
    const double x[kAxes] = {pt, n};
    for (unsigned int a=0; a<kAxes; a++)
      {
      for (unsigned int m=0; m<kMoments; m++) if (Profile * prof = _profiles[a*kMoments+m]) prof->fill(x[a],v[m]);
      for (const Multiplicities & cone : cones)
        {
        const double b[kUeMoments] = {cone.n, cone.nPos, cone.nNeg, cone.n0, cone.n*(cone.n-1.0),
                                      cone.nPos*(cone.nPos-1.0), cone.nNeg*(cone.nNeg-1.0), cone.nPos*cone.nNeg};
        for (unsigned int m=0; m<kUeMoments; m++) _ueProfiles[a*kUeMoments+m]->fill(x[a],b[m]);
        }
      }
    }
}

void JetHistos::scaleObject(double factor)
{
  for (Histogram * h : {h_nJets, h_sumPt, h_nCones}) if (h) h->scale(factor);
}

void JetHistos::normalize(double eventFactor)
{
  scaleObject(eventFactor);
  if (!(_jetCount>0.0)) return;
  const double perJet = 1.0/_jetCount;
  std::vector<Histogram*> jetHistograms = {h_pt, h_eta, h_phi, h_etaPhi, h_mass, h_mult, h_multPos, h_multNeg, h_mult0,
                                           h_netQ, h_jetCharge};
  for (unsigned int k=0; k<3; k++)
    for (Histogram * h : {h_z[k], h_jt[k], h_theta[k], h_r[k], h_rho[k]}) jetHistograms.push_back(h);
  for (Histogram * h : jetHistograms) if (h) h->scale(perJet);
}

} // namespace Datum
