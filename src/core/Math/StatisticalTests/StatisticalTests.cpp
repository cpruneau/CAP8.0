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
//! @file StatisticalTests.cpp
//! @brief Implementation of the statistical tests.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include "StatisticalTests.hpp"
#include "Distributions.hpp"
#include "DataPoints.hpp"
#include "Function.hpp"
#include "MathExceptions.hpp"
#include "ParameterizedFunction.hpp"

namespace CAP
{
  namespace MATH
  {

  using namespace Distributions;

  namespace
  {
  const double kRelativeTolerance = 1.0 + 1.0E-7;   // outcomes "as probable" as the one observed (as R)

  String format(const char * pattern, double a, double b=0, double c=0)
  {
  char text[200];
  std::snprintf(text, sizeof(text), pattern, a, b, c);
  return String(text);
  }

  //! p-value of a statistic of a symmetric distribution, from the lower and upper tail probabilities.
  double pValueOf(Alternative alternative, double lower, double upper)
  {
  switch (alternative)
    {
    case Alternative::kLess:    return lower;
    case Alternative::kGreater: return upper;
    default:                    return std::min(1.0, 2.0*std::min(lower, upper));
    }
  }

  double mean(const std::vector<double> & values)
  {
  double sum = 0;
  for (double v : values) sum += v;
  return sum/double(values.size());
  }

  //! Unbiased variance (n - 1 in the denominator).
  double variance(const std::vector<double> & values, double m)
  {
  double sum = 0;
  for (double v : values) sum += (v - m)*(v - m);
  return sum/double(values.size() - 1);
  }

  //! Two-sided p-value of an exact binomial test: the outcomes no more probable than k.
  double binomialTwoSided(unsigned long k, unsigned long n, double p)
  {
  double d = binomialProbability(k, n, p)*kRelativeTolerance, sum = 0;
  for (unsigned long i=0; i<=n; i++)
    {
    double q = binomialProbability(i, n, p);
    if (q <= d) sum += q;
    }
  return std::min(1.0, sum);
  }

  //! Kolmogorov-Smirnov result of a distance and an effective number of values.
  TestResult kolmogorovResult(const char * name, const char * hypothesis, double distance, double n, int nPoints)
  {
  TestResult r;
  r.test = name;
  r.nullHypothesis = hypothesis;
  r.statisticName = "D";
  r.statistic = distance;
  r.estimate = distance;
  r.nPoints = nPoints;
  double s = std::sqrt(n);
  r.pValue = kolmogorovUpper((s + 0.12 + 0.11/s)*distance);
  return r;
  }
  } // namespace

  String TestResult::summary() const
  {
  static const char * alternatives[] = { "two-sided", "less", "greater" };
  std::string text = std::string(test.data()) + ": " + nullHypothesis.data() + " (" + alternatives[int(alternative)] + "), "
                   + statisticName.data() + format(" = %.6g", statistic).data();
  if (nDegreesOfFreedom > 0) text += format(", ndf = %.4g", nDegreesOfFreedom).data();
  text += format(", p-value = %.4g", pValue).data();
  return String(text.c_str());
  }

  // ------------------------------------------------------------------------------------------------ counts

  TestResult binomialTest(unsigned long k, unsigned long n, double p0, Alternative alternative)
  {
  if (n == 0) throw MathException("A binomial test needs at least one trial", __FUNCTION__);
  if (k > n) throw MathException("More successes than trials", __FUNCTION__);
  if (!(p0 > 0.0 && p0 < 1.0)) throw MathException("The probability of a binomial test must be in (0,1)", __FUNCTION__);
  TestResult r;
  r.test = "binomial test";
  r.nullHypothesis = format("p = %g", p0);
  r.statisticName = "k";
  r.statistic = double(k);
  r.estimate = double(k)/double(n);
  r.alternative = alternative;
  r.nPoints = int(n);
  switch (alternative)
    {
    case Alternative::kLess:    r.pValue = binomialCdf(k, n, p0); break;
    case Alternative::kGreater: r.pValue = binomialUpper(k, n, p0); break;
    default:                    r.pValue = binomialTwoSided(k, n, p0); break;
    }
  return r;
  }

