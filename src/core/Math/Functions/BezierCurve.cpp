/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file BezierCurve.cpp
//! @brief Implementation of BezierCurve.
#include <algorithm>
#include <cmath>
#include <numeric>
#include <string>
#include "BezierCurve.hpp"
#include "MathExceptions.hpp"
#include "GaussLegendreIntegrator.hpp"

namespace CAP
{
  namespace MATH
  {

  namespace
  {
  //! Cubic Bernstein polynomial of control values p at t in [0, 1].
  double bernstein(const double p[4], double t)
  {
  double u = 1.0 - t;
  return u*u*u*p[0] + 3.0*u*u*t*p[1] + 3.0*u*t*t*p[2] + t*t*t*p[3];
  }

  //! Derivative of the Bernstein polynomial with respect to t.
  double bernsteinDerivative(const double p[4], double t)
  {
  double u = 1.0 - t;
  return 3.0*(u*u*(p[1] - p[0]) + 2.0*u*t*(p[2] - p[1]) + t*t*(p[3] - p[2]));
  }

  std::string indexed(const char * name, unsigned int k) { return std::string(name) + std::to_string(k); }
  } // namespace

  BezierCurve::BezierCurve()
  :
  ParameterizedFunction(1)
  {
  build({0.0, 1.0}, {0.0, 1.0}, Parameter::defaultMinimum, Parameter::defaultMaximum);
  }

  BezierCurve::BezierCurve(unsigned int nKnots)
  :
  ParameterizedFunction(1)
  {
  if (nKnots < 2) throw MathException("A Bezier curve has at least 2 knots", __FUNCTION__);
  std::vector<double> x(nKnots), y(nKnots, 0.0);
  for (unsigned int k=0; k<nKnots; k++) x[k] = double(k)/double(nKnots - 1);
  build(x, y, Parameter::defaultMinimum, Parameter::defaultMaximum);
  }

  BezierCurve::BezierCurve(const std::vector<double> & x, const std::vector<double> & y, double lower, double upper)
  :
  ParameterizedFunction(1)
  {
  if (lower > upper) throw MathException("Bounds of a Bezier curve: lower > upper", __FUNCTION__);
  build(x, y, lower, upper);
  }

