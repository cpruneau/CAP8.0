/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Integrator.cpp
//! @brief Implementation of Integrator.
#include <algorithm>
#include <cmath>
#include "Integrator.hpp"
#include "Function.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  Integrator::Integrator(double relativeTolerance, double absoluteTolerance, unsigned int maxLevel)
  :
  _relativeTolerance(relativeTolerance),
  _absoluteTolerance(absoluteTolerance),
  _maxLevel(maxLevel)
  { }

  void Integrator::checkBounds(double low, double high, const char * source)
  {
  if (!std::isfinite(low) || !std::isfinite(high)) throw MathException("Numerical integration requires finite bounds", source);
  }

  void Integrator::checkBounds(const std::vector<double> & low, const std::vector<double> & high, const char * source)
  {
  if (low.empty() || low.size() != high.size())
    throw MathException("The lower and upper bounds have different sizes, or none", source);
  for (std::size_t k=0; k<low.size(); k++) checkBounds(low[k], high[k], source);
  }

  bool Integrator::accepted(double change, double value) const
  {
  return std::fabs(change) <= std::max(_absoluteTolerance, _relativeTolerance*std::fabs(value));
  }

  namespace
  {
  // integral over the axes axis ... n-1, the variables of the lower axes already set in x
  IntegralResult iterated(const Integrator & integrator, const IntegrandND & g, std::vector<double> & x,
                          std::size_t axis, const std::vector<double> & low, const std::vector<double> & high)
  {
  bool last = (axis + 1 == x.size());
  IntegralResult result;
  double innerError = 0.0;
  long innerEvaluations = 0;
  bool innerConverged = true;
  auto h = [&](double t)
    {
    x[axis] = t;
    if (last) { innerEvaluations++; return g(x); }
    IntegralResult inner = iterated(integrator, g, x, axis + 1, low, high);
    innerError = std::max(innerError, inner.error);
    innerEvaluations += inner.nEvaluations;
    innerConverged = innerConverged && inner.converged;
    return inner.value;
    };
  IntegralResult outer = integrator.integrate(Integrand1D(h), low[axis], high[axis]);
  result.value = outer.value;
  // the error of the inner integrals, at most innerError each, integrated over this axis
  result.error = outer.error + innerError*std::fabs(high[axis] - low[axis]);
  result.nEvaluations = last ? outer.nEvaluations : innerEvaluations;
  result.converged = outer.converged && innerConverged;
  return result;
  }
  } // namespace

  IntegralResult Integrator::integrate(const IntegrandND & g, const std::vector<double> & low,
                                       const std::vector<double> & high) const
  {
  checkBounds(low, high, __FUNCTION__);
  std::vector<double> x(low.size());
  return iterated(*this, g, x, 0, low, high);
  }

  IntegralResult Integrator::integrate(const Function & f, double low, double high) const
  {
  if (f.rank() != 1) throw MathException("The function is not of rank 1", __FUNCTION__);
  return integrate(Integrand1D([&f](double x) { return f.evaluateAt(x); }), low, high);
  }

  IntegralResult Integrator::integrate(const Function & f, double x1Low, double x1High, double x2Low, double x2High) const
  {
  if (f.rank() != 2) throw MathException("The function is not of rank 2", __FUNCTION__);
  return integrate(f, std::vector<double>{x1Low, x2Low}, std::vector<double>{x1High, x2High});
  }

  IntegralResult Integrator::integrate(const Function & f, const std::vector<double> & low,
                                       const std::vector<double> & high) const
  {
  if (low.size() != f.rank() || high.size() != f.rank())
    throw MathException("The number of bounds differs from the rank of the function", __FUNCTION__);
  if (f.rank() == 1) return integrate(f, low[0], high[0]);
  return integrate(IntegrandND([&f](const std::vector<double> & x) { return f.evaluateAt(x); }), low, high);
  }

  } // namespace MATH
} // namespace CAP
