/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file StatisticalTests.hpp
//! @brief Statistical tests: binomial, Poisson, Gaussian (z), Student's t, chi square, histogram compatibility,
//! Kolmogorov-Smirnov.
#ifndef CAP__MATH__StatisticalTests
#define CAP__MATH__StatisticalTests
#include <functional>
#include <vector>
#include "Aliases.hpp"

namespace CAP
{
  namespace MATH
  {

  class DataPoints;
  class Function;
  class ParameterizedFunction;

  //!
  //! @brief Alternative hypothesis of a test: the parameter differs from its value under the null hypothesis
  //! (kTwoSided), is below it (kLess), or above it (kGreater).
  //!
  enum class Alternative { kTwoSided, kLess, kGreater };

  //!
  //! @brief Outcome of a statistical test: the statistic, its degrees of freedom, the p-value, and an estimate of the
  //! parameter tested.
  //!
  //! The p-value is the probability, under the null hypothesis, of a statistic at least as extreme as the one
  //! observed, in the direction of the alternative. The null hypothesis is rejected at a significance level alpha
  //! when the p-value is below alpha (rejected()).
  //!
  struct TestResult
  {
    String test;   //!< name of the test, e.g. "binomial test"
    String nullHypothesis;   //!< the null hypothesis, e.g. "p = 0.5"
    String statisticName;   //!< name of the statistic: "k", "z", "t", "chi2", "D"
    double statistic = 0;   //!< value of the statistic
    double nDegreesOfFreedom = 0;   //!< degrees of freedom (t, chi square), 0 if none
    double pValue = 1;   //!< p-value
    Alternative alternative = Alternative::kTwoSided;   //!< alternative hypothesis
    double estimate = 0;   //!< estimate of the parameter tested (e.g. k/n, a mean, a difference of means)
    int    nPoints = 0;   //!< observations, bins or classes used

    //! @brief Whether the null hypothesis is rejected at a significance level. @param alpha level @return p-value < alpha
    bool rejected(double alpha=0.05) const { return pValue < alpha; }
    //! @brief One line: test, null hypothesis, statistic, degrees of freedom, p-value. @return text
    String summary() const;
  };

  //! @name Tests of counts
  //! @{

  //!
  //! @brief Exact binomial test: k successes in n trials, null hypothesis p = p0. The two-sided p-value is the sum of
  //! the probabilities of the outcomes no more probable than k (as R's binom.test). @param k successes @param n trials
  //! @param p0 probability under the null hypothesis (0 < p0 < 1) @param alternative alternative
  //! @return result (statistic k, estimate k/n) @throws MathException if k > n, n = 0 or p0 is not in (0,1)
  //!
  TestResult binomialTest(unsigned long k, unsigned long n, double p0, Alternative alternative=Alternative::kTwoSided);
  //!
  //! @brief Exact Poisson test: k events observed, null hypothesis mean = mu0. The two-sided p-value is the sum of the
  //! probabilities of the counts no more probable than k. @param k count @param mu0 mean under the null hypothesis (> 0)
  //! @param alternative alternative @return result (statistic k, estimate k) @throws MathException if mu0 <= 0
  //!
  TestResult poissonTest(unsigned long k, double mu0, Alternative alternative=Alternative::kTwoSided);
  //!
  //! @brief Comparison of two Poisson counts k1 and k2 observed with exposures t1 and t2 (times, luminosities), null
  //! hypothesis of equal rates: conditionally on k1 + k2, k1 is binomial with p = t1/(t1 + t2).
  //! @param k1 first count @param t1 its exposure (> 0) @param k2 second count @param t2 its exposure (> 0)
  //! @param alternative alternative (kGreater: the first rate is larger) @return result (estimate: ratio of the rates)
  //! @throws MathException if an exposure is not positive or k1 + k2 = 0
  //!
  TestResult poissonComparisonTest(unsigned long k1, double t1, unsigned long k2, double t2, Alternative alternative=Alternative::kTwoSided);
  //! @}

  //! @name Gaussian (z) tests: known standard deviations
  //! @{

