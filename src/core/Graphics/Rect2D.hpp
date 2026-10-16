#ifndef CAP__Rect2D
#define CAP__Rect2D
#include <algorithm>
#include "Point2D.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  //!
  //! @brief Axis-aligned rectangle in normalized canvas coordinates (x0,y0 lower left, x1,y1 upper right).
  //!
  //! Renderers record with it where they drew the parts of a plot (title, axes, legend), so that an interactive
  //! front end can find what is under the mouse. An empty rectangle (the default) contains no point.
  //!
  struct Rect2D
  {
  float x0 = 0.0f, y0 = 0.0f, x1 = 0.0f, y1 = 0.0f;
  bool  empty = true;

  Rect2D() { }
  Rect2D(float xLow, float yLow, float xHigh, float yHigh)
  : x0(std::min(xLow,xHigh)), y0(std::min(yLow,yHigh)), x1(std::max(xLow,xHigh)), y1(std::max(yLow,yHigh)), empty(false) { }

  //! Rectangle of the given size (in canvas units) centered on c.
  static Rect2D centered(const Point2D & c, float width, float height)
  { return Rect2D(c.x() - 0.5f*width, c.y() - 0.5f*height, c.x() + 0.5f*width, c.y() + 0.5f*height); }

  bool contains(const Point2D & p) const { return !empty && p.x() >= x0 && p.x() <= x1 && p.y() >= y0 && p.y() <= y1; }
  //! Grow the rectangle to include p.
  void include(const Point2D & p)
  {
  if (empty) { x0 = x1 = p.x(); y0 = y1 = p.y(); empty = false; return; }
  x0 = std::min(x0, float(p.x())); x1 = std::max(x1, float(p.x()));
  y0 = std::min(y0, float(p.y())); y1 = std::max(y1, float(p.y()));
  }
  //! Grow the rectangle to include r.
  void include(const Rect2D & r) { if (r.empty) return; include(Point2D(r.x0, r.y0)); include(Point2D(r.x1, r.y1)); }
  //! Same rectangle, enlarged by dx and dy on each side.
  Rect2D expanded(float dx, float dy) const { return empty ? *this : Rect2D(x0-dx, y0-dy, x1+dx, y1+dy); }
  void clear() { *this = Rect2D(); }
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Rect2D
