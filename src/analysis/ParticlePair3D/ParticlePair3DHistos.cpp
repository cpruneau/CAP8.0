/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "ParticlePair3DHistos.hpp"
#include "MathConstants.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "Task.hpp"
#include "Configuration.hpp"


namespace CAP
{

  ParticlePair3DHistos::ParticlePair3DHistos()
  :
  HistogramFolder(),
  n2_nbins(100),
  Qinv_nbins(100),
  DeltaPs_nbins(100),
  DeltaPo_nbins(100),
  DeltaPl_nbins(100),
  pt_nbins(100),
  rapidity_nbins(100),
  n2_min(0),
  n2_max(100),
  Qinv_min(0),
  Qinv_max(10.0),
  DeltaPs_min(0),
  DeltaPs_max(10.0),
  DeltaPo_min(0),
  DeltaPo_max(10.0),
  DeltaPl_min(0),
  DeltaPl_max(10.0),
  pt_min(0),
  pt_max(10.0),
  rapidity_minimum(-2),
  rapidity_maximum(2),
  h_n2(nullptr),
  h_n2_Qinv(nullptr),
  h_n2_DeltaP(nullptr)
  {
  appendClassName("ParticlePair3DHistos");
  setName("ParticlePair3DHistos");
  setTitle("ParticlePair3DHistos");
  }

  ParticlePair3DHistos::ParticlePair3DHistos(const ParticlePair3DHistos & source)
  :
  HistogramFolder(source),
  n2_nbins(source.n2_nbins),
  Qinv_nbins(source.Qinv_nbins),
  DeltaPs_nbins(source.DeltaPs_nbins),
  DeltaPo_nbins(source.DeltaPo_nbins),
  DeltaPl_nbins(source.DeltaPl_nbins),
  pt_nbins(source.pt_nbins),
  rapidity_nbins(source.rapidity_nbins),
  n2_min(source.n2_min),
  n2_max(source.n2_max),
  Qinv_min(source.Qinv_min),
  Qinv_max(source.Qinv_max),
  DeltaPs_min(source.DeltaPs_min),
  DeltaPs_max(source.DeltaPs_max),
  DeltaPo_min(source.DeltaPo_min),
  DeltaPo_max(source.DeltaPo_max),
  DeltaPl_min(source.DeltaPl_min),
  DeltaPl_max(source.DeltaPl_max),
  pt_min(source.pt_min),
  pt_max(source.pt_max),
  rapidity_minimum(source.rapidity_minimum),
  rapidity_maximum(source.rapidity_maximum),
  h_n2(nullptr),
  h_n2_Qinv(nullptr),
  h_n2_DeltaP(nullptr)
  {
  cloneB(source);
  }

  ParticlePair3DHistos & ParticlePair3DHistos::operator=(const ParticlePair3DHistos & rhs)
  {
  if (this!=&rhs)
    {
    HistogramFolder::operator=(rhs),
    n2_nbins      = rhs.n2_nbins;
    Qinv_nbins    = rhs.Qinv_nbins;
    DeltaPs_nbins = rhs.DeltaPs_nbins;
    DeltaPo_nbins = rhs.DeltaPo_nbins;
    DeltaPl_nbins = rhs.DeltaPl_nbins;
    pt_nbins      = rhs.pt_nbins;
    rapidity_nbins       = rhs.rapidity_nbins;
    n2_min        = rhs.n2_min;
    n2_max        = rhs.n2_max;
    Qinv_min      = rhs.Qinv_min;
    Qinv_max      = rhs.Qinv_max;
    DeltaPs_min   = rhs.DeltaPs_min;
    DeltaPs_max   = rhs.DeltaPs_max;
    DeltaPo_min   = rhs.DeltaPo_min;
    DeltaPo_max   = rhs.DeltaPo_max;
    DeltaPl_min   = rhs.DeltaPl_min;
    DeltaPl_max   = rhs.DeltaPl_max;
    pt_min        = rhs.pt_min;
    pt_max        = rhs.pt_max;
    rapidity_minimum         = rhs.rapidity_minimum;
    rapidity_maximum         = rhs.rapidity_maximum;
    }
  return *this;
  }

