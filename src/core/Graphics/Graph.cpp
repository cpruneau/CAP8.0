#include <algorithm>
#include <cmath>
#include <limits>
#include "Graph.hpp"
#include "GraphRenderer.hpp"
#include "Function.hpp"
#include "DataPoints.hpp"

ClassImp(CAP::GRAPHICS::Graph);

namespace CAP
{
  namespace GRAPHICS
  {

  // ------------------------------------------------------------------------------------------------ GraphItem

  GraphItem::GraphItem(const MATH::Function * function, float xMinimum, float xMaximum, const String & label)
  :
  _kind(kFunction),
  _function(function),
  _data(nullptr),
  _label(label),
  _xMinimum(xMinimum),
  _xMaximum(xMaximum),
  _nSamples(200),
  _line(*LineAttributes::defaultLineAttributes()),
  _marker(*MarkerAttributes::defaultMarkerAttributes()),
  _showLine(true),
  _showMarkers(false),
  _showErrors(false),
  _showSysErrors(false),
  _sysStyle(kSysBoxes),
  _sysFill(*FillAttributes::defaultFillAttributes()),
  _sysLine(*LineAttributes::defaultLineAttributes()),
  _sysUsesMarkerColor(true),
  _sysWidth(0.012f)
  {
  if (_label.isNull() && function) _label = function->name();
  _line.setWidth(2.0);
  }

  GraphItem::GraphItem(const MATH::DataPoints * data, const String & label)
  :
  _kind(kData),
  _function(nullptr),
  _data(data),
  _label(label),
  _xMinimum(0.0),
  _xMaximum(1.0),
  _nSamples(200),
  _line(*LineAttributes::defaultLineAttributes()),
  _marker(*MarkerAttributes::defaultMarkerAttributes()),
  _showLine(false),
  _showMarkers(true),
  _showErrors(true),
  _showSysErrors(true),
  _sysStyle(kSysBoxes),
  _sysFill(*FillAttributes::defaultFillAttributes()),
  _sysLine(*LineAttributes::defaultLineAttributes()),
  _sysUsesMarkerColor(true),
  _sysWidth(0.012f)
  {
  if (_label.isNull() && data) _label = data->name();
  _marker.setStyle(20);
  _sysFill.set(1, 0.25, 1001);    // translucent solid fill (the color follows the markers)
  _sysLine.set(1, 1.0, 1, 1.0);
  }

  // ------------------------------------------------------------------------------------------------ Graph

  Graph::Graph()
  :
  Drawable2D(),
  _cellXLow(0.0), _cellYLow(0.0), _cellXHigh(1.0), _cellYHigh(1.0),
  _margins(0.18, 0.15, 0.05, 0.09),
  _items(),
  _logX(false), _logY(false),
  _autoX(true), _autoY(true),
  _xMinimum(0.0), _xMaximum(1.0), _yMinimum(0.0), _yMaximum(1.0),
  _dimension(k2D),
  _xAxis(),
  _yAxis(),
  _frameFill(*FillAttributes::defaultFillAttributes()),
  _frameLine(*LineAttributes::defaultLineAttributes()),
  _titleAttributes(*TextAttributes::defaultTextAttributes()),
  _showLegend(true),
  _titleOffsetX(0.0), _titleOffsetY(0.0),
  _titlePointSize(0.0),
  _autoLegend(true),
  _legendX(0.6), _legendY(0.95),
  _legendPointSize(0.0),
  _legendFill(*FillAttributes::defaultFillAttributes()),
  _legendLine(*LineAttributes::defaultLineAttributes()),
  _titleRect(), _legendRect(), _pointPositions()
  {
  _legendFill.set(0, 1.0, 0);       // no background
  _legendLine.set(1, 1.0, 0, 1.0);  // no border
  _frameFill.set(0, 1.0, 1001);     // white, solid
  _frameLine.set(1, 1.0, 1, 1.0);   // black, solid, 1 point
  _titleAttributes.setJustification(TextAttributes::kCenter);
  _yAxis.setTickSide(-1);           // ticks point into the frame for both axes
  _xAxis.setTickSide(1);
  _xAxis.setTitle("");              // Axis titles itself "x" by default
  _yAxis.setTitle("");
  setRenderer(GraphRenderer::defaultRenderer());
  }

