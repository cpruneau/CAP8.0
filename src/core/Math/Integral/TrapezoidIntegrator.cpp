/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TrapezoidIntegrator.cpp
//! @brief Implementation of TrapezoidIntegrator.
#include <cmath>
#include "TrapezoidIntegrator.hpp"

namespace CAP
{
  namespace MATH
  {

  TrapezoidIntegrator::TrapezoidIntegrator(double relativeTolerance, unsigned int maxLevel)
  :
  Integrator(relativeTolerance, 0.0, maxLevel),
  _panels(0),
  _maxEvaluations(10000000)
  { }

  IntegralResult TrapezoidIntegrator::integrate(const Integrand1D & g, double low, double high) const
  {
  checkBounds(low, high, __FUNCTION__);
  IntegralResult result;
  if (low == high) return result;
  double width = high - low;
  if (_panels > 0)
    {
    double h = width/_panels;
    double sum = 0.5*(g(low) + g(high));
    for (unsigned int k=1; k<_panels; k++) sum += g(low + k*h);
    result.value = h*sum;
    result.nEvaluations = _panels + 1;
    return result;
    }
  // T(1), then each halving adds the midpoints of the panels
  double t = 0.5*width*(g(low) + g(high));
  result.nEvaluations = 2;
  result.converged = false;
  unsigned long n = 1;
  for (unsigned int level=1; level<=_maxLevel; level++)
    {
    double h = width/n, sum = 0.0;
    for (unsigned long k=0; k<n; k++) sum += g(low + (k + 0.5)*h);
    result.nEvaluations += long(n);
    double refined = 0.5*(t + h*sum);
    double change = refined - t;
    t = refined;
    n *= 2;
    result.error = std::fabs(change)/3.0;
    if (level >= 3 && accepted(result.error, t)) { result.converged = true; break; }
    }
  result.value = t;
  return result;
  }

  double TrapezoidIntegrator::productRule(const IntegrandND & g, const std::vector<double> & low,
                                          const std::vector<double> & high, unsigned int n, long & nEvaluations)
  {
  const std::size_t rank = low.size();
  std::vector<double> h(rank), x(rank);
  double cell = 1.0;
  for (std::size_t k=0; k<rank; k++) { h[k] = (high[k] - low[k])/n; cell *= h[k]; }
  std::vector<unsigned int> index(rank, 0);
  double sum = 0.0;
  while (true)
    {
    double weight = 1.0;
    for (std::size_t k=0; k<rank; k++)
      {
      x[k] = low[k] + index[k]*h[k];
      if (index[k] == 0 || index[k] == n) weight *= 0.5;
      }
    sum += weight*g(x);
    nEvaluations++;
    std::size_t k = 0;
    while (k < rank && ++index[k] > n) { index[k] = 0; k++; }
    if (k == rank) break;
    }
  return cell*sum;
  }

  IntegralResult TrapezoidIntegrator::integrate(const IntegrandND & g, const std::vector<double> & low,
                                                const std::vector<double> & high) const
  {
  checkBounds(low, high, __FUNCTION__);
  IntegralResult result;
  if (low.size() == 1)
    return integrate(Integrand1D([&g](double t) { return g(std::vector<double>{t}); }), low[0], high[0]);
  for (std::size_t k=0; k<low.size(); k++) if (low[k] == high[k]) return result;
  if (_panels > 0)
    {
    result.value = productRule(g, low, high, _panels, result.nEvaluations);
    return result;
    }
  double previous = productRule(g, low, high, 1, result.nEvaluations);
  result.converged = false;
  unsigned int n = 1;
  for (unsigned int level=1; level<=_maxLevel; level++)
    {
    double next = std::pow(2.0*n + 1.0, double(low.size()));
    if (result.nEvaluations + long(next) > _maxEvaluations) break;
    n *= 2;
    double value = productRule(g, low, high, n, result.nEvaluations);
    result.error = std::fabs(value - previous)/3.0;
    previous = value;
    if (level >= 2 && accepted(result.error, value)) { result.converged = true; break; }
    }
  result.value = previous;
  return result;
  }

  } // namespace MATH
} // namespace CAP
