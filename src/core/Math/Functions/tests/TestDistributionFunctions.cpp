/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestDistributionFunctions.cpp
//! @brief Test program of the special functions and of the distributions (run by ctest).
//
// TestDistributionFunctions -- the gamma, beta and incomplete gamma functions against known values; the distributions
// of counts summed over the counts (amplitude, mean, variance, cumulative distribution) and against each other
// (binomial and hypergeometric, negative binomial and Poisson, multinomial and binomial); the densities integrated
// numerically (area, integralInRange(), mean, variance) and against each other (Student of 1 degree of freedom and
// Cauchy, chi square and incomplete gamma, Weibull of shape 1 and exponential, Dirichlet of 2 categories and beta);
// the registry and records. Prints each failed check and exits with the number of failures.
//
#include <cmath>
#include <cstdio>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "BinaryIO.hpp"
#include "ContinuousDistributions.hpp"
#include "DiscreteDistributions.hpp"
#include "Distributions.hpp"
#include "FunctionRegistry.hpp"
#include "MultivariateDistributions.hpp"
#include "SpecialFunctions.hpp"

using namespace CAP;
using namespace CAP::MATH;

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

#define CHECK(condition) check((condition), #condition, __LINE__)

bool close(double a, double b, double tolerance) { return std::fabs(a - b) <= tolerance*(1.0 + std::fabs(b)); }

template <typename F>
bool throws(F f) { try { f(); } catch (const std::exception &) { return true; } return false; }

//! Simpson integral of g over [a, b] with n intervals (n even).
double simpson(const std::function<double(double)> & g, double a, double b, int n=20000)
{
  double h = (b - a)/n, sum = g(a) + g(b);
  for (int i=1; i<n; i++) sum += (i%2 ? 4.0 : 2.0)*g(a + i*h);
  return sum*h/3.0;
}

//! Sum of the counts 0 to kMax of f, of k f and of k^2 f.
void moments(const DiscreteDistribution & f, unsigned long kMax, double & sum, double & mean, double & variance)
{
  sum = 0.0; mean = 0.0; double second = 0.0;
  for (unsigned long k=0; k<=kMax; k++) { double p = f.evaluateAt(double(k)); sum += p; mean += k*p; second += double(k)*k*p; }
  mean /= sum;
  variance = second/sum - mean*mean;
}

std::unique_ptr<ParameterizedFunction> roundTrip(const ParameterizedFunction & f)
{
  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  writeBinaryObject(stream, f);
  stream.seekg(0);
  return std::unique_ptr<ParameterizedFunction>(ParameterizedFunction::readBinary(stream));
}
} // namespace

