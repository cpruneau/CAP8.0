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
//! @file Grid2D.cpp
//! @brief Implementation of Grid2D.
#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_map>
#include "Grid2D.hpp"
#include "Function.hpp"
#include "Histogram.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  namespace
  {
  const double kNaN = std::numeric_limits<double>::quiet_NaN();

  //! Edges halfway between sorted distinct nodes; the outer edges as far out as the neighboring half spacings.
  std::vector<double> edgesAround(const std::vector<double> & nodes)
  {
  std::vector<double> edges;
  if (nodes.size() == 1) return { nodes[0] - 0.5, nodes[0] + 0.5 };
  edges.push_back(nodes[0] - 0.5*(nodes[1] - nodes[0]));
  for (std::size_t k=0; k+1<nodes.size(); k++) edges.push_back(0.5*(nodes[k] + nodes[k+1]));
  edges.push_back(nodes.back() + 0.5*(nodes.back() - nodes[nodes.size()-2]));
  return edges;
  }

  //! Sorted distinct values (values closer than a tolerance are the same).
  std::vector<double> distinct(std::vector<double> values)
  {
  std::sort(values.begin(), values.end());
  if (values.empty()) return values;
  double tolerance = 1E-9*std::max(1.0, std::fabs(values.back() - values.front()));
  std::vector<double> result{values[0]};
  for (double v : values) if (v - result.back() > tolerance) result.push_back(v);
  return result;
  }

  //! Index of the value in sorted distinct values, or -1.
  int indexOf(const std::vector<double> & nodes, double v)
  {
  auto it = std::lower_bound(nodes.begin(), nodes.end(), v - 1E-9*std::max(1.0, std::fabs(nodes.back() - nodes.front())));
  if (it == nodes.end()) return -1;
  if (std::fabs(*it - v) > 1E-9*std::max(1.0, std::fabs(nodes.back() - nodes.front()))) return -1;
  return int(it - nodes.begin());
  }

  //! n+1 edges from low to high, equally spaced (in log10 if log and low > 0).
  std::vector<double> edges(double low, double high, int n, bool log)
  {
  std::vector<double> e(std::size_t(n) + 1);
  bool useLog = log && low > 0;
  for (int k=0; k<=n; k++)
    e[k] = useLog ? std::pow(10.0, std::log10(low) + (std::log10(high) - std::log10(low))*k/n) : low + (high - low)*k/n;
  return e;
  }
  } // namespace

  Grid2D::Grid2D(const std::vector<double> & xEdges, const std::vector<double> & yEdges)
  :
  _xEdges(xEdges),
  _yEdges(yEdges),
  _values(std::size_t(std::max(0, nX()))*std::max(0, nY()), kNaN)
  { }

  bool Grid2D::range(double xLow, double xHigh, double yLow, double yHigh, bool positiveOnly, double & low, double & high) const
  {
  bool found = false;
  for (int j=0; j<nY(); j++)
    {
    if (_yEdges[j+1] < yLow || _yEdges[j] > yHigh) continue;
    for (int i=0; i<nX(); i++)
      {
      if (_xEdges[i+1] < xLow || _xEdges[i] > xHigh) continue;
      double v = value(i, j);
      if (!std::isfinite(v) || (positiveOnly && v <= 0)) continue;
      if (!found) { low = high = v; found = true; continue; }
      low = std::min(low, v);
      high = std::max(high, v);
      }
    }
  return found;
  }

  bool Grid2D::cell(double x, double y, int & i, int & j) const
  {
  if (isEmpty() || x < _xEdges.front() || x > _xEdges.back() || y < _yEdges.front() || y > _yEdges.back()) return false;
  i = int(std::upper_bound(_xEdges.begin(), _xEdges.end(), x) - _xEdges.begin()) - 1;
  j = int(std::upper_bound(_yEdges.begin(), _yEdges.end(), y) - _yEdges.begin()) - 1;
  i = std::min(std::max(i, 0), nX() - 1);
  j = std::min(std::max(j, 0), nY() - 1);
  return true;
  }

  std::vector<std::vector<Grid2D::Point>> Grid2D::contour(double level, bool logValues) const
  {
  std::vector<std::vector<Point>> lines;
  const int nx = nX(), ny = nY();
  if (nx < 2 || ny < 2 || !std::isfinite(level) || (logValues && level <= 0)) return lines;
  const double target = logValues ? std::log10(level) : level;
  std::vector<double> cx(nx), cy(ny);
  for (int i=0; i<nx; i++) cx[i] = 0.5*(_xEdges[i] + _xEdges[i+1]);
  for (int j=0; j<ny; j++) cy[j] = 0.5*(_yEdges[j] + _yEdges[j+1]);
  auto at = [&](int i, int j)
    {
    double v = value(i, j);
    if (logValues) v = v > 0 ? std::log10(v) : kNaN;
    return v;
    };
  // crossings on the edges between neighboring centers: horizontal edge (i,j)-(i+1,j) has id 2(i + nx j),
  // vertical edge (i,j)-(i,j+1) has id 2(i + nx j) + 1
  std::unordered_map<long,Point> crossing;
  auto edgePoint = [&](long id, double va, double vb, double xa, double ya, double xb, double yb)
    {
    if (crossing.count(id)) return;
    double t = (vb != va) ? (target - va)/(vb - va) : 0.5;
    crossing[id] = Point{ xa + t*(xb - xa), ya + t*(yb - ya) };
    };
  std::vector<std::pair<long,long>> segments;
  for (int j=0; j+1<ny; j++)
    for (int i=0; i+1<nx; i++)
      {
      double v00 = at(i, j), v10 = at(i+1, j), v11 = at(i+1, j+1), v01 = at(i, j+1);
      if (!std::isfinite(v00) || !std::isfinite(v10) || !std::isfinite(v11) || !std::isfinite(v01)) continue;
      int index = (v00 >= target) | ((v10 >= target) << 1) | ((v11 >= target) << 2) | ((v01 >= target) << 3);
      if (index == 0 || index == 15) continue;
      long bottom = 2L*(i + long(nx)*j), top = 2L*(i + long(nx)*(j+1)), left = 2L*(i + long(nx)*j) + 1, right = 2L*(i + 1 + long(nx)*j) + 1;
      edgePoint(bottom, v00, v10, cx[i], cy[j], cx[i+1], cy[j]);
      edgePoint(top,    v01, v11, cx[i], cy[j+1], cx[i+1], cy[j+1]);
      edgePoint(left,   v00, v01, cx[i], cy[j], cx[i], cy[j+1]);
      edgePoint(right,  v10, v11, cx[i+1], cy[j], cx[i+1], cy[j+1]);
      bool centerHigh = 0.25*(v00 + v10 + v11 + v01) >= target;
      switch (index)
        {
        case 1: case 14: segments.push_back({left, bottom}); break;
        case 2: case 13: segments.push_back({bottom, right}); break;
        case 3: case 12: segments.push_back({left, right}); break;
        case 4: case 11: segments.push_back({right, top}); break;
        case 6: case 9:  segments.push_back({bottom, top}); break;
        case 7: case 8:  segments.push_back({left, top}); break;
        case 5:          // corners 00 and 11 above the level
          if (centerHigh) { segments.push_back({bottom, right}); segments.push_back({top, left}); }
          else            { segments.push_back({left, bottom}); segments.push_back({right, top}); }
          break;
        case 10:         // corners 10 and 01 above the level
          if (centerHigh) { segments.push_back({left, bottom}); segments.push_back({right, top}); }
          else            { segments.push_back({bottom, right}); segments.push_back({top, left}); }
          break;
        }
      }
  // join the segments that share an edge into polylines
  std::unordered_map<long,std::vector<int>> byEdge;
  for (int s=0; s<int(segments.size()); s++) { byEdge[segments[s].first].push_back(s); byEdge[segments[s].second].push_back(s); }
  std::vector<bool> used(segments.size(), false);
  auto next = [&](long edge)
    {
    for (int s : byEdge[edge]) if (!used[s]) return s;
    return -1;
    };
  for (int s0=0; s0<int(segments.size()); s0++)
    {
    if (used[s0]) continue;
    used[s0] = true;
    std::vector<long> chain{ segments[s0].first, segments[s0].second };
    for (int s; (s = next(chain.back())) >= 0; )
      {
      used[s] = true;
      chain.push_back(segments[s].first == chain.back() ? segments[s].second : segments[s].first);
      }
    for (int s; chain.front() != chain.back() && (s = next(chain.front())) >= 0; )
      {
      used[s] = true;
      chain.insert(chain.begin(), segments[s].first == chain.front() ? segments[s].second : segments[s].first);
      }
    std::vector<Point> line;
    for (long edge : chain) line.push_back(crossing[edge]);
    lines.push_back(line);
    }
  return lines;
  }

  Grid2D Grid2D::fromHistogram(const MATH::Histogram & histogram)
  {
  if (histogram.rank() != 2) return Grid2D();
  unsigned int nx = histogram.nBins(0), ny = histogram.nBins(1);
  std::vector<double> xEdges, yEdges;
  for (unsigned int i=0; i<=nx; i++) xEdges.push_back(i < nx ? histogram.binLowEdge(i, 0) : histogram.binHighEdge(nx-1, 0));
  for (unsigned int j=0; j<=ny; j++) yEdges.push_back(j < ny ? histogram.binLowEdge(j, 1) : histogram.binHighEdge(ny-1, 1));
  Grid2D grid(xEdges, yEdges);
  for (unsigned int j=0; j<ny; j++)
    for (unsigned int i=0; i<nx; i++)
      {
      unsigned int index = i + nx*j;           // axis 0 varies fastest
      double v = histogram.valueAt(index), e = histogram.statErrorAt(index);
      if (v == 0 && e == 0) continue;           // empty bin: no value
      grid.setValue(int(i), int(j), v);
      }
  return grid;
  }

  Grid2D Grid2D::fromFunction(const MATH::Function & function, double xLow, double xHigh, int nX,
                              double yLow, double yHigh, int nY, bool logX, bool logY)
  {
  if (function.rank() != 2 || !(xHigh > xLow) || !(yHigh > yLow) || nX < 1 || nY < 1) return Grid2D();
  Grid2D grid(edges(xLow, xHigh, nX, logX), edges(yLow, yHigh, nY, logY));
  for (int j=0; j<nY; j++)
    {
    double y = logY && yLow > 0 ? std::sqrt(grid._yEdges[j]*grid._yEdges[j+1]) : 0.5*(grid._yEdges[j] + grid._yEdges[j+1]);
    for (int i=0; i<nX; i++)
      {
      double x = logX && xLow > 0 ? std::sqrt(grid._xEdges[i]*grid._xEdges[i+1]) : 0.5*(grid._xEdges[i] + grid._xEdges[i+1]);
      double v = kNaN;
      try { v = function.evaluateAt(x, y); } catch (...) { }
      grid.setValue(i, j, std::isfinite(v) ? v : kNaN);
      }
    }
  return grid;
  }

  bool Grid2D::fromPoints(const MATH::DataPoints & data, Grid2D & grid)
  {
  if (data.rank() != 2 || data.size() == 0) return false;
  std::vector<double> xs, ys, zs;
  for (unsigned int k=0; k<data.size(); k++)
    {
    std::vector<double> c = data.coordinatesAt(k);
    if (c.size() != 2 || !std::isfinite(c[0]) || !std::isfinite(c[1])) return false;
    xs.push_back(c[0]); ys.push_back(c[1]); zs.push_back(data.valueAt(k));
    }
  std::vector<double> xNodes = distinct(xs), yNodes = distinct(ys);
  if (xNodes.size()*yNodes.size() != data.size()) return false;
  Grid2D result(edgesAround(xNodes), edgesAround(yNodes));
  std::vector<bool> seen(data.size(), false);
  for (std::size_t k=0; k<xs.size(); k++)
    {
    int i = indexOf(xNodes, xs[k]), j = indexOf(yNodes, ys[k]);
    if (i < 0 || j < 0) return false;
    std::size_t cell = std::size_t(i) + xNodes.size()*j;
    if (seen[cell]) return false;               // a node twice: not a regular grid
    seen[cell] = true;
    result.setValue(i, j, std::isfinite(zs[k]) ? zs[k] : kNaN);
    }
  grid = result;
  return true;
  }

  bool Grid2D::fromData(const MATH::DataPoints & data, Grid2D & grid)
  {
  if (const MATH::Histogram * histogram = dynamic_cast<const MATH::Histogram*>(&data))
    {
    grid = fromHistogram(*histogram);
    return !grid.isEmpty();
    }
  return fromPoints(data, grid);
  }

  } // namespace GRAPHICS
} // namespace CAP
