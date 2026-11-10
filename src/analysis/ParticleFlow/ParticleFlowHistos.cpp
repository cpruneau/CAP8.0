/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "ParticleFlowHistos.hpp"
#include <algorithm>
#include <cmath>
#include <string>
#include "Exceptions.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "Particle.hpp"
#include "FlowCumulants.hpp"

namespace CAP
{

  namespace
  {
  String text(unsigned int k) { return String(std::to_string(k)); }
  } // namespace

  ParticleFlowHistos::ParticleFlowHistos()
  :
  HistogramFolder(),
  evtName("Mult0"),
  evt_nbins(1),
  evt_min(0.0),
  evt_max(1.0E6),
  pt_nbins(20),
  pt_min(0.0),
  pt_max(4.0),
  _nHarmonics(4),
  _maxOrder(8),
  _gap(true),
  _gapSize(1.0),
  _gapRapidity(false),
  _scalarProduct(true),
  _poiNames(),
  h_evt(nullptr)
  {
  appendClassName("ParticleFlowHistos");
  setName("ParticleFlowHistos");
  setTitle("ParticleFlowHistos");
  }

  ParticleFlowHistos::ParticleFlowHistos(const ParticleFlowHistos & source)
  :
  HistogramFolder(source),
  evtName(source.evtName),
  evt_nbins(source.evt_nbins),
  evt_min(source.evt_min),
  evt_max(source.evt_max),
  pt_nbins(source.pt_nbins),
  pt_min(source.pt_min),
  pt_max(source.pt_max),
  _nHarmonics(source._nHarmonics),
  _maxOrder(source._maxOrder),
  _gap(source._gap),
  _gapSize(source._gapSize),
  _gapRapidity(source._gapRapidity),
  _scalarProduct(source._scalarProduct),
  _poiNames(source._poiNames),
  h_evt(nullptr)
  {
  cloneB(source);
  }

  ParticleFlowHistos & ParticleFlowHistos::operator=(const ParticleFlowHistos & rhs)
  {
  if (this!=&rhs)
    {
    HistogramFolder::operator=(rhs);
    evtName        = rhs.evtName;
    evt_nbins      = rhs.evt_nbins;
    evt_min        = rhs.evt_min;
    evt_max        = rhs.evt_max;
    pt_nbins       = rhs.pt_nbins;
    pt_min         = rhs.pt_min;
    pt_max         = rhs.pt_max;
    _nHarmonics    = rhs._nHarmonics;
    _maxOrder      = rhs._maxOrder;
    _gap           = rhs._gap;
    _gapSize       = rhs._gapSize;
    _gapRapidity   = rhs._gapRapidity;
    _scalarProduct = rhs._scalarProduct;
    _poiNames      = rhs._poiNames;
    cloneB(rhs);
    }
  return *this;
  }

  void ParticleFlowHistos::cloneB(const ParticleFlowHistos & source)
  {
  auto like = [this](const auto & from, auto & to)
    {
    to.resize(from.size());
    for (unsigned int k=0; k<from.size(); k++) to[k] = histogramLike(from[k]);
    };
  h_evt = histogramLike(source.h_evt);
  like(source._corr,_corr);
  like(source._corrGap,_corrGap);
  like(source._spQaQb,_spQaQb);
  like(source._poiCorr,_poiCorr);
  like(source._poiCorrPt,_poiCorrPt);
  like(source._poiCorrGap,_poiCorrGap);
  like(source._poiCorrGapPt,_poiCorrGapPt);
  like(source._poiSP,_poiSP);
  like(source._poiSPPt,_poiSPPt);
  like(source._cum,_cum);
  like(source._v,_v);
  like(source._cumGap,_cumGap);
  like(source._vGap,_vGap);
  like(source._dcum,_dcum);
  like(source._vp,_vp);
  like(source._dcumPt,_dcumPt);
  like(source._vpPt,_vpPt);
  like(source._dcumGap,_dcumGap);
  like(source._vpGap,_vpGap);
  like(source._dcumGapPt,_dcumGapPt);
  like(source._vpGapPt,_vpGapPt);
  like(source._vSP,_vSP);
  like(source._vSPPt,_vSPPt);
  }

