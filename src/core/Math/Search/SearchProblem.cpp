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
//! @file SearchProblem.cpp
//! @brief Implementation of SearchProblem, FunctionProblem and DataProblem.
#include <algorithm>
#include <cmath>
#include <limits>
#include "SearchProblem.hpp"
#include "DataPoints.hpp"
#include "Function.hpp"
#include "Histogram.hpp"
#include "MathExceptions.hpp"
#include "ParameterizedFunction.hpp"

namespace CAP
{
  namespace MATH
  {

  namespace
  {
  const double kInfinity = std::numeric_limits<double>::infinity();

  //! Sorted distinct values (closer than a tolerance: the same).
  std::vector<double> distinct(std::vector<double> values)
  {
  std::sort(values.begin(), values.end());
  if (values.empty()) return values;
  double tolerance = 1E-9*std::max(1.0, std::fabs(values.back() - values.front()));
  std::vector<double> result{ values[0] };
  for (double v : values) if (v - result.back() > tolerance) result.push_back(v);
  return result;
  }

  //! Index of a value in sorted distinct values, or -1.
  int indexOf(const std::vector<double> & nodes, double v)
  {
  double tolerance = 1E-9*std::max(1.0, std::fabs(nodes.back() - nodes.front()));
  auto it = std::lower_bound(nodes.begin(), nodes.end(), v - tolerance);
  if (it == nodes.end() || std::fabs(*it - v) > tolerance) return -1;
  return int(it - nodes.begin());
  }
  } // namespace

  // ------------------------------------------------------------------------------------------------ SearchProblem

  SearchProblem::SearchProblem(unsigned int dimension, Goal goal)
  :
  _dimension(dimension),
  _goal(goal),
  _nEvaluations(0),
  _nGradients(0)
  {
  if (dimension < 1) throw MathException("A search problem needs at least one variable", __FUNCTION__);
  }

  double SearchProblem::objective(const std::vector<double> & x) const
  {
  _nEvaluations++;
  double v = value(x);
  if (!std::isfinite(v)) return kInfinity;
  return _goal == kMaximum ? -v : v;
  }

  std::vector<double> SearchProblem::gradient(const std::vector<double> &) const
  {
  throw MathException("This problem has no gradient", __FUNCTION__);
  }

  std::vector<double> SearchProblem::objectiveGradient(const std::vector<double> & x, const std::vector<double> & steps) const
  {
  if (hasGradient())
    {
    _nGradients++;
    std::vector<double> g = gradient(x);
    if (_goal == kMaximum) for (auto & v : g) v = -v;
    return g;
    }
  std::vector<double> g(_dimension, 0.0), y(x);
  double center = kInfinity;
  for (unsigned int k=0; k<_dimension; k++)
    {
    double h = steps[k];
    y[k] = x[k] + h; double up   = objective(y);
    y[k] = x[k] - h; double down = objective(y);
    y[k] = x[k];
    if (std::isfinite(up) && std::isfinite(down)) g[k] = (up - down)/(2*h);
    else
      {
      // one-sided where the other side cannot be evaluated
      if (!std::isfinite(center)) center = objective(x);
      if (std::isfinite(up))        g[k] = (up - center)/h;
      else if (std::isfinite(down)) g[k] = (center - down)/h;
      }
    }
  return g;
  }

  std::vector<double> SearchProblem::candidate(unsigned int) const
  {
  throw MathException("This problem has no candidates", __FUNCTION__);
  }

  double SearchProblem::candidateObjective(unsigned int index) const
  {
  _nEvaluations++;
  double v = candidateValue(index);
  if (!std::isfinite(v)) return kInfinity;
  return _goal == kMaximum ? -v : v;
  }

  double SearchProblem::candidateValue(unsigned int) const
  {
  throw MathException("This problem has no candidates", __FUNCTION__);
  }

  std::vector<unsigned int> SearchProblem::neighbors(unsigned int) const
  {
  return std::vector<unsigned int>();
  }

  int SearchProblem::nearestCandidate(const std::vector<double> &) const
  {
  return -1;
  }

  bool SearchProblem::refine(unsigned int, std::vector<double> &, double &) const
  {
  return false;
  }

  // ------------------------------------------------------------------------------------------------ FunctionProblem

  FunctionProblem::FunctionProblem(const Function & function, Goal goal)
  :
  SearchProblem(function.rank(), goal),
  _function(function)
  { }

  double FunctionProblem::value(const std::vector<double> & x) const
  {
  try { return _function.evaluateAt(x); }
  catch (...) { return std::numeric_limits<double>::quiet_NaN(); }
  }

  bool FunctionProblem::hasGradient() const
  {
  return dynamic_cast<const ParameterizedFunction*>(&_function) != nullptr;
  }

