/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FourierTools.cpp
//! @brief Implementation of FourierTools.
#include <algorithm>
#include <cmath>
#include <limits>
#include <mutex>
#include "FourierTools.hpp"
#include "GaussLegendreIntegrator.hpp"
#include "MathExceptions.hpp"

namespace Datum
{
  namespace MATH
  {
  namespace FourierTools
  {

  namespace
  {
  const double kPi = 3.14159265358979323846;
  const double kSqrtPi = 1.77245385090551602730;
  const double kInfinity = std::numeric_limits<double>::infinity();
  const Complex kI(0.0, 1.0);

  //! Coefficients of Weideman's approximation of w(z) (J.A.C. Weideman, SIAM J. Numer. Anal. 31 (1994) 1497).
  struct Weideman
  {
    static const int N = 64;
    double L;
    double a[N];   // a[m-1]: coefficient of Z^(m-1)
    Weideman()
    {
      const int M = 2*N, M2 = 2*M;
      L = std::sqrt(N/std::sqrt(2.0));
      // f on M2 points: f[0] = 0, then the values at k = -M+1 ... M-1; fftshift; real part of the DFT / M2
      std::vector<double> f(M2, 0.0);
      for (int k=-M+1; k<=M-1; k++)
        {
        double theta = k*kPi/M, t = L*std::tan(theta/2.0);
        f[std::size_t(k + M)] = std::exp(-t*t)*(L*L + t*t);
        }
      std::vector<double> y(M2);
      for (int i=0; i<M2; i++) y[std::size_t(i)] = f[std::size_t((i + M2/2) % M2)];
      for (int m=1; m<=N; m++)
        {
        double sum = 0.0;
        for (int j=0; j<M2; j++) sum += y[std::size_t(j)]*std::cos(2.0*kPi*double(j)*m/M2);
        a[m-1] = sum/M2;
        }
    }
  };

  const Weideman & weideman()
  {
    static const Weideman w;
    return w;
  }

  //! w(z) for Im z >= 0.
  Complex faddeevaUpper(Complex z)
  {
  const Weideman & w = weideman();
  Complex denominator = w.L - kI*z;
  Complex Z = (w.L + kI*z)/denominator;
  Complex p = 0.0;
  for (int m=Weideman::N; m>=1; m--) p = p*Z + w.a[m-1];
  return 2.0*p/(denominator*denominator) + (1.0/kSqrtPi)/denominator;
  }

  //! e^z - 1 over z, accurate for small z.
  Complex expm1OverZ(Complex z)
  {
  if (std::abs(z) < 0.5)
    {
    Complex term = 1.0, sum = 1.0;
    for (int n=2; n<30; n++) { term *= z/double(n); sum += term; if (std::abs(term) < 1e-17*std::abs(sum)) break; }
    return sum;
    }
  return (std::exp(z) - 1.0)/z;
  }

  //! Logarithm of cosh(x), for any x >= 0.
  double logCosh(double x)
  {
  x = std::fabs(x);
  return x + std::log1p(std::exp(-2.0*x)) - std::log(2.0);
  }
  } // namespace

  Complex faddeeva(Complex z)
  {
  if (z.imag() >= 0.0) return faddeevaUpper(z);
  return 2.0*std::exp(-z*z) - faddeevaUpper(-z);
  }

  Complex scaledErf(double u, double v)
  {
  if (std::isinf(u)) return (u > 0 ? 1.0 : -1.0)*std::exp(-v*v);
  if (u < 0.0) return -scaledErf(-u, -v);
  // erf(z) = 1 - e^{-z^2} w(iz), z = u + iv; e^{-v^2} e^{-z^2} = e^{-u^2 - 2iuv}
  Complex z(u, v);
  return std::exp(-v*v) - std::exp(Complex(-u*u, -2.0*u*v))*faddeevaUpper(kI*z);
  }

  Complex exponentialIntegral(Complex c, double a, double b)
  {
  if (a == b) return 0.0;
  if (a > b) return -exponentialIntegral(c, b, a);
  bool lowInfinite = std::isinf(a), highInfinite = std::isinf(b);
  if (lowInfinite || highInfinite)
    {
    if ((highInfinite && !(c.real() < 0.0)) || (lowInfinite && !(c.real() > 0.0)))
      throw MathException("Integral of e^{cx} diverges on that domain", __FUNCTION__);
    if (lowInfinite && highInfinite) throw MathException("Integral of e^{cx} diverges on the whole line", __FUNCTION__);
    if (highInfinite) return -std::exp(c*a)/c;
    return std::exp(c*b)/c;
    }
  double length = b - a;
  return std::exp(c*a)*length*expm1OverZ(c*length);
  }

