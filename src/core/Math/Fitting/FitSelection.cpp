/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FitSelection.cpp
//! @brief Implementation of FitSelection.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include "FitSelection.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  void FitSelection::setBox(const std::vector<double> & low, const std::vector<double> & high)
  {
  if (low.size() != high.size()) throw MathException("The ends of the range differ in size", __FUNCTION__);
  for (std::size_t k=0; k<low.size(); k++)
    if (!(low[k] <= high[k])) throw MathException("A lower end of the range is above its upper end", __FUNCTION__);
  _low = low;
  _high = high;
  }

  bool FitSelection::contains(const std::vector<double> & x) const
  {
  if (hasBox())
    {
    if (x.size() != _low.size()) return false;
    for (std::size_t k=0; k<x.size(); k++) if (x[k] < _low[k] || x[k] > _high[k]) return false;
    }
  if (!_included.empty())
    {
    bool in = false;
    for (const Region & region : _included) if (region.contains(x)) { in = true; break; }
    if (!in) return false;
    }
  for (const Region & region : _excluded) if (region.contains(x)) return false;
  return true;
  }

  void FitSelection::validate(unsigned int rank) const
  {
  if (hasBox() && _low.size() != rank) throw MathException("The range must have the rank of the function", __FUNCTION__);
  for (std::size_t k=0; k<_low.size(); k++)
    if (!std::isfinite(_low[k]) || !std::isfinite(_high[k])) throw MathException("The range must be finite", __FUNCTION__);
  for (const Region & region : _included)
    if (!region.isValid(rank)) throw MathException("A region included has an infinite interval or one along a variable the function does not have", __FUNCTION__);
  for (const Region & region : _excluded)
    if (!region.isValid(rank)) throw MathException("A region excluded has an infinite interval or one along a variable the function does not have", __FUNCTION__);
  }

  std::vector<std::pair<std::vector<double>,std::vector<double>>> FitSelection::cells(unsigned int maxCells) const
  {
  if (!hasBox()) throw MathException("The cells of a selection need a box", __FUNCTION__);
  const std::size_t rank = _low.size();
  std::vector<std::pair<std::vector<double>,std::vector<double>>> result;
  if (isBox()) { result.push_back({ _low, _high }); return result; }
  // ends of the cells along each variable: those of the box and of the intervals inside it
  std::vector<std::vector<double>> edges(rank);
  double total = 1;
  for (std::size_t k=0; k<rank; k++)
    {
    std::vector<double> & e = edges[k];
    e = { _low[k], _high[k] };
    auto addEnds = [&](const std::vector<Region> & regions)
      {
      for (const Region & region : regions)
        {
        double a, b;
        if (!region.intervalOf((unsigned int)k, a, b)) continue;
        if (a > _low[k] && a < _high[k]) e.push_back(a);
        if (b > _low[k] && b < _high[k]) e.push_back(b);
        }
      };
    addEnds(_included);
    addEnds(_excluded);
    std::sort(e.begin(), e.end());
    e.erase(std::unique(e.begin(), e.end()), e.end());
    total *= double(e.size() > 1 ? e.size() - 1 : 1);
    }
  if (total > double(maxCells)) throw MathException("Too many cells: simplify the regions of the fit", __FUNCTION__);
  // every cell, tested at its center (a cell is wholly in or out of each region)
  std::vector<std::size_t> index(rank, 0);
  std::vector<double> a(rank), b(rank), center(rank);
  while (true)
    {
    bool degenerate = false;
    for (std::size_t k=0; k<rank; k++)
      {
      const std::vector<double> & e = edges[k];
      a[k] = e[index[k]];
      b[k] = e.size() > 1 ? e[index[k] + 1] : e[index[k]];
      center[k] = 0.5*(a[k] + b[k]);
      degenerate = degenerate || (e.size() > 1 && !(b[k] > a[k]));
      }
    if (!degenerate && contains(center)) result.push_back({ a, b });
    std::size_t k = 0;
    for (; k<rank; k++)
      {
      std::size_t n = edges[k].size() > 1 ? edges[k].size() - 1 : 1;
      if (++index[k] < n) break;
      index[k] = 0;
      }
    if (k == rank) break;
    }
  return result;
  }

  std::string FitSelection::toString(const std::vector<std::string> & names) const
  {
  if (everything()) return "all points";
  char buffer[64];
  std::string text;
  for (std::size_t k=0; k<_low.size(); k++)
    {
    std::snprintf(buffer, sizeof(buffer), "[%.6g, %.6g]", _low[k], _high[k]);
    text += (k ? " x " : "") + std::string(buffer);
    }
  auto list = [&names](const std::vector<Region> & regions)
    {
    std::string t;
    for (std::size_t i=0; i<regions.size(); i++) t += (i ? "; " : "") + regions[i].toString(names);
    return t;
    };
  if (!_included.empty()) text += (text.empty() ? "" : "; ") + std::string("only ") + list(_included);
  if (!_excluded.empty()) text += (text.empty() ? "" : "; ") + std::string("excluding ") + list(_excluded);
  return text;
  }

  bool FitSelection::operator==(const FitSelection & other) const
  {
  return _low == other._low && _high == other._high && _included == other._included && _excluded == other._excluded;
  }

  } // namespace MATH
} // namespace CAP