  Graph::Graph(const Graph & src)
  :
  Drawable2D(src),
  _cellXLow(src._cellXLow), _cellYLow(src._cellYLow), _cellXHigh(src._cellXHigh), _cellYHigh(src._cellYHigh),
  _margins(src._margins),
  _items(src._items),
  _logX(src._logX), _logY(src._logY),
  _autoX(src._autoX), _autoY(src._autoY),
  _xMinimum(src._xMinimum), _xMaximum(src._xMaximum), _yMinimum(src._yMinimum), _yMaximum(src._yMaximum),
  _dimension(src._dimension),
  _xAxis(src._xAxis),
  _yAxis(src._yAxis),
  _frameFill(src._frameFill),
  _frameLine(src._frameLine),
  _titleAttributes(src._titleAttributes),
  _showLegend(src._showLegend),
  _titleOffsetX(src._titleOffsetX), _titleOffsetY(src._titleOffsetY),
  _titlePointSize(src._titlePointSize),
  _autoLegend(src._autoLegend),
  _legendX(src._legendX), _legendY(src._legendY),
  _legendPointSize(src._legendPointSize),
  _legendFill(src._legendFill),
  _legendLine(src._legendLine),
  _titleRect(src._titleRect), _legendRect(src._legendRect), _pointPositions(src._pointPositions)
  { }

  Graph & Graph::operator=(const Graph & rhs)
  {
  if (this != &rhs)
    {
    Drawable2D::operator=(rhs);
    _cellXLow = rhs._cellXLow; _cellYLow = rhs._cellYLow; _cellXHigh = rhs._cellXHigh; _cellYHigh = rhs._cellYHigh;
    _margins = rhs._margins;
    _items = rhs._items;
    _logX = rhs._logX; _logY = rhs._logY;
    _autoX = rhs._autoX; _autoY = rhs._autoY;
    _xMinimum = rhs._xMinimum; _xMaximum = rhs._xMaximum; _yMinimum = rhs._yMinimum; _yMaximum = rhs._yMaximum;
    _dimension = rhs._dimension;
    _xAxis = rhs._xAxis;
    _yAxis = rhs._yAxis;
    _frameFill = rhs._frameFill;
    _frameLine = rhs._frameLine;
    _titleAttributes = rhs._titleAttributes;
    _showLegend = rhs._showLegend;
    _titleOffsetX = rhs._titleOffsetX; _titleOffsetY = rhs._titleOffsetY;
    _titlePointSize = rhs._titlePointSize;
    _autoLegend = rhs._autoLegend;
    _legendX = rhs._legendX; _legendY = rhs._legendY;
    _legendPointSize = rhs._legendPointSize;
    _legendFill = rhs._legendFill;
    _legendLine = rhs._legendLine;
    _titleRect = rhs._titleRect; _legendRect = rhs._legendRect; _pointPositions = rhs._pointPositions;
    }
  return *this;
  }

  void Graph::setCell(float xLow, float yLow, float xHigh, float yHigh)
  {
  _cellXLow = xLow; _cellYLow = yLow; _cellXHigh = xHigh; _cellYHigh = yHigh;
  }

  float Graph::frameXLow()  const { return _cellXLow  + _margins.left()  *(_cellXHigh-_cellXLow); }
  float Graph::frameXHigh() const { return _cellXHigh - _margins.right() *(_cellXHigh-_cellXLow); }
  float Graph::frameYLow()  const { return _cellYLow  + _margins.bottom()*(_cellYHigh-_cellYLow); }
  float Graph::frameYHigh() const { return _cellYHigh - _margins.top()   *(_cellYHigh-_cellYLow); }

  bool Graph::cellContains(const Point2D & p) const
  {
  return p.x() >= _cellXLow && p.x() <= _cellXHigh && p.y() >= _cellYLow && p.y() <= _cellYHigh;
  }

  bool Graph::frameContains(const Point2D & p) const
  {
  return p.x() >= frameXLow() && p.x() <= frameXHigh() && p.y() >= frameYLow() && p.y() <= frameYHigh();
  }

  Graph::Pick Graph::pick(const Point2D & p, float toleranceX, float toleranceY) const
  {
  Pick result;
  if (_legendRect.contains(p)) { result.part = kLegend; return result; }
  if (_titleRect.contains(p))  { result.part = kTitle;  return result; }
  for (int k=0; k<nAxes(); k++)
    if (axis(k).titleRect().contains(p)) { result.part = kAxisTitle; result.axis = k; return result; }
  // nearest data point within the tolerance (distance in units of the tolerance)
  double best = 1.0;
  for (const auto & point : _pointPositions)
    {
    double dx = (point.position.x() - p.x())/toleranceX, dy = (point.position.y() - p.y())/toleranceY;
    double d = dx*dx + dy*dy;
    if (d <= best) { best = d; result.part = kDataPoint; result.item = point.item; result.index = point.index; }
    }
  if (result.part == kDataPoint) return result;
  if (frameContains(p)) { result.part = kFrame; return result; }
  for (int k=0; k<nAxes(); k++)
    if (axis(k).bodyRect().expanded(0.5f*toleranceX, 0.5f*toleranceY).contains(p)) { result.part = kAxis; result.axis = k; return result; }
  if (cellContains(p)) result.part = kCell;
  return result;
  }


