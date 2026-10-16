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

#include "ParticleSingleHistos.hpp"
#include "ParticleSingleDerivedHistos.hpp"
#include "ParticlePairHistos.hpp"
#include "HistogramFolder.hpp"
#include "ParticlePairBfHistos.hpp"
#include "CapFile.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "TRandom.h"
#include "Task.hpp"
#include "Configuration.hpp"

ClassImp(CAP::ParticlePairBfHistos);

namespace CAP
{

ParticlePairBfHistos::ParticlePairBfHistos()
:
ParticlePairDerivedHistos(),
h_C2_CD_ptpt(nullptr),
h_C2_CI_ptpt(nullptr),
h_C2N_B12_ptpt(nullptr),
h_C2N_B21_ptpt(nullptr),
h_C2N_Bs_ptpt(nullptr),
h_R2_B12_ptpt(nullptr),
h_R2_B21_ptpt(nullptr),
h_R2_Bs_ptpt(nullptr),

h_C2_CD_phiphi(nullptr),
h_C2_CI_phiphi(nullptr),
h_C2N_B12_phiphi(nullptr),
h_C2N_B21_phiphi(nullptr),
h_C2N_Bs_phiphi(nullptr),
h_R2_B12_phiphi(nullptr),
h_R2_B21_phiphi(nullptr),
h_R2_Bs_phiphi(nullptr),

h_C2_CD_etaeta(nullptr),
h_C2_CI_etaeta(nullptr),
h_C2N_B12_etaeta(nullptr),
h_C2N_B21_etaeta(nullptr),
h_C2N_Bs_etaeta(nullptr),
h_R2_B12_etaeta(nullptr),
h_R2_B21_etaeta(nullptr),
h_R2_Bs_etaeta(nullptr),

h_C2_CD_yY(nullptr),
h_C2_CI_yY(nullptr),
h_C2N_B12_yY(nullptr),
h_C2N_B21_yY(nullptr),
h_C2N_Bs_yY(nullptr),
h_R2_B12_yY(nullptr),
h_R2_B21_yY(nullptr),
h_R2_Bs_yY(nullptr),

h_C2_CD_DetaDphi(nullptr),
h_C2_CI_DetaDphi(nullptr),
h_C2N_B12_DetaDphi(nullptr),
h_C2N_B21_DetaDphi(nullptr),
h_C2N_Bs_DetaDphi(nullptr),
h_R2_B12_DetaDphi(nullptr),
h_R2_B21_DetaDphi(nullptr),
h_R2_Bs_DetaDphi(nullptr),

h_C2_CD_DyDphi(nullptr),
h_C2_CI_DyDphi(nullptr),
h_C2N_B12_DyDphi(nullptr),
h_C2N_B21_DyDphi(nullptr),
h_C2N_Bs_DyDphi(nullptr),
h_R2_B12_DyDphi(nullptr),
h_R2_B21_DyDphi(nullptr),
h_R2_Bs_DyDphi(nullptr)
{
  appendClassName("ParticlePairBfHistos");
  setName("ParticlePairBfHistos");
  setTitle("ParticlePairBfHistos");
}

ParticlePairBfHistos::ParticlePairBfHistos(const ParticlePairBfHistos & source)
:
ParticlePairDerivedHistos(source),
h_C2_CD_ptpt(nullptr),
h_C2_CI_ptpt(nullptr),
h_C2N_B12_ptpt(nullptr),
h_C2N_B21_ptpt(nullptr),
h_C2N_Bs_ptpt(nullptr),
h_R2_B12_ptpt(nullptr),
h_R2_B21_ptpt(nullptr),
h_R2_Bs_ptpt(nullptr),

h_C2_CD_phiphi(nullptr),
h_C2_CI_phiphi(nullptr),
h_C2N_B12_phiphi(nullptr),
h_C2N_B21_phiphi(nullptr),
h_C2N_Bs_phiphi(nullptr),
h_R2_B12_phiphi(nullptr),
h_R2_B21_phiphi(nullptr),
h_R2_Bs_phiphi(nullptr),

h_C2_CD_etaeta(nullptr),
h_C2_CI_etaeta(nullptr),
h_C2N_B12_etaeta(nullptr),
h_C2N_B21_etaeta(nullptr),
h_C2N_Bs_etaeta(nullptr),
h_R2_B12_etaeta(nullptr),
h_R2_B21_etaeta(nullptr),
h_R2_Bs_etaeta(nullptr),

h_C2_CD_yY(nullptr),
h_C2_CI_yY(nullptr),
h_C2N_B12_yY(nullptr),
h_C2N_B21_yY(nullptr),
h_C2N_Bs_yY(nullptr),
h_R2_B12_yY(nullptr),
h_R2_B21_yY(nullptr),
h_R2_Bs_yY(nullptr),

h_C2_CD_DetaDphi(nullptr),
h_C2_CI_DetaDphi(nullptr),
h_C2N_B12_DetaDphi(nullptr),
h_C2N_B21_DetaDphi(nullptr),
h_C2N_Bs_DetaDphi(nullptr),
h_R2_B12_DetaDphi(nullptr),
h_R2_B21_DetaDphi(nullptr),
h_R2_Bs_DetaDphi(nullptr),

h_C2_CD_DyDphi(nullptr),
h_C2_CI_DyDphi(nullptr),
h_C2N_B12_DyDphi(nullptr),
h_C2N_B21_DyDphi(nullptr),
h_C2N_Bs_DyDphi(nullptr),
h_R2_B12_DyDphi(nullptr),
h_R2_B21_DyDphi(nullptr),
h_R2_Bs_DyDphi(nullptr)
{

}

ParticlePairBfHistos & ParticlePairBfHistos::operator=(const ParticlePairBfHistos & rhs)
{
  if (this!=&rhs)
    {
    ParticlePairDerivedHistos::operator=(rhs);
    cloneBF(rhs);
    }
  return *this;
}

void ParticlePairBfHistos::configure(const String & taskName,
                                           const String & objectType,
                                           const Configuration & configuration,
                                           unsigned int index)
{
  if (reportDebug(__FUNCTION__)) { printCR();}
  ParticlePairDerivedHistos::configure(taskName,objectType,configuration,index);
}

void ParticlePairBfHistos::create()
{
  if (reportDebug(__FUNCTION__)) { printCR();}
  h_C2_CD_ptpt    = createHistogram(createName(_histogramBaseName,"C2_CD_ptpt"),    pt_nbins,pt_min,pt_max,pt_nbins,pt_min,pt_max, "p_{T,1}","p_{T,2}", "C_{2}^{CD}");
  h_C2_CI_ptpt    = createHistogram(createName(_histogramBaseName,"C2_CI_ptpt"),    pt_nbins,pt_min,pt_max,pt_nbins,pt_min,pt_max, "p_{T,1}","p_{T,2}", "C_{2}^{CI}");
  h_C2N_B12_ptpt  = createHistogram(createName(_histogramBaseName,"C2N_B12_ptpt"),  pt_nbins,pt_min,pt_max,pt_nbins,pt_min,pt_max, "p_{T,1}","p_{T,2}", "B_{C2N}^{12}");
  h_C2N_B21_ptpt  = createHistogram(createName(_histogramBaseName,"C2N_B21_ptpt"),  pt_nbins,pt_min,pt_max,pt_nbins,pt_min,pt_max, "p_{T,1}","p_{T,2}", "B_{C2N}^{21}");
  h_C2N_Bs_ptpt   = createHistogram(createName(_histogramBaseName,"C2N_Bs_ptpt"),   pt_nbins,pt_min,pt_max,pt_nbins,pt_min,pt_max, "p_{T,1}","p_{T,2}", "B_{C2N}^{s}");
  h_R2_B12_ptpt   = createHistogram(createName(_histogramBaseName,"R2_B12_ptpt"),   pt_nbins,pt_min,pt_max,pt_nbins,pt_min,pt_max, "p_{T,1}","p_{T,2}", "B_{R2}^{12}");
  h_R2_B21_ptpt   = createHistogram(createName(_histogramBaseName,"R2_B21_ptpt"),   pt_nbins,pt_min,pt_max,pt_nbins,pt_min,pt_max, "p_{T,1}","p_{T,2}", "B_{R2}^{21}");
  h_R2_Bs_ptpt    = createHistogram(createName(_histogramBaseName,"R2_Bs_ptpt"),    pt_nbins,pt_min,pt_max,pt_nbins,pt_min,pt_max, "p_{T,1}","p_{T,2}", "B_{R2}^{s}");

  h_C2_CD_phiphi   = createHistogram(createName(_histogramBaseName,"C2_CD_phiphi"),   phi_nbins,phi_min,phi_max,phi_nbins,phi_min,phi_max, "#varphi_{1}","#varphi_{2}", "C_{2}^{CD}");
  h_C2_CI_phiphi   = createHistogram(createName(_histogramBaseName,"C2_CI_phiphi"),   phi_nbins,phi_min,phi_max,phi_nbins,phi_min,phi_max, "#varphi_{1}","#varphi_{2}", "C_{2}^{CI}");
  h_C2N_B12_phiphi = createHistogram(createName(_histogramBaseName,"C2N_B12_phiphi"), phi_nbins,phi_min,phi_max,phi_nbins,phi_min,phi_max, "#varphi_{1}","#varphi_{2}", "B_{C2N}^{12}");
  h_C2N_B21_phiphi = createHistogram(createName(_histogramBaseName,"C2N_B21_phiphi"), phi_nbins,phi_min,phi_max,phi_nbins,phi_min,phi_max, "#varphi_{1}","#varphi_{2}", "B_{C2N}^{21}");
  h_C2N_Bs_phiphi  = createHistogram(createName(_histogramBaseName,"C2N_Bs_phiphi"),  phi_nbins,phi_min,phi_max,phi_nbins,phi_min,phi_max, "#varphi_{1}","#varphi_{2}", "B_{C2N}^{s}");
  h_R2_B12_phiphi  = createHistogram(createName(_histogramBaseName,"R2_B12_phiphi"),  phi_nbins,phi_min,phi_max,phi_nbins,phi_min,phi_max, "#varphi_{1}","#varphi_{2}", "B_{R2}^{12}");
  h_R2_B21_phiphi  = createHistogram(createName(_histogramBaseName,"R2_B21_phiphi"),  phi_nbins,phi_min,phi_max,phi_nbins,phi_min,phi_max, "#varphi_{1}","#varphi_{2}", "B_{R2}^{21}");
  h_R2_Bs_phiphi   = createHistogram(createName(_histogramBaseName,"R2_Bs_phiphi"),   phi_nbins,phi_min,phi_max,phi_nbins,phi_min,phi_max, "#varphi_{1}","#varphi_{2}", "B_{R2}^{s}");

if (eta_fill)
  {
  h_C2_CD_etaeta   = createHistogram(createName(_histogramBaseName,"C2_CD_etaeta"),   eta_nbins,eta_min,eta_max,eta_nbins,eta_min,eta_max, "#eta_{1}","#eta_{2}", "C_{2}^{CD}");
  h_C2_CI_etaeta   = createHistogram(createName(_histogramBaseName,"C2_CI_etaeta"),   eta_nbins,eta_min,eta_max,eta_nbins,eta_min,eta_max, "#eta_{1}","#eta_{2}", "C_{2}^{CI}");
  h_C2N_B12_etaeta = createHistogram(createName(_histogramBaseName,"C2N_B12_etaeta"), eta_nbins,eta_min,eta_max,eta_nbins,eta_min,eta_max, "#eta_{1}","#eta_{2}", "B_{C2N}^{12}");
  h_C2N_B21_etaeta = createHistogram(createName(_histogramBaseName,"C2N_B21_etaeta"), eta_nbins,eta_min,eta_max,eta_nbins,eta_min,eta_max, "#eta_{1}","#eta_{2}", "B_{C2N}^{21}");
  h_C2N_Bs_etaeta  = createHistogram(createName(_histogramBaseName,"C2N_Bs_etaeta"),  eta_nbins,eta_min,eta_max,eta_nbins,eta_min,eta_max, "#eta_{1}","#eta_{2}", "B_{C2N}^{s}");
  h_R2_B12_etaeta  = createHistogram(createName(_histogramBaseName,"R2_B12_etaeta"),  eta_nbins,eta_min,eta_max,eta_nbins,eta_min,eta_max, "#eta_{1}","#eta_{2}", "B_{R2}^{12}");
  h_R2_B21_etaeta  = createHistogram(createName(_histogramBaseName,"R2_B21_etaeta"),  eta_nbins,eta_min,eta_max,eta_nbins,eta_min,eta_max, "#eta_{1}","#eta_{2}", "B_{R2}^{21}");
  h_R2_Bs_etaeta   = createHistogram(createName(_histogramBaseName,"R2_Bs_etaeta"),   eta_nbins,eta_min,eta_max,eta_nbins,eta_min,eta_max, "#eta_{1}","#eta_{2}", "B_{R2}^{s}");

  h_C2_CD_DetaDphi    = createHistogram(createName(_histogramBaseName,"C2_CD_DetaDphi"),    Deta_nbins,Deta_min,Deta_max,Dphi_nbins,Dphi_min_shft,Dphi_max_shft, "#Delta#eta","#Delta#phi", "C_{2}^{CD}");
  h_C2_CI_DetaDphi    = createHistogram(createName(_histogramBaseName,"C2_CI_DetaDphi"),    Deta_nbins,Deta_min,Deta_max,Dphi_nbins,Dphi_min_shft,Dphi_max_shft, "#Delta#eta","#Delta#phi", "C_{2}^{CI}");
  h_C2N_B12_DetaDphi  = createHistogram(createName(_histogramBaseName,"C2N_B12_DetaDphi"),  Deta_nbins,Deta_min,Deta_max,Dphi_nbins,Dphi_min_shft,Dphi_max_shft, "#Delta#eta","#Delta#phi", "B_{C2N}^{12}");
  h_C2N_B21_DetaDphi  = createHistogram(createName(_histogramBaseName,"C2N_B21_DetaDphi"),  Deta_nbins,Deta_min,Deta_max,Dphi_nbins,Dphi_min_shft,Dphi_max_shft, "#Delta#eta","#Delta#phi", "B_{C2N}^{21}");
  h_C2N_Bs_DetaDphi   = createHistogram(createName(_histogramBaseName,"C2N_Bs_DetaDphi"),   Deta_nbins,Deta_min,Deta_max,Dphi_nbins,Dphi_min_shft,Dphi_max_shft, "#Delta#eta","#Delta#phi", "B_{C2N}^{s}");
  h_R2_B12_DetaDphi   = createHistogram(createName(_histogramBaseName,"R2_B12_DetaDphi"),   Deta_nbins,Deta_min,Deta_max,Dphi_nbins,Dphi_min_shft,Dphi_max_shft, "#Delta#eta","#Delta#phi", "B_{R2}^{12}");
  h_R2_B21_DetaDphi   = createHistogram(createName(_histogramBaseName,"R2_B21_DetaDphi"),   Deta_nbins,Deta_min,Deta_max,Dphi_nbins,Dphi_min_shft,Dphi_max_shft, "#Delta#eta","#Delta#phi", "B_{R2}^{21}");
  h_R2_Bs_DetaDphi    = createHistogram(createName(_histogramBaseName,"R2_Bs_DetaDphi"),    Deta_nbins,Deta_min,Deta_max,Dphi_nbins,Dphi_min_shft,Dphi_max_shft, "#Delta#eta","#Delta#phi", "B_{R2}^{s}");
  }

  if (rapidity_fill)
    {
    h_C2_CD_yY     = createHistogram(createName(_histogramBaseName,"C2_CD_yY"),    rapidity_nbins,rapidity_minimum,rapidity_maximum,rapidity_nbins,rapidity_minimum,rapidity_maximum, "y_{1}","y_{2}", "C_{2}^{CD}");
    h_C2_CI_yY     = createHistogram(createName(_histogramBaseName,"C2_CI_yY"),    rapidity_nbins,rapidity_minimum,rapidity_maximum,rapidity_nbins,rapidity_minimum,rapidity_maximum, "y_{1}","y_{2}", "C_{2}^{CI}");
    h_C2N_B12_yY   = createHistogram(createName(_histogramBaseName,"C2N_B12_yY"),  rapidity_nbins,rapidity_minimum,rapidity_maximum,rapidity_nbins,rapidity_minimum,rapidity_maximum, "y_{1}","y_{2}", "B_{C2N}^{12}");
    h_C2N_B21_yY   = createHistogram(createName(_histogramBaseName,"C2N_B21_yY"),  rapidity_nbins,rapidity_minimum,rapidity_maximum,rapidity_nbins,rapidity_minimum,rapidity_maximum, "y_{1}","y_{2}", "B_{C2N}^{21}");
    h_C2N_Bs_yY    = createHistogram(createName(_histogramBaseName,"C2N_Bs_yY"),   rapidity_nbins,rapidity_minimum,rapidity_maximum,rapidity_nbins,rapidity_minimum,rapidity_maximum, "y_{1}","y_{2}", "B_{C2N}^{s}");
    h_R2_B12_yY    = createHistogram(createName(_histogramBaseName,"R2_B12_yY"),   rapidity_nbins,rapidity_minimum,rapidity_maximum,rapidity_nbins,rapidity_minimum,rapidity_maximum, "y_{1}","y_{2}", "B_{R2}^{12}");
    h_R2_B21_yY    = createHistogram(createName(_histogramBaseName,"R2_B21_yY"),   rapidity_nbins,rapidity_minimum,rapidity_maximum,rapidity_nbins,rapidity_minimum,rapidity_maximum, "y_{1}","y_{2}", "B_{R2}^{21}");
    h_R2_Bs_yY     = createHistogram(createName(_histogramBaseName,"R2_Bs_yY"),    rapidity_nbins,rapidity_minimum,rapidity_maximum,rapidity_nbins,rapidity_minimum,rapidity_maximum, "y_{1}","y_{2}", "B_{R2}^{s}");

    h_C2_CD_DyDphi     = createHistogram(createName(_histogramBaseName,"C2_CD_DyDphi"),    Drapidity_nbins,Drapidity_min,Drapidity_max,Dphi_nbins,Dphi_min_shft,Dphi_max_shft, "#Delta y","#Delta#phi", "C_{2}^{CD}");
    h_C2_CI_DyDphi     = createHistogram(createName(_histogramBaseName,"C2_CI_DyDphi"),    Drapidity_nbins,Drapidity_min,Drapidity_max,Dphi_nbins,Dphi_min_shft,Dphi_max_shft, "#Delta y","#Delta#phi", "C_{2}^{CI}");
    h_C2N_B12_DyDphi   = createHistogram(createName(_histogramBaseName,"C2N_B12_DyDphi"),  Drapidity_nbins,Drapidity_min,Drapidity_max,Dphi_nbins,Dphi_min_shft,Dphi_max_shft, "#Delta y","#Delta#phi", "B_{C2N}^{12}");
    h_C2N_B21_DyDphi   = createHistogram(createName(_histogramBaseName,"C2N_B21_DyDphi"),  Drapidity_nbins,Drapidity_min,Drapidity_max,Dphi_nbins,Dphi_min_shft,Dphi_max_shft, "#Delta y","#Delta#phi", "B_{C2N}^{21}");
    h_C2N_Bs_DyDphi    = createHistogram(createName(_histogramBaseName,"C2N_Bs_DyDphi"),   Drapidity_nbins,Drapidity_min,Drapidity_max,Dphi_nbins,Dphi_min_shft,Dphi_max_shft, "#Delta y","#Delta#phi", "B_{C2N}^{s}");
    h_R2_B12_DyDphi    = createHistogram(createName(_histogramBaseName,"R2_B12_DyDphi"),   Drapidity_nbins,Drapidity_min,Drapidity_max,Dphi_nbins,Dphi_min_shft,Dphi_max_shft, "#Delta y","#Delta#phi", "B_{R2}^{12}");
    h_R2_B21_DyDphi    = createHistogram(createName(_histogramBaseName,"R2_B21_DyDphi"),   Drapidity_nbins,Drapidity_min,Drapidity_max,Dphi_nbins,Dphi_min_shft,Dphi_max_shft, "#Delta y","#Delta#phi", "B_{R2}^{21}");
    h_R2_Bs_DyDphi     = createHistogram(createName(_histogramBaseName,"R2_Bs_DyDphi"),    Drapidity_nbins,Drapidity_min,Drapidity_max,Dphi_nbins,Dphi_min_shft,Dphi_max_shft, "#Delta y","#Delta#phi", "B_{R2}^{s}");
    }
}


void ParticlePairBfHistos::loadFrom(CapFile & inputFile)
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  ParticlePairDerivedHistos::loadFrom(inputFile);
  h_C2_CD_ptpt    = importHistogram(inputFile,createName(_histogramBaseName,"C2_CD_ptpt"));
  h_C2_CI_ptpt    = importHistogram(inputFile,createName(_histogramBaseName,"C2_CI_ptpt"));
  h_C2N_B12_ptpt  = importHistogram(inputFile,createName(_histogramBaseName,"C2N_B12_ptpt"));
  h_C2N_B21_ptpt  = importHistogram(inputFile,createName(_histogramBaseName,"C2N_B21_ptpt"));
  h_C2N_Bs_ptpt   = importHistogram(inputFile,createName(_histogramBaseName,"C2N_Bs_ptpt"));
  h_R2_B12_ptpt   = importHistogram(inputFile,createName(_histogramBaseName,"R2_B12_ptpt"));
  h_R2_B21_ptpt   = importHistogram(inputFile,createName(_histogramBaseName,"R2_B21_ptpt"));
  h_R2_Bs_ptpt    = importHistogram(inputFile,createName(_histogramBaseName,"R2_Bs_ptpt"));

