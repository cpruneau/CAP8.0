/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FunctionTransforms.cpp
//! @brief The Fourier transforms of the functions of Datum (declared with each class; see docs/Fourier.md for the
//! formulas): Exponential, Normal, Normal2D, LogNormal, the continuous and discrete distributions, the Fourier
//! harmonics and decompositions, the Bessel functions, Bezier curves, composite functions.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <memory>
#include "Bessel.hpp"
#include "BesselFunction.hpp"
#include "BezierCurve.hpp"
#include "CompositeFunction.hpp"
#include "ContinuousDistributions.hpp"
#include "DiscreteDistributions.hpp"
#include "Distributions.hpp"
#include "Exponential.hpp"
#include "FourierDecomposition.hpp"
#include "FourierTools.hpp"
#include "FourierTransform.hpp"
#include "LogNormal.hpp"
#include "MathExceptions.hpp"
#include "Normal.hpp"
#include "Normal2D.hpp"
#include "ParameterizedComposite.hpp"

namespace Datum
{
  namespace MATH
  {

  namespace
  {
  using Complex = std::complex<double>;
  const double kPi = 3.14159265358979323846;
  const double kInfinity = std::numeric_limits<double>::infinity();
  const Complex kI(0.0, 1.0);

  Complex phaseAt(double k, double x0) { return std::exp(Complex(0.0, -k*x0)); }

  //! Probabilities at which a distribution is cut into segments: geometric in the tails, uniform in between.
  std::vector<double> segmentProbabilities()
  {
  std::vector<double> p;
  for (double t : { 1e-12, 1e-10, 1e-8, 1e-6, 1e-5, 1e-4, 1e-3, 1e-2 }) p.push_back(t);
  for (int i=1; i<24; i++) p.push_back(i/24.0);
  for (double t : { 1e-2, 1e-3, 1e-4, 1e-5, 1e-6, 1e-8, 1e-10, 1e-12 }) p.push_back(1.0 - t);
  return p;
  }

  //! Edges of segments at the quantiles of the probabilities above.
  std::vector<double> quantileEdges(const std::function<double(double)> & quantile)
  {
  std::vector<double> edges;
  for (double p : segmentProbabilities())
    {
    double x = quantile(p);
    if (std::isfinite(x) && (edges.empty() || x > edges.back())) edges.push_back(x);
    }
  if (edges.size() < 2) throw MathException("Cannot find a domain for the transform", __FUNCTION__);
  return edges;
  }

  String describe(const FourierTools::PiecewiseLegendre & p)
  {
  char text[200];
  std::snprintf(text, sizeof(text), "piecewise Legendre approximation (%u segments on [%g, %g], degrees up to %u, neglected "
                "coefficients %.1e of the largest); all but 1e-12 of the area", (unsigned int)(p.coefficients.size()),
                p.edges.front(), p.edges.back(), p.largestDegree, p.tail);
  return String(text);
  }

  //! e^{inx} terms of a c cos(nx) + s sin(nx): ((c - is)/2) e^{inx} + ((c + is)/2) e^{-inx}, on [low, high].
  Complex harmonicDomain(double n, double c, double s, double k, double low, double high)
  {
  Complex plus  = 0.5*Complex(c, -s)*FourierTools::exponentialIntegral(Complex(0.0, n - k), low, high);
  Complex minus = 0.5*Complex(c, s)*FourierTools::exponentialIntegral(Complex(0.0, -n - k), low, high);
  return plus + minus;
  }

