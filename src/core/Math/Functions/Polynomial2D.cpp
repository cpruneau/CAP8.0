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
//! @file Polynomial2D.cpp
//! @brief Implementation of Polynomial2D.
#include <algorithm>
#include <cmath>
#include <string>
#include "Polynomial2D.hpp"
#include "Matrix.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  namespace
  {
  std::vector<Parameter> coefficients(unsigned int degree)
  {
  if (degree > 9) throw MathException("The degree of a Polynomial2D is at most 9", "Polynomial2D");
  std::vector<Parameter> parameters;
  for (unsigned int d=0; d<=degree; d++)
    for (unsigned int i=d+1; i-->0; )
      parameters.push_back(Parameter(String(("c" + std::to_string(i) + std::to_string(d - i)).c_str()), d == 0 ? 1.0 : 0.0));
  return parameters;
  }

  std::vector<Parameter> parametersOf(unsigned int degree)
  {
  std::vector<Parameter> parameters = coefficients(degree);
  parameters.push_back(Parameter("x0", 0.0, Parameter::Fixed));
  parameters.push_back(Parameter("xScale", 1.0, Parameter::Fixed));
  parameters.push_back(Parameter("y0", 0.0, Parameter::Fixed));
  parameters.push_back(Parameter("yScale", 1.0, Parameter::Fixed));
  return parameters;
  }
  } // namespace

  Polynomial2D::Polynomial2D(unsigned int degree)
  :
  ParameterizedFunction(parametersOf(degree), 2),
  _degree(degree)
  { }

  unsigned int Polynomial2D::index(unsigned int i, unsigned int j)
  {
  unsigned int d = i + j;
  return d*(d + 1)/2 + (d - i);
  }

  void Polynomial2D::setDegree(unsigned int degree)
  {
  std::vector<Parameter> parameters = coefficients(degree);
  unsigned int n = nCoefficients();
  for (unsigned int d=0; d<=std::min(degree, _degree); d++)
    for (unsigned int i=0; i<=d; i++) parameters[index(i, d - i)] = _parameters[index(i, d - i)];
  for (unsigned int k=n; k<_parameters.size(); k++) parameters.push_back(_parameters[k]);   // the mapping
  _parameters = parameters;
  _degree = degree;
  }

  void Polynomial2D::monomials(double x, double y, std::vector<double> & values) const
  {
  unsigned int n = nCoefficients();
  double sx = parameterValue(n + 1), sy = parameterValue(n + 3);
  double t = sx != 0 ? (x - parameterValue(n))/sx : x - parameterValue(n);
  double s = sy != 0 ? (y - parameterValue(n + 2))/sy : y - parameterValue(n + 2);
  std::vector<double> tp(_degree + 1, 1.0), sp(_degree + 1, 1.0);
  for (unsigned int k=1; k<=_degree; k++) { tp[k] = tp[k-1]*t; sp[k] = sp[k-1]*s; }
  values.assign(n, 0.0);
  for (unsigned int d=0; d<=_degree; d++)
    for (unsigned int i=0; i<=d; i++) values[index(i, d - i)] = tp[i]*sp[d - i];
  }

  double Polynomial2D::evaluateAt(double x, double y) const
  {
  std::vector<double> m;
  monomials(x, y, m);
  double sum = 0;
  for (unsigned int k=0; k<m.size(); k++) sum += parameterValue(k)*m[k];
  return sum;
  }

  void Polynomial2D::adaptToRange(const std::vector<double> & low, const std::vector<double> & high)
  {
  if (low.size() != 2 || high.size() != 2) return;
  unsigned int n = nCoefficients();
  auto set = [this](unsigned int k, double v) { if (_parameters[k].type() == Parameter::Fixed) _parameters[k].setValue(v); };
  if (high[0] > low[0]) { set(n, 0.5*(low[0] + high[0])); set(n + 1, 0.5*(high[0] - low[0])); }
  if (high[1] > low[1]) { set(n + 2, 0.5*(low[1] + high[1])); set(n + 3, 0.5*(high[1] - low[1])); }
  }

  bool Polynomial2D::estimate(const std::vector<double> & x, const std::vector<double> & y, const std::vector<double> & z, const std::vector<double> & w)
  {
  unsigned int n = nCoefficients();
  std::vector<unsigned int> free;
  for (unsigned int k=0; k<n; k++) if (_parameters[k].type() != Parameter::Fixed) free.push_back(k);
  if (free.empty() || x.size() < free.size()) return false;
  unsigned int m = (unsigned int)free.size();
  Matrix a(m, m, 0.0);
  std::vector<double> b(m, 0.0), p;
  for (std::size_t i=0; i<x.size(); i++)
    {
    monomials(x[i], y[i], p);
    double r = z[i];
    for (unsigned int k=0; k<n; k++) if (_parameters[k].type() == Parameter::Fixed) r -= parameterValue(k)*p[k];
    for (unsigned int u=0; u<m; u++)
      {
      b[u] += w[i]*r*p[free[u]];
      for (unsigned int v=0; v<m; v++) a(u, v) += w[i]*p[free[u]]*p[free[v]];
      }
    }
  std::vector<double> c;
  try { c = a.solve(b); } catch (...) { return false; }
  for (double v : c) if (!std::isfinite(v)) return false;
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

  } // namespace MATH
} // namespace CAP