  void ParticleFlowHistos::addDefaultConfiguration(Configuration & configuration, const String & taskName)
  {
  const String type = "HISTOGRAM";
  configuration.addProperty(createKey(taskName,type,"evtName"),       String("Mult0"));
  configuration.addProperty(createKey(taskName,type,"evt_nbins"),     1);
  configuration.addProperty(createKey(taskName,type,"evt_min"),       0.0);
  configuration.addProperty(createKey(taskName,type,"evt_max"),       1.0E6);
  configuration.addProperty(createKey(taskName,type,"nHarmonics"),    4);
  configuration.addProperty(createKey(taskName,type,"maxOrder"),      8);
  configuration.addProperty(createKey(taskName,type,"pt_nbins"),      20);
  configuration.addProperty(createKey(taskName,type,"pt_min"),        0.0);
  configuration.addProperty(createKey(taskName,type,"pt_max"),        4.0);
  configuration.addProperty(createKey(taskName,type,"gap"),           true);
  configuration.addProperty(createKey(taskName,type,"gapSize"),       1.0);
  configuration.addProperty(createKey(taskName,type,"gapVariable"),   String("ETA"));
  configuration.addProperty(createKey(taskName,type,"scalarProduct"), true);
  }

  void ParticleFlowHistos::configure(const String & taskName, const String & type, const Configuration & configuration,
                                     unsigned int index __attribute__((unused)))
  {
  String objectType = type.beginsWith("HISTOGRAM") ? String("HISTOGRAM") : type;
  auto key = [&](const char * k) { return createKey(taskName,objectType,k); };
  evtName        = configuration.valueString(key("evtName"));
  evt_nbins      = configuration.valueInt(   key("evt_nbins"));
  evt_min        = configuration.valueDouble(key("evt_min"));
  evt_max        = configuration.valueDouble(key("evt_max"));
  int nh         = configuration.valueInt(   key("nHarmonics"));
  int mo         = configuration.valueInt(   key("maxOrder"));
  pt_nbins       = configuration.valueInt(   key("pt_nbins"));
  pt_min         = configuration.valueDouble(key("pt_min"));
  pt_max         = configuration.valueDouble(key("pt_max"));
  _gap           = configuration.valueBool(  key("gap"));
  _gapSize       = configuration.valueDouble(key("gapSize"));
  String variable = configuration.valueString(key("gapVariable"));
  _scalarProduct = configuration.valueBool(  key("scalarProduct"));
  variable.toUpper();
  if (nh<1)                         throw ConfigurationException(key("nHarmonics"),"must be 1 or more",__FUNCTION__);
  if (mo<2 || mo%2!=0 || mo>16)     throw ConfigurationException(key("maxOrder"),"must be even, from 2 to 16",__FUNCTION__);
  if (evt_nbins<1 || !(evt_max>evt_min)) throw ConfigurationException(key("evt_nbins"),"invalid event class axis",__FUNCTION__);
  if (pt_nbins<1 || !(pt_max>pt_min))    throw ConfigurationException(key("pt_nbins"),"invalid pT axis",__FUNCTION__);
  if (_gapSize<0.0)                 throw ConfigurationException(key("gapSize"),"must not be negative",__FUNCTION__);
  if (variable=="ETA")    _gapRapidity = false;
  else if (variable=="Y" || variable=="RAPIDITY") _gapRapidity = true;
  else throw ConfigurationException(key("gapVariable"),"must be ETA or Y",__FUNCTION__);
  _nHarmonics = nh;
  _maxOrder   = mo;
  if (!_gap) _scalarProduct = false;          // the scalar product uses the sub-events of the gap
  }

  String ParticleFlowHistos::integralName(const char * what, unsigned int order, unsigned int n) const
  {
  return createName(_histogramBaseName,String(what)+text(order)+"_n"+text(n));
  }

  String ParticleFlowHistos::poiName(unsigned int poi, const char * what, unsigned int order, unsigned int n, bool pt) const
  {
  String name = String("POI")+text(poi)+"_"+what+(order>0 ? text(order) : String(""))+"_n"+text(n);
  if (pt) name += "_pt";
  return createName(_histogramBaseName,name);
  }