  //! Transform of J_n(q (x - x0)) or K_0(q (x - x0)) (amplitude 1).
  Complex besselLine(Bessel::BesselType type, unsigned int n, double q, double x0, double k)
  {
  if (q == 0.0) throw MathException("A Bessel function of scale 0 has no transform", __FUNCTION__);
  if (type == Bessel::BesselJ)
    {
    double kappa = k/q;
    if (std::fabs(kappa) > 1.0) return 0.0;
    if (std::fabs(kappa) == 1.0) return kInfinity;
    Complex phase = 1.0;   // (-i)^n
    for (unsigned int m=0; m<n%4; m++) phase *= -kI;
    return 2.0*phase*std::cos(n*std::acos(kappa))/std::sqrt(q*q - k*k)*phaseAt(k, x0);
    }
  if (type == Bessel::BesselK && n == 0)
    return Complex(0.5*kPi, -std::asinh(k/q))/std::sqrt(q*q + k*k)*phaseAt(k, x0);
  throw MathException("No transform in closed form for this Bessel function: give a domain", __FUNCTION__);
  }

  //! Transform of a function of rank 1 given a component: the line transform, or its domain transform.
  std::vector<double> subVector(const std::vector<double> & k, unsigned int first, unsigned int n)
  {
  return std::vector<double>(k.begin() + first, k.begin() + first + n);
  }
  } // namespace

  // ---- Exponential

  Complex Exponential::domainTransform(double k, double low, double high) const
  {
  return amplitude()*FourierTools::exponentialIntegral(Complex(exponent(), -k), low, high);
  }

  // ---- Normal

  Complex Normal::lineTransform(double k) const
  {
  return amplitude()*FourierTools::gaussianLine(k, mean(), width());
  }

  Complex Normal::domainTransform(double k, double low, double high) const
  {
  return amplitude()*FourierTools::gaussianDomain(k, mean(), width(), low, high);
  }

  // ---- Normal2D

  Complex Normal2D::lineTransformND(const std::vector<double> & k) const
  {
  double a = parameterValue(0), mx = parameterValue(1), my = parameterValue(2);
  double sx = parameterValue(3), sy = parameterValue(4), rho = parameterValue(5);
  double kx = k[0], ky = k[1];
  double exponent = -0.5*(sx*sx*kx*kx + 2.0*rho*sx*sy*kx*ky + sy*sy*ky*ky);
  return 2.0*kPi*a*sx*sy*std::sqrt(1.0 - rho*rho)*std::exp(Complex(exponent, -(kx*mx + ky*my)));
  }

  // ---- LogNormal

  namespace
  {
  FourierTools::PiecewiseLegendre logNormalApproximation(const LogNormal & f)
  {
  double mu = f.mean(), sigma = f.width();
  auto quantile = [mu, sigma](double p) { return std::exp(mu + sigma*Distributions::normalQuantile(p)); };
  return FourierTools::PiecewiseLegendre::build([&f](double x) { return f.evaluateAt(x); }, quantileEdges(quantile));
  }
  } // namespace

  Complex LogNormal::lineTransform(double k) const
  {
  return logNormalApproximation(*this).transform(k);
  }

  FourierTransform * LogNormal::newLineTransform() const
  {
  auto p = std::make_shared<FourierTools::PiecewiseLegendre>(logNormalApproximation(*this));
  return new FourierTransform(1, [p](const std::vector<double> & k) { return p->transform(k[0]); },
                              FourierTransform::kApproximation, describe(*p), name());
  }

  // ---- ContinuousDistribution

  double ContinuousDistribution::quantile(double p) const
  {
  if (!(p > 0.0 && p < 1.0)) throw MathException("A quantile needs 0 < p < 1", __FUNCTION__);
  bool upper = p > 0.5;
  double target = upper ? 1.0 - p : p;
  // F(x) - p, increasing; in the upper half, 1 - p - Q(x), with the upper tail for precision
  auto excess = [&](double x) { return upper ? target - cdfUpper(x) : cdf(x) - target; };
  double lo = -1.0, hi = 1.0;
  for (int i=0; i<2000 && excess(lo) > 0.0; i++) lo *= 2.0;
  for (int i=0; i<2000 && excess(hi) < 0.0; i++) hi *= 2.0;
  for (int i=0; i<300; i++)
    {
    double mid = 0.5*(lo + hi);
    if (mid == lo || mid == hi) break;
    if (excess(mid) < 0.0) lo = mid; else hi = mid;
    }
  return 0.5*(lo + hi);
  }