  TestResult poissonTest(unsigned long k, double mu0, Alternative alternative)
  {
  if (!(mu0 > 0)) throw MathException("The mean of a Poisson test must be > 0", __FUNCTION__);
  TestResult r;
  r.test = "Poisson test";
  r.nullHypothesis = format("mean = %g", mu0);
  r.statisticName = "k";
  r.statistic = double(k);
  r.estimate = double(k);
  r.alternative = alternative;
  r.nPoints = 1;
  if (alternative == Alternative::kLess) { r.pValue = poissonCdf(k, mu0); return r; }
  if (alternative == Alternative::kGreater) { r.pValue = poissonUpper(k, mu0); return r; }
  // two-sided: the counts no more probable than k, on both sides of the mode
  double d = poissonProbability(k, mu0)*kRelativeTolerance;
  unsigned long mode = (unsigned long)(std::floor(mu0));
  double p;
  if (double(k) < mu0)
    {
    p = poissonCdf(k, mu0);
    unsigned long i = std::max(mode, k + 1);
    while (poissonProbability(i, mu0) > d) i++;                     // first count above the mode as improbable
    p += poissonUpper(i, mu0);
    }
  else if (double(k) > mu0)
    {
    p = poissonUpper(k, mu0);
    long i = long(std::min(mode, k - 1));
    while (i >= 0 && poissonProbability((unsigned long)i, mu0) > d) i--;   // last count below the mode as improbable
    if (i >= 0) p += poissonCdf((unsigned long)i, mu0);
    }
  else p = 1.0;
  r.pValue = std::min(1.0, p);
  return r;
  }

  TestResult poissonComparisonTest(unsigned long k1, double t1, unsigned long k2, double t2, Alternative alternative)
  {
  if (!(t1 > 0) || !(t2 > 0)) throw MathException("The exposures of a comparison of counts must be > 0", __FUNCTION__);
  if (k1 + k2 == 0) throw MathException("A comparison of counts needs at least one count", __FUNCTION__);
  TestResult r = binomialTest(k1, k1 + k2, t1/(t1 + t2), alternative);
  r.test = "comparison of Poisson counts";
  r.nullHypothesis = "equal rates";
  r.estimate = k2 > 0 ? (double(k1)/t1)/(double(k2)/t2) : HUGE_VAL;
  r.nPoints = 2;
  return r;
  }

  // ------------------------------------------------------------------------------------------------ Gaussian

  TestResult gaussianTest(double x, double sigma, double mu0, Alternative alternative)
  {
  if (!(sigma > 0)) throw MathException("The standard deviation of a z test must be > 0", __FUNCTION__);
  TestResult r;
  r.test = "z test";
  r.nullHypothesis = format("mean = %g", mu0);
  r.statisticName = "z";
  r.statistic = (x - mu0)/sigma;
  r.estimate = x;
  r.alternative = alternative;
  r.nPoints = 1;
  r.pValue = pValueOf(alternative, normalCdf(r.statistic), normalUpper(r.statistic));
  return r;
  }

  TestResult gaussianTest(const std::vector<double> & values, double sigma, double mu0, Alternative alternative)
  {
  if (values.empty()) throw MathException("A z test needs values", __FUNCTION__);
  if (!(sigma > 0)) throw MathException("The standard deviation of a z test must be > 0", __FUNCTION__);
  double m = mean(values);
  TestResult r = gaussianTest(m, sigma/std::sqrt(double(values.size())), mu0, alternative);
  r.nPoints = int(values.size());
  return r;
  }

  TestResult gaussianComparisonTest(double x1, double sigma1, double x2, double sigma2, Alternative alternative)
  {
  double sigma = std::sqrt(sigma1*sigma1 + sigma2*sigma2);
  if (!(sigma > 0)) throw MathException("Two measurements without uncertainties cannot be compared", __FUNCTION__);
  TestResult r = gaussianTest(x1 - x2, sigma, 0.0, alternative);
  r.test = "z test of two measurements";
  r.nullHypothesis = "equal means";
  r.nPoints = 2;
  return r;
  }

  // ------------------------------------------------------------------------------------------------ Student

  TestResult studentTTest(const std::vector<double> & values, double mu0, Alternative alternative)
  {
  if (values.size() < 2) throw MathException("A t test needs at least 2 values", __FUNCTION__);
  double m = mean(values), s2 = variance(values, m);
  if (!(s2 > 0)) throw MathException("The values of a t test are all equal", __FUNCTION__);
  TestResult r;
  r.test = "one-sample t test";
  r.nullHypothesis = format("mean = %g", mu0);
  r.statisticName = "t";
  r.statistic = (m - mu0)/std::sqrt(s2/double(values.size()));
  r.nDegreesOfFreedom = double(values.size() - 1);
  r.estimate = m;
  r.alternative = alternative;
  r.nPoints = int(values.size());
  r.pValue = pValueOf(alternative, studentCdf(r.statistic, r.nDegreesOfFreedom), studentUpper(r.statistic, r.nDegreesOfFreedom));
  return r;
  }

