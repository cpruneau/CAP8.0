/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file PlotOutputs.hpp
//! @brief PlotOutputs: the printed figures, PDF book, PowerPoint presentation and LaTeX document of the canvases of
//! a plot file (CapGui's File > Export, and PlotHistos in batch mode).
#ifndef CAP__GUI__PlotOutputs
#define CAP__GUI__PlotOutputs
#include <vector>
#include "PlotSpec.hpp"

namespace CAP
{
  namespace GUI
  {

  //!
  //! @brief The outputs of the canvases of a plot file (see GRAPHICS::PlotSpec), written in its output directory
  //! under the names it gives (element Output).
  //!
  struct PlotOutputs
  {
  //! @brief All the canvases of windows, in order. @param windows windows @return canvases
  static std::vector<const GRAPHICS::Canvas *> canvases(const std::vector<GRAPHICS::PlotSpec::WindowState> & windows);
  //! @brief Print the canvases and graphs that have file names. @param spec specification @param windows windows
  //! @return paths written
  static std::vector<String> print(const GRAPHICS::PlotSpec & spec, const std::vector<GRAPHICS::PlotSpec::WindowState> & windows);
  //! @brief Write the PDF book: one canvas per page. @param spec specification @param windows windows
  //! @param fileName file ("": the one of the specification) @return path written, or "" on failure
  static String pdf(const GRAPHICS::PlotSpec & spec, const std::vector<GRAPHICS::PlotSpec::WindowState> & windows,
                    const String & fileName="");
  //! @brief Write the PowerPoint presentation: one canvas per slide, as native shapes. @param spec specification
  //! @param windows windows @param fileName file ("": the one of the specification) @return path written, or ""
  static String pptx(const GRAPHICS::PlotSpec & spec, const std::vector<GRAPHICS::PlotSpec::WindowState> & windows,
                     const String & fileName="");
  //!
  //! @brief Write the LaTeX document, with the Latex classes: a Beamer presentation, one frame per canvas
  //! (CanvasPresentation), or an article with one section per window and one figure per canvas (latexStyle of the
  //! specification); compiled with pdflatex if the specification asks for it (latexCompile).
  //! @param spec specification @param windows windows @param error set to the reason of a failure
  //! @param fileName file ("": the one of the specification) @return path of the .tex file, or "" on failure
  //!
  static String latex(const GRAPHICS::PlotSpec & spec, const std::vector<GRAPHICS::PlotSpec::WindowState> & windows,
                      String & error, const String & fileName="");
  //! @brief Text of a title for LaTeX: kept if it is LaTeX ($ or \), translated if it uses ROOT's syntax (#),
  //! otherwise with the special characters escaped. @param text text @return LaTeX
  static String latexText(const String & text);
  };

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__PlotOutputs
