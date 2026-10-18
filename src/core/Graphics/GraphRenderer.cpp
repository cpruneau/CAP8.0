#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include "GraphRenderer.hpp"
#include "Graph.hpp"
#include "Painter.hpp"
#include "Function.hpp"
#include "DataPoints.hpp"
#include "Histogram.hpp"
#include "TColor.h"

ClassImp(CAP::GRAPHICS::GraphRenderer);

namespace CAP
{
  namespace GRAPHICS
  {

  GraphRenderer::GraphRenderer()
  :
  Renderer()
  { }

  GraphRenderer::GraphRenderer(const GraphRenderer & src)
  :
  Renderer(src)
  { }

  GraphRenderer  & GraphRenderer::operator=(const GraphRenderer & rhs)
  {
  Renderer::operator=(rhs);
  return *this;
  }

  const GraphRenderer * GraphRenderer::defaultRenderer()
  {
  static const GraphRenderer renderer;
  return &renderer;
  }

  namespace
  {
  struct Box { float x0, y0, x1, y1; };

  //! Clip the segment a-b to the box (Liang-Barsky). @return false if the segment is entirely outside
  bool clip(const Box & box, Point2D & a, Point2D & b)
  {
  double t0 = 0.0, t1 = 1.0;
  double dx = b.x()-a.x(), dy = b.y()-a.y();
  const double p[4] = { -dx, dx, -dy, dy };
  const double q[4] = { a.x()-box.x0, box.x1-a.x(), a.y()-box.y0, box.y1-a.y() };
  for (int k=0; k<4; k++)
    {
    if (p[k] == 0) { if (q[k] < 0) return false; continue; }
    double t = q[k]/p[k];
    if (p[k] < 0) { if (t > t1) return false; if (t > t0) t0 = t; }
    else          { if (t < t0) return false; if (t < t1) t1 = t; }
    }
  Point2D a0 = a;
  a = Point2D(a0.x() + t0*dx, a0.y() + t0*dy);
  b = Point2D(a0.x() + t1*dx, a0.y() + t1*dy);
  return true;
  }

  //! Draw a polyline clipped to the box; points flagged invalid break the line.
  void drawClipped(Painter & painter, const Box & box, const std::vector<Point2D> & points,
                   const std::vector<bool> & valid, const LineAttributes & line)
  {
  std::vector<Point2D> run;
  auto flush = [&]() { if (run.size() >= 2) painter.drawLine(run, line); run.clear(); };
  for (size_t k=1; k<points.size(); k++)
    {
    if (!valid[k-1] || !valid[k]) { flush(); continue; }
    Point2D a = points[k-1], b = points[k];
    bool aInside = a.x()>=box.x0 && a.x()<=box.x1 && a.y()>=box.y0 && a.y()<=box.y1;
    if (!clip(box, a, b)) { flush(); continue; }
    if (!aInside) flush();
    if (run.empty()) run.push_back(a);
    run.push_back(b);
    bool bInside = points[k].x()>=box.x0 && points[k].x()<=box.x1 && points[k].y()>=box.y0 && points[k].y()<=box.y1;
    if (!bInside) flush();
    }
  flush();
  }

  bool inside(const Box & box, const Point2D & p)
  {
  return p.x()>=box.x0 && p.x()<=box.x1 && p.y()>=box.y0 && p.y()<=box.y1;
  }
  } // namespace

  namespace
  {
  //! The part of the rectangle [x0,x1] x [y0,y1] inside the box. @return false if nothing is inside
  bool clipRect(const Box & box, float & x0, float & y0, float & x1, float & y1)
  {
  if (x0 > x1) std::swap(x0, x1);
  if (y0 > y1) std::swap(y0, y1);
  x0 = std::max(x0, box.x0); x1 = std::min(x1, box.x1);
  y0 = std::max(y0, box.y0); y1 = std::min(y1, box.y1);
  return x1 > x0 && y1 >= y0;
  }

  void drawSegment(Painter & painter, const Box & box, Point2D a, Point2D b, const LineAttributes & line)
  {
  if (clip(box, a, b)) painter.drawLine({a, b}, line);
  }