  namespace
  {
  std::shared_ptr<const FourierTools::PiecewiseLegendre> distributionApproximation(const ContinuousDistribution & d)
  {
  auto quantile = [&d](double p) { return d.quantile(p); };
  return std::make_shared<FourierTools::PiecewiseLegendre>(
    FourierTools::PiecewiseLegendre::build([&d](double x) { return d.evaluateAt(x); }, quantileEdges(quantile)));
  }
  } // namespace

  Complex ContinuousDistribution::lineTransform(double k) const
  {
  std::vector<double> key = parameterValues();
  if (!_fourierCache || key != _fourierCacheKey)
    {
    _fourierCache = distributionApproximation(*this);
    _fourierCacheKey = key;
    }
  return std::static_pointer_cast<const FourierTools::PiecewiseLegendre>(_fourierCache)->transform(k);
  }

  FourierTransform * ContinuousDistribution::newLineTransform() const
  {
  if (hasAnalyticalFourierTransform()) return Function::newLineTransform();
  std::shared_ptr<const FourierTools::PiecewiseLegendre> p = distributionApproximation(*this);
  return new FourierTransform(1, [p](const std::vector<double> & k) { return p->transform(k[0]); },
                              FourierTransform::kApproximation, describe(*p), name());
  }

  Complex StudentDistribution::lineTransform(double k) const
  {
  double nu = ndf();
  return amplitude()*FourierTools::besselKNormalized(0.5*nu, std::sqrt(nu)*scale()*std::fabs(k))*phaseAt(k, location());
  }

  Complex ChiSquareDistribution::lineTransform(double k) const
  {
  return amplitude()*std::pow(Complex(1.0, 2.0*k), -0.5*ndf());
  }

  Complex CauchyDistribution::lineTransform(double k) const
  {
  return amplitude()*std::exp(-gamma()*std::fabs(k))*phaseAt(k, location());
  }

  Complex MaxwellBoltzmannDistribution::lineTransform(double k) const
  {
  double a = scale();
  return amplitude()*std::sqrt(2.0/kPi)/(a*a*a)*FourierTools::halfGaussianMoment(2, k, a);
  }

  Complex WeibullDistribution::lineTransform(double k) const
  {
  double lambda = scale();
  if (shape() == 1.0) return amplitude()/Complex(1.0, k*lambda);
  if (shape() == 2.0) return amplitude()*2.0/(lambda*lambda)*FourierTools::halfGaussianMoment(1, k, lambda/std::sqrt(2.0));
  return ContinuousDistribution::lineTransform(k);
  }

  Complex GammaDistribution::lineTransform(double k) const
  {
  return amplitude()*std::pow(Complex(1.0, k*scale()), -shape());
  }

  // ---- DiscreteDistribution

  Complex DiscreteDistribution::lineTransform(double k) const
  {
  Complex sum = 0.0;
  double total = 0.0, average = mean();
  for (unsigned long m=0; m<100000000UL; m++)
    {
    if (!inSupport(double(m))) { if (double(m) > average) break; continue; }
    double p = probability(m);
    sum += p*std::exp(Complex(0.0, -k*double(m)));
    total += p;
    if (total >= 1.0 - 1e-16 && double(m) > average) break;
    }
  return amplitude()*sum;
  }

  Complex DiscreteDistribution::domainTransform(double k, double low, double high) const
  {
  if (std::isinf(high) && std::isinf(low)) return lineTransform(k);
  double first = std::max(0.0, std::ceil(low));
  Complex sum = 0.0;
  double total = 0.0, average = mean();
  for (double m=first; m<=high && m<1e8; m+=1.0)
    {
    if (!inSupport(m)) { if (m > average) break; continue; }
    double p = probability((unsigned long)(m));
    double weight = (m == low || m == high) ? 0.5 : 1.0;   // an integer on a bound counts half
    sum += weight*p*std::exp(Complex(0.0, -k*m));
    total += p;
    if (std::isinf(high) && total >= 1.0 - 1e-16 && m > average) break;
    }
  return amplitude()*sum;
  }

