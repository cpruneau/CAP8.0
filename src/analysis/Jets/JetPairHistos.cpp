/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "JetPairHistos.hpp"
#include <cmath>
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"

namespace CAP
{

JetPairHistos::JetPairHistos()
:
HistogramFolder(),
nPairs_nbins(100), nPairs_max(100.0),
pt_nbins(100), pt_min(0.0), pt_max(100.0),
eta_nbins(20), eta_min(-1.0), eta_max(1.0),
dphi_nbins(72),
acop_nbins(60), acop_max(M_PI),
deta_nbins(40), deta_max(2.0),
xJ_nbins(50),
mass_nbins(100), mass_max(200.0),
pairPt_nbins(50), pairPt_max(50.0),
y_nbins(20), y_max(1.0),
_pairCount(0.0),
h_nPairs(nullptr),
h_pt1(nullptr), h_pt2(nullptr), h_pt1pt2(nullptr), h_eta1(nullptr), h_eta2(nullptr),
h_dphi(nullptr), h_acop(nullptr), h_deta(nullptr), h_xJ(nullptr), h_aJ(nullptr),
h_mass(nullptr), h_pairPt(nullptr), h_y(nullptr), h_dphiXJ(nullptr),
p_xJ(nullptr), p_aJ(nullptr), p_acop(nullptr)
{
  appendClassName("JetPairHistos");
  setName("JetPairHistos");
  setTitle("JetPairHistos");
  setHistogramBaseName("JetPair");
}

JetPairHistos::JetPairHistos(const JetPairHistos & source)
:
HistogramFolder(source)
{
  *this = source;
}

JetPairHistos & JetPairHistos::operator=(const JetPairHistos & rhs)
{
  if (this!=&rhs)
    {
    HistogramFolder::operator=(rhs);
    nPairs_nbins = rhs.nPairs_nbins;  nPairs_max = rhs.nPairs_max;
    pt_nbins = rhs.pt_nbins;  pt_min = rhs.pt_min;  pt_max = rhs.pt_max;
    eta_nbins = rhs.eta_nbins;  eta_min = rhs.eta_min;  eta_max = rhs.eta_max;
    dphi_nbins = rhs.dphi_nbins;
    acop_nbins = rhs.acop_nbins;  acop_max = rhs.acop_max;
    deta_nbins = rhs.deta_nbins;  deta_max = rhs.deta_max;
    xJ_nbins = rhs.xJ_nbins;
    mass_nbins = rhs.mass_nbins;  mass_max = rhs.mass_max;
    pairPt_nbins = rhs.pairPt_nbins;  pairPt_max = rhs.pairPt_max;
    y_nbins = rhs.y_nbins;  y_max = rhs.y_max;
    _pairCount = rhs._pairCount;
    cloneB(rhs);
    }
  return *this;
}

void JetPairHistos::cloneB(const JetPairHistos & source)
{
  h_nPairs = histogramLike(source.h_nPairs);
  h_pt1 = histogramLike(source.h_pt1);  h_pt2 = histogramLike(source.h_pt2);  h_pt1pt2 = histogramLike(source.h_pt1pt2);
  h_eta1 = histogramLike(source.h_eta1);  h_eta2 = histogramLike(source.h_eta2);
  h_dphi = histogramLike(source.h_dphi);  h_acop = histogramLike(source.h_acop);  h_deta = histogramLike(source.h_deta);
  h_xJ = histogramLike(source.h_xJ);  h_aJ = histogramLike(source.h_aJ);  h_mass = histogramLike(source.h_mass);
  h_pairPt = histogramLike(source.h_pairPt);  h_y = histogramLike(source.h_y);  h_dphiXJ = histogramLike(source.h_dphiXJ);
  p_xJ = histogramLike(source.p_xJ);  p_aJ = histogramLike(source.p_aJ);  p_acop = histogramLike(source.p_acop);
}

void JetPairHistos::addDefaultConfiguration(Configuration & configuration, const String & taskName)
{
  auto add = [&](const char * name, auto value) { configuration.addProperty(createKey(taskName,"HISTOGRAM",name),value); };
  add("nPairs_nbins",100);  add("nPairs_max",100.0);
  add("pt_nbins",100);      add("pt_min",0.0);      add("pt_max",100.0);
  add("eta_nbins",20);      add("eta_min",-1.0);    add("eta_max",1.0);
  add("dphi_nbins",72);
  add("acop_nbins",60);     add("acop_max",M_PI);
  add("deta_nbins",40);     add("deta_max",2.0);
  add("xJ_nbins",50);
  add("mass_nbins",100);    add("mass_max",200.0);
  add("pairPt_nbins",50);   add("pairPt_max",50.0);
  add("y_nbins",20);        add("y_max",1.0);
}

void JetPairHistos::configure(const String & taskName, const String & objectType, const Configuration & configuration,
                              unsigned int index __attribute__((unused)))
{
  String type = objectType.beginsWith("HISTOGRAM") ? String("HISTOGRAM") : objectType;
  auto i = [&](const char * name) { return configuration.valueInt(createKey(taskName,type,name)); };
  auto d = [&](const char * name) { return configuration.valueDouble(createKey(taskName,type,name)); };
  nPairs_nbins = i("nPairs_nbins");  nPairs_max = d("nPairs_max");
  pt_nbins = i("pt_nbins");  pt_min = d("pt_min");  pt_max = d("pt_max");
  eta_nbins = i("eta_nbins");  eta_min = d("eta_min");  eta_max = d("eta_max");
  dphi_nbins = i("dphi_nbins");
  acop_nbins = i("acop_nbins");  acop_max = d("acop_max");
  deta_nbins = i("deta_nbins");  deta_max = d("deta_max");
  xJ_nbins = i("xJ_nbins");
  mass_nbins = i("mass_nbins");  mass_max = d("mass_max");
  pairPt_nbins = i("pairPt_nbins");  pairPt_max = d("pairPt_max");
  y_nbins = i("y_nbins");  y_max = d("y_max");
}

void JetPairHistos::create()
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  auto name = [this](const char * n) { return createName(_histogramBaseName,n); };
  const double low = -0.5*M_PI, high = 1.5*M_PI, xMax = 1.0+1.0E-9;
  h_nPairs = createHistogram(name("nPairs"),nPairs_nbins,0.0,nPairs_max,"n_{pairs}","Events");
  h_pt1    = createHistogram(name("pt1"),pt_nbins,pt_min,pt_max,"p_{T,1} (GeV/c)","Pairs");
  h_pt2    = createHistogram(name("pt2"),pt_nbins,pt_min,pt_max,"p_{T,2} (GeV/c)","Pairs");
  h_pt1pt2 = createHistogram(name("pt1pt2"),pt_nbins,pt_min,pt_max,pt_nbins,pt_min,pt_max,"p_{T,1} (GeV/c)","p_{T,2} (GeV/c)","Pairs");
  h_eta1   = createHistogram(name("eta1"),eta_nbins,eta_min,eta_max,"#eta_{1}","Pairs");
  h_eta2   = createHistogram(name("eta2"),eta_nbins,eta_min,eta_max,"#eta_{2}","Pairs");
  h_dphi   = createHistogram(name("dphi"),dphi_nbins,low,high,"#Delta#varphi","Pairs");
  h_acop   = createHistogram(name("acoplanarity"),acop_nbins,0.0,acop_max,"#pi - |#Delta#varphi|","Pairs");
  h_deta   = createHistogram(name("deta"),deta_nbins,-deta_max,deta_max,"#Delta#eta","Pairs");
  h_xJ     = createHistogram(name("xJ"),xJ_nbins,0.0,xMax,"x_{J} = p_{T,2}/p_{T,1}","Pairs");
  h_aJ     = createHistogram(name("aJ"),xJ_nbins,0.0,xMax,"A_{J}","Pairs");
  h_mass   = createHistogram(name("mass"),mass_nbins,0.0,mass_max,"m_{12} (GeV/c^{2})","Pairs");
  h_pairPt = createHistogram(name("pairPt"),pairPt_nbins,0.0,pairPt_max,"p_{T,12} (GeV/c)","Pairs");
  h_y      = createHistogram(name("pairY"),y_nbins,-y_max,y_max,"y_{12}","Pairs");
  h_dphiXJ = createHistogram(name("dphiXJ"),dphi_nbins,low,high,xJ_nbins,0.0,xMax,"#Delta#varphi","x_{J}","Pairs");
  p_xJ     = createProfile(name("xJ_vsPt1"),pt_nbins,pt_min,pt_max,"p_{T,1} (GeV/c)","<x_{J}>");
  p_aJ     = createProfile(name("aJ_vsPt1"),pt_nbins,pt_min,pt_max,"p_{T,1} (GeV/c)","<A_{J}>");
  p_acop   = createProfile(name("acoplanarity_vsPt1"),pt_nbins,pt_min,pt_max,"p_{T,1} (GeV/c)","<#pi - |#Delta#varphi|>");
}

