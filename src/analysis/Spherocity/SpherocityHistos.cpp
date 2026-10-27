/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "Particle.hpp"
#include "SpherocityHistos.hpp"
#include "GlobalHistos.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "Task.hpp"
#include "Configuration.hpp"


namespace CAP
{

  SpherocityHistos::SpherocityHistos()
  :
  HistogramFolder(),
  fillS0(true),
  fillS1(false),
  fillS1VsS0(false),
  h_s0(nullptr),
  h_s1(),
  h_s1VsS0()
  {
  appendClassName("SpherocityHistos");
  setName("SpherocityHistos");
  setTitle("SpherocityHistos");
  }

  SpherocityHistos::SpherocityHistos(const SpherocityHistos & source)
  :
  HistogramFolder(source),
  fillS0(source.fillS0),
  fillS1(source.fillS1),
  fillS1VsS0(source.fillS1VsS0),
  h_s0(),
  h_s1(),
  h_s1VsS0()
  {
  cloneB(source);
  }

  SpherocityHistos & SpherocityHistos::operator=(const SpherocityHistos & rhs)
  {
  if (this!=&rhs)
    {
    HistogramFolder::operator=(rhs);
    fillS0     = rhs.fillS0;
    fillS1     = rhs.fillS1;
    fillS1VsS0 = rhs.fillS1VsS0;
    cloneB(rhs);
    }
  return *this;
  }

  void SpherocityHistos::cloneB(const SpherocityHistos & source)
  {
  if (reportDebug(__FUNCTION__)) { printCR(); }
  if (source.h_s0)     h_s0 = source.h_s0;
  if (source.h_s1)     h_s1 = source.h_s1;
  if (source.h_s1VsS0) h_s1VsS0 = source.h_s1VsS0;
  }

  void SpherocityHistos::configure(const String & taskName,
                                   const String & type,
                                   const Configuration & configuration,
                                   unsigned int index)
  {
  // the binning is in the HISTOGRAM group of the task, whatever the list of the folder (HISTOGRAM_1, ...)
  String objectType = type.beginsWith("HISTOGRAM") ? String("HISTOGRAM") : type;
  HistogramFolder::configure(taskName,type,configuration,index);
  fillS0     = configuration.valueBool(createKey(taskName,objectType,"FillS0"));
  fillS1     = configuration.valueBool(createKey(taskName,objectType,"FillS1"));
  fillS1VsS0 = configuration.valueBool(createKey(taskName,objectType,"FillS1VsS0"));
  spherocity_nbins = configuration.valueInt(   createKey(taskName,objectType,"spherocity_nbins"));
  spherocity_min   = configuration.valueDouble(createKey(taskName,objectType,"spherocity_min"));
  spherocity_max   = configuration.valueDouble(createKey(taskName,objectType,"spherocity_max"));

  if (reportDebug(__FUNCTION__))
    {
    printCR();
    printValue(createKey(taskName,objectType,"fillS0"),fillS0);
    printValue(createKey(taskName,objectType,"fillS1"),fillS1);
    printValue(createKey(taskName,objectType,"fillS1VsS0"),fillS1VsS0);
    printValue(createKey(taskName,objectType,"spherocity_nbins"),spherocity_nbins);
    printValue(createKey(taskName,objectType,"spherocity_min"),spherocity_min);
    printValue(createKey(taskName,objectType,"spherocity_max"),spherocity_max);
    }
  }

  void SpherocityHistos::create()
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  if (fillS0) h_s0 = createHistogram(createName(_histogramBaseName,"S0"), spherocity_nbins,spherocity_min,spherocity_max,"S_{0}","N");
  if (fillS1) h_s1 = createHistogram(createName(_histogramBaseName,"S1"), spherocity_nbins,spherocity_min,spherocity_max,"S_{1}","N");
  if (fillS1VsS0)
    h_s1VsS0  =  createHistogram(createName(_histogramBaseName,"S1VsS0"),
                                 spherocity_nbins,spherocity_min,spherocity_max,spherocity_nbins,spherocity_min,spherocity_max,
                                 "S_{0}","S_{1}","N");

  }

  void SpherocityHistos::loadFrom(const Folder & inputFile)
  {
  if (fillS0) h_s0 = importHistogram(inputFile, createName(_histogramBaseName,"S0"));
  if (fillS1) h_s1 = importHistogram(inputFile, createName(_histogramBaseName,"S1"));
  if (fillS1VsS0) h_s1VsS0 =  importHistogram(inputFile, createName(_histogramBaseName,"S1VsS0"));
  }

  //!
  //! Fiil  single particle histograms of this class with the particles contained in the given list.
  //!
  void SpherocityHistos::fill(double s0, double s1, double weight)
  {
  if (fillS0) h_s0->fill(s0,weight);
  if (fillS1) h_s1->fill(s1,weight);
  if (fillS1VsS0) h_s1VsS0->fill(s0,s1,weight);
  }

} // namespace CAP
