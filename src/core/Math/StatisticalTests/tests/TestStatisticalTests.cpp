/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestStatisticalTests.cpp
//! @brief Test program of the statistical tests and their distributions (run by ctest).
//
// TestStatisticalTests -- the distributions (special functions, normal, Student t, chi square, binomial, Poisson,
// Kolmogorov) against known values and closed forms, and the tests (binomial, Poisson, z, t, chi square, histogram
// compatibility, Kolmogorov-Smirnov) against values computed independently here, and on samples drawn from equal or
// different distributions. Prints each failed check and exits with the number of failures (0: all passed).
//
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include "DataPointsVector.hpp"
#include "Distributions.hpp"
#include "Histogram.hpp"
#include "Linear.hpp"
#include "MathExceptions.hpp"
#include "Normal.hpp"
#include "StatisticalTests.hpp"

using namespace CAP;
using namespace CAP::MATH;
using namespace CAP::MATH::Distributions;

namespace
{
int nChecks = 0, nFailures = 0;

void check(bool ok, const std::string & what, int line)
{
  nChecks++;
  if (ok) return;
  nFailures++;
  std::printf("FAILED (line %d): %s\n", line, what.c_str());
}

bool near(double a, double b, double tolerance) { return std::fabs(a - b) <= tolerance*std::max(1.0, std::fabs(b)); }

//! Pseudo-random numbers with a fixed seed.
struct Random
{
  unsigned long long seed;
  explicit Random(unsigned long long s) : seed(s) { }
  double uniform() { seed = seed*6364136223846793005ULL + 1442695040888963407ULL; return ((seed >> 11) + 0.5)/9007199254740992.0; }
  double gaussian() { double u1 = uniform(), u2 = uniform(); return std::sqrt(-2*std::log(u1))*std::cos(2*M_PI*u2); }
};

//! P(K = k) of a binomial distribution, by its definition.
double binomial(unsigned long k, unsigned long n, double p)
{
  double c = 1;
  for (unsigned long i=1; i<=k; i++) c *= double(n - k + i)/double(i);
  return c*std::pow(p, double(k))*std::pow(1 - p, double(n - k));
}

//! P(K = k) of a Poisson distribution, by its definition.
double poisson(unsigned long k, double mu)
{
  double p = std::exp(-mu);
  for (unsigned long i=1; i<=k; i++) p *= mu/double(i);
  return p;
}

//! Largest distance between the empirical distributions of two samples, at every value of either.
double distance(const std::vector<double> & a, const std::vector<double> & b)
{
  double d = 0;
  std::vector<double> all(a);
  all.insert(all.end(), b.begin(), b.end());
  for (double v : all)
    {
    double fa = double(std::count_if(a.begin(), a.end(), [v](double x) { return x <= v; }))/double(a.size());
    double fb = double(std::count_if(b.begin(), b.end(), [v](double x) { return x <= v; }))/double(b.size());
    d = std::max(d, std::fabs(fa - fb));
    }
  return d;
}
} // namespace

//! @brief Count a check, and report it if the condition is false.
#define CHECK(condition) check((condition), #condition, __LINE__)