  Complex gaussianLine(double k, double mu, double sigma)
  {
  return sigma*std::sqrt(2.0*kPi)*std::exp(Complex(-0.5*sigma*sigma*k*k, -k*mu));
  }

  Complex gaussianDomain(double k, double mu, double sigma, double a, double b)
  {
  if (!(sigma > 0.0)) throw MathException("The width of a Gaussian must be > 0", __FUNCTION__);
  double s = sigma*std::sqrt(2.0);
  double v = k*sigma/std::sqrt(2.0);
  double ua = std::isinf(a) ? a : (a - mu)/s;
  double ub = std::isinf(b) ? b : (b - mu)/s;
  return sigma*std::sqrt(kPi/2.0)*std::exp(Complex(0.0, -k*mu))*(scaledErf(ub, v) - scaledErf(ua, v));
  }

  Complex halfGaussianMoment(unsigned int m, double k, double s)
  {
  Complex z(-k*s/std::sqrt(2.0), 0.0);
  Complex w = faddeevaUpper(z);
  Complex w1 = -2.0*z*w + 2.0*kI/kSqrtPi;   // w'(z)
  switch (m)
    {
    case 0: return s*std::sqrt(kPi/2.0)*w;
    case 1: return -kI*(s*s*kSqrtPi/2.0)*w1;
    case 2:
      {
      Complex w2 = -2.0*w - 2.0*z*w1;        // w''(z)
      return -(s*s*s*kSqrtPi/(2.0*std::sqrt(2.0)))*w2;
      }
    default: throw MathException("Only the moments 0, 1 and 2 are available", __FUNCTION__);
    }
  }

  void sphericalBesselJ(unsigned int nMax, double x, std::vector<double> & j)
  {
  j.assign(nMax + 1, 0.0);
  if (x == 0.0) { j[0] = 1.0; return; }
  double ax = std::fabs(x);
  double sign = x < 0.0 ? -1.0 : 1.0;
  if (ax > double(nMax))
    {
    // forward recurrence, stable for n < x
    j[0] = std::sin(ax)/ax;
    if (nMax >= 1) j[1] = std::sin(ax)/(ax*ax) - std::cos(ax)/ax;
    for (unsigned int n=1; n<nMax; n++) j[n+1] = (2.0*n + 1.0)/ax*j[n] - j[n-1];
    }
  else
    {
    // Miller: backward recurrence from far above, normalized by j_0 or j_1 (the larger)
    unsigned int start = nMax + 32 + (unsigned int)(std::sqrt(40.0*(nMax + 1)));
    double next = 0.0, current = 1e-300;
    std::vector<double> values(nMax + 1, 0.0);
    for (unsigned int n=start; n>0; n--)
      {
      double previous = (2.0*n + 1.0)/ax*current - next;   // j_{n-1}
      next = current;
      current = previous;
      if (n - 1 <= nMax) values[n-1] = current;
      if (std::fabs(current) > 1e250)
        {
        current *= 1e-250; next *= 1e-250;
        for (auto & v : values) v *= 1e-250;
        }
      }
    double j0 = std::sin(ax)/ax, j1 = std::sin(ax)/(ax*ax) - std::cos(ax)/ax;
    double scale = (std::fabs(j0) >= std::fabs(j1) || nMax == 0) ? j0/values[0] : j1/values[1];
    for (unsigned int n=0; n<=nMax; n++) j[n] = values[n]*scale;
    }
  if (sign < 0.0) for (unsigned int n=1; n<=nMax; n+=2) j[n] = -j[n];
  }

  double sphericalBesselJ(unsigned int n, double x)
  {
  std::vector<double> j;
  sphericalBesselJ(n, x, j);
  return j[n];
  }

  double besselKNormalized(double nu, double z)
  {
  if (!(nu > 0.0)) throw MathException("The order must be > 0", __FUNCTION__);
  if (z < 0.0) z = -z;
  if (z == 0.0) return 1.0;
  // K_nu(z) = integral from 0 to infinity of e^{-z cosh t} cosh(nu t) dt, by the trapezoid rule (exponentially
  // accurate for this integrand), summed in logarithms; normalization Gamma(nu) 2^(nu-1), and z^nu
  double peak = std::asinh(nu/z);
  double width = 1.0/std::sqrt(z*std::cosh(peak));
  double h = std::min(0.05, width/8.0);
  double logNorm = nu*std::log(z) - std::lgamma(nu) - (nu - 1.0)*std::log(2.0);
  auto logTerm = [&](double t) { return -z*std::cosh(t) + logCosh(nu*t); };
  double logMax = logTerm(peak);
  double sum = 0.0;
  for (long i=0; ; i++)
    {
    double t = i*h;
    double l = logTerm(t) - logMax;
    double term = std::exp(l)*(i == 0 ? 0.5 : 1.0);
    sum += term;
    if (t > peak && l < -50.0) break;
    if (i > 2000000) break;
    }
  return std::exp(std::log(sum*h) + logMax + logNorm);
  }