  //!
  //! Read the binning from the HISTOGRAM group of the task (<task>:HISTOGRAM:<param>): the list a folder belongs to
  //! (HISTOGRAM_1, ...) is passed as type, and a folder with a binning of its own gets it in that group too (see
  //! ManagedObjects::objectConfiguration()), as for ParticlePairHistos and ParticleSingleHistos.
  //!
  void ParticlePair3DHistos::configure(const String & taskName,
                                       const String & type,
                                       const Configuration & configuration,
                                       unsigned int index)
  {
  HistogramFolder::configure(taskName,type,configuration,index);
  String objectType = "HISTOGRAM";
  n2_nbins       = configuration.valueInt(   createKey(taskName,objectType,"n2_nbins"));
  n2_min         = configuration.valueDouble(createKey(taskName,objectType,"n2_min"));
  n2_max         = configuration.valueDouble(createKey(taskName,objectType,"n2_max"));
  Qinv_nbins     = configuration.valueInt(   createKey(taskName,objectType,"Qinv_nbins"));
  Qinv_min       = configuration.valueDouble(createKey(taskName,objectType,"Qinv_min"));
  Qinv_max       = configuration.valueDouble(createKey(taskName,objectType,"Qinv_max"));
  DeltaPs_nbins  = configuration.valueInt(   createKey(taskName,objectType,"DeltaPs_nbins"));
  DeltaPo_nbins  = configuration.valueInt(   createKey(taskName,objectType,"DeltaPo_nbins"));
  DeltaPl_nbins  = configuration.valueInt(   createKey(taskName,objectType,"DeltaPl_nbins"));
  DeltaPs_min    = configuration.valueDouble(createKey(taskName,objectType,"DeltaPs_min"));
  DeltaPs_max    = configuration.valueDouble(createKey(taskName,objectType,"DeltaPs_max"));
  DeltaPo_min    = configuration.valueDouble(createKey(taskName,objectType,"DeltaPo_min"));
  DeltaPo_max    = configuration.valueDouble(createKey(taskName,objectType,"DeltaPo_max"));
  DeltaPl_min    = configuration.valueDouble(createKey(taskName,objectType,"DeltaPl_min"));
  DeltaPl_max    = configuration.valueDouble(createKey(taskName,objectType,"DeltaPl_max"));
  pt_nbins          = configuration.valueInt(   createKey(taskName,objectType,"pt_nbins"));         // derived histograms
  pt_min            = configuration.valueDouble(createKey(taskName,objectType,"pt_min"));
  pt_max            = configuration.valueDouble(createKey(taskName,objectType,"pt_max"));
  rapidity_nbins    = configuration.valueInt(   createKey(taskName,objectType,"rapidity_nbins"));
  rapidity_minimum  = configuration.valueDouble(createKey(taskName,objectType,"rapidity_min"));
  rapidity_maximum  = configuration.valueDouble(createKey(taskName,objectType,"rapidity_max"));
  if (reportDebug(__FUNCTION__))
    {
    printValue(createKey(taskName,objectType,"n2_nbins"),n2_nbins);
    printValue(createKey(taskName,objectType,"n2_min"),n2_min);
    printValue(createKey(taskName,objectType,"n2_max"),n2_max);
    printValue(createKey(taskName,objectType,"Qinv_nbins"),Qinv_nbins);
    printValue(createKey(taskName,objectType,"Qinv_min"),Qinv_min);
    printValue(createKey(taskName,objectType,"Qinv_max"),Qinv_max);
    printValue(createKey(taskName,objectType,"DeltaPs_nbins"),DeltaPs_nbins);
    printValue(createKey(taskName,objectType,"DeltaPo_nbins"),DeltaPo_nbins);
    printValue(createKey(taskName,objectType,"DeltaPl_nbins"),DeltaPl_nbins);
    printValue(createKey(taskName,objectType,"pt_nbins"),pt_nbins);
    printValue(createKey(taskName,objectType,"rapidity_nbins"),rapidity_nbins);
    }
  }