  //!
  //! @brief z test of a measurement x of known standard deviation sigma, null hypothesis mean = mu0: z = (x - mu0)/sigma.
  //! @param x measurement @param sigma standard deviation (> 0) @param mu0 mean under the null hypothesis
  //! @param alternative alternative @return result (statistic z, estimate x) @throws MathException if sigma <= 0
  //!
  TestResult gaussianTest(double x, double sigma, double mu0, Alternative alternative=Alternative::kTwoSided);
  //!
  //! @brief z test of the mean of values of known standard deviation sigma: z = (mean - mu0) sqrt(n)/sigma.
  //! @param values values @param sigma standard deviation of each value (> 0) @param mu0 mean under the null hypothesis
  //! @param alternative alternative @return result (estimate: the mean) @throws MathException if there is no value or sigma <= 0
  //!
  TestResult gaussianTest(const std::vector<double> & values, double sigma, double mu0, Alternative alternative=Alternative::kTwoSided);
  //!
  //! @brief Compatibility of two measurements x1 +- sigma1 and x2 +- sigma2 (independent, Gaussian): z = (x1 - x2)/
  //! sqrt(sigma1^2 + sigma2^2). @param x1 first @param sigma1 its standard deviation @param x2 second @param sigma2 its
  //! standard deviation @param alternative alternative (kGreater: x1 is larger) @return result (estimate: x1 - x2)
  //! @throws MathException if both standard deviations are 0
  //!
  TestResult gaussianComparisonTest(double x1, double sigma1, double x2, double sigma2, Alternative alternative=Alternative::kTwoSided);
  //! @}

  //! @name Student's t tests: unknown standard deviations
  //! @{

  //!
  //! @brief One-sample t test: null hypothesis mean = mu0, t = (mean - mu0) sqrt(n)/s, n - 1 degrees of freedom.
  //! @param values values (at least 2) @param mu0 mean under the null hypothesis @param alternative alternative
  //! @return result (estimate: the mean) @throws MathException if there are fewer than 2 values or they are all equal
  //!
  TestResult studentTTest(const std::vector<double> & values, double mu0, Alternative alternative=Alternative::kTwoSided);
  //!
  //! @brief Two-sample t test: null hypothesis of equal means. Welch's test (the default; degrees of freedom of
  //! Welch-Satterthwaite), or with a pooled variance (n1 + n2 - 2 degrees of freedom) if the variances are equal.
  //! @param a first sample (at least 2 values) @param b second sample (at least 2 values) @param equalVariances pool the
  //! variances @param alternative alternative (kGreater: the first mean is larger) @return result (estimate: mean(a) -
  //! mean(b)) @throws MathException if a sample has fewer than 2 values, or both have no spread
  //!
  TestResult studentTTest(const std::vector<double> & a, const std::vector<double> & b, bool equalVariances=false,
                          Alternative alternative=Alternative::kTwoSided);
  //!
  //! @brief Paired t test: the one-sample test of the differences a_i - b_i, null hypothesis mean difference = 0.
  //! @param a first values @param b second values (same number) @param alternative alternative @return result
  //! @throws MathException if the numbers differ or there are fewer than 2 pairs
  //!
  TestResult pairedStudentTTest(const std::vector<double> & a, const std::vector<double> & b, Alternative alternative=Alternative::kTwoSided);
  //! @}

  //! @name Chi square tests
  //! @{

  //! @brief p-value of a chi square of ndf degrees of freedom. @param chiSquare chi square (>= 0) @param ndf degrees of
  //! freedom (> 0) @return result @throws MathException if ndf <= 0 or chiSquare < 0
  TestResult chiSquareTest(double chiSquare, double ndf);
  //!
  //! @brief Goodness of fit of counts to expected counts (Pearson): chi2 = sum (n_i - e_i)^2/e_i over the classes with
  //! e_i > 0, nClasses - 1 - nFitted degrees of freedom (the total is constrained). Classes should expect at least about
  //! 5 counts each for the chi square distribution to hold. @param observed counts @param expected expected counts (> 0;
  //! scaled to the total of the observed counts if normalize) @param nFitted parameters fitted to the data
  //! @param normalize scale the expected counts to the observed total @return result
  //! @throws MathException if the sizes differ, or there are no degrees of freedom
  //!
  TestResult chiSquareTest(const std::vector<double> & observed, const std::vector<double> & expected, int nFitted=0, bool normalize=true);
  //!
  //! @brief Goodness of fit of data points to a function: chi2 = sum ((y_i - f(x_i))/sigma_i)^2 over the points with a
  //! statistical uncertainty, nPoints - nFitted degrees of freedom. @param data data points @param function function
  //! (of the rank of the data) @param nFitted parameters of the function fitted to the data @return result
  //! @throws MathException if no point has an uncertainty or there are no degrees of freedom
  //!
  TestResult chiSquareTest(const DataPoints & data, const Function & function, int nFitted=0);
  //!
  //! @brief Independence of the rows and columns of a contingency table (Pearson): chi2 = sum (n_ij - e_ij)^2/e_ij with
  //! e_ij = (row total)(column total)/total, (rows - 1)(columns - 1) degrees of freedom. @param table counts, row by row
  //! @return result @throws MathException if the table has fewer than 2 rows or columns, rows of different sizes, or an
  //! empty row or column
  //!
  TestResult chiSquareIndependenceTest(const std::vector<std::vector<double>> & table);
  //! @}

