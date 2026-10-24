#ifndef CAP__Polygon
#define CAP__Polygon
#include <vector>
#include "Geom2D.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  //!
  //! Closed polygon.
  //!  - If explicit vertices are set (setVertices), they are used as given (normalized canvas units).
  //!  - Otherwise the polygon is generated from position(), width(), height() and angle():
  //!    4 sides gives a width x height rectangle, any other n >= 3 a regular n-gon inscribed
  //!    in the width x height ellipse.
  //!
  class Polygon : public Geom2D
  {
  public:

  Polygon();
  Polygon(int nSides);
  Polygon(const Polygon & src);
  Polygon  & operator=(const Polygon & rhs);
  virtual ~Polygon() { }

  void setNSides(int nSides) { _nSides = nSides; }
  int  nSides() const { return _nSides; }
  void setVertices(const std::vector<Point2D> & vertices) { _vertices = vertices; }
  void clearVertices() { _vertices.clear(); }

  //! Vertices actually drawn, in normalized canvas units.
  std::vector<Point2D> vertices() const;
  //! Center of the shape, in normalized canvas units.
  Point2D center() const;

  protected:

  int   _nSides;
  std::vector<Point2D> _vertices;

  };

  } //  namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Polygon
