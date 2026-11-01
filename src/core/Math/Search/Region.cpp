/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Region.cpp
//! @brief Implementation of Region.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include "Region.hpp"

namespace CAP
{
  namespace MATH
  {

  Region & Region::add(unsigned int dimension, double low, double high)
  {
  if (high < low) std::swap(low, high);
  for (auto & interval : _intervals)
    if (interval.first == dimension) { interval.second = std::make_pair(low, high); return *this; }
  _intervals.push_back(std::make_pair(dimension, std::make_pair(low, high)));
  return *this;
  }

  Region Region::box(const std::vector<double> & low, const std::vector<double> & high)
  {
  Region region;
  for (unsigned int k=0; k<low.size() && k<high.size(); k++) region.add(k, low[k], high[k]);
  return region;
  }

  bool Region::restricts(unsigned int dimension) const
  {
  for (const auto & interval : _intervals) if (interval.first == dimension) return true;
  return false;
  }

  bool Region::intervalOf(unsigned int dimension, double & low, double & high) const
  {
  for (const auto & interval : _intervals)
    if (interval.first == dimension) { low = interval.second.first; high = interval.second.second; return true; }
  return false;
  }

  Region & Region::remove(unsigned int dimension)
  {
  _intervals.erase(std::remove_if(_intervals.begin(), _intervals.end(),
                                  [dimension](const std::pair<unsigned int,std::pair<double,double>> & i) { return i.first == dimension; }),
                   _intervals.end());
  return *this;
  }

  bool Region::contains(const std::vector<double> & x) const
  {
  for (const auto & interval : _intervals)
    {
    if (interval.first >= x.size()) return false;
    double v = x[interval.first];
    if (v < interval.second.first || v > interval.second.second) return false;
    }
  return true;
  }

  bool Region::isValid(unsigned int rank) const
  {
  for (const auto & interval : _intervals)
    if (interval.first >= rank || !std::isfinite(interval.second.first) || !std::isfinite(interval.second.second)) return false;
  return true;
  }

  std::string Region::dimensionName(unsigned int dimension)
  {
  static const char * names[] = { "x", "y", "z" };
  return dimension < 3 ? std::string(names[dimension]) : "x" + std::to_string(dimension);
  }

  std::string Region::toString(const std::vector<std::string> & names) const
  {
  if (_intervals.empty()) return "everywhere";
  std::vector<std::pair<unsigned int,std::pair<double,double>>> sorted = _intervals;
  std::sort(sorted.begin(), sorted.end());
  std::string text;
  char buffer[64];
  for (const auto & interval : sorted)
    {
    if (!text.empty()) text += " and ";
    text += interval.first < names.size() && !names[interval.first].empty() ? names[interval.first] : dimensionName(interval.first);
    std::snprintf(buffer, sizeof(buffer), " in [%.6g, %.6g]", interval.second.first, interval.second.second);
    text += buffer;
    }
  return text;
  }

  bool Region::operator==(const Region & other) const
  {
  if (_intervals.size() != other._intervals.size()) return false;
  std::vector<std::pair<unsigned int,std::pair<double,double>>> a = _intervals, b = other._intervals;
  std::sort(a.begin(), a.end());
  std::sort(b.begin(), b.end());
  return a == b;
  }

  } // namespace MATH
} // namespace CAP