  //! Systematic uncertainties of a data item (boxes, brackets or band), drawn behind the points.
  void drawSysErrors(Painter & painter, const Box & box, const Graph & graph, const Graph::View & view,
                     const GraphItem & item)
  {
  const MATH::DataPoints & d = *item.data();
  FillAttributes fill = item.sysFillAttributes();
  LineAttributes line = item.sysLineAttributes();
  if (item.sysUsesMarkerColor())
    {
    fill.setColor(item.markerAttributes().color(), fill.alpha());
    line.setColor(item.markerAttributes().color(), line.alpha());
    }
  LineAttributes noLine;
  noLine.setStyle(0);
  const float frameWidth = box.x1 - box.x0;
  const float aspect = painter.width()/painter.height();   // canvas y units per canvas x unit, for equal lengths
  std::vector<Point2D> upper, lower;                        // band
  auto flushBand = [&]()
    {
    if (upper.size() >= 2)
      {
      std::vector<Point2D> polygon = upper;
      polygon.insert(polygon.end(), lower.rbegin(), lower.rend());
      painter.drawPolygon(polygon, fill, noLine);
      if (line.style() != 0) { painter.drawLine(upper, line); painter.drawLine(lower, line); }
      }
    upper.clear(); lower.clear();
    };
  for (unsigned int k=0; k<d.size(); k++)
    {
    MATH::DataPoint2D p = d.point(k);
    double sys = p.ySysError();
    bool ok = std::isfinite(p.x()) && std::isfinite(p.y()) && (!view.logX || p.x() > 0) && (!view.logY || p.y() > 0);
    if (!ok || !(sys > 0)) { if (item.sysStyle() == GraphItem::kSysBand) flushBand(); continue; }
    double low = p.y() - sys;
    if (view.logY && low <= 0) low = view.yMinimum;
    Point2D c  = graph.toCanvas(view, p.x(), p.y());
    float yLow  = graph.toCanvas(view, p.x(), low).y();
    float yHigh = graph.toCanvas(view, p.x(), p.y() + sys).y();
    float xLow, xHigh;
    if (p.xError() > 0 && (!view.logX || p.x() - p.xError() > 0))
      {
      xLow  = graph.toCanvas(view, p.x() - p.xError(), p.y()).x();
      xHigh = graph.toCanvas(view, p.x() + p.xError(), p.y()).x();
      }
    else
      {
      xLow  = c.x() - item.sysWidth()*frameWidth;
      xHigh = c.x() + item.sysWidth()*frameWidth;
      }
    switch (item.sysStyle())
      {
      case GraphItem::kSysBoxes:
        {
        float x0 = xLow, y0 = yLow, x1 = xHigh, y1 = yHigh;
        if (!clipRect(box, x0, y0, x1, y1)) break;
        std::vector<Point2D> corners{ Point2D(x0,y0), Point2D(x1,y0), Point2D(x1,y1), Point2D(x0,y1) };
        painter.drawPolygon(corners, fill, line);
        break;
        }
      case GraphItem::kSysBrackets:
        {
        LineAttributes bracket = line;
        if (bracket.style() == 0) bracket.setStyle(1);
        float t = 0.35f*(xHigh - xLow)*aspect;   // length of the bracket ends
        drawSegment(painter, box, Point2D(xLow, yHigh), Point2D(xHigh, yHigh), bracket);
        drawSegment(painter, box, Point2D(xLow, yHigh), Point2D(xLow, yHigh - t), bracket);
        drawSegment(painter, box, Point2D(xHigh, yHigh), Point2D(xHigh, yHigh - t), bracket);
        drawSegment(painter, box, Point2D(xLow, yLow), Point2D(xHigh, yLow), bracket);
        drawSegment(painter, box, Point2D(xLow, yLow), Point2D(xLow, yLow + t), bracket);
        drawSegment(painter, box, Point2D(xHigh, yLow), Point2D(xHigh, yLow + t), bracket);
        break;
        }
      case GraphItem::kSysBand:
        {
        if (c.x() < box.x0 || c.x() > box.x1) { flushBand(); break; }
        upper.push_back(Point2D(c.x(), std::min(std::max(yHigh, box.y0), box.y1)));
        lower.push_back(Point2D(c.x(), std::min(std::max(yLow,  box.y0), box.y1)));
        break;
        }
      }
    }
  flushBand();
  }

