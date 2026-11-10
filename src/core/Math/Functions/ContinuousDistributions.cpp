/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file ContinuousDistributions.cpp
//! @brief Implementation of the probability densities of one variable.
#include <cmath>
#include <limits>
#include "ContinuousDistributions.hpp"
#include "Distributions.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  namespace
  {
  const double kNaN = std::numeric_limits<double>::quiet_NaN();
  const double kInfinity = std::numeric_limits<double>::infinity();
  const double kPi = 3.14159265358979323846;

  Parameter positive(const char * name, double value) { return Parameter(name, value, Parameter::Low, 0.0); }

  void requirePositive(double value, const char * what, const char * where)
  {
  if (!(value > 0.0)) throw MathException(String(what) + " must be > 0", where);
  }
  } // namespace

  // ---- ContinuousDistribution

  ContinuousDistribution::ContinuousDistribution(const std::vector<Parameter> & parameters)
  :
  ParameterizedFunction(parameters)
  { }

  double ContinuousDistribution::integralInRange(double low, double high) const
  {
  if (low == high) return 0.0;
  if (low > high) return -integralInRange(high, low);
  // in the upper tail, the difference of the upper tails keeps its precision
  double fromCdf = cdf(high) - cdf(low);
  if (cdf(low) > 0.5) fromCdf = cdfUpper(low) - cdfUpper(high);
  return amplitude()*fromCdf;
  }

  // ---- StudentDistribution

  StudentDistribution::StudentDistribution(double amplitude, double ndf, double location, double scale)
  :
  ContinuousDistribution({Parameter("amplitude", amplitude), positive("ndf", ndf), Parameter("location", location),
                          positive("scale", scale)})
  {
  requirePositive(ndf, "The degrees of freedom of a Student distribution", __FUNCTION__);
  requirePositive(scale, "The scale of a Student distribution", __FUNCTION__);
  }

  double StudentDistribution::density(double x) const
  {
  double nu = ndf(), s = scale();
  if (!(nu > 0.0) || !(s > 0.0)) return 0.0;
  double t = (x - location())/s;
  double logNorm = std::lgamma(0.5*(nu + 1.0)) - std::lgamma(0.5*nu) - 0.5*std::log(nu*kPi) - std::log(s);
  return std::exp(logNorm - 0.5*(nu + 1.0)*std::log1p(t*t/nu));
  }

  double StudentDistribution::cdf(double x) const
  {
  if (!(ndf() > 0.0) || !(scale() > 0.0)) return kNaN;
  if (std::isinf(x)) return x > 0 ? 1.0 : 0.0;
  return Distributions::studentCdf((x - location())/scale(), ndf());
  }

  double StudentDistribution::cdfUpper(double x) const
  {
  if (!(ndf() > 0.0) || !(scale() > 0.0)) return kNaN;
  if (std::isinf(x)) return x > 0 ? 0.0 : 1.0;
  return Distributions::studentUpper((x - location())/scale(), ndf());
  }

  double StudentDistribution::mean() const
  {
  return ndf() > 1.0 ? location() : kNaN;
  }

  double StudentDistribution::variance() const
  {
  double nu = ndf();
  if (nu > 2.0) return scale()*scale()*nu/(nu - 2.0);
  return nu > 1.0 ? kInfinity : kNaN;
  }

  // ---- ChiSquareDistribution

  ChiSquareDistribution::ChiSquareDistribution(double amplitude, double ndf)
  :
  ContinuousDistribution({Parameter("amplitude", amplitude), positive("ndf", ndf)})
  {
  requirePositive(ndf, "The degrees of freedom of a chi square distribution", __FUNCTION__);
  }

  double ChiSquareDistribution::density(double x) const
  {
  double k = ndf();
  if (!(k > 0.0) || x < 0.0) return 0.0;
  if (x == 0.0) return k < 2.0 ? kInfinity : (k == 2.0 ? 0.5 : 0.0);
  double h = 0.5*k;
  return std::exp((h - 1.0)*std::log(x) - 0.5*x - h*std::log(2.0) - std::lgamma(h));
  }

  double ChiSquareDistribution::cdf(double x) const
  {
  if (!(ndf() > 0.0)) return kNaN;
  if (x <= 0.0) return 0.0;
  if (std::isinf(x)) return 1.0;
  return Distributions::chiSquareCdf(x, ndf());
  }

  double ChiSquareDistribution::cdfUpper(double x) const
  {
  if (!(ndf() > 0.0)) return kNaN;
  if (x <= 0.0) return 1.0;
  if (std::isinf(x)) return 0.0;
  return Distributions::chiSquareUpper(x, ndf());
  }

  // ---- FDistribution

  FDistribution::FDistribution(double amplitude, double d1, double d2)
  :
  ContinuousDistribution({Parameter("amplitude", amplitude), positive("d1", d1), positive("d2", d2)})
  {
  requirePositive(d1, "The degrees of freedom d1 of an F distribution", __FUNCTION__);
  requirePositive(d2, "The degrees of freedom d2 of an F distribution", __FUNCTION__);
  }

  double FDistribution::density(double x) const
  {
  double a = d1(), b = d2();
  if (!(a > 0.0) || !(b > 0.0) || x < 0.0) return 0.0;
  if (x == 0.0) return a < 2.0 ? kInfinity : (a == 2.0 ? 1.0 : 0.0);
  double logBeta = std::lgamma(0.5*a) + std::lgamma(0.5*b) - std::lgamma(0.5*(a + b));
  double logDensity = 0.5*(a*std::log(a*x) + b*std::log(b) - (a + b)*std::log(a*x + b)) - std::log(x) - logBeta;
  return std::exp(logDensity);
  }

  double FDistribution::cdf(double x) const
  {
  double a = d1(), b = d2();
  if (!(a > 0.0) || !(b > 0.0)) return kNaN;
  if (x <= 0.0) return 0.0;
  if (std::isinf(x)) return 1.0;
  return Distributions::regularizedBeta(0.5*a, 0.5*b, a*x/(a*x + b));
  }

  double FDistribution::cdfUpper(double x) const
  {
  double a = d1(), b = d2();
  if (!(a > 0.0) || !(b > 0.0)) return kNaN;
  if (x <= 0.0) return 1.0;
  if (std::isinf(x)) return 0.0;
  return Distributions::regularizedBeta(0.5*b, 0.5*a, b/(a*x + b));   // I_(1-z)(b/2, a/2) = 1 - I_z(a/2, b/2)
  }

  double FDistribution::mean() const
  {
  double b = d2();
  return b > 2.0 ? b/(b - 2.0) : kNaN;
  }

  double FDistribution::variance() const
  {
  double a = d1(), b = d2();
  if (b > 4.0) return 2.0*b*b*(a + b - 2.0)/(a*(b - 2.0)*(b - 2.0)*(b - 4.0));
  return b > 2.0 ? kInfinity : kNaN;
  }

  // ---- CauchyDistribution

  CauchyDistribution::CauchyDistribution(double amplitude, double location, double gamma)
  :
  ContinuousDistribution({Parameter("amplitude", amplitude), Parameter("location", location), positive("gamma", gamma)})
  {
  requirePositive(gamma, "The half width of a Cauchy distribution", __FUNCTION__);
  }

  double CauchyDistribution::density(double x) const
  {
  double g = gamma();
  if (!(g > 0.0)) return 0.0;
  double d = x - location();
  return g/(kPi*(d*d + g*g));
  }

  double CauchyDistribution::derivativeAt(double x) const
  {
  double g = gamma();
  if (!(g > 0.0)) return 0.0;
  double d = x - location(), s = d*d + g*g;
  return -amplitude()*2.0*g*d/(kPi*s*s);
  }

  double CauchyDistribution::cdf(double x) const
  {
  if (!(gamma() > 0.0)) return kNaN;
  return 0.5 + std::atan((x - location())/gamma())/kPi;
  }

  double CauchyDistribution::cdfUpper(double x) const
  {
  if (!(gamma() > 0.0)) return kNaN;
  double t = (x - location())/gamma();
  if (t > 1.0) return std::atan(1.0/t)/kPi;   // precise far in the tail
  return 0.5 - std::atan(t)/kPi;
  }

  double CauchyDistribution::mean() const { return kNaN; }

  double CauchyDistribution::variance() const { return kNaN; }

  // ---- MaxwellBoltzmannDistribution

  MaxwellBoltzmannDistribution::MaxwellBoltzmannDistribution(double amplitude, double scale)
  :
  ContinuousDistribution({Parameter("amplitude", amplitude), positive("a", scale)})
  {
  requirePositive(scale, "The scale of a Maxwell-Boltzmann distribution", __FUNCTION__);
  }

  double MaxwellBoltzmannDistribution::density(double x) const
  {
  double a = scale();
  if (!(a > 0.0) || x < 0.0) return 0.0;
  double u = x/a;
  return std::sqrt(2.0/kPi)*u*u*std::exp(-0.5*u*u)/a;
  }

  double MaxwellBoltzmannDistribution::cdf(double x) const
  {
  double a = scale();
  if (!(a > 0.0)) return kNaN;
  if (x <= 0.0) return 0.0;
  if (std::isinf(x)) return 1.0;
  double u = x/a;
  return std::erf(u/std::sqrt(2.0)) - std::sqrt(2.0/kPi)*u*std::exp(-0.5*u*u);
  }

  double MaxwellBoltzmannDistribution::cdfUpper(double x) const
  {
  double a = scale();
  if (!(a > 0.0)) return kNaN;
  if (x <= 0.0) return 1.0;
  if (std::isinf(x)) return 0.0;
  double u = x/a;
  return std::erfc(u/std::sqrt(2.0)) + std::sqrt(2.0/kPi)*u*std::exp(-0.5*u*u);
  }

  double MaxwellBoltzmannDistribution::mode() const { return std::sqrt(2.0)*scale(); }

  double MaxwellBoltzmannDistribution::mean() const { return 2.0*scale()*std::sqrt(2.0/kPi); }

  double MaxwellBoltzmannDistribution::variance() const { return scale()*scale()*(3.0*kPi - 8.0)/kPi; }

  // ---- WeibullDistribution

  WeibullDistribution::WeibullDistribution(double amplitude, double shape, double scale)
  :
  ContinuousDistribution({Parameter("amplitude", amplitude), positive("k", shape), positive("lambda", scale)})
  {
  requirePositive(shape, "The shape of a Weibull distribution", __FUNCTION__);
  requirePositive(scale, "The scale of a Weibull distribution", __FUNCTION__);
  }

  double WeibullDistribution::density(double x) const
  {
  double k = shape(), l = scale();
  if (!(k > 0.0) || !(l > 0.0) || x < 0.0) return 0.0;
  if (x == 0.0) return k < 1.0 ? kInfinity : (k == 1.0 ? 1.0/l : 0.0);
  double u = x/l;
  return (k/l)*std::pow(u, k - 1.0)*std::exp(-std::pow(u, k));
  }

  double WeibullDistribution::cdf(double x) const
  {
  if (!(shape() > 0.0) || !(scale() > 0.0)) return kNaN;
  if (x <= 0.0) return 0.0;
  return -std::expm1(-std::pow(x/scale(), shape()));
  }

  double WeibullDistribution::cdfUpper(double x) const
  {
  if (!(shape() > 0.0) || !(scale() > 0.0)) return kNaN;
  if (x <= 0.0) return 1.0;
  return std::exp(-std::pow(x/scale(), shape()));
  }

  double WeibullDistribution::mean() const
  {
  return scale()*std::tgamma(1.0 + 1.0/shape());
  }

  double WeibullDistribution::variance() const
  {
  double g1 = std::tgamma(1.0 + 1.0/shape()), g2 = std::tgamma(1.0 + 2.0/shape());
  return scale()*scale()*(g2 - g1*g1);
  }

  } // namespace MATH
} // namespace CAP
