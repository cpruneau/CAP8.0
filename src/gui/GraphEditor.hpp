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
//! @file GraphEditor.hpp
//! @brief GraphEditor: dialogs editing the parts of a graph (title, axes, legend, items).
#ifndef CAP__GUI__GraphEditor
#define CAP__GUI__GraphEditor
#include "Graph.hpp"

namespace CAP
{
  namespace GUI
  {

  //!
  //! @brief Contents of the dialogs that edit the parts of a graph (opened by CanvasPane with a right click).
  //!
  //! Each function draws its widgets in the current ImGui window (a popup) and applies the changes to the graph at
  //! once; it returns true if something was changed. The editors of an axis and of its title take the index of the
  //! axis (Graph::axis()), so that they serve the axes of the 3D graphs as well.
  //!
  //! The widgets used by the editors (ROOT colors, fonts, markers, sizes in points) are available on their own.
  //!
  namespace GraphEditor
  {
  //! @brief Axis k of the graph: range (or automatic), scale, line, labels (color, font, size, distance), ticks.
  bool editAxis(GRAPHICS::Graph & graph, int axis);
  //! @brief Title of axis k: text, color, font, size, position.
  bool editAxisTitle(GRAPHICS::Graph & graph, int axis);
  //! @brief Title of the graph: text, color, font, size, position.
  bool editTitle(GRAPHICS::Graph & graph);
  //! @brief Legend: shown, position, text size, background and border.
  bool editLegend(GRAPHICS::Graph & graph);
  //! @brief Item of the graph (data points or function): label, markers, line, uncertainties. @param graph graph
  //! @param item index of the item @param point index of the data point that was clicked, or -1
  bool editItem(GRAPHICS::Graph & graph, int item, int point=-1);

  //! @name Widgets
  //! @{

  //! @brief Color swatch opening a palette of ROOT colors. @param label label @param color ROOT color index
  bool colorEdit(const char * label, int & color);
  //! @brief Font family and style (ROOT font code). @param label label @param font font code
  bool fontEdit(const char * label, int & font);
  //! @brief Marker style, with previews. @param label label @param style ROOT marker style
  //! @param color color of the previews
  bool markerEdit(const char * label, int & style, int color);
  //! @brief Text size in points, or automatic (0). @param label label @param points size (0: automatic)
  bool pointSizeEdit(const char * label, float & points);
  //!
  //! @brief Preview of a text typeset by LaTeX (nothing for a plain text): the image, or the LaTeX error, or why the
  //! text is drawn with a font; for a text in ROOT syntax, its translation too. The text is typeset in the background
  //! (see GRAPHICS::LatexRenderer::imageLater()). @param text text
  //!
  void latexPreview(const String & text);
  //! @brief Line style (solid, dashed, dotted, dash-dot, none). @param label label @param style ROOT line style
  //! @param allowNone offer "none" (no line)
  bool lineStyleEdit(const char * label, int & style, bool allowNone=true);
  //! @}
  } // namespace GraphEditor

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__GraphEditor