  TestResult studentTTest(const std::vector<double> & a, const std::vector<double> & b, bool equalVariances, Alternative alternative)
  {
  if (a.size() < 2 || b.size() < 2) throw MathException("A two-sample t test needs at least 2 values in each sample", __FUNCTION__);
  double n1 = double(a.size()), n2 = double(b.size());
  double m1 = mean(a), m2 = mean(b), v1 = variance(a, m1), v2 = variance(b, m2);
  double standardError, ndf;
  if (equalVariances)
    {
    double pooled = ((n1 - 1)*v1 + (n2 - 1)*v2)/(n1 + n2 - 2);
    standardError = std::sqrt(pooled*(1.0/n1 + 1.0/n2));
    ndf = n1 + n2 - 2;
    }
  else
    {
    double e1 = v1/n1, e2 = v2/n2;
    standardError = std::sqrt(e1 + e2);
    ndf = (e1 + e2)*(e1 + e2)/(e1*e1/(n1 - 1) + e2*e2/(n2 - 1));
    }
  if (!(standardError > 0)) throw MathException("The samples of a t test have no spread", __FUNCTION__);
  TestResult r;
  r.test = equalVariances ? "two-sample t test (pooled variance)" : "Welch two-sample t test";
  r.nullHypothesis = "equal means";
  r.statisticName = "t";
  r.statistic = (m1 - m2)/standardError;
  r.nDegreesOfFreedom = ndf;
  r.estimate = m1 - m2;
  r.alternative = alternative;
  r.nPoints = int(a.size() + b.size());
  r.pValue = pValueOf(alternative, studentCdf(r.statistic, ndf), studentUpper(r.statistic, ndf));
  return r;
  }

  TestResult pairedStudentTTest(const std::vector<double> & a, const std::vector<double> & b, Alternative alternative)
  {
  if (a.size() != b.size()) throw MathException("A paired t test needs as many values in each sample", __FUNCTION__);
  std::vector<double> differences;
  for (std::size_t k=0; k<a.size(); k++) differences.push_back(a[k] - b[k]);
  TestResult r = studentTTest(differences, 0.0, alternative);
  r.test = "paired t test";
  r.nullHypothesis = "mean difference = 0";
  return r;
  }

  // ------------------------------------------------------------------------------------------------ chi square

  TestResult chiSquareTest(double chiSquare, double ndf)
  {
  if (!(ndf > 0)) throw MathException("A chi square test needs degrees of freedom", __FUNCTION__);
  if (!(chiSquare >= 0)) throw MathException("A chi square cannot be negative", __FUNCTION__);
  TestResult r;
  r.test = "chi square test";
  r.nullHypothesis = "chi square distribution";
  r.statisticName = "chi2";
  r.statistic = chiSquare;
  r.nDegreesOfFreedom = ndf;
  r.estimate = chiSquare/ndf;
  r.alternative = Alternative::kGreater;
  r.pValue = chiSquareUpper(chiSquare, ndf);
  return r;
  }

  TestResult chiSquareTest(const std::vector<double> & observed, const std::vector<double> & expected, int nFitted, bool normalize)
  {
  if (observed.size() != expected.size()) throw MathException("The observed and expected counts differ in number", __FUNCTION__);
  double totalObserved = 0, totalExpected = 0;
  for (std::size_t k=0; k<observed.size(); k++) { totalObserved += observed[k]; totalExpected += expected[k]; }
  double scale = (normalize && totalExpected > 0) ? totalObserved/totalExpected : 1.0;
  double chi2 = 0;
  int n = 0;
  for (std::size_t k=0; k<observed.size(); k++)
    {
    double e = expected[k]*scale;
    if (!(e > 0)) continue;
    chi2 += (observed[k] - e)*(observed[k] - e)/e;
    n++;
    }
  int ndf = n - (normalize ? 1 : 0) - nFitted;
  if (ndf <= 0) throw MathException("No degree of freedom left for the chi square test", __FUNCTION__);
  TestResult r = chiSquareTest(chi2, double(ndf));
  r.test = "chi square goodness of fit";
  r.nullHypothesis = "observed counts as expected";
  r.nPoints = n;
  return r;
  }