  void BezierCurve::build(std::vector<double> x, std::vector<double> y, double lower, double upper)
  {
  if (x.size() < 2 || x.size() != y.size())
    throw MathException("A Bezier curve needs as many values as abscissas, at least 2", __FUNCTION__);
  std::vector<std::size_t> order(x.size());
  std::iota(order.begin(), order.end(), std::size_t(0));
  std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) { return x[a] < x[b]; });
  std::vector<Parameter> parameters;
  for (std::size_t j=0; j<order.size(); j++)
    {
    double xk = x[order[j]], yk = std::min(std::max(y[order[j]], lower), upper);
    if (j > 0 && !(xk > x[order[j-1]])) throw MathException("Two knots of a Bezier curve at the same x", __FUNCTION__);
    parameters.push_back(Parameter(String(indexed("x", unsigned(j)).c_str()), xk));
    bool low = lower > Parameter::defaultMinimum, high = upper < Parameter::defaultMaximum;
    Parameter::ParameterType type = low && high ? Parameter::LowHigh : low ? Parameter::Low : high ? Parameter::High : Parameter::Free;
    parameters.push_back(Parameter(String(indexed("y", unsigned(j)).c_str()), yk, type, lower, upper));
    }
  _parameters = parameters;
  _cachedParameters.clear();
  }

  double BezierCurve::lower() const
  {
  const Parameter & p = parameter(1);
  return p.isLowConstrained() ? p.minimum() : Parameter::defaultMinimum;
  }

  double BezierCurve::upper() const
  {
  const Parameter & p = parameter(1);
  return p.isHighConstrained() ? p.maximum() : Parameter::defaultMaximum;
  }

  void BezierCurve::setBounds(double lower, double upper)
  {
  if (lower > upper) throw MathException("Bounds of a Bezier curve: lower > upper", __FUNCTION__);
  std::vector<double> x, y;
  for (unsigned int k=0; k<nKnots(); k++) { x.push_back(knotX(k)); y.push_back(knotY(k)); }
  build(x, y, lower, upper);
  }

  void BezierCurve::setKnots(const std::vector<double> & x, const std::vector<double> & y)
  {
  build(x, y, lower(), upper());
  }

  int BezierCurve::insertKnot(double x)
  {
  std::vector<double> xs, ys;
  for (unsigned int k=0; k<nKnots(); k++)
    {
    if (knotX(k) == x) return -1;
    xs.push_back(knotX(k));
    ys.push_back(knotY(k));
    }
  double y = evaluateAt(x);
  xs.push_back(x);
  ys.push_back(y);
  setKnots(xs, ys);
  for (unsigned int k=0; k<nKnots(); k++) if (knotX(k) == x) return int(k);
  return -1;
  }

  bool BezierCurve::removeKnot(unsigned int k)
  {
  if (nKnots() <= 2 || k >= nKnots()) return false;
  std::vector<double> xs, ys;
  for (unsigned int j=0; j<nKnots(); j++)
    if (j != k) { xs.push_back(knotX(j)); ys.push_back(knotY(j)); }
  setKnots(xs, ys);
  return true;
  }

  void BezierCurve::moveKnot(unsigned int k, double x, double y)
  {
  if (k >= nKnots()) return;
  // a gap kept to the neighbors: a small fraction of the span of the knots
  double span = knotX(nKnots() - 1) - knotX(0);
  double gap = 1.0E-4*(span > 0.0 ? span : 1.0);
  if (k > 0)            x = std::max(x, knotX(k-1) + gap);
  if (k+1 < nKnots())   x = std::min(x, knotX(k+1) - gap);
  if (k > 0 && k+1 < nKnots() && knotX(k+1) - knotX(k-1) <= 2.0*gap) x = 0.5*(knotX(k-1) + knotX(k+1));
  y = std::min(std::max(y, lower()), upper());
  setParameterValue(2*k, x);
  setParameterValue(2*k+1, y);
  }

  unsigned int BezierCurve::nearestKnot(double x) const
  {
  unsigned int best = 0;
  for (unsigned int k=1; k<nKnots(); k++)
    if (std::fabs(knotX(k) - x) < std::fabs(knotX(best) - x)) best = k;
  return best;
  }

  void BezierCurve::update() const
  {
  // unchanged parameters (the usual case, e.g. in the loop over the particles): nothing to do, nothing allocated
  bool same = !_x.empty() && _cachedParameters.size() == _parameters.size();
  for (std::size_t j=0; same && j<_parameters.size(); j++) same = _parameters[j].value() == _cachedParameters[j];
  if (same) return;
  std::vector<double> values = parameterValues();
  _cachedParameters = values;
  unsigned int n = nKnots();
  // the knots in order of x (parameters set by a fit may not be), equal abscissas merged
  std::vector<std::pair<double,double>> knots;
  for (unsigned int k=0; k<n; k++) knots.push_back({values[2*k], values[2*k+1]});
  std::stable_sort(knots.begin(), knots.end(), [](const std::pair<double,double> & a, const std::pair<double,double> & b)
                   { return a.first < b.first; });
  _x.clear();
  _y.clear();
  for (const auto & knot : knots)
    {
    if (!_x.empty() && !(knot.first > _x.back())) { _y.back() = knot.second; continue; }
    _x.push_back(knot.first);
    _y.push_back(knot.second);
    }
  n = (unsigned int)_x.size();
  _slopes.assign(n, 0.0);
  if (n < 2) return;
  std::vector<double> h(n-1), d(n-1);
  for (unsigned int k=0; k+1<n; k++) { h[k] = _x[k+1] - _x[k]; d[k] = (_y[k+1] - _y[k])/h[k]; }
  if (n == 2) { _slopes[0] = _slopes[1] = d[0]; return; }
  // interior knots: weighted harmonic mean of the slopes of the two segments (Fritsch-Butland), 0 at an extremum
  for (unsigned int k=1; k+1<n; k++)
    {
    if (d[k-1]*d[k] <= 0.0) { _slopes[k] = 0.0; continue; }
    double w1 = 2.0*h[k] + h[k-1], w2 = h[k] + 2.0*h[k-1];
    _slopes[k] = (w1 + w2)/(w1/d[k-1] + w2/d[k]);
    }
  // end knots: three-point estimate, kept monotone and no larger than 3 times the slope of the end segment
  auto endSlope = [](double h0, double h1, double d0, double d1)
    {
    double m = ((2.0*h0 + h1)*d0 - h0*d1)/(h0 + h1);
    if (m*d0 <= 0.0) return 0.0;
    if (std::fabs(m) > 3.0*std::fabs(d0)) m = 3.0*d0;
    return m;
    };
  _slopes[0]   = endSlope(h[0], h[1], d[0], d[1]);
  _slopes[n-1] = endSlope(h[n-2], h[n-3], d[n-2], d[n-3]);
  }

  unsigned int BezierCurve::segment(double x) const
  {
  std::size_t k = std::size_t(std::upper_bound(_x.begin(), _x.end(), x) - _x.begin());
  if (k == 0) return 0;
  if (k >= _x.size()) return (unsigned int)(_x.size() - 2);
  return (unsigned int)(k - 1);
  }

  std::vector<double> BezierCurve::controlPoints(unsigned int k) const
  {
  update();
  if (k + 1 >= _x.size()) return std::vector<double>();
  double h = _x[k+1] - _x[k];
  return { _y[k], _y[k] + h*_slopes[k]/3.0, _y[k+1] - h*_slopes[k+1]/3.0, _y[k+1] };
  }

  double BezierCurve::evaluateAt(double x) const
  {
  update();
  if (_x.size() < 2) return _y.empty() ? 0.0 : _y.front();
  if (!(x > _x.front())) return _y.front();
  if (!(x < _x.back()))  return _y.back();
  unsigned int k = segment(x);
  double h = _x[k+1] - _x[k];
  const double p[4] = { _y[k], _y[k] + h*_slopes[k]/3.0, _y[k+1] - h*_slopes[k+1]/3.0, _y[k+1] };
  return bernstein(p, (x - _x[k])/h);
  }

  double BezierCurve::derivativeAt(double x) const
  {
  update();
  if (_x.size() < 2 || !(x > _x.front()) || !(x < _x.back())) return 0.0;
  unsigned int k = segment(x);
  std::vector<double> p = controlPoints(k);
  double h = _x[k+1] - _x[k];
  return bernsteinDerivative(p.data(), (x - _x[k])/h)/h;
  }

  double BezierCurve::integralInRange(double low, double high) const
  {
  update();
  if (high < low) return -integralInRange(high, low);
  if (_x.size() < 2) return (high - low)*(_y.empty() ? 0.0 : _y.front());
  double sum = 0.0;
  // the end values held outside the knots
  if (low < _x.front()) { sum += (std::min(high, _x.front()) - low)*_y.front(); low = _x.front(); }
  if (high > _x.back()) { sum += (high - std::max(low, _x.back()))*_y.back(); high = _x.back(); }
  if (!(high > low)) return sum;
  // each piece of a segment: 3-point Gauss-Legendre (exact for a cubic)
  for (unsigned int k=segment(low); k+1<_x.size() && _x[k] < high; k++)
    {
    double a = std::max(low, _x[k]), b = std::min(high, _x[k+1]);
    if (!(b > a)) continue;
    std::vector<double> p = controlPoints(k);
    double h = _x[k+1] - _x[k], x0 = _x[k];
    sum += GaussLegendreIntegrator::quadrature([&](double x) { return bernstein(p.data(), (x - x0)/h); }, a, b, 3);
    }
  return sum;
  }

  } // namespace MATH
} // namespace CAP
