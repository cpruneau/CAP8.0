/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file GaussLegendreIntegrator.cpp
//! @brief Implementation of GaussLegendreIntegrator.
#include <cmath>
#include <map>
#include <mutex>
#include "GaussLegendreIntegrator.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  GaussLegendreIntegrator::GaussLegendreIntegrator(unsigned int nPoints, double relativeTolerance, unsigned int maxLevel)
  :
  Integrator(relativeTolerance, 0.0, maxLevel),
  _nPoints(nPoints > 0 ? nPoints : 1),
  _panels(0)
  { }

  namespace
  {
  // P_m(z) and P_m'(z), by the upward recurrence
  void legendre(unsigned int m, double z, double & p, double & dp)
  {
  double p0 = 1.0, p1 = z;
  if (m == 0) { p = 1.0; dp = 0.0; return; }
  for (unsigned int k=2; k<=m; k++)
    {
    double p2 = ((2.0*k - 1.0)*z*p1 - (k - 1.0)*p0)/k;
    p0 = p1;
    p1 = p2;
    }
  p = p1;
  dp = (std::fabs(z) == 1.0) ? 0.5*m*(m + 1.0)*(m % 2 == 0 && z < 0 ? -1.0 : 1.0) : m*(z*p1 - p0)/(z*z - 1.0);
  }

  struct Rule { std::vector<double> nodes, weights; };

  const Rule & cachedRule(unsigned int m)
  {
  static std::map<unsigned int, Rule> rules;
  static std::mutex mutex;
  std::lock_guard<std::mutex> lock(mutex);
  auto found = rules.find(m);
  if (found != rules.end()) return found->second;
  Rule & r = rules[m];
  r.nodes.assign(m, 0.0);
  r.weights.assign(m, 0.0);
  for (unsigned int i=0; i<(m+1)/2; i++)   // the roots are symmetric: the positive ones, by Newton's method
    {
    double z = std::cos(M_PI*(i + 0.75)/(m + 0.5));
    double p, dp = 1.0;
    for (int iteration=0; iteration<100; iteration++)
      {
      legendre(m, z, p, dp);
      double dz = p/dp;
      z -= dz;
      if (std::fabs(dz) <= 1.0E-16) break;
      }
    legendre(m, z, p, dp);
    double w = 2.0/((1.0 - z*z)*dp*dp);
    r.nodes[m-1-i] =  z;   r.weights[m-1-i] = w;     // increasing order
    r.nodes[i]     = -z;   r.weights[i]     = w;
    }
  if (m % 2 == 1) r.nodes[m/2] = 0.0;
  return r;
  }
  } // namespace

  void GaussLegendreIntegrator::rule(unsigned int m, std::vector<double> & nodes, std::vector<double> & weights)
  {
  if (m == 0) throw MathException("Number of points must be at least 1", __FUNCTION__);
  const Rule & r = cachedRule(m);
  nodes = r.nodes;
  weights = r.weights;
  }

  double GaussLegendreIntegrator::quadrature(const Integrand1D & g, double low, double high, unsigned int m, unsigned int n)
  {
  if (m == 0) throw MathException("Number of points must be at least 1", __FUNCTION__);
  if (n == 0) n = 1;
  const Rule & r = cachedRule(m);
  double width = (high - low)/n, sum = 0.0;
  for (unsigned int k=0; k<n; k++)
    {
    double mid = low + (k + 0.5)*width, half = 0.5*width, panel = 0.0;
    for (unsigned int i=0; i<m; i++) panel += r.weights[i]*g(mid + half*r.nodes[i]);
    sum += half*panel;
    }
  return sum;
  }

  double GaussLegendreIntegrator::productRule(const IntegrandND & g, const std::vector<double> & low,
                                              const std::vector<double> & high, unsigned int m, unsigned int n)
  {
  if (m == 0) throw MathException("Number of points must be at least 1", __FUNCTION__);
  if (n == 0) n = 1;
  const Rule & r = cachedRule(m);
  const std::size_t rank = low.size();
  const unsigned int perAxis = m*n;       // points per variable
  std::vector<double> half(rank), x(rank);
  double jacobian = 1.0;
  for (std::size_t k=0; k<rank; k++) { half[k] = 0.5*(high[k] - low[k])/n; jacobian *= half[k]; }
  std::vector<unsigned int> index(rank, 0);
  double sum = 0.0;
  while (true)
    {
    double weight = 1.0;
    for (std::size_t k=0; k<rank; k++)
      {
      unsigned int panel = index[k]/m, i = index[k] % m;
      x[k] = low[k] + (2.0*panel + 1.0)*half[k] + half[k]*r.nodes[i];
      weight *= r.weights[i];
      }
    sum += weight*g(x);
    std::size_t k = 0;
    while (k < rank && ++index[k] >= perAxis) { index[k] = 0; k++; }
    if (k == rank) break;
    }
  return jacobian*sum;
  }

  IntegralResult GaussLegendreIntegrator::integrate(const Integrand1D & g, double low, double high) const
  {
  checkBounds(low, high, __FUNCTION__);
  IntegralResult result;
  if (low == high) return result;
  if (_panels > 0)
    {
    result.value = quadrature(g, low, high, _nPoints, _panels);
    result.nEvaluations = long(_nPoints)*_panels;
    return result;
    }
  unsigned int n = 1;
  double previous = quadrature(g, low, high, _nPoints, n);
  result.nEvaluations = _nPoints;
  result.converged = false;
  for (unsigned int level=1; level<=_maxLevel; level++)
    {
    n *= 2;
    double value = quadrature(g, low, high, _nPoints, n);
    result.nEvaluations += long(_nPoints)*n;
    result.error = std::fabs(value - previous);
    previous = value;
    if (accepted(result.error, value)) { result.converged = true; break; }
    }
  result.value = previous;
  return result;
  }

  IntegralResult GaussLegendreIntegrator::integrate(const IntegrandND & g, const std::vector<double> & low,
                                                    const std::vector<double> & high) const
  {
  checkBounds(low, high, __FUNCTION__);
  IntegralResult result;
  if (low.size() == 1)
    return integrate(Integrand1D([&g](double t) { return g(std::vector<double>{t}); }), low[0], high[0]);
  double points = std::pow(double(_nPoints), double(low.size()));
  if (_panels > 0)
    {
    result.value = productRule(g, low, high, _nPoints, _panels);
    result.nEvaluations = long(points*std::pow(double(_panels), double(low.size())));
    return result;
    }
  unsigned int n = 1;
  double previous = productRule(g, low, high, _nPoints, n);
  result.nEvaluations = long(points);
  result.converged = false;
  for (unsigned int level=1; level<=_maxLevel; level++)
    {
    double cost = points*std::pow(2.0*n, double(low.size()));
    if (cost > 1.0E7) break;
    n *= 2;
    double value = productRule(g, low, high, _nPoints, n);
    result.nEvaluations += long(cost);
    result.error = std::fabs(value - previous);
    previous = value;
    if (accepted(result.error, value)) { result.converged = true; break; }
    }
  result.value = previous;
  return result;
  }

  } // namespace MATH
} // namespace CAP
