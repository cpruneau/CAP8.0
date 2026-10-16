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
  virtual ~SvgPainter() { }

  //! @brief Draw into the rectangle (x,y,width,height) of the page (points, y down from the top left), until
  //! endRegion(): the normalized coordinates of the canvas drawn then map to this rectangle.
  void beginRegion(float x, float y, float width, float height);
  //! @brief Back to the whole page.
  void endRegion();

  virtual float width()  const { return _regionWidth;  }
  virtual float height() const { return _regionHeight; }
  virtual void drawLine(const std::vector<Point2D> & points, const LineAttributes & line);
  virtual void drawPolygon(const std::vector<Point2D> & points, const FillAttributes & fill, const LineAttributes & line);
  virtual void drawMarker(const Point2D & position, const MarkerAttributes & marker);
  virtual void drawText(const Point2D & position, const String & text, const TextAttributes & attributes, float angle=0.0);

  //! @brief The SVG document. @return text
  String document() const;
  //! @brief Write the SVG document to fileName. @return true on success
  bool write(const String & fileName) const;

  protected:

  float _pageWidth, _pageHeight;
  float _regionX, _regionY, _regionWidth, _regionHeight;
  std::ostringstream _body;

  float px(const Point2D & p) const { return _regionX + p.x()*_regionWidth; }
  float py(const Point2D & p) const { return _regionY + (1.0f - p.y())*_regionHeight; }
  static String color(int color);
  static String dashArray(int style, float width);
  static String escape(const String & text);
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__SvgPainter