  TestResult chiSquareTest(const DataPoints & data, const Function & function, int nFitted)
  {
  if (data.rank() != function.rank()) throw MathException("The data and the function have different ranks", __FUNCTION__);
  double chi2 = 0;
  int n = 0;
  for (unsigned int k=0; k<data.size(); k++)
    {
    double sigma = data.statErrorAt(k);
    if (!(sigma > 0)) continue;
    double f;
    try { f = data.rank() == 1 ? function.evaluateAt(data.coordinatesAt(k)[0]) : function.evaluateAt(data.coordinatesAt(k)); }
    catch (...) { continue; }
    if (!std::isfinite(f)) continue;
    double pull = (data.valueAt(k) - f)/sigma;
    chi2 += pull*pull;
    n++;
    }
  if (n == 0) throw MathException("No data point has an uncertainty", __FUNCTION__);
  int ndf = n - nFitted;
  if (ndf <= 0) throw MathException("No degree of freedom left for the chi square test", __FUNCTION__);
  TestResult r = chiSquareTest(chi2, double(ndf));
  r.test = "chi square of data and a function";
  r.nullHypothesis = "data distributed about the function";
  r.nPoints = n;
  return r;
  }

  TestResult chiSquareIndependenceTest(const std::vector<std::vector<double>> & table)
  {
  std::size_t rows = table.size(), columns = rows ? table[0].size() : 0;
  if (rows < 2 || columns < 2) throw MathException("A contingency table needs at least 2 rows and 2 columns", __FUNCTION__);
  std::vector<double> rowTotals(rows, 0.0), columnTotals(columns, 0.0);
  double total = 0;
  for (std::size_t i=0; i<rows; i++)
    {
    if (table[i].size() != columns) throw MathException("The rows of a contingency table differ in size", __FUNCTION__);
    for (std::size_t j=0; j<columns; j++) { rowTotals[i] += table[i][j]; columnTotals[j] += table[i][j]; total += table[i][j]; }
    }
  for (double t : rowTotals) if (!(t > 0)) throw MathException("A row of the contingency table is empty", __FUNCTION__);
  for (double t : columnTotals) if (!(t > 0)) throw MathException("A column of the contingency table is empty", __FUNCTION__);
  double chi2 = 0;
  for (std::size_t i=0; i<rows; i++)
    for (std::size_t j=0; j<columns; j++)
      {
      double e = rowTotals[i]*columnTotals[j]/total;
      chi2 += (table[i][j] - e)*(table[i][j] - e)/e;
      }
  TestResult r = chiSquareTest(chi2, double((rows - 1)*(columns - 1)));
  r.test = "chi square test of independence";
  r.nullHypothesis = "rows and columns independent";
  r.nPoints = int(rows*columns);
  return r;
  }

  // ------------------------------------------------------------------------------------------------ histograms

  TestResult histogramCompatibilityTest(const DataPoints & a, const DataPoints & b, HistogramComparison comparison)
  {
  if (a.size() != b.size()) throw MathException("The histograms have different numbers of bins", __FUNCTION__);
  unsigned int n = a.size();
  double chi2 = 0;
  int used = 0;
  if (comparison == HistogramComparison::kCounts)
    {
    double total1 = 0, total2 = 0;
    for (unsigned int k=0; k<n; k++) { total1 += a.valueAt(k); total2 += b.valueAt(k); }
    if (!(total1 > 0) || !(total2 > 0)) throw MathException("A histogram has no entries", __FUNCTION__);
    for (unsigned int k=0; k<n; k++)
      {
      double n1 = a.valueAt(k), n2 = b.valueAt(k);
      if (!(n1 + n2 > 0)) continue;
      double d = total2*n1 - total1*n2;
      chi2 += d*d/(total1*total2*(n1 + n2));
      used++;
      }
    }
  else
    {
    double scale1 = 1, scale2 = 1;
    if (comparison == HistogramComparison::kShapes)
      {
      double total1 = 0, total2 = 0;
      for (unsigned int k=0; k<n; k++) { total1 += a.valueAt(k); total2 += b.valueAt(k); }
      if (!(total1 > 0) || !(total2 > 0)) throw MathException("A histogram has no positive sum", __FUNCTION__);
      scale1 = 1.0/total1;
      scale2 = 1.0/total2;
      }
    for (unsigned int k=0; k<n; k++)
      {
      double s1 = a.statErrorAt(k)*scale1, s2 = b.statErrorAt(k)*scale2, variance = s1*s1 + s2*s2;
      if (!(variance > 0)) continue;
      double d = a.valueAt(k)*scale1 - b.valueAt(k)*scale2;
      chi2 += d*d/variance;
      used++;
      }
    }
  int ndf = used - (comparison == HistogramComparison::kValues ? 0 : 1);
  if (ndf <= 0) throw MathException("No bin can be compared", __FUNCTION__);
  TestResult r = chiSquareTest(chi2, double(ndf));
  static const char * names[] = { "compatibility of histograms (counts)", "compatibility of histograms (values)",
                                  "compatibility of histograms (shapes)" };
  r.test = names[int(comparison)];
  r.nullHypothesis = comparison == HistogramComparison::kValues ? "same values" : "same distribution";
  r.nPoints = used;
  return r;
  }

