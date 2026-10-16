#include <cmath>
#include "Line.hpp"
#include "LineRenderer.hpp"

ClassImp(CAP::GRAPHICS::Line);

namespace CAP
{
  namespace GRAPHICS
  {
  Line::Line()
  :
  Drawable2D(),
  _lineAttributes(*LineAttributes::defaultLineAttributes())
  {
  _points.clear();
  addPoint(Point2D(0.0,0.0));
  addPoint(Point2D(1.0,0.0));
  setRenderer(LineRenderer::defaultRenderer());
  }

  Line::Line(const Point2D & point1, const Point2D & point2)
  :
  Drawable2D(),
  _lineAttributes(*LineAttributes::defaultLineAttributes())
  {
  _points.clear();
  addPoint(point1);
  addPoint(point2);
  setRenderer(LineRenderer::defaultRenderer());
  }

  Line::Line(const std::vector<Point2D> & points)
  :
  Drawable2D(),
  _lineAttributes(*LineAttributes::defaultLineAttributes())
  {
  _points = points;
  setRenderer(LineRenderer::defaultRenderer());
  }

  Line::Line(const Line & src)
  :
  Drawable2D(src),
  _lineAttributes(src._lineAttributes)
  {  }

  Line  & Line::operator=(const Line & rhs)
  {
  if (this != &rhs)
    {
    Drawable2D::operator=(rhs);
    _lineAttributes = rhs._lineAttributes;
    }
  return *this;
  }

  void Line::reset()
  {
  Drawable2D::reset();
  _lineAttributes.reset();
  }


  //!
  //!Compute distance from point px,py to a line.
  //!Compute the closest distance of approach from point px,py to this line.
  //!The distance is in the units of the arguments (e.g. pixels).
  ///
  //!Algorithm:
  ///~~~ {.cpp}
  //!  A(x1,y1)         P                             B(x2,y2)
  //!  -----------------+------------------------------
  //!                   |
  //!                   |
  //!                   |
  //!                   |
  //!                  M(x,y)
  ///
  //!Let us call  a = distance AM     A=a**2
  //!             b = distance BM     B=b**2
  //!             c = distance AB     C=c**2
  //!             d = distance PM     D=d**2
  //!             u = distance AP     U=u**2
  //!             v = distance BP     V=v**2     c = u + v
  ///
  //!D = A - U
  //!D = B - V  = B -(c-u)**2
  //!   ==> u = (A -B +C)/2c
  ///~~~

  float Line::distanceToLine(const Point2D & p, const Point2D & p1, const Point2D & p2, float lineWidth)
  {
  double  xl, xt, yl, yt;
  double  x     = p.x();
  double  y     = p.y();
  double  x1    = p1.x();
  double  y1    = p1.y();
  double  x2    = p2.x();
  double  y2    = p2.y();
  if (x1 < x2) {xl = x1; xt = x2;}
  else         {xl = x2; xt = x1;}
  if (y1 < y2) {yl = y1; yt = y2;}
  else         {yl = y2; yt = y1;}
  if (x < xl-2 || x> xt+2) return 9999;  //following algorithm only valid in the box
  if (y < yl-2 || y> yt+2) return 9999;  //surrounding the line
  double  xx1   = x  - x1;
  double  xx2   = x  - x2;
  double  x1x2  = x1 - x2;
  double  yy1   = y  - y1;
  double  yy2   = y  - y2;
  double  y1y2  = y1 - y2;
  double  a     = xx1*xx1   + yy1*yy1;
  double  b     = xx2*xx2   + yy2*yy2;
  double  c     = x1x2*x1x2 + y1y2*y1y2;
  if (c <= 0)  return 9999;
  double  v     = std::sqrt(c);
  double  u     = (a - b + c)/(2*v);
  double  d     = std::abs(a - u*u);

  return float(std::sqrt(d) - 0.5*lineWidth);
  }


  } // namespace GRAPHICS
} // namespace CAP
