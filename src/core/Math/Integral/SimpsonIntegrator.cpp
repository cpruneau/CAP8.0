/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file SimpsonIntegrator.cpp
//! @brief Implementation of SimpsonIntegrator.
#include <algorithm>
#include <cmath>
#include "SimpsonIntegrator.hpp"

namespace CAP
{
  namespace MATH
  {

  SimpsonIntegrator::SimpsonIntegrator(double relativeTolerance, unsigned int maxLevel, unsigned int initialPanels)
  :
  Integrator(relativeTolerance, 0.0, maxLevel),
  _panels(0),
  _initialPanels(initialPanels > 0 ? initialPanels : 1)
  { }

  namespace
  {
  struct Adaptive
  {
  const Integrand1D & g;
  long nEvaluations;
  double error;
  bool converged;

  // Recursive step on [a,b], with g evaluated at a, m=(a+b)/2, b; whole is Simpson's rule on [a,b].
  double step(double a, double b, double fa, double fm, double fb, double whole, double tolerance, int depth)
  {
  double m   = 0.5*(a+b);
  double lm  = 0.5*(a+m),  rm  = 0.5*(m+b);
  double flm = g(lm), frm = g(rm);
  nEvaluations += 2;
  double left  = (m-a)*(fa + 4.0*flm + fm)/6.0;
  double right = (b-m)*(fm + 4.0*frm + fb)/6.0;
  double delta = left + right - whole;
  if (depth<=0 || std::fabs(delta) <= 15.0*tolerance)
    {
    if (std::fabs(delta) > 15.0*tolerance) converged = false;
    error += std::fabs(delta)/15.0;
    return left + right + delta/15.0;   // Richardson extrapolation
    }
  return step(a,m,fa,flm,fm,left, 0.5*tolerance,depth-1)
       + step(m,b,fm,frm,fb,right,0.5*tolerance,depth-1);
  }
  };

  // Simpson's rule on n panels of [low, high]
  double fixedRule(const Integrand1D & g, double low, double high, unsigned int n, long & nEvaluations)
  {
  double width = (high - low)/n;
  double sum = g(low) + g(high);
  for (unsigned int k=1; k<2*n; k++) sum += (k % 2 == 1 ? 4.0 : 2.0)*g(low + 0.5*k*width);
  nEvaluations += 2*n + 1;
  return width*sum/6.0;
  }
  } // namespace

  IntegralResult SimpsonIntegrator::integrate(const Integrand1D & g, double low, double high) const
  {
  checkBounds(low, high, __FUNCTION__);
  IntegralResult result;
  if (low == high) return result;
  if (low > high)
    {
    result = integrate(g, high, low);
    result.value = -result.value;
    return result;
    }
  if (_panels > 0)
    {
    result.value = fixedRule(g, low, high, _panels, result.nEvaluations);
    if (_panels % 2 == 0)
      {
      double coarse = fixedRule(g, low, high, _panels/2, result.nEvaluations);
      result.error = std::fabs(result.value - coarse)/15.0;
      result.converged = accepted(result.error, result.value);
      }
    return result;
    }
  const unsigned int nPanels = _initialPanels;
  double width = (high - low)/nPanels;
  double coarseAbs = 0.0;
  std::vector<double> fs(2*nPanels + 1);
  for (unsigned int k=0; k<=2*nPanels; k++) fs[k] = g(low + 0.5*k*width);
  for (unsigned int k=0; k<nPanels; k++) coarseAbs += width*(std::fabs(fs[2*k]) + 4.0*std::fabs(fs[2*k+1]) + std::fabs(fs[2*k+2]))/6.0;
  double tolerance = std::max(_absoluteTolerance, _relativeTolerance*std::max(coarseAbs, 1.0E-300));
  Adaptive adaptive{g, long(2*nPanels + 1), 0.0, true};
  double sum = 0.0;
  for (unsigned int k=0; k<nPanels; k++)
    {
    double a = low + k*width, b = a + width;
    double whole = width*(fs[2*k] + 4.0*fs[2*k+1] + fs[2*k+2])/6.0;
    sum += adaptive.step(a, b, fs[2*k], fs[2*k+1], fs[2*k+2], whole, tolerance/nPanels, int(_maxLevel));
    }
  result.value = sum;
  result.error = adaptive.error;
  result.nEvaluations = adaptive.nEvaluations;
  result.converged = adaptive.converged;
  return result;
  }

  } // namespace MATH
} // namespace CAP
