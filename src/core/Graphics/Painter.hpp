#ifndef CAP__Painter
#define CAP__Painter
#include <vector>
#include "String.hpp"
#include "Point2D.hpp"
#include "FillAttributes.hpp"
#include "LineAttributes.hpp"
#include "MarkerAttributes.hpp"
#include "TextAttributes.hpp"
#include "Color.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  struct LatexImage;

  //!
  //! Abstract drawing surface used by all renderers.
  //!
  //! Coordinates passed to a Painter are normalized canvas coordinates:
  //! (0,0) is the lower-left corner and (1,1) the upper-right corner of the canvas.
  //! Each concrete Painter (MacOsCanvas, a future PDF or ROOT backend, ...) converts
  //! them to its own device units. Renderers never talk to a platform API directly.
  //!
  //! Style conventions (ROOT-like):
  //!  - colors are ROOT color indices (resolved by rgb())
  //!  - line style 0 = no line, 1 = solid, 2 = dashed, 3 = dotted, 4 = dash-dot
  //!  - fill style 0 = hollow, any other value = solid fill
  //!  - marker size 1 is about 8 device units across
  //!  - text size is a fraction of the canvas height
  //!
  class Painter
  {
  public:

  Painter() { }
  virtual ~Painter() { }

  //! Size of the drawing area in device units (points or pixels).
  virtual float width()  const = 0;
  virtual float height() const = 0;

  //! Open polyline through the given points.
  virtual void drawLine(const std::vector<Point2D> & points, const LineAttributes & line) = 0;

  //! Closed polygon: filled with fill, outlined with line.
  virtual void drawPolygon(const std::vector<Point2D> & points, const FillAttributes & fill, const LineAttributes & line) = 0;

  //! Polygon filled with any color, without outline (e.g. a cell of a color map). The default uses the ROOT
  //! color nearest to color, through drawPolygon(); the painters override it.
  virtual void fillPolygon(const std::vector<Point2D> & points, const Color & color);

  //! Rectangle [x0,x1] x [y0,y1] filled with any color, without outline; adjacent rectangles leave no gap between
  //! them. The default calls fillPolygon().
  virtual void fillRectangle(float x0, float y0, float x1, float y1, const Color & color);

  //! One marker centered on position.
  virtual void drawMarker(const Point2D & position, const MarkerAttributes & marker) = 0;

  //! Text anchored at position (justification sets the horizontal anchor; text is
  //! vertically centered on position). angle is in degrees, counter-clockwise.
  virtual void drawText(const Point2D & position, const String & text, const TextAttributes & attributes, float angle=0.0) = 0;

  //! Width of text in device units: the width of its LaTeX image (see latexImage()), otherwise
  //! plainTextWidth(). Backends that can measure text (e.g. MacOsCanvas) override it.
  virtual float textWidth(const String & text, const TextAttributes & attributes) const;

  //! Estimated width of text drawn with a font, in device units (0.55 x height per character).
  float plainTextWidth(const String & text, const TextAttributes & attributes) const;

  //! The LaTeX image of text, typeset if needed (see LatexRenderer::image()), or nullptr if text is
  //! drawn with a font. The image may hold an error instead (ok false): draw LatexRenderer::plainText().
  static const LatexImage * latexImage(const String & text);

  //! Convert a ROOT color index to RGB components in [0,1].
  //! Uses ROOT's color table when available, otherwise a built-in table of the basic colors.
  static void rgb(int color, float & red, float & green, float & blue);

  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Painter
