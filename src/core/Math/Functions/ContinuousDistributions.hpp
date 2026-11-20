/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file ContinuousDistributions.hpp
//! @brief Probability densities of one variable: StudentDistribution, ChiSquareDistribution, FDistribution,
//! CauchyDistribution, MaxwellBoltzmannDistribution, WeibullDistribution.
#ifndef DATUM_MATH_ContinuousDistributions
#define DATUM_MATH_ContinuousDistributions
#include <memory>
#include <vector>
#include "ParameterizedFunction.hpp"

namespace Datum
{
  namespace MATH
  {

  //!
  //! @brief Probability densities of one variable (base of StudentDistribution, ChiSquareDistribution, FDistribution,
  //! CauchyDistribution, MaxwellBoltzmannDistribution, WeibullDistribution).
  //!
  //! Parameter 0 is the amplitude A, the area under the curve: f(x) = A p(x), p the probability density (A = 1: the
  //! density; A = number of events: the expected number per unit of x). integralInRange() is analytical (from the
  //! cumulative distribution cdf(); infinite bounds allowed). Parameters that must be positive are bounded below by 0
  //! (Parameter::Low); with a parameter out of its domain (e.g. during a fit), f is 0.
  //!
  class ContinuousDistribution : public ParameterizedFunction
  {
  public:

  //! @brief Distribution with parameters (the first one: the amplitude). @param parameters parameters
  explicit ContinuousDistribution(const std::vector<Parameter> & parameters);
  virtual ~ContinuousDistribution() { }

  using ParameterizedFunction::evaluateAt;
  using ParameterizedFunction::integralInRange;
  //! @brief f(x) = A p(x). @param x x @return value
  double evaluateAt(double x) const override { return amplitude()*density(x); }
  //! @brief A [cdf(high) - cdf(low)] (with the upper tail when both bounds are in it). @param low lower bound
  //! @param high upper bound @return integral (negative if low > high)
  double integralInRange(double low, double high) const override;
  //! @brief Amplitude (area). @return A
  double amplitude() const { return parameterValue(0); }
  //! @brief Set the amplitude. @param a A
  void setAmplitude(double a) { setParameterValue(0, a); }
  //! @brief Set the amplitude to 1 (f is then the density).
  void setNormalized() { setParameterValue(0, 1.0); }
  //! @brief Probability density at x. @param x x @return p(x)
  virtual double density(double x) const = 0;
  //! @brief Cumulative distribution. @param x x @return P(X <= x)
  virtual double cdf(double x) const = 0;
  //! @brief Upper tail, P(X > x) (precise in the tail where it is computed directly). @param x x @return probability
  virtual double cdfUpper(double x) const { return 1.0 - cdf(x); }
  //! @brief Mean. @return mean (NaN if undefined, infinity if infinite)
  virtual double mean() const = 0;
  //! @brief Variance. @return variance (NaN if undefined, infinity if infinite)
  virtual double variance() const = 0;

  public:
  //! @brief Quantile: x such that cdf(x) = p (by bisection; derived classes may override it). @param p probability
  //! in (0, 1) @return x
  virtual double quantile(double p) const;

  protected:
  //! Without a closed form (hasAnalyticalFourierTransform() false), the transform on the line is that of a piecewise
  //! Legendre approximation of f on the domain holding all but 1e-12 of the probability, made once for the
  //! values of the parameters and kept.
  std::complex<double> lineTransform(double k) const override;
  FourierTransform * newLineTransform() const override;
  mutable std::shared_ptr<const void> _fourierCache;   //!< approximation of the transform (FourierTools::PiecewiseLegendre)
  mutable std::vector<double> _fourierCacheKey;   //!< parameters of the approximation
  };

  //!
  //! @brief Student t distribution of nu degrees of freedom, location mu and scale sigma: with t = (x - mu)/sigma,
  //! p(x) = Gamma((nu + 1)/2)/(sqrt(nu pi) Gamma(nu/2) sigma) (1 + t^2/nu)^(-(nu + 1)/2). Parameters: 0 amplitude,
  //! 1 ndf nu (> 0, not necessarily an integer), 2 location mu, 3 scale sigma (> 0). nu = 1: Cauchy; normal as nu goes
  //! to infinity. Mean mu (nu > 1), variance sigma^2 nu/(nu - 2) (nu > 2).
  //!
  class StudentDistribution : public ContinuousDistribution
  {
  public:
  //! @brief Distribution. @param amplitude A @param ndf nu (> 0) @param location mu @param scale sigma (> 0)
  //! @throws MathException if nu <= 0 or sigma <= 0
  StudentDistribution(double amplitude=1.0, double ndf=1.0, double location=0.0, double scale=1.0);
  StudentDistribution * clone() const override { return new StudentDistribution(*this); }
  //! @brief Degrees of freedom. @return nu
  double ndf() const { return parameterValue(1); }
  //! @brief Location. @return mu
  double location() const { return parameterValue(2); }
  //! @brief Scale. @return sigma
  double scale() const { return parameterValue(3); }
  double density(double x) const override;
  double cdf(double x) const override;
  double cdfUpper(double x) const override;
  double mean() const override;
  double variance() const override;