  h_C2_CD_phiphi   = importHistogram(inputFile,createName(_histogramBaseName,"C2_CD_phiphi"));
  h_C2_CI_phiphi   = importHistogram(inputFile,createName(_histogramBaseName,"C2_CI_phiphi"));
  h_C2N_B12_phiphi = importHistogram(inputFile,createName(_histogramBaseName,"C2N_B12_phiphi"));
  h_C2N_B21_phiphi = importHistogram(inputFile,createName(_histogramBaseName,"C2N_B21_phiphi"));
  h_C2N_Bs_phiphi  = importHistogram(inputFile,createName(_histogramBaseName,"C2N_Bs_phiphi"));
  h_R2_B12_phiphi  = importHistogram(inputFile,createName(_histogramBaseName,"R2_B12_phiphi"));
  h_R2_B21_phiphi  = importHistogram(inputFile,createName(_histogramBaseName,"R2_B21_phiphi"));
  h_R2_Bs_phiphi   = importHistogram(inputFile,createName(_histogramBaseName,"R2_Bs_phiphi"));

  if (eta_fill)
    {
    h_C2_CD_etaeta   = importHistogram(inputFile,createName(_histogramBaseName,"C2_CD_etaeta"));
    h_C2_CI_etaeta   = importHistogram(inputFile,createName(_histogramBaseName,"C2_CI_etaeta"));
    h_C2N_B12_etaeta = importHistogram(inputFile,createName(_histogramBaseName,"C2N_B12_etaeta"));
    h_C2N_B21_etaeta = importHistogram(inputFile,createName(_histogramBaseName,"C2N_B21_etaeta"));
    h_C2N_Bs_etaeta  = importHistogram(inputFile,createName(_histogramBaseName,"C2N_Bs_etaeta"));
    h_R2_B12_etaeta  = importHistogram(inputFile,createName(_histogramBaseName,"R2_B12_etaeta"));
    h_R2_B21_etaeta  = importHistogram(inputFile,createName(_histogramBaseName,"R2_B21_etaeta"));
    h_R2_Bs_etaeta   = importHistogram(inputFile,createName(_histogramBaseName,"R2_Bs_etaeta"));

    h_C2_CD_DetaDphi    = importHistogram(inputFile,createName(_histogramBaseName,"C2_CD_DetaDphi_shft"));
    h_C2_CI_DetaDphi    = importHistogram(inputFile,createName(_histogramBaseName,"C2_CI_DetaDphi_shft"));
    h_C2N_B12_DetaDphi  = importHistogram(inputFile,createName(_histogramBaseName,"C2N_B12_DetaDphi_shft"));
    h_C2N_B21_DetaDphi  = importHistogram(inputFile,createName(_histogramBaseName,"C2N_B21_DetaDphi_shft"));
    h_C2N_Bs_DetaDphi   = importHistogram(inputFile,createName(_histogramBaseName,"C2N_Bs_DetaDphi_shft"));
    h_R2_B12_DetaDphi   = importHistogram(inputFile,createName(_histogramBaseName,"R2_B12_DetaDphi_shft"));
    h_R2_B21_DetaDphi   = importHistogram(inputFile,createName(_histogramBaseName,"R2_B21_DetaDphi_shft"));
    h_R2_Bs_DetaDphi    = importHistogram(inputFile,createName(_histogramBaseName,"R2_Bs_DetaDphi_shft"));
    }