  GraphItem & Graph::add(const GraphItem & item)
  {
  for (auto & existing : _items) if (existing.object() == item.object()) return existing;
  _items.push_back(item);
  return _items.back();
  }

  GraphItem & Graph::replace(const GraphItem & item)
  {
  _items.clear();
  _items.push_back(item);
  return _items.back();
  }

  bool Graph::remove(const void * object)
  {
  for (auto it = _items.begin(); it != _items.end(); ++it)
    if (it->object() == object) { _items.erase(it); return true; }
  return false;
  }

  void Graph::reset()
  {
  Drawable2D::reset();
  _items.clear();
  setAutoRange();
  }

  void niceLinear(double & low, double & high, double & step)
  {
  if (high <= low)
    {
    double d = (low == 0) ? 1.0 : 0.1*std::fabs(low);
    low -= d; high += d;
    }
  double span = high - low;
  double raw  = span/5.0;
  double p    = std::pow(10.0, std::floor(std::log10(raw)));
  double f    = raw/p;
  step = (f < 1.5 ? 1.0 : (f < 3.0 ? 2.0 : (f < 7.0 ? 5.0 : 10.0)))*p;
  low  = std::floor(low/step  + 1E-9)*step;
  high = std::ceil (high/step - 1E-9)*step;
  if (std::fabs(low)  < 1E-12*step) low  = 0.0;
  }

  namespace
  {
  //! Extend [low,high] to include v (positive values only when positiveOnly).
  void include(double v, bool positiveOnly, double & low, double & high)
  {
  if (!std::isfinite(v) || (positiveOnly && v <= 0)) return;
  if (v < low)  low  = v;
  if (v > high) high = v;
  }

  void niceLinear(double & low, double & high)
  {
  double step;
  CAP::GRAPHICS::niceLinear(low, high, step);
  }

  //! Round a log range [low,high] (> 0) outward to whole decades.
  void niceLog(double & low, double & high)
  {
  if (!(low > 0) || !(high > 0)) { low = 0.1; high = 10.0; return; }
  low  = std::pow(10.0, std::floor(std::log10(low)  + 1E-9));
  high = std::pow(10.0, std::ceil (std::log10(high) - 1E-9));
  if (high <= low) high = 10.0*low;
  }
  } // namespace

  Graph::View Graph::view() const
  {
  View v;
  v.logX = _logX;
  v.logY = _logY;
  const double inf = std::numeric_limits<double>::infinity();
  double xLow = inf, xHigh = -inf, yLow = inf, yHigh = -inf;
  double yCentralMinimum = inf;   // smallest value without its uncertainty

  // x range: data extent, function curve ranges
  for (const auto & item : _items)
    {
    if (item.kind() == GraphItem::kData && item.data())
      {
      const MATH::DataPoints & d = *item.data();
      for (unsigned int k=0; k<d.size(); k++)
        {
        MATH::DataPoint2D p = d.point(k);
        include(p.x() - p.xError(), _logX, xLow, xHigh);
        include(p.x() + p.xError(), _logX, xLow, xHigh);
        include(p.x(), _logX, xLow, xHigh);
        }
      }
    else if (item.kind() == GraphItem::kFunction && item.function())
      {
      include(item.xMinimum(), _logX, xLow, xHigh);
      include(item.xMaximum(), _logX, xLow, xHigh);
      }
    }
  if (_autoX)
    {
    if (!(xHigh >= xLow)) { xLow = _logX ? 0.1 : 0.0; xHigh = _logX ? 10.0 : 1.0; }
    if (_logX) niceLog(xLow, xHigh); else niceLinear(xLow, xHigh);
    }
  else
    {
    xLow = _xMinimum; xHigh = _xMaximum;
    }
  v.xMinimum = xLow;
  v.xMaximum = xHigh;

  // y range: data (with errors) and function values within the x range
  for (const auto & item : _items)
    {
    if (item.kind() == GraphItem::kData && item.data())
      {
      const MATH::DataPoints & d = *item.data();
      for (unsigned int k=0; k<d.size(); k++)
        {
        MATH::DataPoint2D p = d.point(k);
        if (p.x() < xLow || p.x() > xHigh) continue;
        double stat = item.showErrors()    ? p.yStatError() : 0.0;
        double sys  = item.showSysErrors() ? p.ySysError()  : 0.0;
        double e = std::max(stat, sys);
        if (p.y() < yCentralMinimum) yCentralMinimum = p.y();
        include(p.y() - e, _logY, yLow, yHigh);
        include(p.y() + e, _logY, yLow, yHigh);
        include(p.y(), _logY, yLow, yHigh);
        }
      }
    else if (item.kind() == GraphItem::kFunction && item.function() && item.function()->rank() == 1)
      {
      double a = std::max<double>(item.xMinimum(), xLow);
      double b = std::min<double>(item.xMaximum(), xHigh);
      int n = item.nSamples();
      for (int k=0; k<n && b > a; k++)
        {
        double x = a + (b-a)*k/(n-1);
        try
          {
          double y = item.function()->evaluateAt(x);
          include(y, _logY, yLow, yHigh);
          if (std::isfinite(y) && y < yCentralMinimum) yCentralMinimum = y;
          }
        catch (...) { }
        }
      }
    }
  if (_autoY)
    {
    if (!(yHigh >= yLow)) { yLow = _logY ? 0.1 : 0.0; yHigh = _logY ? 10.0 : 1.0; }
    if (_logY)
      niceLog(yLow, yHigh);
    else
      {
      if (yLow > 0 && yLow < 0.25*yHigh) yLow = 0.0;   // start at zero when the data are well above it
      if (yLow < 0 && yCentralMinimum >= 0) yLow = 0.0; // values >= 0: errors bars do not push the axis below zero
      niceLinear(yLow, yHigh);
      }
    }
  else
    {
    yLow = _yMinimum; yHigh = _yMaximum;
    }
  v.yMinimum = yLow;
  v.yMaximum = yHigh;
  return v;
  }