  Complex PoissonDistribution::lineTransform(double k) const
  {
  return amplitude()*std::exp(mean()*(std::exp(Complex(0.0, -k)) - 1.0));
  }

  Complex BinomialDistribution::lineTransform(double k) const
  {
  double p = successProbability();
  return amplitude()*std::pow(1.0 - p + p*std::exp(Complex(0.0, -k)), trials());
  }

  Complex NegativeBinomialDistribution::lineTransform(double k) const
  {
  double r = shape();
  return amplitude()*std::pow(1.0 + (mean()/r)*(1.0 - std::exp(Complex(0.0, -k))), -r);
  }

  // ---- Fourier harmonics and decompositions

  Complex FourierHarmonic::domainTransform(double k, double low, double high) const
  {
  double n = _n, a = parameterValue(0), b = parameterValue(1);
  if (_form == kFlow) return harmonicDomain(n, a*std::cos(n*b), a*std::sin(n*b), k, low, high);   // v cos(n(x - psi))
  return harmonicDomain(n, a, b, k, low, high);
  }

  Complex FourierDecomposition::domainTransform(double k, double low, double high) const
  {
  Complex sum = constant()*FourierTools::exponentialIntegral(Complex(0.0, -k), low, high);
  for (unsigned int n=1; n<=nMax(); n++) sum += harmonicDomain(n, cosine(n), sine(n), k, low, high);
  return sum;
  }

  // ---- Bessel functions

  bool BesselFunction::hasAnalyticalFourierTransform() const
  {
  return _type == Bessel::BesselJ || (_type == Bessel::BesselK && order() == 0);
  }

  Complex BesselFunction::lineTransform(double k) const
  {
  return parameterValue(0)*besselLine(_type, order(), parameterValue(1), parameterValue(2), k);
  }

  bool Bessel::hasAnalyticalFourierTransform() const
  {
  return _type == BesselJ || (_type == BesselK && _order == 0);
  }

  Complex Bessel::lineTransform(double k) const
  {
  return besselLine(_type, _order, 1.0, 0.0, k);
  }

  // ---- BezierCurve

  Complex BezierCurve::domainTransform(double k, double low, double high) const
  {
  update();
  const std::vector<double> & x = _x;
  auto f = [this](double t) { return evaluateAt(t); };
  Complex sum = 0.0;
  // constant before the first knot and after the last one, a cubic between knots
  auto add = [&](double from, double to, unsigned int degree)
    {
    double lo = std::max(low, from), hi = std::min(high, to);
    if (!(hi > lo)) return;
    if (degree == 0)
      {
      double value = std::isinf(from) ? evaluateAt(to) : evaluateAt(from);
      sum += value*FourierTools::exponentialIntegral(Complex(0.0, -k), lo, hi);
      }
    else
      sum += FourierTools::polynomialTransform(f, degree, k, lo, hi);
    };
  add(-kInfinity, x.front(), 0);
  for (std::size_t s=0; s+1<x.size(); s++) add(x[s], x[s+1], 3);
  add(x.back(), kInfinity, 0);
  return sum;
  }

  // ---- CompositeFunction

  bool CompositeFunction::hasAnalyticalFourierTransform() const
  {
  if (_functions.empty() || _operation == kPointwiseProduct) return false;
  for (const Function * f : _functions) if (!f->hasAnalyticalFourierTransform()) return false;
  return true;
  }

  bool CompositeFunction::hasAnalyticalFourierTransformOnDomain() const
  {
  if (_functions.empty() || _operation != kSum || rank() != 1) return false;
  for (const Function * f : _functions) if (!f->hasAnalyticalFourierTransformOnDomain()) return false;
  return true;
  }

