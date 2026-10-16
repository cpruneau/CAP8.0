/* **********************************************************************
 * Copyright (C) 2019-2024, Claude Pruneau, Akash Raj
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau, Akash Raj,  Nov 2024
 *
 * *********************************************************************/
#include "JetPairHistosDerived.hpp"
#include "JetHelpers.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "Configuration.hpp"
#include "Task.hpp"

ClassImp(CAP::JetPairHistosDerived);

namespace CAP
{

JetPairHistosDerived::JetPairHistosDerived()
:
JetPairHistos(),
h_jet_a2pp_ptpt(nullptr),
 h_jet_a2pp_phiphi(nullptr),
 h_jet_a2pp_etaeta(nullptr),
 h_jet_a2pp_thth(nullptr),
 h_jet_a2pp_jtjt(nullptr),
 h_jet_a2pp_zz(nullptr),

 h_jet_a2mp_ptpt(nullptr),
 h_jet_a2mp_phiphi(nullptr),
 h_jet_a2mp_etaeta(nullptr),
 h_jet_a2mp_thth(nullptr),
 h_jet_a2mp_jtjt(nullptr),
 h_jet_a2mp_zz(nullptr),

 h_jet_a2pm_ptpt(nullptr),
 h_jet_a2pm_phiphi(nullptr),
 h_jet_a2pm_etaeta(nullptr),
 h_jet_a2pm_thth(nullptr),
 h_jet_a2pm_jtjt(nullptr),
 h_jet_a2pm_zz(nullptr),

 h_jet_a2mm_ptpt(nullptr),
 h_jet_a2mm_phiphi(nullptr),
 h_jet_a2mm_etaeta(nullptr),
 h_jet_a2mm_thth(nullptr),
 h_jet_a2mm_jtjt(nullptr),
 h_jet_a2mm_zz(nullptr),

 h_jet_b2pm_ptpt(nullptr),
 h_jet_b2pm_phiphi(nullptr),
 h_jet_b2pm_etaeta(nullptr),
 h_jet_b2pm_thth(nullptr),
 h_jet_b2pm_jtjt(nullptr),
 h_jet_b2pm_zz(nullptr),

 h_jet_b2mp_ptpt(nullptr),
 h_jet_b2mp_phiphi(nullptr),
 h_jet_b2mp_etaeta(nullptr),
 h_jet_b2mp_thth(nullptr),
 h_jet_b2mp_jtjt(nullptr),
 h_jet_b2mp_zz(nullptr),