  bool hasSysErrors(const GraphItem & item)
  {
  if (item.kind() != GraphItem::kData || !item.data() || !item.showSysErrors() || item.isMap()) return false;
  for (unsigned int k=0; k<item.data()->size(); k++) if (item.data()->point(k).ySysError() > 0) return true;
  return false;
  }

  //! Color map of a grid: the cells with a value (> 0 on a logarithmic z scale), clipped to the frame.
  void drawColorMap(Painter & painter, const Box & box, const Graph & graph, const Graph::View & view, const Grid2D & grid)
  {
  const std::vector<double> & xEdges = grid.xEdges(), & yEdges = grid.yEdges();
  // canvas positions of the edges; an edge <= 0 on a logarithmic axis goes to the lower end of the axis
  std::vector<float> cx(xEdges.size()), cy(yEdges.size());
  for (size_t i=0; i<xEdges.size(); i++)
    cx[i] = graph.toCanvas(view, view.logX && xEdges[i] <= 0 ? view.xMinimum : xEdges[i], view.yMaximum).x();
  for (size_t j=0; j<yEdges.size(); j++)
    cy[j] = graph.toCanvas(view, view.xMaximum, view.logY && yEdges[j] <= 0 ? view.yMinimum : yEdges[j]).y();
  for (int j=0; j<grid.nY(); j++)
    for (int i=0; i<grid.nX(); i++)
      {
      double v = grid.value(i, j);
      if (!std::isfinite(v) || (view.logZ && v <= 0)) continue;
      float x0 = cx[i], x1 = cx[i+1], y0 = cy[j], y1 = cy[j+1];
      if (!clipRect(box, x0, y0, x1, y1)) continue;
      painter.fillRectangle(x0, y0, x1, y1, graph.colorOf(view, v));
      }
  }

  //! A point of a map: position, value, and index in its data (-1 if it has none, e.g. a node of a regular grid).
  struct MapPoint { double x, y, value; int index; };

  //! The points of a map item: the centers of the cells with a value of its grid, or its data points.
  std::vector<MapPoint> mapPoints(const GraphItem & item, const Grid2D * grid)
  {
  std::vector<MapPoint> points;
  if (grid)
    {
    // the cells of a histogram are its bins, in the same order (axis 0 fastest)
    bool bins = item.kind() == GraphItem::kData && dynamic_cast<const MATH::Histogram*>(item.data());
    for (int j=0; j<grid->nY(); j++)
      for (int i=0; i<grid->nX(); i++)
        {
        double v = grid->value(i, j);
        if (!std::isfinite(v)) continue;
        points.push_back({ 0.5*(grid->xEdges()[i] + grid->xEdges()[i+1]), 0.5*(grid->yEdges()[j] + grid->yEdges()[j+1]), v,
                           bins ? i + grid->nX()*j : -1 });
        }
    return points;
    }
  const MATH::DataPoints & d = *item.data();
  for (unsigned int n=0; n<d.size(); n++)
    {
    std::vector<double> c = d.coordinatesAt(n);
    if (c.size() == 2) points.push_back({ c[0], c[1], d.valueAt(n), int(n) });
    }
  return points;
  }

  //! Points of a map as disks colored by their values: of the size of the markers of the item, or with an area
  //! proportional to the magnitude of the values; with a thin outline if asked.
  void drawColoredPoints(Painter & painter, const Box & box, const Graph & graph, const Graph::View & view, const GraphItem & item,
                         const std::vector<MapPoint> & points, int iItem, std::vector<Graph::PointPosition> & positions)
  {
  const float radius = 4.0f*std::max(0.3f, float(item.markerAttributes().size()));   // device units (size 1: 8 across)
  double largest = 0;
  for (const auto & point : points) if (std::isfinite(point.value)) largest = std::max(largest, std::fabs(point.value));
  LineAttributes outline;
  outline.set(1, 0.7, 1, 0.5);
  for (const auto & point : points)
    {
    double v = point.value;
    if (!std::isfinite(v) || (view.logZ && v <= 0) || (view.logX && point.x <= 0) || (view.logY && point.y <= 0)) continue;
    Point2D p = graph.toCanvas(view, point.x, point.y);
    if (!inside(box, p)) continue;
    float r = radius;
    if (item.pointSizeByValue() && largest > 0) r *= float(std::max(0.15, std::sqrt(std::fabs(v)/largest)));
    const float rx = r/painter.width(), ry = r/painter.height();
    std::vector<Point2D> disk;
    for (int k=0; k<16; k++)
      {
      double a = 2.0*3.14159265358979323846*k/16;
      disk.push_back(Point2D(p.x() + rx*std::cos(a), p.y() + ry*std::sin(a)));
      }
    painter.fillPolygon(disk, graph.colorOf(view, v));
    if (item.pointOutline()) { disk.push_back(disk.front()); painter.drawLine(disk, outline); }
    if (point.index >= 0) positions.push_back({ iItem, point.index, p });
    }
  }