  public:
  //! @brief Transform: A e^{-ik mu} z^(nu/2) K_(nu/2)(z)/(Gamma(nu/2) 2^(nu/2 - 1)), z = sqrt(nu) sigma |k|. @return true
  bool hasAnalyticalFourierTransform() const override { return true; }

  protected:
  std::complex<double> lineTransform(double k) const override;
  };

  //!
  //! @brief Chi square distribution of k degrees of freedom: p(x) = x^(k/2 - 1) e^(-x/2)/(2^(k/2) Gamma(k/2)), x >= 0.
  //! Parameters: 0 amplitude, 1 ndf k (> 0, not necessarily an integer). Mean k, variance 2 k, mode max(k - 2, 0).
  //! (The chi square of a fit is ChiSquare.)
  //!
  class ChiSquareDistribution : public ContinuousDistribution
  {
  public:
  //! @brief Distribution. @param amplitude A @param ndf k (> 0) @throws MathException if k <= 0
  ChiSquareDistribution(double amplitude=1.0, double ndf=1.0);
  ChiSquareDistribution * clone() const override { return new ChiSquareDistribution(*this); }
  //! @brief Degrees of freedom. @return k
  double ndf() const { return parameterValue(1); }
  //! @brief Mode. @return max(k - 2, 0)
  double mode() const { return ndf() > 2.0 ? ndf() - 2.0 : 0.0; }
  double density(double x) const override;
  double cdf(double x) const override;
  double cdfUpper(double x) const override;
  double mean() const override { return ndf(); }
  double variance() const override { return 2.0*ndf(); }

  public:
  //! @brief Transform: A (1 + 2ik)^(-nu/2). @return true
  bool hasAnalyticalFourierTransform() const override { return true; }

  protected:
  std::complex<double> lineTransform(double k) const override;
  };

  //!
  //! @brief F distribution (Fisher-Snedecor) of d1 and d2 degrees of freedom, the distribution of (X1/d1)/(X2/d2) for
  //! X1, X2 chi square of d1, d2 degrees of freedom: p(x) = sqrt((d1 x)^d1 d2^d2/(d1 x + d2)^(d1 + d2))/(x B(d1/2,
  //! d2/2)), x >= 0. Parameters: 0 amplitude, 1 d1 (> 0), 2 d2 (> 0). Mean d2/(d2 - 2) (d2 > 2), variance
  //! 2 d2^2 (d1 + d2 - 2)/(d1 (d2 - 2)^2 (d2 - 4)) (d2 > 4).
  //!
  class FDistribution : public ContinuousDistribution
  {
  public:
  //! @brief Distribution. @param amplitude A @param d1 d1 (> 0) @param d2 d2 (> 0)
  //! @throws MathException if d1 <= 0 or d2 <= 0
  FDistribution(double amplitude=1.0, double d1=5.0, double d2=10.0);
  FDistribution * clone() const override { return new FDistribution(*this); }
  //! @brief Degrees of freedom of the numerator. @return d1
  double d1() const { return parameterValue(1); }
  //! @brief Degrees of freedom of the denominator. @return d2
  double d2() const { return parameterValue(2); }
  double density(double x) const override;
  double cdf(double x) const override;
  double cdfUpper(double x) const override;
  double mean() const override;
  double variance() const override;
  };

  //!
  //! @brief Cauchy distribution (Breit-Wigner, Lorentzian) of location x0 and half width at half maximum gamma:
  //! p(x) = gamma/(pi ((x - x0)^2 + gamma^2)). Parameters: 0 amplitude, 1 location x0, 2 gamma (> 0). Median and mode
  //! x0, full width at half maximum 2 gamma; no mean, no variance (NaN). Analytical derivative.
  //!
  class CauchyDistribution : public ContinuousDistribution
  {
  public:
  //! @brief Distribution. @param amplitude A @param location x0 @param gamma half width (> 0)
  //! @throws MathException if gamma <= 0
  CauchyDistribution(double amplitude=1.0, double location=0.0, double gamma=1.0);
  CauchyDistribution * clone() const override { return new CauchyDistribution(*this); }
  //! @brief Location (median, mode). @return x0
  double location() const { return parameterValue(1); }
  //! @brief Half width at half maximum. @return gamma
  double gamma() const { return parameterValue(2); }
  //! @brief Derivative. @param x x @return f'(x)
  double derivativeAt(double x) const override;
  //! @brief The derivative is analytical. @return true
  bool hasAnalyticalDerivative() const override { return true; }
  double density(double x) const override;
  double cdf(double x) const override;
  double cdfUpper(double x) const override;
  double mean() const override;
  double variance() const override;

  public:
  //! @brief Transform: A e^{-ik x0} e^{-gamma |k|}. @return true
  bool hasAnalyticalFourierTransform() const override { return true; }

