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
//! @file SvgPainter.hpp
//! @brief SvgPainter: painter writing SVG files.
#ifndef CAP__SvgPainter
#define CAP__SvgPainter
#include <sstream>
#include "Painter.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  class Drawable2D;

  //!
  //! @brief Painter writing SVG (scalable vector graphics, readable by browsers, Inkscape, Illustrator, LaTeX via
  //! svg packages).
  //!
  //! Usage: create the painter with the page size in points, draw one or several canvases (each in its own
  //! rectangle of the page with beginRegion()/endRegion(), or the whole page), then write() the file.
  //! @code
  //!   SvgPainter svg(800, 600);
  //!   canvas.draw(svg);
  //!   svg.write("plot.svg");
  //! @endcode
  //!
  class SvgPainter : public Painter
  {
  public:

  //! @brief Page of the given size in points. @param width page width @param height page height
  SvgPainter(float width, float height);
  //! @brief Destructor.
  virtual ~SvgPainter() { }

  //! @brief Draw into the rectangle (x,y,width,height) of the page (points, y down from the top left), until
  //! endRegion(): the normalized coordinates of the canvas drawn then map to this rectangle.
  void beginRegion(float x, float y, float width, float height);
  //! @brief Back to the whole page.
  void endRegion();

  virtual float width()  const { return _regionWidth;  }   //!< Width of the current region (points)
  virtual float height() const { return _regionHeight; }   //!< Height of the current region (points)
  //! @brief Open polyline, as an SVG polyline (see Painter::drawLine()).
  virtual void drawLine(const std::vector<Point2D> & points, const LineAttributes & line);
  //! @brief Closed polygon, as an SVG polygon (see Painter::drawPolygon()).
  virtual void drawPolygon(const std::vector<Point2D> & points, const FillAttributes & fill, const LineAttributes & line);
  //! @brief Polygon of any color, as an SVG polygon (see Painter::fillPolygon()).
  virtual void fillPolygon(const std::vector<Point2D> & points, const Color & color);
  //! @brief Rectangle of any color, as an SVG rect with crisp edges, so that adjacent cells show no seam (see
  //! Painter::fillRectangle()).
  virtual void fillRectangle(float x0, float y0, float x1, float y1, const Color & color);
  //! @brief One marker, as SVG shapes (see Painter::drawMarker()).
  virtual void drawMarker(const Point2D & position, const MarkerAttributes & marker);
  //! @brief Text (see Painter::drawText()): the paths typeset by LaTeX for a LaTeX or ROOT-syntax text (see LatexRenderer),
  //! otherwise an SVG text with the family, weight and style of its ROOT font code.
  virtual void drawText(const Point2D & position, const String & text, const TextAttributes & attributes, float angle=0.0);

  //! @brief The SVG document. @return text
  String document() const;
  //! @brief Write the SVG document to fileName. @return true on success
  bool write(const String & fileName) const;

  protected:

  float _pageWidth;   //!< size of the page (points)
  float _pageHeight;   //!< size of the page (points)
  float _regionX;   //!< rectangle of the page drawn into (points, y down)
  float _regionY;   //!< rectangle of the page drawn into (points, y down)
  float _regionWidth;   //!< rectangle of the page drawn into (points, y down)
  float _regionHeight;   //!< rectangle of the page drawn into (points, y down)
  std::ostringstream _body;   //!< SVG elements written so far

  float px(const Point2D & p) const { return _regionX + p.x()*_regionWidth; }   //!< Page x of a canvas point
  float py(const Point2D & p) const { return _regionY + (1.0f - p.y())*_regionHeight; }   //!< Page y of a canvas point (y down)
  //! @brief SVG color of a ROOT color index. @param color index @return "#rrggbb"
  static String color(int color);
  //! @brief SVG color of a Color. @param color color @return "#rrggbb"
  static String color(const Color & color);
  //! @brief SVG dash pattern of a ROOT line style. @param style line style @param width line width @return dash array, or "" for solid lines
  static String dashArray(int style, float width);
  //! @brief Text with the XML special characters escaped. @param text text @return escaped text
  static String escape(const String & text);
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__SvgPainter