  //! Contour lines of a grid at the levels of the item, clipped to the frame.
  void drawContours(Painter & painter, const Box & box, const Graph & graph, const Graph::View & view, const GraphItem & item,
                    const Grid2D & grid)
  {
  for (double level : graph.contourLevels(view, item))
    {
    LineAttributes line = item.contourLineAttributes();
    if (item.contourUsesPalette())
      {
      Color c = graph.colorOf(view, level);
      line.setColor(TColor::GetColor(c.red, c.green, c.blue), line.alpha());
      }
    for (const auto & polyline : grid.contour(level, view.logZ))
      {
      std::vector<Point2D> points;
      std::vector<bool> valid;
      for (const auto & q : polyline)
        {
        bool ok = (!view.logX || q.x > 0) && (!view.logY || q.y > 0);
        points.push_back(graph.toCanvas(view, ok ? q.x : 1.0, ok ? q.y : 1.0));
        valid.push_back(ok);
        }
      drawClipped(painter, box, points, valid, line);
      }
    }
  }

  //! Whether a printf format writes one floating point number (and nothing else that needs an argument).
  bool numberFormat(const String & format)
  {
  int conversions = 0;
  const char * f = format.data();
  for (int k=0; f[k]; k++)
    {
    if (f[k] != '%') continue;
    if (f[k+1] == '%') { k++; continue; }
    int m = k + 1;
    while (f[m] && std::strchr("+- #0123456789.", f[m])) m++;
    if (!f[m] || !std::strchr("eEfFgG", f[m])) return false;
    conversions++;
    k = m;
    }
  return conversions == 1;
  }

  //! The values of the cells of a grid written at their centers, where they fit.
  void drawValues(Painter & painter, const Box & box, const Graph & graph, const Graph::View & view, const GraphItem & item,
                  const Grid2D & grid, float automaticSize)
  {
  const float W = painter.width(), H = painter.height();
  const String format = numberFormat(item.valueFormat()) ? item.valueFormat() : String("%.3g");
  const std::vector<double> & xEdges = grid.xEdges(), & yEdges = grid.yEdges();
  std::vector<float> cx(xEdges.size()), cy(yEdges.size());
  for (size_t i=0; i<xEdges.size(); i++) cx[i] = graph.toCanvas(view, view.logX && xEdges[i] <= 0 ? view.xMinimum : xEdges[i], view.yMaximum).x();
  for (size_t j=0; j<yEdges.size(); j++) cy[j] = graph.toCanvas(view, view.xMaximum, view.logY && yEdges[j] <= 0 ? view.yMinimum : yEdges[j]).y();
  int inView = 0;
  for (int j=0; j<grid.nY(); j++)
    for (int i=0; i<grid.nX(); i++)
      if (inside(box, Point2D(0.5f*(cx[i] + cx[i+1]), 0.5f*(cy[j] + cy[j+1])))) inView++;
  if (inView > 4000) return;                 // far too many to read
  TextAttributes text = *TextAttributes::defaultTextAttributes();
  text.setJustification(TextAttributes::kCenter);
  for (int j=0; j<grid.nY(); j++)
    for (int i=0; i<grid.nX(); i++)
      {
      double v = grid.value(i, j);
      if (!std::isfinite(v)) continue;
      Point2D center(0.5f*(cx[i] + cx[i+1]), 0.5f*(cy[j] + cy[j+1]));
      if (!inside(box, center)) continue;
      float cellWidth = std::fabs(cx[i+1] - cx[i])*W, cellHeight = std::fabs(cy[j+1] - cy[j])*H;   // device units
      float size = item.valuePointSize() > 0 ? item.valuePointSize()/H : std::min(automaticSize, 0.55f*cellHeight/H);
      if (size*H < 5.0f || size*H > cellHeight) continue;
      text.setSize(size);
      char label[64];
      std::snprintf(label, sizeof(label), format.data(), v);
      if (painter.textWidth(label, text) > 0.92f*cellWidth) continue;
      int color = item.valueColor();
      if (color < 0)
        {
        color = 1;                            // black, or white on a dark cell
        if (item.showColorMap() && (!view.logZ || v > 0))
          {
          Color c = graph.colorOf(view, v);
          if (0.2126f*c.red + 0.7152f*c.green + 0.0722f*c.blue < 0.5f) color = 0;
          }
        }
      text.setColor(color);
      painter.drawText(center, label, text);
      }
  }
  } // namespace

