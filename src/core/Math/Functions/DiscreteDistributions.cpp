/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file DiscreteDistributions.cpp
//! @brief Implementation of the distributions of counts.
#include <algorithm>
#include <cmath>
#include <limits>
#include "DiscreteDistributions.hpp"
#include "Distributions.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  namespace
  {
  const double kMinusInfinity = -std::numeric_limits<double>::infinity();

  //! x ln y, 0 if x = 0 (0^0 = 1)
  double xLogY(double x, double y) { return x == 0.0 ? 0.0 : x*std::log(y); }

  //! ln C(n, k) = ln Gamma(n + 1) - ln Gamma(k + 1) - ln Gamma(n - k + 1)
  double logBinomial(double n, double k) { return std::lgamma(n + 1.0) - std::lgamma(k + 1.0) - std::lgamma(n - k + 1.0); }

  Parameter positive(const char * name, double value)
  {
  return Parameter(name, value, Parameter::Low, 0.0);
  }

  Parameter fixed(const char * name, double value)
  {
  Parameter p(name, value, Parameter::Low, 0.0);
  p.setType(Parameter::Fixed);
  return p;
  }
  } // namespace

  // ---- DiscreteDistribution

  DiscreteDistribution::DiscreteDistribution(const std::vector<Parameter> & parameters)
  :
  ParameterizedFunction(parameters)
  { }

  double DiscreteDistribution::evaluateAt(double x) const
  {
  if (!inSupport(x)) return 0.0;
  double l = logProbability(x);
  return l == kMinusInfinity ? 0.0 : amplitude()*std::exp(l);
  }

  double DiscreteDistribution::probability(unsigned long k) const
  {
  double x = double(k);
  if (!inSupport(x)) return 0.0;
  double l = logProbability(x);
  return l == kMinusInfinity ? 0.0 : std::exp(l);
  }

  double DiscreteDistribution::cdf(unsigned long k) const
  {
  double sum = 0.0;
  for (unsigned long j=0; j<=k; j++) sum += probability(j);
  return std::min(sum, 1.0);
  }

  // ---- PoissonDistribution

  PoissonDistribution::PoissonDistribution(double amplitude, double mean)
  :
  DiscreteDistribution({Parameter("amplitude", amplitude), positive("mean", mean)})
  {
  if (mean < 0.0) throw MathException("The mean of a Poisson distribution is >= 0", __FUNCTION__);
  }

  double PoissonDistribution::logProbability(double x) const
  {
  double mu = parameterValue(1);
  if (mu < 0.0) return kMinusInfinity;
  if (mu == 0.0) return x == 0.0 ? 0.0 : kMinusInfinity;
  return x*std::log(mu) - mu - std::lgamma(x + 1.0);
  }

  double PoissonDistribution::cdf(unsigned long k) const
  {
  return Distributions::poissonCdf(k, mean());
  }

  // ---- BinomialDistribution

  BinomialDistribution::BinomialDistribution(double amplitude, double n, double p)
  :
  DiscreteDistribution({Parameter("amplitude", amplitude), fixed("n", n),
                        Parameter("p", p, Parameter::LowHigh, 0.0, 1.0)})
  {
  if (n < 0.0) throw MathException("The number of trials of a binomial distribution is >= 0", __FUNCTION__);
  if (p < 0.0 || p > 1.0) throw MathException("The probability of a binomial distribution is in [0, 1]", __FUNCTION__);
  }

  double BinomialDistribution::logProbability(double x) const
  {
  double n = trials(), p = successProbability();
  if (p < 0.0 || p > 1.0) return kMinusInfinity;
  if ((p == 0.0 && x > 0.0) || (p == 1.0 && x < n)) return kMinusInfinity;
  return logBinomial(n, x) + xLogY(x, p) + xLogY(n - x, 1.0 - p);
  }

  // ---- NegativeBinomialDistribution

  NegativeBinomialDistribution::NegativeBinomialDistribution(double amplitude, double mean, double k)
  :
  DiscreteDistribution({Parameter("amplitude", amplitude), positive("mean", mean), positive("k", k)})
  {
  if (mean < 0.0) throw MathException("The mean of a negative binomial distribution is >= 0", __FUNCTION__);
  if (!(k > 0.0)) throw MathException("The shape k of a negative binomial distribution is > 0", __FUNCTION__);
  }

  double NegativeBinomialDistribution::logProbability(double x) const
  {
  double mu = mean(), k = shape();
  if (mu < 0.0 || !(k > 0.0)) return kMinusInfinity;
  if (mu == 0.0) return x == 0.0 ? 0.0 : kMinusInfinity;
  return std::lgamma(x + k) - std::lgamma(k) - std::lgamma(x + 1.0) + x*std::log(mu/(mu + k)) + k*std::log(k/(mu + k));
  }

  double NegativeBinomialDistribution::cdf(unsigned long n) const
  {
  if (mean() == 0.0) return 1.0;
  return Distributions::regularizedBeta(shape(), double(n) + 1.0, shape()/(shape() + mean()));
  }

  // ---- HypergeometricDistribution

  HypergeometricDistribution::HypergeometricDistribution(double amplitude, double population, double successes,
                                                         double draws)
  :
  DiscreteDistribution({Parameter("amplitude", amplitude), fixed("N", population), fixed("K", successes),
                        fixed("n", draws)})
  {
  if (successes < 0.0 || successes > population || draws < 0.0 || draws > population)
    throw MathException("Hypergeometric distribution: K and n are within [0, N]", __FUNCTION__);
  }

  bool HypergeometricDistribution::inSupport(double x) const
  {
  double N = population(), K = successes(), n = draws();
  return x >= std::max(0.0, n + K - N) && x <= std::min(n, K);
  }

  double HypergeometricDistribution::logProbability(double x) const
  {
  double N = population(), K = successes(), n = draws();
  return logBinomial(K, x) + logBinomial(N - K, n - x) - logBinomial(N, n);
  }

  double HypergeometricDistribution::variance() const
  {
  double N = population(), q = successes()/N, n = draws();
  return N > 1.0 ? n*q*(1.0 - q)*(N - n)/(N - 1.0) : 0.0;
  }

  } // namespace MATH
} // namespace CAP
