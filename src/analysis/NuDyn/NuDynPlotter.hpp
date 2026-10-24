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
#ifndef CAP__NuDynPlotter
#define CAP__NuDynPlotter
#include "Plotter.hpp"
#include "NuDynHistos.hpp"
#include "NuDynDerivedHistos.hpp"
#include "ClassDef.hpp"

namespace CAP
{


//!
//! @brief Plots of the multiplicity moments and nu_dyn results (NuDynHistos, NuDynDerivedHistos).
//! @note Not part of the build (not listed in NuDyn/CMakeLists.txt).
//!
class NuDynPlotter : public Plotter
{
public:
  
  //! @brief Default constructor.
  NuDynPlotter();
  //! @brief Destructor.
  virtual ~NuDynPlotter();
  //! @brief Plot the results of one analysis. @param canvasNameBase base of the canvas names
  //! @param nuDynHistosUsed moments @param nuDynDerivedHistosUsed derived quantities
  //! @param canvasConfigurationUsed canvas style @param graphConfigurationsUsed graph styles
  void makePlots(const String & canvasNameBase,
                 NuDynHistos * nuDynHistosUsed,
                 NuDynDerivedHistos * nuDynDerivedHistosUsed,
                 CanvasConfiguration & canvasConfigurationUsed,
                 GraphConfiguration  ** graphConfigurationsUsed);

  //! @brief Compare the results of several analyses. @param canvasNameBase base of the canvas names
  //! @param canvasConfiguration canvas style @param nGraphs number of analyses @param nuDynHistos moments
  //! @param nuDynDerivedHistos derived quantities @param histLabels legends @param graphConfigurations graph styles
  void makeComparisonPlots(const String & canvasNameBase,
                           Configuration & canvasConfiguration,
                           int nGraphs,
                           NuDynHistos ** nuDynHistos,
                           NuDynDerivedHistos ** nuDynDerivedHistos,
                           String ** histLabels,
                           GraphConfiguration  ** graphConfigurations);

  //! @brief Compare the nu_dyn results of several analyses (see makeComparisonPlots() for the parameters).
  void makeNudynComparisonPlots(const String & canvasNameBase,
                                Configuration & canvasConfiguration,
                                int nGraphs,
                                NuDynHistos ** nuDynHistos,
                                NuDynDerivedHistos ** nuDynDerivedHistos,
                                String ** histLabels,
                                GraphConfiguration  ** graphConfigurations);

  ClassDef(NuDynPlotter,0)
};


} // namespace CAP

#endif /* CAP__NuDynPlotter */
