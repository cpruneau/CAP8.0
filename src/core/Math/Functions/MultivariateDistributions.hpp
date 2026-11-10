/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file MultivariateDistributions.hpp
//! @brief Distributions of several variables: MultinomialDistribution, DirichletDistribution.
#ifndef CAP_MATH_MultivariateDistributions
#define CAP_MATH_MultivariateDistributions
#include <vector>
#include "ParameterizedFunction.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Multinomial distribution: the probability of counts (k_1, ..., k_m), k_1 + ... + k_m = n, in n trials,
  //! each falling in category i with probability p_i: P = n!/(k_1! ... k_m!) p_1^k_1 ... p_m^k_m.
  //!
  //! Since k_m = n - (k_1 + ... + k_m-1) and p_m = 1 - (p_1 + ... + p_m-1), it is a function of the m - 1 variables
  //! k_1 ... k_m-1 (rank m - 1; m = 2: the binomial distribution), with parameters 0 amplitude A, 1 n (fixed by
  //! default), 2 ... m p_1 ... p_m-1 (each in [0, 1]). f = A P, continued between the integers by the gamma function
  //! (as DiscreteDistribution); 0 if a count or p_m is negative. The sum over all the counts is A.
  //!
  //! @code
  //!   CAP::MATH::MultinomialDistribution f(10.0, {0.2, 0.3, 0.5});   // rank 2: k_1, k_2 (k_3 = 10 - k_1 - k_2)
  //!   double p = f.evaluateAt(2.0, 3.0);                              // P(2, 3, 5)
  //! @endcode
  //!
  class MultinomialDistribution : public ParameterizedFunction
  {
  public:
  //! @brief Distribution of m >= 2 categories. @param n trials (>= 0) @param p probabilities of the m categories
  //! (each in [0, 1], their sum 1 within 1E-9) @param amplitude A
  //! @throws MathException if m < 2, n < 0, a probability is out of [0, 1] or their sum is not 1
  MultinomialDistribution(double n, const std::vector<double> & p, double amplitude=1.0);
  //! @brief Distribution of m categories of equal probabilities, 10 trials (the registry makes it). @param m m (>= 2)
  explicit MultinomialDistribution(unsigned int m=3);
  MultinomialDistribution * clone() const override { return new MultinomialDistribution(*this); }
  using ParameterizedFunction::evaluateAt;
  //! @brief f(k_1, ..., k_m-1). @param k counts of the first m - 1 categories @return value
  double evaluateAt(const std::vector<double> & k) const override;
  //! @brief f(k_1) (m = 2). @param k count @return value
  double evaluateAt(double k) const override;
  //! @brief f(k_1, k_2) (m = 3). @param k1 count @param k2 count @return value
  double evaluateAt(double k1, double k2) const override;
  //! @brief Number of categories. @return m
  unsigned int nCategories() const { return rank() + 1; }
  //! @brief Amplitude. @return A
  double amplitude() const { return parameterValue(0); }
  //! @brief Trials. @return n
  double trials() const { return parameterValue(1); }
  //! @brief Probability of category i (the last one: 1 minus the others). @param i category (0 to m - 1) @return p_i
  double probability(unsigned int i) const;
  //! @brief Mean count of category i. @param i category @return n p_i
  double mean(unsigned int i) const { return trials()*probability(i); }
  //! @brief Covariance of the counts of categories i and j. @param i category @param j category
  //! @return n p_i (1 - p_i) if i = j, -n p_i p_j otherwise
  double covariance(unsigned int i, unsigned int j) const;
  };

  //!
  //! @brief Dirichlet distribution of K >= 2 categories, of concentrations alpha_1 ... alpha_K > 0: the density of
  //! (x_1, ..., x_K) on the simplex x_i > 0, x_1 + ... + x_K = 1,
  //! p = Gamma(alpha_0)/(Gamma(alpha_1) ... Gamma(alpha_K)) x_1^(alpha_1 - 1) ... x_K^(alpha_K - 1),
  //! alpha_0 = alpha_1 + ... + alpha_K.
  //!
  //! Since x_K = 1 - (x_1 + ... + x_K-1), it is a function of the K - 1 variables x_1 ... x_K-1 (rank K - 1; K = 2:
  //! the beta distribution of x_1), whose integral over the simplex is 1. Parameters: 0 amplitude A, 1 ... K
  //! alpha_1 ... alpha_K (> 0). f = A p; 0 outside the simplex. Mean of x_i alpha_i/alpha_0, covariance
  //! (delta_ij alpha_i/alpha_0 - alpha_i alpha_j/alpha_0^2)/(alpha_0 + 1).
  //!
  //! @code
  //!   CAP::MATH::DirichletDistribution f({2.0, 3.0, 4.0});   // rank 2: x_1, x_2 (x_3 = 1 - x_1 - x_2)
  //!   double p = f.evaluateAt(0.2, 0.3);
  //! @endcode
  //!
  class DirichletDistribution : public ParameterizedFunction
  {
  public:
  //! @brief Distribution. @param alpha concentrations of the K categories (each > 0) @param amplitude A
  //! @throws MathException if K < 2 or a concentration is not > 0
  explicit DirichletDistribution(const std::vector<double> & alpha, double amplitude=1.0);
  //! @brief Distribution of K categories of concentrations 1 (uniform; the registry makes it). @param K K (>= 2)
  explicit DirichletDistribution(unsigned int K=3);
  DirichletDistribution * clone() const override { return new DirichletDistribution(*this); }
  using ParameterizedFunction::evaluateAt;
  //! @brief f(x_1, ..., x_K-1). @param x the first K - 1 fractions @return value
  double evaluateAt(const std::vector<double> & x) const override;
  //! @brief f(x_1) (K = 2). @param x fraction @return value
  double evaluateAt(double x) const override;
  //! @brief f(x_1, x_2) (K = 3). @param x1 fraction @param x2 fraction @return value
  double evaluateAt(double x1, double x2) const override;
  //! @brief Number of categories. @return K
  unsigned int nCategories() const { return rank() + 1; }
  //! @brief Amplitude. @return A
  double amplitude() const { return parameterValue(0); }
  //! @brief Concentration of category i. @param i category (0 to K - 1) @return alpha_i
  double concentration(unsigned int i) const { return parameterValue(1 + i); }
  //! @brief Mean of x_i. @param i category @return alpha_i/alpha_0
  double mean(unsigned int i) const;
  //! @brief Covariance of x_i and x_j. @param i category @param j category @return covariance
  double covariance(unsigned int i, unsigned int j) const;
  };

  } // namespace MATH
} // namespace CAP

#endif // CAP_MATH_MultivariateDistributions