 h_jet_b2s_ptpt(nullptr),
 h_jet_b2s_phiphi(nullptr),
 h_jet_b2s_etaeta(nullptr),
 h_jet_b2s_thth(nullptr),
 h_jet_b2s_jtjt(nullptr),
 h_jet_b2s_zz(nullptr)
{   }

JetPairHistosDerived::JetPairHistosDerived(const JetPairHistosDerived & group)
:
JetPairHistos(group),
h_jet_a2pp_ptpt(nullptr),
h_jet_a2pp_phiphi(nullptr),
h_jet_a2pp_etaeta(nullptr),
h_jet_a2pp_thth(nullptr),
h_jet_a2pp_jtjt(nullptr),
h_jet_a2pp_zz(nullptr),

h_jet_a2mp_ptpt(nullptr),
h_jet_a2mp_phiphi(nullptr),
h_jet_a2mp_etaeta(nullptr),
h_jet_a2mp_thth(nullptr),
h_jet_a2mp_jtjt(nullptr),
h_jet_a2mp_zz(nullptr),

h_jet_a2pm_ptpt(nullptr),
h_jet_a2pm_phiphi(nullptr),
h_jet_a2pm_etaeta(nullptr),
h_jet_a2pm_thth(nullptr),
h_jet_a2pm_jtjt(nullptr),
h_jet_a2pm_zz(nullptr),

h_jet_a2mm_ptpt(nullptr),
h_jet_a2mm_phiphi(nullptr),
h_jet_a2mm_etaeta(nullptr),
h_jet_a2mm_thth(nullptr),
h_jet_a2mm_jtjt(nullptr),
h_jet_a2mm_zz(nullptr),

h_jet_b2pm_ptpt(nullptr),
h_jet_b2pm_phiphi(nullptr),
h_jet_b2pm_etaeta(nullptr),
h_jet_b2pm_thth(nullptr),
h_jet_b2pm_jtjt(nullptr),
h_jet_b2pm_zz(nullptr),

h_jet_b2mp_ptpt(nullptr),
h_jet_b2mp_phiphi(nullptr),
h_jet_b2mp_etaeta(nullptr),
h_jet_b2mp_thth(nullptr),
h_jet_b2mp_jtjt(nullptr),
h_jet_b2mp_zz(nullptr),

h_jet_b2s_ptpt(nullptr),
h_jet_b2s_phiphi(nullptr),
h_jet_b2s_etaeta(nullptr),
h_jet_b2s_thth(nullptr),
h_jet_b2s_jtjt(nullptr),
h_jet_b2s_zz(nullptr)
{   }

JetPairHistosDerived & JetPairHistosDerived::operator=(const JetPairHistosDerived & group)
{
  if (this!=&group)
    {
    JetPairHistos::operator=(group);
    }
  return *this;
}

void JetPairHistosDerived::cloneD(const JetPairHistosDerived & source)
{
  h_jet_a2pp_ptpt   = histogramLike(source.h_jet_a2pp_ptpt);
  h_jet_a2pp_phiphi = histogramLike(source.h_jet_a2pp_phiphi);
  h_jet_a2pp_etaeta = histogramLike(source.h_jet_a2pp_etaeta);
  h_jet_a2pp_thth   = histogramLike(source.h_jet_a2pp_thth);
  h_jet_a2pp_jtjt   = histogramLike(source.h_jet_a2pp_jtjt);
  h_jet_a2pp_zz     = histogramLike(source.h_jet_a2pp_zz);

  h_jet_a2mp_ptpt   = histogramLike(source.h_jet_a2mp_ptpt);
  h_jet_a2mp_phiphi = histogramLike(source.h_jet_a2mp_phiphi);
  h_jet_a2mp_etaeta = histogramLike(source.h_jet_a2mp_etaeta);
  h_jet_a2mp_thth   = histogramLike(source.h_jet_a2mp_thth);
  h_jet_a2mp_jtjt   = histogramLike(source.h_jet_a2mp_jtjt);
  h_jet_a2mp_zz     = histogramLike(source.h_jet_a2mp_zz);

  h_jet_a2pm_ptpt   = histogramLike(source.h_jet_a2pm_ptpt);
  h_jet_a2pm_phiphi = histogramLike(source.h_jet_a2pm_phiphi);
  h_jet_a2pm_etaeta = histogramLike(source.h_jet_a2pm_etaeta);
  h_jet_a2pm_thth   = histogramLike(source.h_jet_a2pm_thth);
  h_jet_a2pm_jtjt   = histogramLike(source.h_jet_a2pm_jtjt);
  h_jet_a2pm_zz     = histogramLike(source.h_jet_a2pm_zz);

  h_jet_a2mm_ptpt   = histogramLike(source.h_jet_a2mm_ptpt);
  h_jet_a2mm_phiphi = histogramLike(source.h_jet_a2mm_phiphi);
  h_jet_a2mm_etaeta = histogramLike(source.h_jet_a2mm_etaeta);
  h_jet_a2mm_thth   = histogramLike(source.h_jet_a2mm_thth);
  h_jet_a2mm_jtjt   = histogramLike(source.h_jet_a2mm_jtjt);
  h_jet_a2mm_zz     = histogramLike(source.h_jet_a2mm_zz);

  h_jet_b2pm_ptpt   = histogramLike(source.h_jet_b2pm_ptpt);
  h_jet_b2pm_phiphi = histogramLike(source.h_jet_b2pm_phiphi);
  h_jet_b2pm_etaeta = histogramLike(source.h_jet_b2pm_etaeta);
  h_jet_b2pm_thth   = histogramLike(source.h_jet_b2pm_thth);
  h_jet_b2pm_jtjt   = safeCloneH2h_jet_b2pm_jtjt;
  h_jet_b2pm_zz     = histogramLike(source.h_jet_b2pm_zz);

  h_jet_b2mp_ptpt   = histogramLike(source.h_jet_b2mp_ptpt);
  h_jet_b2mp_phiphi = histogramLike(source.h_jet_b2mp_phiphi);
  h_jet_b2mp_etaeta = histogramLike(source.h_jet_b2mp_etaeta);
  h_jet_b2mp_thth   = histogramLike(source.h_jet_b2mp_thth);
  h_jet_b2mp_jtjt   = histogramLike(source.h_jet_b2mp_jtjt);
  h_jet_b2mp_zz     = histogramLike(source.h_jet_b2mp_zz);

  h_jet_b2s_ptpt    = histogramLike(source.h_jet_b2s_ptpt);
  h_jet_b2s_phiphi  = histogramLike(source.h_jet_b2s_phiphi);
  h_jet_b2s_etaeta  = histogramLike(source.h_jet_b2s_etaeta);
  h_jet_b2s_thth    = histogramLike(source.h_jet_b2s_thth);
  h_jet_b2s_jtjt    = histogramLike(source.h_jet_b2s_jtjt);
  h_jet_b2s_zz      = histogramLike(source.h_jet_b2s_zz);

}


void JetPairHistosDerived::configure(const String & taskName,
                                           const String & objectType,
                                           const Configuration & configuration,
                                           unsigned int index)
{
  JetPairHistos::configure(taskName,objectType,configuration,index);
}

void JetPairHistosDerived::create()
{
  if (reportDebug(__FUNCTION__)) { printCR();}
  h_jet_a2pp_ptpt   = createHistogram(createName(_histogramBaseName,"jet_a2pp_ptpt"),  pt_nbins,pt_min,pt_max,    pt_nbins,pt_min,pt_max,    "p_{T,1}",    "p_{T,2}",    "A_{2}^{++}");
  h_jet_a2pp_phiphi = createHistogram(createName(_histogramBaseName,"jet_a2pp_phiphi"),phi_nbins,phi_min,phi_max, phi_nbins,phi_min,phi_max, "#varphi_{1}","#varphi_{1}","A_{2}^{++}");
  h_jet_a2pp_etaeta = createHistogram(createName(_histogramBaseName,"jet_a2pp_etaet"), eta_nbins,eta_min,eta_max, eta_nbins,eta_min,eta_max, "#eta_{1}",   "#eta_{2}",   "A_{2}^{++}");
  h_jet_a2pp_thth   = createHistogram(createName(_histogramBaseName,"jet_a2pp_thth"),  th_nbins,th_min,th_max,    th_nbins,th_min,th_max,    "#theta_{1}", "#theta_{1}", "A_{2}^{++}");
  h_jet_a2pp_jtjt   = createHistogram(createName(_histogramBaseName,"jet_a2pp_jtjt"),  jt_nbins,jt_min,jt_max,    jt_nbins,jt_min,jt_max,    "j_{T,1}",    "j_{T,2}",    "A_{2}^{++}");
  h_jet_a2pp_zz     = createHistogram(createName(_histogramBaseName,"jet_a2pp_zz"),    z_nbins,z_min,z_max,       z_nbins,z_min,z_max,       "z_{1}",      "z_{2}",      "A_{2}^{++}");

  h_jet_a2mp_ptpt   = createHistogram(createName(_histogramBaseName,"jet_a2mp_ptpt"),  pt_nbins,pt_min,pt_max,    pt_nbins,pt_min,pt_max,    "p_{T,1}",    "p_{T,2}",    "A_{2}^{-+}");
  h_jet_a2mp_phiphi = createHistogram(createName(_histogramBaseName,"jet_a2mp_phiphi"),phi_nbins,phi_min,phi_max, phi_nbins,phi_min,phi_max, "#varphi_{1}","#varphi_{1}","A_{2}^{-+}");
  h_jet_a2mp_etaeta = createHistogram(createName(_histogramBaseName,"jet_a2mp_etaeta"),eta_nbins,eta_min,eta_max, eta_nbins,eta_min,eta_max, "#eta_{1}",   "#eta_{2}",   "A_{2}^{-+}");
  h_jet_a2mp_thth   = createHistogram(createName(_histogramBaseName,"jet_a2mp_thth"),  th_nbins,th_min,th_max,    th_nbins,th_min,th_max,    "#theta_{1}", "#theta_{1}", "A_{2}^{-+}");
  h_jet_a2mp_jtjt   = createHistogram(createName(_histogramBaseName,"jet_a2mp_jtjt"),  jt_nbins,jt_min,jt_max,    jt_nbins,jt_min,jt_max,    "j_{T,1}",    "j_{T,2}",    "A_{2}^{-+}");
  h_jet_a2mp_zz     = createHistogram(createName(_histogramBaseName,"jet_a2mp_zz"),    z_nbins,z_min,z_max,       z_nbins,z_min,z_max,       "z_{1}",      "z_{2}",      "A_{2}^{-+}");

  h_jet_a2pm_ptpt   = createHistogram(createName(_histogramBaseName,"jet_a2pm_ptpt"),  pt_nbins,pt_min,pt_max,    pt_nbins,pt_min,pt_max,    "p_{T,1}",    "p_{T,2}",    "A_{2}^{+-}");
  h_jet_a2pm_phiphi = createHistogram(createName(_histogramBaseName,"jet_a2pm_phiphi"),phi_nbins,phi_min,phi_max, phi_nbins,phi_min,phi_max, "#varphi_{1}","#varphi_{1}","A_{2}^{+-}");
  h_jet_a2pm_etaeta = createHistogram(createName(_histogramBaseName,"jet_a2pm_etaeta"),eta_nbins,eta_min,eta_max, eta_nbins,eta_min,eta_max, "#eta_{1}",   "#eta_{2}",   "A_{2}^{+-}");
  h_jet_a2pm_thth   = createHistogram(createName(_histogramBaseName,"jet_a2pm_thth"),  th_nbins,th_min,th_max,    th_nbins,th_min,th_max,    "#theta_{1}", "#theta_{1}", "A_{2}^{+-}");
  h_jet_a2pm_jtjt   = createHistogram(createName(_histogramBaseName,"jet_a2pm_jtjt"),  jt_nbins,jt_min,jt_max,    jt_nbins,jt_min,jt_max,    "j_{T,1}",    "j_{T,2}",    "A_{2}^{+-}");
  h_jet_a2pm_zz     = createHistogram(createName(_histogramBaseName,"jet_a2pm_zz"),    z_nbins,z_min,z_max,       z_nbins,z_min,z_max,       "z_{1}",      "z_{2}",      "A_{2}^{+-}");

  h_jet_a2mm_ptpt   = createHistogram(createName(_histogramBaseName,"jet_a2mm_ptpt"),  pt_nbins,pt_min,pt_max,    pt_nbins,pt_min,pt_max,    "p_{T,1}",    "p_{T,2}",    "A_{2}^{--}");
  h_jet_a2mm_phiphi = createHistogram(createName(_histogramBaseName,"jet_a2mm_phiphi"),phi_nbins,phi_min,phi_max, phi_nbins,phi_min,phi_max, "#varphi_{1}","#varphi_{1}","A_{2}^{--}");
  h_jet_a2mm_etaeta = createHistogram(createName(_histogramBaseName,"jet_a2mm_etaeta"),eta_nbins,eta_min,eta_max, eta_nbins,eta_min,eta_max, "#eta_{1}",   "#eta_{2}",   "A_{2}^{--}");
  h_jet_a2mm_thth   = createHistogram(createName(_histogramBaseName,"jet_a2mm_thth"),  th_nbins,th_min,th_max,    th_nbins,th_min,th_max,    "#theta_{1}", "#theta_{1}", "A_{2}^{--}");
  h_jet_a2mm_jtjt   = createHistogram(createName(_histogramBaseName,"jet_a2mm_jtjt"),  jt_nbins,jt_min,jt_max,    jt_nbins,jt_min,jt_max,    "j_{T,1}",    "j_{T,2}",    "A_{2}^{--}");
  h_jet_a2mm_zz     = createHistogram(createName(_histogramBaseName,"jet_a2mm_zz"),    z_nbins,z_min,z_max,       z_nbins,z_min,z_max,       "z_{1}",      "z_{2}",      "A_{2}^{--}");

  h_jet_b2pm_ptpt   = createHistogram(createName(_histogramBaseName,"jet_b2pm_ptpt"),  pt_nbins,pt_min,pt_max,    pt_nbins,pt_min,pt_max,    "p_{T,1}",    "p_{T,2}",    "B_{2}^{+-}");
  h_jet_b2pm_phiphi = createHistogram(createName(_histogramBaseName,"jet_b2pm_phiphi"),phi_nbins,phi_min,phi_max, phi_nbins,phi_min,phi_max, "#varphi_{1}","#varphi_{1}","B_{2}^{+-}");
  h_jet_b2pm_etaeta = createHistogram(createName(_histogramBaseName,"jet_b2pm_etaeta"),eta_nbins,eta_min,eta_max, eta_nbins,eta_min,eta_max, "#eta_{1}",   "#eta_{2}",   "B_{2}^{+-}");
  h_jet_b2pm_thth   = createHistogram(createName(_histogramBaseName,"jet_b2pm_thth"),  th_nbins,th_min,th_max,    th_nbins,th_min,th_max,    "#theta_{1}", "#theta_{1}", "B_{2}^{+-}");
  h_jet_b2pm_jtjt   = createHistogram(createName(_histogramBaseName,"jet_b2pm_jtjt"),  jt_nbins,jt_min,jt_max,    jt_nbins,jt_min,jt_max,    "j_{T,1}",    "j_{T,2}",    "B_{2}^{+-}");
  h_jet_b2pm_zz     = createHistogram(createName(_histogramBaseName,"jet_b2pm_zz"),    z_nbins,z_min,z_max,       z_nbins,z_min,z_max,       "z_{1}",      "z_{2}",      "B_{2}^{+-}");

  h_jet_b2mp_ptpt   = createHistogram(createName(_histogramBaseName,"jet_b2mp_ptpt"),  pt_nbins,pt_min,pt_max,    pt_nbins,pt_min,pt_max,    "p_{T,1}",    "p_{T,2}",    "B_{2}^{-+}");
  h_jet_b2mp_phiphi = createHistogram(createName(_histogramBaseName,"jet_b2mp_phiphi"),phi_nbins,phi_min,phi_max, phi_nbins,phi_min,phi_max, "#varphi_{1}","#varphi_{1}","B_{2}^{-+}");
  h_jet_b2mp_etaeta = createHistogram(createName(_histogramBaseName,"jet_b2mp_etaeta"),eta_nbins,eta_min,eta_max, eta_nbins,eta_min,eta_max, "#eta_{1}",   "#eta_{2}",   "B_{2}^{-+}");
  h_jet_b2mp_thth   = createHistogram(createName(_histogramBaseName,"jet_b2mp_thth"),  th_nbins,th_min,th_max,    th_nbins,th_min,th_max,    "#theta_{1}", "#theta_{1}", "B_{2}^{-+}");
  h_jet_b2mp_jtjt   = createHistogram(createName(_histogramBaseName,"jet_b2mp_jtjt"),  jt_nbins,jt_min,jt_max,    jt_nbins,jt_min,jt_max,    "j_{T,1}",    "j_{T,2}",    "B_{2}^{-+}");
  h_jet_b2mp_zz     = createHistogram(createName(_histogramBaseName,"jet_b2mp_zz"),    z_nbins,z_min,z_max,       z_nbins,z_min,z_max,       "z_{1}",      "z_{2}",      "B_{2}^{-+}");

  h_jet_b2s_ptpt    = createHistogram(createName(_histogramBaseName,"jet_b2s_ptpt "),  pt_nbins,pt_min,pt_max,    pt_nbins,pt_min,pt_max,    "p_{T,1}",    "p_{T,2}",    "B_{2}^{s}");
  h_jet_b2s_phiphi  = createHistogram(createName(_histogramBaseName,"jet_b2s_phiphi"), phi_nbins,phi_min,phi_max, phi_nbins,phi_min,phi_max, "#varphi_{1}","#varphi_{1}","B_{2}^{s}");
  h_jet_b2s_etaeta  = createHistogram(createName(_histogramBaseName,"jet_b2s_etaeta"), eta_nbins,eta_min,eta_max, eta_nbins,eta_min,eta_max, "#eta_{1}",   "#eta_{2}",   "B_{2}^{s}");
  h_jet_b2s_thth    = createHistogram(createName(_histogramBaseName,"jet_b2s_thth"),   th_nbins,th_min,th_max,    th_nbins,th_min,th_max,    "#theta_{1}", "#theta_{1}", "B_{2}^{s}");
  h_jet_b2s_jtjt    = createHistogram(createName(_histogramBaseName,"jet_b2s_jtjt"),   jt_nbins,jt_min,jt_max,    jt_nbins,jt_min,jt_max,    "j_{T,1}",    "j_{T,2}",    "B_{2}^{s}");
  h_jet_b2s_zz      = createHistogram(createName(_histogramBaseName,"jet_b2s_zz"),     z_nbins,z_min,z_max,       z_nbins,z_min,z_max,       "z_{1}",      "z_{2}",      "B_{2}^{s}");

}

void JetPairHistosDerived::loadFrom(CapFile & inputFile)
{
  if (reportDebug(__FUNCTION__)) { printCR();}
  JetPairHistos::loadFrom(inputFile);
  h_jet_a2pp_ptpt   = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2pp_ptpt"));
  h_jet_a2pp_phiphi = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2pp_phiphi"));
  h_jet_a2pp_etaeta = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2pp_etaet"));
  h_jet_a2pp_thth   = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2pp_thth"));
  h_jet_a2pp_jtjt   = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2pp_jtjt"));
  h_jet_a2pp_zz     = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2pp_zz"));

  h_jet_a2mp_ptpt   = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2mp_ptpt"));
  h_jet_a2mp_phiphi = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2mp_phiphi"));
  h_jet_a2mp_etaeta = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2mp_etaeta"));
  h_jet_a2mp_thth   = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2mp_thth"));
  h_jet_a2mp_jtjt   = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2mp_jtjt"));
  h_jet_a2mp_zz     = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2mp_zz"));

  h_jet_a2pm_ptpt   = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2pm_ptpt"));
  h_jet_a2pm_phiphi = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2pm_phiphi"));
  h_jet_a2pm_etaeta = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2pm_etaeta"));
  h_jet_a2pm_thth   = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2pm_thth"));
  h_jet_a2pm_jtjt   = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2pm_jtjt"));
  h_jet_a2pm_zz     = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2pm_zz"));

  h_jet_a2mm_ptpt   = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2mm_ptpt"));
  h_jet_a2mm_phiphi = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2mm_phiphi"));
  h_jet_a2mm_etaeta = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2mm_etaeta"));
  h_jet_a2mm_thth   = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2mm_thth"));
  h_jet_a2mm_jtjt   = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2mm_jtjt"));
  h_jet_a2mm_zz     = importHistogram(inputFile,createName(_histogramBaseName,"jet_a2mm_zz"));

  h_jet_b2pm_ptpt   = importHistogram(inputFile,createName(_histogramBaseName,"jet_b2pm_ptpt"));
  h_jet_b2pm_phiphi = importHistogram(inputFile,createName(_histogramBaseName,"jet_b2pm_phiphi"));
  h_jet_b2pm_etaeta = importHistogram(inputFile,createName(_histogramBaseName,"jet_b2pm_etaeta"));
  h_jet_b2pm_thth   = importHistogram(inputFile,createName(_histogramBaseName,"jet_b2pm_thth"));
  h_jet_b2pm_jtjt   = importHistogram(inputFile,createName(_histogramBaseName,"jet_b2pm_jtjt"));
  h_jet_b2pm_zz     = importHistogram(inputFile,createName(_histogramBaseName,"jet_b2pm_zz"));

  h_jet_b2mp_ptpt   = importHistogram(inputFile,createName(_histogramBaseName,"jet_b2mp_ptpt"));
  h_jet_b2mp_phiphi = importHistogram(inputFile,createName(_histogramBaseName,"jet_b2mp_phiphi"));
  h_jet_b2mp_etaeta = importHistogram(inputFile,createName(_histogramBaseName,"jet_b2mp_etaeta"));
  h_jet_b2mp_thth   = importHistogram(inputFile,createName(_histogramBaseName,"jet_b2mp_thth"));
  h_jet_b2mp_jtjt   = importHistogram(inputFile,createName(_histogramBaseName,"jet_b2mp_jtjt"));
  h_jet_b2mp_zz     = importHistogram(inputFile,createName(_histogramBaseName,"jet_b2mp_zz"));

  h_jet_b2s_ptpt    = importHistogram(inputFile,createName(_histogramBaseName,"jet_b2s_ptpt "));
  h_jet_b2s_phiphi  = importHistogram(inputFile,createName(_histogramBaseName,"jet_b2s_phiphi"));
  h_jet_b2s_etaeta  = importHistogram(inputFile,createName(_histogramBaseName,"jet_b2s_etaeta"));
  h_jet_b2s_thth    = importHistogram(inputFile,createName(_histogramBaseName,"jet_b2s_thth"));
  h_jet_b2s_jtjt    = importHistogram(inputFile,createName(_histogramBaseName,"jet_b2s_jtjt"));
  h_jet_b2s_zz      = importHistogram(inputFile,createName(_histogramBaseName,"jet_b2s_zz"));

}

void JetPairHistosDerived::calculateDerivedHistograms(const JetSingleHistos & singleHistos, const JetPairHistos & pairHistos)
{
  double pIntegral = singleHistos.h_jet_n1p_phi->integral();
  double mIntegral = singleHistos.h_jet_n1m_phi->integral();
  double pScale = (pIntegral>0) ? 1.0/pIntegral : -1.0;  // -1 indicate error or no statistics
  double mScale = (mIntegral>0) ? 1.0/mIntegral : -1.0;  // -1 indicate error or no statistics

  printString("jet_a2pp");
  h_jet_a2pp_ptpt  ->add(*pairHistos.h_jet_n2pp_ptpt);   h_jet_a2pp_ptpt->scale(pScale);
  h_jet_a2pp_phiphi->add(*pairHistos.h_jet_n2pp_phiphi); h_jet_a2pp_phiphi->scale(pScale);
  h_jet_a2pp_etaeta->add(*pairHistos.h_jet_n2pp_etaeta); h_jet_a2pp_etaeta->scale(pScale);
  h_jet_a2pp_thth  ->add(*pairHistos.h_jet_n2pp_thth);   h_jet_a2pp_thth->scale(pScale);
  h_jet_a2pp_jtjt  ->add(*pairHistos.h_jet_n2pp_jtjt);   h_jet_a2pp_jtjt->scale(pScale);
  h_jet_a2pp_zz    ->add(*pairHistos.h_jet_n2pp_zz);     h_jet_a2pp_zz->scale(pScale);

  printString("jet_a2mp");
  h_jet_a2mp_ptpt  ->add(*pairHistos.h_jet_n2pm_ptpt);   h_jet_a2mp_ptpt->scale(pScale);
  h_jet_a2mp_phiphi->add(*pairHistos.h_jet_n2pm_phiphi); h_jet_a2mp_phiphi->scale(pScale);
  h_jet_a2mp_etaeta->add(*pairHistos.h_jet_n2pm_etaeta); h_jet_a2mp_etaeta->scale(pScale);
  h_jet_a2mp_thth  ->add(*pairHistos.h_jet_n2pm_thth);   h_jet_a2mp_thth->scale(pScale);
  h_jet_a2mp_jtjt  ->add(*pairHistos.h_jet_n2pm_jtjt);   h_jet_a2mp_jtjt->scale(pScale);
  h_jet_a2mp_zz    ->add(*pairHistos.h_jet_n2pm_zz);     h_jet_a2mp_zz->scale(pScale);

  printString("jet_a2pm");
  h_jet_a2pm_ptpt  ->add(*pairHistos.h_jet_n2pm_ptpt);   h_jet_a2pm_ptpt->scale(mScale);
  h_jet_a2pm_phiphi->add(*pairHistos.h_jet_n2pm_phiphi); h_jet_a2pm_phiphi->scale(mScale);
  h_jet_a2pm_etaeta->add(*pairHistos.h_jet_n2pm_etaeta); h_jet_a2pm_etaeta->scale(mScale);
  h_jet_a2pm_thth  ->add(*pairHistos.h_jet_n2pm_thth);   h_jet_a2pm_thth->scale(mScale);
  h_jet_a2pm_jtjt  ->add(*pairHistos.h_jet_n2pm_jtjt);   h_jet_a2pm_jtjt->scale(mScale);
  h_jet_a2pm_zz    ->add(*pairHistos.h_jet_n2pm_zz);     h_jet_a2pm_zz->scale(mScale);

  printString("jet_a2mm");
  h_jet_a2mm_ptpt  ->add(*pairHistos.h_jet_n2mm_ptpt);   h_jet_a2mm_ptpt->scale(mScale);
  h_jet_a2mm_phiphi->add(*pairHistos.h_jet_n2mm_phiphi); h_jet_a2mm_phiphi->scale(mScale);
  h_jet_a2mm_etaeta->add(*pairHistos.h_jet_n2mm_etaeta); h_jet_a2mm_etaeta->scale(mScale);
  h_jet_a2mm_thth  ->add(*pairHistos.h_jet_n2mm_thth);   h_jet_a2mm_thth->scale(mScale);
  h_jet_a2mm_jtjt  ->add(*pairHistos.h_jet_n2mm_jtjt);   h_jet_a2mm_jtjt->scale(mScale);
  h_jet_a2mm_zz    ->add(*pairHistos.h_jet_n2mm_zz);     h_jet_a2mm_zz->scale(mScale);

  printString("jet_b2pm");
  h_jet_b2pm_ptpt   ->setAsSumOf(*h_jet_a2pm_ptpt,*h_jet_a2mm_ptpt,1.0,-1.0);
  h_jet_b2pm_phiphi ->setAsSumOf(*h_jet_a2pm_phiphi,*h_jet_a2mm_phiphi,1.0,-1.0);
  h_jet_b2pm_etaeta ->setAsSumOf(*h_jet_a2pm_etaeta,*h_jet_a2mm_etaeta,1.0,-1.0);
  h_jet_b2pm_thth   ->setAsSumOf(*h_jet_a2pm_thth,*h_jet_a2mm_thth,1.0,-1.0);
  h_jet_b2pm_jtjt   ->setAsSumOf(*h_jet_a2pm_jtjt,*h_jet_a2mm_jtjt,1.0,-1.0);
  h_jet_b2pm_zz     ->setAsSumOf(*h_jet_a2pm_zz,*h_jet_a2mm_zz,1.0,-1.0);

  printString("jet_b2mp");
  h_jet_b2mp_ptpt   ->setAsSumOf(*h_jet_a2mp_ptpt,*h_jet_a2pp_ptpt,1.0,-1.0);
  h_jet_b2mp_phiphi ->setAsSumOf(*h_jet_a2mp_phiphi,*h_jet_a2pp_phiphi,1.0,-1.0);
  h_jet_b2mp_etaeta ->setAsSumOf(*h_jet_a2mp_etaeta,*h_jet_a2pp_etaeta,1.0,-1.0);
  h_jet_b2mp_thth   ->setAsSumOf(*h_jet_a2mp_thth,*h_jet_a2pp_thth,1.0,-1.0);
  h_jet_b2mp_jtjt   ->setAsSumOf(*h_jet_a2mp_jtjt,*h_jet_a2pp_jtjt,1.0,-1.0);
  h_jet_b2mp_zz     ->setAsSumOf(*h_jet_a2mp_zz,*h_jet_a2pp_zz,1.0,-1.0);

  printString("jet_b2s");
  h_jet_b2s_ptpt   ->setAsSumOf(*h_jet_b2mp_ptpt,*h_jet_b2pm_ptpt,0.5,0.5);
  h_jet_b2s_phiphi ->setAsSumOf(*h_jet_b2mp_phiphi,*h_jet_b2pm_phiphi,0.5,0.5);
  h_jet_b2s_etaeta ->setAsSumOf(*h_jet_b2mp_etaeta,*h_jet_b2pm_etaeta,0.5,0.5);
  h_jet_b2s_thth   ->setAsSumOf(*h_jet_b2mp_thth,*h_jet_b2pm_thth,0.5,0.5);
  h_jet_b2s_jtjt   ->setAsSumOf(*h_jet_b2mp_jtjt,*h_jet_b2pm_jtjt,0.5,0.5);
  h_jet_b2s_zz     ->setAsSumOf(*h_jet_b2mp_zz,*h_jet_b2pm_zz,0.5,0.5);
}


} // namespace CAP
