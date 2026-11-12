/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "JetFluctuationHistos.hpp"
#include <cmath>
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"

namespace CAP
{

namespace
{
const char * kMomentNames[] = {"n", "nPos", "nNeg", "n0", "q", "q2", "f2", "f2Pos", "f2Neg", "f20", "f2PosNeg"};
const char * kMomentTitles[] = {"<N>", "<N_{+}>", "<N_{-}>", "<N_{0}>", "<Q>", "<Q^{2}>", "<N(N-1)>", "<N_{+}(N_{+}-1)>",
                                "<N_{-}(N_{-}-1)>", "<N_{0}(N_{0}-1)>", "<N_{+}N_{-}>"};
const char * kDerivedNames[] = {"var_n", "var_nPos", "var_nNeg", "var_n0", "var_q", "R2_PP", "R2_MM", "R2_PM", "nudyn",
                                "sub_n", "sub_nPos", "sub_nNeg", "sub_n0", "sub_f2", "sub_f2Pos", "sub_f2Neg", "sub_f20",
                                "sub_f2PosNeg", "sub_var_n", "sub_var_nPos", "sub_var_nNeg", "sub_var_n0", "sub_R2_PP",
                                "sub_R2_MM", "sub_R2_PM", "sub_nudyn"};
const char * kAxisSuffix[] = {"", "_vsPt", "_vsMult"};
const char * kAxisTitle[] = {"", "p_{T,jet} (GeV/c)", "N"};
} // namespace

const char * JetFluctuationHistos::momentName(unsigned int m)  { return kMomentNames[m]; }
const char * JetFluctuationHistos::derivedName(unsigned int d) { return kDerivedNames[d]; }

JetFluctuationHistos::JetFluctuationHistos()
:
HistogramFolder(),
pt_nbins(100), pt_min(0.0), pt_max(100.0),
mult_nbins(50), mult_max(50.0),
nCones_nbins(5), nCones_max(5.0),
h_nCones(nullptr),
_jet(kAxes*kMoments,nullptr),
_ue(kAxes*kMoments,nullptr),
_derived(kAxes*kDerived,nullptr)
{
  appendClassName("JetFluctuationHistos");
  setName("JetFluctuationHistos");
  setTitle("JetFluctuationHistos");
  setHistogramBaseName("JetFluct");
}

JetFluctuationHistos::JetFluctuationHistos(const JetFluctuationHistos & source)
:
HistogramFolder(source),
_jet(kAxes*kMoments,nullptr),
_ue(kAxes*kMoments,nullptr),
_derived(kAxes*kDerived,nullptr)
{
  *this = source;
}

JetFluctuationHistos & JetFluctuationHistos::operator=(const JetFluctuationHistos & rhs)
{
  if (this!=&rhs)
    {
    HistogramFolder::operator=(rhs);
    pt_nbins = rhs.pt_nbins;  pt_min = rhs.pt_min;  pt_max = rhs.pt_max;
    mult_nbins = rhs.mult_nbins;  mult_max = rhs.mult_max;
    nCones_nbins = rhs.nCones_nbins;  nCones_max = rhs.nCones_max;
    cloneB(rhs);
    }
  return *this;
}

void JetFluctuationHistos::cloneB(const JetFluctuationHistos & source)
{
  h_nCones = histogramLike(source.h_nCones);
  for (unsigned int k=0; k<_jet.size(); k++) _jet[k] = histogramLike(source._jet[k]);
  for (unsigned int k=0; k<_ue.size(); k++)  _ue[k]  = histogramLike(source._ue[k]);
  for (unsigned int k=0; k<_derived.size(); k++) _derived[k] = histogramLike(source._derived[k]);
}

void JetFluctuationHistos::addDefaultConfiguration(Configuration & configuration, const String & taskName)
{
  auto add = [&](const char * name, auto value) { configuration.addProperty(createKey(taskName,"HISTOGRAM",name),value); };
  add("pt_nbins",100);    add("pt_min",0.0);   add("pt_max",100.0);
  add("mult_nbins",50);   add("mult_max",50.0);
  add("nCones_nbins",5);  add("nCones_max",5.0);
}

void JetFluctuationHistos::configure(const String & taskName, const String & objectType, const Configuration & configuration,
                                     unsigned int index __attribute__((unused)))
{
  String type = objectType.beginsWith("HISTOGRAM") ? String("HISTOGRAM") : objectType;
  auto i = [&](const char * name) { return configuration.valueInt(createKey(taskName,type,name)); };
  auto d = [&](const char * name) { return configuration.valueDouble(createKey(taskName,type,name)); };
  pt_nbins = i("pt_nbins");  pt_min = d("pt_min");  pt_max = d("pt_max");
  mult_nbins = i("mult_nbins");  mult_max = d("mult_max");
  nCones_nbins = i("nCones_nbins");  nCones_max = d("nCones_max");
}

String JetFluctuationHistos::histogramName(const String & name, unsigned int a) const
{
  return createName(_histogramBaseName,name+kAxisSuffix[a]);
}

void JetFluctuationHistos::create()
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  h_nCones = createHistogram(createName(_histogramBaseName,"nCones"),nCones_nbins,0.0,nCones_max,"n_{cones}","Events");
  for (unsigned int a=0; a<kAxes; a++)
    {
    const int n = a==kIntegrated ? 1 : a==kVsPt ? pt_nbins : mult_nbins;
    const double low = a==kVsPt ? pt_min : 0.0, high = a==kIntegrated ? 1.0 : a==kVsPt ? pt_max : mult_max;
    for (unsigned int m=0; m<kMoments; m++)
      {
      _jet[a*kMoments+m] = createProfile(histogramName(kMomentNames[m],a),n,low,high,kAxisTitle[a],kMomentTitles[m]);
      if (coneMoment(m))
        _ue[a*kMoments+m] = createProfile(histogramName(String("ue_")+kMomentNames[m],a),n,low,high,kAxisTitle[a],
                                          kMomentTitles[m]);
      }
    }
}

