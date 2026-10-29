/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "NuDynHistos.hpp"
#include "Exceptions.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "Configuration.hpp"
#include "Task.hpp"
#include "NameHelpers.hpp"


namespace CAP
{

  NuDynHistos::NuDynHistos()
  :
  HistogramFolder(),
  h_evt(nullptr),
  h_f1_1_evt(nullptr),
  h_f1_2_evt(nullptr),
  h_f2_11_evt(nullptr),
  h_f2_12_evt(nullptr),
  h_f2_22_evt(nullptr),
  evtName(),
  evt_nbins(0),
  evt_min(0),
  evt_max(0),
  h_F2_11_evt(nullptr),
  h_F2_12_evt(nullptr),
  h_F2_22_evt(nullptr),
  h_R2_11_evt(nullptr),
  h_R2_12_evt(nullptr),
  h_R2_22_evt(nullptr),
  h_nudyn_evt(nullptr)
  {
  appendClassName("NuDynHistos");
  setName("NuDynHistos");
  setTitle("NuDynHistos");
  }

  NuDynHistos::NuDynHistos(const NuDynHistos & source)
  :
  HistogramFolder(source),
  h_evt(nullptr),
  h_f1_1_evt(nullptr),
  h_f1_2_evt(nullptr),
  h_f2_11_evt(nullptr),
  h_f2_12_evt(nullptr),
  h_f2_22_evt(nullptr),
  evtName(source.evtName),
  evt_nbins(source.evt_nbins),
  evt_min(source.evt_min),
  evt_max(source.evt_max),
  h_F2_11_evt(nullptr),
  h_F2_12_evt(nullptr),
  h_F2_22_evt(nullptr),
  h_R2_11_evt(nullptr),
  h_R2_12_evt(nullptr),
  h_R2_22_evt(nullptr),
  h_nudyn_evt(nullptr)
  {
  cloneB(source);
  }