  if (rapidity_fill)
    {
    h_C2_CD_yY     = importHistogram(inputFile,createName(_histogramBaseName,"C2_CD_yY"));
    h_C2_CI_yY     = importHistogram(inputFile,createName(_histogramBaseName,"C2_CI_yY"));
    h_C2N_B12_yY   = importHistogram(inputFile,createName(_histogramBaseName,"C2N_B12_yY"));
    h_C2N_B21_yY   = importHistogram(inputFile,createName(_histogramBaseName,"C2N_B21_yY"));
    h_C2N_Bs_yY    = importHistogram(inputFile,createName(_histogramBaseName,"C2N_Bs_yY"));
    h_R2_B12_yY    = importHistogram(inputFile,createName(_histogramBaseName,"R2_B12_yY"));
    h_R2_B21_yY    = importHistogram(inputFile,createName(_histogramBaseName,"R2_B21_yY"));
    h_R2_Bs_yY     = importHistogram(inputFile,createName(_histogramBaseName,"R2_Bs_yY"));

    h_C2_CD_DyDphi     = importHistogram(inputFile,createName(_histogramBaseName,"C2_CD_DyDphi_shft"));
    h_C2_CI_DyDphi     = importHistogram(inputFile,createName(_histogramBaseName,"C2_CI_DyDphi_shft"));
    h_C2N_B12_DyDphi   = importHistogram(inputFile,createName(_histogramBaseName,"C2N_B12_DyDphi_shft"));
    h_C2N_B21_DyDphi   = importHistogram(inputFile,createName(_histogramBaseName,"C2N_B21_DyDphi_shft"));
    h_C2N_Bs_DyDphi    = importHistogram(inputFile,createName(_histogramBaseName,"C2N_Bs_DyDphi_shft"));
    h_R2_B12_DyDphi    = importHistogram(inputFile,createName(_histogramBaseName,"R2_B12_DyDphi_shft"));
    h_R2_B21_DyDphi    = importHistogram(inputFile,createName(_histogramBaseName,"R2_B21_DyDphi_shft"));
    h_R2_Bs_DyDphi     = importHistogram(inputFile,createName(_histogramBaseName,"R2_Bs_DyDphi_shft"));
    }
}

