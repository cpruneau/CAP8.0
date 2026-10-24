/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Graph.cpp
//! @brief Implementation of Graph and GraphItem.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>
#include "Graph.hpp"
#include "GraphRenderer.hpp"
#include "Function.hpp"
#include "DataPoints.hpp"
#include "FitRecord.hpp"
#include "ParameterizedFunction.hpp"
#include "Histogram.hpp"


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
  _yMinimum(0.0),
  _yMaximum(1.0),
  _nSamples(200),
  _mapSamples(100),
  _showColorMap(true),
  _grid(),
  _gridKey(),
  _showContours(false),
  _contourIntervals(10),
  _contourLevels(),
  _contourLine(*LineAttributes::defaultLineAttributes()),
  _contourUsesPalette(false),
  _showValues(false),
  _valueFormat("%.3g"),
  _valuePointSize(0.0f),
  _valueColor(-1),
  _showMapPoints(false),
  _pointSizeByValue(false),
  _pointOutline(true),
  _style3D(kSurface),
  _showSurfaceLines(true),
  _surfaceSamples(40),
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
  _yMinimum(0.0),
  _yMaximum(1.0),
  _nSamples(200),
  _mapSamples(100),
  _showColorMap(true),
  _grid(),
  _gridKey(),
  _showContours(false),
  _contourIntervals(10),
  _contourLevels(),
  _contourLine(*LineAttributes::defaultLineAttributes()),
  _contourUsesPalette(false),
  _showValues(false),
  _valueFormat("%.3g"),
  _valuePointSize(0.0f),
  _valueColor(-1),
  _showMapPoints(false),
  _pointSizeByValue(false),
  _pointOutline(true),
  _style3D(kSurface),
  _showSurfaceLines(true),
  _surfaceSamples(40),
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
  if (dynamic_cast<const MATH::Histogram*>(data)) _style3D = kLego;   // bins: boxes
  _marker.setStyle(20);
  _sysFill.set(1, 0.25, 1001);    // translucent solid fill (the color follows the markers)
  _sysLine.set(1, 1.0, 1, 1.0);
  }

  GraphItem::GraphItem(const MATH::Function * function, float xMinimum, float xMaximum, float yMinimum, float yMaximum,
                       const String & label)
  :
  GraphItem(function, xMinimum, xMaximum, label)
  {
  _yMinimum = yMinimum;
  _yMaximum = yMaximum;
  }

  unsigned int GraphItem::rank() const
  {
  if (_kind == kFunction) return _function ? _function->rank() : 1;
  return _data ? _data->rank() : 1;
  }

  std::shared_ptr<const Grid2D> GraphItem::grid(double xLow, double xHigh, double yLow, double yHigh, bool logX, bool logY, int samples) const
  {
  if (samples <= 0) samples = _mapSamples;
  if (!isMap()) return nullptr;
  if (_kind == kData)
    {
    // made at each call: data may change between drawings, and histograms are quick to read
    Grid2D grid;
    if (!_data || !Grid2D::fromData(*_data, grid)) return nullptr;
    return std::make_shared<const Grid2D>(grid);
    }
  if (!_function) return nullptr;
  double x0 = std::max<double>(_xMinimum, xLow), x1 = std::min<double>(_xMaximum, xHigh);
  double y0 = std::max<double>(_yMinimum, yLow), y1 = std::min<double>(_yMaximum, yHigh);
  if (!(x1 > x0) || !(y1 > y0)) return nullptr;
  std::vector<double> key = { double(reinterpret_cast<std::uintptr_t>(_function)), x0, x1, y0, y1,
                              double(logX), double(logY), double(samples) };
  if (_grid && key == _gridKey) return _grid;            // a function is evaluated again only when needed
  Grid2D grid = Grid2D::fromFunction(*_function, x0, x1, samples, y0, y1, samples, logX, logY);
  _grid = grid.isEmpty() ? nullptr : std::make_shared<const Grid2D>(grid);
  _gridKey = key;
  return _grid;
  }

  bool GraphItem::isFittable() const
  {
  if (_kind == kData) return _data != nullptr && _data->size() > 0 && _data->rank() <= 2;
  return _function != nullptr && _function->rank() <= 2;
  }

  void GraphItem::applyDefaultStyle(int k)
  {
  static const int colors[]  = {1, 2, 4, 8, 6, 9, 28, 46};
  static const int markers[] = {20, 21, 22, 23, 33, 29, 34, 47};
  int n = (k < 0 ? 0 : k) % 8;
  _line.setColor(colors[n]);
  _marker.setColor(colors[n]);
  _marker.setStyle(markers[n]);
  }

  unsigned int GraphItem::nFits() const
  {
  if (_kind == kFunction) return _function ? _function->nFits() : 0;
  return _data ? _data->nFits() : 0;
  }

  const MATH::FitRecord * GraphItem::fit(unsigned int k) const
  {
  if (k >= nFits()) return nullptr;
  return _kind == kFunction ? &_function->fit(k) : &_data->fit(k);
  }

  void GraphItem::setFitShown(unsigned int k, bool v)
  {
  if (k >= _fitHidden.size()) { if (v) return; _fitHidden.resize(k + 1, false); }
  _fitHidden[k] = !v;
  }

  LineAttributes GraphItem::fitLineAttributes(unsigned int k) const
  {
  return (k < _fitLineSet.size() && _fitLineSet[k]) ? _fitLines[k] : defaultFitLine(k);
  }

  void GraphItem::setFitLineAttributes(unsigned int k, const LineAttributes & line)
  {
  if (k >= _fitLines.size()) { _fitLines.resize(k + 1); _fitLineSet.resize(k + 1, false); }
  _fitLines[k] = line;
  _fitLineSet[k] = true;
  }

  LineAttributes GraphItem::defaultFitLine(unsigned int k)
  {
  static const int colors[] = { 2, 4, 8, 6, 9, 46, 28, 7 };   // red, blue, green, magenta, ...
  LineAttributes line;
  line.set(colors[k % 8], 1.0, 1, 2.0);
  return line;
  }

  String GraphItem::fitSummary(const MATH::FitRecord & fit)
  {
  char text[96];
  const MATH::FitResult & r = fit.result;
  if (r.statistic.contains("unbinned"))
    std::snprintf(text, sizeof(text), "-2 ln L = %.4g", r.chiSquare);
  else
    std::snprintf(text, sizeof(text), "\xCF\x87\xC2\xB2/ndf = %.4g/%d", r.chiSquare, r.nDegreesOfFreedom);
  return String(text);
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
  _azimuth(30.0f), _elevation(30.0f),
  _xAxis(),
  _yAxis(),
  _zAxis(),
  _logZ(false), _autoZ(true), _zMinimum(0.0), _zMaximum(1.0), _symmetricZ(false),
  _paletteType(Palette::kViridis), _paletteSteps(0), _showColorScale(true),
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
  _zAxis.setTitle("");
  _zAxis.setTickSide(1);            // along the right edge of the color scale: ticks into it, labels to the right
  _residualAxis.setTitle("");       // automatic: "data - fit" or "pull"
  _residualAxis.setTickSide(-1);
  setRenderer(GraphRenderer::defaultRenderer());
  }

  Graph::Graph(const Graph & src)
  :
  Drawable2D(src),
  _cellXLow(src._cellXLow), _cellYLow(src._cellYLow), _cellXHigh(src._cellXHigh), _cellYHigh(src._cellYHigh),
  _margins(src._margins),
  _items(src._items),
  _shapes(src._shapes),
  _logX(src._logX), _logY(src._logY),
  _autoX(src._autoX), _autoY(src._autoY),
  _xMinimum(src._xMinimum), _xMaximum(src._xMaximum), _yMinimum(src._yMinimum), _yMaximum(src._yMaximum),
  _dimension(src._dimension),
  _azimuth(src._azimuth), _elevation(src._elevation),
  _xAxis(src._xAxis),
  _yAxis(src._yAxis),
  _zAxis(src._zAxis),
  _logZ(src._logZ), _autoZ(src._autoZ), _zMinimum(src._zMinimum), _zMaximum(src._zMaximum), _symmetricZ(src._symmetricZ),
  _paletteType(src._paletteType), _paletteSteps(src._paletteSteps), _showColorScale(src._showColorScale),
  _frameFill(src._frameFill),
  _frameLine(src._frameLine),
  _titleAttributes(src._titleAttributes),
  _showLegend(src._showLegend),
  _showFits(src._showFits),
  _showFitStatistics(src._showFitStatistics),
  _showResiduals(src._showResiduals),
  _residualMode(src._residualMode),
  _residualFraction(src._residualFraction),
  _autoResidual(src._autoResidual),
  _residualMinimum(src._residualMinimum),
  _residualMaximum(src._residualMaximum),
  _residualAxis(src._residualAxis),
  _titleOffsetX(src._titleOffsetX), _titleOffsetY(src._titleOffsetY),
  _titlePointSize(src._titlePointSize),
  _autoLegend(src._autoLegend),
  _legendX(src._legendX), _legendY(src._legendY),
  _legendPointSize(src._legendPointSize),
  _legendFill(src._legendFill),
  _legendLine(src._legendLine),
  _titleRect(src._titleRect), _legendRect(src._legendRect), _pointPositions(src._pointPositions)
  { }

  void Graph::draw(Painter & painter) const
  {
  if (_excluded) return;
  Drawable2D::draw(painter);
  for (const auto & shape : _shapes) shape.draw(painter, *this);
  }

  int Graph::shapeAt(const Point2D & p, float toleranceX, float toleranceY) const
  {
  float w = _cellXHigh - _cellXLow, h = _cellYHigh - _cellYLow;
  if (w <= 0 || h <= 0) return -1;
  Point2D q = Shape::fromCanvas(*this, p);
  for (int k=int(_shapes.size())-1; k>=0; k--) if (_shapes[size_t(k)].contains(q, toleranceX/w, toleranceY/h)) return k;
  return -1;
  }

  Graph & Graph::operator=(const Graph & rhs)
  {
  if (this != &rhs)
    {
    Drawable2D::operator=(rhs);
    _cellXLow = rhs._cellXLow; _cellYLow = rhs._cellYLow; _cellXHigh = rhs._cellXHigh; _cellYHigh = rhs._cellYHigh;
    _margins = rhs._margins;
    _items = rhs._items;
    _shapes = rhs._shapes;
    _logX = rhs._logX; _logY = rhs._logY;
    _autoX = rhs._autoX; _autoY = rhs._autoY;
    _xMinimum = rhs._xMinimum; _xMaximum = rhs._xMaximum; _yMinimum = rhs._yMinimum; _yMaximum = rhs._yMaximum;
    _dimension = rhs._dimension;
    _azimuth = rhs._azimuth; _elevation = rhs._elevation;
    _xAxis = rhs._xAxis;
    _yAxis = rhs._yAxis;
    _zAxis = rhs._zAxis;
    _logZ = rhs._logZ; _autoZ = rhs._autoZ; _zMinimum = rhs._zMinimum; _zMaximum = rhs._zMaximum; _symmetricZ = rhs._symmetricZ;
    _paletteType = rhs._paletteType; _paletteSteps = rhs._paletteSteps; _showColorScale = rhs._showColorScale;
    _frameFill = rhs._frameFill;
    _frameLine = rhs._frameLine;
    _titleAttributes = rhs._titleAttributes;
    _showLegend = rhs._showLegend;
    _showFits = rhs._showFits;
    _showFitStatistics = rhs._showFitStatistics;
    _showResiduals = rhs._showResiduals;
    _residualMode = rhs._residualMode;
    _residualFraction = rhs._residualFraction;
    _autoResidual = rhs._autoResidual;
    _residualMinimum = rhs._residualMinimum;
    _residualMaximum = rhs._residualMaximum;
    _residualAxis = rhs._residualAxis;
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
  float Graph::frameXHigh() const { return _cellXHigh - (_margins.right() + colorScaleWidth())*(_cellXHigh-_cellXLow); }

  void Graph::setViewAngles(float azimuth, float elevation)
  {
  azimuth = std::fmod(azimuth, 360.0f);
  if (azimuth < 0) azimuth += 360.0f;
  _azimuth = azimuth;
  _elevation = std::min(90.0f, std::max(-90.0f, elevation));
  }

  bool Graph::hasMaps() const
  {
  for (const auto & item : _items) if (item.isMap()) return true;
  return false;
  }

  float Graph::colorScaleWidth() const
  {
  return (_showColorScale && hasMaps()) ? 0.15f : 0.0f;
  }

  std::vector<double> Graph::contourLevels(const View & view, const GraphItem & item) const
  {
  std::vector<double> levels = item.contourLevels();
  if (!levels.empty())
    {
    std::sort(levels.begin(), levels.end());
    return levels;
    }
  int n = item.contourIntervals();
  bool log = view.logZ && view.zMinimum > 0;
  for (int k=1; k<n; k++)
    levels.push_back(log ? std::pow(10.0, std::log10(view.zMinimum) + (std::log10(view.zMaximum) - std::log10(view.zMinimum))*k/n)
                         : view.zMinimum + (view.zMaximum - view.zMinimum)*k/n);
  return levels;
  }

  Color Graph::colorOf(const View & view, double z) const
  {
  double t;
  if (view.logZ && view.zMinimum > 0)
    t = z > 0 ? (std::log10(z) - std::log10(view.zMinimum))/(std::log10(view.zMaximum) - std::log10(view.zMinimum)) : 0.0;
  else
    t = view.zMaximum > view.zMinimum ? (z - view.zMinimum)/(view.zMaximum - view.zMinimum) : 0.5;
  return palette().color(t);
  }
  float Graph::frameYLow()  const { return hasResidualPanel() ? residualYHigh() + residualGap() : residualYLow(); }

  float Graph::residualGap() const { return 0.05f*(_cellYHigh - _cellYLow); }

  float Graph::residualYLow() const { return _cellYLow + _margins.bottom()*(_cellYHigh-_cellYLow); }

  float Graph::residualYHigh() const
  {
  float low = residualYLow();
  return low + _residualFraction*(frameYHigh() - residualGap() - low);
  }

  bool Graph::hasResidualPanel() const
  {
  return _showResiduals && _dimension == k2D && hasFits(1);   // residuals of curves; those of maps are maps
  }

  bool Graph::residualContains(const Point2D & p) const
  {
  return hasResidualPanel() && p.x() >= frameXLow() && p.x() <= frameXHigh() && p.y() >= residualYLow() && p.y() < residualYHigh();
  }

  bool Graph::residualAt(const GraphItem & item, const MATH::FitRecord & fit, unsigned int index, ResidualMode mode,
                         double & x, double & r, double & error)
  {
  if (item.kind() != GraphItem::kData || !item.data() || index >= item.data()->size()) return false;
  MATH::DataPoint2D p = item.data()->point(index);
  x = p.x();
  if (!std::isfinite(x) || !std::isfinite(p.y())) return false;
  if (!item.extendFits() && !fit.low.empty() && !fit.inRange(x)) return false;
  double f = fit.valueAt(x);
  if (!std::isfinite(f)) return false;
  double sigma = p.yStatError();
  if (mode == kPull)
    {
    if (!(sigma > 0)) return false;
    r = (p.y() - f)/sigma;
    error = 0;
    }
  else
    {
    r = p.y() - f;
    error = sigma > 0 ? sigma : 0;
    }
  return std::isfinite(r);
  }

  void Graph::residualRange(const View & view, double & low, double & high) const
  {
  if (!_autoResidual && _residualMaximum > _residualMinimum) { low = _residualMinimum; high = _residualMaximum; return; }
  double largest = 0;
  for (const auto & item : _items)
    {
    if (item.isMap() || !item.showFits() || !_showFits) continue;
    for (unsigned int k=0; k<item.nFits(); k++)
      {
      const MATH::FitRecord * fit = item.fit(k);
      if (!fit || !fit->function || !item.isFitShown(k)) continue;
      if (item.kind() == GraphItem::kData)
        {
        for (unsigned int i=0; i<item.data()->size(); i++)
          {
          double x, r, e;
          if (!residualAt(item, *fit, i, _residualMode, x, r, e) || x < view.xMinimum || x > view.xMaximum) continue;
          largest = std::max(largest, std::fabs(r) + e);
          }
        }
      else if (item.function())
        {
        double a = std::max<double>(view.xMinimum, item.extendFits() || fit->low.empty() ? view.xMinimum : fit->low[0]);
        double b = std::min<double>(view.xMaximum, item.extendFits() || fit->high.empty() ? view.xMaximum : fit->high[0]);
        for (int i=0; i<=100 && b > a; i++)
          {
          double x = a + (b - a)*i/100.0, r;
          try { r = item.function()->evaluateAt(x) - fit->valueAt(x); } catch (...) { continue; }
          if (std::isfinite(r)) largest = std::max(largest, std::fabs(r));
          }
        }
      }
    }
  if (!(largest > 0)) largest = 1;
  double step;
  low = -1.05*largest; high = 1.05*largest;
  niceLinear(low, high, step);
  }

  Point2D Graph::toResidualCanvas(const View & view, double low, double high, double x, double r) const
  {
  Point2D p = toCanvas(view, x, view.yMinimum);
  double f = high > low ? (r - low)/(high - low) : 0.5;
  return Point2D(p.x(), residualYLow() + f*(residualYHigh() - residualYLow()));
  }
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
  if (_items.empty()) { if (cellContains(p)) result.part = kCell; return result; }   // nothing drawn
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
  // the axes before the frame: their line and ticks lie on the edge of the frame and inside it
  for (int k=0; k<nAxes(); k++)
    if (axis(k).bodyRect().expanded(0.5f*toleranceX, 0.5f*toleranceY).contains(p)) { result.part = kAxis; result.axis = k; return result; }
  if (frameContains(p)) { result.part = kFrame; return result; }
  if (residualContains(p)) { result.part = kResiduals; return result; }
  if (cellContains(p)) result.part = kCell;
  return result;
  }


  GraphItem & Graph::show(const GraphItem & item, bool replace)
  {
  bool fresh = _items.empty() || replace;
  if (replace) _items.clear();
  for (auto & existing : _items) if (existing.object() == item.object()) return existing;
  _items.push_back(item);
  GraphItem & added = _items.back();
  added.applyDefaultStyle(int(_items.size()) - 1);
  if (fresh)
    {
    setTitlesFrom(added);
    setAutoRange();
    }
  return added;
  }

  void Graph::setTitlesFrom(const GraphItem & item)
  {
  const MATH::Histogram * histogram = item.kind() == GraphItem::kData ? dynamic_cast<const MATH::Histogram*>(item.data()) : nullptr;
  String title;
  if (item.kind() == GraphItem::kData && item.data()) title = item.data()->title();
  else if (item.function()) title = item.function()->title();
  setTitle(title.isNull() ? item.label() : title);
  _xAxis.setTitle(histogram ? histogram->dimensionTitle(0) : String(""));
  _yAxis.setTitle(histogram && histogram->rank() == 2 ? histogram->dimensionTitle(1) : String(""));
  _zAxis.setTitle("");
  }

  bool Graph::hasMovedTexts() const
  {
  if (!_autoLegend || _titleOffsetX != 0 || _titleOffsetY != 0) return true;
  for (const Axis * axis : { &_xAxis, &_yAxis, &_zAxis, &_residualAxis })
    if (axis->titleOffsetX() != 0 || axis->titleOffsetY() != 0) return true;
  return false;
  }

  void Graph::resetTextPositions()
  {
  _autoLegend = true;
  _titleOffsetX = _titleOffsetY = 0;
  for (Axis * axis : { &_xAxis, &_yAxis, &_zAxis, &_residualAxis }) axis->setTitleOffset(0, 0);
  }

  bool Graph::zoom(const Point2D & a, const Point2D & b, bool x, bool y)
  {
  const View v = view();
  double xa, ya, xb, yb;
  fromCanvas(v, Point2D(std::min(a.x(), b.x()), std::min(a.y(), b.y())), xa, ya);
  fromCanvas(v, Point2D(std::max(a.x(), b.x()), std::max(a.y(), b.y())), xb, yb);
  if ((x && !(xb > xa)) || (y && !(yb > ya))) return false;
  if (x) setXRange(xa, xb);
  if (y) setYRange(ya, yb);
  return true;
  }

  bool Graph::valueAt(const View & v, double x, double y, double & z, int & index) const
  {
  for (int n=int(_items.size())-1; n>=0; n--)
    {
    const GraphItem & item = _items[std::size_t(n)];
    if (!item.isMap()) continue;
    std::shared_ptr<const Grid2D> grid = item.grid(v.xMinimum, v.xMaximum, v.yMinimum, v.yMaximum, v.logX, v.logY);
    int i, j;
    if (!grid || !grid->cell(x, y, i, j) || !std::isfinite(grid->value(i, j))) continue;
    z = grid->value(i, j);
    index = n;
    return true;
    }
  return false;
  }

  void Graph::fromResidualCanvas(const View & v, const Point2D & p, double & x, double & r) const
  {
  double y, low, high;
  fromCanvas(v, p, x, y);
  residualRange(v, low, high);
  double height = residualYHigh() - residualYLow();
  r = low + (height > 0 ? (p.y() - residualYLow())/height : 0.5)*(high - low);
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

  bool Graph::hasFits(unsigned int rank) const
  {
  if (!_showFits) return false;
  for (const auto & item : _items)
    {
    if (rank != 0 && item.rank() != rank) continue;
    for (unsigned int k=0; k<item.nFits(); k++) if (drawnFit(item, k)) return true;
    }
  return false;
  }

  const MATH::FitRecord * Graph::drawnFit(const GraphItem & item, unsigned int k) const
  {
  if (!_showFits || !item.showFits() || !item.isFitShown(k)) return nullptr;
  const MATH::FitRecord * fit = item.fit(k);
  if (!fit || !fit->function || fit->function->rank() != item.rank() || item.rank() > 2) return nullptr;
  return fit;
  }

  void Graph::setResidualMapStyle(bool pulls)
  {
  setPalette(Palette(Palette::kDiverging, 0));
  setSymmetricZ(true);
  setAutoZ();
  _zAxis.setTitle(pulls ? "pull" : "data \xE2\x88\x92 fit");
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
  bool hasCurves = false;         // items of rank 1
  bool maps = false;              // items of rank 2

  // extent of an item of rank 2 in x or y: its grid, its points, or the range of its function
  auto mapExtent = [&](const GraphItem & item, int k, double & low, double & high)
    {
    bool log = k == 0 ? _logX : _logY;
    if (item.kind() == GraphItem::kFunction)
      {
      include(k == 0 ? item.xMinimum() : item.yMinimum(), log, low, high);
      include(k == 0 ? item.xMaximum() : item.yMaximum(), log, low, high);
      return;
      }
    if (std::shared_ptr<const Grid2D> grid = item.grid(-inf, inf, -inf, inf, _logX, _logY))
      {
      for (double edge : (k == 0 ? grid->xEdges() : grid->yEdges())) include(edge, log, low, high);
      return;
      }
    const MATH::DataPoints & d = *item.data();             // scattered points
    for (unsigned int n=0; n<d.size(); n++) include(d.coordinatesAt(n)[k], log, low, high);
    };

  // x range: data extent, function curve ranges, maps
  for (const auto & item : _items)
    {
    if ((item.kind() == GraphItem::kData && !item.data()) || (item.kind() == GraphItem::kFunction && !item.function())) continue;
    if (item.isMap()) { maps = true; mapExtent(item, 0, xLow, xHigh); continue; }
    hasCurves = true;
    if (item.kind() == GraphItem::kData)
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
    else
      {
      include(item.xMinimum(), _logX, xLow, xHigh);
      include(item.xMaximum(), _logX, xLow, xHigh);
      }
    }
  const bool mapsOnly = maps && !hasCurves;   // a graph of maps only takes the exact extent of the maps
  if (_autoX)
    {
    if (!(xHigh >= xLow)) { xLow = _logX ? 0.1 : 0.0; xHigh = _logX ? 10.0 : 1.0; }
    if (mapsOnly && xHigh > xLow) { }
    else if (_logX) niceLog(xLow, xHigh);
    else niceLinear(xLow, xHigh);
    }
  else
    {
    xLow = _xMinimum; xHigh = _xMaximum;
    }
  v.xMinimum = xLow;
  v.xMaximum = xHigh;

  // y range: data (with errors) and function values within the x range, maps
  for (const auto & item : _items)
    {
    if ((item.kind() == GraphItem::kData && !item.data()) || (item.kind() == GraphItem::kFunction && !item.function())) continue;
    if (item.isMap()) { mapExtent(item, 1, yLow, yHigh); continue; }
    if (item.kind() == GraphItem::kData)
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
    else if (item.function()->rank() == 1)
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
    if (mapsOnly && yHigh > yLow) { }
    else if (_logY)
      niceLog(yLow, yHigh);
    else
      {
      if (hasCurves && yLow > 0 && yLow < 0.25*yHigh) yLow = 0.0;    // start at zero when the data are well above it
      if (hasCurves && yLow < 0 && yCentralMinimum >= 0) yLow = 0.0; // values >= 0: errors bars do not push the axis below zero
      niceLinear(yLow, yHigh);
      }
    }
  else
    {
    yLow = _yMinimum; yHigh = _yMaximum;
    }
  v.yMinimum = yLow;
  v.yMaximum = yHigh;

  // z range: the values of the maps in view
  v.hasZ = maps;
  v.logZ = _logZ;
  if (maps)
    {
    double zLow = inf, zHigh = -inf;
    for (const auto & item : _items)
      {
      if (!item.isMap() || (item.kind() == GraphItem::kData && !item.data()) || (item.kind() == GraphItem::kFunction && !item.function())) continue;
      double low, high;
      if (std::shared_ptr<const Grid2D> grid = item.grid(xLow, xHigh, yLow, yHigh, _logX, _logY))
        {
        if (grid->range(xLow, xHigh, yLow, yHigh, _logZ, low, high)) { include(low, _logZ, zLow, zHigh); include(high, _logZ, zLow, zHigh); }
        }
      else if (item.kind() == GraphItem::kData)
        {
        const MATH::DataPoints & d = *item.data();
        for (unsigned int n=0; n<d.size(); n++)
          {
          std::vector<double> c = d.coordinatesAt(n);
          if (c[0] < xLow || c[0] > xHigh || c[1] < yLow || c[1] > yHigh) continue;
          include(d.valueAt(n), _logZ, zLow, zHigh);
          }
        }
      }
    if (_autoZ)
      {
      if (!(zHigh >= zLow)) { zLow = _logZ ? 0.1 : 0.0; zHigh = _logZ ? 10.0 : 1.0; }
      // lego boxes rise from zero: in a 3D view, the automatic linear range includes it
      bool lego = false;
      for (const auto & item : _items) lego = lego || (item.isMap() && item.style3D() == GraphItem::kLego);
      if (lego && is3D() && !_logZ) { zLow = std::min(zLow, 0.0); zHigh = std::max(zHigh, 0.0); }
      if (_logZ) niceLog(zLow, zHigh);
      else
        {
        if (_symmetricZ)
          {
          double m = std::max(std::fabs(zLow), std::fabs(zHigh));
          zLow = -m; zHigh = m;
          niceLinear(zLow, zHigh);
          m = std::max(std::fabs(zLow), std::fabs(zHigh));   // rounding may have made it lopsided
          zLow = -m; zHigh = m;
          }
        else
          {
          if (zLow > 0 && zLow < 0.25*zHigh) zLow = 0.0;    // counts: the scale starts at zero
          niceLinear(zLow, zHigh);
          }
        }
      }
    else
      {
      zLow = _zMinimum; zHigh = _zMaximum;
      }
    v.zMinimum = zLow;
    v.zMaximum = zHigh;
    }
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
