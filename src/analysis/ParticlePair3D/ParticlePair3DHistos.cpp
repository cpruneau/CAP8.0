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
#include "Exceptions.hpp"
#include "TRandom.h"
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
  h_n2_DeltaP(nullptr),
  h_n1n1_Qinv(nullptr),
  h_n1n1_DeltaP(nullptr),
  h_n1n1_DeltaPs(nullptr),
  h_n1n1_DeltaPo(nullptr),
  h_n1n1_DeltaPl(nullptr),
  h_n2_DeltaPs(nullptr),
  h_n2_DeltaPo(nullptr),
  h_n2_DeltaPl(nullptr),
  h_c2_Qinv(nullptr),
  h_c2_DeltaP(nullptr),
  h_c2_DeltaPs(nullptr),
  h_c2_DeltaPo(nullptr),
  h_c2_DeltaPl(nullptr),
  h_r2_Qinv(nullptr),
  h_r2_DeltaP(nullptr),
  h_r2_DeltaPs(nullptr),
  h_r2_DeltaPo(nullptr),
  h_r2_DeltaPl(nullptr),
  h_a12_Qinv(nullptr),
  h_a12_DeltaPs(nullptr),
  h_a12_DeltaPo(nullptr),
  h_a12_DeltaPl(nullptr),
  h_a21_Qinv(nullptr),
  h_a21_DeltaPs(nullptr),
  h_a21_DeltaPo(nullptr),
  h_a21_DeltaPl(nullptr),
  h_n1_1_pt(nullptr),
  h_n1_2_pt(nullptr),
  h_n1_1_y(nullptr),
  h_n1_2_y(nullptr),
  h_n1_1_pt_mc(nullptr),
  h_n1_2_pt_mc(nullptr),
  h_n1_1_y_mc(nullptr),
  h_n1_2_y_mc(nullptr),
  h_n1r_1_pt_mc(nullptr),
  h_n1r_2_pt_mc(nullptr),
  h_n1r_1_y_mc(nullptr),
  h_n1r_2_y_mc(nullptr),
  h_n1_1_ptY_mc(nullptr),
  h_n1_2_ptY_mc(nullptr),
  h_n1r_1_ptY_mc(nullptr),
  h_n1r_2_ptY_mc(nullptr),
  mass1(0.13957039),
  mass2(0.13957039),
  nMcSimEvents(1000)
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
  h_n2_DeltaP(nullptr),
  h_n1n1_Qinv(nullptr),
  h_n1n1_DeltaP(nullptr),
  h_n1n1_DeltaPs(nullptr),
  h_n1n1_DeltaPo(nullptr),
  h_n1n1_DeltaPl(nullptr),
  h_n2_DeltaPs(nullptr),
  h_n2_DeltaPo(nullptr),
  h_n2_DeltaPl(nullptr),
  h_c2_Qinv(nullptr),
  h_c2_DeltaP(nullptr),
  h_c2_DeltaPs(nullptr),
  h_c2_DeltaPo(nullptr),
  h_c2_DeltaPl(nullptr),
  h_r2_Qinv(nullptr),
  h_r2_DeltaP(nullptr),
  h_r2_DeltaPs(nullptr),
  h_r2_DeltaPo(nullptr),
  h_r2_DeltaPl(nullptr),
  h_a12_Qinv(nullptr),
  h_a12_DeltaPs(nullptr),
  h_a12_DeltaPo(nullptr),
  h_a12_DeltaPl(nullptr),
  h_a21_Qinv(nullptr),
  h_a21_DeltaPs(nullptr),
  h_a21_DeltaPo(nullptr),
  h_a21_DeltaPl(nullptr),
  h_n1_1_pt(nullptr),
  h_n1_2_pt(nullptr),
  h_n1_1_y(nullptr),
  h_n1_2_y(nullptr),
  h_n1_1_pt_mc(nullptr),
  h_n1_2_pt_mc(nullptr),
  h_n1_1_y_mc(nullptr),
  h_n1_2_y_mc(nullptr),
  h_n1r_1_pt_mc(nullptr),
  h_n1r_2_pt_mc(nullptr),
  h_n1r_1_y_mc(nullptr),
  h_n1r_2_y_mc(nullptr),
  h_n1_1_ptY_mc(nullptr),
  h_n1_2_ptY_mc(nullptr),
  h_n1r_1_ptY_mc(nullptr),
  h_n1r_2_ptY_mc(nullptr),
  mass1(source.mass1),
  mass2(source.mass2),
  nMcSimEvents(source.nMcSimEvents)
  {
  cloneB(source);
  }

  ParticlePair3DHistos & ParticlePair3DHistos::operator=(const ParticlePair3DHistos & rhs)
  {
  if (this!=&rhs)
    {
    HistogramFolder::operator=(rhs);
    cloneB(rhs);
    mass1 = rhs.mass1;
    mass2 = rhs.mass2;
    nMcSimEvents = rhs.nMcSimEvents;
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
  void ParticlePair3DHistos::addDefaultConfiguration(Configuration & configuration, const String & taskName)
  {
  ParticleSingleHistos::addDefaultConfiguration(configuration,taskName);
  const String type = "HISTOGRAM";
  configuration.addProperty(createKey(taskName,type,"n1_nbins"),      100);
  configuration.addProperty(createKey(taskName,type,"n1_min"),        0.0);
  configuration.addProperty(createKey(taskName,type,"n1_max"),        100.0);
  configuration.addProperty(createKey(taskName,type,"etot_nbins"),    100);
  configuration.addProperty(createKey(taskName,type,"etot_min"),      0.0);
  configuration.addProperty(createKey(taskName,type,"etot_max"),      100.0);
  configuration.addProperty(createKey(taskName,type,"pt_nbins"),      100);
  configuration.addProperty(createKey(taskName,type,"pt_min"),        0.0);
  configuration.addProperty(createKey(taskName,type,"pt_max"),        100.0);
  configuration.addProperty(createKey(taskName,type,"eta_nbins"),     20);       // read by the single-particle histograms (eta_fill)
  configuration.addProperty(createKey(taskName,type,"eta_min"),       -1.0);
  configuration.addProperty(createKey(taskName,type,"eta_max"),       1.0);
  configuration.addProperty(createKey(taskName,type,"rapidity_nbins"), 200);
  configuration.addProperty(createKey(taskName,type,"rapidity_min"),  -10.0);
  configuration.addProperty(createKey(taskName,type,"rapidity_max"),  10.0);
  configuration.addProperty(createKey(taskName,type,"phi_nbins"),     36);
  configuration.addProperty(createKey(taskName,type,"phi_min"),       0.0);
  configuration.addProperty(createKey(taskName,type,"phi_max"),       CAP::MATH::twoPi());
  configuration.addProperty(createKey(taskName,type,"phiEta_nbins"),  720);
  configuration.addProperty(createKey(taskName,type,"phiEtaPt_nbins"), 7200);
  configuration.addProperty(createKey(taskName,type,"phiY_nbins"),    720);
  configuration.addProperty(createKey(taskName,type,"phiYPt_nbins"),  7200);
  configuration.addProperty(createKey(taskName,type,"eta_fill"),      false);
  configuration.addProperty(createKey(taskName,type,"rapidity_fill"), true);
  configuration.addProperty(createKey(taskName,type,"p2_fill"),       false);
  configuration.addProperty(createKey(taskName,type,"FillPid"),       false);
  configuration.addProperty(createKey(taskName,type,"FillPtvsY"),     true); // needed for n1n1 calculation
  configuration.addProperty(createKey(taskName,type,"n2_nbins"),      100);    // pair multiplicity (ParticlePair3DHistos)
  configuration.addProperty(createKey(taskName,type,"n2_min"),        0.0);
  configuration.addProperty(createKey(taskName,type,"n2_max"),        1000.0);
  configuration.addProperty(createKey(taskName,type,"Qinv_nbins"),    90);     // q invariant
  configuration.addProperty(createKey(taskName,type,"Qinv_min"),      0.0);
  configuration.addProperty(createKey(taskName,type,"Qinv_max"),      1.8);
  configuration.addProperty(createKey(taskName,type,"DeltaPs_nbins"), 60);     // q invariant side
  configuration.addProperty(createKey(taskName,type,"DeltaPs_min"),   0.0);
  configuration.addProperty(createKey(taskName,type,"DeltaPs_max"),   1.2);
  configuration.addProperty(createKey(taskName,type,"DeltaPo_nbins"), 60);     // q invariant out
  configuration.addProperty(createKey(taskName,type,"DeltaPo_min"),   0.0);
  configuration.addProperty(createKey(taskName,type,"DeltaPo_max"),   1.2);
  configuration.addProperty(createKey(taskName,type,"DeltaPl_nbins"), 60);     // q invariant long
  configuration.addProperty(createKey(taskName,type,"DeltaPl_min"),   0.0);
  configuration.addProperty(createKey(taskName,type,"DeltaPl_max"),   1.2);
  configuration.addProperty(createKey(taskName,type,"nMcSimEvents"),   1000);   // Monte Carlo events of the n1n1 calculation
  }

  void ParticlePair3DHistos::configure(const String & taskName,
                                       const String & type,
                                       const Configuration & configuration,
                                       unsigned int index)
  {
  HistogramFolder::configure(taskName,type,configuration,index);
  String objectType = "HISTOGRAM";
  nMcSimEvents   = configuration.valueInt(   createKey(taskName,objectType,"nMcSimEvents"));
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
  // derived histograms: present once ParticlePair3DDerivedCalculator has run
  if (inputFile.contains(createName(_histogramBaseName,"n1n1_Qinv"))) loadDerivedFrom(inputFile);
  }

  void ParticlePair3DHistos::cloneB(const ParticlePair3DHistos & source)
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  h_n2        = histogramLike(source.h_n2);
  h_n2_Qinv   = histogramLike(source.h_n2_Qinv);
  h_n2_DeltaP = histogramLike(source.h_n2_DeltaP);

  if (reportDebug(__FUNCTION__)) { printCR(); }
  h_n1n1_Qinv    = histogramLike(source.h_n1n1_Qinv);
  h_n1n1_DeltaP  = histogramLike(source.h_n1n1_DeltaP);
  h_n1n1_DeltaPs = histogramLike(source.h_n1n1_DeltaPs);
  h_n1n1_DeltaPo = histogramLike(source.h_n1n1_DeltaPo);
  h_n1n1_DeltaPl = histogramLike(source.h_n1n1_DeltaPl);

  h_n2_DeltaPs   = histogramLike(source.h_n2_DeltaPs);
  h_n2_DeltaPo   = histogramLike(source.h_n2_DeltaPo);
  h_n2_DeltaPl   = histogramLike(source.h_n2_DeltaPl);

  h_c2_Qinv      = histogramLike(source.h_c2_Qinv);
  h_c2_DeltaP    = histogramLike(source.h_c2_DeltaP);
  h_c2_DeltaPs   = histogramLike(source.h_c2_DeltaPs);
  h_c2_DeltaPo   = histogramLike(source.h_c2_DeltaPo);
  h_c2_DeltaPl   = histogramLike(source.h_c2_DeltaPl);

  h_r2_Qinv      = histogramLike(source.h_r2_Qinv);
  h_r2_DeltaP    = histogramLike(source.h_r2_DeltaP);
  h_r2_DeltaPs   = histogramLike(source.h_r2_DeltaPs);
  h_r2_DeltaPo   = histogramLike(source.h_r2_DeltaPo);
  h_r2_DeltaPl   = histogramLike(source.h_r2_DeltaPl);

  h_a12_Qinv     = histogramLike(source.h_a12_Qinv);
  h_a12_DeltaPs  = histogramLike(source.h_a12_DeltaPs);
  h_a12_DeltaPo  = histogramLike(source.h_a12_DeltaPo);
  h_a12_DeltaPl  = histogramLike(source.h_a12_DeltaPl);

  h_a21_Qinv     = histogramLike(source.h_a21_Qinv);
  h_a21_DeltaPs  = histogramLike(source.h_a21_DeltaPs);
  h_a21_DeltaPo  = histogramLike(source.h_a21_DeltaPo);
  h_a21_DeltaPl  = histogramLike(source.h_a21_DeltaPl);

  h_n1_1_pt      = histogramLike(source.h_n1_1_pt);
  h_n1_2_pt      = histogramLike(source.h_n1_2_pt);
  h_n1_1_y       = histogramLike(source.h_n1_1_y);
  h_n1_2_y       = histogramLike(source.h_n1_2_y);

  h_n1_1_pt_mc   = histogramLike(source.h_n1_1_pt_mc);
  h_n1_2_pt_mc   = histogramLike(source.h_n1_2_pt_mc);
  h_n1_1_y_mc    = histogramLike(source.h_n1_1_y_mc);
  h_n1_2_y_mc    = histogramLike(source.h_n1_2_y_mc);

  h_n1r_1_pt_mc  = histogramLike(source.h_n1r_1_pt_mc);
  h_n1r_2_pt_mc  = histogramLike(source.h_n1r_2_pt_mc);
  h_n1r_1_y_mc   = histogramLike(source.h_n1r_1_y_mc);
  h_n1r_2_y_mc   = histogramLike(source.h_n1r_2_y_mc);

  h_n1_1_ptY_mc  = histogramLike(source.h_n1_1_ptY_mc);
  h_n1_2_ptY_mc  = histogramLike(source.h_n1_2_ptY_mc);
  h_n1r_1_ptY_mc = histogramLike(source.h_n1r_1_ptY_mc);
  h_n1r_2_ptY_mc = histogramLike(source.h_n1r_2_ptY_mc);
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



  void ParticlePair3DHistos::createDerived()
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  h_n2_DeltaPs= createHistogram(createName(_histogramBaseName,"n2_DeltaPs"),DeltaPs_nbins,DeltaPs_min,DeltaPs_max,"p_{s}","n_{2}",2);
  h_n2_DeltaPo= createHistogram(createName(_histogramBaseName,"n2_DeltaPo"),DeltaPo_nbins,DeltaPo_min,DeltaPo_max,"p_{o}","n_{2}",2);
  h_n2_DeltaPl= createHistogram(createName(_histogramBaseName,"n2_DeltaPl"),DeltaPl_nbins,DeltaPl_min,DeltaPl_max,"p_{l}","n_{2}",2);

  h_n1n1_Qinv   = createHistogram(createName(_histogramBaseName,"n1n1_Qinv"),Qinv_nbins,Qinv_min,Qinv_max, "Q_{inv}","n_{1}n_{1}",2);
  h_n1n1_DeltaP = createHistogram(createName(_histogramBaseName,"n1n1_DeltaP"),
                                  DeltaPs_nbins,DeltaPs_min,DeltaPs_max,
                                  DeltaPo_nbins,DeltaPo_min,DeltaPo_max,
                                  DeltaPl_nbins,DeltaPl_min,DeltaPl_max,
                                  "p_{s}","p_{o}", "p_{l}");

  h_n1n1_DeltaPs= createHistogram(createName(_histogramBaseName,"n1n1_DeltaPs"),DeltaPs_nbins,DeltaPs_min,DeltaPs_max,"p_{s}","n_{1}n_{1}",2);
  h_n1n1_DeltaPo= createHistogram(createName(_histogramBaseName,"n1n1_DeltaPo"),DeltaPo_nbins,DeltaPo_min,DeltaPo_max,"p_{o}","n_{1}n_{1}",2);
  h_n1n1_DeltaPl= createHistogram(createName(_histogramBaseName,"n1n1_DeltaPl"),DeltaPl_nbins,DeltaPl_min,DeltaPl_max,"p_{l}","n_{1}n_{1}",2);

  h_c2_Qinv   = createHistogram(createName(_histogramBaseName,"c2_Qinv"), Qinv_nbins,Qinv_min,Qinv_max, "Q_{inv}","c_{2}",2);
  h_c2_DeltaP = createHistogram(createName(_histogramBaseName,"c2_DeltaP"),
                                DeltaPs_nbins,DeltaPs_min,DeltaPs_max,
                                DeltaPo_nbins,DeltaPo_min,DeltaPo_max,
                                DeltaPl_nbins,DeltaPl_min,DeltaPl_max,
                                "p_{s}","p_{o}", "p_{l}");
  h_c2_DeltaPs= createHistogram(createName(_histogramBaseName,"c2_DeltaPs"),DeltaPs_nbins,DeltaPs_min,DeltaPs_max,"p_{s}","c_{2}",2);
  h_c2_DeltaPo= createHistogram(createName(_histogramBaseName,"c2_DeltaPo"),DeltaPo_nbins,DeltaPo_min,DeltaPo_max,"p_{o}","c_{2}",2);
  h_c2_DeltaPl= createHistogram(createName(_histogramBaseName,"c2_DeltaPl"),DeltaPl_nbins,DeltaPl_min,DeltaPl_max,"p_{l}","c_{2}",2);

  
  h_r2_Qinv   = createHistogram(createName(_histogramBaseName,"r2_Qinv"), Qinv_nbins,Qinv_min,Qinv_max, "Q_{inv}","r_{2}",2);
  h_r2_DeltaP = createHistogram(createName(_histogramBaseName,"r2_DeltaP"),
                                DeltaPs_nbins,DeltaPs_min,DeltaPs_max,
                                DeltaPo_nbins,DeltaPo_min,DeltaPo_max,
                                DeltaPl_nbins,DeltaPl_min,DeltaPl_max,
                                "p_{s}","p_{o}", "p_{l}");

  h_r2_DeltaPs= createHistogram(createName(_histogramBaseName,"r2_DeltaPs"),DeltaPs_nbins,DeltaPs_min,DeltaPs_max,"p_{s}","r_{2}",2);
  h_r2_DeltaPo= createHistogram(createName(_histogramBaseName,"r2_DeltaPo"),DeltaPo_nbins,DeltaPo_min,DeltaPo_max,"p_{o}","r_{2}",2);
  h_r2_DeltaPl= createHistogram(createName(_histogramBaseName,"r2_DeltaPl"),DeltaPl_nbins,DeltaPl_min,DeltaPl_max,"p_{l}","r_{2}",2);

  h_a12_Qinv      = createHistogram(createName(_histogramBaseName,"A12_Qinv"),    Qinv_nbins,Qinv_min,Qinv_max, "Q_{inv}","A_{12}",2);
  h_a12_DeltaPs   = createHistogram(createName(_histogramBaseName,"A12_DeltaPs"), DeltaPs_nbins,DeltaPs_min,DeltaPs_max, "p_{s}","A_{12}",2);
  h_a12_DeltaPo   = createHistogram(createName(_histogramBaseName,"A12_DeltaPo"), DeltaPo_nbins,DeltaPo_min,DeltaPo_max, "p_{o}","A_{12}",2);
  h_a12_DeltaPl   = createHistogram(createName(_histogramBaseName,"A12_DeltaPl"), DeltaPl_nbins,DeltaPl_min,DeltaPl_max, "p_{l}","A_{12}",2);
  h_a21_Qinv      = createHistogram(createName(_histogramBaseName,"A21_Qinv"),    Qinv_nbins,Qinv_min,Qinv_max, "Q_{inv}","A_{21}",2);
  h_a21_DeltaPs   = createHistogram(createName(_histogramBaseName,"A21_DeltaPs"), DeltaPs_nbins,DeltaPs_min,DeltaPs_max, "p_{s}","A_{21}",2);
  h_a21_DeltaPo   = createHistogram(createName(_histogramBaseName,"A21_DeltaPo"), DeltaPo_nbins,DeltaPo_min,DeltaPo_max, "p_{o}","A_{21}",2);
  h_a21_DeltaPl   = createHistogram(createName(_histogramBaseName,"A21_DeltaPl"), DeltaPl_nbins,DeltaPl_min,DeltaPl_max, "p_{l}","A_{21}",2);

  h_n1_1_pt       = createHistogram(createName(_histogramBaseName,"n1_1_pt"), pt_nbins, pt_min,pt_max,"p_{T}",   "#rho_{1}^{1}(p_{T})");
  h_n1_2_pt       = createHistogram(createName(_histogramBaseName,"n1_2_pt"), pt_nbins, pt_min,pt_max,"p_{T}",   "#rho_{1}^{2}(p_{T})");
  h_n1_1_y        = createHistogram(createName(_histogramBaseName,"n1_1_y"),  rapidity_nbins,rapidity_minimum,rapidity_maximum, "y",   "#rho_{1}^{1}(y)");
  h_n1_2_y        = createHistogram(createName(_histogramBaseName,"n1_2_y"),  rapidity_nbins,rapidity_minimum,rapidity_maximum, "y",   "#rho_{1}^{2}(y)");

  h_n1_1_pt_mc  = createHistogram(createName(_histogramBaseName,"n1_1_pt_mc"), pt_nbins,pt_min,pt_max, "p_{T}","N",2);
  h_n1_2_pt_mc  = createHistogram(createName(_histogramBaseName,"n1_2_pt_mc"), pt_nbins,pt_min,pt_max, "p_{T}","N",2);
  h_n1_1_y_mc   = createHistogram(createName(_histogramBaseName,"n1_1_y_mc"),  rapidity_nbins,rapidity_minimum,rapidity_maximum, "y","N",2);
  h_n1_2_y_mc   = createHistogram(createName(_histogramBaseName,"n1_2_y_mc"),  rapidity_nbins,rapidity_minimum,rapidity_maximum, "y","N",2);
  h_n1_1_ptY_mc = createHistogram(createName(_histogramBaseName,"n1_1_ptY_mc"),rapidity_nbins,rapidity_minimum,rapidity_maximum, pt_nbins,pt_min,pt_max, "y","p_{T}","N");
  h_n1_2_ptY_mc = createHistogram(createName(_histogramBaseName,"n1_2_ptY_mc"),rapidity_nbins,rapidity_minimum,rapidity_maximum, pt_nbins,pt_min,pt_max, "y","p_{T}","N");

  h_n1r_1_pt_mc  = createHistogram(createName(_histogramBaseName,"n1r_1_pt_mc"), pt_nbins,pt_min,pt_max, "p_{T}","N",2);
  h_n1r_2_pt_mc  = createHistogram(createName(_histogramBaseName,"n1r_2_pt_mc"), pt_nbins,pt_min,pt_max, "p_{T}","N",2);
  h_n1r_1_y_mc   = createHistogram(createName(_histogramBaseName,"n1r_1_y_mc"),  rapidity_nbins,rapidity_minimum,rapidity_maximum, "y","N",2);
  h_n1r_2_y_mc   = createHistogram(createName(_histogramBaseName,"n1r_2_y_mc"),  rapidity_nbins,rapidity_minimum,rapidity_maximum, "y","N",2);
  h_n1r_1_ptY_mc = createHistogram(createName(_histogramBaseName,"n1r_1_ptY_mc"),rapidity_nbins,rapidity_minimum,rapidity_maximum, pt_nbins,pt_min,pt_max, "y","p_{T}","N");
  h_n1r_2_ptY_mc = createHistogram(createName(_histogramBaseName,"n1r_2_ptY_mc"),rapidity_nbins,rapidity_minimum,rapidity_maximum, pt_nbins,pt_min,pt_max, "y","p_{T}","N");
  
}

  void ParticlePair3DHistos::loadDerivedFrom(const Folder & inputFile)
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  h_n2_DeltaPs = importHistogram(inputFile,createName(_histogramBaseName,"n2_DeltaPs"));
  h_n2_DeltaPo = importHistogram(inputFile,createName(_histogramBaseName,"n2_DeltaPo"));
  h_n2_DeltaPl = importHistogram(inputFile,createName(_histogramBaseName,"n2_DeltaPl"));
  
  h_n1n1_Qinv   = importHistogram(inputFile,createName(_histogramBaseName,"n1n1_Qinv"));
  h_n1n1_DeltaP = importHistogram(inputFile,createName(_histogramBaseName,"n1n1_DeltaP"));
  h_n1n1_DeltaPs= importHistogram(inputFile,createName(_histogramBaseName,"n1n1_DeltaPs"));
  h_n1n1_DeltaPo= importHistogram(inputFile,createName(_histogramBaseName,"n1n1_DeltaPo"));
  h_n1n1_DeltaPl= importHistogram(inputFile,createName(_histogramBaseName,"n1n1_DeltaPl"));
  
  h_c2_Qinv   = importHistogram(inputFile,createName(_histogramBaseName,"c2_Qinv"));
  h_c2_DeltaP = importHistogram(inputFile,createName(_histogramBaseName,"c2_DeltaP"));
  h_c2_DeltaPs= importHistogram(inputFile,createName(_histogramBaseName,"c2_DeltaPs"));
  h_c2_DeltaPo= importHistogram(inputFile,createName(_histogramBaseName,"c2_DeltaPo"));
  h_c2_DeltaPl= importHistogram(inputFile,createName(_histogramBaseName,"c2_DeltaPl"));
  
  h_r2_Qinv   = importHistogram(inputFile,createName(_histogramBaseName,"r2_Qinv"));
  h_r2_DeltaP = importHistogram(inputFile,createName(_histogramBaseName,"r2_DeltaP"));
  h_r2_DeltaPs= importHistogram(inputFile,createName(_histogramBaseName,"r2_DeltaPs"));
  h_r2_DeltaPo= importHistogram(inputFile,createName(_histogramBaseName,"r2_DeltaPo"));
  h_r2_DeltaPl= importHistogram(inputFile,createName(_histogramBaseName,"r2_DeltaPl"));
  
  h_a12_Qinv      = importHistogram(inputFile,createName(_histogramBaseName,"A12_Qinv"));
  h_a12_DeltaPs   = importHistogram(inputFile,createName(_histogramBaseName,"A12_DeltaPs"));
  h_a12_DeltaPo   = importHistogram(inputFile,createName(_histogramBaseName,"A12_DeltaPo"));
  h_a12_DeltaPl   = importHistogram(inputFile,createName(_histogramBaseName,"A12_DeltaPl"));
  h_a21_Qinv      = importHistogram(inputFile,createName(_histogramBaseName,"A21_Qinv"));
  h_a21_DeltaPs   = importHistogram(inputFile,createName(_histogramBaseName,"A21_DeltaPs"));
  h_a21_DeltaPo   = importHistogram(inputFile,createName(_histogramBaseName,"A21_DeltaPo"));
  h_a21_DeltaPl   = importHistogram(inputFile,createName(_histogramBaseName,"A21_DeltaPl"));

  h_n1_1_pt_mc  = importHistogram(inputFile,createName(_histogramBaseName,"n1_1_pt_mc"));
  h_n1_2_pt_mc  = importHistogram(inputFile,createName(_histogramBaseName,"n1_2_pt_mc"));
  h_n1_1_y_mc   = importHistogram(inputFile,createName(_histogramBaseName,"n1_1_y_mc"));
  h_n1_2_y_mc   = importHistogram(inputFile,createName(_histogramBaseName,"n1_2_y_mc"));


  h_n1_1_ptY_mc  = importHistogram(inputFile,createName(_histogramBaseName,"n1_1_ptY_mc"));
  h_n1_2_ptY_mc  = importHistogram(inputFile,createName(_histogramBaseName,"n1_2_ptY_mc"));
  h_n1r_1_ptY_mc = importHistogram(inputFile,createName(_histogramBaseName,"n1r_1_ptY_mc"));
  h_n1r_2_ptY_mc = importHistogram(inputFile,createName(_histogramBaseName,"n1r_2_ptY_mc"));
  
}

  //! The histograms must be normalized "per event" before calling this function.
  void ParticlePair3DHistos::calculateDerived(const ParticleSingleHistos & part1Histos, const ParticleSingleHistos & part2Histos, bool same)
  {
  if (!hasDerived()) throw Exception("The derived histograms were not created (createDerived())",__FUNCTION__);
  if (!part1Histos.h_n1_ptY || !part2Histos.h_n1_ptY)
    throw Exception("The single-particle histograms have no (y, pT) histogram (FillPtvsY)",__FUNCTION__);
  if (reportDebug(__FUNCTION__)) { printCR(); }

  // to calculate n1n1, we fold n1 by n1 using an MC technique.
  double y1, pt1, mt1, phi1;
  double y2, pt2, mt2, phi2;
  double pt,s,Mlong,roots;
  double weight=1.0;
  double pa[4], pb[4], ptot[4],q[4];
  //  const double m1 = 0.13957039;
  //  const double m2 = 0.13957039;
  const double g[4]={1.0,-1.0,-1.0,-1.0};
  double qinv, qside, qlong, qout;

  // copies: the bin 0 (no particle) of the multiplicity distributions is replaced below for their mean
  Histogram part1_n1Copy(*part1Histos.h_n1);
  Histogram part2_n1Copy(*part2Histos.h_n1);
  Histogram * part1_n1 = &part1_n1Copy;
  Histogram * part2_n1 = &part2_n1Copy;

  Histogram * part1_ptY = part1Histos.h_n1_ptY;
  Histogram * part2_ptY = part2Histos.h_n1_ptY;

  double v1 = part1_n1->binValueAt(1);
  part1_n1->setBinValueAt(unsigned(0),v1);
  double v2 = part2_n1->binValueAt(1);
  part2_n1->setBinValueAt(unsigned(0),v2);
  double nParticle1_mean = part1_n1->mean();
  double nParticle2_mean = part2_n1->mean();
  double nParticle1 = part1_ptY->integral();
  double nParticle2 = part2_ptY->integral();
  printValue("nParticle1 (integral)",nParticle1);
  printValue("nParticle1_mean",nParticle1_mean);

  printValue("nParticle2 (integral)",nParticle1);
  printValue("nParticle2_mean",nParticle2_mean);

  int nPart1 = nParticle1;
  int nPart2 = nParticle2;
  double scale = 1.0/double(nMcSimEvents);
  printValue("nMcSimEvents",nMcSimEvents);
  printValue("scale",scale);

// projections
  Histogram* hTemp;
  hTemp = new Histogram(part1_ptY->project2DOnto1D(0));
  h_n1_1_y->add(*hTemp);
  delete hTemp;

  hTemp = new Histogram(part2_ptY->project2DOnto1D(0));
  h_n1_2_y->add(*hTemp);
  delete hTemp;

  hTemp = new Histogram(part1_ptY->project2DOnto1D(1));
  h_n1_1_pt->add(*hTemp);
  delete hTemp;

  hTemp = new Histogram(part2_ptY->project2DOnto1D(1));
  h_n1_2_pt->add(*hTemp);
  delete hTemp;


  vector<double> evPhi1;
  vector<double> evPt1;
  vector<double> evY1;
  vector<double> evPhi2;
  vector<double> evPt2;
  vector<double> evY2;

  //nMcSimEvents = 10000;
  for (long iEvent=0; iEvent<nMcSimEvents; iEvent++)
    {
    // create event
    evPhi1.clear();
    evPt1.clear();
    evY1.clear();
    evPhi2.clear();
    evPt2.clear();
    evY2.clear();

    for (int iParticle1=0; iParticle1<nPart1; iParticle1++)
      {
      phi1 = CAP::MATH::twoPi() * gRandom->Rndm();
      part1_ptY->random(y1,pt1);
      h_n1_1_pt_mc->fill(pt1);
      h_n1_1_y_mc->fill(y1);
      h_n1_1_ptY_mc->fill(y1,pt1,1.0);
      evPhi1.push_back(phi1);
      evPt1.push_back(pt1);
      evY1.push_back(y1);
      }
    if (!same)
      {
      for (int iParticle2=0; iParticle2<nPart2; iParticle2++)
        {
        phi2 = CAP::MATH::twoPi() * gRandom->Rndm();
        part2_ptY->random(y2,pt2);
        h_n1_2_pt_mc->fill(pt2);
        h_n1_2_y_mc->fill(y2);
        h_n1_2_ptY_mc->fill(y2,pt2,1.0);
        evPhi2.push_back(phi2);
        evPt2.push_back(pt2);
        evY2.push_back(y2);
        }
      }
    else
      {
      evPhi2 = evPhi1;
      evPt2  = evPt1;
      evY2   = evY1;
      }

    for (unsigned long iParticle1=0; iParticle1<evPhi1.size(); iParticle1++)
      {
      phi1 = evPhi1[iParticle1];
      pt1  = evPt1[iParticle1];
      y1   = evY1[iParticle1];
      mt1  = std::sqrt(mass1*mass1+pt1*pt1);
      pa[0] = mt1*std::cosh(y1);
      pa[1] = pt1*std::cos(phi1);
      pa[2] = pt1*std::sin(phi1);
      pa[3] = mt1*std::sinh(y1);

      for (unsigned long iParticle2=0; iParticle2<evPhi2.size(); iParticle2++)
        {
        if (same  &&  iParticle1==iParticle2) continue;
        phi2 = evPhi2[iParticle2];
        pt2  = evPt2[iParticle2];
        y2   = evY2[iParticle2];
        mt2  = std::sqrt(mass2*mass2+pt2*pt2);
        pb[0] = mt2*std::cosh(y2);
        pb[1] = pt2*std::cos(phi2);
        pb[2] = pt2*std::sin(phi2);
        pb[3] = mt2*std::sinh(y2);
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
        h_n1n1_Qinv->fill(qinv,weight);
        h_n1n1_DeltaP->fill(qside,qout,qlong,weight);
        }
      }
    }

  h_n1_1_pt_mc->scale(scale);
  h_n1_1_y_mc->scale(scale);
  h_n1_2_pt_mc->scale(scale);
  h_n1_2_y_mc->scale(scale);
  h_n1_1_ptY_mc->scale(scale);
  h_n1_2_ptY_mc->scale(scale);

  double nParticle1_mc = h_n1_1_pt_mc->integral();
  double nParticle2_mc = h_n1_2_pt_mc->integral();
  double tweak1;
  double tweak2;

  tweak1= nParticle1/nParticle1_mc;
  if (same)
    {
    nParticle2_mc = nParticle1_mc;
    tweak2 = tweak1;
    h_n1_2_y_mc->add(*h_n1_1_y_mc);
    h_n1_2_pt_mc->add(*h_n1_1_pt_mc);
    h_n1_2_ptY_mc->add(*h_n1_1_ptY_mc);
    }
  else
    tweak2= nParticle2/nParticle2_mc;

  printValue("nPart1",nPart1);
  printValue("nParticle1",nParticle1);
  printValue("nParticle1_mc",nParticle1_mc);
  printValue("nPart2",nPart2);
  printValue("nParticle2",nParticle2);
  printValue("nParticle2_mc",nParticle2_mc);
  printValue("tweak1",tweak1);
  printValue("tweak2",tweak2);

  h_n1_1_pt_mc->scale(tweak1);
  h_n1_1_y_mc->scale(tweak1);
  h_n1_1_ptY_mc->scale(tweak1);

  h_n1_2_pt_mc->scale(tweak2);
  h_n1_2_y_mc->scale(tweak2);
  h_n1_2_ptY_mc->scale(tweak2);

  h_n1r_1_pt_mc->setAsRatioOf(*h_n1_1_pt_mc,*h_n1_1_pt);
  h_n1r_1_y_mc->setAsRatioOf(*h_n1_1_y_mc,*h_n1_1_y);
  h_n1r_1_ptY_mc->setAsRatioOf(*h_n1_1_ptY_mc,*part1_ptY);

  h_n1r_2_pt_mc->setAsRatioOf(*h_n1_2_pt_mc,*h_n1_2_pt);
  h_n1r_2_y_mc->setAsRatioOf(*h_n1_2_y_mc,*h_n1_2_y);
  h_n1r_2_ptY_mc->setAsRatioOf(*h_n1_2_ptY_mc,*part2_ptY);

  h_n1n1_Qinv->scale(scale);
  h_n1n1_DeltaP->scale(scale);

  double n2_yield   = h_n2_Qinv->binValueAt(4);
  double n1n1_yield = h_n1n1_Qinv->binValueAt(4);

  double n2_yield_err   = h_n2_Qinv->binUncertaintyAt(4);
  double n1n1_yield_err = h_n1n1_Qinv->binUncertaintyAt(4);
  double tweak = n2_yield/n1n1_yield;
  printValue("n2_yield",n2_yield);
  printValue("n1n1_yield",n1n1_yield);
  printValue("n2_yield/n1n1_yield",tweak);
  printValue("n2_yield_err",n2_yield_err);
  printValue("n1n1_yield_err",n1n1_yield_err);

  projectInto(h_n1n1_DeltaP,0,createName(_histogramBaseName,"n1n1_DeltaPs"));
  projectInto(h_n1n1_DeltaP,1,createName(_histogramBaseName,"n1n1_DeltaPo"));
  projectInto(h_n1n1_DeltaP,2,createName(_histogramBaseName,"n1n1_DeltaPl"));

  projectInto(h_n2_DeltaP,0,createName(_histogramBaseName,"n2_DeltaPs"));
  projectInto(h_n2_DeltaP,1,createName(_histogramBaseName,"n2_DeltaPo"));
  projectInto(h_n2_DeltaP,2,createName(_histogramBaseName,"n2_DeltaPl"));

  calculateC2_H1H1H1(h_n2_Qinv,h_n1n1_Qinv,h_c2_Qinv);
  calculateC2_H3H3H3(h_n2_DeltaP,h_n1n1_DeltaP,h_c2_DeltaP);
  projectInto(h_c2_DeltaP,0,createName(_histogramBaseName,"c2_DeltaPs"));
  projectInto(h_c2_DeltaP,1,createName(_histogramBaseName,"c2_DeltaPo"));
  projectInto(h_c2_DeltaP,2,createName(_histogramBaseName,"c2_DeltaPl"));

  calculateR2_H1H1H1(h_n2_Qinv,h_n1n1_Qinv,h_r2_Qinv);

  double r2_yield = h_r2_Qinv->binValueAt(4);
  double r2_yield_err = h_r2_Qinv->binUncertaintyAt(4);
  printValue("r2_yield",r2_yield);
  printValue("r2_yield_err",r2_yield_err);

  calculateR2_H3H3H3(h_n2_DeltaP,h_n1n1_DeltaP,h_r2_DeltaP);
  projectInto(h_r2_DeltaP,0,createName(_histogramBaseName,"r2_DeltaPs"));
  projectInto(h_r2_DeltaP,1,createName(_histogramBaseName,"r2_DeltaPo"));
  projectInto(h_r2_DeltaP,2,createName(_histogramBaseName,"r2_DeltaPl"));

  double invN1 = 1.0/nParticle1;
  double invN2 = 1.0/nParticle2;
  h_a12_Qinv    ->add(*h_c2_Qinv,invN2);
  h_a12_DeltaPs ->add(*h_c2_DeltaPs,invN2);
  h_a12_DeltaPo ->add(*h_c2_DeltaPo,invN2);
  h_a12_DeltaPl ->add(*h_c2_DeltaPl,invN2);
  h_a21_Qinv    ->add(*h_c2_Qinv,invN1);
  h_a21_DeltaPs ->add(*h_c2_DeltaPs,invN1);
  h_a21_DeltaPo ->add(*h_c2_DeltaPo,invN1);
  h_a21_DeltaPl ->add(*h_c2_DeltaPl,invN1);
}

  //! Set the masses used in the calculation of q_inv (default: pi+ mass).
  void ParticlePair3DHistos::setMasses(double m1, double m2)
  {
  mass1 = m1;
  mass2 = m2;
  }

  //!
  //! Project a rank 3 histogram onto one of its axes (all bins of the other axes) and store the result in the histogram of
  //! this group with the given name (replacing its content), or in a new histogram of the group if there is none.
  //!
  void ParticlePair3DHistos::projectInto(const Histogram * source, unsigned int axis, const String & targetName)
  {
  if (!source) throw NullPointerException("source",__FUNCTION__);
  Histogram projection = source->project3DOnto1D(axis);
  Histogram * target = histogram(targetName);
  if (target)
    {
    target->reset();
    target->add(projection);
    }
  else
    {
    projection.setName(targetName);
    append(new Histogram(projection));
    }
}

}  //  namespace CAP



