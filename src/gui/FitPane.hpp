/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FitPane.hpp
//! @brief FitPane: pane of the CapGui window showing the results of the fits drawn in the selected graph.
#ifndef CAP__GUI__FitPane
#define CAP__GUI__FitPane
#include <functional>
#include "String.hpp"

namespace CAP
{
  namespace MATH
  {
  struct FitRecord;
  }

  namespace GRAPHICS
  {
  class Graph;
  class GraphItem;
  }

  namespace GUI
  {

  //!
  //! @brief Pane of the CapGui window showing the fits attached to the objects of the selected graph, one tab per fit.
  //!
  //! Each tab gives the fitted function and the object it was fitted to, the quality of the fit (chi square or
  //! -2 ln L, degrees of freedom, chi square per degree of freedom, p-value, points, convergence, method), the
  //! parameters with their uncertainties and bounds, and the correlations of the free parameters; Copy puts them on
  //! the clipboard as tab-separated text. The curve of the fit can be hidden, and its color, style and width
  //! changed; check boxes above the tabs show or hide all the fits of the graph and their quality in the legend.
  //!
  //! The pane is docked below the canvases when first shown (CapGui::setFitPaneVisible()).
  //!
  class FitPane
  {
  public:

  //! @brief Name of the window of the pane. @return name
  static const char * windowName() { return "Fits###FitPane"; }

  //!
  //! @brief Draw the pane in the current ImGui window, for a graph.
  //! @param graph graph whose fits are shown (may be null) @param beforeChange called before the graph is changed (undo)
  //!
  void render(GRAPHICS::Graph * graph, const std::function<void()> & beforeChange);

  //! Called by the "Fit..." button of the pane (to open the Fit window).
  std::function<void()> onNewFit;

  //! @brief A fit as tab-separated text: its function and object, quality, parameters, correlations.
  //! @param item item the fit is attached to @param fit fit @return text
  static String text(const GRAPHICS::GraphItem & item, const MATH::FitRecord & fit);

  protected:

  //! @brief Draw the tab of fit k of an item. @param item item @param k index of the fit @param beforeChange undo hook
  void renderFit(GRAPHICS::GraphItem & item, unsigned int k, const std::function<void()> & beforeChange);
  };

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__FitPane
