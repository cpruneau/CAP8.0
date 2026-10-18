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
//! @file Likelihood.cpp
//! @brief Implementation of ChiSquareLikelihood, PoissonLikelihood and UnbinnedLikelihood.
#include <algorithm>
#include <cmath>
#include <limits>
#include "Likelihood.hpp"
#include "DataPoints.hpp"
#include "MathExceptions.hpp"
#include "ParameterizedFunction.hpp"

namespace CAP
{
  namespace MATH
  {

  namespace
  {
  const double kInfinity = std::numeric_limits<double>::infinity();

  //! Value of the function at the coordinates of a point; NaN if it cannot be computed.
  double modelAt(const ParameterizedFunction & f, const DataPoints & data, unsigned int index)
  {
  try { return data.rank() == 1 ? f.evaluateAt(data.point(index).x()) : f.evaluateAt(data.coordinatesAt(index)); }
  catch (...) { return std::numeric_limits<double>::quiet_NaN(); }
  }

  //! Count of a point.
  double countAt(const DataPoints & data, unsigned int index)
  {
  return data.rank() == 1 ? double(data.point(index).y()) : data.valueAt(index);
  }

  //! Baker-Cousins term 2 [f - n + n ln(n/f)]; +infinity if f <= 0 with n > 0.
  double deviance(double n, double f)
  {
  if (!std::isfinite(f)) return kInfinity;
  if (n <= 0) return f >= 0 ? 2*f : kInfinity;
  if (f <= 0) return kInfinity;
  double d = 2*(f - n + n*std::log(n/f));
  return d < 0 ? 0 : d;                                 // rounding near f = n
  }

  //! Extent of the coordinates of some points.
  bool extent(const DataPoints & data, const std::vector<unsigned int> & used, std::vector<double> & low, std::vector<double> & high)
  {
  low.clear(); high.clear();
  for (unsigned int i : used)
    {
    std::vector<double> c = data.rank() == 1 ? std::vector<double>{ double(data.point(i).x()) } : data.coordinatesAt(i);
    if (low.empty()) { low = c; high = c; continue; }
    for (std::size_t k=0; k<c.size(); k++) { low[k] = std::min(low[k], c[k]); high[k] = std::max(high[k], c[k]); }
    }
  return !low.empty();
  }

  } // namespace

  bool ChiSquareLikelihood::range(std::vector<double> & low, std::vector<double> & high) const { return extent(_data, _used, low, high); }
  bool PoissonLikelihood::range(std::vector<double> & low, std::vector<double> & high) const { return extent(_data, _used, low, high); }

  // ------------------------------------------------------------------------------------------------ chi square

  ChiSquareLikelihood::ChiSquareLikelihood(ParameterizedFunction & function, const DataPoints & data)
  :
  Likelihood(function),
  _chiSquare(function, data),
  _data(data),
  _used()
  { }

  void ChiSquareLikelihood::prepare()
  {
  _used.clear();
  for (unsigned int i=0; i<_data.size(); i++)
    {
    try { if (std::isfinite(_chiSquare.pullAt(i))) _used.push_back(i); }
    catch (...) { }
    }
  if (_used.empty()) throw MathException("No data point has a nonzero uncertainty", __FUNCTION__);
  }

  bool ChiSquareLikelihood::residuals(std::vector<double> & r) const
  {
  r.resize(_used.size());
  for (unsigned int i=0; i<_used.size(); i++)
    {
    try { r[i] = _chiSquare.pullAt(_used[i]); }
    catch (...) { return false; }
    if (!std::isfinite(r[i])) return false;
    }
  return true;
  }

  double ChiSquareLikelihood::statistic() const
  {
  std::vector<double> r;
  if (!residuals(r)) return kInfinity;
  double sum = 0;
  for (double v : r) sum += v*v;
  return sum;
  }

  // ------------------------------------------------------------------------------------------------ Poisson

  PoissonLikelihood::PoissonLikelihood(ParameterizedFunction & function, const DataPoints & data)
  :
  Likelihood(function),
  _data(data),
  _used()
  {
  if (function.rank() != data.rank()) throw MathException("The function and the data have different ranks", __FUNCTION__);
  }

  void PoissonLikelihood::prepare()
  {
  _used.clear();
  for (unsigned int i=0; i<_data.size(); i++)
    {
    double n = countAt(_data, i);
    if (std::isfinite(n) && n >= 0) _used.push_back(i);
    }
  if (_used.empty()) throw MathException("No point has a count to fit", __FUNCTION__);
  }

  double PoissonLikelihood::statistic() const
  {
  double sum = 0;
  for (unsigned int i : _used) sum += deviance(countAt(_data, i), modelAt(_function, _data, i));
  return sum;
  }

  bool PoissonLikelihood::residuals(std::vector<double> & r) const
  {
  r.resize(_used.size());
  for (unsigned int k=0; k<_used.size(); k++)
    {
    double n = countAt(_data, _used[k]), f = modelAt(_function, _data, _used[k]);
    double d = deviance(n, f);
    if (!std::isfinite(d)) return false;
    r[k] = (n >= f ? 1.0 : -1.0)*std::sqrt(d);
    }
  return true;
  }

  // ------------------------------------------------------------------------------------------------ unbinned

  UnbinnedLikelihood::UnbinnedLikelihood(ParameterizedFunction & function, const std::vector<double> & events, double low, double high, bool extended)
  :
  Likelihood(function),
  _events(),
  _low{ low },
  _high{ high },
  _extended(extended),
  _used()
  {
  if (function.rank() != 1) throw MathException("Events of rank 1 need a function of rank 1", __FUNCTION__);
  for (double x : events) _events.push_back({ x });
  }

  UnbinnedLikelihood::UnbinnedLikelihood(ParameterizedFunction & function, const std::vector<std::vector<double>> & events,
                                         const std::vector<double> & low, const std::vector<double> & high, bool extended)
  :
  Likelihood(function),
  _events(events),
  _low(low),
  _high(high),
  _extended(extended),
  _used()
  {
  if (low.size() != function.rank() || high.size() != function.rank())
    throw MathException("The range must have the rank of the function", __FUNCTION__);
  }

  void UnbinnedLikelihood::prepare()
  {
  _used.clear();
  for (unsigned int i=0; i<_events.size(); i++)
    {
    const std::vector<double> & x = _events[i];
    bool inside = x.size() == _low.size();
    for (unsigned int k=0; k<x.size() && inside; k++) inside = x[k] >= _low[k] && x[k] <= _high[k];
    if (inside) _used.push_back(i);
    }
  if (_used.empty()) throw MathException("No event inside the range", __FUNCTION__);
  }

  double UnbinnedLikelihood::statistic() const
  {
  double integral;
  try { integral = _function.integralInRange(_low, _high); }
  catch (...) { return kInfinity; }
  if (!(integral > 0) || !std::isfinite(integral)) return kInfinity;
  double sum = 0;
  for (unsigned int i : _used)
    {
    double f;
    try { f = _low.size() == 1 ? _function.evaluateAt(_events[i][0]) : _function.evaluateAt(_events[i]); }
    catch (...) { return kInfinity; }
    if (!(f > 0)) return kInfinity;
    sum += std::log(f);
    }
  return _extended ? 2*integral - 2*sum : -2*(sum - _used.size()*std::log(integral));
  }

  } // namespace MATH
} // namespace CAP