//! @brief Run the checks. @return number of failed checks (0: all passed)
int main()
{
  // ---- distributions
  CHECK(near(regularizedBeta(2.0, 3.0, 0.4), 0.5248, 1E-12));                        // P(Bin(4, 0.4) >= 2)
  CHECK(near(regularizedGammaP(1.0, 2.0), 1.0 - std::exp(-2.0), 1E-13) && near(regularizedGammaQ(3.0, 0.5) + regularizedGammaP(3.0, 0.5), 1.0, 1E-14));
  CHECK(near(normalCdf(1.96), 0.9750021048517795, 1E-14) && near(normalUpper(5.0), 2.866515718791939E-7, 1E-12));
  CHECK(near(normalQuantile(0.975), 1.959963984540054, 1E-12) && near(normalQuantile(1E-10), -6.361340902404056, 1E-10));
  CHECK(near(studentCdf(2.228138851986274, 10.0), 0.975, 1E-12) && near(studentCdf(1.0, 1.0), 0.75, 1E-13)
        && near(studentUpper(-1.0, 1.0), 0.75, 1E-13));
  CHECK(near(studentCdf(1.7, 1E7), normalCdf(1.7), 1E-7));
  CHECK(near(chiSquareUpper(3.841458820694124, 1.0), 0.05, 1E-12) && near(chiSquareUpper(18.307038053275146, 10.0), 0.05, 1E-12)
        && near(chiSquareUpper(7.3, 2.0), std::exp(-3.65), 1E-13) && near(chiSquareCdf(7.3, 2.0), 1 - std::exp(-3.65), 1E-13));
  {
    double cdf = 0, upper = 0;
    for (unsigned long k=0; k<=7; k++) cdf += binomial(k, 20, 0.35);
    for (unsigned long k=7; k<=20; k++) upper += binomial(k, 20, 0.35);
    CHECK(near(binomialProbability(7, 20, 0.35), binomial(7, 20, 0.35), 1E-12) && near(binomialCdf(7, 20, 0.35), cdf, 1E-12)
          && near(binomialUpper(7, 20, 0.35), upper, 1E-12) && binomialCdf(20, 20, 0.35) == 1.0 && binomialUpper(0, 20, 0.35) == 1.0);
    cdf = 0; upper = 0;
    for (unsigned long k=0; k<=12; k++) cdf += poisson(k, 8.5);
    upper = 1.0 - cdf + poisson(12, 8.5);
    CHECK(near(poissonProbability(12, 8.5), poisson(12, 8.5), 1E-12) && near(poissonCdf(12, 8.5), cdf, 1E-12) && near(poissonUpper(12, 8.5), upper, 1E-11));
  }
  CHECK(near(kolmogorovUpper(1.3580986393225505), 0.05, 1E-6) && near(kolmogorovUpper(1.0), 0.26999967167735456, 1E-9) && kolmogorovUpper(0.0) == 1.0);
  bool thrown = false;
  try { normalQuantile(1.0); } catch (const MathException &) { thrown = true; }
  CHECK(thrown);

  // ---- binomial test
  {
    TestResult r = binomialTest(7, 20, 0.5);
    CHECK(near(r.pValue, 0.26317596435546875, 1E-12) && r.statistic == 7 && r.estimate == 0.35);   // 2 P(K <= 7), symmetric
    CHECK(near(binomialTest(7, 20, 0.5, Alternative::kLess).pValue, 0.131587982177734375, 1E-12));
    double two = 0, d = binomial(3, 30, 0.25);
    for (unsigned long k=0; k<=30; k++) if (binomial(k, 30, 0.25) <= d*(1 + 1E-7)) two += binomial(k, 30, 0.25);
    CHECK(near(binomialTest(3, 30, 0.25).pValue, two, 1E-10));                     // asymmetric: outcomes as improbable
    double greater = 0;
    for (unsigned long k=12; k<=30; k++) greater += binomial(k, 30, 0.25);
    CHECK(near(binomialTest(12, 30, 0.25, Alternative::kGreater).pValue, greater, 1E-11));
    CHECK(r.rejected(0.3) && !r.rejected(0.2) && !r.rejected());                   // p-value 0.263
    thrown = false;
    try { binomialTest(5, 4, 0.5); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);
    CHECK(r.summary().contains("binomial test: p = 0.5 (two-sided), k = 7, p-value = 0.2632"));
  }

  // ---- Poisson tests
  {
    for (unsigned long k : { 2ul, 10ul, 5ul, 0ul })
      {
      double expected = 0, d = poisson(k, 5.0);
      for (unsigned long i=0; i<200; i++) if (poisson(i, 5.0) <= d*(1 + 1E-7)) expected += poisson(i, 5.0);
      CHECK(near(poissonTest(k, 5.0).pValue, std::min(1.0, expected), 1E-10));
      }
    double less = 0;
    for (unsigned long i=0; i<=2; i++) less += poisson(i, 5.0);
    CHECK(near(poissonTest(2, 5.0, Alternative::kLess).pValue, less, 1E-12) && near(poissonTest(10, 5.0, Alternative::kGreater).pValue, poissonUpper(10, 5.0), 1E-14));
    TestResult c = poissonComparisonTest(30, 1.0, 10, 1.0);                         // 30 against 10 in equal times
    CHECK(near(c.pValue, binomialTest(30, 40, 0.5).pValue, 1E-14) && c.estimate == 3.0 && c.rejected(0.01));
    CHECK(near(poissonComparisonTest(20, 2.0, 10, 1.0).pValue, 1.0, 1E-9));         // equal rates
  }

  // ---- z tests
  {
    TestResult z = gaussianTest(11.96, 1.0, 10.0);
    CHECK(near(z.statistic, 1.96, 1E-12) && near(z.pValue, 0.04999579029644087, 1E-10) && near(gaussianTest(11.96, 1.0, 10.0, Alternative::kGreater).pValue, 0.024997895148220435, 1E-10));
    TestResult m = gaussianTest(std::vector<double>{ 1.0, 2.0, 3.0, 4.0 }, 2.0, 1.0);
    CHECK(near(m.statistic, 1.5, 1E-12) && m.estimate == 2.5 && m.nPoints == 4);   // (2.5 - 1)/(2/2)
    TestResult c = gaussianComparisonTest(10.0, 3.0, 4.0, 4.0);
    CHECK(near(c.statistic, 1.2, 1E-12) && near(c.pValue, 2*normalUpper(1.2), 1E-14) && c.estimate == 6.0);
  }

  // ---- t tests
  {
    TestResult t = studentTTest(std::vector<double>{ 1, 2, 3, 4, 5 }, 2.0);
    double tt = std::sqrt(2.0), f = 0.5 + 0.375*(tt/std::sqrt(1 + tt*tt/4))*(1 - tt*tt/(12*(1 + tt*tt/4)));   // closed form, 4 degrees of freedom
    CHECK(near(t.statistic, tt, 1E-12) && t.nDegreesOfFreedom == 4 && near(t.pValue, 2*(1 - f), 1E-12) && near(t.pValue, 0.2302, 1E-4));
    std::vector<double> a{ 19.1, 21.4, 20.2, 18.7, 22.0, 20.9 }, b{ 22.3, 23.1, 21.8, 24.0, 22.9, 23.5, 22.1 };
    double ma = 0, mb = 0, va = 0, vb = 0;
    for (double v : a) ma += v / a.size();
    for (double v : b) mb += v / b.size();
    for (double v : a) va += (v - ma)*(v - ma)/(a.size() - 1);
    for (double v : b) vb += (v - mb)*(v - mb)/(b.size() - 1);
    TestResult w = studentTTest(a, b);
    double ea = va/a.size(), eb = vb/b.size(), dfW = (ea + eb)*(ea + eb)/(ea*ea/(a.size() - 1) + eb*eb/(b.size() - 1));
    CHECK(near(w.statistic, (ma - mb)/std::sqrt(ea + eb), 1E-12) && near(w.nDegreesOfFreedom, dfW, 1E-12) && w.rejected(0.01) && w.estimate < 0);
    TestResult pooled = studentTTest(a, b, true, Alternative::kLess);
    double sp = ((a.size() - 1)*va + (b.size() - 1)*vb)/(a.size() + b.size() - 2);
    CHECK(near(pooled.statistic, (ma - mb)/std::sqrt(sp*(1.0/a.size() + 1.0/b.size())), 1E-12) && pooled.nDegreesOfFreedom == 11
          && near(pooled.pValue, studentCdf(pooled.statistic, 11), 1E-14));
    TestResult p = pairedStudentTTest({ 10.0, 12.0, 9.0, 11.0 }, { 9.0, 11.5, 8.0, 10.0 });
    CHECK(near(p.estimate, 0.875, 1E-12) && p.nDegreesOfFreedom == 3);
    thrown = false;
    try { studentTTest(std::vector<double>{ 3.0, 3.0, 3.0 }, 1.0); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);
  }

  // ---- chi square tests
  {
    TestResult g = chiSquareTest(std::vector<double>{ 18, 22, 20, 40 }, std::vector<double>{ 1, 1, 1, 1 });
    double x = 12.32, upper3 = 2*normalUpper(std::sqrt(x)) + std::sqrt(2*x/M_PI)*std::exp(-x/2);   // closed form, 3 degrees of freedom
    CHECK(near(g.statistic, 12.32, 1E-12) && g.nDegreesOfFreedom == 3 && near(g.pValue, upper3, 1E-12) && g.rejected(0.01));
    CHECK(chiSquareTest(std::vector<double>{ 18, 22, 20, 40 }, std::vector<double>{ 25, 25, 25, 25 }, 1).nDegreesOfFreedom == 2);
    TestResult i = chiSquareIndependenceTest({ { 10, 20 }, { 30, 40 } });
    double chi2 = 4.0/12 + 4.0/18 + 4.0/28 + 4.0/42;
    CHECK(near(i.statistic, chi2, 1E-12) && i.nDegreesOfFreedom == 1 && near(i.pValue, 2*normalUpper(std::sqrt(chi2)), 1E-12));
    DataPointsVector points;
    points.addPoints({ 0.0, 1.0, 2.0, 3.0 }, { 1.1, 2.9, 5.2, 6.8 }, { 0.1, 0.2, 0.1, 0.4 });
    Linear line(1.0, 2.0);
    TestResult d = chiSquareTest(points, line, 0);
    double expected = 1.0 + 0.25 + 4.0 + 0.25;                                     // pulls 1, -0.5, 2, -0.5
    CHECK(near(d.statistic, expected, 1E-5) && d.nDegreesOfFreedom == 4 && d.nPoints == 4);
    thrown = false;
    try { chiSquareTest(points, line, 4); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);
  }

  // ---- compatibility of histograms
  {
    Random random(17);
    Histogram h1("h1", "h1", 20, -4.0, 4.0), h2("h2", "h2", 20, -4.0, 4.0), h3("h3", "h3", 20, -4.0, 4.0);
    for (int k=0; k<5000; k++) h1.fill(random.gaussian());
    for (int k=0; k<8000; k++) h2.fill(random.gaussian());
    for (int k=0; k<8000; k++) h3.fill(0.25 + random.gaussian());
    TestResult same = histogramCompatibilityTest(h1, h2);
    TestResult shifted = histogramCompatibilityTest(h1, h3);
    CHECK(same.pValue > 0.001 && shifted.pValue < 1E-6 && same.nDegreesOfFreedom == same.nPoints - 1);
    double chi2 = 0, n1 = 0, n2 = 0;                                                 // the homogeneity chi square, by hand
    for (unsigned int k=0; k<20; k++) { n1 += h1.valueAt(k); n2 += h2.valueAt(k); }
    for (unsigned int k=0; k<20; k++)
      if (h1.valueAt(k) + h2.valueAt(k) > 0) chi2 += std::pow(n2*h1.valueAt(k) - n1*h2.valueAt(k), 2)/(n1*n2*(h1.valueAt(k) + h2.valueAt(k)));
    CHECK(near(same.statistic, chi2, 1E-10));
    TestResult shapes = histogramCompatibilityTest(h1, h2, HistogramComparison::kShapes);
    TestResult values = histogramCompatibilityTest(h1, h2, HistogramComparison::kValues);
    CHECK(shapes.pValue > 0.001 && values.pValue < 1E-6 && values.nDegreesOfFreedom == values.nPoints);   // 5000 against 8000 entries
    TestResult itself = histogramCompatibilityTest(h1, h1, HistogramComparison::kValues);
    CHECK(itself.statistic == 0 && itself.pValue == 1.0);
    TestResult ks = kolmogorovSmirnovTest(h1, h2), ksShifted = kolmogorovSmirnovTest(h1, h3);
    CHECK(ks.pValue > 0.001 && ksShifted.pValue < 1E-6 && kolmogorovSmirnovTest(h1, h1).statistic == 0);
  }

  // ---- Kolmogorov-Smirnov tests
  {
    TestResult u = kolmogorovSmirnovTest({ 0.1, 0.4, 0.7 }, [](double x) { return std::min(1.0, std::max(0.0, x)); });
    CHECK(near(u.statistic, 0.3, 1E-12));
    Random random(5);
    std::vector<double> a, b, c;
    for (int k=0; k<400; k++) a.push_back(random.gaussian());
    for (int k=0; k<600; k++) b.push_back(random.gaussian());
    for (int k=0; k<600; k++) c.push_back(0.4 + random.gaussian());
    TestResult one = kolmogorovSmirnovTest(a, [](double x) { return normalCdf(x); });
    CHECK(one.pValue > 0.001 && kolmogorovSmirnovTest(c, [](double x) { return normalCdf(x); }).pValue < 1E-6);
    Normal density(1.0, 0.0, 1.0);
    TestResult fromDensity = kolmogorovSmirnovTest(a, density, -20.0, 20.0);
    CHECK(near(fromDensity.statistic, one.statistic, 1E-8));
    TestResult two = kolmogorovSmirnovTest(a, b), different = kolmogorovSmirnovTest(a, c);
    CHECK(near(two.statistic, distance(a, b), 1E-12) && near(different.statistic, distance(a, c), 1E-12));
    CHECK(two.pValue > 0.001 && different.pValue < 1E-6);
    std::vector<double> ties{ 1, 1, 2, 2, 3 }, others{ 1, 2, 2, 3, 3, 3 };
    CHECK(near(kolmogorovSmirnovTest(ties, others).statistic, distance(ties, others), 1E-12));
  }

  std::printf("TestStatisticalTests: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