  // ------------------------------------------------------------------------------------------------ Kolmogorov-Smirnov

  TestResult kolmogorovSmirnovTest(const std::vector<double> & values, const std::function<double(double)> & cdf)
  {
  if (values.empty()) throw MathException("A Kolmogorov-Smirnov test needs values", __FUNCTION__);
  std::vector<double> sorted(values);
  std::sort(sorted.begin(), sorted.end());
  double n = double(sorted.size()), distance = 0;
  for (std::size_t k=0; k<sorted.size(); k++)
    {
    double f = cdf(sorted[k]);
    distance = std::max(distance, std::max(std::fabs(f - double(k)/n), std::fabs(double(k + 1)/n - f)));
    }
  return kolmogorovResult("Kolmogorov-Smirnov test", "values distributed as F", distance, n, int(sorted.size()));
  }

  TestResult kolmogorovSmirnovTest(const std::vector<double> & values, const ParameterizedFunction & density, double low, double high)
  {
  std::vector<double> inside;
  for (double v : values) if (v >= low && v <= high) inside.push_back(v);
  if (inside.empty()) throw MathException("No value is inside the range", __FUNCTION__);
  std::sort(inside.begin(), inside.end());
  double total = density.integralInRange(low, high);
  if (!(total > 0)) throw MathException("The integral of the density is not positive", __FUNCTION__);
  // cumulative distribution at the sorted values, integrated piece by piece
  double n = double(inside.size()), distance = 0, cumulative = 0, previous = low;
  for (std::size_t k=0; k<inside.size(); k++)
    {
    cumulative += density.integralInRange(previous, inside[k]);
    previous = inside[k];
    double f = cumulative/total;
    distance = std::max(distance, std::max(std::fabs(f - double(k)/n), std::fabs(double(k + 1)/n - f)));
    }
  return kolmogorovResult("Kolmogorov-Smirnov test", "values distributed as the density", distance, n, int(inside.size()));
  }

  TestResult kolmogorovSmirnovTest(const std::vector<double> & a, const std::vector<double> & b)
  {
  if (a.empty() || b.empty()) throw MathException("A two-sample Kolmogorov-Smirnov test needs values in each sample", __FUNCTION__);
  std::vector<double> x(a), y(b);
  std::sort(x.begin(), x.end());
  std::sort(y.begin(), y.end());
  double n1 = double(x.size()), n2 = double(y.size()), distance = 0;
  std::size_t i = 0, j = 0;
  while (i < x.size() && j < y.size())
    {
    double v = std::min(x[i], y[j]);
    while (i < x.size() && x[i] <= v) i++;                         // ties advance both samples together
    while (j < y.size() && y[j] <= v) j++;
    distance = std::max(distance, std::fabs(double(i)/n1 - double(j)/n2));
    }
  return kolmogorovResult("two-sample Kolmogorov-Smirnov test", "same distribution", distance, n1*n2/(n1 + n2), int(x.size() + y.size()));
  }

  TestResult kolmogorovSmirnovTest(const DataPoints & a, const DataPoints & b)
  {
  if (a.rank() != 1 || b.rank() != 1) throw MathException("The Kolmogorov-Smirnov test of histograms needs rank 1", __FUNCTION__);
  if (a.size() != b.size()) throw MathException("The histograms have different numbers of bins", __FUNCTION__);
  double total1 = 0, total2 = 0;
  for (unsigned int k=0; k<a.size(); k++) { total1 += a.valueAt(k); total2 += b.valueAt(k); }
  if (!(total1 > 0) || !(total2 > 0)) throw MathException("A histogram has no positive sum", __FUNCTION__);
  double c1 = 0, c2 = 0, distance = 0;
  for (unsigned int k=0; k<a.size(); k++)
    {
    c1 += a.valueAt(k);
    c2 += b.valueAt(k);
    distance = std::max(distance, std::fabs(c1/total1 - c2/total2));
    }
  TestResult r = kolmogorovResult("Kolmogorov-Smirnov test of histograms", "same distribution", distance, total1*total2/(total1 + total2), int(a.size()));
  return r;
  }

  } // namespace MATH
} // namespace CAP
