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
#include "GlobalDerivedHistos.hpp"
#include "Task.hpp"
#include "Configuration.hpp"
#include "NameHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"

ClassImp(CAP::GlobalDerivedHistos);

namespace CAP
{

  GlobalDerivedHistos::GlobalDerivedHistos()
  :
  GlobalHistos(),
  h_ptAvgIncVsN()
  {   }

  void GlobalDerivedHistos::cloneD(const GlobalDerivedHistos & group)
  {
  h_ptAvgIncVsN  = histogramLike(group.h_ptAvgIncVsN);
  }

  void GlobalDerivedHistos::configure(const String & taskName,
                                            const String & objectType,
                                            const Configuration & configuration,
                                            unsigned int index)
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  GlobalHistos::configure(taskName,objectType,configuration,index);
  }

  // for now use the same boundaries for eta and y histogram
  void GlobalDerivedHistos::create()
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  h_ptAvgIncVsN = createHistogram(createName(_histogramBaseName,"n","ptAvgIncl"),n_nbins, n_min,  n_max,"n","ptAvgIncl");
  }

  void GlobalDerivedHistos::loadFrom(CapFile & inputFile)
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  GlobalHistos::loadFrom(inputFile);
  h_ptAvgIncVsN = importHistogram(inputFile, createName(_histogramBaseName,"n","ptAvgIncl"));
  }

  //!
  //! Calculates derived histograms. This is currently limited to pt averages vs eta and phi.
  //!
  void GlobalDerivedHistos::calculateDerivedHistograms(const GlobalHistos & baseHistos)
  {
  unsigned int n = baseHistos.p_ptSumVsN->nBins(0);
  for (unsigned int k=1; k<n; k++)
    {
    double v  = baseHistos.p_ptSumVsN->binValueAt((k)-1);
    double ev = baseHistos.p_ptSumVsN->binUncertaintyAt((k)-1);
    double n  = baseHistos.p_ptSumVsN->binCenter((k)-1,0);
    h_ptAvgIncVsN->setBinValueAt(unsigned((k)-1),v/n);
    h_ptAvgIncVsN->setBinUncertaintyAt(unsigned((k)-1),ev/n);
    }
  }

} // namespace CAP