  void ParticleFlowHistos::create()
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  const unsigned int N = _nHarmonics, K = nOrders(), J = nPoi();
  h_evt = createHistogram(createName(_histogramBaseName,"evt"),evt_nbins,evt_min,evt_max,evtName,"Counts");
  _corr.assign(N*K,nullptr);
  _corrGap.assign(_gap ? N*K : 0,nullptr);
  _spQaQb.assign(_scalarProduct ? N : 0,nullptr);
  _poiCorr.assign(J*N*K,nullptr);
  _poiCorrPt.assign(J*N*K,nullptr);
  _poiCorrGap.assign(_gap ? J*N*K : 0,nullptr);
  _poiCorrGapPt.assign(_gap ? J*N*K : 0,nullptr);
  _poiSP.assign(_scalarProduct ? J*N : 0,nullptr);
  _poiSPPt.assign(_scalarProduct ? J*N : 0,nullptr);
  for (unsigned int n=1; n<=N; n++)
    {
    for (unsigned int k=1; k<=K; k++)
      {
      String t = String("<<")+text(2*k)+">>_{"+text(n)+"}";
      _corr[at(n,k)] = createProfile(integralName("Corr",2*k,n),evt_nbins,evt_min,evt_max,evtName,t);
      if (_gap) _corrGap[at(n,k)] = createProfile(integralName("CorrGap",2*k,n),evt_nbins,evt_min,evt_max,evtName,
                                                  String("<<")+text(2*k)+">>_{"+text(n)+",gap}");
      for (unsigned int j=0; j<J; j++)
        {
        String tp = String("<<")+text(2*k)+"'>>_{"+text(n)+"} "+_poiNames[j];
        _poiCorr[at(j,n,k)]   = createProfile(poiName(j,"Corr",2*k,n,false),evt_nbins,evt_min,evt_max,evtName,tp);
        _poiCorrPt[at(j,n,k)] = createProfile(poiName(j,"Corr",2*k,n,true),evt_nbins,evt_min,evt_max,
                                              pt_nbins,pt_min,pt_max,evtName,"p_{T}",tp);
        if (_gap)
          {
          String tg = String("<<")+text(2*k)+"'>>_{"+text(n)+",gap} "+_poiNames[j];
          _poiCorrGap[at(j,n,k)]   = createProfile(poiName(j,"CorrGap",2*k,n,false),evt_nbins,evt_min,evt_max,evtName,tg);
          _poiCorrGapPt[at(j,n,k)] = createProfile(poiName(j,"CorrGap",2*k,n,true),evt_nbins,evt_min,evt_max,
                                                   pt_nbins,pt_min,pt_max,evtName,"p_{T}",tg);
          }
        }
      }
    if (_scalarProduct)
      {
      _spQaQb[n-1] = createProfile(createName(_histogramBaseName,String("SPQaQb_n")+text(n)),evt_nbins,evt_min,evt_max,
                                   evtName,String("<Q_{A}Q_{B}^{*}>_{")+text(n)+"}");
      for (unsigned int j=0; j<J; j++)
        {
        String t = String("<uQ^{*}>_{")+text(n)+"} "+_poiNames[j];
        _poiSP[atSP(j,n)]   = createProfile(poiName(j,"SPuQ",0,n,false),evt_nbins,evt_min,evt_max,evtName,t);
        _poiSPPt[atSP(j,n)] = createProfile(poiName(j,"SPuQ",0,n,true),evt_nbins,evt_min,evt_max,
                                            pt_nbins,pt_min,pt_max,evtName,"p_{T}",t);
        }
      }
    }
  prepareWork();
  }

  void ParticleFlowHistos::loadFrom(const Folder & inputFile)
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  auto has = [&](const String & name) { return inputFile.get<Profile>(name)!=nullptr; };
  // sizes: from the histograms in the file
  _maxOrder = 2;
  _nHarmonics = 1;
  if (!has(integralName("Corr",2,1)))
    throw FileException(inputFile.path(),String("No profile named ")+integralName("Corr",2,1)+" in the file",__FUNCTION__);
  while (has(integralName("Corr",2,_nHarmonics+1))) _nHarmonics++;
  while (has(integralName("Corr",_maxOrder+2,1)))   _maxOrder += 2;
  _gap = has(integralName("CorrGap",2,1));
  _scalarProduct = has(createName(_histogramBaseName,"SPQaQb_n1"));
  unsigned int J = 0;
  while (has(poiName(J,"Corr",2,1,false))) J++;
  _poiNames.resize(J);
  for (unsigned int j=0; j<J; j++) _poiNames[j] = String("POI")+text(j);
  const unsigned int N = _nHarmonics, K = nOrders();
  h_evt = importHistogram(inputFile,createName(_histogramBaseName,"evt"));
  _corr.assign(N*K,nullptr);
  _corrGap.assign(_gap ? N*K : 0,nullptr);
  _spQaQb.assign(_scalarProduct ? N : 0,nullptr);
  _poiCorr.assign(J*N*K,nullptr);
  _poiCorrPt.assign(J*N*K,nullptr);
  _poiCorrGap.assign(_gap ? J*N*K : 0,nullptr);
  _poiCorrGapPt.assign(_gap ? J*N*K : 0,nullptr);
  _poiSP.assign(_scalarProduct ? J*N : 0,nullptr);
  _poiSPPt.assign(_scalarProduct ? J*N : 0,nullptr);
  for (unsigned int n=1; n<=N; n++)
    {
    for (unsigned int k=1; k<=K; k++)
      {
      _corr[at(n,k)] = importProfile(inputFile,integralName("Corr",2*k,n));
      if (_gap) _corrGap[at(n,k)] = importProfile(inputFile,integralName("CorrGap",2*k,n));
      for (unsigned int j=0; j<J; j++)
        {
        _poiCorr[at(j,n,k)]   = importProfile(inputFile,poiName(j,"Corr",2*k,n,false));
        _poiCorrPt[at(j,n,k)] = importProfile(inputFile,poiName(j,"Corr",2*k,n,true));
        if (_gap)
          {
          _poiCorrGap[at(j,n,k)]   = importProfile(inputFile,poiName(j,"CorrGap",2*k,n,false));
          _poiCorrGapPt[at(j,n,k)] = importProfile(inputFile,poiName(j,"CorrGap",2*k,n,true));
          }
        }
      }
    if (_scalarProduct)
      {
      _spQaQb[n-1] = importProfile(inputFile,createName(_histogramBaseName,String("SPQaQb_n")+text(n)));
      for (unsigned int j=0; j<J; j++)
        {
        _poiSP[atSP(j,n)]   = importProfile(inputFile,poiName(j,"SPuQ",0,n,false));
        _poiSPPt[atSP(j,n)] = importProfile(inputFile,poiName(j,"SPuQ",0,n,true));
        }
      }
    }
  // binning: from the histograms
  evt_nbins = h_evt->nBins(0);
  evt_min   = h_evt->minimum(0);
  evt_max   = h_evt->maximum(0);
  if (J>0)
    {
    pt_nbins = _poiCorrPt[0]->nBins(1);
    pt_min   = _poiCorrPt[0]->minimum(1);
    pt_max   = _poiCorrPt[0]->maximum(1);
    }
  loadDerivedFrom(inputFile);
  }

  void ParticleFlowHistos::scaleObject(double factor)
  {
  if (h_evt) h_evt->scale(factor);
  }

  void ParticleFlowHistos::createDerived()
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  for (HistogramList::value_type & entry : derivedHistograms()) *entry.first = nullptr;
  const unsigned int N = _nHarmonics, K = nOrders(), J = nPoi();
  auto h1 = [&](const String & name, const String & title)
    { return createHistogram(name,evt_nbins,evt_min,evt_max,evtName,title); };
  auto h2 = [&](const String & name, const String & title)
    { return createHistogram(name,evt_nbins,evt_min,evt_max,pt_nbins,pt_min,pt_max,evtName,"p_{T}",title); };
  _cum.assign(N*K,nullptr);
  _v.assign(N*K,nullptr);
  _cumGap.assign(_gap ? N*K : 0,nullptr);
  _vGap.assign(_gap ? N*K : 0,nullptr);
  _dcum.assign(J*N*K,nullptr);
  _vp.assign(J*N*K,nullptr);
  _dcumPt.assign(J*N*K,nullptr);
  _vpPt.assign(J*N*K,nullptr);
  _dcumGap.assign(_gap ? J*N*K : 0,nullptr);
  _vpGap.assign(_gap ? J*N*K : 0,nullptr);
  _dcumGapPt.assign(_gap ? J*N*K : 0,nullptr);
  _vpGapPt.assign(_gap ? J*N*K : 0,nullptr);
  _vSP.assign(_scalarProduct ? J*N : 0,nullptr);
  _vSPPt.assign(_scalarProduct ? J*N : 0,nullptr);
  for (unsigned int n=1; n<=N; n++)
    {
    for (unsigned int k=1; k<=K; k++)
      {
      String o = text(2*k), h = text(n);
      _cum[at(n,k)] = h1(integralName("Cum",2*k,n),String("c_{")+h+"}{"+o+"}");
      _v[at(n,k)]   = h1(integralName("V",2*k,n),  String("v_{")+h+"}{"+o+"}");
      if (_gap)
        {
        _cumGap[at(n,k)] = h1(integralName("CumGap",2*k,n),String("c_{")+h+"}{"+o+",gap}");
        _vGap[at(n,k)]   = h1(integralName("VGap",2*k,n),  String("v_{")+h+"}{"+o+",gap}");
        }
      for (unsigned int j=0; j<J; j++)
        {
        String p = String(" ")+_poiNames[j];
        _dcum[at(j,n,k)]   = h1(poiName(j,"DCum",2*k,n,false),String("d_{")+h+"}{"+o+"}"+p);
        _vp[at(j,n,k)]     = h1(poiName(j,"V",2*k,n,false),   String("v'_{")+h+"}{"+o+"}"+p);
        _dcumPt[at(j,n,k)] = h2(poiName(j,"DCum",2*k,n,true), String("d_{")+h+"}{"+o+"}"+p);
        _vpPt[at(j,n,k)]   = h2(poiName(j,"V",2*k,n,true),    String("v'_{")+h+"}{"+o+"}"+p);
        if (_gap)
          {
          _dcumGap[at(j,n,k)]   = h1(poiName(j,"DCumGap",2*k,n,false),String("d_{")+h+"}{"+o+",gap}"+p);
          _vpGap[at(j,n,k)]     = h1(poiName(j,"VGap",2*k,n,false),   String("v'_{")+h+"}{"+o+",gap}"+p);
          _dcumGapPt[at(j,n,k)] = h2(poiName(j,"DCumGap",2*k,n,true), String("d_{")+h+"}{"+o+",gap}"+p);
          _vpGapPt[at(j,n,k)]   = h2(poiName(j,"VGap",2*k,n,true),    String("v'_{")+h+"}{"+o+",gap}"+p);
          }
        }
      }
    if (_scalarProduct)
      for (unsigned int j=0; j<J; j++)
        {
        String t = String("v_{")+text(n)+"}{SP} "+_poiNames[j];
        _vSP[atSP(j,n)]   = h1(poiName(j,"VSP",0,n,false),t);
        _vSPPt[atSP(j,n)] = h2(poiName(j,"VSP",0,n,true),t);
        }
    }
  }

  HistogramFolder::HistogramList ParticleFlowHistos::derivedHistograms()
  {
  const unsigned int N = _nHarmonics, K = nOrders(), J = nPoi();
  auto fit = [](std::vector<Histogram*> & v, unsigned int size) { if (v.size()!=size) v.assign(size,nullptr); };
  fit(_cum,N*K); fit(_v,N*K);
  fit(_cumGap,_gap ? N*K : 0); fit(_vGap,_gap ? N*K : 0);
  fit(_dcum,J*N*K); fit(_vp,J*N*K); fit(_dcumPt,J*N*K); fit(_vpPt,J*N*K);
  fit(_dcumGap,_gap ? J*N*K : 0); fit(_vpGap,_gap ? J*N*K : 0);
  fit(_dcumGapPt,_gap ? J*N*K : 0); fit(_vpGapPt,_gap ? J*N*K : 0);
  fit(_vSP,_scalarProduct ? J*N : 0); fit(_vSPPt,_scalarProduct ? J*N : 0);
  HistogramList list;
  for (unsigned int n=1; n<=N; n++)
    {
    for (unsigned int k=1; k<=K; k++)
      {
      list.push_back({&_cum[at(n,k)],integralName("Cum",2*k,n)});
      list.push_back({&_v[at(n,k)],  integralName("V",2*k,n)});
      if (_gap)
        {
        list.push_back({&_cumGap[at(n,k)],integralName("CumGap",2*k,n)});
        list.push_back({&_vGap[at(n,k)],  integralName("VGap",2*k,n)});
        }
      for (unsigned int j=0; j<J; j++)
        {
        list.push_back({&_dcum[at(j,n,k)],  poiName(j,"DCum",2*k,n,false)});
        list.push_back({&_vp[at(j,n,k)],    poiName(j,"V",2*k,n,false)});
        list.push_back({&_dcumPt[at(j,n,k)],poiName(j,"DCum",2*k,n,true)});
        list.push_back({&_vpPt[at(j,n,k)],  poiName(j,"V",2*k,n,true)});
        if (_gap)
          {
          list.push_back({&_dcumGap[at(j,n,k)],  poiName(j,"DCumGap",2*k,n,false)});
          list.push_back({&_vpGap[at(j,n,k)],    poiName(j,"VGap",2*k,n,false)});
          list.push_back({&_dcumGapPt[at(j,n,k)],poiName(j,"DCumGap",2*k,n,true)});
          list.push_back({&_vpGapPt[at(j,n,k)],  poiName(j,"VGap",2*k,n,true)});
          }
        }
      }
    if (_scalarProduct)
      for (unsigned int j=0; j<J; j++)
        {
        list.push_back({&_vSP[atSP(j,n)],  poiName(j,"VSP",0,n,false)});
        list.push_back({&_vSPPt[atSP(j,n)],poiName(j,"VSP",0,n,true)});
        }
    }
  return list;
  }

  void ParticleFlowHistos::loadDerivedFrom(const Folder & inputFile)
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  importAllOrNone(inputFile,derivedHistograms());
  }

  namespace
  {
  //! Mean and uncertainty of a profile bin; false if the bin is empty.
  bool meanOf(const Profile * p, unsigned int bin, double & mean, double & error)
  {
  if (!(p->sumWeights(bin)>0.0)) { mean = 0.0; error = 0.0; return false; }
  mean  = p->binValueAtGlobal(bin);
  error = p->binUncertaintyAtGlobal(bin);
  return true;
  }

  void setBin(Histogram * h, unsigned int bin, bool defined, double value, double error)
  {
  if (defined && std::isfinite(value) && std::isfinite(error)) h->setBinAtGlobal(bin,1,value,error);
  else                                                          h->setBinAtGlobal(bin,0,0.0,0.0);
  }
  } // namespace

  void ParticleFlowHistos::calculateDerived()
  {
  if (!hasDerived()) throw Exception("The derived histograms were not created (createDerived())",__FUNCTION__);
  if (reportDebug(__FUNCTION__)) { printCR(); }
  const unsigned int N = _nHarmonics, K = nOrders(), J = nPoi();
  const unsigned int nEvt = h_evt->nBins(0);
  const unsigned int nPt  = J>0 ? _poiCorrPt[0]->nBins(1) : 0;
  for (bool gapped : {false, true})
    {
    if (gapped && !_gap) continue;
    const std::vector<Profile*> & corr    = gapped ? _corrGap : _corr;
    const std::vector<Profile*> & poi     = gapped ? _poiCorrGap : _poiCorr;
    const std::vector<Profile*> & poiPt   = gapped ? _poiCorrGapPt : _poiCorrPt;
    std::vector<Histogram*> & cum   = gapped ? _cumGap : _cum;
    std::vector<Histogram*> & v     = gapped ? _vGap : _v;
    std::vector<Histogram*> & dcum  = gapped ? _dcumGap : _dcum;
    std::vector<Histogram*> & vp    = gapped ? _vpGap : _vp;
    std::vector<Histogram*> & dcumPt= gapped ? _dcumGapPt : _dcumPt;
    std::vector<Histogram*> & vpPt  = gapped ? _vpGapPt : _vpPt;
    for (unsigned int n=1; n<=N; n++)
      for (unsigned int e=0; e<nEvt; e++)
        {
        std::vector<double> m(K), em(K);
        std::vector<bool> filled(K);
        for (unsigned int k=1; k<=K; k++) filled[k-1] = meanOf(corr[at(n,k)],e,m[k-1],em[k-1]);
        for (unsigned int k=1; k<=K; k++)
          {
          bool all = std::all_of(filled.begin(),filled.begin()+k,[](bool b) { return b; });
          double c = 0.0, ec = 0.0, vv = 0.0, ev = 0.0;
          bool defined = all && FlowCumulants::flowWithError(m,em,k,c,ec,vv,ev);
          setBin(cum[at(n,k)],e,all,c,ec);
          setBin(v[at(n,k)],e,defined,vv,ev);
          // differential: integrated over the POI, then vs pT
          for (unsigned int j=0; j<J; j++)
            {
            for (int b=-1; b<int(nPt); b++)
              {
              std::vector<double> mp(k), emp(k);
              bool ok = all;
              for (unsigned int i=1; i<=k && ok; i++)
                ok = b<0 ? meanOf(poi[at(j,n,i)],e,mp[i-1],emp[i-1])
                         : meanOf(poiPt[at(j,n,i)],e+nEvt*b,mp[i-1],emp[i-1]);
              double d = 0.0, ed = 0.0, w = 0.0, ew = 0.0;
              bool vdefined = ok && FlowCumulants::differentialFlowWithError(m,em,mp,emp,k,d,ed,w,ew);
              if (b<0) { setBin(dcum[at(j,n,k)],e,ok,d,ed);           setBin(vp[at(j,n,k)],e,vdefined,w,ew); }
              else     { setBin(dcumPt[at(j,n,k)],e+nEvt*b,ok,d,ed);  setBin(vpPt[at(j,n,k)],e+nEvt*b,vdefined,w,ew); }
              }
            }
          }
        }
    }
  // scalar product: v{SP} = <u Q*>/sqrt(<Q_A Q_B*>)
  if (_scalarProduct)
    for (unsigned int n=1; n<=N; n++)
      for (unsigned int e=0; e<nEvt; e++)
        {
        double den, eden;
        bool ok = meanOf(_spQaQb[n-1],e,den,eden) && den>0.0;
        for (unsigned int j=0; j<J; j++)
          for (int b=-1; b<int(nPt); b++)
            {
            double num, enm;
            unsigned int bin = b<0 ? e : e+nEvt*b;
            bool okNum = ok && meanOf(b<0 ? _poiSP[atSP(j,n)] : _poiSPPt[atSP(j,n)],bin,num,enm);
            double r = 0.0, er = 0.0;
            if (okNum)
              {
              double s = std::sqrt(den);
              r  = num/s;
              er = std::sqrt(enm*enm/den + 0.25*r*r*eden*eden/(den*den));
              }
            setBin(b<0 ? _vSP[atSP(j,n)] : _vSPPt[atSP(j,n)],bin,okNum,r,er);
            }
        }
  }

  void ParticleFlowHistos::prepareWork()
  {
  const unsigned int K = nOrders(), M = _maxOrder, H = _nHarmonics*K;
  const unsigned int nBins = (pt_nbins+1)*nPoi();     // pT bins and the total, per POI filter
  _q.resize(H,M);
  _qA.resize(H,K);
  _qB.resize(H,K);
  _p.assign(nBins,FlowVector(H,1));
  _pq.assign(nBins,FlowVector(H,M));
  _pA.assign(nBins,FlowVector(H,1));
  _pB.assign(nBins,FlowVector(H,1));
  _pqA.assign(nBins,FlowVector(H,K));
  _pqB.assign(nBins,FlowVector(H,K));
  }

  int ParticleFlowHistos::subEvent(const Particle & particle) const
  {
  const MATH::VectorLorentz & p = particle.momentum();
  double x = _gapRapidity ? p.rapidity() : p.pseudorapidity();
  if (x < -0.5*_gapSize) return 0;
  if (x >  0.5*_gapSize) return 1;
  return -1;
  }

  int ParticleFlowHistos::ptBin(double pt) const
  {
  if (pt<pt_min || pt>=pt_max) return -1;
  int b = int((pt-pt_min)/(pt_max-pt_min)*pt_nbins);
  return b<pt_nbins ? b : pt_nbins-1;
  }

  void ParticleFlowHistos::fill(double evt, const std::vector<Particle*> & ref, const std::vector<double> & refWeights,
                                const std::vector<const std::vector<Particle*>*> & poi,
                                const std::vector<std::vector<double>> & poiWeights)
  {
  typedef std::complex<double> Complex;
  const unsigned int N = _nHarmonics, K = nOrders(), M = _maxOrder, J = nPoi();
  if (poi.size()!=J || poiWeights.size()!=J) throw Exception("Number of POI lists differs from the POI filters",__FUNCTION__);
  const unsigned int nBins = pt_nbins+1;
  if (_p.size()!=J*nBins || _q.maxPower()!=M) prepareWork();
  h_evt->fill(evt);

  // reference flow vectors
  _q.clear(); _qA.clear(); _qB.clear();
  for (unsigned int i=0; i<ref.size(); i++)
    {
    const Particle & particle = *ref[i];
    double phi = particle.momentum().phi(), w = refWeights[i];
    _q.add(phi,w);
    if (_gap)
      {
      int s = subEvent(particle);
      if (s==0) _qA.add(phi,w); else if (s==1) _qB.add(phi,w);
      }
    }
  const std::vector<int> zeros(M,0), zerosK(K,0);
  std::vector<Complex> D, DA, DB;
  FlowCorrelator::subsetSums(_q,zeros,D);
  if (_gap) { FlowCorrelator::subsetSums(_qA,zerosK,DA); FlowCorrelator::subsetSums(_qB,zerosK,DB); }

  // POI flow vectors, per pT bin (the last: all the POI)
  std::vector<Particle*> sortedRef(ref.begin(),ref.end());
  std::sort(sortedRef.begin(),sortedRef.end());
  const unsigned int total = pt_nbins;
  for (auto * v : {&_p, &_pq, &_pA, &_pB, &_pqA, &_pqB}) for (FlowVector & f : *v) f.clear();
  for (unsigned int j=0; j<J; j++)
    {
    const std::vector<Particle*> & particles = *poi[j];
    for (unsigned int i=0; i<particles.size(); i++)
      {
      Particle * particle = particles[i];
      const MATH::VectorLorentz & mom = particle->momentum();
      double phi = mom.phi(), w = poiWeights[j][i];
      int b = ptBin(mom.perp());
      bool both = std::binary_search(sortedRef.begin(),sortedRef.end(),particle);
      int s = _gap ? subEvent(*particle) : -1;
      for (int bin : {b, int(total)})
        {
        if (bin<0) continue;
        unsigned int k = j*nBins+bin;
        _p[k].add(phi,w);
        if (both) _pq[k].add(phi,w);
        if (s==0) { _pA[k].add(phi,w); if (both) _pqA[k].add(phi,w); }
        if (s==1) { _pB[k].add(phi,w); if (both) _pqB[k].add(phi,w); }
        }
      }
    }

  std::vector<Complex> Nn, NA, NB;
  for (unsigned int n=1; n<=N; n++)
    {
    const std::vector<int> h = FlowCorrelator::alternating(n,M);
    const std::vector<int> hK(K,int(n));
    FlowCorrelator::subsetSums(_q,h,Nn);
    for (unsigned int k=1; k<=K; k++)
      {
      unsigned int mask = FlowCorrelator::firstSlots(2*k);
      double den = D[mask].real();
      if (den>0.0) _corr[at(n,k)]->fill(evt,Nn[mask].real()/den,den);
      }
    if (_gap)
      {
      FlowCorrelator::subsetSums(_qA,hK,NA);
      FlowCorrelator::subsetSums(_qB,hK,NB);
      for (unsigned int k=1; k<=K; k++)
        {
        unsigned int mask = FlowCorrelator::firstSlots(k);
        double den = DA[mask].real()*DB[mask].real();
        if (den>0.0) _corrGap[at(n,k)]->fill(evt,(NA[mask]*std::conj(NB[mask])).real()/den,den);
        }
      }
    Complex qa = _gap ? _qA.q(n,1) : Complex(), qb = _gap ? _qB.q(n,1) : Complex();
    bool sp = _scalarProduct && _qA.count()>0 && _qB.count()>0;
    if (sp) _spQaQb[n-1]->fill(evt,(qa*std::conj(qb)).real(),1.0);

    for (unsigned int j=0; j<J; j++)
      for (unsigned int bin=0; bin<=total; bin++)
        {
        unsigned int jb = j*nBins+bin;
        if (_p[jb].count()==0) continue;
        bool all = bin==total;
        double pt = all ? 0.0 : pt_min + (bin+0.5)*(pt_max-pt_min)/pt_nbins;
        auto fillTo = [&](Profile * integrated, Profile * differential, double value, double weight)
          { if (all) integrated->fill(evt,value,weight); else differential->fill(evt,pt,value,weight); };
        for (unsigned int k=1; k<=K; k++)
          {
          unsigned int others = FlowCorrelator::firstSlots(2*k) ^ 1u;
          double den = FlowCorrelator::differential(_p[jb],_pq[jb],0,zeros,others,D).real();
          if (den>0.0)
            {
            double num = FlowCorrelator::differential(_p[jb],_pq[jb],n,h,others,Nn).real();
            fillTo(_poiCorr[at(j,n,k)],_poiCorrPt[at(j,n,k)],num/den,den);
            }
          }
        if (_gap)
          for (unsigned int k=1; k<=K; k++)
            {
            unsigned int full = FlowCorrelator::firstSlots(k), others = full ^ 1u;
            Complex num(0.0,0.0);
            double den = 0.0;
            if (_pA[jb].count()>0)
              {
              num += FlowCorrelator::differential(_pA[jb],_pqA[jb],n,hK,others,NA)*std::conj(NB[full]);
              den += FlowCorrelator::differential(_pA[jb],_pqA[jb],0,zerosK,others,DA).real()*DB[full].real();
              }
            if (_pB[jb].count()>0)
              {
              num += FlowCorrelator::differential(_pB[jb],_pqB[jb],n,hK,others,NB)*std::conj(NA[full]);
              den += FlowCorrelator::differential(_pB[jb],_pqB[jb],0,zerosK,others,DB).real()*DA[full].real();
              }
            if (den>0.0) fillTo(_poiCorrGap[at(j,n,k)],_poiCorrGapPt[at(j,n,k)],num.real()/den,den);
            }
        if (sp)
          {
          double num = (_pA[jb].q(n,1)*std::conj(qb) + _pB[jb].q(n,1)*std::conj(qa)).real();
          double den = _pA[jb].weightSum(1) + _pB[jb].weightSum(1);
          if (den>0.0) fillTo(_poiSP[atSP(j,n)],_poiSPPt[atSP(j,n)],num/den,den);
          }
        }
    }
  }

} // namespace CAP