  int Graph::niceTickCount(double minimum, double maximum)
  {
  // the number of divisions whose step is 1, 2, 2.5 or 5 x 10^k and divides the range exactly, closest to 5
  double span = maximum - minimum;
  if (!(span > 0) || !std::isfinite(span)) return 1;
  int best = 0;
  double bestScore = 1E9;
  double p = std::pow(10.0, std::floor(std::log10(span)) - 2.0);
  for (int e=0; e<5; e++, p *= 10.0)
    for (double m : {1.0, 2.0, 2.5, 5.0})
      {
      double n = span/(m*p);
      if (n < 2.0 - 1E-9 || n > 10.0 + 1E-9) continue;
      if (std::fabs(n - std::round(n)) > 1E-6*n) continue;
      double score = std::fabs(std::round(n) - 5.0) + (m == 2.5 ? 0.5 : 0.0);
      if (score < bestScore) { bestScore = score; best = int(std::lround(n)); }
      }
  if (best > 0) return best;
  double low = minimum, high = maximum, step;
  CAP::GRAPHICS::niceLinear(low, high, step);
  int n = int(std::lround(span/step));
  return n < 1 ? 1 : (n > 12 ? 12 : n);
  }

  Point2D Graph::toCanvas(const View & v, double x, double y) const
  {
  auto fraction = [](double value, double low, double high, bool log)
    {
    if (log)
      {
      if (value <= 0) return -1.0;   // below the frame
      return (std::log10(value) - std::log10(low))/(std::log10(high) - std::log10(low));
      }
    return (value - low)/(high - low);
    };
  double fx = fraction(x, v.xMinimum, v.xMaximum, v.logX && v.xMinimum > 0);
  double fy = fraction(y, v.yMinimum, v.yMaximum, v.logY && v.yMinimum > 0);
  return Point2D(frameXLow() + fx*(frameXHigh()-frameXLow()), frameYLow() + fy*(frameYHigh()-frameYLow()));
  }

  void Graph::fromCanvas(const View & v, const Point2D & p, double & x, double & y) const
  {
  double fx = (p.x() - frameXLow())/(frameXHigh() - frameXLow());
  double fy = (p.y() - frameYLow())/(frameYHigh() - frameYLow());
  if (v.logX && v.xMinimum > 0) x = std::pow(10.0, std::log10(v.xMinimum) + fx*(std::log10(v.xMaximum) - std::log10(v.xMinimum)));
  else                          x = v.xMinimum + fx*(v.xMaximum - v.xMinimum);
  if (v.logY && v.yMinimum > 0) y = std::pow(10.0, std::log10(v.yMinimum) + fy*(std::log10(v.yMaximum) - std::log10(v.yMinimum)));
  else                          y = v.yMinimum + fy*(v.yMaximum - v.yMinimum);
  }

  } // namespace GRAPHICS
} // namespace CAP
