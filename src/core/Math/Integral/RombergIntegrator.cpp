/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file RombergIntegrator.cpp
//! @brief Implementation of RombergIntegrator.
#include <cmath>
#include "RombergIntegrator.hpp"

namespace CAP
{
  namespace MATH
  {

  RombergIntegrator::RombergIntegrator(double relativeTolerance, unsigned int maxLevel)
  :
  Integrator(relativeTolerance, 0.0, maxLevel)
  { }

  IntegralResult RombergIntegrator::integrate(const Integrand1D & g, double low, double high) const
  {
  checkBounds(low, high, __FUNCTION__);
  IntegralResult result;
  if (low == high) return result;
  double width = high - low;
  std::vector<double> previous(1), current;
  previous[0] = 0.5*width*(g(low) + g(high));
  result.nEvaluations = 2;
  result.converged = false;
  unsigned long n = 1;
  double best = previous[0];
  for (unsigned int k=1; k<=_maxLevel; k++)
    {
    // trapezoidal rule on 2n panels from the one on n panels: the midpoints only
    double h = width/n, sum = 0.0;
    for (unsigned long i=0; i<n; i++) sum += g(low + (i + 0.5)*h);
    result.nEvaluations += long(n);
    n *= 2;
    current.assign(k + 1, 0.0);
    current[0] = 0.5*(previous[0] + h*sum);
    double factor = 1.0;
    for (unsigned int j=1; j<=k; j++)
      {
      factor *= 4.0;
      current[j] = current[j-1] + (current[j-1] - previous[j-1])/(factor - 1.0);
      }
    result.error = std::fabs(current[k] - previous[k-1]);
    best = current[k];
    previous.swap(current);
    if (k >= 3 && accepted(result.error, best)) { result.converged = true; break; }
    }
  result.value = best;
  return result;
  }

  } // namespace MATH
} // namespace CAP
