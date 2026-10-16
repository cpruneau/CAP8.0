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
#include "GlobalHistos.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "Task.hpp"
#include "NameHelpers.hpp"
#include "Configuration.hpp"

ClassImp(CAP::GlobalHistos);

namespace CAP
{
  
  GlobalHistos::GlobalHistos()
  :
  HistogramFolder(),
  h_n(nullptr),
  h_e(nullptr),
  h_q(nullptr),
  h_s(nullptr),
  h_b(nullptr),
  h_ptSum(nullptr),
  h_ptAvg(nullptr),
  p_eVsN(nullptr),
  p_qVsN(nullptr),
  p_sVsN(nullptr),
  p_bVsN(nullptr),
  p_ptSumVsN(nullptr),
  p_ptAvgVsN(nullptr),
  n_nbins(0),
  e_nbins(0),
  q_nbins(0),
  b_nbins(0),
  ptSum_nbins(0),
  s_nbins(0),
  ptAvg_nbins(0),
  n_min(0.0),
  n_max(0.0),
  e_min(0.0),
  e_max(0.0),
  q_min(0.0),
  q_max(0.0),
  b_min(0.0),
  b_max(0.0),
  s_min(0.0),
  s_max(0.0),
  ptSum_min(0.0),
  ptSum_max(0.0),
  ptAvg_min(0.0),
  ptAvg_max(0.0)
  { }
  
  GlobalHistos::GlobalHistos(const GlobalHistos & source)
  :
  HistogramFolder(),
  h_n(nullptr),
  h_e(nullptr),
  h_q(nullptr),
  h_s(nullptr),
  h_b(nullptr),
  h_ptSum(nullptr),
  h_ptAvg(nullptr),
  p_eVsN(nullptr),
  p_qVsN(nullptr),
  p_sVsN(nullptr),
  p_bVsN(nullptr),
  p_ptSumVsN(nullptr),
  p_ptAvgVsN(nullptr),
  n_nbins(source.n_nbins),
  e_nbins(source.e_nbins),
  q_nbins(source.q_nbins),
  b_nbins(source.b_nbins),
  ptSum_nbins(source.ptSum_nbins),
  s_nbins(source.s_nbins),
  ptAvg_nbins(source.ptAvg_nbins),
  n_min(source.n_min),
  n_max(source.n_max),
  e_min(source.e_min),
  e_max(source.e_max),
  q_min(source.q_min),
  q_max(source.q_max),
  b_min(source.b_min),
  b_max(source.b_max),
  s_min(source.s_min),
  s_max(source.s_max),
  ptSum_min(source.ptSum_min),
  ptSum_max(source.ptSum_max),
  ptAvg_min(source.ptAvg_min),
  ptAvg_max(source.ptAvg_max)
  {
  cloneB(source);
  }
  
  GlobalHistos & GlobalHistos::operator=(const GlobalHistos & rhs)
  {
  if (this!=&rhs)
    {
    HistogramFolder::operator=(rhs);
    cloneB(rhs);
    n_nbins = rhs.n_nbins;
    e_nbins = rhs.e_nbins;
    q_nbins = rhs.q_nbins;
    b_nbins = rhs.b_nbins;
    ptSum_nbins = rhs.ptSum_nbins;
    s_nbins = rhs.s_nbins;
    ptAvg_nbins = rhs.ptAvg_nbins;
    n_min = rhs.n_min;
    n_max = rhs.n_max;
    e_min = rhs.e_min;
    e_max = rhs.e_max;
    q_min = rhs.q_min;
    q_max = rhs.q_max;
    b_min = rhs.b_min;
    b_max = rhs.b_max;
    s_min = rhs.s_min;
    s_max = rhs.s_max;
    ptSum_min = rhs.ptSum_min;
    ptSum_max = rhs.ptSum_max;
    ptAvg_min = rhs.ptAvg_min;
    ptAvg_max = rhs.ptAvg_max;
    }
  return *this;
  }
  
  
  void GlobalHistos::cloneB(const GlobalHistos & group)
  {
  h_n  = histogramLike(group.h_n);
  h_e  = histogramLike(group.h_e);
  h_q  = histogramLike(group.h_q);
  h_s  = histogramLike(group.h_s);
  h_b  = histogramLike(group.h_b);
  h_ptSum  = histogramLike(group.h_ptSum);
  h_ptAvg  = histogramLike(group.h_ptAvg);
  p_eVsN   = histogramLike(group.p_eVsN);
  p_qVsN   = histogramLike(group.p_qVsN);
  p_sVsN   = histogramLike(group.p_sVsN);
  p_bVsN   = histogramLike(group.p_bVsN);
  p_ptSumVsN  = histogramLike(group.p_ptSumVsN);
  p_ptAvgVsN  = histogramLike(group.p_ptAvgVsN);
  }
  
