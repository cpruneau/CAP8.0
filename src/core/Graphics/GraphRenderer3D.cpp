/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file GraphRenderer3D.cpp
//! @brief Implementation of GraphRenderer::draw3D(): the 3D views of the maps (surfaces, meshes, lego boxes).
#include <algorithm>
#include <cmath>
#include "GraphRenderer.hpp"
#include "Graph.hpp"
#include "FitRecord.hpp"
#include "ParameterizedFunction.hpp"
#include "Painter.hpp"
#include "Projection3D.hpp"
#include "DataPoints.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  namespace
  {
  //! A polygon to draw, with its distance from the eye (the farthest are drawn first).
  struct Face
  {
    std::vector<Point2D> points;   // canvas points
    double depth;                  // distance from the eye
    Color  color;                  // fill
    bool   outline;                // with the lines of the cells
    bool   filled = true;          // filled (otherwise only its outline, in line: a cell of the mesh of a fit)
    LineAttributes line;           // outline of an unfilled face
  };

  Color shade(const Color & c, double f) { return Color(float(c.red*f), float(c.green*f), float(c.blue*f), c.alpha); }

  //! A closed polyline through the points.
  std::vector<Point2D> closed(std::vector<Point2D> points) { points.push_back(points.front()); return points; }

  double clamp01(double v) { return v < 0 ? 0 : (v > 1 ? 1 : v); }
  } // namespace

  void GraphRenderer::draw3D(const Graph & graph, Painter & painter, const Graph::View & view, float textSize,
                             float cellWidth, float cellHeight) const
  {
  const float W = painter.width(), H = painter.height();
  const Projection3D projection(graph, view, W, H);
  auto at = [&](double fx, double fy, double fz) { return projection.toCanvas(fx, fy, fz); };
  auto sized = [&](float points, float automatic) { return points > 0 ? points/H : automatic; };
  const bool fromAbove = graph.elevation() >= 0;

  // the floor (the ceiling, seen from below) and the two back walls
  const double zFloor = fromAbove ? 0.0 : 1.0;
  const double xBack = projection.depth(0, 0.5, 0.5) > projection.depth(1, 0.5, 0.5) ? 0.0 : 1.0;
  const double yBack = projection.depth(0.5, 0, 0.5) > projection.depth(0.5, 1, 0.5) ? 0.0 : 1.0;
  const Color paneColor(0.965f, 0.965f, 0.965f);
  LineAttributes paneLine;
  paneLine.set(1, 0.4, 1, 0.75);
  std::vector<std::vector<Point2D>> panes = {
    { at(0, 0, zFloor), at(1, 0, zFloor), at(1, 1, zFloor), at(0, 1, zFloor) },
    { at(xBack, 0, 0), at(xBack, 1, 0), at(xBack, 1, 1), at(xBack, 0, 1) },
    { at(0, yBack, 0), at(1, yBack, 0), at(1, yBack, 1), at(0, yBack, 1) } };
  for (const auto & pane : panes) { painter.fillPolygon(pane, paneColor); painter.drawLine(closed(pane), paneLine); }

  // the faces of the maps, from all the items, drawn from the farthest
  std::vector<Face> faces;
  double lightRight = -0.35, lightUp = 0.65, lightAway = -0.67;   // light from the upper left, behind the eye
  double lightNorm = std::sqrt(lightRight*lightRight + lightUp*lightUp + lightAway*lightAway);
  lightRight /= lightNorm; lightUp /= lightNorm; lightAway /= lightNorm;
  float frameRed, frameGreen, frameBlue;
  Painter::rgb(graph.frameFill().color(), frameRed, frameGreen, frameBlue);
  const Color background(frameRed, frameGreen, frameBlue);
  auto valid = [&](double v) { return std::isfinite(v) && (!view.logZ || v > 0); };
  for (const auto & item : graph.items())
    {
    if (!item.isMap() || (item.kind() == GraphItem::kData && !item.data()) || (item.kind() == GraphItem::kFunction && !item.function())) continue;
    int samples = item.kind() == GraphItem::kFunction ? item.surfaceSamples() : 0;
    std::shared_ptr<const Grid2D> grid = item.grid(view.xMinimum, view.xMaximum, view.yMinimum, view.yMaximum, view.logX, view.logY, samples);
    if (!grid)
      {
      // scattered points: colored disks at their place in the box
      const MATH::DataPoints & d = *item.data();
      const float radius = 4.0f*std::max(0.3f, float(item.markerAttributes().size()));
      for (unsigned int n=0; n<d.size(); n++)
        {
        std::vector<double> c = d.coordinatesAt(n);
        double v = d.valueAt(n);
        if (c.size() != 2 || !valid(v)) continue;
        double fx = projection.fraction(0, c[0]), fy = projection.fraction(1, c[1]), fz = projection.fraction(2, v);
        if (fx < 0 || fx > 1 || fy < 0 || fy > 1 || fz < 0 || fz > 1) continue;
        Point2D p = at(fx, fy, fz);
        Face disk{ {}, projection.depth(fx, fy, fz), graph.colorOf(view, v), item.pointOutline() };
        for (int k=0; k<12; k++)
          disk.points.push_back(Point2D(p.x() + radius/W*std::cos(k*M_PI/6), p.y() + radius/H*std::sin(k*M_PI/6)));
        faces.push_back(disk);
        }
      continue;
      }
    const std::vector<double> & xEdges = grid->xEdges(), & yEdges = grid->yEdges();
    if (item.style3D() == GraphItem::kLego)
      {
      // boxes from the base (zero if in the z range, otherwise the nearest end) to the value
      double base = (view.zMinimum <= 0 && view.zMaximum >= 0) ? 0.0 : (view.zMinimum > 0 ? view.zMinimum : view.zMaximum);
      double baseFraction = view.logZ ? 0.0 : clamp01(projection.fraction(2, base));
      for (int j=0; j<grid->nY(); j++)
        for (int i=0; i<grid->nX(); i++)
          {
          double v = grid->value(i, j);
          if (!valid(v)) continue;
          double x0 = clamp01(projection.fraction(0, xEdges[i])), x1 = clamp01(projection.fraction(0, xEdges[i+1]));
          double y0 = clamp01(projection.fraction(1, yEdges[j])), y1 = clamp01(projection.fraction(1, yEdges[j+1]));
          if (x1 <= x0 || y1 <= y0) continue;              // outside the box
          double top = clamp01(projection.fraction(2, v));
          double zLow = std::min(baseFraction, top), zHigh = std::max(baseFraction, top);
          if (zHigh <= zLow) continue;
          Color color = graph.colorOf(view, v);
          double depth = projection.depth(0.5*(x0 + x1), 0.5*(y0 + y1), 0.5);
          // the sides turned to the eye: x sides lit more than y sides
          struct Side { double nx, ny; std::vector<Point2D> points; double light; };
          Side sides[4] = {
            { -1, 0, { at(x0, y0, zLow), at(x0, y1, zLow), at(x0, y1, zHigh), at(x0, y0, zHigh) }, 0.80 },
            {  1, 0, { at(x1, y0, zLow), at(x1, y1, zLow), at(x1, y1, zHigh), at(x1, y0, zHigh) }, 0.80 },
            { 0, -1, { at(x0, y0, zLow), at(x1, y0, zLow), at(x1, y0, zHigh), at(x0, y0, zHigh) }, 0.62 },
            { 0,  1, { at(x0, y1, zLow), at(x1, y1, zLow), at(x1, y1, zHigh), at(x0, y1, zHigh) }, 0.62 } };
          for (const auto & side : sides)
            {
            double right, up, away;
            projection.direction(side.nx, side.ny, 0, right, up, away);
            if (away < 0) faces.push_back({ side.points, depth, shade(color, side.light), item.showSurfaceLines() });
            }
          double z = fromAbove ? zHigh : zLow;
          faces.push_back({ { at(x0, y0, z), at(x1, y0, z), at(x1, y1, z), at(x0, y1, z) }, depth - 1E-9, color, item.showSurfaceLines() });
          }
      }
    else
      {
      // a surface through the values at the centers of the cells
      const int nx = grid->nX(), ny = grid->nY();
      std::vector<double> fx(nx), fy(ny);
      for (int i=0; i<nx; i++) fx[i] = projection.fraction(0, 0.5*(xEdges[i] + xEdges[i+1]));
      for (int j=0; j<ny; j++) fy[j] = projection.fraction(1, 0.5*(yEdges[j] + yEdges[j+1]));
      const bool mesh = item.style3D() == GraphItem::kMesh;
      for (int j=0; j+1<ny; j++)
        for (int i=0; i+1<nx; i++)
          {
          double v[4] = { grid->value(i, j), grid->value(i+1, j), grid->value(i+1, j+1), grid->value(i, j+1) };
          if (!valid(v[0]) || !valid(v[1]) || !valid(v[2]) || !valid(v[3])) continue;
          double px[4] = { fx[i], fx[i+1], fx[i+1], fx[i] }, py[4] = { fy[j], fy[j], fy[j+1], fy[j+1] }, pz[4];
          bool inside = true;
          double depth = 0;
          for (int c=0; c<4; c++)
            {
            pz[c] = clamp01(projection.fraction(2, v[c]));
            inside = inside && px[c] >= -1E-9 && px[c] <= 1 + 1E-9 && py[c] >= -1E-9 && py[c] <= 1 + 1E-9;
            depth += 0.25*projection.depth(px[c], py[c], pz[c]);
            }
          if (!inside) continue;
          Face face{ { at(px[0], py[0], pz[0]), at(px[1], py[1], pz[1]), at(px[2], py[2], pz[2]), at(px[3], py[3], pz[3]) },
                     depth, background, true };
          if (!mesh)
            {
            // shaded by the angle between the normal and the light
            double ax = px[2] - px[0], ay = py[2] - py[0], az = pz[2] - pz[0];
            double bx = px[3] - px[1], by = py[3] - py[1], bz = pz[3] - pz[1];
            double right, up, away;
            projection.direction(ay*bz - az*by, az*bx - ax*bz, ax*by - ay*bx, right, up, away);
            double norm = std::sqrt(right*right + up*up + away*away);
            double light = norm > 0 ? std::fabs(right*lightRight + up*lightUp + away*lightAway)/norm : 1.0;
            face.color = shade(graph.colorOf(view, 0.25*(v[0] + v[1] + v[2] + v[3])), 0.45 + 0.55*light);
            face.outline = item.showSurfaceLines();
            }
          faces.push_back(face);
          }
      }
    }
  // fits of rank 2 of the maps: wire meshes in the lines of the fits, sorted with the faces of the data (those in
  // front hide them)
  for (const auto & item : graph.items())
    {
    if (!item.isMap()) continue;
    for (unsigned int k=0; k<item.nFits(); k++)
      {
      const MATH::FitRecord * fit = graph.drawnFit(item, k);
      if (!fit) continue;
      double x0 = view.xMinimum, x1 = view.xMaximum, y0 = view.yMinimum, y1 = view.yMaximum;
      if (!item.extendFits() && fit->low.size() == 2)
        {
        x0 = std::max(x0, fit->low[0]); x1 = std::min(x1, fit->high[0]);
        y0 = std::max(y0, fit->low[1]); y1 = std::min(y1, fit->high[1]);
        }
      if (!(x1 > x0) || !(y1 > y0)) continue;
      const int n = 30;
      Grid2D grid = Grid2D::fromFunction(*fit->function, x0, x1, n, y0, y1, n, view.logX, view.logY);
      if (grid.isEmpty()) continue;
      const std::vector<double> & xEdges = grid.xEdges(), & yEdges = grid.yEdges();
      LineAttributes line = item.fitLineAttributes(k);
      line.setWidth(std::max(0.75f, 0.6f*line.width()));
      for (int j=0; j+1<grid.nY(); j++)
        for (int i=0; i+1<grid.nX(); i++)
          {
          double v[4] = { grid.value(i, j), grid.value(i+1, j), grid.value(i+1, j+1), grid.value(i, j+1) };
          if (!valid(v[0]) || !valid(v[1]) || !valid(v[2]) || !valid(v[3])) continue;
          double cx0 = projection.fraction(0, 0.5*(xEdges[i] + xEdges[i+1])), cx1 = projection.fraction(0, 0.5*(xEdges[i+1] + xEdges[i+2]));
          double cy0 = projection.fraction(1, 0.5*(yEdges[j] + yEdges[j+1])), cy1 = projection.fraction(1, 0.5*(yEdges[j+1] + yEdges[j+2]));
          double px[4] = { cx0, cx1, cx1, cx0 }, py[4] = { cy0, cy0, cy1, cy1 }, pz[4];
          double depth = 0;
          double highest = 0;
          for (int c=0; c<4; c++)
            {
            pz[c] = clamp01(projection.fraction(2, v[c]));
            highest = std::max(highest, pz[c]);
            depth += 0.25*projection.depth(px[c], py[c], pz[c]);
            }
          if (highest < 0.01) continue;                     // cells on the floor would only cover it
          Face face{ { at(px[0], py[0], pz[0]), at(px[1], py[1], pz[1]), at(px[2], py[2], pz[2]), at(px[3], py[3], pz[3]) },
                     depth, background, true };
          face.filled = false;
          face.line = line;
          faces.push_back(face);
          }
      }
    }
  std::stable_sort(faces.begin(), faces.end(), [](const Face & a, const Face & b) { return a.depth > b.depth; });
  LineAttributes cellLine;
  cellLine.set(1, 0.45, 1, 0.5);
  for (const auto & face : faces)
    {
    if (!face.filled) { painter.drawLine(closed(face.points), face.line); continue; }
    painter.fillPolygon(face.points, face.color);
    if (face.outline) painter.drawLine(closed(face.points), cellLine);
    }

  // axes: x and y along the front edges of the floor, z along the leftmost vertical edge
  const double xFront = 1 - xBack, yFront = 1 - yBack;
  double zx = 0, zy = 0;
  float leftmost = 1E9f;
  for (int corner=0; corner<4; corner++)
    {
    Point2D p = at(corner & 1, corner >> 1, 0);
    if (p.x() < leftmost) { leftmost = p.x(); zx = corner & 1; zy = corner >> 1; }
    }
  Point2D ends[3][2] = { { at(0, yFront, zFloor), at(1, yFront, zFloor) },
                         { at(xFront, 0, zFloor), at(xFront, 1, zFloor) },
                         { at(zx, zy, 0), at(zx, zy, 1) } };
  const Point2D center = at(0.5, 0.5, 0.5);
  for (int k=0; k<3; k++)
    {
    Axis axis = graph.axis(k);
    const Point2D & a = ends[k][0], & b = ends[k][1];
    axis.setEndPoints(a, b);
    axis.setLog(view.log(k));
    axis.setRange(view.minimum(k), view.maximum(k));
    float labelSize = sized(axis.labelPointSize(), textSize);
    axis.textAttributes().setSize(labelSize);
    axis.titleAttributes().setSize(sized(axis.titlePointSize(), textSize));
    axis.setMajorTickSize(0.5*textSize);
    axis.setMinorTickSize(0.25*textSize);
    axis.setLabelOffset(0.4*labelSize*axis.labelGap());
    axis.setNMinorTicks(5);
    axis.setTitleOffset(axis.titleOffsetX()*cellWidth, axis.titleOffsetY()*cellHeight);
    // ticks into the box, labels outside: the side of the normal that faces the center of the box
    double ux = (b.x() - a.x())*W, uy = (b.y() - a.y())*H;
    double toCenter = -uy*(center.x() - 0.5*(a.x() + b.x()))*W + ux*(center.y() - 0.5*(a.y() + b.y()))*H;
    axis.setTickSide(toCenter > 0 ? 1 : -1);
    if (!view.log(k)) axis.setNMajorTicks(Graph::niceTickCount(view.minimum(k), view.maximum(k)));
    // labels where axes meet: the y label at the front corner (the x label is kept), the bottom z label when the z
    // axis stands on a floor axis
    if (k == 1) axis.hideEndLabels(yFront == 0, yFront == 1);
    if (k == 2) axis.hideEndLabels(zy == yFront || zx == xFront, false);
    axis.draw(painter);
    graph.axis(k).setDrawnRects(axis.titleRect(), axis.bodyRect());
    }
  }

  } // namespace GRAPHICS
} // namespace CAP