void JetFluctuationHistos::createDerived()
{
  for (unsigned int a=0; a<kAxes; a++)
    {
    const int n = a==kIntegrated ? 1 : a==kVsPt ? pt_nbins : mult_nbins;
    const double low = a==kVsPt ? pt_min : 0.0, high = a==kIntegrated ? 1.0 : a==kVsPt ? pt_max : mult_max;
    for (unsigned int d=0; d<kDerived; d++)
      _derived[a*kDerived+d] = createHistogram(histogramName(kDerivedNames[d],a),n,low,high,kAxisTitle[a],kDerivedNames[d]);
    }
}

void JetFluctuationHistos::loadFrom(const Folder & inputFile)
{
  h_nCones = importHistogram(inputFile,createName(_histogramBaseName,"nCones"));
  for (unsigned int a=0; a<kAxes; a++)
    for (unsigned int m=0; m<kMoments; m++)
      {
      _jet[a*kMoments+m] = importProfile(inputFile,histogramName(kMomentNames[m],a));
      if (coneMoment(m)) _ue[a*kMoments+m] = importProfile(inputFile,histogramName(String("ue_")+kMomentNames[m],a));
      }
  HistogramList list;
  for (unsigned int a=0; a<kAxes; a++)
    for (unsigned int d=0; d<kDerived; d++) list.emplace_back(&_derived[a*kDerived+d],histogramName(kDerivedNames[d],a));
  importAllOrNone(inputFile,list);
}

namespace
{
//! Values of the base quantities of a jet or cone.
void momentsOf(const Multiplicities & m, double * v)
{
  using H = JetFluctuationHistos;
  v[H::kN] = m.n;  v[H::kNPos] = m.nPos;  v[H::kNNeg] = m.nNeg;  v[H::kN0] = m.n0;
  v[H::kQ] = m.nPos-m.nNeg;  v[H::kQ2] = v[H::kQ]*v[H::kQ];
  v[H::kF2] = m.n*(m.n-1.0);  v[H::kF2Pos] = m.nPos*(m.nPos-1.0);  v[H::kF2Neg] = m.nNeg*(m.nNeg-1.0);
  v[H::kF20] = m.n0*(m.n0-1.0);  v[H::kF2PosNeg] = m.nPos*m.nNeg;
}
} // namespace

void JetFluctuationHistos::fill(const Jet & jet, const std::vector<Multiplicities> & cones)
{
  double v[kMoments], b[kMoments];
  const Multiplicities m = constituentMultiplicities(jet);
  momentsOf(m,v);
  const double x[kAxes] = {0.5, jet.pt(), m.n};
  for (unsigned int a=0; a<kAxes; a++)
    for (unsigned int k=0; k<kMoments; k++) _jet[a*kMoments+k]->fill(x[a],v[k]);
  for (const Multiplicities & cone : cones)
    {
    momentsOf(cone,b);
    for (unsigned int a=0; a<kAxes; a++)
      for (unsigned int k=0; k<kMoments; k++) if (coneMoment(k)) _ue[a*kMoments+k]->fill(x[a],b[k]);
    }
}