  NuDynHistos & NuDynHistos::operator=(const NuDynHistos & rhs)
  {
  if (this!=&rhs)
    {
    HistogramFolder::operator=(rhs),
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
  h_evt         = histogramLike(source.h_evt);
  h_f1_1_evt    = histogramLike(source.h_f1_1_evt);
  h_f1_2_evt    = histogramLike(source.h_f1_2_evt);
  h_f2_11_evt   = histogramLike(source.h_f2_11_evt);
  h_f2_12_evt   = histogramLike(source.h_f2_12_evt);
  h_f2_22_evt   = histogramLike(source.h_f2_22_evt);
  h_F2_11_evt = histogramLike(source.h_F2_11_evt);
  h_F2_12_evt = histogramLike(source.h_F2_12_evt);
  h_F2_22_evt = histogramLike(source.h_F2_22_evt);
  h_R2_11_evt = histogramLike(source.h_R2_11_evt);
  h_R2_12_evt = histogramLike(source.h_R2_12_evt);
  h_R2_22_evt = histogramLike(source.h_R2_22_evt);
  h_nudyn_evt = histogramLike(source.h_nudyn_evt);
  }


  void NuDynHistos::addDefaultConfiguration(Configuration & configuration, const String & taskName)
  {
  const String type = "HISTOGRAM";
  configuration.addProperty(createKey(taskName,type,"evtName"),  String("Mult0"));
  configuration.addProperty(createKey(taskName,type,"evt_nbins"),200);
  configuration.addProperty(createKey(taskName,type,"evt_min"),  0.0);
  configuration.addProperty(createKey(taskName,type,"evt_max"),  200.0);
  }

  void NuDynHistos::configure(const String & taskName,
                              const String & type,
                              const Configuration & configuration,
                              unsigned int index __attribute__((unused)) )
  {
  // the binning is in the HISTOGRAM group of the task, whatever the list of the folder (HISTOGRAM_1, ...)
  String objectType = type.beginsWith("HISTOGRAM") ? String("HISTOGRAM") : type;
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

  void NuDynHistos::loadFrom(const Folder & inputFile)
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  h_evt       = importHistogram(inputFile,createName(_histogramBaseName,"mult"));
  h_f1_1_evt  = importProfile(inputFile,createName(_histogramBaseName,"f1_1"));
  h_f1_2_evt  = importProfile(inputFile,createName(_histogramBaseName,"f1_2"));
  h_f2_11_evt = importProfile(inputFile,createName(_histogramBaseName,"f2_11"));
  h_f2_12_evt = importProfile(inputFile,createName(_histogramBaseName,"f2_12"));
  h_f2_22_evt = importProfile(inputFile,createName(_histogramBaseName,"f2_22"));
  // derived histograms: present once NuDynDerivedCalculator has run
  if (inputFile.contains(createName(_histogramBaseName,"F2_11"))) loadDerivedFrom(inputFile);
  }

  void NuDynHistos::fill(double mult, double n1_1, double n1_2, double n2_11, double n2_12, double n2_22)
  {
  h_evt->fill(mult);
  h_f1_1_evt->fill(mult,n1_1);
  h_f1_2_evt->fill(mult,n1_2);
  h_f2_11_evt->fill(mult,n2_11);
  h_f2_12_evt->fill(mult,n2_12);
  h_f2_22_evt->fill(mult,n2_22);
  }

  void NuDynHistos::scaleObject(double factor)
  {
  h_evt->scale(factor);
  }


  void NuDynHistos::createDerived()
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  h_F2_11_evt = createHistogram(createName(_histogramBaseName,"F2_11"), evt_nbins,evt_min,evt_max,"evtClass","F2_{11}");
  h_F2_12_evt = createHistogram(createName(_histogramBaseName,"F2_12"), evt_nbins,evt_min,evt_max,"evtClass","F2_{12}");
  h_F2_22_evt = createHistogram(createName(_histogramBaseName,"F2_22"), evt_nbins,evt_min,evt_max,"evtClass","F2_{22}");
  h_R2_11_evt = createHistogram(createName(_histogramBaseName,"R2_11"), evt_nbins,evt_min,evt_max,"evtClass","R2_{11}");
  h_R2_12_evt = createHistogram(createName(_histogramBaseName,"R2_12"), evt_nbins,evt_min,evt_max,"evtClass","R2_{12}");
  h_R2_22_evt = createHistogram(createName(_histogramBaseName,"R2_22"), evt_nbins,evt_min,evt_max,"evtClass","R2_{22}");
  h_nudyn_evt = createHistogram(createName(_histogramBaseName,"Nu_dyn"),evt_nbins,evt_min,evt_max,"evtClass","#nu_{Dyn}");
}

  void NuDynHistos::loadDerivedFrom(const Folder & inputFile)
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  h_F2_11_evt = importHistogram(inputFile,createName(_histogramBaseName,"F2_11"));
  h_F2_12_evt = importHistogram(inputFile,createName(_histogramBaseName,"F2_12"));
  h_F2_22_evt = importHistogram(inputFile,createName(_histogramBaseName,"F2_22"));
  h_R2_11_evt = importHistogram(inputFile,createName(_histogramBaseName,"R2_11"));
  h_R2_12_evt = importHistogram(inputFile,createName(_histogramBaseName,"R2_12"));
  h_R2_22_evt = importHistogram(inputFile,createName(_histogramBaseName,"R2_22"));
  h_nudyn_evt = importHistogram(inputFile,createName(_histogramBaseName,"Nu_dyn"));
}

  void NuDynHistos::calculateDerived()
  {
  if (!hasDerived()) throw Exception("The derived histograms were not created (createDerived())",__FUNCTION__);
  if (reportDebug(__FUNCTION__)) { printCR(); }
  calculateF2R2(h_f1_1_evt,h_f1_1_evt,h_f2_11_evt,h_F2_11_evt,h_R2_11_evt);
  calculateF2R2(h_f1_1_evt,h_f1_2_evt,h_f2_12_evt,h_F2_12_evt,h_R2_12_evt);
  calculateF2R2(h_f1_2_evt,h_f1_2_evt,h_f2_22_evt,h_F2_22_evt,h_R2_22_evt);
  calculateNudyn(h_R2_11_evt,h_R2_12_evt,h_R2_12_evt,h_R2_22_evt,h_nudyn_evt);
}

} // namespace CAP

