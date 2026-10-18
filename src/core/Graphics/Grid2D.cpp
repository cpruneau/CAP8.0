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