  void GraphRenderer::draw(const Drawable2D & drawable, Painter & painter) const
  {
  const Graph * graph = dynamic_cast<const Graph*>(&drawable);
  if (!graph) return;
  if (painter.width() <= 0 || painter.height() <= 0) return;
  // a graph showing nothing is not drawn (no frame, axes or title), and has no parts to pick
  if (graph->items().empty())
    {
    graph->xAxis().setDrawnRects(Rect2D(), Rect2D());
    graph->yAxis().setDrawnRects(Rect2D(), Rect2D());
    graph->zAxis().setDrawnRects(Rect2D(), Rect2D());
    graph->setDrawnLayout(Rect2D(), Rect2D(), std::vector<Graph::PointPosition>());
    return;
    }

  const Graph::View view = graph->view();
  const Box box { graph->frameXLow(), graph->frameYLow(), graph->frameXHigh(), graph->frameYHigh() };
  const float H = painter.height(), W = painter.width();
  const float cellHeight = graph->cellYHigh() - graph->cellYLow();
  const float cellWidth  = graph->cellXHigh() - graph->cellXLow();
  // text and tick sizes follow the size of the cell (fractions of the canvas height)
  const float scale = std::min(cellHeight, cellWidth*W/H);
  // ... within 9 to 17 device units, so that labels stay readable without dominating a large canvas
  const float textSize = std::min(std::max(0.045f*scale, 9.0f/H), 17.0f/H);
  // a size in points (device units) if set, otherwise the automatic size
  auto sized = [&](float points, float automatic) { return points > 0 ? points/H : automatic; };
  std::vector<Graph::PointPosition> pointPositions;

  // frame background
  std::vector<Point2D> corners{ Point2D(box.x0,box.y0), Point2D(box.x1,box.y0), Point2D(box.x1,box.y1), Point2D(box.x0,box.y1) };
  LineAttributes noLine;
  noLine.setStyle(0);
  painter.drawPolygon(corners, graph->frameFill(), noLine);

  // items
  for (size_t iItem=0; iItem<graph->items().size(); iItem++)
    {
    const GraphItem & item = graph->items()[iItem];
    if (item.isMap())
      {
      if ((item.kind() == GraphItem::kData && !item.data()) || (item.kind() == GraphItem::kFunction && !item.function())) continue;
      std::shared_ptr<const Grid2D> grid = item.grid(view.xMinimum, view.xMaximum, view.yMinimum, view.yMaximum, view.logX, view.logY);
      if (grid && item.showColorMap()) drawColorMap(painter, box, *graph, view, *grid);
      if (grid && item.showContours()) drawContours(painter, box, *graph, view, item, *grid);
      if (item.kind() == GraphItem::kData && (!grid || item.showMapPoints()))       // scattered points always
        drawColoredPoints(painter, box, *graph, view, item, mapPoints(item, grid.get()), int(iItem), pointPositions);
      if (grid && item.showValues()) drawValues(painter, box, *graph, view, item, *grid, 0.8f*textSize);
      }
    else if (item.kind() == GraphItem::kFunction && item.function() && item.function()->rank() == 1)
      {
      double a = std::max<double>(item.xMinimum(), view.xMinimum);
      double b = std::min<double>(item.xMaximum(), view.xMaximum);
      if (!(b > a)) continue;
      int n = item.nSamples();
      std::vector<Point2D> points;
      std::vector<bool> valid;
      for (int k=0; k<n; k++)
        {
        double x;
        if (view.logX && a > 0) x = std::pow(10.0, std::log10(a) + (std::log10(b)-std::log10(a))*k/(n-1));
        else                    x = a + (b-a)*k/(n-1);
        double y = 0.0;
        bool ok = true;
        try { y = item.function()->evaluateAt(x); } catch (...) { ok = false; }
        ok = ok && std::isfinite(y) && (!view.logY || y > 0);
        points.push_back(graph->toCanvas(view, x, ok ? y : 1.0));
        valid.push_back(ok);
        }
      if (item.showLine()) drawClipped(painter, box, points, valid, item.lineAttributes());
      }
    else if (item.kind() == GraphItem::kData && item.data())
      {
      const MATH::DataPoints & d = *item.data();
      if (item.showSysErrors()) drawSysErrors(painter, box, *graph, view, item);
      std::vector<Point2D> points;
      std::vector<bool> valid;
      LineAttributes errorLine;
      errorLine.set(item.markerAttributes().color(), item.markerAttributes().alpha(), 1, 1.0);
      for (unsigned int k=0; k<d.size(); k++)
        {
        MATH::DataPoint2D p = d.point(k);
        bool ok = std::isfinite(p.x()) && std::isfinite(p.y()) && (!view.logX || p.x() > 0) && (!view.logY || p.y() > 0);
        Point2D c = graph->toCanvas(view, p.x(), ok ? p.y() : 1.0);
        points.push_back(c);
        valid.push_back(ok);
        if (!ok) continue;
        if (inside(box, c)) pointPositions.push_back({ int(iItem), int(k), c });
        if (item.showErrors())
          {
          // statistical uncertainty on y (the systematic one is drawn separately), uncertainty on x
          double ey = p.yStatError();
          if (ey > 0)
            {
            double low = p.y() - ey;
            if (view.logY && low <= 0) low = view.yMinimum;
            drawSegment(painter, box, graph->toCanvas(view, p.x(), low), graph->toCanvas(view, p.x(), p.y() + ey), errorLine);
            }
          double ex = p.xError();
          if (ex > 0)
            {
            double low = p.x() - ex;
            if (view.logX && low <= 0) low = view.xMinimum;
            drawSegment(painter, box, graph->toCanvas(view, low, p.y()), graph->toCanvas(view, p.x() + ex, p.y()), errorLine);
            }
          }
        }
      if (item.showLine()) drawClipped(painter, box, points, valid, item.lineAttributes());
      if (item.showMarkers())
        for (size_t k=0; k<points.size(); k++)
          if (valid[k] && inside(box, points[k])) painter.drawMarker(points[k], item.markerAttributes());
      }
    }

  // frame outline
  if (graph->frameLine().style() != 0)
    {
    std::vector<Point2D> outline = corners;
    outline.push_back(corners[0]);
    painter.drawLine(outline, graph->frameLine());
    }

  // axes, from the templates of the graph
  Axis xAxis = graph->xAxis();
  xAxis.setEndPoints(Point2D(box.x0, box.y0), Point2D(box.x1, box.y0));
  xAxis.setLog(view.logX);
  xAxis.setRange(view.xMinimum, view.xMaximum);
  Axis yAxis = graph->yAxis();
  yAxis.setEndPoints(Point2D(box.x0, box.y0), Point2D(box.x0, box.y1));
  yAxis.setLog(view.logY);
  yAxis.setRange(view.yMinimum, view.yMaximum);
  for (Axis * axis : { &xAxis, &yAxis })
    {
    float labelSize = sized(axis->labelPointSize(), textSize);
    axis->textAttributes().setSize(labelSize);
    axis->titleAttributes().setSize(sized(axis->titlePointSize(), textSize));
    axis->setMajorTickSize(0.6*textSize);
    axis->setMinorTickSize(0.3*textSize);
    axis->setLabelOffset(0.35*labelSize*axis->labelGap());
    axis->setNMinorTicks(5);
    // the title offset of the template is in fractions of the cell
    axis->setTitleOffset(axis->titleOffsetX()*cellWidth, axis->titleOffsetY()*cellHeight);
    }
  // labels centered below the x axis: clear the ticks
  xAxis.setLabelOffset((0.6*textSize + 0.3*xAxis.textAttributes().size())*xAxis.labelGap());
  if (!view.logX) xAxis.setNMajorTicks(Graph::niceTickCount(view.xMinimum, view.xMaximum));
  if (!view.logY) yAxis.setNMajorTicks(Graph::niceTickCount(view.yMinimum, view.yMaximum));
  xAxis.draw(painter);
  yAxis.draw(painter);
  graph->xAxis().setDrawnRects(xAxis.titleRect(), xAxis.bodyRect());
  graph->yAxis().setDrawnRects(yAxis.titleRect(), yAxis.bodyRect());

  // color scale of the maps, right of the frame: the palette over the z range, and the z axis along its right edge
  if (view.hasZ && graph->showColorScale())
    {
    const Palette palette = graph->palette();
    const float x0 = box.x1 + 0.025f*cellWidth, x1 = x0 + 0.035f*cellWidth;
    const int nSlices = palette.nSteps() > 1 ? palette.nSteps() : 128;
    for (int s=0; s<nSlices; s++)
      painter.fillRectangle(x0, box.y0 + (box.y1 - box.y0)*s/nSlices, x1, box.y0 + (box.y1 - box.y0)*(s + 1)/nSlices,
                            palette.color((s + 0.5)/nSlices));
    LineAttributes outline = graph->frameLine();
    if (outline.style() == 0) outline.set(1, 1.0, 1, 1.0);
    painter.drawLine({ Point2D(x0, box.y0), Point2D(x1, box.y0), Point2D(x1, box.y1), Point2D(x0, box.y1), Point2D(x0, box.y0) }, outline);
    Axis zAxis = graph->zAxis();
    zAxis.setEndPoints(Point2D(x1, box.y0), Point2D(x1, box.y1));
    zAxis.setTickSide(1);
    zAxis.setLog(view.logZ);
    zAxis.setRange(view.zMinimum, view.zMaximum);
    float labelSize = sized(zAxis.labelPointSize(), textSize);
    zAxis.textAttributes().setSize(labelSize);
    zAxis.titleAttributes().setSize(sized(zAxis.titlePointSize(), textSize));
    zAxis.setMajorTickSize(0.4*textSize);
    zAxis.setMinorTickSize(0.2*textSize);
    zAxis.setLabelOffset(0.35*labelSize*zAxis.labelGap());
    zAxis.setNMinorTicks(5);
    zAxis.setTitleOffset(zAxis.titleOffsetX()*cellWidth, zAxis.titleOffsetY()*cellHeight);
    if (!view.logZ) zAxis.setNMajorTicks(Graph::niceTickCount(view.zMinimum, view.zMaximum));
    zAxis.draw(painter);
    Rect2D body = zAxis.bodyRect();
    body.include(Rect2D(x0, box.y0, x1, box.y1));                    // the bar belongs to the axis (to pick it)
    graph->zAxis().setDrawnRects(zAxis.titleRect(), body);
    }
  else
    graph->zAxis().setDrawnRects(Rect2D(), Rect2D());

  // title above the frame, moved by the title offset (fractions of the cell)
  Rect2D titleRect;
  if (!graph->title().isNull())
    {
    TextAttributes titleAttributes = graph->titleAttributes();
    titleAttributes.setSize(sized(graph->titlePointSize(), 1.15*textSize));
    titleAttributes.setJustification(TextAttributes::kCenter);
    Point2D position(0.5*(box.x0+box.x1) + graph->titleOffsetX()*cellWidth,
                     0.5*(box.y1 + graph->cellYHigh()) + graph->titleOffsetY()*cellHeight);
    painter.drawText(position, graph->title(), titleAttributes);
    titleRect = Rect2D::centered(position, painter.textWidth(graph->title(), titleAttributes)/W, titleAttributes.size());
    }

  // legend: upper right corner inside the frame, or where it was put (upper left corner, fractions of the frame)
  Rect2D legendRect;
  // (a map alone has no legend: its colors are explained by the color scale, its name by the title)
  bool loneMap = graph->items().size() == 1 && graph->items()[0].isMap();
  if (graph->showLegend() && !graph->items().empty() && !loneMap)
    {
    TextAttributes legendText = *TextAttributes::defaultTextAttributes();
    legendText.setSize(sized(graph->legendPointSize(), 0.9*textSize));
    legendText.setJustification(TextAttributes::kLeft);
    const float lineHeight = 1.4f*legendText.size();                 // fractions of the canvas height
    const float sample     = 2.2f*legendText.size()*H/W;             // sample length, fractions of the width
    const float gap        = 0.6f*legendText.size()*H/W;
    const float padX       = 0.5f*legendText.size()*H/W;
    const float padY       = 0.25f*lineHeight;
    float maxLabel = 0.0;
    for (const auto & item : graph->items()) maxLabel = std::max(maxLabel, painter.textWidth(item.label(), legendText)/W);
    const float legendWidth  = padX + sample + gap + maxLabel + padX;
    const float legendHeight = graph->items().size()*lineHeight + 2*padY;
    float left, top;
    if (graph->isAutoLegendPosition())
      {
      left = box.x1 - legendWidth;
      top  = box.y1;
      }
    else
      {
      left = box.x0 + graph->legendX()*(box.x1 - box.x0);
      top  = box.y0 + graph->legendY()*(box.y1 - box.y0);
      }
    legendRect = Rect2D(left, top - legendHeight, left + legendWidth, top);
    std::vector<Point2D> frame{ Point2D(legendRect.x0, legendRect.y0), Point2D(legendRect.x1, legendRect.y0),
                                Point2D(legendRect.x1, legendRect.y1), Point2D(legendRect.x0, legendRect.y1) };
    if (graph->legendFill().style() != 0 || graph->legendLine().style() != 0)
      painter.drawPolygon(frame, graph->legendFill(), graph->legendLine());
    float x0 = left + padX;
    float y  = top - padY - 0.5f*lineHeight;
    for (const auto & item : graph->items())
      {
      Point2D l(x0, y), r(x0 + sample, y), middle(x0 + 0.5f*sample, y);
      if (hasSysErrors(item))
        {
        FillAttributes fill = item.sysFillAttributes();
        LineAttributes line = item.sysLineAttributes();
        if (item.sysUsesMarkerColor())
          {
          fill.setColor(item.markerAttributes().color(), fill.alpha());
          line.setColor(item.markerAttributes().color(), line.alpha());
          }
        float hw = 0.3f*sample, hh = 0.35f*lineHeight;
        std::vector<Point2D> swatch{ Point2D(middle.x()-hw, y-hh), Point2D(middle.x()+hw, y-hh),
                                     Point2D(middle.x()+hw, y+hh), Point2D(middle.x()-hw, y+hh) };
        painter.drawPolygon(swatch, fill, item.sysStyle() == GraphItem::kSysBand ? noLine : line);
        }
      bool contoursOnly = item.isMap() && !item.showColorMap() && item.showContours()
                          && (item.kind() == GraphItem::kFunction || item.grid(0, 0, 0, 0, false, false));
      if (contoursOnly) painter.drawLine({l, r}, item.contourLineAttributes());   // contours only: their line
      else if (item.isMap())          // a sample of the palette
        {
        const Palette palette = graph->palette();
        float hh = 0.35f*lineHeight;
        for (int s=0; s<3; s++)
          painter.fillRectangle(x0 + sample*s/3.0f, y - hh, x0 + sample*(s + 1)/3.0f, y + hh, palette.color(0.15 + 0.35*s));
        }
      else
        {
        if (item.showLine())    painter.drawLine({l, r}, item.lineAttributes());
        if (item.showMarkers()) painter.drawMarker(middle, item.markerAttributes());
        }
      painter.drawText(Point2D(x0 + sample + gap, y), item.label(), legendText);
      y -= lineHeight;
      }
    }
  graph->setDrawnLayout(titleRect, legendRect, pointPositions);
  }

  } // namespace GRAPHICS
} // namespace CAP
