/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file InterpolationTools.hpp
//! @brief Interpolation of tabulated values: polynomial (Neville), local polynomial of a chosen degree, inverse linear
//! interpolation of a monotonic table, and multilinear interpolation on a regular grid of any rank.
//!
//! These functions are the interpolation tools of CAP: ScalarField, the tabulated random distributions, the sampled
//! integration rules (Integral) and the derivatives of sampled values (Derivative) use them. Cubic splines and other
//! piecewise polynomials are functions (see PiecewisePolynomial and CubicSpline). The older functions of
//! MathBasicInterpolation.hpp are kept.
#ifndef CAP__MATH__InterpolationTools
#define CAP__MATH__InterpolationTools
#include <cstddef>
#include <vector>

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Value at x0 of the polynomial of degree n-1 through the n points (x[i], y[i]), by Neville's algorithm.
  //! @param x abscissas (distinct, any order) @param y values @param n number of points (1 or more)
  //! @param x0 where the polynomial is evaluated
  //! @param error if not null, set to an estimate of the error (the last correction of Neville's table)
  //! @return p(x0)
  //! @throws MathException if n is 0 or two abscissas are equal
  //!
  //! @code
  //!   double x[3] = {0, 1, 2}, y[3] = {1, 2, 5};               // 1 + x^2
  //!   double v = CAP::MATH::interpolatePolynomial(x, y, 3, 1.5);   // 3.25
  //! @endcode
  //!
  double interpolatePolynomial(const double * x, const double * y, std::size_t n, double x0, double * error=nullptr);

  //! @brief Polynomial interpolation through all the points (see the pointer version). @param x abscissas @param y values
  //! @param x0 where the polynomial is evaluated @param error error estimate if not null @return p(x0)
  //! @throws MathException if the sizes differ, are 0, or two abscissas are equal
  double interpolatePolynomial(const std::vector<double> & x, const std::vector<double> & y, double x0,
                               double * error=nullptr);

  //!
  //! @brief Index i of the interval [x[i], x[i+1]] holding x0, for increasing abscissas: 0 below x[1], n-2 at or
  //! above x[n-2] (the end intervals extend to the outside), by bisection.
  //! @param x increasing abscissas (2 or more) @param x0 value @return index of the interval
  //! @throws MathException if there are fewer than 2 abscissas
  //!
  std::size_t findInterval(const std::vector<double> & x, double x0);

  //!
  //! @brief Indices of the degree+1 points nearest to x0 in an increasing table, as a window [first, first+degree]:
  //! centered on x0 when possible, shifted inside the table at its ends. @param x increasing abscissas
  //! @param x0 value @param degree degree of the local polynomial (the window has degree+1 points; it is reduced to
  //! the size of the table) @return index of the first point of the window
  //!
  std::size_t localWindow(const std::vector<double> & x, double x0, unsigned int degree);

  //!
  //! @brief Value at x0 of the polynomial of the given degree through the degree+1 points of the table nearest to x0
  //! (see localWindow()): degree 1 is linear interpolation, 3 cubic (four points), and so on.
  //! @param x increasing abscissas @param y values @param x0 where to interpolate @param degree degree (1 or more)
  //! @return interpolated value (extrapolated outside the table)
  //! @throws MathException if the sizes differ or the table has fewer than 2 points
  //!
  double interpolateLocalPolynomial(const std::vector<double> & x, const std::vector<double> & y, double x0,
                                    unsigned int degree=3);

  //!
  //! @brief Abscissa where a nondecreasing table reaches the value y0, by linear interpolation in the interval that
  //! holds it (the inverse of a tabulated cumulative distribution, for instance).
  //! @param x abscissas @param y nondecreasing values @param y0 value sought (clamped to [y.front(), y.back()])
  //! @return x such that the linear interpolation of the table gives y0 (the middle of a flat interval)
  //! @throws MathException if the sizes differ or there are fewer than 2 points
  //!
  double inverseLinearInterpolation(const std::vector<double> & x, const std::vector<double> & y, double y0);

  //!
  //! @brief Multilinear interpolation on a regular grid of any rank n: the values at the 2^n corners of the cell
  //! holding x, weighted. Beyond the edges the end cells are extended (linear extrapolation); an axis with a single
  //! node is constant.
  //! @param values values at the nodes, axis 0 varying fastest (node (i0, i1, ...) at i0 + n0 (i1 + n1 (...)))
  //! @param rank number of axes @param nPoints nodes along each axis @param minima position of the first node of each
  //! axis @param spacings distance between the nodes of each axis @param x point (rank coordinates)
  //! @param source name of the calling function (for the exception)
  //! @return interpolated value
  //! @throws MathException if a coordinate is not finite
  //!
  double interpolateMultilinear(const double * values, unsigned int rank, const unsigned int * nPoints,
                                const double * minima, const double * spacings, const double * x,
                                const char * source="interpolateMultilinear");

  } // namespace MATH
} // namespace CAP

#endif // CAP__MATH__InterpolationTools
