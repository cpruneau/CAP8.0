/* **********************************************************************
 * Copyright (C) 2019-2024, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "ParticlePairDerivedHistos.hpp"
#include "Task.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "Task.hpp"

ClassImp(CAP::ParticlePairDerivedHistos);

namespace CAP
{

ParticlePairDerivedHistos::ParticlePairDerivedHistos()
:
ParticlePairHistos(),
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
  appendClassName("ParticlePairDerivedHistos");
  setName("ParticlePairDerivedHistos");
  setTitle("ParticlePairDerivedHistos");
}

ParticlePairDerivedHistos::ParticlePairDerivedHistos(const ParticlePairDerivedHistos & source)
:
ParticlePairHistos(source),
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
  cloneD(source);
}

ParticlePairDerivedHistos & ParticlePairDerivedHistos::operator=(const ParticlePairDerivedHistos & rhs)
{
  if (this!=&rhs)
  {
    ParticlePairHistos::operator=(rhs);
    cloneD(rhs);
  }
  return *this;
}

void ParticlePairDerivedHistos::cloneD(const ParticlePairDerivedHistos & source)
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  ParticlePairHistos::cloneB(source);
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

void ParticlePairDerivedHistos::configure(const String & taskName,
                                                const String & objectType,
                                                const Configuration & configuration,
                                                unsigned int index)
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  ParticlePairHistos::configure(taskName,objectType,configuration,index);
}

void ParticlePairDerivedHistos::create()
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

void ParticlePairDerivedHistos::loadFrom(CapFile & inputFile)
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

void quickCopy(Histogram * source, Histogram * target)
{
  int sNx = source->nBins(0);
  int sNy = source->nBins(1);
  int tNx = target->nBins(0);
  int tNy = target->nBins(1);
  
  
  if (sNx!=tNx || sNy!=tNy)
  {
    String s = source->name();
    s += "-->";
    s += target->name();
    throw CAP::HistogramException(s,"sNx!=tNx || sNy!=tNy","ParticlePairDerivedHistos::quickCopy(Histogram * source, Histogram * target)");
  }
  for (unsigned int k=0; k<source->nBinsTotal(); k++)
    target->setBinAtGlobal(k,source->binEntriesAtGlobal(k),source->binValueAtGlobal(k),source->binUncertaintyAtGlobal(k));
}


// HistogramFolder from ParticlePairDerivedHistos must be normalized "per event" before calling this function
void ParticlePairDerivedHistos::
calculateDerivedHistograms(ParticleSingleDerivedHistos & part1Histos,
                           ParticleSingleDerivedHistos & part2Histos,
                           ParticlePairHistos          & pairHistos)
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
  
  if (!part1Histos.h_n1_pt )     throw HistogramException("LOGIC","!part1Histos.h_n1_pt",__FUNCTION__);
  if (!part2Histos.h_n1_pt )     throw HistogramException("LOGIC","!part2Histos.h_n1_pt",__FUNCTION__);
  if (!pairHistos.h_n2_ptpt )    throw HistogramException("LOGIC","!pairHistos.h_n2_ptpt",__FUNCTION__);
  if (!h_n1n1_ptpt )             throw HistogramException("LOGIC","!h_n1n1_ptpt",__FUNCTION__);
  if (!h_C2_ptpt )               throw HistogramException("LOGIC","!h_C2_ptpt",__FUNCTION__);
  if (!h_C2N_ptpt )              throw HistogramException("LOGIC","!h_C2N_ptpt",__FUNCTION__);
  if (!h_R2_ptpt )               throw HistogramException("LOGIC","!h_R2_ptpt",__FUNCTION__);
  
  yield2 = part2Histos.h_n1_pt->integral();
  calculateN1N1_H1H1H2( part1Histos.h_n1_pt,   part2Histos.h_n1_pt,  h_n1n1_ptpt,1.0, 1.0);
  h_C2_ptpt->setAsSumOf(*pairHistos.h_n2_ptpt,*h_n1n1_ptpt,1.0,-1.0);
  h_C2N_ptpt->add(*h_C2_ptpt);
  h_C2N_ptpt->scale(1.0/yield2);
  calculateR2_H2H2H2(pairHistos.h_n2_ptpt,h_n1n1_ptpt,h_R2_ptpt,0, 1.0, 1.0);
  
  //
  // phi vs phi
  //
  yield2 = part2Histos.h_n1_phi->integral();
  calculateN1N1_H1H1H2( part1Histos.h_n1_phi,  part2Histos.h_n1_phi, h_n1n1_phiphi,1.0, 1.0);
  h_C2_phiphi->setAsSumOf(*pairHistos.h_n2_phiphi,*h_n1n1_phiphi,1.0,-1.0);
  h_C2N_phiphi->add(*h_C2_phiphi);
  h_C2N_phiphi->scale(1.0/yield2);
  calculateR2_H2H2H2(pairHistos.h_n2_phiphi,h_n1n1_phiphi,h_R2_phiphi,0, 1.0, 1.0);
  
  if (p2_fill)
  {
    avgPt1Phi = avgValue(part1Histos.h_avg_pt_phi);
    avgPt2Phi = avgValue(part2Histos.h_avg_pt_phi);
    scalingFactorPhi = 1.0/avgPt1Phi/avgPt2Phi;
    h_P2_phiphi->setAsSumOf(*h_DptDpt_phiphi,*h_DptDpt_phiphi,1.0,0.0);
    h_P2_phiphi->scale(scalingFactorPhi);
  }
  
  
  if (eta_fill)
  {
    
    //
    // eta vs eta
    //
    yield2 = part2Histos.h_n1_eta->integral();
    calculateN1N1_H1H1H2(part1Histos.h_n1_eta,part2Histos.h_n1_eta,h_n1n1_etaeta,1.0, 1.0);
    h_C2_etaeta->setAsSumOf(*pairHistos.h_n2_etaeta,*h_n1n1_etaeta,1.0,-1.0);
    h_C2N_etaeta->add(*h_C2_etaeta);
    h_C2N_etaeta->scale(1.0/yield2);
    calculateR2_H2H2H2(pairHistos.h_n2_etaeta,h_n1n1_etaeta,h_R2_etaeta,0, 1.0, 1.0);
    
    //
    // Deta vs Dphi
    //
    reduce_n1EtaPhiN1EtaPhiOntoN1N1DetaDphi(part1Histos.h_n1_phiEta,part2Histos.h_n1_phiEta,h_n1n1_DetaDphi,Deta_nbins,Dphi_nbins);
    h_C2_DetaDphi->setAsSumOf(*pairHistos.h_n2_DetaDphi,*h_n1n1_DetaDphi,1.0,-1.0);
    h_C2N_DetaDphi->add(*h_C2_DetaDphi);
    h_C2N_DetaDphi->scale(1.0/yield2);
    calculateR2_H2H2H2(pairHistos.h_n2_DetaDphi,h_n1n1_DetaDphi,h_R2_DetaDphi,0,1.0,1.0);
    
    shiftY(*h_C2N_DetaDphi,*h_C2N_DetaDphi_shft,Dphi_nbins_shft);
    shiftY(*h_C2_DetaDphi,*h_C2_DetaDphi_shft,Dphi_nbins_shft);
    shiftY(*h_R2_DetaDphi,*h_R2_DetaDphi_shft,Dphi_nbins_shft);
    
    if (p2_fill)
    {
      avgPt1Eta = avgValue(part1Histos.h_avg_pt_eta);
      avgPt2Eta = avgValue(part2Histos.h_avg_pt_eta);
      scalingFactorEta = 1.0/avgPt1Eta/avgPt2Eta;
      h_P2_etaeta->setAsSumOf(*h_DptDpt_etaeta,*h_DptDpt_etaeta,1.0,0.0);
      h_P2_etaeta->scale(scalingFactorEta);
      shiftY(*h_DptDpt_DetaDphi, *h_DptDpt_DetaDphi_shft,Dphi_nbins_shft);
      
      /// calculate average pt base on h_avg_pt_eta histograms
      h_P2_DetaDphi->setAsSumOf(*h_DptDpt_DetaDphi,*h_DptDpt_DetaDphi,1.0,0.0);
      h_P2_DetaDphi->scale(scalingFactorEta);
      //symmetrizeDeltaEtaDeltaPhi(h_P2_DetaDphi,ijNormalization);
      shiftY(*h_P2_DetaDphi,*h_P2_DetaDphi_shft, Dphi_nbins_shft);

      calculateG2_H2H2H2H2( h_DptDpt_etaeta,   h_n1n1_etaeta,   h_pt1pt1_etaeta,   h_G2_etaeta,   ijNormalization, bincorrection, 1.0);
      calculateG2_H2H2H2H2( h_DptDpt_phiphi,   h_n1n1_phiphi,   h_pt1pt1_phiphi,   h_G2_phiphi,   ijNormalization, bincorrection, 1.0);
      calculateG2_H2H2H2H2( h_DptDpt_DetaDphi, h_n1n1_DetaDphi, h_pt1pt1_DetaDphi, h_G2_DetaDphi, ijNormalization, bincorrection, 1.0);
      shiftY(*h_G2_DetaDphi,*h_G2_DetaDphi_shft,Dphi_nbins_shft);
    }
  }
  
  if (rapidity_fill)
  {
    //
    // y vs y
    //
    yield2 = part2Histos.h_n1_y->integral();
    calculateN1N1_H1H1H2(part1Histos.h_n1_y,part2Histos.h_n1_y,h_n1n1_yY,1.0, 1.0);
    h_C2_yY->setAsSumOf(*pairHistos.h_n2_yY,*h_n1n1_yY,1.0,-1.0);
    h_C2N_yY->add(*h_C2_yY);
    h_C2N_yY->scale(1.0/yield2);
    calculateR2_H2H2H2(pairHistos.h_n2_yY,h_n1n1_yY,h_R2_yY,0, 1.0, 1.0);
    
    //
    // Dy vs Dphi
    //
    //if (reportDebug(__FUNCTION__))  cout << "Calculate XXX_DyDphi" << endl;
    reduce_n1EtaPhiN1EtaPhiOntoN1N1DetaDphi(part1Histos.h_n1_phiY,part2Histos.h_n1_phiY,h_n1n1_DyDphi,Drapidity_nbins,Dphi_nbins);
    h_C2_DyDphi->setAsSumOf(*pairHistos.h_n2_DyDphi,*h_n1n1_DyDphi,1.0,-1.0);
    h_C2N_DyDphi->add(*h_C2_DyDphi);
    h_C2N_DyDphi->scale(1.0/yield2);
    calculateR2_H2H2H2(pairHistos.h_n2_DyDphi,h_n1n1_DyDphi,h_R2_DyDphi,0,1.0,1.0);
    
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
      avgPt1Y = avgValue(part1Histos.h_avg_pt_y);
      avgPt2Y = avgValue(part2Histos.h_avg_pt_y);
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
      calculateG2_H2H2H2H2( h_DptDpt_yY,       h_n1n1_yY,       h_pt1pt1_yY,       h_G2_yY,       ijNormalization, bincorrection, 1.0);
      calculateG2_H2H2H2H2( h_DptDpt_phiphi,   h_n1n1_phiphi,   h_pt1pt1_phiphi,   h_G2_phiphi,   ijNormalization, bincorrection, 1.0);
      calculateG2_H2H2H2H2( h_DptDpt_DetaDphi, h_n1n1_DetaDphi, h_pt1pt1_DetaDphi, h_G2_DetaDphi, ijNormalization, bincorrection, 1.0);
      shiftY(*h_G2_DetaDphi,*h_G2_DetaDphi_shft,Dphi_nbins_shft);
    }
  }
}

void ParticlePairDerivedHistos::calculateOmegaFactor(Histogram * source, vector<double> & omegaFactor)
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


void ParticlePairDerivedHistos::calculateAverageYbar(Histogram * source,
                                                     Histogram * target,
                                                     vector<double> & omegaFactor)
{
  //  cout << "calculateAverageYbar(Histogram *source,Histogram*target,vector<double>&omegaFactor)" << endl;
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