  void ParticlePair3DHistos::create()
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  h_n2        = createHistogram(createName(_histogramBaseName,"n2"),n2_nbins,  n2_min,  n2_max, "n_{2}", "Yield");
  h_n2_Qinv   = createHistogram(createName(_histogramBaseName,"n2_Qinv"),Qinv_nbins,Qinv_min,Qinv_max, "Q_{inv}","n_{2}",2);
  h_n2_DeltaP = createHistogram(createName(_histogramBaseName,"n2_DeltaP"),
                                DeltaPs_nbins,  DeltaPs_min, DeltaPs_max,
                                DeltaPo_nbins,  DeltaPo_min, DeltaPo_max,
                                DeltaPl_nbins,  DeltaPl_min, DeltaPl_max,
                                "p_{s}","p_{o}", "p_{l}");
  }

  void ParticlePair3DHistos::loadFrom(const Folder & inputFile)
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  h_n2        = importHistogram(inputFile, createName(_histogramBaseName,"n2"));
  h_n2_Qinv   = importHistogram(inputFile, createName(_histogramBaseName,"n2_Qinv"));
  h_n2_DeltaP = importHistogram(inputFile, createName(_histogramBaseName,"n2_DeltaP"));
  }

  void ParticlePair3DHistos::cloneB(const ParticlePair3DHistos & source)
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  h_n2        = histogramLike(source.h_n2);
  h_n2_Qinv   = histogramLike(source.h_n2_Qinv);
  h_n2_DeltaP = histogramLike(source.h_n2_DeltaP);
  }


  void ParticlePair3DHistos::fill(Particle & particle1, Particle & particle2, double weight)
  {
  //decompose(double *pa,double *pb,double & qlong,double & qout,double & qside,double & qinv)
  // ====================================================================================
  // Method from Scott Pratt to do three dimensional qinv components
  // ====================================================================================
  double pt,s,Mlong,roots;
  double pa[4], pb[4], ptot[4],q[4];
  const int g[4]={1,-1,-1,-1};
  double qinv, qside, qlong, qout;

  VectorLorentz & momentumA = particle1.momentum();
  VectorLorentz & momentumB = particle2.momentum();
  pa[0] = momentumA.t();
  pa[1] = momentumA.x();
  pa[2] = momentumA.y();
  pa[3] = momentumA.z();

  pb[0] = momentumB.t();
  pb[1] = momentumB.x();
  pb[2] = momentumB.y();
  pb[3] = momentumB.z();

  qinv=0.0;
  s=0.0;
  for(int alpha=0;alpha<4;alpha++)
    {
    ptot[alpha]=pa[alpha]+pb[alpha];
    s+=g[alpha]*ptot[alpha]*ptot[alpha];
    q[alpha]=pa[alpha]-pb[alpha];
    qinv-=g[alpha]*q[alpha]*q[alpha];
    }
  pt=std::sqrt(ptot[1]*ptot[1]+ptot[2]*ptot[2]);
  Mlong=std::sqrt(s+pt*pt);
  roots=std::sqrt(s);

  qside=(ptot[1]*q[2]-ptot[2]*q[1])/pt;
  qlong=(ptot[0]*q[3]-ptot[3]*q[0])/Mlong;
  qout=(roots/Mlong)*(ptot[1]*q[1]+ptot[2]*q[2])/pt;
  qinv=std::sqrt(qinv);

  h_n2_Qinv->fill(qinv,weight);
  h_n2_DeltaP->fill(qside,qout,qlong,weight);
  }


  void ParticlePair3DHistos::fillMultiplicity(double nPairs, double weight)
  {
  h_n2->fill(nPairs, weight);
  }


}  //  namespace CAP