void ParticlePairBfHistos::cloneBF(const ParticlePairBfHistos & source)
{
  h_C2_CD_ptpt    = histogramLike(source.h_C2_CD_ptpt);
  h_C2_CI_ptpt    = histogramLike(source.h_C2_CI_ptpt);
  h_C2N_B12_ptpt  = histogramLike(source.h_C2N_B12_ptpt);
  h_C2N_B21_ptpt  = histogramLike(source.h_C2N_B21_ptpt);
  h_C2N_Bs_ptpt   = histogramLike(source.h_C2N_Bs_ptpt);
  h_R2_B12_ptpt   = histogramLike(source.h_R2_B12_ptpt);
  h_R2_B21_ptpt   = histogramLike(source.h_R2_B21_ptpt);
  h_R2_Bs_ptpt    = histogramLike(source.h_R2_Bs_ptpt);

  h_C2_CD_phiphi   = histogramLike(source.h_C2_CD_phiphi);
  h_C2_CI_phiphi   = histogramLike(source.h_C2_CI_phiphi);
  h_C2N_B12_phiphi = histogramLike(source.h_C2N_B12_phiphi);
  h_C2N_B21_phiphi = histogramLike(source.h_C2N_B21_phiphi);
  h_C2N_Bs_phiphi  = histogramLike(source.h_C2N_Bs_phiphi);
  h_R2_B12_phiphi  = histogramLike(source.h_R2_B12_phiphi);
  h_R2_B21_phiphi  = histogramLike(source.h_R2_B21_phiphi);
  h_R2_Bs_phiphi   = histogramLike(source.h_R2_Bs_phiphi);

  h_C2_CD_etaeta   = histogramLike(source.h_C2_CD_etaeta);
  h_C2_CI_etaeta   = histogramLike(source.h_C2_CI_etaeta);
  h_C2N_B12_etaeta = histogramLike(source.h_C2N_B12_etaeta);
  h_C2N_B21_etaeta = histogramLike(source.h_C2N_B21_etaeta);
  h_C2N_Bs_etaeta  = histogramLike(source.h_C2N_Bs_etaeta);
  h_R2_B12_etaeta  = histogramLike(source.h_R2_B12_etaeta);
  h_R2_B21_etaeta  = histogramLike(source.h_R2_B21_etaeta);
  h_R2_Bs_etaeta   = histogramLike(source.h_R2_Bs_etaeta);

  h_C2_CD_DetaDphi    = histogramLike(source.h_C2_CD_DetaDphi);
  h_C2_CI_DetaDphi    = histogramLike(source.h_C2_CI_DetaDphi);
  h_C2N_B12_DetaDphi  = histogramLike(source.h_C2N_B12_DetaDphi);
  h_C2N_B21_DetaDphi  = histogramLike(source.h_C2N_B21_DetaDphi);
  h_C2N_Bs_DetaDphi   = histogramLike(source.h_C2N_Bs_DetaDphi);
  h_R2_B12_DetaDphi   = histogramLike(source.h_R2_B12_DetaDphi);
  h_R2_B21_DetaDphi   = histogramLike(source.h_R2_B21_DetaDphi);
  h_R2_Bs_DetaDphi    = histogramLike(source.h_R2_Bs_DetaDphi);

  h_C2_CD_yY     = histogramLike(source.h_C2_CD_yY);
  h_C2_CI_yY     = histogramLike(source.h_C2_CI_yY);
  h_C2N_B12_yY   = histogramLike(source.h_C2N_B12_yY);
  h_C2N_B21_yY   = histogramLike(source.h_C2N_B21_yY);
  h_C2N_Bs_yY    = histogramLike(source.h_C2N_Bs_yY);
  h_R2_B12_yY    = histogramLike(source.h_R2_B12_yY);
  h_R2_B21_yY    = histogramLike(source.h_R2_B21_yY);
  h_R2_Bs_yY     = histogramLike(source.h_R2_Bs_yY);

  h_C2_CD_DyDphi     = histogramLike(source.h_C2_CD_DyDphi);
  h_C2_CI_DyDphi     = histogramLike(source.h_C2_CI_DyDphi);
  h_C2N_B12_DyDphi   = histogramLike(source.h_C2N_B12_DyDphi);
  h_C2N_B21_DyDphi   = histogramLike(source.h_C2N_B21_DyDphi);
  h_C2N_Bs_DyDphi    = histogramLike(source.h_C2N_Bs_DyDphi);
  h_R2_B12_DyDphi    = histogramLike(source.h_R2_B12_DyDphi);
  h_R2_B21_DyDphi    = histogramLike(source.h_R2_B21_DyDphi);
  h_R2_Bs_DyDphi     = histogramLike(source.h_R2_Bs_DyDphi);
}


