#include <cmath>
#include "Polygon.hpp"
#include "PolygonRenderer.hpp"

ClassImp(CAP::GRAPHICS::Polygon);

namespace CAP
{
  namespace GRAPHICS
  {

  Polygon::Polygon()
  :
  Geom2D(),
  _nSides(4), // rectangle by default
  _vertices()
  {
  setRenderer(PolygonRenderer::defaultRenderer());
  }

  Polygon::Polygon(int nSides)
  :
  Geom2D(),
  _nSides(nSides),
  _vertices()
  {
  setRenderer(PolygonRenderer::defaultRenderer());
  }

  Polygon::Polygon(const Polygon & src)
  :
  Geom2D(src),
  _nSides(src._nSides),
  _vertices(src._vertices)
  { }

  Polygon  & Polygon::operator=(const Polygon & rhs)
  {
  if (this != &rhs)
    {
    Geom2D::operator=(rhs);
    _nSides   = rhs._nSides;
    _vertices = rhs._vertices;
    }
  return *this;
  }

  Point2D Polygon::center() const
  {
  if (!_vertices.empty())
    {
    float sx = 0, sy = 0;
    for (const auto & v : _vertices) { sx += v.x(); sy += v.y(); }
    return Point2D(sx/_vertices.size(), sy/_vertices.size());
    }
  Point2D p = position();
  if (fromCenter()) return p;
  return Point2D(p.x() + 0.5*_width, p.y() + 0.5*_height);
  }

  std::vector<Point2D> Polygon::vertices() const
  {
  if (!_vertices.empty()) return _vertices;

  std::vector<Point2D> v;
  if (_nSides < 3) return v;

  // shape around the origin, before rotation
  float hw = 0.5*_width;
  float hh = 0.5*_height;
  std::vector<Point2D> local;
  if (_nSides == 4)
    {
    local.push_back(Point2D(-hw,-hh));
    local.push_back(Point2D( hw,-hh));
    local.push_back(Point2D( hw, hh));
    local.push_back(Point2D(-hw, hh));
    }
  else
    {
    const double pi = 3.14159265358979323846;
    for (int k=0; k<_nSides; k++)
      {
      double phi = pi/2.0 + 2.0*pi*k/_nSides; // first vertex at the top
      local.push_back(Point2D(hw*std::cos(phi), hh*std::sin(phi)));
      }
    }

  // rotate around the center, then move to the center
  Point2D c = center();
  double a  = _angle*3.14159265358979323846/180.0;
  double ca = std::cos(a);
  double sa = std::sin(a);
  for (const auto & p : local)
    v.push_back(Point2D(c.x() + ca*p.x() - sa*p.y(), c.y() + sa*p.x() + ca*p.y()));
  return v;
  }

  } //  namespace GRAPHICS
} // namespace CAP
