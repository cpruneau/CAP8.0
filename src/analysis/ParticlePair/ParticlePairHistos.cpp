/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "ParticlePairHistos.hpp"
#include "MathBasicFunctions.hpp"
#include "Histogram.hpp"
#include "MathConstants.hpp"
#include "PrintHelpers.hpp"
#include "Task.hpp"
#include "Configuration.hpp"
#include "NameHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"


namespace CAP
{
  ParticlePairHistos::ParticlePairHistos()
  :
  HistogramFolder(),
  eta_fill(false),
  rapidity_fill(false),
  p2_fill(false),
  n2_nbins(100),
  pt_nbins(100),
  phi_nbins(72),
  eta_nbins(200),
  rapidity_nbins(200),
  Dphi_nbins(0),
  Deta_nbins(0),
  Drapidity_nbins(0),
  Dphi_nbins_shft(0),
  n2_min(0.0),
  n2_max(10.0),
  pt_min(0.0),
  pt_max(10.0),
  phi_min(0),
  phi_max(2.0*3.14159265359),
  eta_min(-5.0),
  eta_max(5.0),
  rapidity_minimum(-10.0),
  rapidity_maximum(10.0),
  phi_scale(0),
  rapidity_scale(0),
  eta_scale(0),
  pt_scale(0),
  Dphi_width(0),
  Dphi_min(0),
  Dphi_max(0),
  Deta_min(0),
  Deta_max(0),
  Drapidity_min(0),
  Drapidity_max(0),
  Dphi_min_shft(0),
  Dphi_max_shft(0),
  rapidity_maximumAcc(0),
  h_n2(nullptr),
  h_n2_ptpt(nullptr),
  h_n2_etaeta(nullptr),
  h_ptpt_etaeta(nullptr),
  h_n2_phiphi(nullptr),
  h_ptpt_phiphi(nullptr),
  h_n2_yY(nullptr),
  h_ptpt_yY(nullptr),
  h_n2_DetaDphi(nullptr),
  h_ptpt_DetaDphi(nullptr),
  h_n2_DyDphi(nullptr),
  h_ptpt_DyDphi(nullptr),
  h_n1n1_ptpt(nullptr),
  h_C2_ptpt(nullptr),
  h_C2N_ptpt(nullptr),
  h_R2_ptpt(nullptr),
  h_n1n1_phiphi(nullptr),
  h_C2_phiphi(nullptr),
  h_C2N_phiphi(nullptr),
  h_R2_phiphi(nullptr),
  h_P2_phiphi(nullptr),
  h_G2_phiphi(nullptr),
  h_DptDpt_phiphi(nullptr),
  h_pt1pt1_phiphi(nullptr),
  h_n1n1_etaeta(nullptr),
  h_C2_etaeta(nullptr),
  h_C2N_etaeta(nullptr),
  h_R2_etaeta(nullptr),
  h_G2_etaeta(nullptr),
  h_P2_etaeta(nullptr),
  h_DptDpt_etaeta(nullptr),
  h_pt1pt1_etaeta(nullptr),
  h_n1n1_yY(nullptr),
  h_C2_yY(nullptr),
  h_C2N_yY(nullptr),
  h_R2_yY(nullptr),
  h_G2_yY(nullptr),
  h_P2_yY(nullptr),
  h_DptDpt_yY(nullptr),
  h_pt1pt1_yY(nullptr),
  h_n1n1_DetaDphi(nullptr),
  h_C2_DetaDphi(nullptr),
  h_C2N_DetaDphi(nullptr),
  h_C2O_DetaDphi(nullptr),
  h_R2_DetaDphi(nullptr),
  h_P2_DetaDphi(nullptr),
  h_G2_DetaDphi(nullptr),
  h_DptDpt_DetaDphi(nullptr),
  h_pt1pt1_DetaDphi(nullptr),
  h_n1n1_DetaDphi_shft(nullptr),
  h_C2_DetaDphi_shft(nullptr),
  h_C2N_DetaDphi_shft(nullptr),
  h_C2O_DetaDphi_shft(nullptr),
  h_R2_DetaDphi_shft(nullptr),
  h_P2_DetaDphi_shft(nullptr),
  h_G2_DetaDphi_shft(nullptr),
  h_DptDpt_DetaDphi_shft(nullptr),
  h_n1n1_DyDphi(nullptr),
  h_C2_DyDphi(nullptr),
  h_C2N_DyDphi(nullptr),
  h_C2O_DyDphi(nullptr),
  h_R2_DyDphi(nullptr),
  h_P2_DyDphi(nullptr),
  h_G2_DyDphi(nullptr),
  h_DptDpt_DyDphi(nullptr),
  h_pt1pt1_DyDphi(nullptr),
  h_n1n1_DyDphi_shft(nullptr),
  h_C2_DyDphi_shft(nullptr),
  h_C2N_DyDphi_shft(nullptr),
  h_C2O_DyDphi_shft(nullptr),
  h_R2_DyDphi_shft(nullptr),
  h_P2_DyDphi_shft(nullptr),
  h_G2_DyDphi_shft(nullptr),
  h_DptDpt_DyDphi_shft(nullptr)
  {
  appendClassName("ParticlePairHistos");
  setName("ParticlePairHistos");
  setTitle("ParticlePairHistos");
  }

  ParticlePairHistos::ParticlePairHistos(const ParticlePairHistos & source)
  :
  HistogramFolder(),
  eta_fill(source.eta_fill),
  rapidity_fill(source.rapidity_fill),
  p2_fill(source.p2_fill),
  n2_nbins(source.n2_nbins),
  pt_nbins(source.pt_nbins),
  phi_nbins(source.phi_nbins),
  eta_nbins(source.eta_nbins),
  rapidity_nbins(source.rapidity_nbins),
  Dphi_nbins(source.Dphi_nbins),
  Deta_nbins(source.Deta_nbins),
  Drapidity_nbins(source.Drapidity_nbins),
  Dphi_nbins_shft(source.Dphi_nbins_shft),
  n2_min(source.n2_min),
  n2_max(source.n2_max),
  pt_min(source.pt_min),
  pt_max(source.pt_max),
  phi_min(source.phi_min),
  phi_max(source.phi_max),
  eta_min(source.eta_min),
  eta_max(source.eta_max),
  rapidity_minimum(source.rapidity_minimum),
  rapidity_maximum(source.rapidity_maximum),
  phi_scale(source.phi_scale),
  rapidity_scale(source.rapidity_scale),
  eta_scale(source.eta_scale),
  pt_scale(source.pt_scale),
  Dphi_width(source.Dphi_width),
  Dphi_min(source.Dphi_min),
  Dphi_max(source.Dphi_max),
  Deta_min(source.Deta_min),
  Deta_max(source.Deta_max),
  Drapidity_min(source.Drapidity_min),
  Drapidity_max(source.Drapidity_max),
  Dphi_min_shft(source.Dphi_min_shft),
  Dphi_max_shft(source.Dphi_max_shft),
  rapidity_maximumAcc(source.rapidity_maximumAcc),
  h_n2(nullptr),
  h_n2_ptpt(nullptr),
  h_n2_etaeta(nullptr),
  h_ptpt_etaeta(nullptr),
  h_n2_phiphi(nullptr),
  h_ptpt_phiphi(nullptr),
  h_n2_yY(nullptr),
  h_ptpt_yY(nullptr),
  h_n2_DetaDphi(nullptr),
  h_ptpt_DetaDphi(nullptr),
  h_n2_DyDphi(nullptr),
  h_ptpt_DyDphi(nullptr),
  h_n1n1_ptpt(nullptr),
  h_C2_ptpt(nullptr),
  h_C2N_ptpt(nullptr),
  h_R2_ptpt(nullptr),
  h_n1n1_phiphi(nullptr),
  h_C2_phiphi(nullptr),
  h_C2N_phiphi(nullptr),
  h_R2_phiphi(nullptr),
  h_P2_phiphi(nullptr),
  h_G2_phiphi(nullptr),
  h_DptDpt_phiphi(nullptr),
  h_pt1pt1_phiphi(nullptr),
  h_n1n1_etaeta(nullptr),
  h_C2_etaeta(nullptr),
  h_C2N_etaeta(nullptr),
  h_R2_etaeta(nullptr),
  h_G2_etaeta(nullptr),
  h_P2_etaeta(nullptr),
  h_DptDpt_etaeta(nullptr),
  h_pt1pt1_etaeta(nullptr),
  h_n1n1_yY(nullptr),
  h_C2_yY(nullptr),
  h_C2N_yY(nullptr),
  h_R2_yY(nullptr),
  h_G2_yY(nullptr),
  h_P2_yY(nullptr),
  h_DptDpt_yY(nullptr),
  h_pt1pt1_yY(nullptr),
  h_n1n1_DetaDphi(nullptr),
  h_C2_DetaDphi(nullptr),
  h_C2N_DetaDphi(nullptr),
  h_C2O_DetaDphi(nullptr),
  h_R2_DetaDphi(nullptr),
  h_P2_DetaDphi(nullptr),
  h_G2_DetaDphi(nullptr),
  h_DptDpt_DetaDphi(nullptr),
  h_pt1pt1_DetaDphi(nullptr),
  h_n1n1_DetaDphi_shft(nullptr),
  h_C2_DetaDphi_shft(nullptr),
  h_C2N_DetaDphi_shft(nullptr),
  h_C2O_DetaDphi_shft(nullptr),
  h_R2_DetaDphi_shft(nullptr),
  h_P2_DetaDphi_shft(nullptr),
  h_G2_DetaDphi_shft(nullptr),
  h_DptDpt_DetaDphi_shft(nullptr),
  h_n1n1_DyDphi(nullptr),
  h_C2_DyDphi(nullptr),
  h_C2N_DyDphi(nullptr),
  h_C2O_DyDphi(nullptr),
  h_R2_DyDphi(nullptr),
  h_P2_DyDphi(nullptr),
  h_G2_DyDphi(nullptr),
  h_DptDpt_DyDphi(nullptr),
  h_pt1pt1_DyDphi(nullptr),
  h_n1n1_DyDphi_shft(nullptr),
  h_C2_DyDphi_shft(nullptr),
  h_C2N_DyDphi_shft(nullptr),
  h_C2O_DyDphi_shft(nullptr),
  h_R2_DyDphi_shft(nullptr),
  h_P2_DyDphi_shft(nullptr),
  h_G2_DyDphi_shft(nullptr),
  h_DptDpt_DyDphi_shft(nullptr)
  {
  cloneB(source);
  }