  protected:
  std::complex<double> lineTransform(double k) const override;
  };

  //!
  //! @brief Maxwell-Boltzmann distribution of the speed v of the particles of an ideal gas, of scale a = sqrt(kT/m):
  //! p(v) = sqrt(2/pi) v^2 e^(-v^2/(2 a^2))/a^3, v >= 0. Parameters: 0 amplitude, 1 a (> 0). Mean 2 a sqrt(2/pi),
  //! variance a^2 (3 pi - 8)/pi, mode sqrt(2) a.
  //!
  class MaxwellBoltzmannDistribution : public ContinuousDistribution
  {
  public:
  //! @brief Distribution. @param amplitude A @param scale a (> 0) @throws MathException if a <= 0
  MaxwellBoltzmannDistribution(double amplitude=1.0, double scale=1.0);
  MaxwellBoltzmannDistribution * clone() const override { return new MaxwellBoltzmannDistribution(*this); }
  //! @brief Scale. @return a
  double scale() const { return parameterValue(1); }
  //! @brief Mode. @return sqrt(2) a
  double mode() const;
  double density(double x) const override;
  double cdf(double x) const override;
  double cdfUpper(double x) const override;
  double mean() const override;
  double variance() const override;

  public:
  //! @brief Transform: with the Faddeeva function (FourierTools::halfGaussianMoment()). @return true
  bool hasAnalyticalFourierTransform() const override { return true; }

  protected:
  std::complex<double> lineTransform(double k) const override;
  };

  //!
  //! @brief Weibull distribution of shape k and scale lambda: p(x) = (k/lambda) (x/lambda)^(k - 1) e^(-(x/lambda)^k),
  //! x >= 0. Parameters: 0 amplitude, 1 shape k (> 0), 2 scale lambda (> 0). k = 1: exponential; k = 2: Rayleigh.
  //! Mean lambda Gamma(1 + 1/k), variance lambda^2 [Gamma(1 + 2/k) - Gamma(1 + 1/k)^2].
  //!
  class WeibullDistribution : public ContinuousDistribution
  {
  public:
  //! @brief Distribution. @param amplitude A @param shape k (> 0) @param scale lambda (> 0)
  //! @throws MathException if k <= 0 or lambda <= 0
  WeibullDistribution(double amplitude=1.0, double shape=1.5, double scale=1.0);
  WeibullDistribution * clone() const override { return new WeibullDistribution(*this); }
  //! @brief Shape. @return k
  double shape() const { return parameterValue(1); }
  //! @brief Scale. @return lambda
  double scale() const { return parameterValue(2); }
  double density(double x) const override;
  double cdf(double x) const override;
  double cdfUpper(double x) const override;
  double mean() const override;
  double variance() const override;

  public:
  //! @brief Transform in closed form for the shapes 1 (exponential: A/(1 + ik lambda)) and 2 (Rayleigh, with the
  //! Faddeeva function); approximated otherwise. @return true for the shapes 1 and 2
  bool hasAnalyticalFourierTransform() const override { return shape() == 1.0 || shape() == 2.0; }

  protected:
  std::complex<double> lineTransform(double k) const override;
  };

  //!
  //! @brief Gamma distribution of shape alpha and scale theta: p(x) = x^(alpha - 1) e^(-x/theta)/(Gamma(alpha)
  //! theta^alpha), x >= 0. Parameters: 0 amplitude, 1 shape alpha (> 0), 2 scale theta (> 0). Mean alpha theta, variance
  //! alpha theta^2. alpha = 1: exponential; alpha = nu/2, theta = 2: chi square of nu degrees of freedom. Fourier
  //! transform: A (1 + ik theta)^(-alpha).
  //!
  class GammaDistribution : public ContinuousDistribution
  {
  public:
  //! @brief Distribution. @param amplitude A @param shape alpha (> 0) @param scale theta (> 0)
  //! @throws MathException if alpha or theta is not > 0
  GammaDistribution(double amplitude=1.0, double shape=2.0, double scale=1.0);
  GammaDistribution * clone() const override { return new GammaDistribution(*this); }   //!< Copy
  double shape() const { return parameterValue(1); }   //!< Shape alpha
  double scale() const { return parameterValue(2); }   //!< Scale theta
  double density(double x) const override;   //!< p(x)
  double cdf(double x) const override;   //!< P(alpha, x/theta), regularized incomplete gamma function
  double cdfUpper(double x) const override;   //!< Q(alpha, x/theta)
  double mean() const override { return shape()*scale(); }   //!< alpha theta
  double variance() const override { return shape()*scale()*scale(); }   //!< alpha theta^2
  //! @brief Transform: A (1 + ik theta)^(-alpha). @return true
  bool hasAnalyticalFourierTransform() const override { return true; }

  protected:
  std::complex<double> lineTransform(double k) const override;
  };

  } // namespace MATH
} // namespace Datum

#endif // DATUM_MATH_ContinuousDistributions