//!
//! HistogramFolder from ParticlePairBfHistos must be normalized "per event" before calling this function
//!
void ParticlePairBfHistos::calculateBfHistograms(ParticlePairDerivedHistos & pair21B,
                                                 ParticlePairDerivedHistos & pair2B1B,
                                                 ParticlePairDerivedHistos & pair2B1,
                                                 ParticlePairDerivedHistos & pair21)
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  if (pair21B.h_C2_ptpt)
    {
    h_C2_CD_ptpt    ->setAsSumOf(*pair21B.h_C2_ptpt,*pair2B1B.h_C2_ptpt,1.0,-1.0);
    h_C2_CD_ptpt    ->add(*pair2B1.h_C2_ptpt,+1.0);
    h_C2_CD_ptpt    ->add(*pair21.h_C2_ptpt,-1.0);
    printString("ParticlePairBfHistos::calculateBfHistograms -- 21 --");
    h_C2_CI_ptpt    ->setAsSumOf(*pair2B1.h_C2_ptpt,*pair2B1B.h_C2_ptpt,1.0,+1.0);
    h_C2_CI_ptpt    ->add(*pair2B1.h_C2_ptpt);
    h_C2_CI_ptpt    ->add(*pair21.h_C2_ptpt);
    printString("ParticlePairBfHistos::calculateBfHistograms -- 3 --");
    h_C2N_B12_ptpt  ->setAsSumOf(*pair21B.h_C2_ptpt,*pair2B1B.h_C2_ptpt,1.0,-1.0);
    h_C2N_B21_ptpt  ->setAsSumOf(*pair2B1.h_C2_ptpt,*pair21.h_C2_ptpt,1.0,-1.0);
    h_C2N_Bs_ptpt   ->setAsSumOf(*h_C2N_B12_ptpt,*h_C2N_B21_ptpt,0.5,0.5);
    printString("ParticlePairBfHistos::calculateBfHistograms -- 4 --");
    h_R2_B12_ptpt   ->setAsSumOf(*pair21B.h_R2_ptpt,*pair2B1B.h_R2_ptpt,1.0,-1.0);
    h_R2_B21_ptpt   ->setAsSumOf(*pair2B1.h_R2_ptpt,*pair21.h_R2_ptpt,1.0,-1.0);
    h_R2_Bs_ptpt    ->setAsSumOf(*h_R2_B12_ptpt,*h_R2_B21_ptpt,0.5,0.5);
    }
  printString("ParticlePairBfHistos::calculateBfHistograms -- 5 --");

  if (pair21B.h_C2_phiphi)
    {
    h_C2_CD_phiphi   ->setAsSumOf(*pair21B.h_C2_phiphi,*pair2B1B.h_C2_phiphi,1.0,-1.0);
    h_C2_CD_phiphi   ->add(*pair2B1.h_C2_phiphi,+1.0);
    h_C2_CD_phiphi   ->add(*pair21.h_C2_phiphi,-1.0);
    printString("ParticlePairBfHistos::calculateBfHistograms -- 6 --");
    h_C2_CI_phiphi   ->setAsSumOf(*pair21B.h_C2_phiphi,*pair2B1B.h_C2_phiphi,1.0,1.0);
    h_C2_CI_phiphi   ->add(*pair2B1.h_C2_phiphi,+1.0);
    h_C2_CI_phiphi   ->add(*pair21.h_C2_phiphi,+1.0);
    h_C2N_B12_phiphi ->setAsSumOf(*pair21B.h_C2_phiphi,*pair2B1B.h_C2_phiphi,1.0,-1.0);
    h_C2N_B21_phiphi ->setAsSumOf(*pair2B1.h_C2_phiphi,*pair21.h_C2_phiphi,1.0,-1.0);
    h_C2N_Bs_phiphi  ->setAsSumOf(*h_C2N_B12_phiphi,*h_C2N_B21_phiphi,0.5,0.5);
    h_R2_B12_phiphi  ->setAsSumOf(*pair21B.h_R2_phiphi,*pair2B1B.h_R2_phiphi,1.0,-1.0);
    h_R2_B21_phiphi  ->setAsSumOf(*pair2B1.h_R2_phiphi,*pair21.h_R2_phiphi,1.0,-1.0);
    h_R2_Bs_phiphi   ->setAsSumOf(*h_R2_B12_phiphi,*h_R2_B21_phiphi,0.5,0.5);
    }
  printString("ParticlePairBfHistos::calculateBfHistograms -- 7 --");

  if (pair21B.h_C2_etaeta)
    {
    h_C2_CD_etaeta   ->setAsSumOf(*pair21B.h_C2_etaeta,*pair2B1B.h_C2_etaeta,1.0,-1.0);
    h_C2_CD_etaeta   ->add(*pair2B1.h_C2_etaeta,+1.0);
    h_C2_CD_etaeta   ->add(*pair21.h_C2_etaeta,-1.0);
    h_C2_CI_etaeta   ->setAsSumOf(*pair21B.h_C2_etaeta,*pair2B1B.h_C2_etaeta,1.0,1.0);
    h_C2_CI_etaeta   ->add(*pair2B1.h_C2_etaeta,+1.0);
    h_C2_CI_etaeta   ->add(*pair21.h_C2_etaeta,+1.0);
    printString("ParticlePairBfHistos::calculateBfHistograms -- 8 --");
    h_C2N_B12_etaeta ->setAsSumOf(*pair21B.h_C2_etaeta,*pair2B1B.h_C2_etaeta,1.0,-1.0);
    h_C2N_B21_etaeta ->setAsSumOf(*pair2B1.h_C2_etaeta,*pair21.h_C2_etaeta,1.0,-1.0);
    h_C2N_Bs_etaeta  ->setAsSumOf(*h_C2N_B12_etaeta,*h_C2N_B21_etaeta,0.5,0.5);
    h_R2_B12_etaeta  ->setAsSumOf(*pair21B.h_R2_etaeta,*pair2B1B.h_R2_etaeta,1.0,-1.0);
    h_R2_B21_etaeta  ->setAsSumOf(*pair2B1.h_R2_etaeta,*pair21.h_R2_etaeta,1.0,-1.0);
    h_R2_Bs_etaeta   ->setAsSumOf(*h_R2_B12_etaeta,*h_R2_B21_etaeta,0.5,0.5);
    }
  printString("ParticlePairBfHistos::calculateBfHistograms -- 9 --");

  if (pair21B.h_C2_yY)
    {
    h_C2_CD_yY   ->setAsSumOf(*pair21B.h_C2_yY,*pair2B1B.h_C2_yY,1.0,-1.0);
    h_C2_CD_yY   ->add(*pair2B1.h_C2_yY,+1.0);
    h_C2_CD_yY   ->add(*pair21.h_C2_yY,-1.0);
    h_C2_CI_yY   ->setAsSumOf(*pair21B.h_C2_yY,*pair2B1B.h_C2_yY,1.0,1.0);
    h_C2_CI_yY   ->add(*pair2B1.h_C2_yY,+1.0);
    h_C2_CI_yY   ->add(*pair21.h_C2_yY,+1.0);
    h_C2N_B12_yY ->setAsSumOf(*pair21B.h_C2_yY,*pair2B1B.h_C2_yY,1.0,-1.0);
    h_C2N_B21_yY ->setAsSumOf(*pair2B1.h_C2_yY,*pair21.h_C2_yY,1.0,-1.0);
    h_C2N_Bs_yY  ->setAsSumOf(*h_C2N_B12_yY,*h_C2N_B21_yY,0.5,0.5);
    h_R2_B12_yY  ->setAsSumOf(*pair21B.h_R2_yY,*pair2B1B.h_R2_yY,1.0,-1.0);
    h_R2_B21_yY  ->setAsSumOf(*pair2B1.h_R2_yY,*pair21.h_R2_yY,1.0,-1.0);
    h_R2_Bs_yY   ->setAsSumOf(*h_R2_B12_yY,*h_R2_B21_yY,0.5,0.5);
    }
  printString("ParticlePairBfHistos::calculateBfHistograms -- 10 --");

  if (pair21B.h_C2_DetaDphi)
    {
    h_C2_CD_DetaDphi   ->setAsSumOf(*pair21B.h_C2_DetaDphi_shft,*pair2B1B.h_C2_DetaDphi_shft,1.0,-1.0);
    h_C2_CD_DetaDphi   ->add(*pair2B1.h_C2_DetaDphi_shft,+1.0);
    h_C2_CD_DetaDphi   ->add(*pair21.h_C2_DetaDphi_shft,-1.0);
    h_C2_CI_DetaDphi   ->setAsSumOf(*pair21B.h_C2_DetaDphi_shft,*pair2B1B.h_C2_DetaDphi_shft,1.0,1.0);
    h_C2_CI_DetaDphi   ->add(*pair2B1.h_C2_DetaDphi_shft,+1.0);
    h_C2_CI_DetaDphi   ->add(*pair21.h_C2_DetaDphi_shft,+1.0);
    h_C2N_B12_DetaDphi ->setAsSumOf(*pair21B.h_C2_DetaDphi_shft,*pair2B1B.h_C2_DetaDphi_shft,1.0,-1.0);
    h_C2N_B21_DetaDphi ->setAsSumOf(*pair2B1.h_C2_DetaDphi_shft,*pair21.h_C2_DetaDphi_shft,1.0,-1.0);
    h_C2N_Bs_DetaDphi  ->setAsSumOf(*h_C2N_B12_DetaDphi,*h_C2N_B21_DetaDphi,0.5,0.5);
    h_R2_B12_DetaDphi  ->setAsSumOf(*pair21B.h_R2_DetaDphi_shft,*pair2B1B.h_R2_DetaDphi_shft,1.0,-1.0);
    h_R2_B21_DetaDphi  ->setAsSumOf(*pair2B1.h_R2_DetaDphi_shft,*pair21.h_R2_DetaDphi_shft,1.0,-1.0);
    h_R2_Bs_DetaDphi   ->setAsSumOf(*h_R2_B12_DetaDphi,*h_R2_B21_DetaDphi,0.5,0.5);
    }
  printString("ParticlePairBfHistos::calculateBfHistograms -- 11 --");

  if (pair21B.h_C2_DyDphi)
    {
    h_C2_CD_DyDphi   ->setAsSumOf(*pair21B.h_C2_DyDphi_shft,*pair2B1B.h_C2_DyDphi_shft,1.0,-1.0);
    h_C2_CD_DyDphi   ->add(*pair2B1.h_C2_DyDphi_shft,+1.0);
    h_C2_CD_DyDphi   ->add(*pair21.h_C2_DyDphi_shft,-1.0);
    h_C2_CI_DyDphi   ->setAsSumOf(*pair21B.h_C2_DyDphi_shft,*pair2B1B.h_C2_DyDphi_shft,1.0,1.0);
    h_C2_CI_DyDphi   ->add(*pair2B1.h_C2_DyDphi_shft,+1.0);
    h_C2_CI_DyDphi   ->add(*pair21.h_C2_DyDphi_shft,+1.0);
    h_C2N_B12_DyDphi ->setAsSumOf(*pair21B.h_C2_DyDphi_shft,*pair2B1B.h_C2_DyDphi_shft,1.0,-1.0);
    h_C2N_B21_DyDphi ->setAsSumOf(*pair2B1.h_C2_DyDphi_shft,*pair21.h_C2_DyDphi_shft,1.0,-1.0);
    h_C2N_Bs_DyDphi  ->setAsSumOf(*h_C2N_B12_DyDphi,*h_C2N_B21_DyDphi,0.5,0.5);
    h_R2_B12_DyDphi  ->setAsSumOf(*pair21B.h_R2_DyDphi_shft,*pair2B1B.h_R2_DyDphi_shft,1.0,-1.0);
    h_R2_B21_DyDphi  ->setAsSumOf(*pair2B1.h_R2_DyDphi_shft,*pair21.h_R2_DyDphi_shft,1.0,-1.0);
    h_R2_Bs_DyDphi   ->setAsSumOf(*h_R2_B12_DyDphi,*h_R2_B21_DyDphi,0.5,0.5);
    }
}


} //  namespace CAP