  //! @name Compatibility of histograms
  //! @{

  //!
  //! @brief How two histograms (or sets of data points) are compared by histogramCompatibilityTest().
  //!  - kCounts: unweighted histograms of counts, of any totals: the chi square of homogeneity
  //!    sum (N2 n1_i - N1 n2_i)^2/(N1 N2 (n1_i + n2_i)) over the bins with entries, bins - 1 degrees of freedom
  //!    (as ROOT's TH1::Chi2Test "UU");
  //!  - kValues: values with uncertainties, compared as they are: sum (y1_i - y2_i)^2/(sigma1_i^2 + sigma2_i^2) over
  //!    the bins with an uncertainty, bins degrees of freedom;
  //!  - kShapes: values with uncertainties, each set scaled to a sum of 1 first (only their shapes are compared),
  //!    bins - 1 degrees of freedom.
  //!
  enum class HistogramComparison { kCounts, kValues, kShapes };
  //!
  //! @brief Compatibility of two histograms (or sets of data points) of the same binning: a chi square test, see
  //! HistogramComparison. @param a first @param b second @param comparison how they are compared @return result
  //! @throws MathException if the numbers of points differ, or no bin can be compared
  //!
  TestResult histogramCompatibilityTest(const DataPoints & a, const DataPoints & b, HistogramComparison comparison=HistogramComparison::kCounts);
  //! @}

  //! @name Kolmogorov-Smirnov tests
  //! Two-sided: D is the largest distance between the cumulative distributions; the p-value is Q_KS((sqrt(n) + 0.12 +
  //! 0.11/sqrt(n)) D) (Stephens' approximation, good from n of about 5), n being the number of values, or
  //! n1 n2/(n1 + n2) for two samples.
  //! @{

  //!
  //! @brief Kolmogorov-Smirnov test of a sample against a cumulative distribution. @param values values (at least 1)
  //! @param cdf cumulative distribution F(x) @return result (statistic D) @throws MathException if there is no value
  //!
  TestResult kolmogorovSmirnovTest(const std::vector<double> & values, const std::function<double(double)> & cdf);
  //!
  //! @brief Kolmogorov-Smirnov test of a sample against a density (not necessarily normalized) over [low, high]: its
  //! cumulative distribution is computed by integration (ParameterizedFunction::integralInRange()). Values outside
  //! the range are left out. @param values values @param density density, of rank 1 @param low lower end @param high
  //! upper end @return result @throws MathException if no value is in the range or the integral of the density is not positive
  //!
  TestResult kolmogorovSmirnovTest(const std::vector<double> & values, const ParameterizedFunction & density, double low, double high);
  //!
  //! @brief Two-sample Kolmogorov-Smirnov test: whether two samples come from the same distribution.
  //! @param a first sample @param b second sample @return result @throws MathException if a sample is empty
  //!
  TestResult kolmogorovSmirnovTest(const std::vector<double> & a, const std::vector<double> & b);
  //!
  //! @brief Kolmogorov-Smirnov test of two histograms of rank 1 with the same bins: the cumulative distributions are
  //! those of the bin contents, and n1, n2 their sums (the entries of histograms of counts). Binned data have fewer
  //! distinct values than the samples they come from: the p-value is then too large (the test is conservative).
  //! @param a first histogram @param b second histogram @return result
  //! @throws MathException if the ranks are not 1, the numbers of bins differ, or a sum is not positive
  //!
  TestResult kolmogorovSmirnovTest(const DataPoints & a, const DataPoints & b);
  //! @}

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__StatisticalTests
