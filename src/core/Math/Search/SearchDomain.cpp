/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file SearchDomain.cpp
//! @brief Implementation of SearchDomain and ExclusionRegion.
#include <algorithm>
#include <cmath>
#include "SearchDomain.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  ExclusionRegion & ExclusionRegion::add(unsigned int dimension, double low, double high)
  {
  if (high < low) std::swap(low, high);
  for (auto & interval : _intervals)
    if (interval.first == dimension) { interval.second = std::make_pair(low, high); return *this; }
  _intervals.push_back(std::make_pair(dimension, std::make_pair(low, high)));
  return *this;
  }

  ExclusionRegion ExclusionRegion::box(const std::vector<double> & low, const std::vector<double> & high)
  {
  ExclusionRegion region;
  for (unsigned int k=0; k<low.size() && k<high.size(); k++) region.add(k, low[k], high[k]);
  return region;
  }

  bool ExclusionRegion::contains(const std::vector<double> & x) const
  {
  for (const auto & interval : _intervals)
    {
    if (interval.first >= x.size()) return false;
    double v = x[interval.first];
    if (v < interval.second.first || v > interval.second.second) return false;
    }
  return true;
  }

  SearchDomain::SearchDomain(unsigned int dimension)
  :
  _lower(dimension, -std::numeric_limits<double>::infinity()),
  _upper(dimension,  std::numeric_limits<double>::infinity()),
  _exclusions(),
  _start()
  { }

  SearchDomain::SearchDomain(const std::vector<double> & lower, const std::vector<double> & upper)
  :
  _lower(lower),
  _upper(upper),
  _exclusions(),
  _start()
  {
  if (lower.size() != upper.size()) throw MathException("The lower and upper bounds have different sizes", __FUNCTION__);
  for (unsigned int k=0; k<dimension(); k++) if (_upper[k] < _lower[k]) std::swap(_lower[k], _upper[k]);
  }

  void SearchDomain::setBounds(unsigned int k, double low, double high)
  {
  if (k >= dimension()) throw MathException("Dimension out of range", __FUNCTION__);
  if (high < low) std::swap(low, high);
  _lower[k] = low;
  _upper[k] = high;
  }

  bool SearchDomain::isBounded(unsigned int k) const
  {
  return std::isfinite(_lower[k]) && std::isfinite(_upper[k]);
  }

  bool SearchDomain::isBounded() const
  {
  for (unsigned int k=0; k<dimension(); k++) if (!isBounded(k)) return false;
  return true;
  }

  double SearchDomain::width(unsigned int k) const
  {
  double w = _upper[k] - _lower[k];
  return (isBounded(k) && w > 0) ? w : 1.0;
  }

  bool SearchDomain::inBounds(const std::vector<double> & x) const
  {
  if (x.size() != dimension()) return false;
  for (unsigned int k=0; k<dimension(); k++)
    if (!(x[k] >= _lower[k] && x[k] <= _upper[k])) return false;   // also NaN
  return true;
  }

  bool SearchDomain::excluded(const std::vector<double> & x) const
  {
  for (const auto & region : _exclusions) if (region.contains(x)) return true;
  return false;
  }

  std::vector<double> SearchDomain::clamped(const std::vector<double> & x) const
  {
  std::vector<double> y(x);
  for (unsigned int k=0; k<dimension() && k<y.size(); k++) y[k] = std::min(std::max(y[k], _lower[k]), _upper[k]);
  return y;
  }

  } // namespace MATH
} // namespace CAP