  std::vector<double> FunctionProblem::gradient(const std::vector<double> & x) const
  {
  const ParameterizedFunction * f = dynamic_cast<const ParameterizedFunction*>(&_function);
  if (!f) return SearchProblem::gradient(x);
  return f->gradientAt(x).values();
  }

  // ------------------------------------------------------------------------------------------------ DataProblem

  DataProblem::DataProblem(const DataPoints & data, Goal goal, bool useEmptyBins)
  :
  SearchProblem(data.rank(), goal),
  _data(data),
  _points(),
  _values(),
  _scale(data.rank(), 1.0),
  _gridSize(),
  _gridNodes(),
  _gridCandidate(),
  _gridIndex(),
  _nNeighbors(0)
  {
  // neighbors of scattered points: as many as on a grid (3^n - 1), at least 2n + 2
  unsigned int moore = 1;
  for (unsigned int k=0; k<data.rank(); k++) moore *= 3;
  _nNeighbors = std::max(moore - 1, 2*data.rank() + 2);
  const unsigned int rank = data.rank();
  if (const Histogram * histogram = dynamic_cast<const Histogram*>(&data))
    {
    // the bins: a regular grid, axis 0 fastest
    unsigned int total = 1;
    for (unsigned int k=0; k<rank; k++)
      {
      _gridSize.push_back(histogram->nBins(k));
      std::vector<double> nodes;
      for (unsigned int i=0; i<histogram->nBins(k); i++) nodes.push_back(histogram->binCenter(i, k));
      _gridNodes.push_back(nodes);
      total *= histogram->nBins(k);
      }
    _gridCandidate.assign(total, -1);
    for (unsigned int index=0; index<total; index++)
      {
      double v = histogram->valueAt(index), e = histogram->statErrorAt(index);
      if (!std::isfinite(v) || (!useEmptyBins && v == 0 && e == 0)) continue;
      std::vector<unsigned int> node(rank);
      unsigned int rest = index;
      for (unsigned int k=0; k<rank; k++) { node[k] = rest % _gridSize[k]; rest /= _gridSize[k]; }
      _gridCandidate[index] = int(_points.size());
      _gridIndex.push_back(node);
      _points.push_back(histogram->coordinatesAt(index));
      _values.push_back(v);
      }
    }
  else
    {
    std::vector<std::vector<double>> all;
    std::vector<double> values;
    for (unsigned int n=0; n<data.size(); n++)
      {
      std::vector<double> c = data.coordinatesAt(n);
      bool finite = c.size() == rank;
      for (double v : c) finite = finite && std::isfinite(v);
      if (!finite) continue;
      all.push_back(c);
      values.push_back(data.valueAt(n));
      }
    // a regular grid: every combination of the distinct coordinates once
    std::vector<std::vector<double>> nodes(rank);
    std::size_t combinations = all.empty() ? 0 : 1;
    for (unsigned int k=0; k<rank && !all.empty(); k++)
      {
      std::vector<double> column;
      for (const auto & c : all) column.push_back(c[k]);
      nodes[k] = distinct(column);
      combinations *= nodes[k].size();
      }
    bool grid = !all.empty() && combinations == all.size() && rank > 0;
    std::vector<int> owner(grid ? combinations : 0, -1);
    std::vector<std::vector<unsigned int>> indices;
    for (std::size_t n=0; n<all.size() && grid; n++)
      {
      std::vector<unsigned int> node(rank);
      std::size_t flat = 0, stride = 1;
      for (unsigned int k=0; k<rank; k++)
        {
        int i = indexOf(nodes[k], all[n][k]);
        if (i < 0) { grid = false; break; }
        node[k] = (unsigned int)(i);
        flat += stride*i;
        stride *= nodes[k].size();
        }
      if (!grid) break;
      if (owner[flat] >= 0) { grid = false; break; }
      owner[flat] = int(n);
      indices.push_back(node);
      }
    if (grid)
      for (unsigned int k=0; k<rank; k++) { _gridNodes.push_back(nodes[k]); _gridSize.push_back((unsigned int)(nodes[k].size())); }
    std::vector<int> candidateOf(all.size(), -1);
    for (std::size_t n=0; n<all.size(); n++)
      {
      if (!std::isfinite(values[n])) continue;
      candidateOf[n] = int(_points.size());
      _points.push_back(all[n]);
      _values.push_back(values[n]);
      if (grid) _gridIndex.push_back(indices[n]);
      }
    if (grid)
      {
      _gridCandidate.assign(owner.size(), -1);
      for (std::size_t flat=0; flat<owner.size(); flat++) if (owner[flat] >= 0) _gridCandidate[flat] = candidateOf[owner[flat]];
      }
    }
  for (unsigned int k=0; k<rank; k++)
    {
    double low = kInfinity, high = -kInfinity;
    for (const auto & p : _points) { low = std::min(low, p[k]); high = std::max(high, p[k]); }
    _scale[k] = (high > low) ? high - low : 1.0;
    }
  }

