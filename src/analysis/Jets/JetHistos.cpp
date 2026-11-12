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

namespace CAP
{

namespace
{
const char * kClass[3] = {"c", "cPos", "cNeg"};
const char * kClassTitle[3] = {"", "^{+}", "^{-}"};
} // namespace

JetHistos::JetHistos()
:
HistogramFolder(),
nJets_nbins(20), nJets_max(20.0),
sumPt_nbins(100), sumPt_max(200.0),
pt_nbins(100), pt_min(0.0), pt_max(100.0),
eta_nbins(20), eta_min(-1.0), eta_max(1.0),
phi_nbins(36),
mass_nbins(50), mass_max(50.0),
mult_nbins(50), mult_max(50.0),
netQ_nbins(40), netQ_max(20.0),
jetCharge_nbins(40), jetCharge_max(2.0),
jetChargeKappa(0.5),
z_nbins(50),
jt_nbins(50), jt_max(5.0),
theta_nbins(50), theta_max(1.0),
r_nbins(20), r_max(1.0),
_jetCount(0.0),
h_nJets(nullptr), h_sumPt(nullptr),
h_pt(nullptr), h_eta(nullptr), h_phi(nullptr), h_etaPhi(nullptr), h_mass(nullptr),
h_mult(nullptr), h_multPos(nullptr), h_multNeg(nullptr), h_mult0(nullptr),
h_netQ(nullptr), h_jetCharge(nullptr),
p_mult(nullptr), p_multPos(nullptr), p_multNeg(nullptr), p_mult0(nullptr), p_netQ(nullptr),
p_mass(nullptr), p_jetCharge(nullptr),
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
  h_pt = histogramLike(source.h_pt);         h_eta = histogramLike(source.h_eta);
  h_phi = histogramLike(source.h_phi);       h_etaPhi = histogramLike(source.h_etaPhi);
  h_mass = histogramLike(source.h_mass);     h_mult = histogramLike(source.h_mult);
  h_multPos = histogramLike(source.h_multPos);  h_multNeg = histogramLike(source.h_multNeg);
  h_mult0 = histogramLike(source.h_mult0);   h_netQ = histogramLike(source.h_netQ);
  h_jetCharge = histogramLike(source.h_jetCharge);
  p_mult = histogramLike(source.p_mult);     p_multPos = histogramLike(source.p_multPos);
  p_multNeg = histogramLike(source.p_multNeg);  p_mult0 = histogramLike(source.p_mult0);
  p_netQ = histogramLike(source.p_netQ);     p_mass = histogramLike(source.p_mass);
  p_jetCharge = histogramLike(source.p_jetCharge);
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

void JetHistos::create()
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  const double twoPi = 2.0*M_PI;
  auto name = [this](const char * n) { return createName(_histogramBaseName,n); };
  h_nJets   = createHistogram(name("nJets"),nJets_nbins,0.0,nJets_max,"n_{jets}","Events");
  h_sumPt   = createHistogram(name("sumPt"),sumPt_nbins,0.0,sumPt_max,"#Sigma p_{T,jet} (GeV/c)","Events");
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
  p_mult    = createProfile(name("mult_vsPt"),pt_nbins,pt_min,pt_max,"p_{T,jet} (GeV/c)","<N>");
  p_multPos = createProfile(name("multPos_vsPt"),pt_nbins,pt_min,pt_max,"p_{T,jet} (GeV/c)","<N_{+}>");
  p_multNeg = createProfile(name("multNeg_vsPt"),pt_nbins,pt_min,pt_max,"p_{T,jet} (GeV/c)","<N_{-}>");
  p_mult0   = createProfile(name("mult0_vsPt"),pt_nbins,pt_min,pt_max,"p_{T,jet} (GeV/c)","<N_{0}>");
  p_netQ    = createProfile(name("netQ_vsPt"),pt_nbins,pt_min,pt_max,"p_{T,jet} (GeV/c)","<Q>");
  p_mass    = createProfile(name("mass_vsPt"),pt_nbins,pt_min,pt_max,"p_{T,jet} (GeV/c)","<m_{jet}>");
  p_jetCharge = createProfile(name("jetCharge_vsPt"),pt_nbins,pt_min,pt_max,"p_{T,jet} (GeV/c)","<Q_{#kappa}>");
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
  auto p = [&](const char * n) { return importProfile(inputFile,createName(_histogramBaseName,n)); };
  h_nJets = h("nJets");  h_sumPt = h("sumPt");
  h_pt = h("pt");  h_eta = h("eta");  h_phi = h("phi");  h_etaPhi = h("etaPhi");  h_mass = h("mass");
  h_mult = h("mult");  h_multPos = h("multPos");  h_multNeg = h("multNeg");  h_mult0 = h("mult0");
  h_netQ = h("netQ");  h_jetCharge = h("jetCharge");
  p_mult = p("mult_vsPt");  p_multPos = p("multPos_vsPt");  p_multNeg = p("multNeg_vsPt");  p_mult0 = p("mult0_vsPt");
  p_netQ = p("netQ_vsPt");  p_mass = p("mass_vsPt");  p_jetCharge = p("jetCharge_vsPt");
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

void JetHistos::fill(const std::vector<const Jet*> & jets)
{
  double sumPt = 0.0;
  for (const Jet * jet : jets) sumPt += jet->pt();
  h_nJets->fill(double(jets.size()));
  h_sumPt->fill(sumPt);
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
    p_mult->fill(pt,n);
    p_multPos->fill(pt,nPos);
    p_multNeg->fill(pt,nNeg);
    p_mult0->fill(pt,n0);
    p_netQ->fill(pt,netQ);
    p_mass->fill(pt,mass);
    p_jetCharge->fill(pt,jetCharge);
    }
}

void JetHistos::scaleObject(double factor)
{
  for (Histogram * h : {h_nJets, h_sumPt}) if (h) h->scale(factor);
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

} // namespace CAP
