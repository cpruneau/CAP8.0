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
#include "NuDynHistos.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "RootHistogramHelpers.hpp"
#include "Configuration.hpp"
#include "Task.hpp"
#include "NameHelpers.hpp"

ClassImp(CAP::NuDynHistos);

namespace CAP
{

  NuDynHistos::NuDynHistos()
  :
  HistogramGroup(),
  h_evt(nullptr),
  h_f1_1_evt(nullptr),
  h_f1_2_evt(nullptr),
  h_f2_11_evt(nullptr),
  h_f2_12_evt(nullptr),
  h_f2_22_evt(nullptr),
  evtName(),
  evt_nbins(0),
  evt_min(0),
  evt_max(0)
  {
  appendClassName("NuDynHistos");
  setName("NuDynHistos");
  setTitle("NuDynHistos");
  }

  NuDynHistos::NuDynHistos(const NuDynHistos & source)
  :
  HistogramGroup(source),
  h_evt(nullptr),
  h_f1_1_evt(nullptr),
  h_f1_2_evt(nullptr),
  h_f2_11_evt(nullptr),
  h_f2_12_evt(nullptr),
  h_f2_22_evt(nullptr),
  evtName(source.evtName),
  evt_nbins(source.evt_nbins),
  evt_min(source.evt_min),
  evt_max(source.evt_max)
  {
  cloneB(source);
  }

  NuDynHistos & NuDynHistos::operator=(const NuDynHistos & rhs)
  {
  if (this!=&rhs)
    {
    HistogramGroup::operator=(rhs),
    cloneB(rhs);
    evtName  = rhs.evtName;
    evt_nbins = rhs.evt_nbins;
    evt_min   = rhs.evt_min;
    evt_max   = rhs.evt_max;
    }
  return *this;
  }

  void NuDynHistos::cloneB(const NuDynHistos & source)
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  h_evt         = safeCloneH1(source.h_evt);
  h_f1_1_evt    = safeCloneProfile(source.h_f1_1_evt);
  h_f1_2_evt    = safeCloneProfile(source.h_f1_2_evt);
  h_f2_11_evt   = safeCloneProfile(source.h_f2_11_evt);
  h_f2_12_evt   = safeCloneProfile(source.h_f2_12_evt);
  h_f2_22_evt   = safeCloneProfile(source.h_f2_22_evt);
  }


  void NuDynHistos::configure(const String & taskName,
                              const String & objectType,
                              const Configuration & configuration,
                              unsigned int index __attribute__((unused)) )
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  evtName   = configuration.valueString(createKey(taskName,objectType,"evtName"));
  evt_nbins = configuration.valueInt(   createKey(taskName,objectType,"evt_nbins"));
  evt_min   = configuration.valueDouble(createKey(taskName,objectType,"evt_min"));
  evt_max   = configuration.valueDouble(createKey(taskName,objectType,"evt_max"));
  }

  void NuDynHistos::create()
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  h_evt       = createHistogram(createName(_histogramBaseName,"mult"),evt_nbins,evt_min,evt_max,"M","Counts");
  h_f1_1_evt  = createProfile(createName(_histogramBaseName,"f1_1"),evt_nbins,evt_min,evt_max,"EvtClass","f_{1}^{1}");
  h_f1_2_evt  = createProfile(createName(_histogramBaseName,"f1_2"),evt_nbins,evt_min,evt_max,"EvtClass","f_{1}^{2}");
  h_f2_11_evt = createProfile(createName(_histogramBaseName,"f2_11"),evt_nbins,evt_min,evt_max,"EvtClass","f_{2}^{11}");
  h_f2_12_evt = createProfile(createName(_histogramBaseName,"f2_12"),evt_nbins,evt_min,evt_max,"EvtClass","f_{2}^{12}");
  h_f2_22_evt = createProfile(createName(_histogramBaseName,"f2_22"),evt_nbins,evt_min,evt_max,"EvtClass","f_{2}^{22}");
  }

  void NuDynHistos::loadFrom(TFile & inputFile)
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  h_evt       = importH1(inputFile,createName(_histogramBaseName,"mult"));
  h_f1_1_evt  = importProfile(inputFile,createName(_histogramBaseName,"f1_1"));
  h_f1_2_evt  = importProfile(inputFile,createName(_histogramBaseName,"f1_2"));
  h_f2_11_evt = importProfile(inputFile,createName(_histogramBaseName,"f2_11"));
  h_f2_12_evt = importProfile(inputFile,createName(_histogramBaseName,"f2_12"));
  h_f2_22_evt = importProfile(inputFile,createName(_histogramBaseName,"f2_22"));
  }

  void NuDynHistos::fill(double mult, double n1_1, double n1_2, double n2_11, double n2_12, double n2_22)
  {
  h_evt->Fill(mult);
  h_f1_1_evt->Fill(mult,n1_1);
  h_f1_2_evt->Fill(mult,n1_2);
  h_f2_11_evt->Fill(mult,n2_11);
  h_f2_12_evt->Fill(mult,n2_12);
  h_f2_22_evt->Fill(mult,n2_22);
  }

  void NuDynHistos::scaleObject(double factor)
  {
  h_evt->Scale(factor);
  }

} // namespace CAP

