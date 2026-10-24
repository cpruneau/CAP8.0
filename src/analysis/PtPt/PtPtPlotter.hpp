/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__PtPtPlotter
#define CAP__PtPtPlotter
#include "Plotter.hpp"
#include "PtPtHistos.hpp"
#include "PtPtDerivedHistos.hpp"

namespace CAP
{


//!
//! @brief Plots of the pT correlation results (PtPtHistos, PtPtDerivedHistos).
//! @note Not part of the build (not listed in PtPt/CMakeLists.txt).
//!
class PtPtPlotter : public Plotter
{
public:
  
  //! @brief Default constructor.
  PtPtPlotter();
  //! @brief Destructor.
  virtual ~PtPtPlotter();
  //! @brief Plot the results of one analysis. @param canvasNameBase base of the canvas names @param PtPtHistosUsed moments
  //! @param PtPtDerivedHistosUsed derived quantities @param canvasConfigurationUsed canvas style
  //! @param graphConfigurationsUsed graph styles
  void makePlots(const String & canvasNameBase,
                 PtPtHistos * PtPtHistosUsed,
                 PtPtDerivedHistos * PtPtDerivedHistosUsed,
                 CanvasConfiguration & canvasConfigurationUsed,
                 GraphConfiguration  ** graphConfigurationsUsed);

  //! @brief Compare the results of several analyses. @param canvasNameBase base of the canvas names
  //! @param canvasConfiguration canvas style @param nGraphs number of analyses @param PtPtHistos moments
  //! @param PtPtDerivedHistos derived quantities @param histLabels legends @param graphConfigurations graph styles
  void makeComparisonPlots(const String & canvasNameBase,
                           Configuration & canvasConfiguration,
                           int nGraphs,
                           PtPtHistos ** PtPtHistos,
                           PtPtDerivedHistos ** PtPtDerivedHistos,
                           String ** histLabels,
                           GraphConfiguration  ** graphConfigurations);

  //! @brief Compare the pT correlators of several analyses (see makeComparisonPlots() for the parameters).
  void makePtPtComparisonPlots(const String & canvasNameBase,
                                Configuration & canvasConfiguration,
                                int nGraphs,
                                PtPtHistos ** PtPtHistos,
                                PtPtDerivedHistos ** PtPtDerivedHistos,
                                String ** histLabels,
                                GraphConfiguration  ** graphConfigurations);

};


} // namespace CAP

#endif /* CAP__PtPtPlotter */