  double DataProblem::value(const std::vector<double> & x) const
  {
  int index = nearestCandidate(x);
  return index < 0 ? std::numeric_limits<double>::quiet_NaN() : _values[index];
  }

  std::vector<unsigned int> DataProblem::neighbors(unsigned int index) const
  {
  std::vector<unsigned int> result;
  const unsigned int rank = _dimension;
  if (isGrid())
    {
    // the adjacent nodes along the axes and the diagonals that hold a candidate
    const std::vector<unsigned int> & node = _gridIndex[index];
    unsigned int nOffsets = 1;
    for (unsigned int k=0; k<rank; k++) nOffsets *= 3;
    for (unsigned int code=0; code<nOffsets; code++)
      {
      unsigned int rest = code;
      bool inside = true, center = true;
      std::size_t flat = 0, stride = 1;
      for (unsigned int k=0; k<rank; k++)
        {
        int offset = int(rest % 3) - 1;
        rest /= 3;
        int i = int(node[k]) + offset;
        if (offset != 0) center = false;
        if (i < 0 || i >= int(_gridSize[k])) { inside = false; break; }
        flat += stride*i;
        stride *= _gridSize[k];
        }
      if (!inside || center) continue;
      if (_gridCandidate[flat] >= 0) result.push_back((unsigned int)(_gridCandidate[flat]));
      }
    return result;
    }
  // scattered points: the nearest ones (distances scaled by the extent of the points)
  std::vector<std::pair<double,unsigned int>> distances;
  for (unsigned int n=0; n<_points.size(); n++)
    {
    if (n == index) continue;
    double d = 0;
    for (unsigned int k=0; k<rank; k++) { double t = (_points[n][k] - _points[index][k])/_scale[k]; d += t*t; }
    distances.push_back(std::make_pair(d, n));
    }
  unsigned int n = std::min<unsigned int>(_nNeighbors, (unsigned int)(distances.size()));
  std::partial_sort(distances.begin(), distances.begin() + n, distances.end());
  for (unsigned int k=0; k<n; k++) result.push_back(distances[k].second);
  return result;
  }

  int DataProblem::nearestCandidate(const std::vector<double> & x) const
  {
  int best = -1;
  double bestDistance = kInfinity;
  for (unsigned int n=0; n<_points.size(); n++)
    {
    double d = 0;
    for (unsigned int k=0; k<_dimension && k<x.size(); k++) { double t = (_points[n][k] - x[k])/_scale[k]; d += t*t; }
    if (d < bestDistance) { bestDistance = d; best = int(n); }
    }
  return best;
  }

  bool DataProblem::refine(unsigned int index, std::vector<double> & point, double & value) const
  {
  if (!isGrid()) return false;
  const std::vector<unsigned int> & node = _gridIndex[index];
  point = _points[index];
  value = _values[index];
  bool refined = false;
  for (unsigned int k=0; k<_dimension; k++)
    {
    // the nodes before and after along axis k, with the other indices unchanged
    int before = -1, after = -1;
    std::size_t flat = 0, stride = 1, axisStride = 1;
    for (unsigned int j=0; j<_dimension; j++)
      {
      flat += stride*node[j];
      if (j == k) axisStride = stride;
      stride *= _gridSize[j];
      }
    if (node[k] > 0) before = _gridCandidate[flat - axisStride];
    if (node[k] + 1 < _gridSize[k]) after = _gridCandidate[flat + axisStride];
    if (before < 0 || after < 0) continue;
    double fm = _values[before], f0 = _values[index], fp = _values[after];
    double curvature = fm - 2*f0 + fp;
    if (curvature == 0) continue;
    double t = 0.5*(fm - fp)/curvature;                // vertex, in units of the spacing
    if (std::fabs(t) > 1) continue;                     // not an extremum around this node
    double spacing = t > 0 ? _gridNodes[k][node[k]+1] - _gridNodes[k][node[k]] : _gridNodes[k][node[k]] - _gridNodes[k][node[k]-1];
    point[k] = _gridNodes[k][node[k]] + t*spacing;
    value += 0.25*(fp - fm)*t;                          // f(t) - f0 at the vertex: (fp - fm) t/4
    refined = true;
    }
  return refined;
  }

  SearchDomain DataProblem::naturalDomain() const
  {
  SearchDomain domain(_dimension);
  for (unsigned int k=0; k<_dimension; k++)
    {
    double low = kInfinity, high = -kInfinity;
    for (const auto & p : _points) { low = std::min(low, p[k]); high = std::max(high, p[k]); }
    if (high >= low) domain.setBounds(k, low, high);
    }
  return domain;
  }

  } // namespace MATH
} // namespace CAP
