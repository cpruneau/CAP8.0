/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FiniteDifference.hpp
//! @brief Finite differences: the weights of any derivative on any set of points (Fornberg's algorithm), the
//! stencils of the derivatives of interpolating polynomials of a chosen degree, and the derivatives of sampled values.
//!
//! The derivative of order n of the polynomial of degree d through d+1 points x + o_j h is sum_j w_j f(x + o_j h)/h^n;
//! the weights w_j depend on the offsets o_j only. Degree 1 (two points) is the derivative of the linear
//! interpolation, degree 2 that of the parabola through three points, degree 3 the cubic through four, and so on;
//! the error is of order h^p (finiteDifferenceAccuracy()). Differentiation and Derivative use these functions on
//! functions; derivativesOfSamples() and derivativeOnGrid() on values already computed.
#ifndef CAP__MATH__FiniteDifference
#define CAP__MATH__FiniteDifference
#include <cstddef>
#include <vector>

namespace CAP
{
  namespace MATH
  {

  //! @brief Placement of the points of a finite difference around x.
  enum class Stencil
  {
  Central,    //!< symmetric about x (half-integer offsets for an even number of points)
  Forward,    //!< x and the points above it
  Backward    //!< x and the points below it
  };

  //!
  //! @brief Weights of the derivative of order n at 0 of the polynomial through the points at the given offsets,
  //! by Fornberg's algorithm (B. Fornberg, Math. Comp. 51 (1988) 699): f^(n)(0) ~ sum_j w_j f(o_j) (unit spacing).
  //! @param order order n of the derivative (0 is interpolation) @param offsets offsets o_j (distinct; at least n+1)
  //! @return weights, one per offset
  //! @throws MathException if there are fewer than order+1 offsets or two offsets are equal
  //!
  std::vector<double> finiteDifferenceWeights(unsigned int order, const std::vector<double> & offsets);

  //! @brief Offsets of the degree+1 points of a stencil, in units of the step. @param degree degree of the
  //! interpolating polynomial @param stencil placement @return offsets (increasing)
  std::vector<double> finiteDifferenceOffsets(unsigned int degree, Stencil stencil);

  //! @brief Order p of the error (h^p) of the derivative of order n by the polynomial of degree d.
  //! @param order n @param degree d (>= n) @param stencil placement @return p
  //! @throws MathException if the degree is less than the order
  unsigned int finiteDifferenceAccuracy(unsigned int order, unsigned int degree, Stencil stencil);

  //! @brief Step balancing the truncation error (h^p) and the rounding error (eps/h^n) of a finite difference:
  //! 10^round(log10(eps^(1/(p+n)))) max(1, |x|) (1e-3 max(1, |x|) for the first derivative by the five-point central
  //! difference). @param order n @param degree d @param stencil placement @param x point @return step
  double finiteDifferenceStep(unsigned int order, unsigned int degree, Stencil stencil, double x);

  //!
  //! @brief Derivatives of order n of sampled values, at the samples: at each point, the derivative of the polynomial
  //! of the given degree through the degree+1 nearest samples (centered when possible, one-sided at the ends).
  //! @param x increasing abscissas (any spacing) @param y values @param order order n (1 or more) @param degree degree
  //! of the local polynomial (>= order; reduced to the number of samples - 1) @return derivatives, one per sample
  //! @throws MathException if the sizes differ or there are too few samples for the order
  //!
  //! @code
  //!   std::vector<double> x = {0, 0.5, 1, 1.5, 2}, y = {0, 0.25, 1, 2.25, 4};    // x^2
  //!   std::vector<double> d = CAP::MATH::derivativesOfSamples(x, y);              // 0, 1, 2, 3, 4 (exact, degree 2)
  //! @endcode
  //!
  std::vector<double> derivativesOfSamples(const std::vector<double> & x, const std::vector<double> & y,
                                           unsigned int order=1, unsigned int degree=2);

  //!
  //! @brief Derivatives of order n along one axis of values on a regular grid of any rank, at the nodes (local
  //! polynomials of the given degree, as derivativesOfSamples()).
  //! @param values values (axis 0 varying fastest) @param nPoints nodes along each axis @param axis axis
  //! @param spacing distance between the nodes of the axis @param order order n @param degree degree (>= order;
  //! reduced to nPoints[axis] - 1) @return derivatives at the nodes, stored as the values
  //! @throws MathException if the sizes do not match, or there are too few nodes along the axis for the order
  //!
  std::vector<double> derivativeOnGrid(const std::vector<double> & values, const std::vector<unsigned int> & nPoints,
                                       unsigned int axis, double spacing, unsigned int order=1, unsigned int degree=2);

  } // namespace MATH
} // namespace CAP

#endif // CAP__MATH__FiniteDifference