void JetFluctuationHistos::fillEvent(unsigned int nCones)
{
  h_nCones->fill(double(nCones));
}

void JetFluctuationHistos::scaleObject(double factor)
{
  if (h_nCones) h_nCones->scale(factor);
}

void JetFluctuationHistos::calculateDerived()
{
  if (!hasDerived()) createDerived();
  for (unsigned int a=0; a<kAxes; a++)
    {
    const unsigned int nBins = _jet[a*kMoments]->nBinsTotal();
    for (unsigned int bin=0; bin<nBins; bin++)
      {
      auto set = [&](unsigned int d, double value) { _derived[a*kDerived+d]->setBinValueAtGlobal(bin,value); };
      if (_jet[a*kMoments+kN]->binEntriesAtGlobal(bin)<=0) continue;
      double J[kMoments], B[kMoments];
      for (unsigned int k=0; k<kMoments; k++) J[k] = _jet[a*kMoments+k]->binValueAtGlobal(bin);
      const bool ue = _ue[a*kMoments+kN]->binEntriesAtGlobal(bin)>0;
      for (unsigned int k=0; k<kMoments; k++) B[k] = coneMoment(k) && ue ? _ue[a*kMoments+k]->binValueAtGlobal(bin) : 0.0;
      // R2 of positive and negative particles and nu_dyn, from averages; false if undefined
      auto r2 = [&](double f2PP, double f2MM, double f2PM, double nP, double nM, unsigned int dPP)
        {
        bool pp = nP>0.0, mm = nM>0.0;
        if (pp)       set(dPP,f2PP/(nP*nP)-1.0);
        if (mm)       set(dPP+1,f2MM/(nM*nM)-1.0);
        if (pp && mm) set(dPP+2,f2PM/(nP*nM)-1.0);
        if (pp && mm) set(dPP+3,f2PP/(nP*nP)+f2MM/(nM*nM)-2.0*f2PM/(nP*nM));
        };
      set(kVarN,   J[kF2]   +J[kN]   -J[kN]*J[kN]);
      set(kVarNPos,J[kF2Pos]+J[kNPos]-J[kNPos]*J[kNPos]);
      set(kVarNNeg,J[kF2Neg]+J[kNNeg]-J[kNNeg]*J[kNNeg]);
      set(kVarN0,  J[kF20]  +J[kN0]  -J[kN0]*J[kN0]);
      set(kVarQ,   J[kQ2]-J[kQ]*J[kQ]);
      r2(J[kF2Pos],J[kF2Neg],J[kF2PosNeg],J[kNPos],J[kNNeg],kR2PP);
      if (!ue) continue;
      const double s = J[kN]-B[kN], sPos = J[kNPos]-B[kNPos], sNeg = J[kNNeg]-B[kNNeg], s0 = J[kN0]-B[kN0];
      const double f2   = J[kF2]   -2.0*s*B[kN]      -B[kF2];
      const double f2P  = J[kF2Pos]-2.0*sPos*B[kNPos]-B[kF2Pos];
      const double f2M  = J[kF2Neg]-2.0*sNeg*B[kNNeg]-B[kF2Neg];
      const double f20  = J[kF20]  -2.0*s0*B[kN0]    -B[kF20];
      const double f2PM = J[kF2PosNeg]-sPos*B[kNNeg]-B[kNPos]*sNeg-B[kF2PosNeg];
      set(kSubN,s);  set(kSubNPos,sPos);  set(kSubNNeg,sNeg);  set(kSubN0,s0);
      set(kSubF2,f2);  set(kSubF2Pos,f2P);  set(kSubF2Neg,f2M);  set(kSubF20,f20);  set(kSubF2PosNeg,f2PM);
      set(kSubVarN,f2+s-s*s);  set(kSubVarNPos,f2P+sPos-sPos*sPos);  set(kSubVarNNeg,f2M+sNeg-sNeg*sNeg);
      set(kSubVarN0,f20+s0-s0*s0);
      r2(f2P,f2M,f2PM,sPos,sNeg,kSubR2PP);
      }
    }
}

} // namespace CAP
