/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FiniteDifference.cpp
//! @brief Implementation of the finite differences.
#include <algorithm>
#include <cmath>
#include <limits>
#include "FiniteDifference.hpp"
#include "InterpolationTools.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  std::vector<double> finiteDifferenceWeights(unsigned int order, const std::vector<double> & offsets)
  {
  const std::size_t n = offsets.size();
  if (n < std::size_t(order) + 1) throw MathException("Too few points for the order of the derivative", __FUNCTION__);
  // Fornberg: c[j][m] weight of point j for the derivative m, built point by point
  std::vector<std::vector<double>> c(n, std::vector<double>(order + 1, 0.0));
  c[0][0] = 1.0;
  double c1 = 1.0, c4 = offsets[0];
  for (std::size_t i=1; i<n; i++)
    {
    unsigned int mn = std::min<unsigned int>(unsigned(i), order);
    double c2 = 1.0, c5 = c4;
    c4 = offsets[i];
    for (std::size_t j=0; j<i; j++)
      {
      double c3 = offsets[i] - offsets[j];
      if (c3 == 0.0) throw MathException("Two points are equal", __FUNCTION__);
      c2 *= c3;
      if (j == i - 1)
        {
        for (unsigned int k=mn; k>=1; k--) c[i][k] = c1*(k*c[i-1][k-1] - c5*c[i-1][k])/c2;
        c[i][0] = -c1*c5*c[i-1][0]/c2;
        }
      for (unsigned int k=mn; k>=1; k--) c[j][k] = (c4*c[j][k] - k*c[j][k-1])/c3;
      c[j][0] = c4*c[j][0]/c3;
      }
    c1 = c2;
    }
  std::vector<double> weights(n);
  double largest = 0.0;
  for (std::size_t j=0; j<n; j++) { weights[j] = c[j][order]; largest = std::max(largest, std::fabs(weights[j])); }
  for (double & w : weights) if (std::fabs(w) < 1.0E-13*largest) w = 0.0;   // e.g. the center of a central difference
  return weights;
  }

  std::vector<double> finiteDifferenceOffsets(unsigned int degree, Stencil stencil)
  {
  std::vector<double> offsets(degree + 1);
  for (unsigned int j=0; j<=degree; j++)
    {
    switch (stencil)
      {
      case Stencil::Central:  offsets[j] = double(j) - 0.5*degree; break;
      case Stencil::Forward:  offsets[j] = double(j); break;
      case Stencil::Backward: offsets[j] = double(j) - double(degree); break;
      }
    }
  return offsets;
  }

  unsigned int finiteDifferenceAccuracy(unsigned int order, unsigned int degree, Stencil stencil)
  {
  if (degree < order) throw MathException("The degree of the polynomial is less than the order of the derivative", __FUNCTION__);
  unsigned int p = degree + 1 - order;
  if (stencil == Stencil::Central && p % 2 == 1) p++;     // symmetric stencils gain one order
  return p;
  }

  double finiteDifferenceStep(unsigned int order, unsigned int degree, Stencil stencil, double x)
  {
  unsigned int p = finiteDifferenceAccuracy(order, degree, stencil);
  double optimal = std::pow(std::numeric_limits<double>::epsilon(), 1.0/double(p + order));
  return std::pow(10.0, std::round(std::log10(optimal)))*std::max(1.0, std::fabs(x));
  }

  std::vector<double> derivativesOfSamples(const std::vector<double> & x, const std::vector<double> & y,
                                           unsigned int order, unsigned int degree)
  {
  if (x.size() != y.size()) throw MathException("The sizes of x and y differ", __FUNCTION__);
  if (order == 0) return y;
  if (x.size() < std::size_t(order) + 1) throw MathException("Too few samples for the order of the derivative", __FUNCTION__);
  degree = std::max(degree, order);
  degree = unsigned(std::min<std::size_t>(degree, x.size() - 1));
  std::vector<double> result(x.size());
  std::vector<double> offsets(degree + 1);
  for (std::size_t i=0; i<x.size(); i++)
    {
    std::size_t first = localWindow(x, x[i], degree);
    for (unsigned int j=0; j<=degree; j++) offsets[j] = x[first + j] - x[i];
    std::vector<double> w = finiteDifferenceWeights(order, offsets);
    double sum = 0.0;
    for (unsigned int j=0; j<=degree; j++) if (w[j] != 0.0) sum += w[j]*y[first + j];
    result[i] = sum;
    }
  return result;
  }

  std::vector<double> derivativeOnGrid(const std::vector<double> & values, const std::vector<unsigned int> & nPoints,
                                       unsigned int axis, double spacing, unsigned int order, unsigned int degree)
  {
  if (axis >= nPoints.size()) throw MathException("No such axis", __FUNCTION__);
  std::size_t total = 1, stride = 1;
  for (std::size_t k=0; k<nPoints.size(); k++) { total *= nPoints[k]; if (k < axis) stride *= nPoints[k]; }
  if (total != values.size()) throw MathException("The number of values differs from the number of nodes", __FUNCTION__);
  const unsigned int n = nPoints[axis];
  if (order == 0) return values;
  if (n < order + 1) throw MathException("Too few nodes along the axis for the order of the derivative", __FUNCTION__);
  degree = std::max(degree, order);
  degree = std::min(degree, n - 1);
  // the weights of each node of the axis (the same for all the lines of the grid)
  std::vector<double> nodes(n);
  for (unsigned int i=0; i<n; i++) nodes[i] = double(i);
  std::vector<std::vector<double>> weights(n);
  std::vector<std::size_t> firsts(n);
  std::vector<double> offsets(degree + 1);
  double scale = std::pow(spacing, double(order));
  for (unsigned int i=0; i<n; i++)
    {
    firsts[i] = localWindow(nodes, nodes[i], degree);
    for (unsigned int j=0; j<=degree; j++) offsets[j] = double(firsts[i] + j) - double(i);
    weights[i] = finiteDifferenceWeights(order, offsets);
    for (double & w : weights[i]) w /= scale;
    }
  std::vector<double> result(values.size());
  for (std::size_t g=0; g<values.size(); g++)
    {
    unsigned int i = unsigned((g/stride) % n);          // node index along the axis
    const double * f = &values[g - i*stride];           // f[m*stride] is node m along the axis
    double sum = 0.0;
    for (unsigned int j=0; j<=degree; j++)
      if (weights[i][j] != 0.0) sum += weights[i][j]*f[(firsts[i] + j)*stride];
    result[g] = sum;
    }
  return result;
  }

  } // namespace MATH
} // namespace CAP
