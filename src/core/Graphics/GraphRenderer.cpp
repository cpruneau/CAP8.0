#include <algorithm>
#include <cmath>
#include "GraphRenderer.hpp"
#include "Graph.hpp"
#include "Painter.hpp"
#include "Function.hpp"
#include "DataPoints.hpp"

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

  void GraphRenderer::draw(const Drawable2D & drawable, Painter & painter) const
  {
  const Graph * graph = dynamic_cast<const Graph*>(&drawable);
  if (!graph) return;
  if (painter.width() <= 0 || painter.height() <= 0) return;

  const Graph::View view = graph->view();
  const Box box { graph->frameXLow(), graph->frameYLow(), graph->frameXHigh(), graph->frameYHigh() };
  const float cellHeight = graph->cellYHigh() - graph->cellYLow();
  const float cellWidth  = graph->cellXHigh() - graph->cellXLow();
  // text and tick sizes follow the size of the cell (fractions of the canvas height)
  const float scale = std::min(cellHeight, cellWidth*painter.width()/painter.height());
  // ... within 9 to 17 device units, so that labels stay readable without dominating a large canvas
  const float textSize = std::min(std::max(0.045f*scale, 9.0f/painter.height()), 17.0f/painter.height());

  // frame background
  std::vector<Point2D> corners{ Point2D(box.x0,box.y0), Point2D(box.x1,box.y0), Point2D(box.x1,box.y1), Point2D(box.x0,box.y1) };
  LineAttributes noLine;
  noLine.setStyle(0);
  painter.drawPolygon(corners, graph->frameFill(), noLine);

  // items
  for (const auto & item : graph->items())
    {
    if (item.kind() == GraphItem::kFunction && item.function() && item.function()->rank() == 1)
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
        if (item.showErrors())
          {
          double ey = p.yError();
          if (ey > 0)
            {
            double low = p.y() - ey;
            if (view.logY && low <= 0) low = view.yMinimum;
            Point2D e0 = graph->toCanvas(view, p.x(), low);
            Point2D e1 = graph->toCanvas(view, p.x(), p.y() + ey);
            if (clip(box, e0, e1)) painter.drawLine({e0, e1}, errorLine);
            }
          double ex = p.xError();
          if (ex > 0)
            {
            double low = p.x() - ex;
            if (view.logX && low <= 0) low = view.xMinimum;
            Point2D e0 = graph->toCanvas(view, low, p.y());
            Point2D e1 = graph->toCanvas(view, p.x() + ex, p.y());
            if (clip(box, e0, e1)) painter.drawLine({e0, e1}, errorLine);
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
    axis->textAttributes().setSize(textSize);
    axis->setMajorTickSize(0.6*textSize);
    axis->setMinorTickSize(0.3*textSize);
    axis->setLabelOffset(0.35*textSize);
    axis->setNMinorTicks(5);
    }
  xAxis.setLabelOffset(0.9*textSize);   // labels centered below the axis: clear the ticks
  if (!view.logX) xAxis.setNMajorTicks(Graph::niceTickCount(view.xMinimum, view.xMaximum));
  if (!view.logY) yAxis.setNMajorTicks(Graph::niceTickCount(view.yMinimum, view.yMaximum));
  xAxis.draw(painter);
  yAxis.draw(painter);

  // title above the frame
  if (!graph->title().isNull())
    {
    TextAttributes titleAttributes = graph->titleAttributes();
    titleAttributes.setSize(1.15*textSize);
    titleAttributes.setJustification(TextAttributes::kCenter);
    float yTitle = 0.5*(box.y1 + graph->cellYHigh());
    painter.drawText(Point2D(0.5*(box.x0+box.x1), yTitle), graph->title(), titleAttributes);
    }

  // legend, upper right corner inside the frame
  if (graph->showLegend() && !graph->items().empty())
    {
    TextAttributes legendText = *TextAttributes::defaultTextAttributes();
    legendText.setSize(0.9*textSize);
    legendText.setJustification(TextAttributes::kLeft);
    const float H = painter.height(), W = painter.width();
    const float lineHeight = 1.4f*legendText.size();                 // fractions of the canvas height
    const float sample     = 2.2f*legendText.size()*H/W;             // sample length, fractions of the width
    float maxLabel = 0.0;
    for (const auto & item : graph->items()) maxLabel = std::max(maxLabel, painter.textWidth(item.label(), legendText)/W);
    float x0 = box.x1 - (sample + 0.6f*legendText.size()*H/W + maxLabel) - 0.5f*legendText.size()*H/W;
    float y  = box.y1 - 0.8f*lineHeight;
    for (const auto & item : graph->items())
      {
      Point2D left(x0, y), right(x0 + sample, y), middle(x0 + 0.5f*sample, y);
      if (item.showLine())    painter.drawLine({left, right}, item.lineAttributes());
      if (item.showMarkers()) painter.drawMarker(middle, item.markerAttributes());
      painter.drawText(Point2D(x0 + sample + 0.6f*legendText.size()*H/W, y), item.label(), legendText);
      y -= lineHeight;
      if (y < box.y0) break;
      }
    }
  }

  } // namespace GRAPHICS
} // namespace CAP