  ParticlePairHistos & ParticlePairHistos::operator=(const ParticlePairHistos & source)
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  if (this!=&source)
    {
    HistogramFolder::operator=(source);
    eta_fill      =  source.eta_fill;
    rapidity_fill =  source.rapidity_fill;
    p2_fill       =  source.p2_fill;
    n2_nbins      =  source.n2_nbins;
    pt_nbins      =  source.pt_nbins;
    phi_nbins     =  source.phi_nbins;
    eta_nbins     =  source.eta_nbins;
    rapidity_nbins     =  source.rapidity_nbins;
    Dphi_nbins  =  source.Dphi_nbins;
    Deta_nbins  =  source.Deta_nbins;
    Drapidity_nbins    =  source.Drapidity_nbins;
    Dphi_nbins_shft = source.Dphi_nbins_shft;
    n2_min      =  source.n2_min;
    n2_max      =  source.n2_max;
    pt_min      =  source.pt_min;
    pt_max      =  source.pt_max;
    phi_min     =  source.phi_min;
    phi_max     =  source.phi_max;
    eta_min     =  source.eta_min;
    eta_max     =  source.eta_max;
    rapidity_minimum       =  source.rapidity_minimum;
    rapidity_maximum       =  source.rapidity_maximum;
    phi_scale   =  source.phi_scale;
    rapidity_scale     =  source.rapidity_scale;
    eta_scale   =  source.eta_scale;
    pt_scale    =  source.pt_scale;
    Dphi_width  =  source.Dphi_width;
    Dphi_min    =  source.Dphi_min;
    Dphi_max    =  source.Dphi_max;
    Deta_min    =  source.Deta_min;
    Deta_max    =  source.Deta_max;
    Drapidity_min       =  source.Drapidity_min;
    Drapidity_max       =  source.Drapidity_max;
    Dphi_min_shft       =  source.Dphi_min_shft;
    Dphi_max_shft       =  source.Dphi_max_shft;
    rapidity_maximumAcc =  source.rapidity_maximumAcc;
    cloneB(source);
    }
  return *this;
  }

  void ParticlePairHistos::cloneB(const ParticlePairHistos & source)
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  h_n2              = histogramLike(source.h_n2);
  h_n2_ptpt         = histogramLike(source.h_n2_ptpt);
  h_n2_etaeta       = histogramLike(source.h_n2_etaeta);
  h_ptpt_etaeta   = histogramLike(source.h_ptpt_etaeta);
  h_n2_phiphi       = histogramLike(source.h_n2_phiphi);
  h_ptpt_phiphi   = histogramLike(source.h_ptpt_phiphi);
  h_n2_yY           = histogramLike(source.h_n2_yY);
  h_ptpt_yY       = histogramLike(source.h_ptpt_yY);
  h_n2_DetaDphi     = histogramLike(source.h_n2_DetaDphi);
  h_ptpt_DetaDphi = histogramLike(source.h_ptpt_DetaDphi);
  h_n2_DyDphi       = histogramLike(source.h_n2_DyDphi);
  h_ptpt_DyDphi   = histogramLike(source.h_ptpt_DyDphi);

  if (reportDebug(__FUNCTION__)) { printCR(); }
  h_n1n1_ptpt  = histogramLike(source.h_n1n1_ptpt);
  h_C2_ptpt    = histogramLike(source.h_C2_ptpt);
  h_C2N_ptpt   = histogramLike(source.h_C2N_ptpt);
  h_R2_ptpt    = histogramLike(source.h_R2_ptpt);
  
  h_n1n1_phiphi  = histogramLike(source.h_n1n1_phiphi);
  h_C2_phiphi    = histogramLike(source.h_C2_phiphi);
  h_C2N_phiphi   = histogramLike(source.h_C2N_phiphi);
  h_R2_phiphi    = histogramLike(source.h_R2_phiphi);
  h_P2_phiphi    = histogramLike(source.h_P2_phiphi);
  h_G2_phiphi    = histogramLike(source.h_G2_phiphi);
  h_DptDpt_phiphi  = histogramLike(source.h_DptDpt_phiphi);
  h_pt1pt1_phiphi  = histogramLike(source.h_pt1pt1_phiphi);
  
  h_n1n1_etaeta    = histogramLike(source.h_n1n1_etaeta);
  h_C2_etaeta      = histogramLike(source.h_C2_etaeta);
  h_C2N_etaeta     = histogramLike(source.h_C2N_etaeta);
  h_R2_etaeta      = histogramLike(source.h_R2_etaeta);
  h_G2_etaeta      = histogramLike(source.h_G2_etaeta);
  h_P2_etaeta      = histogramLike(source.h_P2_etaeta);
  h_DptDpt_etaeta  = histogramLike(source.h_DptDpt_etaeta);
  h_pt1pt1_etaeta  = histogramLike(source.h_pt1pt1_etaeta);
  
  h_n1n1_yY    = histogramLike(source.h_n1n1_yY);
  h_C2_yY      = histogramLike(source.h_C2_yY);
  h_C2N_yY     = histogramLike(source.h_C2N_yY);
  h_R2_yY      = histogramLike(source.h_R2_yY);
  h_G2_yY      = histogramLike(source.h_G2_yY);
  h_P2_yY      = histogramLike(source.h_P2_yY);
  h_DptDpt_yY  = histogramLike(source.h_DptDpt_yY);
  h_pt1pt1_yY  = histogramLike(source.h_pt1pt1_yY);
  
  h_n1n1_DetaDphi    = histogramLike(source.h_n1n1_DetaDphi);
  h_C2_DetaDphi      = histogramLike(source.h_C2_DetaDphi);
  h_C2N_DetaDphi     = histogramLike(source.h_C2N_DetaDphi);
  h_C2O_DetaDphi     = histogramLike(source.h_C2O_DetaDphi);
  h_R2_DetaDphi      = histogramLike(source.h_R2_DetaDphi);
  h_P2_DetaDphi      = histogramLike(source.h_P2_DetaDphi);
  h_G2_DetaDphi      = histogramLike(source.h_G2_DetaDphi);
  h_DptDpt_DetaDphi  = histogramLike(source.h_DptDpt_DetaDphi);
  h_pt1pt1_DetaDphi  = histogramLike(source.h_pt1pt1_DetaDphi);
  
  h_C2_DetaDphi_shft  = histogramLike(source.h_C2_DetaDphi_shft);
  h_C2N_DetaDphi_shft = histogramLike(source.h_C2N_DetaDphi_shft);
  h_C2O_DetaDphi_shft = histogramLike(source.h_C2O_DetaDphi_shft);
  h_R2_DetaDphi_shft  = histogramLike(source.h_R2_DetaDphi_shft);
  h_P2_DetaDphi_shft  = histogramLike(source.h_P2_DetaDphi_shft);
  h_G2_DetaDphi_shft  = histogramLike(source.h_G2_DetaDphi_shft);
  h_DptDpt_DetaDphi_shft  = histogramLike(source.h_DptDpt_DetaDphi_shft);
  
  h_n1n1_DyDphi  = histogramLike(source.h_n1n1_DyDphi);
  h_C2_DyDphi    = histogramLike(source.h_C2_DyDphi);
  h_C2N_DyDphi   = histogramLike(source.h_C2N_DyDphi);
  h_C2O_DyDphi   = histogramLike(source.h_C2O_DyDphi);
  h_R2_DyDphi    = histogramLike(source.h_R2_DyDphi);
  h_P2_DyDphi    = histogramLike(source.h_P2_DyDphi);
  h_G2_DyDphi    = histogramLike(source.h_G2_DyDphi);
  h_DptDpt_DyDphi  = histogramLike(source.h_DptDpt_DyDphi);
  
  h_n1n1_DyDphi_shft   = histogramLike(source.h_n1n1_DyDphi_shft);
  h_C2_DyDphi_shft     = histogramLike(source.h_C2_DyDphi_shft);
  h_C2N_DyDphi_shft    = histogramLike(source.h_C2N_DyDphi_shft);
  h_C2O_DyDphi_shft    = histogramLike(source.h_C2O_DyDphi_shft);
  h_R2_DyDphi_shft     = histogramLike(source.h_R2_DyDphi_shft);
  h_P2_DyDphi_shft     = histogramLike(source.h_P2_DyDphi_shft);
  h_G2_DyDphi_shft     = histogramLike(source.h_G2_DyDphi_shft);
  h_DptDpt_DyDphi_shft = histogramLike(source.h_DptDpt_DyDphi_shft);
  }

  void ParticlePairHistos::addDefaultConfiguration(Configuration & configuration, const String & taskName)
  {
  ParticleSingleHistos::addDefaultConfiguration(configuration,taskName);
  const String type = "HISTOGRAM";
  configuration.addProperty(createKey(taskName,type,"eta_fill"),       false);
  configuration.addProperty(createKey(taskName,type,"rapidity_fill"),  true);
  configuration.addProperty(createKey(taskName,type,"p2_fill"),        false);
  configuration.addProperty(createKey(taskName,type,"FillPid"),        false);
  configuration.addProperty(createKey(taskName,type,"n1_nbins"),       100);
  configuration.addProperty(createKey(taskName,type,"n1_min"),         0.0);
  configuration.addProperty(createKey(taskName,type,"n1_max"),         100.0);
  configuration.addProperty(createKey(taskName,type,"etot_nbins"),     100);
  configuration.addProperty(createKey(taskName,type,"etot_min"),       0.0);
  configuration.addProperty(createKey(taskName,type,"etot_max"),       100.0);
  configuration.addProperty(createKey(taskName,type,"pt_nbins"),       18);
  configuration.addProperty(createKey(taskName,type,"pt_min"),         0.20);
  configuration.addProperty(createKey(taskName,type,"pt_max"),         2.00);
  configuration.addProperty(createKey(taskName,type,"phi_nbins"),      72);
  configuration.addProperty(createKey(taskName,type,"phi_min"),        0.0);
  configuration.addProperty(createKey(taskName,type,"phi_max"),        CAP::MATH::twoPi());
  configuration.addProperty(createKey(taskName,type,"eta_nbins"),      20);
  configuration.addProperty(createKey(taskName,type,"eta_min"),        -1.0);
  configuration.addProperty(createKey(taskName,type,"eta_max"),        1.0);
  configuration.addProperty(createKey(taskName,type,"rapidity_nbins"), 20);
  configuration.addProperty(createKey(taskName,type,"rapidity_min"),    -1.0);
  configuration.addProperty(createKey(taskName,type,"rapidity_max"),     1.0);
  configuration.addProperty(createKey(taskName,type,"n2_nbins"),         100);
  configuration.addProperty(createKey(taskName,type,"n2_min"),           0.0);
  configuration.addProperty(createKey(taskName,type,"n2_max"),          1000.0);
  configuration.addProperty(createKey(taskName,type,"Dphi_nbins"),      36);
  configuration.addProperty(createKey(taskName,type,"Dphi_min"),        0.0);
  configuration.addProperty(createKey(taskName,type,"Dphi_max"),        CAP::MATH::twoPi());
  configuration.addProperty(createKey(taskName,type,"Dphi_width"),      CAP::MATH::twoPi());
  configuration.addProperty(createKey(taskName,type,"Dphi_nbins_shft"),  36);
  configuration.addProperty(createKey(taskName,type,"Dphi_min_shft"),   0.0);
  configuration.addProperty(createKey(taskName,type,"Dphi_max_shft"),   0.0);
  configuration.addProperty(createKey(taskName,type,"Deta_nbins"),       39);
  configuration.addProperty(createKey(taskName,type,"Deta_min"),       -2.0);
  configuration.addProperty(createKey(taskName,type,"Deta_max"),        2.0);
  configuration.addProperty(createKey(taskName,type,"Deta_width"),       4.0/39.0);
  configuration.addProperty(createKey(taskName,type,"Drapidity_nbins"), 39);
  configuration.addProperty(createKey(taskName,type,"Drapidity_min"),   -2.0);
  configuration.addProperty(createKey(taskName,type,"Drapidity_max"),    2.0);
  configuration.addProperty(createKey(taskName,type,"Drapidity_width"), 4.0/39.0);
  configuration.addProperty(createKey(taskName,type,"DeltaP_nbins"),     10);
  configuration.addProperty(createKey(taskName,type,"DeltaP_min"),       -4.0);
  configuration.addProperty(createKey(taskName,type,"DeltaP_max"),      4.0);
  }

  void ParticlePairHistos::configure(const String & taskName,
                                     const String & type,
                                     const Configuration & configuration,
                                     unsigned int index)
  {
  String objectType = "HISTOGRAM";
  HistogramFolder::configure(taskName,type,configuration,index);
  eta_fill         = configuration.valueBool(createKey(taskName,objectType,"eta_fill"));
  rapidity_fill    = configuration.valueBool(createKey(taskName,objectType,"rapidity_fill"));
  p2_fill          = configuration.valueBool(createKey(taskName,objectType,"p2_fill"));
  n2_nbins         = configuration.valueInt( createKey(taskName,objectType,"n2_nbins"));
  n2_min           = configuration.valueDouble(createKey(taskName,objectType,"n2_min"));
  n2_max           = configuration.valueDouble(createKey(taskName,objectType,"n2_max"));
  pt_nbins         = configuration.valueInt(createKey(taskName,objectType,"pt_nbins"));
  pt_min           = configuration.valueDouble(createKey(taskName,objectType,"pt_min"));
  pt_max           = configuration.valueDouble(createKey(taskName,objectType,"pt_max"));
  phi_nbins        = configuration.valueInt(createKey(taskName,objectType,"phi_nbins"));
  phi_min          = configuration.valueDouble(createKey(taskName,objectType,"phi_min"));
  phi_max          = configuration.valueDouble(createKey(taskName,objectType,"phi_max"));
  eta_nbins        = configuration.valueInt(createKey(taskName,objectType,"eta_nbins"));
  eta_min          = configuration.valueDouble(createKey(taskName,objectType,"eta_min"));
  eta_max          = configuration.valueDouble(createKey(taskName,objectType,"eta_max"));
  rapidity_nbins   = configuration.valueInt(createKey(taskName,objectType,"rapidity_nbins"));
  rapidity_minimum = configuration.valueDouble(createKey(taskName,objectType,"rapidity_min"));
  rapidity_maximum = configuration.valueDouble(createKey(taskName,objectType,"rapidity_max"));
  phi_scale        = double(phi_nbins)/(phi_max-phi_min);
  rapidity_scale   = double(rapidity_nbins)/(rapidity_maximum-rapidity_minimum);
  eta_scale        = double(eta_nbins)/(eta_max-eta_min);
  pt_scale         = double(pt_nbins)/(pt_max-pt_min);

  Dphi_nbins  = phi_nbins;
  Deta_nbins  = 2*eta_nbins-1;
  Drapidity_nbins    = 2*rapidity_nbins-1;
  Dphi_width  = CAP::MATH::twoPi()/double(Dphi_nbins);
  Dphi_min    = -Dphi_width/2.0;
  Dphi_max    = CAP::MATH::twoPi() - Dphi_width/2.;
  Deta_min    = -(eta_max-eta_min);
  Deta_max    = eta_max-eta_min;
  Drapidity_min      = -(rapidity_maximum-rapidity_minimum);
  Drapidity_max      = rapidity_maximum-rapidity_minimum;

  Dphi_nbins_shft = phi_nbins/4;
  Dphi_min_shft   = Dphi_min - CAP::MATH::twoPi()/4.0;
  Dphi_max_shft   = Dphi_max - CAP::MATH::twoPi()/4.0;
  rapidity_maximumAcc = 4.0;

  if (reportInfo(__FUNCTION__))
    {
    printCR();
    printValue("eta_fill",eta_fill);
    printValue("rapidity_fill",rapidity_fill);
    printValue("p2_fill",p2_fill);
    printValue("n2_nbins",n2_nbins);
    printValue("n2_min",n2_min);
    printValue("n2_max",n2_max);
    printValue("pt_nbins",pt_nbins);
    printValue("pt_min",pt_min);
    printValue("pt_max",pt_max);
    printValue("pt_scale",pt_scale);
    printValue("phi_nbins",phi_nbins);
    printValue("phi_min",phi_min);
    printValue("phi_max",phi_max);
    printValue("phi_scale",phi_scale);
    printValue("eta_nbins",eta_nbins);
    printValue("eta_min",eta_min);
    printValue("eta_max",eta_max);
    printValue("eta_scale",eta_scale);
    printValue("rapidity_nbins",rapidity_nbins);
    printValue("rapidity_min",rapidity_minimum);
    printValue("rapidity_max",rapidity_maximum);
    printValue("rapidity_scale",rapidity_scale);
    printValue("Dphi_nbins",phi_nbins);
    printValue("Dphi_min",Dphi_min);
    printValue("Dphi_max",Dphi_max);
    printValue("Dphi_width",Dphi_width);
    printValue("Deta_nbins",eta_nbins);
    printValue("Deta_min",Deta_min);
    printValue("Deta_max",Deta_max);
    printValue("Drapidity_nbins",Drapidity_nbins);
    printValue("Drapidity_min",Drapidity_min);
    printValue("Drapidity_max",Drapidity_max);
    }
  }

  void ParticlePairHistos::create()
  {
  if (reportInfo(__FUNCTION__)) { printCR();}
  h_n2          = createHistogram(createName(_histogramBaseName,"n2"),         n2_nbins,  n2_min,  n2_max, "n_{2}", "Yield");
  h_n2_ptpt     = createHistogram(createName(_histogramBaseName,"n2_ptpt"),    pt_nbins,  pt_min,  pt_max, pt_nbins, pt_min, pt_max,   "p_{T,1}",  "p_{T,2}", "N_{2}");
  if (!h_n2_ptpt) throw Exception("!h_n2_ptpt",__FUNCTION__);


  h_n2_phiphi   = createHistogram(createName(_histogramBaseName,"n2_phiphi"),  phi_nbins, phi_min, phi_max, phi_nbins, phi_min, phi_max, "#varphi_{1}", "#varphi_{2}", "N_{2}");

  if (p2_fill)
    {
    h_ptpt_phiphi = createHistogram(createName(_histogramBaseName,"ptpt_phiphi"),   phi_nbins, phi_min, phi_max, phi_nbins, phi_min, phi_max, "#varphi_{1}", "#varphi_{2}", "p_{T}xp_{T}");
    }

  if (eta_fill)
    {
    h_n2_etaeta   = createHistogram(createName(_histogramBaseName,"n2_etaeta"),   eta_nbins,  eta_min, eta_max, eta_nbins, eta_min, eta_max, "#eta_{1}", "#eta_{2}", "N_{2}");
    h_n2_DetaDphi = createHistogram(createName(_histogramBaseName,"n2_DetaDphi"), Deta_nbins, Deta_min, Deta_max, Dphi_nbins, Dphi_min, Dphi_max, "#Delta#eta", "#Delta#phi", "N_{2}");
    if (p2_fill)
      {
      h_ptpt_etaeta = createHistogram(createName(_histogramBaseName,"ptpt_etaeta"), eta_nbins, eta_min, eta_max, eta_nbins, eta_min, eta_max, "#eta_{1}", "#eta_{2}", "p_{T}xp_{T}");
      h_ptpt_DetaDphi = createHistogram(createName(_histogramBaseName,"ptpt_DetaDphi"),Deta_nbins, Deta_min, Deta_max, Dphi_nbins, Dphi_min, Dphi_max, "#Delta#eta", "#Delta#phi", "ptpt");
      }
    }

  if (rapidity_fill)
    {
    h_n2_yY     = createHistogram(createName(_histogramBaseName,"n2_yY"),     rapidity_nbins,  rapidity_minimum,  rapidity_maximum,  rapidity_nbins, rapidity_minimum, rapidity_maximum, "y_{1}","y_{2}", "N_{2}");
    h_n2_DyDphi = createHistogram(createName(_histogramBaseName,"n2_DyDphi"), Drapidity_nbins, Drapidity_min, Drapidity_max, Dphi_nbins, Dphi_min, Dphi_max, "#Delta y", "#Delta#phi", "N_{2}");
    if (p2_fill)
      {
      h_ptpt_yY    = createHistogram(createName(_histogramBaseName,"ptpt_yY"),  rapidity_nbins,  rapidity_minimum, rapidity_maximum, rapidity_nbins, rapidity_minimum, rapidity_maximum, "y_{1}","y_{2}", "p_{T}xp_{T}");
      h_ptpt_DyDphi = createHistogram(createName(_histogramBaseName,"ptpt_DyDphi"),Drapidity_nbins, Drapidity_min, Drapidity_max, Dphi_nbins, Dphi_min, Dphi_max, "#Delta y", "#Delta#phi", "ptpt");
      }
    }
  }

  void ParticlePairHistos::loadFrom(const Folder & inputFile)
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  h_n2          = importHistogram(inputFile,createName(_histogramBaseName,"n2"));
  h_n2_ptpt     = importHistogram(inputFile,createName(_histogramBaseName,"n2_ptpt"));
  h_n2_phiphi   = importHistogram(inputFile,createName(_histogramBaseName,"n2_phiphi"));

  if (p2_fill)
    {
    h_ptpt_phiphi = importHistogram(inputFile,createName(_histogramBaseName,"ptpt_phiphi"));
    }

  if (eta_fill)
    {
    h_n2_etaeta   = importHistogram(inputFile,createName(_histogramBaseName,"n2_etaeta"));
    h_n2_DetaDphi = importHistogram(inputFile,createName(_histogramBaseName,"n2_DetaDphi"));
    if (p2_fill)
      {
      h_ptpt_etaeta   = importHistogram(inputFile,createName(_histogramBaseName,"ptpt_etaeta"));
      h_ptpt_DetaDphi = importHistogram(inputFile,createName(_histogramBaseName,"ptpt_DetaDphi"));
      }
    }
  if (rapidity_fill)
    {
    h_n2_yY     = importHistogram(inputFile,createName(_histogramBaseName,"n2_yY"));
    h_n2_DyDphi = importHistogram(inputFile,createName(_histogramBaseName,"n2_DyDphi"));
    if (p2_fill)
      {
      h_ptpt_yY     = importHistogram(inputFile,createName(_histogramBaseName,"ptpt_yY"));
      h_ptpt_DyDphi = importHistogram(inputFile,createName(_histogramBaseName,"ptpt_DyDphi"));
      }
    }
    // derived histograms: present once the derived histograms were computed
  if (inputFile.contains(createName(_histogramBaseName,"n1n1_phiphi"))) loadDerivedFrom(inputFile);
  }


  void ParticlePairHistos::fill(Particle & particle1, Particle & particle2, double weight)
  {
  int iPhi1, iPt1, iEta1, iY1;
  int iPhi2, iPt2, iEta2, iY2;

  VectorLorentz & momentum1 = particle1.momentum();
  double pt1   = momentum1.perp();  iPt1 = ptBinFor(pt1);
  double phi1  = momentum1.phi();   if (phi1<0.0) phi1 += CAP::MATH::twoPi(); iPhi1 = phiBinFor(phi1);
  double eta1  = momentum1.pseudorapidity(); iEta1 = eta_fill      ? getEtaBinFor(eta1) : 0;
  double y1    = momentum1.rapidity();       iY1   = rapidity_fill ? getYBinFor(y1)     : 0;
  //  printValue("ParticlePairHistos::fill() eta_fill",eta_fill);
  //  printValue("ParticlePairHistos::fill() rapidity_fill",rapidity_fill);
  //  printValue("ParticlePairHistos::fill() pt1",pt1);
  //  printValue("ParticlePairHistos::fill() phi1",phi1);
  //  printValue("ParticlePairHistos::fill() eta1",eta1);
  //  printValue("ParticlePairHistos::fill() y1",y1);
  //  printValue("ParticlePairHistos::fill() iPt1",iPt1);
  //  printValue("ParticlePairHistos::fill() iEta1",iEta1);
  //  printValue("ParticlePairHistos::fill() iY1",iY1);
  //  printValue("ParticlePairHistos::fill() iPhi1",iPhi1);
  VectorLorentz & momentum2 = particle2.momentum();
  double pt2   = momentum2.perp();  iPt2 = ptBinFor(pt2);
  double phi2  = momentum2.phi();   if (phi2<0.0) phi2 += CAP::MATH::twoPi(); iPhi2 = phiBinFor(phi2);
  double eta2  = momentum2.pseudorapidity(); iEta2 = eta_fill      ? getEtaBinFor(eta2) : 0;
  double y2    = momentum2.rapidity();       iY2   = rapidity_fill ? getYBinFor(y2)     : 0;

  // bin indices from the *BinFor() functions start at 1; 0 means out of range
  if (iPt1==0  || iPt2==0)  return;
  if (iPhi1==0 || iPhi2==0) return;
  if (iEta1==0  &&  iY1==0) return;
  if (iEta2==0  &&  iY2==0) return;
  int iDeltaEta  = iEta1-iEta2 + eta_nbins-1;
  int iDeltaY    = iY1-iY2 + rapidity_nbins-1;
  int iDeltaPhi  = iPhi1-iPhi2;
  if (iDeltaPhi < 0) iDeltaPhi += phi_nbins;
  if (!h_n2_ptpt) throw Exception("!h_n2_ptpt",__FUNCTION__);
  if (!h_n2_phiphi) throw Exception("!h_n2_phiphi",__FUNCTION__);
  double ptpt = weight*pt1*pt2;
  h_n2_ptpt  ->fillBin(iPt1-1,  iPt2-1,  weight);
  h_n2_phiphi->fillBin(iPhi1-1, iPhi2-1, weight);
  if (p2_fill) h_ptpt_phiphi->fillBin(iPhi1-1,iPhi2-1,ptpt);

  if (eta_fill  &&  iEta1!=0  &&  iEta2!=0 )
    {
    h_n2_etaeta  ->fillBin(iEta1-1,  iEta2-1,   weight);
    h_n2_DetaDphi->fillBin(iDeltaEta,iDeltaPhi, weight);
    if (p2_fill)
      {
      h_ptpt_etaeta  ->fillBin(iEta1-1,  iEta2-1,   ptpt);
      h_ptpt_DetaDphi->fillBin(iDeltaEta,iDeltaPhi, ptpt);
      }
    }

  if (rapidity_fill  &&  iY1!=0  &&  iY2!=0 )
    {
    h_n2_yY    ->fillBin(iY1-1,  iY2-1,   weight);
    h_n2_DyDphi->fillBin(iDeltaY,iDeltaPhi, weight);
    if (p2_fill)
      {
      h_ptpt_yY    ->fillBin(iY1-1,  iY2-1,   ptpt);
      h_ptpt_DyDphi->fillBin(iDeltaY,iDeltaPhi, ptpt);
      }
    }
  }

  void ParticlePairHistos::fillMultiplicity(double nPairs, double weight)
  {
  h_n2->fill(nPairs, weight);
  }

  void ParticlePairHistos::scaleObject(double factor)
  {
  h_n2->scale(factor);
  h_n2_ptpt->scale(factor);
  h_n2_phiphi->scale(factor);
  if (p2_fill) h_ptpt_phiphi->scale(factor);
  if (eta_fill)
    {
    h_n2_etaeta->scale(factor);
    h_n2_DetaDphi->scale(factor);
    if (p2_fill)
      {
      h_ptpt_etaeta->scale(factor);
      h_ptpt_DetaDphi->scale(factor);
      }
    }
  if (rapidity_fill)
    {
    h_n2_yY->scale(factor);
    h_n2_DyDphi->scale(factor);
    if (p2_fill)
      {
      h_ptpt_yY->scale(factor);
      h_ptpt_DyDphi->scale(factor);
      }
    }
  }


  void ParticlePairHistos::createDerived()
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  h_n1n1_ptpt            = createHistogram(createName(_histogramBaseName,"n1n1_ptpt"),   pt_nbins,  pt_min,  pt_max,  pt_nbins,  pt_min,   pt_max,   "p_{T,1}", "p_{T,2}", "<n_{1}><n_{2}>");
  h_C2N_ptpt             = createHistogram(createName(_histogramBaseName,"C2N_ptpt"),     pt_nbins,  pt_min,  pt_max,  pt_nbins,  pt_min,   pt_max,   "p_{T,1}", "p_{T,2}", "A_{2}");
  h_C2_ptpt              = createHistogram(createName(_histogramBaseName,"C2_ptpt"),     pt_nbins,  pt_min,  pt_max,  pt_nbins,  pt_min,   pt_max,   "p_{T,1}", "p_{T,2}", "C_{2}");
  h_R2_ptpt              = createHistogram(createName(_histogramBaseName,"R2_ptpt"),     pt_nbins,  pt_min,  pt_max,  pt_nbins,  pt_min,   pt_max,   "p_{T,1}", "p_{T,2}", "R_{2}");
  
  h_n1n1_phiphi          = createHistogram(createName(_histogramBaseName,"n1n1_phiphi"), phi_nbins, phi_min, phi_max, phi_nbins, phi_min,  phi_max,  "#varphi_{1}", "#varphi_{2}", "<n_{1}><n_{2}>");
  h_C2N_phiphi           = createHistogram(createName(_histogramBaseName,"C2N_phiphi"),   phi_nbins, phi_min, phi_max, phi_nbins, phi_min,  phi_max,  "#varphi_{1}", "#varphi_{2}", "A_{2}");
  h_C2_phiphi            = createHistogram(createName(_histogramBaseName,"C2_phiphi"),   phi_nbins, phi_min, phi_max, phi_nbins, phi_min,  phi_max,  "#varphi_{1}", "#varphi_{2}", "C_{2}");
  h_R2_phiphi            = createHistogram(createName(_histogramBaseName,"R2_phiphi"),   phi_nbins, phi_min, phi_max, phi_nbins, phi_min,  phi_max,  "#varphi_{1}", "#varphi_{2}", "R_{2}");
  
  if (p2_fill)
  {
    h_pt1pt1_phiphi      = createHistogram(createName(_histogramBaseName,"pt1pt1_phiphi"), phi_nbins, phi_min, phi_max, phi_nbins, phi_min,  phi_max,  "#varphi_{1}","#varphi_{2}",   "pt1pt1");
    h_DptDpt_phiphi      = createHistogram(createName(_histogramBaseName,"DptDpt_phiphi"), phi_nbins, phi_min, phi_max, phi_nbins, phi_min,  phi_max,  "#varphi_{1}", "#varphi_{2}",   "<#Delta p_{T} #Delta p_{T}>");
    h_P2_phiphi          = createHistogram(createName(_histogramBaseName,"P2_phiphi"),     phi_nbins, phi_min, phi_max, phi_nbins, phi_min,  phi_max,  "#varphi_{1}", "#varphi_{2}",   "P_{2}");
    h_G2_phiphi          = createHistogram(createName(_histogramBaseName,"G2_phiphi"),     phi_nbins, phi_min, phi_max, phi_nbins, phi_min,  phi_max,  "#varphi_{1}", "#varphi_{2}",   "G_{2}");
  }
  
  if (eta_fill)
  {
    h_n1n1_etaeta        = createHistogram(createName(_histogramBaseName,"n1n1_etaeta"),    eta_nbins,   eta_min,   eta_max,   eta_nbins,   eta_min,  eta_max,  "#eta_{1}", "#eta_{2}", "<n_{1}><n_{2}>");
    h_C2N_etaeta         = createHistogram(createName(_histogramBaseName,"C2N_etaeta"),      eta_nbins,   eta_min,   eta_max,   eta_nbins,   eta_min,  eta_max,  "#eta_{1}", "#eta_{2}", "A_{2}");
    h_C2_etaeta          = createHistogram(createName(_histogramBaseName,"C2_etaeta"),      eta_nbins,   eta_min,   eta_max,   eta_nbins,   eta_min,  eta_max,  "#eta_{1}", "#eta_{2}", "C_{2}");
    h_R2_etaeta          = createHistogram(createName(_histogramBaseName,"R2_etaeta"),      eta_nbins,   eta_min,   eta_max,   eta_nbins,   eta_min,  eta_max,  "#eta_{1}", "#eta_{2}", "R_{2}");
    
    h_n1n1_DetaDphi      = createHistogram(createName(_histogramBaseName,"n1n1_DetaDphi"),  Deta_nbins, Deta_min, Deta_max, Dphi_nbins, Dphi_min, Dphi_max, "#Delta#eta","#Delta#varphi", "<n_{1}><n_{1}>");
    h_C2_DetaDphi        = createHistogram(createName(_histogramBaseName,"C2_DetaDphi"),    Deta_nbins, Deta_min, Deta_max, Dphi_nbins, Dphi_min, Dphi_max, "#Delta#eta","#Delta#varphi", "C_{2}");
    h_C2N_DetaDphi       = createHistogram(createName(_histogramBaseName,"C2N_DetaDphi"),    Deta_nbins, Deta_min, Deta_max, Dphi_nbins, Dphi_min, Dphi_max, "#Delta#eta","#Delta#varphi", "A_{2}");
    h_C2O_DetaDphi       = createHistogram(createName(_histogramBaseName,"C2O_DetaDphi"),    Deta_nbins, Deta_min, Deta_max, Dphi_nbins, Dphi_min, Dphi_max, "#Delta#eta","#Delta#varphi", "D_{2}");
    h_R2_DetaDphi        = createHistogram(createName(_histogramBaseName,"R2_DetaDphi"),    Deta_nbins, Deta_min, Deta_max, Dphi_nbins, Dphi_min, Dphi_max, "#Delta#eta","#Delta#varphi", "R_{2}");
    
    h_C2_DetaDphi_shft   = createHistogram(createName(_histogramBaseName,"C2_DetaDphi_shft"),    Deta_nbins,  Deta_min,  Deta_max,  Dphi_nbins,  Dphi_min_shft, Dphi_max_shft, "#Delta#eta","#Delta#varphi", "C_{2}");
    h_C2N_DetaDphi_shft  = createHistogram(createName(_histogramBaseName,"C2N_DetaDphi_shft"),    Deta_nbins,  Deta_min,  Deta_max,  Dphi_nbins,  Dphi_min_shft, Dphi_max_shft, "#Delta#eta","#Delta#varphi", "A_{2}");
    h_C2O_DetaDphi_shft  = createHistogram(createName(_histogramBaseName,"C2O_DetaDphi_shft"),    Deta_nbins,  Deta_min,  Deta_max,  Dphi_nbins,  Dphi_min_shft, Dphi_max_shft, "#Delta#eta","#Delta#varphi", "D_{2}");
    h_R2_DetaDphi_shft   = createHistogram(createName(_histogramBaseName,"R2_DetaDphi_shft"),    Deta_nbins,  Deta_min,  Deta_max,  Dphi_nbins,  Dphi_min_shft, Dphi_max_shft, "#Delta#eta","#Delta#varphi", "R_{2}");
    
    if (p2_fill)
    {
      h_pt1pt1_etaeta        = createHistogram(createName(_histogramBaseName,"pt1pt1_etaeta"),  eta_nbins,   eta_min,   eta_max,   eta_nbins,   eta_min,  eta_max,            "#eta_{1}", "#eta_{2}",      "pt1pt1");
      h_DptDpt_etaeta        = createHistogram(createName(_histogramBaseName,"DptDpt_etaeta"),  eta_nbins,   eta_min,   eta_max,   eta_nbins,   eta_min,  eta_max,            "#eta_{1}", "#eta_{2}",      "<#Delta p_{T} #Delta p_{T}>");
      h_P2_etaeta            = createHistogram(createName(_histogramBaseName,"P2_etaeta"),      eta_nbins,   eta_min,   eta_max,   eta_nbins,   eta_min,  eta_max,            "#eta_{1}", "#eta_{2}",      "P_{2}");
      h_G2_etaeta            = createHistogram(createName(_histogramBaseName,"G2_etaeta"),      eta_nbins,   eta_min,   eta_max,   eta_nbins,   eta_min,  eta_max,            "#eta_{1}", "#eta_{2}",         "G_{2}");
      
      h_pt1pt1_DetaDphi      = createHistogram(createName(_histogramBaseName,"pt1pt1_DetaDphi"),Deta_nbins,  Deta_min,  Deta_max,  Dphi_nbins,  Dphi_min,      Dphi_max,      "#Delta#eta", "#Delta#varphi", "pt1pt1");
      h_DptDpt_DetaDphi      = createHistogram(createName(_histogramBaseName,"DptDpt_DetaDphi"),Deta_nbins,  Deta_min,  Deta_max,  Dphi_nbins,  Dphi_min,      Dphi_max,      "#Delta#eta",  "#Delta#varphi", "<#Delta p_{T}#Delta p_{T}>");
      h_P2_DetaDphi          = createHistogram(createName(_histogramBaseName,"P2_DetaDphi"),    Deta_nbins,  Deta_min,  Deta_max,  Dphi_nbins,  Dphi_min,      Dphi_max,      "#Delta#eta",  "#Delta#varphi", "P_{2}");
      h_G2_DetaDphi          = createHistogram(createName(_histogramBaseName,"G2_DetaDphi"),    Deta_nbins,  Deta_min,  Deta_max,  Dphi_nbins,  Dphi_min,      Dphi_max,      "#Delta#eta","#Delta#varphi",   "G_{2}");
      h_DptDpt_DetaDphi_shft = createHistogram(createName(_histogramBaseName,"DptDpt_DetaDphi_shft"),Deta_nbins,  Deta_min,  Deta_max,  Dphi_nbins,  Dphi_min_shft, Dphi_max_shft, "#Delta#eta",  "#Delta#varphi", "<#Delta p_{T} #Delta p_{T}>");
      h_P2_DetaDphi_shft     = createHistogram(createName(_histogramBaseName,"P2_DetaDphi_shft"),    Deta_nbins,  Deta_min,  Deta_max,  Dphi_nbins,  Dphi_min_shft, Dphi_max_shft, "#Delta#eta",  "#Delta#varphi", "P_{2}");
      h_G2_DetaDphi_shft     = createHistogram(createName(_histogramBaseName,"G2_DetaDphi_shft"),    Deta_nbins,  Deta_min,  Deta_max,  Dphi_nbins,  Dphi_min_shft, Dphi_max_shft, "#Delta#eta", "#Delta#varphi",  "G_{2}");
    }
  }
  
  if (rapidity_fill)
  {
    h_n1n1_yY           = createHistogram(createName(_histogramBaseName,"n1n1_yY"),      rapidity_nbins,   rapidity_minimum,   rapidity_maximum,   rapidity_nbins,   rapidity_minimum,  rapidity_maximum,                 "y_{1}",     "y_{2}", "<n_{1}><n_{1}>");
    h_C2_yY             = createHistogram(createName(_histogramBaseName,"C2_yY"),        rapidity_nbins,   rapidity_minimum,   rapidity_maximum,   rapidity_nbins,   rapidity_minimum,  rapidity_maximum,                 "y_{1}",    "y_{2}", "C_{2}");
    h_C2N_yY            = createHistogram(createName(_histogramBaseName,"C2N_yY"),        rapidity_nbins,   rapidity_minimum,   rapidity_maximum,   rapidity_nbins,   rapidity_minimum,  rapidity_maximum,                 "y_{1}",    "y_{2}", "A_{2}");
    h_R2_yY             = createHistogram(createName(_histogramBaseName,"R2_yY"),        rapidity_nbins,   rapidity_minimum,   rapidity_maximum,   rapidity_nbins,   rapidity_minimum,  rapidity_maximum,                 "y_{1}",    "y_{2}", "R_{2}");
    
    h_n1n1_DyDphi       = createHistogram(createName(_histogramBaseName,"n1n1_DyDphi"),  Drapidity_nbins,  Drapidity_min,  Drapidity_max,  Dphi_nbins,  Dphi_min,      Dphi_max,     "#Delta y",  "#Delta#varphi", "<n_{1}><n_{1}>");
    h_C2_DyDphi         = createHistogram(createName(_histogramBaseName,"C2_DyDphi"),    Drapidity_nbins,  Drapidity_min,  Drapidity_max,  Dphi_nbins,  Dphi_min,      Dphi_max,     "#Delta y",  "#Delta#varphi", "C_{2}");
    h_C2N_DyDphi        = createHistogram(createName(_histogramBaseName,"C2N_DyDphi"),   Drapidity_nbins,  Drapidity_min,  Drapidity_max,  Dphi_nbins,  Dphi_min,      Dphi_max,     "#Delta y",  "#Delta#varphi", "A_{2}");
    h_C2O_DyDphi        = createHistogram(createName(_histogramBaseName,"C2O_DyDphi"),   Drapidity_nbins,  Drapidity_min,  Drapidity_max,  Dphi_nbins,  Dphi_min,      Dphi_max,     "#Delta y",  "#Delta#varphi", "D_{2}");
    h_R2_DyDphi         = createHistogram(createName(_histogramBaseName,"R2_DyDphi"),    Drapidity_nbins,  Drapidity_min,  Drapidity_max,  Dphi_nbins,  Dphi_min,      Dphi_max,     "#Delta y",  "#Delta#varphi", "R_{2}");
    
    h_C2_DyDphi_shft    = createHistogram(createName(_histogramBaseName,"C2_DyDphi_shft"),  Drapidity_nbins,  Drapidity_min,  Drapidity_max,  Dphi_nbins,  Dphi_min_shft, Dphi_max_shft, "#Delta y", "#Delta#varphi", "C_{2}");
    h_C2N_DyDphi_shft   = createHistogram(createName(_histogramBaseName,"C2N_DyDphi_shft"), Drapidity_nbins,  Drapidity_min,  Drapidity_max,  Dphi_nbins,  Dphi_min_shft, Dphi_max_shft, "#Delta y", "#Delta#varphi", "A_{2}");
    h_C2O_DyDphi_shft   = createHistogram(createName(_histogramBaseName,"C2O_DyDphi_shft"), Drapidity_nbins,  Drapidity_min,  Drapidity_max,  Dphi_nbins,  Dphi_min_shft, Dphi_max_shft, "#Delta y", "#Delta#varphi", "D_{2}");
    h_R2_DyDphi_shft    = createHistogram(createName(_histogramBaseName,"R2_DyDphi_shft"),  Drapidity_nbins,  Drapidity_min,  Drapidity_max,  Dphi_nbins,  Dphi_min_shft, Dphi_max_shft, "#Delta y", "#Delta#varphi", "R_{2}");
    
    if (p2_fill)
    {
      h_pt1pt1_yY         = createHistogram(createName(_histogramBaseName,"pt1pt1_yY"),    rapidity_nbins,   rapidity_minimum,   rapidity_maximum,   rapidity_nbins,   rapidity_minimum,  rapidity_maximum,                  "y_{1}",    "y_{2}", "pt1pt1");
      h_DptDpt_yY         = createHistogram(createName(_histogramBaseName,"DptDpt_yY"),    rapidity_nbins,   rapidity_minimum,   rapidity_maximum,   rapidity_nbins,   rapidity_minimum,  rapidity_maximum,                  "y_{1}",    "y_{2}", "<#Delta p_{T} #Delta p_{T}>");
      h_P2_yY             = createHistogram(createName(_histogramBaseName,"P2_yY"),        rapidity_nbins,   rapidity_minimum,   rapidity_maximum,   rapidity_nbins,     rapidity_minimum,         rapidity_maximum,         "y_{1}",    "y_{2}",         "P_{2}");
      h_G2_yY             = createHistogram(createName(_histogramBaseName,"G2_yY"),        rapidity_nbins,   rapidity_minimum,   rapidity_maximum,   rapidity_nbins,   rapidity_minimum,  rapidity_maximum,                  "y_{1}",    "y_{2}", "G_{2}");
      
      h_pt1pt1_DyDphi     = createHistogram(createName(_histogramBaseName,"pt1pt1_DyDphi"),Drapidity_nbins,  Drapidity_min,  Drapidity_max,  Dphi_nbins,  Dphi_min,      Dphi_max,      "#Delta y", "#Delta#varphi", "pt1pt1");
      h_DptDpt_DyDphi     = createHistogram(createName(_histogramBaseName,"DptDpt_DyDphi"),Drapidity_nbins,  Drapidity_min,  Drapidity_max,  Dphi_nbins,  Dphi_min,      Dphi_max,      "#Delta y", "#Delta#varphi", "<#Delta p_{T}#Delta p_{T}>");
      h_P2_DyDphi         = createHistogram(createName(_histogramBaseName,"P2_DyDphi"),    Drapidity_nbins,  Drapidity_min,  Drapidity_max,  Dphi_nbins,  Dphi_min,      Dphi_max,      "#Delta y", "#Delta#varphi", "P_{2}");
      h_G2_DyDphi         = createHistogram(createName(_histogramBaseName,"G2_DyDphi"),    Drapidity_nbins,  Drapidity_min,  Drapidity_max,  Dphi_nbins,  Dphi_min,      Dphi_max,      "#Delta y", "#Delta#varphi", "G_{2}");
      
      h_DptDpt_DyDphi_shft= createHistogram(createName(_histogramBaseName,"DptDpt_DyDphi_shft"),Drapidity_nbins,  Drapidity_min,  Drapidity_max,  Dphi_nbins,  Dphi_min_shft, Dphi_max_shft, "#Delta y", "#Delta#varphi", "<#Delta p_{T}#Delta p_{T}>");
      h_P2_DyDphi_shft    = createHistogram(createName(_histogramBaseName,"P2_DyDphi_shft"),    Drapidity_nbins,  Drapidity_min,  Drapidity_max,  Dphi_nbins,  Dphi_min_shft, Dphi_max_shft, "#Delta y", "#Delta#varphi", "P_{2}");
      h_G2_DyDphi_shft    = createHistogram(createName(_histogramBaseName,"G2_DyDphi_shft"),    Drapidity_nbins,  Drapidity_min,  Drapidity_max,  Dphi_nbins,  Dphi_min_shft, Dphi_max_shft, "#Delta y", "#Delta#varphi", "G_{2}");
    }
  }
}

  void ParticlePairHistos::loadDerivedFrom(const Folder & inputFile)
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  h_n1n1_phiphi          = importHistogram(inputFile, CAP::createName(_histogramBaseName,"n1n1_phiphi"));
  h_C2_phiphi            = importHistogram(inputFile, CAP::createName(_histogramBaseName,"C2_phiphi"));
  h_C2N_phiphi           = importHistogram(inputFile, CAP::createName(_histogramBaseName,"C2N_phiphi"));
  h_R2_phiphi            = importHistogram(inputFile, CAP::createName(_histogramBaseName,"R2_phiphi"));
  
  h_n1n1_ptpt            = importHistogram(inputFile, CAP::createName(_histogramBaseName,"n1n1_ptpt"));
  h_C2_ptpt              = importHistogram(inputFile, CAP::createName(_histogramBaseName,"C2_ptpt"));
  h_C2N_ptpt             = importHistogram(inputFile, CAP::createName(_histogramBaseName,"C2N_ptpt"));
  h_R2_ptpt              = importHistogram(inputFile, CAP::createName(_histogramBaseName,"R2_ptpt"));
  
  if (p2_fill)
  {
    h_pt1pt1_phiphi      = importHistogram(inputFile, CAP::createName(_histogramBaseName,"pt1pt1_phiphi"));
    h_DptDpt_phiphi      = importHistogram(inputFile, CAP::createName(_histogramBaseName,"DptDpt_phiphi"));
    h_P2_phiphi          = importHistogram(inputFile, CAP::createName(_histogramBaseName,"P2_phiphi"));
    h_G2_phiphi          = importHistogram(inputFile, CAP::createName(_histogramBaseName,"G2_phiphi"));
  }
  
  if (eta_fill)
  {
    h_n1n1_etaeta        = importHistogram(inputFile, CAP::createName(_histogramBaseName,"n1n1_etaeta"));
    h_C2_etaeta          = importHistogram(inputFile, CAP::createName(_histogramBaseName,"C2_etaeta"));
    h_C2N_etaeta         = importHistogram(inputFile, CAP::createName(_histogramBaseName,"C2N_etaeta"));
    h_R2_etaeta          = importHistogram(inputFile, CAP::createName(_histogramBaseName,"R2_etaeta"));
    
    h_n1n1_DetaDphi      = importHistogram(inputFile, CAP::createName(_histogramBaseName,"n1n1_DetaDphi_shft"));
    h_C2_DetaDphi        = importHistogram(inputFile, CAP::createName(_histogramBaseName,"C2_DetaDphi_shft"));
    h_C2N_DetaDphi       = importHistogram(inputFile, CAP::createName(_histogramBaseName,"C2N_DetaDphi_shft"));
    h_R2_DetaDphi        = importHistogram(inputFile, CAP::createName(_histogramBaseName,"R2_DetaDphi_shft"));
    
    h_C2_DetaDphi_shft   = importHistogram(inputFile, CAP::createName(_histogramBaseName,"C2_DetaDphi_shft"));
    h_C2N_DetaDphi_shft  = importHistogram(inputFile, CAP::createName(_histogramBaseName,"C2N_DetaDphi_shft"));
    h_R2_DetaDphi_shft   = importHistogram(inputFile, CAP::createName(_histogramBaseName,"R2_DetaDphi_shft"));
    
    if (p2_fill)
    {
      h_pt1pt1_etaeta        = importHistogram(inputFile, CAP::createName(_histogramBaseName,"pt1pt1_etaeta"));
      h_DptDpt_etaeta        = importHistogram(inputFile, CAP::createName(_histogramBaseName,"DptDpt_etaeta"));
      h_P2_etaeta            = importHistogram(inputFile, CAP::createName(_histogramBaseName,"P2_etaeta"));
      h_G2_etaeta            = importHistogram(inputFile, CAP::createName(_histogramBaseName,"G2_etaeta"));
      
      h_pt1pt1_DetaDphi      = importHistogram(inputFile, CAP::createName(_histogramBaseName,"pt1pt1_DetaDphi_shft"));
      h_DptDpt_DetaDphi      = importHistogram(inputFile, CAP::createName(_histogramBaseName,"DptDpt_DetaDphi_shft"));
      h_P2_DetaDphi          = importHistogram(inputFile, CAP::createName(_histogramBaseName,"P2_DetaDphi_shft"));
      h_G2_DetaDphi          = importHistogram(inputFile, CAP::createName(_histogramBaseName,"G2_DetaDphi_shft"));
      h_DptDpt_DetaDphi_shft = importHistogram(inputFile, CAP::createName(_histogramBaseName,"DptDpt_DetaDphi_shft"));
      h_P2_DetaDphi_shft     = importHistogram(inputFile, CAP::createName(_histogramBaseName,"P2_DetaDphi_shft"));
      h_G2_DetaDphi_shft     = importHistogram(inputFile, CAP::createName(_histogramBaseName,"G2_DetaDphi_shft"));
    }
  }
  
  if (rapidity_fill)
  {
    h_n1n1_yY           = importHistogram(inputFile, CAP::createName(_histogramBaseName,"n1n1_yY"));
    h_C2_yY             = importHistogram(inputFile, CAP::createName(_histogramBaseName,"C2_yY"));
    h_C2N_yY            = importHistogram(inputFile,CAP::createName(_histogramBaseName,"C2N_yY"));
    h_R2_yY             = importHistogram(inputFile, CAP::createName(_histogramBaseName,"R2_yY"));
    
    h_n1n1_DyDphi       = importHistogram(inputFile, CAP::createName(_histogramBaseName,"n1n1_DyDphi"));
    h_C2_DyDphi         = importHistogram(inputFile, CAP::createName(_histogramBaseName,"C2_DyDphi"));
    h_C2N_DyDphi        = importHistogram(inputFile, CAP::createName(_histogramBaseName,"C2N_DyDphi"));
    h_C2O_DyDphi        = importHistogram(inputFile, CAP::createName(_histogramBaseName,"C2O_DyDphi"));
    h_R2_DyDphi         = importHistogram(inputFile, CAP::createName(_histogramBaseName,"R2_DyDphi"));
    
    h_C2_DyDphi_shft    = importHistogram(inputFile, CAP::createName(_histogramBaseName,"C2_DyDphi_shft"));
    h_C2N_DyDphi_shft   = importHistogram(inputFile, CAP::createName(_histogramBaseName,"C2N_DyDphi_shft"));
    h_C2O_DyDphi_shft   = importHistogram(inputFile, CAP::createName(_histogramBaseName,"C2_DyDphi_shft"));
    h_R2_DyDphi_shft    = importHistogram(inputFile, CAP::createName(_histogramBaseName,"R2_DyDphi_shft"));
    
    if (p2_fill)
    {
      h_pt1pt1_yY         = importHistogram(inputFile, CAP::createName(_histogramBaseName,"pt1pt1_yY"));
      h_DptDpt_yY         = importHistogram(inputFile, CAP::createName(_histogramBaseName,"DptDpt_yY"));
      h_P2_yY             = importHistogram(inputFile, CAP::createName(_histogramBaseName,"P2_yY"));
      h_G2_yY             = importHistogram(inputFile, CAP::createName(_histogramBaseName,"G2_yY"));
      
      h_pt1pt1_DyDphi     = importHistogram(inputFile, CAP::createName(_histogramBaseName,"pt1pt1_DyDphi_shft"));
      h_DptDpt_DyDphi     = importHistogram(inputFile, CAP::createName(_histogramBaseName,"DptDpt_DyDphi_shft"));
      h_P2_DyDphi         = importHistogram(inputFile, CAP::createName(_histogramBaseName,"P2_DyDphi_shft"));
      h_G2_DyDphi         = importHistogram(inputFile, CAP::createName(_histogramBaseName,"G2_DyDphi_shft"));
      
      h_DptDpt_DyDphi_shft= importHistogram(inputFile, CAP::createName(_histogramBaseName,"DptDpt_DyDphi_shft"));
      h_P2_DyDphi_shft    = importHistogram(inputFile, CAP::createName(_histogramBaseName,"P2_DyDphi_shft"));
      h_G2_DyDphi_shft    = importHistogram(inputFile, CAP::createName(_histogramBaseName,"G2_DyDphi_shft"));
    }
  }
}

  //!
  //! The derived histograms are recomputed from scratch from the base histograms of this group (pair densities n2 and
  //! pT1 pT2 sums, per event) and those of its two particles (densities n1, pT sums and averages, per event).
  //!
  void ParticlePairHistos::calculateDerived(const ParticleSingleHistos & single1, const ParticleSingleHistos & single2)
  {
  double bincorrection = 1.0;
  double yield2;
  double avgPt1Phi;
  double avgPt2Phi;
  double scalingFactorPhi;
  double avgPt1Eta;
  double avgPt2Eta;
  double avgPt1Y;
  double avgPt2Y;
  double scalingFactorEta;
  double scalingFactorY;
  int ijNormalization = 0;
  
  if (!single1.h_n1_pt )     throw HistogramException("LOGIC","!single1.h_n1_pt",__FUNCTION__);
  if (!single2.h_n1_pt )     throw HistogramException("LOGIC","!single2.h_n1_pt",__FUNCTION__);
  if (!h_n2_ptpt )    throw HistogramException("LOGIC","!h_n2_ptpt",__FUNCTION__);
  if (!h_n1n1_ptpt )             throw HistogramException("LOGIC","!h_n1n1_ptpt",__FUNCTION__);
  if (!h_C2_ptpt )               throw HistogramException("LOGIC","!h_C2_ptpt",__FUNCTION__);
  if (!h_C2N_ptpt )              throw HistogramException("LOGIC","!h_C2N_ptpt",__FUNCTION__);
  if (!h_R2_ptpt )               throw HistogramException("LOGIC","!h_R2_ptpt",__FUNCTION__);
  
  yield2 = single2.h_n1_pt->integral();
  calculateN1N1_H1H1H2( single1.h_n1_pt,   single2.h_n1_pt,  h_n1n1_ptpt,1.0, 1.0);
  h_C2_ptpt->setAsSumOf(*h_n2_ptpt,*h_n1n1_ptpt,1.0,-1.0);
  h_C2N_ptpt->setAsSumOf(*h_C2_ptpt,*h_C2_ptpt,1.0,0.0);
  h_C2N_ptpt->scale(1.0/yield2);
  calculateR2_H2H2H2(h_n2_ptpt,h_n1n1_ptpt,h_R2_ptpt,0, 1.0, 1.0);
  
  //
  // phi vs phi
  //
  yield2 = single2.h_n1_phi->integral();
  calculateN1N1_H1H1H2( single1.h_n1_phi,  single2.h_n1_phi, h_n1n1_phiphi,1.0, 1.0);
  h_C2_phiphi->setAsSumOf(*h_n2_phiphi,*h_n1n1_phiphi,1.0,-1.0);
  h_C2N_phiphi->setAsSumOf(*h_C2_phiphi,*h_C2_phiphi,1.0,0.0);
  h_C2N_phiphi->scale(1.0/yield2);
  calculateR2_H2H2H2(h_n2_phiphi,h_n1n1_phiphi,h_R2_phiphi,0, 1.0, 1.0);
  
  if (p2_fill)
  {
    avgPt1Phi = avgValue(single1.h_avg_pt_phi);
    avgPt2Phi = avgValue(single2.h_avg_pt_phi);
    scalingFactorPhi = 1.0/avgPt1Phi/avgPt2Phi;
    h_P2_phiphi->setAsSumOf(*h_DptDpt_phiphi,*h_DptDpt_phiphi,1.0,0.0);
    h_P2_phiphi->scale(scalingFactorPhi);
  }
  
  
  if (eta_fill)
  {
    
    //
    // eta vs eta
    //
    yield2 = single2.h_n1_eta->integral();
    calculateN1N1_H1H1H2(single1.h_n1_eta,single2.h_n1_eta,h_n1n1_etaeta,1.0, 1.0);
    h_C2_etaeta->setAsSumOf(*h_n2_etaeta,*h_n1n1_etaeta,1.0,-1.0);
    h_C2N_etaeta->setAsSumOf(*h_C2_etaeta,*h_C2_etaeta,1.0,0.0);
    h_C2N_etaeta->scale(1.0/yield2);
    calculateR2_H2H2H2(h_n2_etaeta,h_n1n1_etaeta,h_R2_etaeta,0, 1.0, 1.0);
    
    //
    // Deta vs Dphi
    //
    reduce_n1EtaPhiN1EtaPhiOntoN1N1DetaDphi(single1.h_n1_phiEta,single2.h_n1_phiEta,h_n1n1_DetaDphi,Deta_nbins,Dphi_nbins);
    h_C2_DetaDphi->setAsSumOf(*h_n2_DetaDphi,*h_n1n1_DetaDphi,1.0,-1.0);
    h_C2N_DetaDphi->setAsSumOf(*h_C2_DetaDphi,*h_C2_DetaDphi,1.0,0.0);
    h_C2N_DetaDphi->scale(1.0/yield2);
    calculateR2_H2H2H2(h_n2_DetaDphi,h_n1n1_DetaDphi,h_R2_DetaDphi,0,1.0,1.0);
    
    shiftY(*h_C2N_DetaDphi,*h_C2N_DetaDphi_shft,Dphi_nbins_shft);
    shiftY(*h_C2_DetaDphi,*h_C2_DetaDphi_shft,Dphi_nbins_shft);
    shiftY(*h_R2_DetaDphi,*h_R2_DetaDphi_shft,Dphi_nbins_shft);
    
    if (p2_fill)
    {
      avgPt1Eta = avgValue(single1.h_avg_pt_eta);
      avgPt2Eta = avgValue(single2.h_avg_pt_eta);
      scalingFactorEta = 1.0/avgPt1Eta/avgPt2Eta;
      h_P2_etaeta->setAsSumOf(*h_DptDpt_etaeta,*h_DptDpt_etaeta,1.0,0.0);
      h_P2_etaeta->scale(scalingFactorEta);
      shiftY(*h_DptDpt_DetaDphi, *h_DptDpt_DetaDphi_shft,Dphi_nbins_shft);
      
      /// calculate average pt base on h_avg_pt_eta histograms
      h_P2_DetaDphi->setAsSumOf(*h_DptDpt_DetaDphi,*h_DptDpt_DetaDphi,1.0,0.0);
      h_P2_DetaDphi->scale(scalingFactorEta);
      //symmetrizeDeltaEtaDeltaPhi(h_P2_DetaDphi,ijNormalization);
      shiftY(*h_P2_DetaDphi,*h_P2_DetaDphi_shft, Dphi_nbins_shft);

      calculateN1N1_H1H1H2(single1.h_avg_pt_phi, single2.h_avg_pt_phi, h_pt1pt1_phiphi, 1.0, 1.0);
      calculateN1N1_H1H1H2(single1.h_avg_pt_eta, single2.h_avg_pt_eta, h_pt1pt1_etaeta, 1.0, 1.0);
      calculatePt1Pt1(single1.h_spt_phiEta, single2.h_spt_phiEta, h_n1n1_DetaDphi, h_pt1pt1_DetaDphi);
      calculateG2_H2H2H2H2( h_ptpt_etaeta,   h_n1n1_etaeta,   h_pt1pt1_etaeta,   h_G2_etaeta,   ijNormalization, bincorrection, 1.0);
      calculateG2_H2H2H2H2( h_ptpt_phiphi,   h_n1n1_phiphi,   h_pt1pt1_phiphi,   h_G2_phiphi,   ijNormalization, bincorrection, 1.0);
      calculateG2_H2H2H2H2( h_ptpt_DetaDphi, h_n1n1_DetaDphi, h_pt1pt1_DetaDphi, h_G2_DetaDphi, ijNormalization, bincorrection, 1.0);
      shiftY(*h_G2_DetaDphi,*h_G2_DetaDphi_shft,Dphi_nbins_shft);
    }
  }
  
  if (rapidity_fill)
  {
    //
    // y vs y
    //
    yield2 = single2.h_n1_y->integral();
    calculateN1N1_H1H1H2(single1.h_n1_y,single2.h_n1_y,h_n1n1_yY,1.0, 1.0);
    h_C2_yY->setAsSumOf(*h_n2_yY,*h_n1n1_yY,1.0,-1.0);
    h_C2N_yY->setAsSumOf(*h_C2_yY,*h_C2_yY,1.0,0.0);
    h_C2N_yY->scale(1.0/yield2);
    calculateR2_H2H2H2(h_n2_yY,h_n1n1_yY,h_R2_yY,0, 1.0, 1.0);
    
    //
    // Dy vs Dphi
    //
    //if (reportDebug(__FUNCTION__))  cout << "Calculate XXX_DyDphi" << endl;
    reduce_n1EtaPhiN1EtaPhiOntoN1N1DetaDphi(single1.h_n1_phiY,single2.h_n1_phiY,h_n1n1_DyDphi,Drapidity_nbins,Dphi_nbins);
    h_C2_DyDphi->setAsSumOf(*h_n2_DyDphi,*h_n1n1_DyDphi,1.0,-1.0);
    h_C2N_DyDphi->setAsSumOf(*h_C2_DyDphi,*h_C2_DyDphi,1.0,0.0);
    h_C2N_DyDphi->scale(1.0/yield2);
    calculateR2_H2H2H2(h_n2_DyDphi,h_n1n1_DyDphi,h_R2_DyDphi,0,1.0,1.0);
    
    vector<double> omegaFactor;
    omegaFactor.clear();
    calculateOmegaFactor(h_C2N_DyDphi, omegaFactor);
    calculateAverageYbar(h_C2N_DyDphi, h_C2O_DyDphi, omegaFactor);
    
    //if (reportDebug(__FUNCTION__))  cout <<  " shift XXX_DyDphi" << endl;
    shiftY(*h_C2_DyDphi,  *h_C2_DyDphi_shft,    Dphi_nbins_shft);
    shiftY(*h_C2N_DyDphi, *h_C2N_DyDphi_shft,   Dphi_nbins_shft);
    shiftY(*h_C2O_DyDphi, *h_C2O_DyDphi_shft,   Dphi_nbins_shft);
    shiftY(*h_R2_DyDphi,  *h_R2_DyDphi_shft,    Dphi_nbins_shft);
    
    if (p2_fill)
    {
      avgPt1Y = avgValue(single1.h_avg_pt_y);
      avgPt2Y = avgValue(single2.h_avg_pt_y);
      scalingFactorY = 1.0/avgPt1Y/avgPt2Y;
      h_P2_yY->setAsSumOf(*h_DptDpt_yY,*h_DptDpt_yY,1.0,0.0);
      h_P2_yY->scale(scalingFactorY);
      //if (reportDebug(__FUNCTION__))  cout <<  " shiftY(*h_R2_DyDphi,*h_R2_DyDphi_shft,Dphi_nbins_shft)" << endl;
      shiftY(*h_DptDpt_DyDphi, *h_DptDpt_DyDphi_shft,Dphi_nbins_shft);
      
      /// calculate average pt base on h_avg_pt_eta histograms
      h_P2_DyDphi->setAsSumOf(*h_DptDpt_DyDphi,*h_DptDpt_DyDphi,1.0,0.0);
      h_P2_DyDphi->scale(scalingFactorY);
      //symmetrizeDeltaEtaDeltaPhi(h_P2_DetaDphi,ijNormalization);
      shiftY(*h_P2_DyDphi,*h_P2_DyDphi_shft, Dphi_nbins_shft);
      calculateN1N1_H1H1H2(single1.h_avg_pt_phi, single2.h_avg_pt_phi, h_pt1pt1_phiphi, 1.0, 1.0);
      calculateN1N1_H1H1H2(single1.h_avg_pt_y, single2.h_avg_pt_y, h_pt1pt1_yY, 1.0, 1.0);
      calculatePt1Pt1(single1.h_spt_phiY, single2.h_spt_phiY, h_n1n1_DyDphi, h_pt1pt1_DyDphi);
      calculateG2_H2H2H2H2( h_ptpt_yY,       h_n1n1_yY,       h_pt1pt1_yY,       h_G2_yY,       ijNormalization, bincorrection, 1.0);
      calculateG2_H2H2H2H2( h_ptpt_phiphi,   h_n1n1_phiphi,   h_pt1pt1_phiphi,   h_G2_phiphi,   ijNormalization, bincorrection, 1.0);
      calculateG2_H2H2H2H2( h_ptpt_DyDphi,   h_n1n1_DyDphi,   h_pt1pt1_DyDphi,   h_G2_DyDphi,   ijNormalization, bincorrection, 1.0);
      shiftY(*h_G2_DyDphi,*h_G2_DyDphi_shft,Dphi_nbins_shft);
    }
  }
}

  void ParticlePairHistos::calculatePt1Pt1(const Histogram * spt1, const Histogram * spt2, const Histogram * n1n1, Histogram * target)
  {
  if (!spt1 || !spt2 || !n1n1 || !target) throw HistogramException("LOGIC","calculatePt1Pt1: missing histogram",__FUNCTION__);
  reduce_n1EtaPhiN1EtaPhiOntoN1N1DetaDphi(spt1,const_cast<Histogram*>(spt2),target,target->nBins(0),target->nBins(1));
  for (unsigned int i=0; i<target->nBins(0); i++)
    for (unsigned int j=0; j<target->nBins(1); j++)
      {
      double n = n1n1->binValueAt(i,j);
      target->setBinValueAt(i,j, n>0 ? target->binValueAt(i,j)/n : 0.0);
      target->setBinUncertaintyAt(i,j,0.0);
      }
  }

