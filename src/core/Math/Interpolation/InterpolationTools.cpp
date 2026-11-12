/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file InterpolationTools.cpp
//! @brief Implementation of the interpolation functions.
#include <algorithm>
#include <cmath>
#include "InterpolationTools.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  double interpolatePolynomial(const double * x, const double * y, std::size_t n, double x0, double * error)
  {
  if (n == 0 || !x || !y) throw MathException("No points", "interpolatePolynomial");
  // Neville's table, kept as the corrections c (upward) and d (downward), starting from the nearest point
  std::vector<double> c(y, y + n), d(y, y + n);
  std::size_t nearest = 0;
  for (std::size_t i=1; i<n; i++) if (std::fabs(x0 - x[i]) < std::fabs(x0 - x[nearest])) nearest = i;
  double value = y[nearest];
  long ns = long(nearest) - 1;
  double last = 0.0;
  for (std::size_t m=1; m<n; m++)
    {
    for (std::size_t i=0; i<n-m; i++)
      {
      double ho = x[i] - x0, hp = x[i+m] - x0, w = c[i+1] - d[i], den = ho - hp;
      if (den == 0.0) throw MathException("Two abscissas are equal", "interpolatePolynomial");
      den = w/den;
      d[i] = hp*den;
      c[i] = ho*den;
      }
    // the correction that keeps the path closest to the middle of the table
    last = (2*(ns + 1) < long(n - m)) ? c[ns + 1] : d[ns--];
    value += last;
    }
  if (error) *error = std::fabs(last);
  return value;
  }

  double interpolatePolynomial(const std::vector<double> & x, const std::vector<double> & y, double x0, double * error)
  {
  if (x.size() != y.size()) throw MathException("The sizes of x and y differ", "interpolatePolynomial");
  return interpolatePolynomial(x.data(), y.data(), x.size(), x0, error);
  }

  std::size_t findInterval(const std::vector<double> & x, double x0)
  {
  std::size_t n = x.size();
  if (n < 2) throw MathException("Fewer than 2 abscissas", "findInterval");
  if (!(x0 >= x[1])) return 0;          // also for NaN
  if (x0 >= x[n-2]) return n - 2;
  std::size_t i = std::size_t(std::upper_bound(x.begin(), x.end(), x0) - x.begin());
  return i - 1;
  }

  std::size_t localWindow(const std::vector<double> & x, double x0, unsigned int degree)
  {
  std::size_t n = x.size();
  std::size_t m = std::min<std::size_t>(std::size_t(degree) + 1, n);
  if (n <= m) return 0;
  std::size_t i = findInterval(x, x0);                  // x0 in [x[i], x[i+1]]
  // take the m points around the interval: (m-1)/2 below x[i]... centered on the interval
  long first = long(i) - long((m - 1)/2);
  if (m % 2 == 1 && i + 1 < n && std::fabs(x0 - x[i+1]) < std::fabs(x0 - x[i])) first++;   // odd: center on the nearest
  first = std::max(0L, std::min(first, long(n - m)));
  return std::size_t(first);
  }

  double interpolateLocalPolynomial(const std::vector<double> & x, const std::vector<double> & y, double x0,
                                    unsigned int degree)
  {
  if (x.size() != y.size()) throw MathException("The sizes of x and y differ", "interpolateLocalPolynomial");
  if (x.size() < 2) throw MathException("Fewer than 2 points", "interpolateLocalPolynomial");
  if (degree == 0) degree = 1;
  std::size_t first = localWindow(x, x0, degree);
  std::size_t m = std::min<std::size_t>(std::size_t(degree) + 1, x.size());
  return interpolatePolynomial(x.data() + first, y.data() + first, m, x0);
  }

  double inverseLinearInterpolation(const std::vector<double> & x, const std::vector<double> & y, double y0)
  {
  if (x.size() != y.size()) throw MathException("The sizes of x and y differ", "inverseLinearInterpolation");
  if (x.size() < 2) throw MathException("Fewer than 2 points", "inverseLinearInterpolation");
  // the interval [k-1, k] where the table reaches y0
  std::size_t k = std::size_t(std::upper_bound(y.begin(), y.end(), y0) - y.begin());
  if (k == 0) k = 1;
  if (k >= y.size()) k = y.size() - 1;
  double y1 = y[k-1], y2 = y[k];
  double f = y2 > y1 ? (y0 - y1)/(y2 - y1) : 0.5;
  f = std::max(0.0, std::min(1.0, f));
  return x[k-1] + f*(x[k] - x[k-1]);
  }

  double interpolateMultilinear(const double * values, unsigned int rank, const unsigned int * nPoints,
                                const double * minima, const double * spacings, const double * x, const char * source)
  {
  std::vector<unsigned int> base(rank), stride(rank);
  std::vector<double> frac(rank);
  unsigned int s = 1;
  for (unsigned int k=0; k<rank; k++)
    {
    if (!std::isfinite(x[k])) throw MathException("Coordinate is not finite", source);
    stride[k] = s;
    s *= nPoints[k];
    if (nPoints[k] == 1) { base[k] = 0; frac[k] = 0.0; continue; }
    double t = (x[k] - minima[k])/spacings[k];
    double cell = std::floor(t);
    if (cell < 0.0) cell = 0.0;
    if (cell > double(nPoints[k] - 2)) cell = double(nPoints[k] - 2);   // last cell; linear extrapolation beyond the edges
    base[k] = (unsigned int) cell;
    frac[k] = t - cell;
    }
  // sum over the 2^n corners of the cell; axes with a single node contribute one corner only
  double sum = 0.0;
  const unsigned long long nCorners = 1ull << rank;
  for (unsigned long long corner=0; corner<nCorners; corner++)
    {
    double weight = 1.0;
    unsigned int index = 0;
    bool valid = true;
    for (unsigned int k=0; k<rank; k++)
      {
      bool upper = (corner >> k) & 1ull;
      if (upper && nPoints[k] == 1) { valid = false; break; }
      weight *= upper ? frac[k] : 1.0 - frac[k];
      index  += (base[k] + (upper ? 1 : 0))*stride[k];
      }
    if (valid) sum += weight*values[index];
    }
  return sum;
  }

  } // namespace MATH
} // namespace CAP