int main()
{
  const double pi = 3.14159265358979323846;

  // ---- special functions
  GammaFunction gamma;
  CHECK(gamma.rank() == 1 && close(gamma.evaluateAt(5.0), 24.0, 1.0E-14) && close(gamma.evaluateAt(0.5), std::sqrt(pi), 1.0E-14));
  CHECK(close(gamma.evaluateAt(-0.5), -2.0*std::sqrt(pi), 1.0E-14) && std::isfinite(GammaFunction::logGamma(500.0)));
  CHECK(throws([&] { gamma.evaluateAt(std::vector<double>{1.0, 2.0}); }));
  BetaFunction beta;
  CHECK(beta.rank() == 2 && close(beta.evaluateAt(2.0, 3.0), 1.0/12.0, 1.0E-14) && close(beta.evaluateAt(0.5, 0.5), pi, 1.0E-13));
  CHECK(close(beta.evaluateAt(std::vector<double>{3.5, 1.2}), beta.evaluateAt(1.2, 3.5), 1.0E-14));
  CHECK(close(BetaFunction::beta(-0.5, 2.0), std::tgamma(-0.5)*std::tgamma(2.0)/std::tgamma(1.5), 1.0E-13));
  IncompleteGammaFunction p, q(IncompleteGammaFunction::kUpperRegularized), lower(IncompleteGammaFunction::kLower),
                          upper(IncompleteGammaFunction::kUpper);
  for (double x : {0.1, 1.0, 2.5, 10.0})
    {
    CHECK(close(p.evaluateAt(1.0, x), 1.0 - std::exp(-x), 1.0E-12));
    CHECK(close(p.evaluateAt(3.2, x) + q.evaluateAt(3.2, x), 1.0, 1.0E-12));
    CHECK(close(lower.evaluateAt(2.0, x), 1.0 - (1.0 + x)*std::exp(-x), 1.0E-12));
    CHECK(close(lower.evaluateAt(2.7, x) + upper.evaluateAt(2.7, x), std::tgamma(2.7), 1.0E-12));
    }
  CHECK(close(upper.evaluateAt(4.0, 0.0), 6.0, 1.0E-12) && throws([&] { p.evaluateAt(-1.0, 1.0); }));

  // ---- counts: sums, means, variances, cumulative distributions
  double sum, mean, variance;
  PoissonDistribution poisson(2.0, 3.7);
  moments(poisson, 100, sum, mean, variance);
  CHECK(close(sum, 2.0, 1.0E-12) && close(mean, 3.7, 1.0E-10) && close(variance, 3.7, 1.0E-9));
  CHECK(close(poisson.probability(4), Distributions::poissonProbability(4, 3.7), 1.0E-12));
  CHECK(close(poisson.cdf(5), poisson.probability(0) + poisson.probability(1) + poisson.probability(2)
                              + poisson.probability(3) + poisson.probability(4) + poisson.probability(5), 1.0E-12));
  CHECK(poisson.evaluateAt(-0.5) == 0.0 && poisson.evaluateAt(2.5) > 0.0);
  CHECK(close(PoissonDistribution(1.0, 0.0).evaluateAt(0.0), 1.0, 0.0) && PoissonDistribution(1.0, 0.0).evaluateAt(1.0) == 0.0);

  BinomialDistribution binomial(1.0, 12.0, 0.3);
  moments(binomial, 12, sum, mean, variance);
  CHECK(close(sum, 1.0, 1.0E-12) && close(mean, 3.6, 1.0E-12) && close(variance, 12*0.3*0.7, 1.0E-11));
  CHECK(close(binomial.probability(5), Distributions::binomialProbability(5, 12, 0.3), 1.0E-12));
  CHECK(close(binomial.cdf(4), Distributions::binomialCdf(4, 12, 0.3), 1.0E-12) && binomial.evaluateAt(13.0) == 0.0);
  CHECK(binomial.parameter(1).type() == Parameter::Fixed && close(binomial.mean(), 3.6, 1.0E-15));
  CHECK(close(BinomialDistribution(1.0, 5.0, 0.0).evaluateAt(0.0), 1.0, 0.0) && BinomialDistribution(1.0, 5.0, 1.0).evaluateAt(5.0) == 1.0);

  NegativeBinomialDistribution nbd(1.0, 5.0, 2.0);
  moments(nbd, 2000, sum, mean, variance);
  CHECK(close(sum, 1.0, 1.0E-12) && close(mean, 5.0, 1.0E-10) && close(variance, 5.0 + 25.0/2.0, 1.0E-9));
  CHECK(close(nbd.variance(), 17.5, 1.0E-15));
  double partial = 0.0;
  for (unsigned long k=0; k<=4; k++) partial += nbd.probability(k);
  CHECK(close(nbd.cdf(4), partial, 1.0E-12));
  NegativeBinomialDistribution nearPoisson(1.0, 3.7, 1.0E7);
  for (unsigned long k : {0ul, 2ul, 5ul, 9ul}) CHECK(close(nearPoisson.probability(k), poisson.probability(k), 1.0E-5));

  HypergeometricDistribution hyper(1.0, 50.0, 10.0, 12.0);
  moments(hyper, 12, sum, mean, variance);
  CHECK(close(sum, 1.0, 1.0E-12) && close(mean, hyper.mean(), 1.0E-12) && close(variance, hyper.variance(), 1.0E-11));
  CHECK(close(hyper.mean(), 12.0*10.0/50.0, 1.0E-15) && hyper.evaluateAt(11.0) == 0.0);
  HypergeometricDistribution large(1.0, 1.0E7, 3.0E6, 12.0);
  for (unsigned long k : {0ul, 3ul, 6ul}) CHECK(close(large.probability(k), binomial.probability(k), 1.0E-5));
  CHECK(throws([] { HypergeometricDistribution bad(1.0, 10.0, 11.0, 2.0); }));

  // ---- densities: areas, integralInRange(), means, variances
  auto checkDensity = [&](const ContinuousDistribution & f, double low, double high, double meanRange, double tolerance)
    {
    CHECK(close(f.integralInRange(-INFINITY, INFINITY), f.amplitude(), 1.0E-10));
    double numeric = simpson([&](double x) { return f.evaluateAt(x); }, low, high);
    CHECK(close(f.integralInRange(low, high), numeric, tolerance));
    CHECK(close(f.integralInRange(high, low), -numeric, tolerance));
    CHECK(close(f.amplitude()*f.cdf(high), f.integralInRange(-INFINITY, high), 1.0E-10));
    CHECK(close(f.cdf(high) + f.cdfUpper(high), 1.0, 1.0E-12));
    if (std::isfinite(f.mean()))
      {
      double lowM = std::isfinite(meanRange) ? -meanRange : 0.0, m0 = 0.0, m1 = 0.0, m2 = 0.0;
      if (dynamic_cast<const StudentDistribution *>(&f) == nullptr) lowM = 0.0;
      m0 = simpson([&](double x) { return f.density(x); }, lowM, meanRange, 400000);
      m1 = simpson([&](double x) { return x*f.density(x); }, lowM, meanRange, 400000);
      m2 = simpson([&](double x) { return x*x*f.density(x); }, lowM, meanRange, 400000);
      CHECK(close(m0, 1.0, 1.0E-6) && close(m1, f.mean(), 1.0E-5));
      if (!(close(m0, 1.0, 1.0E-6) && close(m1, f.mean(), 1.0E-5))) std::printf("  %s: %.10g %.10g %.10g\n", FunctionRegistry::typeName(f).data(), m0, m1, f.mean());
      if (std::isfinite(f.variance())) CHECK(close(m2 - m1*m1, f.variance(), 1.0E-4));
      }
    };

  StudentDistribution student(3.0, 8.0, 1.0, 2.0);
  checkDensity(student, -1.0, 4.0, 2000.0, 1.0E-10);
  StudentDistribution student1(1.0, 1.0, 0.5, 2.0);
  CauchyDistribution cauchy(1.0, 0.5, 2.0);
  for (double x : {-7.0, 0.0, 0.5, 3.0}) CHECK(close(student1.evaluateAt(x), cauchy.evaluateAt(x), 1.0E-13));
  CHECK(close(student1.cdf(3.0), cauchy.cdf(3.0), 1.0E-10) && std::isnan(student1.mean()));
  CHECK(std::isinf(StudentDistribution(1.0, 1.5).variance()));

  ChiSquareDistribution chi2(2.0, 4.0);
  checkDensity(chi2, 0.5, 9.0, 200.0, 1.0E-10);
  for (double x : {0.3, 2.0, 7.0}) CHECK(close(chi2.density(x), x*std::exp(-0.5*x)/4.0, 1.0E-13));
  CHECK(close(chi2.cdf(3.3), p.evaluateAt(2.0, 1.65), 1.0E-12) && chi2.mode() == 2.0 && chi2.density(-1.0) == 0.0);
  CHECK(std::isinf(ChiSquareDistribution(1.0, 1.0).density(0.0)) && close(ChiSquareDistribution(1.0, 2.0).density(0.0), 0.5, 0.0));

  FDistribution fdist(1.0, 5.0, 12.0);
  checkDensity(fdist, 0.2, 3.0, 2000.0, 1.0E-10);
  CHECK(close(fdist.mean(), 12.0/10.0, 1.0E-15) && std::isnan(FDistribution(1.0, 3.0, 2.0).mean()));

  checkDensity(cauchy, -3.0, 6.0, NAN, 1.0E-10);
  for (double x : {-2.0, 0.4, 3.0}) CHECK(close(cauchy.derivativeAt(x), (cauchy(x + 1.0E-6) - cauchy(x - 1.0E-6))/2.0E-6, 1.0E-6));
  CHECK(close(cauchy.cdfUpper(1.0E8), 2.0/(pi*1.0E8), 1.0E-6) && close(cauchy.evaluateAt(0.5), 1.0/(2.0*pi), 1.0E-15));

  MaxwellBoltzmannDistribution maxwell(1.0, 1.5);
  checkDensity(maxwell, 0.3, 4.0, 30.0, 1.0E-10);
  CHECK(maxwell.density(maxwell.mode()) > maxwell.density(maxwell.mode() + 1.0E-3)
        && maxwell.density(maxwell.mode()) > maxwell.density(maxwell.mode() - 1.0E-3));

  WeibullDistribution weibull(1.0, 2.0, 1.3);
  checkDensity(weibull, 0.1, 2.0, 30.0, 1.0E-10);
  WeibullDistribution exponential(1.0, 1.0, 2.0);
  for (double x : {0.0, 0.5, 4.0}) CHECK(close(exponential.density(x), std::exp(-x/2.0)/2.0, 1.0E-14));
  CHECK(close(exponential.mean(), 2.0, 1.0E-14) && close(exponential.variance(), 4.0, 1.0E-13));
  CHECK(throws([] { WeibullDistribution bad(1.0, 0.0, 1.0); }) && throws([] { StudentDistribution bad(1.0, 2.0, 0.0, -1.0); }));

  // ---- multinomial and Dirichlet
  MultinomialDistribution two(12.0, {0.3, 0.7});
  CHECK(two.rank() == 1 && two.nCategories() == 2);
  for (double k : {0.0, 3.0, 7.0, 12.0}) CHECK(close(two.evaluateAt(k), binomial.evaluateAt(k), 1.0E-12));
  MultinomialDistribution three(6.0, {0.2, 0.3, 0.5}, 4.0);
  CHECK(three.rank() == 2 && close(three.probability(2), 0.5, 1.0E-15));
  double total = 0.0, m1 = 0.0, c12 = 0.0, m2 = 0.0;
  for (int k1=0; k1<=6; k1++)
    for (int k2=0; k1+k2<=6; k2++)
      {
      double f = three.evaluateAt(double(k1), double(k2));
      total += f; m1 += k1*f; m2 += k2*f; c12 += k1*k2*f;
      }
  m1 /= total; m2 /= total;
  CHECK(close(total, 4.0, 1.0E-12) && close(m1, three.mean(0), 1.0E-12) && close(c12/total - m1*m2, three.covariance(0, 1), 1.0E-11));
  CHECK(close(three.evaluateAt(2.0, 1.0), 4.0*720.0/(2.0*1.0*6.0)*0.04*0.3*0.125, 1.0E-12) && three.evaluateAt(4.0, 3.0) == 0.0);
  CHECK(throws([] { MultinomialDistribution bad(5.0, {0.2, 0.2}); }));

  DirichletDistribution betaLike({2.5, 4.0});
  CHECK(betaLike.rank() == 1);
  for (double x : {0.1, 0.4, 0.8})
    CHECK(close(betaLike.evaluateAt(x), std::pow(x, 1.5)*std::pow(1.0 - x, 3.0)/BetaFunction::beta(2.5, 4.0), 1.0E-12));
  DirichletDistribution dirichlet({2.0, 3.0, 4.0}, 3.0);
  int n = 600;
  double area = 0.0, mx = 0.0, h = 1.0/n;
  for (int i=0; i<n; i++)
    for (int j=0; i+j<n; j++)
      {
      double x1 = (i + 0.5)*h, x2 = (j + 0.5)*h;
      if (x1 + x2 >= 1.0) continue;
      double f = dirichlet.evaluateAt(x1, x2)*h*h;
      area += f; mx += x1*f;
      }
  CHECK(close(area, 3.0, 2.0E-3) && close(mx/area, dirichlet.mean(0), 2.0E-3) && close(dirichlet.mean(0), 2.0/9.0, 1.0E-15));
  CHECK(dirichlet.evaluateAt(0.7, 0.4) == 0.0 && dirichlet.evaluateAt(-0.1, 0.4) == 0.0);
  CHECK(close(dirichlet.covariance(0, 1), -2.0*3.0/(81.0*10.0), 1.0E-15));
  CHECK(throws([] { DirichletDistribution bad({1.0, 0.0}); }));

  // ---- registry and records
  for (const char * name : {"Poisson", "Binomial", "NegativeBinomial", "Hypergeometric", "Student", "ChiSquare",
                            "FDistribution", "Cauchy", "MaxwellBoltzmann", "Weibull", "Multinomial", "Dirichlet"})
    {
    std::unique_ptr<ParameterizedFunction> made(FunctionRegistry::create(name));
    CHECK(made && FunctionRegistry::typeName(*made) == name);
    if (!made) continue;
    std::unique_ptr<ParameterizedFunction> back = roundTrip(*made);
    CHECK(back && FunctionRegistry::typeName(*back) == name && back->nParameters() == made->nParameters());
    if (back && back->rank() == 1) CHECK(close(back->evaluateAt(1.0), made->evaluateAt(1.0), 1.0E-14));
    }
  std::unique_ptr<ParameterizedFunction> four(FunctionRegistry::create("Multinomial", 5));
  CHECK(four && four->rank() == 3 && FunctionRegistry::create("Dirichlet", 2) == nullptr);
  std::unique_ptr<ParameterizedFunction> threeBack = roundTrip(three);
  CHECK(threeBack && threeBack->rank() == 2 && close(threeBack->evaluateAt(std::vector<double>{2.0, 1.0}), three.evaluateAt(2.0, 1.0), 1.0E-14));
  std::unique_ptr<ParameterizedFunction> binomialBack = roundTrip(binomial);
  CHECK(binomialBack && binomialBack->parameter(1).type() == Parameter::Fixed && binomialBack->parameterValues() == binomial.parameterValues());

  std::printf("TestDistributionFunctions: %d checks, %d failures\n", nChecks, nFailures);
  return nFailures;
}
