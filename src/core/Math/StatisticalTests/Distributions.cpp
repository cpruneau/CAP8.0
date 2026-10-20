/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Distributions.cpp
//! @brief Implementation of the distributions of the statistical tests.
#include <algorithm>
#include <cmath>
#include <limits>
#include "Distributions.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {
  namespace Distributions
  {

  namespace
  {
  const double kEpsilon = 1.0E-15;
  const double kTiny = 1.0E-300;
  const int    kMaximumIterations = 10000;

  //! P(a,x) by its series (for x < a+1).
  double gammaSeries(double a, double x)
  {
  double term = 1.0/a, sum = term;
  for (int n=1; n<kMaximumIterations; n++)
    {
    term *= x/(a + n);
    sum += term;
    if (std::fabs(term) < std::fabs(sum)*kEpsilon) break;
    }
  return sum*std::exp(-x + a*std::log(x) - std::lgamma(a));
  }

  //! Q(a,x) by its continued fraction (for x >= a+1), Lentz's method.
  double gammaFraction(double a, double x)
  {
  double b = x + 1.0 - a, c = 1.0/kTiny, d = 1.0/b, h = d;
  for (int i=1; i<kMaximumIterations; i++)
    {
    double an = -i*(i - a);
    b += 2.0;
    d = an*d + b; if (std::fabs(d) < kTiny) d = kTiny;
    c = b + an/c; if (std::fabs(c) < kTiny) c = kTiny;
    d = 1.0/d;
    double delta = d*c;
    h *= delta;
    if (std::fabs(delta - 1.0) < kEpsilon) break;
    }
  return std::exp(-x + a*std::log(x) - std::lgamma(a))*h;
  }

  void checkGamma(double a, double x, const char * source)
  {
  if (!(a > 0)) throw MathException("The parameter a of the incomplete gamma function must be > 0", source);
  if (!(x >= 0)) throw MathException("The variable of the incomplete gamma function must be >= 0", source);
  }

  //! Continued fraction of the incomplete beta function (Lentz's method).
  double betaFraction(double a, double b, double x)
  {
  double qab = a + b, qap = a + 1.0, qam = a - 1.0;
  double c = 1.0, d = 1.0 - qab*x/qap;
  if (std::fabs(d) < kTiny) d = kTiny;
  d = 1.0/d;
  double h = d;
  for (int m=1; m<kMaximumIterations; m++)
    {
    int m2 = 2*m;
    double aa = m*(b - m)*x/((qam + m2)*(a + m2));
    d = 1.0 + aa*d; if (std::fabs(d) < kTiny) d = kTiny;
    c = 1.0 + aa/c; if (std::fabs(c) < kTiny) c = kTiny;
    d = 1.0/d;
    h *= d*c;
    aa = -(a + m)*(qab + m)*x/((a + m2)*(qap + m2));
    d = 1.0 + aa*d; if (std::fabs(d) < kTiny) d = kTiny;
    c = 1.0 + aa/c; if (std::fabs(c) < kTiny) c = kTiny;
    d = 1.0/d;
    double delta = d*c;
    h *= delta;
    if (std::fabs(delta - 1.0) < kEpsilon) break;
    }
  return h;
  }

  //! Tail of Student's t: P(|T| > |t|)/2.
  double studentTail(double t, double ndf)
  {
  if (!(ndf > 0)) throw MathException("The degrees of freedom of Student's t must be > 0", "Distributions::studentCdf()");
  if (std::isinf(t)) return 0.0;
  return 0.5*regularizedBeta(0.5*ndf, 0.5, ndf/(ndf + t*t));
  }

  void checkProbability(double p, const char * source)
  {
  if (!(p >= 0.0 && p <= 1.0)) throw MathException("A probability must be in [0,1]", source);
  }
  } // namespace

  double regularizedGammaP(double a, double x)
  {
  checkGamma(a, x, __FUNCTION__);
  if (x == 0.0) return 0.0;
  return x < a + 1.0 ? gammaSeries(a, x) : 1.0 - gammaFraction(a, x);
  }

  double regularizedGammaQ(double a, double x)
  {
  checkGamma(a, x, __FUNCTION__);
  if (x == 0.0) return 1.0;
  return x < a + 1.0 ? 1.0 - gammaSeries(a, x) : gammaFraction(a, x);
  }

  double regularizedBeta(double a, double b, double x)
  {
  if (!(a > 0) || !(b > 0)) throw MathException("The parameters of the incomplete beta function must be > 0", __FUNCTION__);
  if (!(x >= 0.0 && x <= 1.0)) throw MathException("The variable of the incomplete beta function must be in [0,1]", __FUNCTION__);
  if (x == 0.0 || x == 1.0) return x;
  double front = std::exp(std::lgamma(a + b) - std::lgamma(a) - std::lgamma(b) + a*std::log(x) + b*std::log1p(-x));
  if (x < (a + 1.0)/(a + b + 2.0)) return front*betaFraction(a, b, x)/a;
  return 1.0 - front*betaFraction(b, a, 1.0 - x)/b;
  }

  double normalDensity(double z) { return std::exp(-0.5*z*z)/std::sqrt(2.0*M_PI); }
  double normalCdf(double z)     { return 0.5*std::erfc(-z/std::sqrt(2.0)); }
  double normalUpper(double z)   { return 0.5*std::erfc(z/std::sqrt(2.0)); }

  double normalQuantile(double p)
  {
  if (!(p > 0.0 && p < 1.0)) throw MathException("The probability of a quantile must be in (0,1)", __FUNCTION__);
  // Acklam's rational approximations (relative error 1.15E-9), then one step of Halley's method
  static const double a[] = { -3.969683028665376e+01, 2.209460984245205e+02, -2.759285104469687e+02,
                              1.383577518672690e+02, -3.066479806614716e+01, 2.506628277459239e+00 };
  static const double b[] = { -5.447609879822406e+01, 1.615858368580409e+02, -1.556989798598866e+02,
                              6.680131188771972e+01, -1.328068155288572e+01 };
  static const double c[] = { -7.784894002430293e-03, -3.223964580411365e-01, -2.400758277161838e+00,
                              -2.549732539343734e+00, 4.374664141464968e+00, 2.938163982698783e+00 };
  static const double d[] = { 7.784695709041462e-03, 3.224671290700398e-01, 2.445134137142996e+00, 3.754408661907416e+00 };
  const double low = 0.02425, high = 1.0 - low;
  double x;
  if (p < low)
    {
    double q = std::sqrt(-2.0*std::log(p));
    x = (((((c[0]*q + c[1])*q + c[2])*q + c[3])*q + c[4])*q + c[5])/((((d[0]*q + d[1])*q + d[2])*q + d[3])*q + 1.0);
    }
  else if (p <= high)
    {
    double q = p - 0.5, r = q*q;
    x = (((((a[0]*r + a[1])*r + a[2])*r + a[3])*r + a[4])*r + a[5])*q/(((((b[0]*r + b[1])*r + b[2])*r + b[3])*r + b[4])*r + 1.0);
    }
  else
    {
    double q = std::sqrt(-2.0*std::log(1.0 - p));
    x = -(((((c[0]*q + c[1])*q + c[2])*q + c[3])*q + c[4])*q + c[5])/((((d[0]*q + d[1])*q + d[2])*q + d[3])*q + 1.0);
    }
  double e = (p < 0.5 ? normalCdf(x) - p : -(normalUpper(x) - (1.0 - p)));
  double u = e*std::sqrt(2.0*M_PI)*std::exp(0.5*x*x);
  return x - u/(1.0 + 0.5*x*u);
  }

  double studentCdf(double t, double ndf)
  {
  double tail = studentTail(t, ndf);
  return t >= 0 ? 1.0 - tail : tail;
  }

  double studentUpper(double t, double ndf)
  {
  double tail = studentTail(t, ndf);
  return t >= 0 ? tail : 1.0 - tail;
  }

  double chiSquareCdf(double x, double ndf)
  {
  if (!(ndf > 0)) throw MathException("The degrees of freedom of a chi square must be > 0", __FUNCTION__);
  return regularizedGammaP(0.5*ndf, 0.5*x);
  }

  double chiSquareUpper(double x, double ndf)
  {
  if (!(ndf > 0)) throw MathException("The degrees of freedom of a chi square must be > 0", __FUNCTION__);
  return regularizedGammaQ(0.5*ndf, 0.5*x);
  }

  double binomialProbability(unsigned long k, unsigned long n, double p)
  {
  checkProbability(p, __FUNCTION__);
  if (k > n) return 0.0;
  if (p == 0.0) return k == 0 ? 1.0 : 0.0;
  if (p == 1.0) return k == n ? 1.0 : 0.0;
  return std::exp(std::lgamma(n + 1.0) - std::lgamma(k + 1.0) - std::lgamma(double(n - k) + 1.0)
                  + double(k)*std::log(p) + double(n - k)*std::log1p(-p));
  }

  double binomialCdf(unsigned long k, unsigned long n, double p)
  {
  checkProbability(p, __FUNCTION__);
  if (k >= n) return 1.0;
  if (p == 0.0) return 1.0;
  if (p == 1.0) return 0.0;
  return regularizedBeta(double(n - k), double(k) + 1.0, 1.0 - p);
  }

  double binomialUpper(unsigned long k, unsigned long n, double p)
  {
  checkProbability(p, __FUNCTION__);
  if (k == 0) return 1.0;
  if (k > n) return 0.0;
  if (p == 0.0) return 0.0;
  if (p == 1.0) return 1.0;
  return regularizedBeta(double(k), double(n - k) + 1.0, p);
  }

  double poissonProbability(unsigned long k, double mu)
  {
  if (!(mu >= 0)) throw MathException("The mean of a Poisson distribution must be >= 0", __FUNCTION__);
  if (mu == 0.0) return k == 0 ? 1.0 : 0.0;
  return std::exp(double(k)*std::log(mu) - mu - std::lgamma(k + 1.0));
  }

  double poissonCdf(unsigned long k, double mu)
  {
  if (!(mu >= 0)) throw MathException("The mean of a Poisson distribution must be >= 0", __FUNCTION__);
  if (mu == 0.0) return 1.0;
  return regularizedGammaQ(double(k) + 1.0, mu);
  }

  double poissonUpper(unsigned long k, double mu)
  {
  if (!(mu >= 0)) throw MathException("The mean of a Poisson distribution must be >= 0", __FUNCTION__);
  if (k == 0) return 1.0;
  if (mu == 0.0) return 0.0;
  return regularizedGammaP(double(k), mu);
  }

  double kolmogorovUpper(double lambda)
  {
  if (!(lambda > 0.0)) return 1.0;
  if (lambda < 0.2) return 1.0;                                   // the series converges slowly there, and Q is 1 to 1E-10
  double sum = 0.0, sign = 1.0, previous = 0.0;
  for (int j=1; j<=100; j++)
    {
    double term = sign*2.0*std::exp(-2.0*j*j*lambda*lambda);
    sum += term;
    if (std::fabs(term) <= 1.0E-12*std::fabs(sum) || std::fabs(term) <= 1.0E-16*previous) break;
    previous = std::fabs(term);
    sign = -sign;
    }
  return std::min(1.0, std::max(0.0, sum));
  }

  } // namespace Distributions
  } // namespace MATH
} // namespace CAP
