/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Differentiation.cpp
//! @brief Implementation of Differentiation.
#include <algorithm>
#include <cmath>
#include <memory>
#include "Differentiation.hpp"
#include "Function.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  Differentiation::Differentiation(Method method, unsigned int degree, Stencil stencil)
  :
  _method(method),
  _degree(degree > 0 ? degree : 1),
  _stencil(stencil),
  _step(0.0),
  _richardson(false)
  { }

  String Differentiation::methodName(Method method)
  {
  switch (method)
    {
    case Automatic:  return "Automatic";
    case Analytical: return "Analytical";
    case Linear:     return "Linear";
    case Quadratic:  return "Quadratic";
    case Cubic:      return "Cubic";
    case Polynomial: return "Polynomial";
    }
  return "Unknown";
  }

  unsigned int Differentiation::degree() const
  {
  switch (_method)
    {
    case Linear:    return 1;
    case Quadratic: return 2;
    case Cubic:     return 3;
    default:        return _degree;
    }
  }

  double Differentiation::stepAt(unsigned int order, double x) const
  {
  unsigned int d = std::max(degree(), order);
  if (_step > 0.0) return _step*std::max(1.0, std::fabs(x));
  return finiteDifferenceStep(order, d, _stencil, x);
  }

  double Differentiation::numerical(const std::function<double(double)> & g, double x, unsigned int order) const
  {
  if (order == 0) return g(0.0);
  unsigned int d = std::max(degree(), order);
  std::vector<double> offsets = finiteDifferenceOffsets(d, _stencil);
  std::vector<double> weights = finiteDifferenceWeights(order, offsets);
  double h = stepAt(order, x);
  auto estimate = [&](double step)
    {
    double sum = 0.0;
    for (std::size_t j=0; j<offsets.size(); j++) if (weights[j] != 0.0) sum += weights[j]*g(offsets[j]*step);
    return sum/std::pow(step, double(order));
    };
  if (!_richardson) return estimate(h);
  double factor = std::pow(2.0, double(finiteDifferenceAccuracy(order, d, _stencil)));
  double coarse = estimate(h), fine = estimate(0.5*h);
  return (factor*fine - coarse)/(factor - 1.0);
  }

  // ---------------------------------------------------------------- callables

  double Differentiation::derivative(const std::function<double(double)> & g, double x, unsigned int order) const
  {
  return numerical([&](double t) { return g(x + t); }, x, order);
  }

  double Differentiation::partialDerivative(const std::function<double(const std::vector<double> &)> & g,
                                            const std::vector<double> & x, unsigned int axis, unsigned int order) const
  {
  if (axis >= x.size()) throw MathException("No such axis", __FUNCTION__);
  std::vector<double> y(x);
  return numerical([&](double t) { y[axis] = x[axis] + t; double v = g(y); y[axis] = x[axis]; return v; }, x[axis], order);
  }

  double Differentiation::mixedDerivative(const std::function<double(const std::vector<double> &)> & g,
                                          const std::vector<double> & x, const std::vector<unsigned int> & axes) const
  {
  if (axes.empty()) return g(x);
  if (axes.size() == 1) return partialDerivative(g, x, axes[0]);
  // along the first axis, of the mixed derivative along the others
  std::vector<unsigned int> rest(axes.begin() + 1, axes.end());
  std::function<double(const std::vector<double> &)> inner =
    [&](const std::vector<double> & y) { return mixedDerivative(g, y, rest); };
  return partialDerivative(inner, x, axes[0]);
  }

  VectorND Differentiation::gradient(const std::function<double(const std::vector<double> &)> & g,
                                     const std::vector<double> & x) const
  {
  VectorND result(x.size());
  for (unsigned int i=0; i<x.size(); i++) result[i] = partialDerivative(g, x, i);
  return result;
  }

  // ---------------------------------------------------------------- functions

  namespace
  {
  // Differentiates f analytically along the axes, as far as its derivativeFunction() allows: the function reached
  // (owned by `owned` unless it is f) and the number of axes consumed.
  const Function * analytical(const Function & f, const std::vector<unsigned int> & axes,
                              std::vector<std::unique_ptr<Function>> & owned, std::size_t & consumed)
  {
  const Function * g = &f;
  consumed = 0;
  while (consumed < axes.size())
    {
    Function * d = g->derivativeFunction(axes[consumed]);
    if (!d) break;
    owned.emplace_back(d);
    g = d;
    consumed++;
    }
  return g;
  }

  std::vector<unsigned int> repeated(unsigned int axis, unsigned int order) { return std::vector<unsigned int>(order, axis); }
  } // namespace

  double Differentiation::mixedDerivative(const Function & f, const std::vector<double> & x,
                                          const std::vector<unsigned int> & axes) const
  {
  if (x.size() != f.rank()) throw MathException("The number of coordinates differs from the rank of the function", __FUNCTION__);
  for (unsigned int a : axes) if (a >= f.rank()) throw MathException("No such axis", __FUNCTION__);
  const Function * g = &f;
  std::vector<std::unique_ptr<Function>> owned;
  std::size_t consumed = 0;
  if (_method == Automatic || _method == Analytical) g = analytical(f, axes, owned, consumed);
  std::vector<unsigned int> rest(axes.begin() + consumed, axes.end());
  if (rest.empty()) return g->evaluateAt(x);
  if ((_method == Automatic || _method == Analytical) && rest.size() == 1 && g->hasAnalyticalDerivative())
    return g->rank() == 1 ? g->derivativeAt(x[0]) : g->gradientAt(x)[rest[0]];
  if (_method == Analytical)
    throw MathException("The function has no analytical derivative of this order", __FUNCTION__);
  // numerical, for the rest
  bool sameAxis = std::all_of(rest.begin(), rest.end(), [&](unsigned int a) { return a == rest[0]; });
  if (g->rank() == 1)
    return numerical([&](double t) { return g->evaluateAt(x[0] + t); }, x[0], unsigned(rest.size()));
  std::function<double(const std::vector<double> &)> h = [g](const std::vector<double> & y) { return g->evaluateAt(y); };
  if (sameAxis) return partialDerivative(h, x, rest[0], unsigned(rest.size()));
  return mixedDerivative(h, x, rest);
  }

  double Differentiation::derivative(const Function & f, double x, unsigned int order) const
  {
  if (f.rank() != 1) throw MathException("The function is not of rank 1", __FUNCTION__);
  if (order == 0) return f.evaluateAt(x);
  if (order == 1 && (_method == Automatic || _method == Analytical) && f.hasAnalyticalDerivative()) return f.derivativeAt(x);
  return mixedDerivative(f, std::vector<double>{x}, repeated(0, order));
  }

  double Differentiation::partialDerivative(const Function & f, const std::vector<double> & x, unsigned int axis,
                                            unsigned int order) const
  {
  if (order == 0) return f.evaluateAt(x);
  return mixedDerivative(f, x, repeated(axis, order));
  }

  VectorND Differentiation::gradient(const Function & f, const std::vector<double> & x) const
  {
  if (x.size() != f.rank()) throw MathException("The number of coordinates differs from the rank of the function", __FUNCTION__);
  if ((_method == Automatic || _method == Analytical) && f.hasAnalyticalDerivative()) return f.gradientAt(x);
  VectorND result(x.size());
  for (unsigned int i=0; i<x.size(); i++) result[i] = partialDerivative(f, x, i);
  return result;
  }

  std::vector<double> Differentiation::derivatives(const std::vector<double> & x, const std::vector<double> & y,
                                                   unsigned int order) const
  {
  return derivativesOfSamples(x, y, order, std::max(degree(), order));
  }

  } // namespace MATH
} // namespace CAP
