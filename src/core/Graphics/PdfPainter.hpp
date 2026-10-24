/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file PdfPainter.hpp
//! @brief PdfPainter: painter writing PDF files (no library needed).
#ifndef CAP__PdfPainter
#define CAP__PdfPainter
#include <map>
#include <sstream>
#include <string>
#include "Painter.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  //!
  //! @brief Painter writing PDF (one page of vector graphics), with no library: the file is written directly.
  //!
  //! Usage as SvgPainter: create the painter with the page size in points, draw one or several canvases (each in its
  //! own rectangle of the page with beginRegion()/endRegion(), or the whole page), then write() the file.
  //! @code
  //!   PdfPainter pdf(800, 600);
  //!   canvas.draw(pdf);
  //!   pdf.write("plot.pdf");
  //! @endcode
  //!
  //! - Lines, polygons, rectangles and markers are PDF paths; transparency uses graphics states (/ca, /CA).
  //! - Texts use the 14 fonts every PDF reader has (Helvetica, Times, Courier with their bold and italic variants,
  //!   Symbol): no font is embedded. They are encoded in WinAnsi (Latin-1 characters; others are replaced by '?').
  //! - Texts typeset by LaTeX (see LatexRenderer) are drawn as the paths of their glyphs, as in SVG files: the SVG
  //!   paths of the LaTeX images are converted to PDF paths.
  //!
  class PdfPainter : public Painter
  {
  public:

  //! @brief Page of the given size in points. @param width page width @param height page height
  PdfPainter(float width, float height);
  //! @brief Destructor.
  virtual ~PdfPainter() { }

  //! @brief Draw into the rectangle (x,y,width,height) of the page (points, y down from the top left), until
  //! endRegion(): the normalized coordinates of the canvas drawn then map to this rectangle.
  void beginRegion(float x, float y, float width, float height);
  //! @brief Back to the whole page.
  void endRegion();

  virtual float width()  const { return _regionWidth;  }   //!< Width of the current region (points)
  virtual float height() const { return _regionHeight; }   //!< Height of the current region (points)
  //! @brief Open polyline (see Painter::drawLine()).
  virtual void drawLine(const std::vector<Point2D> & points, const LineAttributes & line);
  //! @brief Closed polygon (see Painter::drawPolygon()).
  virtual void drawPolygon(const std::vector<Point2D> & points, const FillAttributes & fill, const LineAttributes & line);
  //! @brief Polygon of any color (see Painter::fillPolygon()).
  virtual void fillPolygon(const std::vector<Point2D> & points, const Color & color);
  //! @brief Rectangle of any color, slightly enlarged so that adjacent cells show no seam (see Painter::fillRectangle()).
  virtual void fillRectangle(float x0, float y0, float x1, float y1, const Color & color);
  //! @brief One marker (see Painter::drawMarker()).
  virtual void drawMarker(const Point2D & position, const MarkerAttributes & marker);
  //! @brief Text (see Painter::drawText()): the glyph paths of a text typeset by LaTeX, otherwise a text in the standard
  //! font of its ROOT font code.
  virtual void drawText(const Point2D & position, const String & text, const TextAttributes & attributes, float angle=0.0);

  //! @brief The PDF document. @return bytes of the file
  std::string document() const;
  //! @brief Write the PDF document to fileName. @return true on success
  bool write(const String & fileName) const;

  //! @brief Width of a text in one of the standard fonts. @param text text (WinAnsi bytes) @param font index of the
  //! standard font (see fontIndex()) @param size font size @return width (points)
  static float standardTextWidth(const std::string & text, int font, float size);
  //! @brief Index of the standard font of a ROOT font code: 0-3 Helvetica (regular, bold, oblique, bold oblique), 4-7
  //! Times, 8-11 Courier, 12 Symbol. @param font ROOT font code @return index
  static int fontIndex(int font);
  //! @brief Text converted from UTF-8 to WinAnsi bytes, '?' for the characters it lacks. @param text text @return bytes
  static std::string winAnsi(const String & text);

  protected:

  float _pageWidth;   //!< size of the page (points)
  float _pageHeight;   //!< size of the page (points)
  float _regionX;   //!< rectangle of the page drawn into (points, y down)
  float _regionY;   //!< rectangle of the page drawn into (points, y down)
  float _regionWidth;   //!< rectangle of the page drawn into (points, y down)
  float _regionHeight;   //!< rectangle of the page drawn into (points, y down)
  std::ostringstream _content;   //!< content stream (drawing operators)
  std::map<int, std::string> _alphas;   //!< graphics states of the opacities used: per mille -> name
  bool _fontsUsed[13];   //!< standard fonts used

  float px(const Point2D & p) const { return _regionX + p.x()*_regionWidth; }   //!< Page x of a canvas point (y down)
  float py(const Point2D & p) const { return _regionY + (1.0f - p.y())*_regionHeight; }   //!< Page y of a canvas point (y down)
  //! @brief PDF y (up from the bottom) of a page y (down from the top). @param y page y @return PDF y
  float flip(float y) const { return _pageHeight - y; }
  //! @brief Set the fill (or stroke) color and opacity. @param r red @param g green @param b blue @param alpha
  //! opacity @param stroke stroke color if true, fill color otherwise
  void setColor(float r, float g, float b, float alpha, bool stroke);
  //! @brief Set the color of a ROOT color index. @param color index @param alpha opacity @param stroke stroke color if true
  void setColor(int color, float alpha, bool stroke);
  //! @brief Set the line width and dash pattern of a ROOT line style. @param style style @param width width
  void setLine(int style, float width);
  //! @brief Path through points (page coordinates, y down), closed if asked. @param points points @param close close it
  void path(const std::vector<Point2D> & points, bool close);
  //! @brief Draw the SVG paths and rectangles of a LaTeX image, transformed. @param svg SVG elements
  //! @param a,b,c,d,e,f affine transform from the image to the page (x' = a x + c y + e, y' = b x + d y + f; page y down)
  void svgPaths(const std::string & svg, double a, double b, double c, double d, double e, double f);
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__PdfPainter