  void GlobalHistos::configure(const String & taskName,
                               const String & objectType,
                               const Configuration & configuration,
                               unsigned int index)
  {
  HistogramFolder::configure(taskName,objectType,configuration,index);
  n_nbins     = configuration.valueInt(   createKey(taskName,objectType,index,"n_nbins"));
  n_min       = configuration.valueDouble(createKey(taskName,objectType,index,"n_min"));
  n_max       = configuration.valueDouble(createKey(taskName,objectType,index,"n_max"));
  e_nbins     = configuration.valueInt(   createKey(taskName,objectType,index,"e_nbins"));
  e_min       = configuration.valueDouble(createKey(taskName,objectType,index,"e_min"));
  e_max       = configuration.valueDouble(createKey(taskName,objectType,index,"e_max"));
  q_nbins     = configuration.valueInt(   createKey(taskName,objectType,index,"q_nbins"));
  q_min       = configuration.valueDouble(createKey(taskName,objectType,index,"q_min"));
  q_max       = configuration.valueDouble(createKey(taskName,objectType,index,"q_max"));
  b_nbins     = configuration.valueInt(   createKey(taskName,objectType,index,"b_Bins"));
  b_min       = configuration.valueDouble(createKey(taskName,objectType,index,"b_min"));
  b_max       = configuration.valueDouble(createKey(taskName,objectType,index,"b_max"));
  s_nbins     = configuration.valueInt(   createKey(taskName,objectType,index,"s_nbins"));
  s_min       = configuration.valueDouble(createKey(taskName,objectType,index,"s_min"));
  s_max       = configuration.valueDouble(createKey(taskName,objectType,index,"s_max"));
  ptSum_nbins = configuration.valueInt(   createKey(taskName,objectType,index,"ptSum_nbins"));
  ptSum_min   = configuration.valueDouble(createKey(taskName,objectType,index,"ptSum_min"));
  ptSum_max   = configuration.valueDouble(createKey(taskName,objectType,index,"ptSum_max"));
  ptAvg_nbins = configuration.valueInt(   createKey(taskName,objectType,index,"ptAvg_nbins"));
  ptAvg_min   = configuration.valueDouble(createKey(taskName,objectType,index,"ptAvg_min"));
  ptAvg_max   = configuration.valueDouble(createKey(taskName,objectType,index,"ptAvg_max"));
  if (reportDebug(__FUNCTION__))
    {
    printCR();
    //printValue("Histo Base Name",bn);
    printValue("n_nbins",    n_nbins);
    printValue("n_min",      n_min);
    printValue("n_max",      n_max);
    printValue("e_nbins",    e_nbins);
    printValue("e_min",      e_min);
    printValue("e_max",      e_max);
    printValue("q_nbins",    q_nbins);
    printValue("q_min",      q_min);
    printValue("q_max",      q_max);
    printValue("b_nbins",    b_nbins);
    printValue("b_min",      b_min);
    printValue("b_max",      b_max);
    printValue("s_nbins",    s_nbins);
    printValue("s_min",      s_min);
    printValue("s_max",      s_max);
    printValue("ptSum_nbins",ptSum_nbins);
    printValue("ptSum_min",  ptSum_min );
    printValue("ptSum_max",  ptSum_max );
    printValue("ptAvg_nbins",ptAvg_nbins);
    printValue("ptAvg_min",  ptAvg_min);
    printValue("ptAvg_max",  ptAvg_max);
    printCR();
    }
  }
  
