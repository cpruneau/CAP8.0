/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include <algorithm>
#include <cmath>
#include "RandomGenerator.hpp"
#include "RandomDistribution.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  long RandomGenerator::uniformInteger(long low, long high)
  {
  if (high < low) std::swap(low, high);
  double n = double(high) - double(low) + 1.0;
  long k = low + long(std::floor(n*random()));
  return std::min(k, high);
  }

  double RandomGenerator::positive()
  {
  double u;
  do u = random(); while (u == 0.0);
  return u;
  }

  double RandomGenerator::gauss(double mean, double rms)
  {
  if (_hasGauss)
    {
    _hasGauss = false;
    return mean + rms*_gauss;
    }
  // Box-Muller: two independent standard normal numbers from two uniform ones
  double r = std::sqrt(-2.0*std::log(positive()));
  double phi = 2.0*M_PI*random();
  _gauss = r*std::sin(phi);
  _hasGauss = true;
  return mean + rms*r*std::cos(phi);
  }

  double RandomGenerator::exp(double mean, double minimum, double maximum)
  {
  if (!(mean > 0.0)) throw MathException("The mean must be positive", "RandomGenerator::exp");
  if (!(maximum > minimum)) throw MathException("The maximum must exceed the minimum", "RandomGenerator::exp");
  // inversion of F(x) = (1 - exp(-(x-minimum)/mean)) / (1 - exp(-(maximum-minimum)/mean))
  double range = std::isinf(maximum) ? 1.0 : -std::expm1(-(maximum - minimum)/mean);
  double x = minimum - mean*std::log1p(-random()*range);
  return std::min(std::max(x, minimum), maximum);
  }

  long RandomGenerator::poisson(double mean)
  {
  if (mean < 0.0) throw MathException("The mean must not be negative", "RandomGenerator::poisson");
  if (mean == 0.0) return 0;
  if (mean < 30.0)
    {
    // product of uniform numbers until it falls below exp(-mean)
    double limit = std::exp(-mean), product = random();
    long k = 0;
    while (product > limit) { product *= random(); k++; }
    return k;
    }
  // transformed rejection with squeeze (W. Hoermann, Insurance: Mathematics and Economics 12 (1993) 39)
  double slam = std::sqrt(mean), loglam = std::log(mean);
  double b = 0.931 + 2.53*slam, a = -0.059 + 0.02483*b;
  double invalpha = 1.1239 + 1.1328/(b - 3.4), vr = 0.9277 - 3.6224/(b - 2.0);
  while (true)
    {
    double u = random() - 0.5, v = random();
    double us = 0.5 - std::fabs(u);
    long k = long(std::floor((2.0*a/us + b)*u + mean + 0.43));
    if (us >= 0.07 && v <= vr) return k;
    if (k < 0 || (us < 0.013 && v > us)) continue;
    if (std::log(v) + std::log(invalpha) - std::log(a/(us*us) + b) <= -mean + double(k)*loglam - std::lgamma(double(k) + 1.0))
      return k;
    }
  }

  long RandomGenerator::binomialInversion(long n, double p)
  {
  // sequential search from 0: P(0) = q^n, P(k+1) = P(k) (n-k)/(k+1) p/q
  double q = 1.0 - p, s = p/q, a = double(n + 1)*s;
  while (true)
    {
    double r = std::pow(q, double(n)), u = random();
    long x = 0;
    while (u > r)
      {
      u -= r;
      x++;
      if (x > n) break;                        // rounding: start again
      r *= a/double(x) - s;
      }
    if (x <= n) return x;
    }
  }

  long RandomGenerator::binomial(long n, double p)
  {
  if (n < 0) throw MathException("The number of trials must not be negative", "RandomGenerator::binomial");
  if (!(p >= 0.0 && p <= 1.0)) throw MathException("The probability must be in [0,1]", "RandomGenerator::binomial");
  if (n == 0 || p == 0.0) return 0;
  if (p == 1.0) return n;
  if (p > 0.5) return n - binomial(n, 1.0 - p);
  // Knuth (The Art of Computer Programming, 3.4.1): the a-th smallest of n uniform numbers is a beta(a, n+1-a) number
  // X; the successes are then those below X, or a plus those above it, binomial numbers of fewer trials
  long successes = 0;
  while (double(n)*p >= 30.0)
    {
    long a = 1 + n/2, b = n + 1 - a;
    double x = beta(double(a), double(b));
    if (x >= p) { n = a - 1; p /= x; }
    else        { successes += a; n = b - 1; p = (p - x)/(1.0 - x); }
    if (p > 0.5) return successes + n - binomial(n, 1.0 - p);
    }
  return successes + (n > 0 && p > 0.0 ? binomialInversion(n, p) : 0);
  }

  double RandomGenerator::gamma(double shape, double scale)
  {
  if (!(shape > 0.0) || !(scale > 0.0)) throw MathException("The shape and the scale must be positive", "RandomGenerator::gamma");
  if (shape < 1.0)
    {
    // gamma(shape) = gamma(shape + 1) u^(1/shape)
    double u = positive();
    return gamma(shape + 1.0, scale)*std::pow(u, 1.0/shape);
    }
  // G. Marsaglia and W. W. Tsang, ACM Transactions on Mathematical Software 26 (2000) 363
  double d = shape - 1.0/3.0, c = 1.0/std::sqrt(9.0*d);
  while (true)
    {
    double x, v;
    do { x = gauss(); v = 1.0 + c*x; } while (v <= 0.0);
    v = v*v*v;
    double u = positive();
    if (u < 1.0 - 0.0331*x*x*x*x) return scale*d*v;
    if (std::log(u) < 0.5*x*x + d*(1.0 - v + std::log(v))) return scale*d*v;
    }
  }

  double RandomGenerator::beta(double a, double b)
  {
  if (!(a > 0.0) || !(b > 0.0)) throw MathException("The parameters must be positive", "RandomGenerator::beta");
  double x = gamma(a), y = gamma(b);
  return x/(x + y);
  }

  double RandomGenerator::chiSquare(double ndf)
  {
  if (!(ndf > 0.0)) throw MathException("The number of degrees of freedom must be positive", "RandomGenerator::chiSquare");
  return gamma(0.5*ndf, 2.0);
  }

  double RandomGenerator::polynomial(const std::vector<double> & c, double minimum, double maximum)
  {
  if (c.empty() || !(maximum > minimum)) throw MathException("No coefficients, or an empty interval", "RandomGenerator::polynomial");
  auto density = [&c](double x) { double v = 0.0; for (size_t k=c.size(); k-->0;) v = v*x + c[k]; return v; };
  // integral from minimum: P(x) - P(minimum), with P the primitive of the polynomial
  auto primitive = [&c](double x) { double v = 0.0; for (size_t k=c.size(); k-->0;) v = v*x + c[k]/double(k + 1); return v*x; };
  const int nChecks = 1000;
  for (int k=0; k<=nChecks; k++)
    if (density(minimum + (maximum - minimum)*k/nChecks) < 0.0)
      throw MathException("The polynomial is negative on the interval", "RandomGenerator::polynomial");
  double low = primitive(minimum), total = primitive(maximum) - low;
  if (!(total > 0.0)) throw MathException("The integral of the polynomial is not positive", "RandomGenerator::polynomial");
  // the cumulative distribution increases: bisection to the precision of the doubles
  double target = random()*total, a = minimum, b = maximum;
  for (int k=0; k<100 && b - a > 1e-15*(std::fabs(a) + std::fabs(b) + 1e-300); k++)
    {
    double m = 0.5*(a + b);
    if (primitive(m) - low < target) a = m; else b = m;
    }
  return 0.5*(a + b);
  }

  double RandomGenerator::sample(const RandomDistribution & distribution)
  {
  return distribution.generate(*this);
  }

  } // namespace MATH
} // namespace CAP