void ParticlePairHistos::calculateOmegaFactor(Histogram * source, std::vector<double> & omegaFactor)
{
  int nY  = source->nBins(0); // bins in delta rapidity
  double y0;
  y0 = 2.0*rapidity_maximumAcc;
  for (int iY=1; iY<=nY; iY++) // delta rapidity loop
  {
    double dy = source->binCenter((iY)-1,0);
    omegaFactor.push_back(1.0- absolute(dy/y0));
    //cout << " index : " << iY << "  dy: " << dy << " Omega: " << omegaFactor[iY] << endl;
  }
}

void ParticlePairHistos::calculateAverageYbar(Histogram * source,
                                                     Histogram * target,
                                                     std::vector<double> & omegaFactor)
{
  //  cout << "calculateAverageYbar(Histogram *source,Histogram*target,std::vector<double>&omegaFactor)" << endl;
  double scale = 1.0;
  int nX = source->nBins(0);
  int nY = source->nBins(1);
  for (int iX=1; iX<=nX; iX++)
  {
    scale = 1.0/omegaFactor[iX-1];
    //if (iX>180  &&  iX<220) cout << " iX: " << iX << " omegaFactor: " << omegaFactor[iX-1] << " scale:" << scale << endl;
    for (int iY=1; iY<=nY; iY++)
    {
      double v  =  source->binValueAt((iX)-1,(iY)-1);
      double ev =  source->binUncertaintyAt((iX)-1,(iY)-1);
      //if (iX>190  &&  iX<210) cout << " v: " << v << "+-" << ev << " v*scale:" << v*scale << "+-" << ev*scale << endl;
      target->setBinValueAt(unsigned((iX)-1),unsigned((iY)-1),v*scale);
      target->setBinUncertaintyAt(unsigned((iX)-1),unsigned((iY)-1),ev*scale);
    }
  }
}

} // namespace CAP
