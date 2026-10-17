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
//! @file ImGuiPainter.hpp
//! @brief ImGuiPainter: painter drawing CAP Graphics objects with Dear ImGui.
#ifndef CAP__GUI__ImGuiPainter
#define CAP__GUI__ImGuiPainter
#include "Painter.hpp"

struct ImDrawList;
struct ImFont;

namespace CAP
{
  namespace GUI
  {

  //!
  //! @brief Painter drawing CAP Graphics objects (Canvas, Graph, ...) into an ImGui draw list.
  //!
  //! The canvas fills the rectangle given to the constructor (screen pixels, y down); normalized canvas coordinates
  //! (0,0 lower-left, 1,1 upper-right) are mapped onto it. Marker styles, line styles and text sizes follow
  //! MacOsCanvas, so that a canvas looks the same in the GUI, in a macOS window and in SVG files.
  //!
  class ImGuiPainter : public GRAPHICS::Painter
  {
  public:

  //! @brief Painter. @param drawList ImGui draw list @param font font used for text (nullptr: the current font)
  //! @param x left edge @param y top edge @param width width @param height height (pixels)
  ImGuiPainter(ImDrawList * drawList, ImFont * font, float x, float y, float width, float height);
  //! @brief Destructor.
  virtual ~ImGuiPainter() { }

  virtual float width()  const { return _width;  }   //!< Width of the canvas (pixels)
  virtual float height() const { return _height; }   //!< Height of the canvas (pixels)
  //! @brief Open polyline (see Painter::drawLine()).
  virtual void drawLine(const std::vector<GRAPHICS::Point2D> & points, const GRAPHICS::LineAttributes & line);
  //! @brief Closed polygon, filled and outlined (see Painter::drawPolygon()).
  virtual void drawPolygon(const std::vector<GRAPHICS::Point2D> & points, const GRAPHICS::FillAttributes & fill,
                           const GRAPHICS::LineAttributes & line);
  //! @brief One marker (see Painter::drawMarker()).
  virtual void drawMarker(const GRAPHICS::Point2D & position, const GRAPHICS::MarkerAttributes & marker);
  //! @brief Text, in the font of its ROOT font code (see Painter::drawText(), setFont()).
  virtual void drawText(const GRAPHICS::Point2D & position, const String & text, const GRAPHICS::TextAttributes & attributes,
                        float angle=0.0);
  //! @brief Width of a text in pixels, measured with its font.
  virtual float textWidth(const String & text, const GRAPHICS::TextAttributes & attributes) const;

  //! @brief Font used for the ROOT font index (1 to 15, see TextAttributes); texts in a font without an entry use
  //! the font given to the constructor. @param index ROOT font index @param font font (may be null)
  static void setFont(int index, ImFont * font);
  //! @brief Font used for a ROOT font code. @param font font code (e.g. 42) @return font, or null if none was set
  static ImFont * fontFor(int font);

  //! @brief Screen position of a canvas point. @param p canvas point @param x screen x @param y screen y
  void toScreen(const GRAPHICS::Point2D & p, float & x, float & y) const;
  //! @brief Canvas point of a screen position. @return canvas coordinates
  GRAPHICS::Point2D toCanvas(float x, float y) const;

  protected:

  ImDrawList * _drawList;   //!< draw list receiving the drawing
  ImFont     * _font;   //!< font of texts whose ROOT font has no entry (null: the current font)
  float _x, _y, _width, _height;   //!< rectangle of the canvas on the screen (pixels, y down)

  //! @brief Font used for a text. @param attributes attributes of the text @return font
  ImFont * font(const GRAPHICS::TextAttributes & attributes) const;
  };

  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__ImGuiPainter