  // for now use the same boundaries for eta and y histogram
  void GlobalHistos::create()
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  h_n = createHistogram(createName(_histogramBaseName,"n"), n_nbins, n_min,  n_max, "n","N");
  h_e = createHistogram(createName(_histogramBaseName,"e"), e_nbins, e_min,  e_max, "e","N");
  h_q = createHistogram(createName(_histogramBaseName,"q"), q_nbins, q_min,  q_max, "q","N");
  h_s = createHistogram(createName(_histogramBaseName,"s"), s_nbins, s_min,  s_max, "s","N");
  h_b = createHistogram(createName(_histogramBaseName,"b"), b_nbins, b_min,  b_max, "b","N");
  h_ptSum = createHistogram(createName(_histogramBaseName,"ptSum"), ptSum_nbins, ptSum_min,  ptSum_max, "ptSum","N");
  h_ptAvg = createHistogram(createName(_histogramBaseName,"ptAvg"), ptAvg_nbins, ptAvg_min,  ptAvg_max, "ptAvg","N");
  p_eVsN  = createProfile(createName(_histogramBaseName,String("n"),String("eProf")),n_nbins, n_min,  n_max, "n", "e");
  p_qVsN  = createProfile(createName(_histogramBaseName,String("n"),String("qProf")),n_nbins, n_min,  n_max, "n", "q");
  p_sVsN  = createProfile(createName(_histogramBaseName,String("n"),String("sProf")),n_nbins, n_min,  n_max, "n", "s");
  p_bVsN  = createProfile(createName(_histogramBaseName,String("n"),String("bProf")),n_nbins, n_min,  n_max, "n", "b");
  p_ptSumVsN = createProfile(createName(_histogramBaseName,String("n"),String("ptSumProf")),n_nbins, n_min,  n_max, "n", "ptSum");
  p_ptAvgVsN = createProfile(createName(_histogramBaseName,String("n"),String("ptAvgProf")),n_nbins, n_min,  n_max, "n", "ptAvg");
  }
  
  void GlobalHistos::loadFrom(CapFile & inputFile)
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  h_n  = importHistogram(inputFile, createName(_histogramBaseName,"n"));
  h_e  = importHistogram(inputFile, createName(_histogramBaseName,"e"));
  h_q  = importHistogram(inputFile, createName(_histogramBaseName,"q"));
  h_s  = importHistogram(inputFile, createName(_histogramBaseName,"s"));
  h_b  = importHistogram(inputFile, createName(_histogramBaseName,"b"));
  h_ptSum  = importHistogram(inputFile, createName(_histogramBaseName,"ptSum"));
  h_ptAvg  = importHistogram(inputFile, createName(_histogramBaseName,"ptAvg"));
  p_eVsN   = importProfile(inputFile, createName(_histogramBaseName,"n","eProf"));
  p_qVsN   = importProfile(inputFile, createName(_histogramBaseName,"n","qProf"));
  p_sVsN   = importProfile(inputFile, createName(_histogramBaseName,"n","sProf"));
  p_bVsN   = importProfile(inputFile, createName(_histogramBaseName,"n","bProf"));
  p_ptSumVsN  = importProfile(inputFile, createName(_histogramBaseName,"n","ptSumProf"));
  p_ptAvgVsN  = importProfile(inputFile, createName(_histogramBaseName,"n","ptAvgProf"));
  }
  
  void GlobalHistos::fill(double n,
                          double ptSum,
                          double e,
                          double q,
                          double s,
                          double b,
                          double weight)
  {
  h_n ->fill(n ,weight);
  h_e ->fill(e ,weight);
  h_q ->fill(q ,weight);
  h_s ->fill(s ,weight);
  h_b ->fill(b ,weight);
  h_ptSum ->fill(ptSum, weight);
  if (n>0)
    {
    double ptAvg = ptSum/n;
    h_ptAvg ->fill(ptAvg, weight);
    p_eVsN  ->fill(n,e );
    p_qVsN  ->fill(n,q );
    p_sVsN  ->fill(n,s );
    p_bVsN  ->fill(n,b );
    p_ptSumVsN ->fill(n,ptSum);
    p_ptAvgVsN ->fill(n,ptAvg);
    }
  }
  
} // namespace CAP
