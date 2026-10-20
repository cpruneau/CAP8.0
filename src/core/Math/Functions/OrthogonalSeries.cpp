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
//! @file OrthogonalSeries.cpp
//! @brief Implementation of OrthogonalSeries.
#include <algorithm>
#include <cmath>
#include <string>
#include "OrthogonalSeries.hpp"
#include "Matrix.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  namespace
  {
  std::vector<Parameter> parametersOf(OrthogonalSeries::Family family, unsigned int degree)
  {
  std::vector<Parameter> parameters;
  for (unsigned int k=0; k<=degree; k++) parameters.push_back(Parameter(String(("c" + std::to_string(k)).c_str()), k == 0 ? 1.0 : 0.0));
  if (family == OrthogonalSeries::kLegendre || family == OrthogonalSeries::kChebyshev)
    {
    parameters.push_back(Parameter("low", -1.0, Parameter::Fixed));
    parameters.push_back(Parameter("high", 1.0, Parameter::Fixed));
    }
  else
    {
    parameters.push_back(Parameter("x0", 0.0, Parameter::Fixed));
    parameters.push_back(Parameter("scale", 1.0, Parameter::Fixed));
    if (family == OrthogonalSeries::kLaguerre) parameters.push_back(Parameter("alpha", 0.0, Parameter::Fixed));
    }
  return parameters;
  }
  } // namespace

  OrthogonalSeries::OrthogonalSeries(Family family, unsigned int degree)
  :
  ParameterizedFunction(parametersOf(family, degree), 1),
  _family(family),
  _nCoefficients(degree + 1)
  { }

  void OrthogonalSeries::setDegree(unsigned int degree)
  {
  unsigned int n = degree + 1;
  while (_nCoefficients < n)
    {
    _parameters.insert(_parameters.begin() + _nCoefficients, Parameter(String(("c" + std::to_string(_nCoefficients)).c_str()), 0.0));
    _nCoefficients++;
    }
  while (_nCoefficients > n)
    {
    _parameters.erase(_parameters.begin() + (_nCoefficients - 1));
    _nCoefficients--;
    }
  }

  double OrthogonalSeries::variable(double x) const
  {
  double a = parameterValue(_nCoefficients), b = parameterValue(_nCoefficients + 1);
  if (_family == kLegendre || _family == kChebyshev) return b != a ? (2.0*x - a - b)/(b - a) : x;
  return b != 0 ? (x - a)/b : x - a;
  }

  void OrthogonalSeries::polynomials(Family family, unsigned int degree, double t, double alpha, std::vector<double> & p)
  {
  p.assign(degree + 1, 1.0);
  if (degree == 0) return;
  switch (family)
    {
    case kLegendre:
      p[1] = t;
      for (unsigned int k=1; k<degree; k++) p[k+1] = ((2.0*k + 1.0)*t*p[k] - k*p[k-1])/(k + 1.0);
      break;
    case kChebyshev:
      p[1] = t;
      for (unsigned int k=1; k<degree; k++) p[k+1] = 2.0*t*p[k] - p[k-1];
      break;
    case kHermite:
      p[1] = 2.0*t;
      for (unsigned int k=1; k<degree; k++) p[k+1] = 2.0*t*p[k] - 2.0*k*p[k-1];
      break;
    case kLaguerre:
      p[1] = 1.0 + alpha - t;
      for (unsigned int k=1; k<degree; k++) p[k+1] = ((2.0*k + 1.0 + alpha - t)*p[k] - (k + alpha)*p[k-1])/(k + 1.0);
      break;
    }
  }

  double OrthogonalSeries::evaluateAt(double x) const
  {
  std::vector<double> p;
  polynomials(_family, _nCoefficients - 1, variable(x), _family == kLaguerre ? parameterValue(_nCoefficients + 2) : 0.0, p);
  double sum = 0;
  for (unsigned int k=0; k<_nCoefficients; k++) sum += parameterValue(k)*p[k];
  return sum;
  }

  void OrthogonalSeries::adaptToRange(double low, double high)
  {
  if (!(high > low)) return;
  unsigned int a = _nCoefficients, b = _nCoefficients + 1;
  auto set = [this](unsigned int k, double v) { if (_parameters[k].type() == Parameter::Fixed) _parameters[k].setValue(v); };
  switch (_family)
    {
    case kLegendre: case kChebyshev: set(a, low); set(b, high); break;
    case kHermite:  set(a, 0.5*(low + high)); set(b, 0.25*(high - low)); break;      // t in [-2, 2]
    case kLaguerre: set(a, low); set(b, 0.25*(high - low)); break;                   // t in [0, 4]
    }
  }

  bool OrthogonalSeries::estimate(const std::vector<double> & x, const std::vector<double> & y, const std::vector<double> & w)
  {
  std::vector<unsigned int> free;
  for (unsigned int k=0; k<_nCoefficients; k++) if (_parameters[k].type() != Parameter::Fixed) free.push_back(k);
  if (free.empty() || x.size() < free.size()) return false;
  unsigned int m = (unsigned int)free.size();
  Matrix a(m, m, 0.0);
  std::vector<double> b(m, 0.0), p;
  double alpha = _family == kLaguerre ? parameterValue(_nCoefficients + 2) : 0.0;
  for (std::size_t i=0; i<x.size(); i++)
    {
    polynomials(_family, _nCoefficients - 1, variable(x[i]), alpha, p);
    double r = y[i];
    for (unsigned int k=0; k<_nCoefficients; k++) if (_parameters[k].type() == Parameter::Fixed) r -= parameterValue(k)*p[k];
    for (unsigned int u=0; u<m; u++)
      {
      b[u] += w[i]*r*p[free[u]];
      for (unsigned int v=0; v<m; v++) a(u, v) += w[i]*p[free[u]]*p[free[v]];
      }
    }
  std::vector<double> c;
  try { c = a.solve(b); } catch (...) { return false; }
  for (unsigned int u=0; u<m; u++)
    {
    if (!std::isfinite(c[u])) return false;
    }
  for (unsigned int u=0; u<m; u++)
    {
    Parameter & q = _parameters[free[u]];
    double v = c[u];
    if (q.type() == Parameter::Low || q.type() == Parameter::LowHigh) v = std::max(v, q.minimum());
    if (q.type() == Parameter::High || q.type() == Parameter::LowHigh) v = std::min(v, q.maximum());
    q.setValue(v);
    }
  return true;
  }

  LaguerreSeries::LaguerreSeries(unsigned int degree, double alpha)
  :
  OrthogonalSeries(kLaguerre, degree)
  {
  if (!(alpha > -1.0)) throw MathException("The alpha of Laguerre polynomials must be > -1", __FUNCTION__);
  _parameters[_nCoefficients + 2].setValue(alpha);
  }

  } // namespace MATH
} // namespace CAP