  Complex CompositeFunction::lineTransformND(const std::vector<double> & k) const
  {
  if (_operation == kPointwiseProduct)
    throw MathException("No transform on the line of a pointwise product: give a domain", __FUNCTION__);
  if (_operation == kSum)
    {
    Complex sum = 0.0;
    for (const Function * f : _functions) sum += f->fourierTransformAt(k);
    return sum;
    }
  Complex product = 1.0;
  unsigned int first = 0;
  for (const Function * f : _functions)
    {
    product *= f->fourierTransformAt(subVector(k, first, f->rank()));
    first += f->rank();
    }
  return product;
  }

  Complex CompositeFunction::domainTransform(double k, double low, double high) const
  {
  if (_operation == kSum)
    {
    Complex sum = 0.0;
    for (const Function * f : _functions) sum += f->fourierTransformAt(k, low, high);
    return sum;
    }
  return Function::domainTransform(k, low, high);
  }

  FourierTransform * CompositeFunction::newLineTransform() const
  {
  if (!hasAnalyticalFourierTransform()) return Function::newLineTransform();   // throws
  std::vector<std::shared_ptr<const FourierTransform>> parts;
  for (const Function * f : _functions) parts.emplace_back(f->fourierTransform());
  bool sum = _operation == kSum;
  return new FourierTransform(rank(), [parts, sum](const std::vector<double> & k)
    {
    Complex value = sum ? 0.0 : 1.0;
    unsigned int first = 0;
    for (const auto & t : parts)
      {
      if (sum) value += t->valueAt(k);
      else { value *= t->valueAt(subVector(k, first, t->rank())); first += t->rank(); }
      }
    return value;
    }, FourierTransform::kClosedForm, sum ? "closed form (sum)" : "closed form (product)", name());
  }

  FourierTransform * CompositeFunction::newDomainTransform(double low, double high) const
  {
  if (_operation != kSum) return approximateDomainTransform(low, high);
  std::vector<std::shared_ptr<const FourierTransform>> parts;
  bool exact = true;
  for (const Function * f : _functions)
    {
    parts.emplace_back(f->fourierTransform(low, high));
    exact = exact && parts.back()->isAnalytical();
    }
  FourierTransform * t = new FourierTransform(1, [parts](const std::vector<double> & k)
    {
    Complex value = 0.0;
    for (const auto & p : parts) value += p->valueAt(k);
    return value;
    }, exact ? FourierTransform::kExactOnDomain : FourierTransform::kApproximation,
    exact ? "exact on the domain (sum)" : "sum of transforms on the domain, some approximated", name());
  t->setDomain(low, high);
  return t;
  }

  // ---- ParameterizedComposite

  bool ParameterizedComposite::hasAnalyticalFourierTransform() const
  {
  if (_components.empty()) return false;
  for (const auto & c : _components) if (!c->hasAnalyticalFourierTransform()) return false;
  return true;
  }

  bool ParameterizedComposite::hasAnalyticalFourierTransformOnDomain() const
  {
  if (_components.empty() || _operation != kSum || rank() != 1) return false;
  for (const auto & c : _components) if (!c->hasAnalyticalFourierTransformOnDomain()) return false;
  return true;
  }

  Complex ParameterizedComposite::lineTransformND(const std::vector<double> & k) const
  {
  synchronize();
  if (_operation == kSum)
    {
    Complex sum = 0.0;
    for (const auto & c : _components) sum += c->fourierTransformAt(k);
    return sum;
    }
  Complex product = 1.0;
  for (std::size_t i=0; i<_components.size(); i++)
    product *= _components[i]->fourierTransformAt(subVector(k, _offsets[i], _components[i]->rank()));
  return product;
  }

  Complex ParameterizedComposite::domainTransform(double k, double low, double high) const
  {
  synchronize();
  if (_operation == kSum)
    {
    Complex sum = 0.0;
    for (const auto & c : _components) sum += c->fourierTransformAt(k, low, high);
    return sum;
    }
  return Function::domainTransform(k, low, high);
  }

  } // namespace MATH
} // namespace Datum