void JetPairHistos::loadFrom(const Folder & inputFile)
{
  auto h = [&](const char * n) { return importHistogram(inputFile,createName(_histogramBaseName,n)); };
  auto p = [&](const char * n) { return importProfile(inputFile,createName(_histogramBaseName,n)); };
  h_nPairs = h("nPairs");
  h_pt1 = h("pt1");  h_pt2 = h("pt2");  h_pt1pt2 = h("pt1pt2");  h_eta1 = h("eta1");  h_eta2 = h("eta2");
  h_dphi = h("dphi");  h_acop = h("acoplanarity");  h_deta = h("deta");  h_xJ = h("xJ");  h_aJ = h("aJ");
  h_mass = h("mass");  h_pairPt = h("pairPt");  h_y = h("pairY");  h_dphiXJ = h("dphiXJ");
  p_xJ = p("xJ_vsPt1");  p_aJ = p("aJ_vsPt1");  p_acop = p("acoplanarity_vsPt1");
}

void JetPairHistos::fill(const std::vector<JetPair> & pairs)
{
  h_nPairs->fill(double(pairs.size()));
  _pairCount += double(pairs.size());
  for (const JetPair & pair : pairs)
    {
    const double pt1 = pair.first->pt(), pt2 = pair.second->pt();
    const double dphi = pair.deltaPhi(), acop = pair.acoplanarity(), xJ = pair.ptRatio(), aJ = pair.asymmetry();
    const MATH::VectorLorentz p = pair.momentum();
    h_pt1->fill(pt1);
    h_pt2->fill(pt2);
    h_pt1pt2->fill(pt1,pt2,1.0);
    h_eta1->fill(pair.first->eta());
    h_eta2->fill(pair.second->eta());
    h_dphi->fill(dphi);
    h_acop->fill(acop);
    h_deta->fill(pair.deltaEta());
    h_xJ->fill(xJ);
    h_aJ->fill(aJ);
    h_mass->fill(pair.mass());
    h_pairPt->fill(p.perp());
    h_y->fill(p.rapidity());
    h_dphiXJ->fill(dphi,xJ,1.0);
    p_xJ->fill(pt1,xJ);
    p_aJ->fill(pt1,aJ);
    p_acop->fill(pt1,acop);
    }
}

std::vector<HistogramFolder::Histogram*> JetPairHistos::pairHistograms() const
{
  return {h_pt1, h_pt2, h_pt1pt2, h_eta1, h_eta2, h_dphi, h_acop, h_deta, h_xJ, h_aJ, h_mass, h_pairPt, h_y, h_dphiXJ};
}

void JetPairHistos::scaleObject(double factor)
{
  if (h_nPairs) h_nPairs->scale(factor);
}

void JetPairHistos::normalize(double eventFactor)
{
  scaleObject(eventFactor);
  if (!(_pairCount>0.0)) return;
  for (Histogram * h : pairHistograms()) if (h) h->scale(1.0/_pairCount);
}

} // namespace CAP
