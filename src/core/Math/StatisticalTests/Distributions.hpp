/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Distributions.hpp
//! @brief Distributions: probabilities and cumulative distributions used by the statistical tests.
#ifndef CAP__MATH__Distributions
#define CAP__MATH__Distributions

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Probability distributions of the statistical tests (see StatisticalTests.hpp): probabilities of the
  //! binomial and Poisson distributions, cumulative distributions of the normal, Student t, chi square, binomial and
  //! Poisson distributions, and of the Kolmogorov distribution, with the special functions they are made of.
  //!
  //! The cumulative distributions give P(X <= x); the "upper" ones P(X > x) (P(X >= k) for counts), computed directly
  //! so that small tail probabilities keep their precision.
  //!
  namespace Distributions
  {

  //! @name Special functions
  //! @{

  //! @brief Regularized lower incomplete gamma function P(a,x). @param a a (> 0) @param x x (>= 0) @return P(a,x)
  //! @throws MathException if a <= 0 or x < 0
  double regularizedGammaP(double a, double x);
  //! @brief Regularized upper incomplete gamma function Q(a,x) = 1 - P(a,x). @param a a (> 0) @param x x (>= 0)
  //! @return Q(a,x) @throws MathException if a <= 0 or x < 0
  double regularizedGammaQ(double a, double x);
  //!
  //! @brief Regularized incomplete beta function I_x(a,b) (continued fraction of Lentz).
  //! @param a a (> 0) @param b b (> 0) @param x x in [0,1] @return I_x(a,b) @throws MathException if a parameter is invalid
  //!
  double regularizedBeta(double a, double b, double x);
  //! @}

  //! @name Normal distribution (mean 0, standard deviation 1)
  //! @{

  //! @brief Density. @param z z @return exp(-z^2/2)/sqrt(2 pi)
  double normalDensity(double z);
  //! @brief P(Z <= z). @param z z @return probability
  double normalCdf(double z);
  //! @brief P(Z > z), precise far in the tail. @param z z @return probability
  double normalUpper(double z);
  //! @brief Quantile: z such that P(Z <= z) = p (Acklam's approximation refined by a Newton step, relative error
  //! about 1E-15). @param p probability in (0,1) @return z @throws MathException if p is not in (0,1)
  double normalQuantile(double p);
  //! @}

  //! @name Student t distribution
  //! @{

  //! @brief P(T <= t) for ndf degrees of freedom. @param t t @param ndf degrees of freedom (> 0, not necessarily an
  //! integer) @return probability @throws MathException if ndf <= 0
  double studentCdf(double t, double ndf);
  //! @brief P(T > t). @param t t @param ndf degrees of freedom (> 0) @return probability
  double studentUpper(double t, double ndf);
  //! @}

  //! @name Chi square distribution
  //! @{

  //! @brief P(X <= x) for ndf degrees of freedom. @param x x (>= 0) @param ndf degrees of freedom (> 0)
  //! @return probability @throws MathException if ndf <= 0 or x < 0
  double chiSquareCdf(double x, double ndf);
  //! @brief P(X > x): the p-value of a chi square. @param x x (>= 0) @param ndf degrees of freedom (> 0) @return probability
  double chiSquareUpper(double x, double ndf);
  //! @}

  //! @name Binomial distribution (n trials of probability p)
  //! @{

  //! @brief P(K = k). @param k k @param n n @param p p in [0,1] @return probability (0 if k > n)
  double binomialProbability(unsigned long k, unsigned long n, double p);
  //! @brief P(K <= k). @param k k @param n n @param p p in [0,1] @return probability
  double binomialCdf(unsigned long k, unsigned long n, double p);
  //! @brief P(K >= k). @param k k @param n n @param p p in [0,1] @return probability
  double binomialUpper(unsigned long k, unsigned long n, double p);
  //! @}

  //! @name Poisson distribution (mean mu)
  //! @{

  //! @brief P(K = k). @param k k @param mu mu (>= 0) @return probability
  double poissonProbability(unsigned long k, double mu);
  //! @brief P(K <= k). @param k k @param mu mu (>= 0) @return probability
  double poissonCdf(unsigned long k, double mu);
  //! @brief P(K >= k). @param k k @param mu mu (>= 0) @return probability
  double poissonUpper(unsigned long k, double mu);
  //! @}

  //! @name Kolmogorov distribution
  //! @{

  //!
  //! @brief Q_KS(lambda) = 2 sum_{j>=1} (-1)^(j-1) exp(-2 j^2 lambda^2): the probability that sqrt(n) D_n exceeds
  //! lambda for large n, D_n being the Kolmogorov-Smirnov distance. @param lambda lambda (>= 0) @return probability
  //!
  double kolmogorovUpper(double lambda);
  //! @}

  } // namespace Distributions

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__Distributions
