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
#include <algorithm>
#include <cmath>
#include "Shape.hpp"
#include "Graph.hpp"
#include "Painter.hpp"

ClassImp(CAP::GRAPHICS::Shape);

namespace CAP
{
  namespace GRAPHICS
  {

  namespace
  {
  //! Distance from p to the segment a-b.
  float segmentDistance(const Point2D & p, const Point2D & a, const Point2D & b)
  {
  float dx = b.x() - a.x(), dy = b.y() - a.y();
  float length2 = dx*dx + dy*dy;
  float t = length2 > 0 ? ((p.x() - a.x())*dx + (p.y() - a.y())*dy)/length2 : 0.0f;
  t = std::min(std::max(t, 0.0f), 1.0f);
  float ex = a.x() + t*dx - p.x(), ey = a.y() + t*dy - p.y();
  return std::sqrt(ex*ex + ey*ey);
  }

  //! Whether p is inside the polygon (even-odd rule).
  bool inside(const Point2D & p, const std::vector<Point2D> & polygon)
  {
  bool in = false;
  for (size_t k=0, j=polygon.size()-1; k<polygon.size(); j=k++)
    {
    const Point2D & a = polygon[k], & b = polygon[j];
    if ((a.y() > p.y()) != (b.y() > p.y()) && p.x() < (b.x() - a.x())*(p.y() - a.y())/(b.y() - a.y()) + a.x()) in = !in;
    }
  return in;
  }
  } // namespace

  Shape::Shape(Kind kind)
  :
  Geom2D(),
  _kind(kind),
  _arrows(kNoArrow)
  {
  _fillAttributes.set(0, 1.0, FillAttributes::kHollow);
  _lineAttributes.set(1, 1.0, 1, 2.0f);
  }

  Shape::Shape(const Shape & src)
  :
  Geom2D(src),
  _kind(src._kind),
  _arrows(src._arrows)
  { }

  Shape & Shape::operator=(const Shape & rhs)
  {
  if (this != &rhs)
    {
    Geom2D::operator=(rhs);
    _kind   = rhs._kind;
    _arrows = rhs._arrows;
    }
  return *this;
  }

  const char * Shape::kindName(Kind kind)
  {
  switch (kind)
    {
    case kRectangle: return "rectangle";
    case kEllipse:   return "ellipse";
    case kTriangle:  return "triangle";
    case kPolygon:   return "polygon";
    case kLine:      return "line";
    }
  return "shape";
  }

  bool Shape::isValid() const
  {
  if (_kind == kPolygon) return _points.size() >= 3;
  return _points.size() >= 2 && (_points[0].x() != _points[1].x() || _points[0].y() != _points[1].y());
  }

  std::vector<Point2D> Shape::outline() const
  {
  if (_kind == kPolygon || _kind == kLine || _points.size() < 2) return _points;
  const Point2D & a = _points[0], & b = _points[1];
  switch (_kind)
    {
    case kRectangle:
      return { a, Point2D(b.x(), a.y()), b, Point2D(a.x(), b.y()) };
    case kTriangle:
      return { a, Point2D(b.x(), a.y()), Point2D(0.5f*(a.x() + b.x()), b.y()) };
    case kEllipse:
      {
      std::vector<Point2D> points;
      float cx = 0.5f*(a.x() + b.x()), cy = 0.5f*(a.y() + b.y()), rx = 0.5f*std::fabs(b.x() - a.x()), ry = 0.5f*std::fabs(b.y() - a.y());
      for (int k=0; k<72; k++)
        {
        float t = 2.0f*float(M_PI)*float(k)/72.0f;
        points.push_back(Point2D(cx + rx*std::cos(t), cy + ry*std::sin(t)));
        }
      return points;
      }
    default: return _points;
    }
  }

  void Shape::move(float dx, float dy)
  {
  for (auto & p : _points) p.set(p.x() + dx, p.y() + dy);
  }

  bool Shape::contains(const Point2D & p, float toleranceX, float toleranceY) const
  {
  std::vector<Point2D> points = outline();
  if (points.size() < 2) return false;
  // distances measured with the tolerances as unit (an ellipse of tolerance around the point)
  auto scaled = [toleranceX, toleranceY](const Point2D & q) { return Point2D(q.x()/toleranceX, q.y()/toleranceY); };
  Point2D s = scaled(p);
  size_t n = points.size();
  size_t nSegments = isClosed() ? n : n - 1;
  for (size_t k=0; k<nSegments; k++)
    if (segmentDistance(s, scaled(points[k]), scaled(points[(k + 1) % n])) <= 1.0f) return true;
  return isClosed() && _fillAttributes.style() != FillAttributes::kHollow && inside(p, points);
  }

  Point2D Shape::toCanvas(const Graph & graph, const Point2D & p)
  {
  return Point2D(graph.cellXLow() + p.x()*(graph.cellXHigh() - graph.cellXLow()),
                 graph.cellYLow() + p.y()*(graph.cellYHigh() - graph.cellYLow()));
  }

  Point2D Shape::fromCanvas(const Graph & graph, const Point2D & p)
  {
  float w = graph.cellXHigh() - graph.cellXLow(), h = graph.cellYHigh() - graph.cellYLow();
  return Point2D(w > 0 ? (p.x() - graph.cellXLow())/w : 0.0f, h > 0 ? (p.y() - graph.cellYLow())/h : 0.0f);
  }

  void Shape::draw(Painter & painter, const Graph & graph) const
  {
  if (_excluded || !isValid()) return;
  std::vector<Point2D> points;
  for (const auto & p : outline()) points.push_back(toCanvas(graph, p));
  if (isClosed())
    {
    painter.drawShape(points, _fillAttributes, _lineAttributes);
    return;
    }
  if (_lineAttributes.style() == 0) return;
  // a line, shortened under its arrow heads (so that a wide line does not show past the tip)
  const float W = painter.width(), H = painter.height();
  if (W <= 0 || H <= 0) return;
  float length = std::max(10.0f, 4.0f*_lineAttributes.width());   // device units
  float halfWidth = 0.4f*length;
  FillAttributes headFill;
  headFill.set(_lineAttributes.color(), _lineAttributes.alpha(), FillAttributes::kSolid);
  LineAttributes noLine(_lineAttributes);
  noLine.setStyle(0);
  std::vector<Point2D> line = points;
  auto head = [&](const Point2D & tip, const Point2D & from, Point2D & base)
    {
    float dx = (tip.x() - from.x())*W, dy = (tip.y() - from.y())*H;
    float d = std::sqrt(dx*dx + dy*dy);
    if (d <= 0) { base = tip; return; }
    dx /= d; dy /= d;
    float l = std::min(length, d);
    float bx = tip.x()*W - l*dx, by = tip.y()*H - l*dy;
    base = Point2D(bx/W, by/H);
    painter.drawPolygon({ tip, Point2D((bx - halfWidth*dy)/W, (by + halfWidth*dx)/H), Point2D((bx + halfWidth*dy)/W, (by - halfWidth*dx)/H) },
                        headFill, noLine);
    };
  size_t n = line.size();
  if (_arrows != kNoArrow && n >= 2) head(points[n-1], points[n-2], line[n-1]);
  if (_arrows == kDoubleArrow && n >= 2) head(points[0], points[1], line[0]);
  painter.drawLine(line, _lineAttributes);
  }

  } // namespace GRAPHICS
} // namespace CAP
