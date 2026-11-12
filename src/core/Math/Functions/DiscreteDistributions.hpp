/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file DiscreteDistributions.hpp
//! @brief Distributions of counts: PoissonDistribution, BinomialDistribution, NegativeBinomialDistribution,
//! HypergeometricDistribution.
#ifndef CAP_MATH_DiscreteDistributions
#define CAP_MATH_DiscreteDistributions
#include "ParameterizedFunction.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Distributions of counts k = 0, 1, 2, ... (base of PoissonDistribution, BinomialDistribution,
  //! NegativeBinomialDistribution, HypergeometricDistribution).
  //!
  //! Parameter 0 is the amplitude A: f(k) = A P(k), where P(k) is the probability of k, so that the sum of f over all
  //! the counts is A (A = 1: the probabilities; A = number of events: the expected number of events with k). Between
  //! the integers, f is the continuation of P(k) given by the gamma function (k! = Gamma(k + 1), ...): smooth in k and
  //! in the parameters, as needed to fit a histogram of counts with bins of width 1 centered on the integers (compare
  //! the bins to the value at their centers, not to the integral over them). f is 0 outside the support.
  //! probability() and cdf() are those of the counts (integers).
  //!
  class DiscreteDistribution : public ParameterizedFunction
  {
  public:

  //! @brief Distribution with parameters (the first one: the amplitude). @param parameters parameters
  explicit DiscreteDistribution(const std::vector<Parameter> & parameters);
  virtual ~DiscreteDistribution() { }

  using ParameterizedFunction::evaluateAt;
  //! @brief f(x) = A P(x), P continued between the integers. @param x count @return value (0 outside the support)
  double evaluateAt(double x) const override;
  //! @brief Amplitude. @return A
  double amplitude() const { return parameterValue(0); }
  //! @brief Set the amplitude. @param a A
  void setAmplitude(double a) { setParameterValue(0, a); }
  //! @brief Set the amplitude to 1 (f is then the probability).
  void setNormalized() { setParameterValue(0, 1.0); }
  //! @brief Probability of k. @param k count @return P(k)
  double probability(unsigned long k) const;
  //! @brief Probability of at most k. @param k count @return P(K <= k)
  virtual double cdf(unsigned long k) const;
  //! @brief Mean of the counts. @return mean
  virtual double mean() const = 0;
  //! @brief Variance of the counts. @return variance
  virtual double variance() const = 0;

  protected:

  //! @brief ln P(x) for x within the support (continued between the integers). @param x count @return ln P(x),
  //! -infinity if P(x) = 0
  virtual double logProbability(double x) const = 0;
  //! @brief Whether x is within the support. @param x count @return true if so
  virtual bool inSupport(double x) const = 0;
  };

  //!
  //! @brief Poisson distribution of mean mu: P(k) = mu^k e^(-mu)/k!. Parameters: 0 amplitude, 1 mean (>= 0).
  //! Mean and variance mu.
  //!
  class PoissonDistribution : public DiscreteDistribution
  {
  public:
  //! @brief Distribution. @param amplitude A @param mean mu (>= 0) @throws MathException if mu < 0
  PoissonDistribution(double amplitude=1.0, double mean=1.0);
  PoissonDistribution * clone() const override { return new PoissonDistribution(*this); }
  //! @brief Mean mu. @return mu
  double mean() const override { return parameterValue(1); }
  //! @brief Variance. @return mu
  double variance() const override { return parameterValue(1); }
  //! @brief P(K <= k). @param k count @return probability
  double cdf(unsigned long k) const override;
  protected:
  double logProbability(double x) const override;
  bool inSupport(double x) const override { return x >= 0.0; }
  };

  //!
  //! @brief Binomial distribution of n trials of probability p: P(k) = C(n, k) p^k (1 - p)^(n - k), k = 0 to n.
  //! Parameters: 0 amplitude, 1 n (trials, fixed by default), 2 p (in [0, 1]). Mean n p, variance n p (1 - p).
  //!
  class BinomialDistribution : public DiscreteDistribution
  {
  public:
  //! @brief Distribution. @param amplitude A @param n trials (>= 0) @param p probability (in [0, 1])
  //! @throws MathException if n < 0 or p is not in [0, 1]
  BinomialDistribution(double amplitude=1.0, double n=10.0, double p=0.5);
  BinomialDistribution * clone() const override { return new BinomialDistribution(*this); }
  //! @brief Trials. @return n
  double trials() const { return parameterValue(1); }
  //! @brief Probability of a success. @return p
  double successProbability() const { return parameterValue(2); }
  //! @brief Mean. @return n p
  double mean() const override { return trials()*successProbability(); }
  //! @brief Variance. @return n p (1 - p)
  double variance() const override { return trials()*successProbability()*(1.0 - successProbability()); }
  protected:
  double logProbability(double x) const override;
  bool inSupport(double x) const override { return x >= 0.0 && x <= trials(); }
  };

  //!
  //! @brief Negative binomial distribution of mean mu and shape k (as used for multiplicities):
  //! P(n) = Gamma(n + k)/(Gamma(k) n!) (mu/(mu + k))^n (k/(mu + k))^k. In terms of r failures and a probability p of
  //! success: r = k, p = k/(k + mu). Parameters: 0 amplitude, 1 mean mu (>= 0), 2 k (> 0). Mean mu, variance
  //! mu + mu^2/k; Poisson of mean mu as k goes to infinity.
  //!
  class NegativeBinomialDistribution : public DiscreteDistribution
  {
  public:
  //! @brief Distribution. @param amplitude A @param mean mu (>= 0) @param k shape (> 0)
  //! @throws MathException if mu < 0 or k <= 0
  NegativeBinomialDistribution(double amplitude=1.0, double mean=5.0, double k=2.0);
  NegativeBinomialDistribution * clone() const override { return new NegativeBinomialDistribution(*this); }
  //! @brief Mean. @return mu
  double mean() const override { return parameterValue(1); }
  //! @brief Shape. @return k
  double shape() const { return parameterValue(2); }
  //! @brief Variance. @return mu + mu^2/k
  double variance() const override { return mean() + mean()*mean()/shape(); }
  //! @brief P(N <= n) = I_p(k, n + 1), p = k/(k + mu). @param n count @return probability
  double cdf(unsigned long n) const override;
  protected:
  double logProbability(double x) const override;
  bool inSupport(double x) const override { return x >= 0.0; }
  };

  //!
  //! @brief Hypergeometric distribution: probability of k successes in n draws without replacement from a population
  //! of N holding K successes, P(k) = C(K, k) C(N - K, n - k)/C(N, n), max(0, n + K - N) <= k <= min(n, K).
  //! Parameters: 0 amplitude, 1 N, 2 K, 3 n (the last three fixed by default). Mean n K/N, variance
  //! n (K/N) (1 - K/N) (N - n)/(N - 1).
  //!
  class HypergeometricDistribution : public DiscreteDistribution
  {
  public:
  //! @brief Distribution. @param amplitude A @param population N @param successes K (0 to N)
  //! @param draws n (0 to N) @throws MathException if K or n is not within [0, N]
  HypergeometricDistribution(double amplitude=1.0, double population=50.0, double successes=10.0, double draws=10.0);
  HypergeometricDistribution * clone() const override { return new HypergeometricDistribution(*this); }
  //! @brief Population. @return N
  double population() const { return parameterValue(1); }
  //! @brief Successes in the population. @return K
  double successes() const { return parameterValue(2); }
  //! @brief Draws. @return n
  double draws() const { return parameterValue(3); }
  //! @brief Mean. @return n K/N
  double mean() const override { return draws()*successes()/population(); }
  //! @brief Variance. @return n (K/N) (1 - K/N) (N - n)/(N - 1)
  double variance() const override;
  protected:
  double logProbability(double x) const override;
  bool inSupport(double x) const override;
  };

  } // namespace MATH
} // namespace CAP

#endif // CAP_MATH_DiscreteDistributions