  double sinc(double u)
  {
  if (std::fabs(u) < 1e-4) return 1.0 - u*u/6.0;
  return std::sin(u)/u;
  }

  std::vector<double> legendreCoefficients(const std::function<double(double)> & f, double a, double b, unsigned int degree)
  {
  if (!std::isfinite(a) || !std::isfinite(b) || !(b > a)) throw MathException("A finite domain [a, b] with a < b is needed", __FUNCTION__);
  std::vector<double> nodes, weights;
  GaussLegendreIntegrator::rule(degree + 1, nodes, weights);
  double m = 0.5*(a + b), h = 0.5*(b - a);
  std::vector<double> c(degree + 1, 0.0);
  for (std::size_t q=0; q<nodes.size(); q++)
    {
    double t = nodes[q];
    double value = f(m + h*t)*weights[q];
    double p0 = 1.0, p1 = t;
    c[0] += value*p0;
    if (degree >= 1) c[1] += value*p1;
    for (unsigned int n=1; n<degree; n++)
      {
      double p2 = ((2.0*n + 1.0)*t*p1 - n*p0)/(n + 1.0);
      c[n+1] += value*p2;
      p0 = p1; p1 = p2;
      }
    }
  for (unsigned int n=0; n<=degree; n++) c[n] *= (2.0*n + 1.0)/2.0;
  return c;
  }

  std::vector<double> adaptiveLegendreCoefficients(const std::function<double(double)> & f, double a, double b,
                                                   double tolerance, unsigned int maxDegree, double * tail)
  {
  std::vector<double> c;
  double relative = 1.0;
  for (unsigned int degree=16; ; degree*=2)
    {
    if (degree > maxDegree) degree = maxDegree;
    c = legendreCoefficients(f, a, b, degree);
    double largest = 0.0, last = 0.0;
    for (double v : c) largest = std::max(largest, std::fabs(v));
    for (std::size_t n=c.size() - c.size()/8; n<c.size(); n++) last = std::max(last, std::fabs(c[n]));
    relative = largest > 0.0 ? last/largest : 0.0;
    if (relative <= tolerance || degree >= maxDegree) break;
    }
  // trailing coefficients below the tolerance are dropped
  double largest = 0.0;
  for (double v : c) largest = std::max(largest, std::fabs(v));
  while (c.size() > 1 && std::fabs(c.back()) <= 1e-16*largest) c.pop_back();
  if (tail) *tail = relative;
  return c;
  }

  Complex legendreTransform(const std::vector<double> & c, double k, double a, double b)
  {
  if (c.empty()) return 0.0;
  double m = 0.5*(a + b), h = 0.5*(b - a);
  std::vector<double> j;
  sphericalBesselJ((unsigned int)(c.size() - 1), k*h, j);
  Complex sum = 0.0, phase = 1.0;   // (-i)^n
  for (std::size_t n=0; n<c.size(); n++)
    {
    sum += c[n]*phase*j[n];
    phase *= -kI;
    }
  return 2.0*h*std::exp(Complex(0.0, -k*m))*sum;
  }

  Complex polynomialTransform(const std::function<double(double)> & f, unsigned int degree, double k, double a, double b)
  {
  if (a == b) return 0.0;
  if (a > b) return -polynomialTransform(f, degree, k, b, a);
  return legendreTransform(legendreCoefficients(f, a, b, degree), k, a, b);
  }

  PiecewiseLegendre PiecewiseLegendre::build(const std::function<double(double)> & f, const std::vector<double> & edges,
                                             double tolerance, unsigned int maxDegree)
  {
  PiecewiseLegendre p;
  for (std::size_t s=0; s+1<edges.size(); s++)
    {
    if (!(edges[s+1] > edges[s])) continue;
    double tail = 0.0;
    p.coefficients.push_back(adaptiveLegendreCoefficients(f, edges[s], edges[s+1], tolerance, maxDegree, &tail));
    if (p.edges.empty()) p.edges.push_back(edges[s]);
    p.edges.push_back(edges[s+1]);
    p.tail = std::max(p.tail, tail);
    p.largestDegree = std::max(p.largestDegree, (unsigned int)(p.coefficients.back().size() - 1));
    }
  if (p.coefficients.empty()) throw MathException("No segment of positive length", __FUNCTION__);
  return p;
  }

  Complex PiecewiseLegendre::transform(double k) const
  {
  Complex sum = 0.0;
  for (std::size_t s=0; s<coefficients.size(); s++) sum += legendreTransform(coefficients[s], k, edges[s], edges[s+1]);
  return sum;
  }

  } // namespace FourierTools
  } // namespace MATH
} // namespace Datum
