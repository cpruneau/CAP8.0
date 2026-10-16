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
#include "JetHistosDerived.hpp"
#include "PrintHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "Configuration.hpp"
#include "NameHelpers.hpp"
#include "Task.hpp"

ClassImp(CAP::JetHistosDerived);

namespace CAP
{
JetHistosDerived::JetHistosDerived()
:
JetHistos(),
h_jet_R200_pt(nullptr),
h_jet_R2pp_pt(nullptr),
h_jet_R2pm_pt(nullptr),
h_jet_R2mm_pt(nullptr),
h_jet_nudyn_pt(nullptr),
h_jet_netQrms_pt(nullptr)
{  }


JetHistosDerived::JetHistosDerived(const JetHistosDerived & source)
:
JetHistos(source),
h_jet_R200_pt(nullptr),
h_jet_R2pp_pt(nullptr),
h_jet_R2pm_pt(nullptr),
h_jet_R2mm_pt(nullptr),
h_jet_nudyn_pt(nullptr),
h_jet_netQrms_pt(nullptr)
{ }

JetHistosDerived & JetHistosDerived::operator=(const JetHistosDerived & rhs)
{
  if (this!=&rhs)
    {
    JetHistos::operator=(rhs);
    cloneD(rhs);
    }
  return *this;
}

void JetHistosDerived::cloneD(const JetHistosDerived & source)
{
  h_jet_R200_pt    = histogramLike(source.h_jet_R200_pt);
  h_jet_R2pp_pt    = histogramLike(source.h_jet_R2pp_pt);
  h_jet_R2pm_pt    = histogramLike(source.h_jet_R2pm_pt);
  h_jet_R2mm_pt    = histogramLike(source.h_jet_R2mm_pt);
  h_jet_nudyn_pt   = histogramLike(source.h_jet_nudyn_pt);
  h_jet_netQrms_pt = histogramLike(source.h_jet_netQrms_pt);
}


void JetHistosDerived::configure(const String & taskName,
                                       const String & objectType,
                                       const Configuration & configuration,
                                       unsigned int index)
{
  JetHistos::configure(taskName,objectType,configuration,index);
}


void JetHistosDerived::create()
{
  if (reportDebug(__FUNCTION__)) { printCR();}
  h_jet_R200_pt    = createHistogram(createName(_histogramBaseName,"jet_R200_pt"),    nBins_jet_pt,  min_jet_pt,  max_jet_pt,   "p_{T} (GeV/c)","R_{2}^{00}");
  h_jet_R2pp_pt    = createHistogram(createName(_histogramBaseName,"jet_R2pp_pt"),    nBins_jet_pt,  min_jet_pt,  max_jet_pt,   "p_{T} (GeV/c)","R_{2}^{00}");
  h_jet_R2pm_pt    = createHistogram(createName(_histogramBaseName,"jet_R2pm_pt"),    nBins_jet_pt,  min_jet_pt,  max_jet_pt,   "p_{T} (GeV/c)","R_{2}^{00}");
  h_jet_R2mm_pt    = createHistogram(createName(_histogramBaseName,"jet_R2mm_pt"),    nBins_jet_pt,  min_jet_pt,  max_jet_pt,   "p_{T} (GeV/c)","R_{2}^{00}");
  h_jet_nudyn_pt   = createHistogram(createName(_histogramBaseName,"jet_nudyn_pt"),   nBins_jet_pt,  min_jet_pt,  max_jet_pt,   "p_{T} (GeV/c)","#nu_{dyn}");
  h_jet_netQrms_pt = createHistogram(createName(_histogramBaseName,"jet_netQrms_pt"), nBins_jet_pt,  min_jet_pt,  max_jet_pt,   "p_{T} (GeV/c)","<#Delta Q^2>");
}

void JetHistosDerived::loadFrom(CapFile & inputFile)
{
  if (reportDebug(__FUNCTION__)) { printCR();}
  JetHistos::loadFrom(inputFile);
  h_jet_R200_pt    = importHistogram(inputFile,createName(_histogramBaseName,"jet_R200_pt"));
  h_jet_R2pp_pt    = importHistogram(inputFile,createName(_histogramBaseName,"jet_R2pp_pt"));
  h_jet_R2pm_pt    = importHistogram(inputFile,createName(_histogramBaseName,"jet_R2pm_pt"));
  h_jet_R2mm_pt    = importHistogram(inputFile,createName(_histogramBaseName,"jet_R2mm_pt"));
  h_jet_nudyn_pt   = importHistogram(inputFile,createName(_histogramBaseName,"jet_nudyn_pt"));
  h_jet_netQrms_pt = importHistogram(inputFile,createName(_histogramBaseName,"jet_netQrms_pt"));
}

void calculateR2_P1P1P1H1(const Profile * n2_12, const Profile * n1_1, const Profile * n1_2, Histogram * R2_12)
{
  if (!ptrExist(__FUNCTION__,n2_12,n1_1,n1_2,R2_12))  throw NullPointerException(__FUNCTION__);
  if (!sameDimensions(__FUNCTION__,n2_12,n1_1,n1_2,R2_12))  throw IncompatibleHistogramException(__FUNCTION__);
  int n_x = n2_12->nBins(0);
  double v_12,v_1,v_2,r2,er2;
  for (int i1=1; i1<=n_x;++i1)
  {
    v_12  = n2_12->binValueAt(i1-1); //ev_12 = n2_12->GetBinError(i1);
    v_1   = n1_1->binValueAt(i1-1);  //ev_1  = n1_1->GetBinError(i1);
    v_2   = n1_2->binValueAt(i1-1);  //ev_2  = n1_2->GetBinError(i1);
    if (v_1>0  &&  v_2>0)
    {
      r2   = v_12/(v_1 * v_2) - 1.0;
      er2  = 0;
    }
    else
    {
    r2 = 0; er2 = 0;
    }
    R2_12->setBinValueAt(unsigned((i1)-1),r2); R2_12->setBinUncertaintyAt(unsigned((i1)-1),er2);
  }
}

void calculateRms(const Histogram * valueSqVsX, const Histogram * valueVsX, Histogram * rmsVsX)
{
  int n_x = valueSqVsX->nBins(0);
  double vSq, v, rmsSq, rms;
  for (int i1=1; i1<=n_x;++i1)
  {
    vSq  = valueSqVsX->binValueAt((i1)-1);
    v    = valueVsX->binValueAt((i1)-1);
    rmsSq  = vSq - v*v;
    if (rms>0.0)
      rms = std::sqrt(rmsSq);
    else
      rms = 0.0;
    rmsVsX->setBinValueAt(unsigned((i1)-1),rms); rmsVsX->setBinUncertaintyAt(unsigned((i1)-1),0.0);
  }
}

void JetHistosDerived::calculateDerivedHistograms(const JetHistos & baseHistos)
{
  calculateR2_P1P1P1H1(baseHistos.p_jet_n0Sq,baseHistos.p_jet_n0,baseHistos.p_jet_n0,h_jet_R200_pt);
  calculateR2_P1P1P1H1(baseHistos.p_jet_npSq,baseHistos.p_jet_np,baseHistos.p_jet_np,h_jet_R2pp_pt);
  calculateR2_P1P1P1H1(baseHistos.p_jet_nmSq,baseHistos.p_jet_nm,baseHistos.p_jet_nm,h_jet_R2mm_pt);
  calculateR2_P1P1P1H1(baseHistos.p_jet_npnm,baseHistos.p_jet_np,baseHistos.p_jet_nm,h_jet_R2pm_pt);
  calculateNudyn(h_jet_R2pp_pt,h_jet_R2pm_pt,h_jet_R2pm_pt,h_jet_R2mm_pt,h_jet_nudyn_pt);
  calculateRms(baseHistos.p_jet_netQSq,baseHistos.p_jet_netQ,h_jet_netQrms_pt);
}

} // namespace CAP
