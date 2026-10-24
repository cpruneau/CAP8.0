/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestRandom.cpp
//! @brief Tests of the random numbers: the Mersenne Twister against the reference implementation (std::mt19937 and
//! the published output of mt19937ar.c), and the moments of each distribution (within 5 standard errors).
#include <cmath>
#include <cstdio>
#include <functional>
#include <random>
#include <string>
#include "MersenneTwister.hpp"
#include "RandomDistribution.hpp"
#include "MathExceptions.hpp"

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

//! Mean and variance of n numbers of a generator.
struct Moments { double mean = 0, variance = 0; };
Moments moments(const std::function<double()> & draw, int n)
{
  double s = 0, s2 = 0;
  for (int k=0; k<n; k++) { double x = draw(); s += x; s2 += x*x; }
  Moments m;
  m.mean = s/n;
  m.variance = s2/n - m.mean*m.mean;
  return m;
}

//! Whether a sample mean agrees with the expected mean and variance (5 standard errors), and the variance too
//! (within 5%, enough for the sizes used).
bool agrees(const Moments & m, double mean, double variance, int n)
{
  double error = std::sqrt(variance/n);
  bool ok = std::fabs(m.mean - mean) < 5*error + 1e-12 && std::fabs(m.variance - variance) < 0.05*variance + 1e-12;
  if (!ok) std::printf("  mean %.6g (expected %.6g +- %.2g), variance %.6g (expected %.6g)\n", m.mean, mean, error, m.variance, variance);
  return ok;
}

template <class F> bool throws(F f) { try { f(); } catch (const MathException &) { return true; } return false; }
} // namespace

#define CHECK(condition) check((condition), #condition, __LINE__)

int main()
{
  const int n = 400000;

  // ---- the engine: as std::mt19937 and the reference implementation
  {
    for (unsigned seed : { 5489u, 1u, 12345u, 4294967295u })
      {
      MersenneTwister mt(seed);
      std::mt19937 reference(seed);
      bool same = true;
      for (int k=0; k<20000; k++) same = same && mt.next32() == reference();
      CHECK(same);
      }
    MersenneTwister mt;
    for (int k=0; k<9999; k++) mt.next32();
    CHECK(mt.next32() == 4123659995u);                                   // the 10000th number for the default seed
    mt.setSeeds({ 0x123, 0x234, 0x345, 0x456 });                         // mt19937ar.out
    CHECK(mt.next32() == 1067595299u && mt.next32() == 955945823u && mt.next32() == 477289528u);
    CHECK(mt.next32() == 4107218783u && mt.next32() == 4228976476u);
  }

  // ---- reproducibility: seeds, states
  {
    MersenneTwister a(77), b(77), c(78);
    CHECK(a.random() == b.random() && a.gauss() == b.gauss() && a.random() != c.random());
    std::vector<std::uint32_t> saved = a.state();
    double x1 = a.random(), x2 = a.random();
    a.setState(saved);
    CHECK(a.random() == x1 && a.random() == x2);
    MersenneTwister d(0), e(0);                                          // seeds of their own: different
    CHECK(d.seed() != e.seed() && d.seed() != 0);
    CHECK(throws([&]() { a.setState({ 1, 2, 3 }); }));
  }

  MersenneTwister g(2026);

  // ---- uniform numbers
  {
    double low = 1.0, high = 0.0;
    Moments m = moments([&]() { double x = g.random(); low = std::min(low, x); high = std::max(high, x); return x; }, n);
    CHECK(low >= 0.0 && high < 1.0);
    CHECK(agrees(m, 0.5, 1.0/12.0, n));
    CHECK(agrees(moments([&]() { return g.uniform(-2.0, 6.0); }, n), 2.0, 64.0/12.0, n));
    bool inRange = true;
    Moments mi = moments([&]() { long k = g.uniformInteger(1, 6); inRange = inRange && k >= 1 && k <= 6; return double(k); }, n);
    CHECK(inRange && agrees(mi, 3.5, 35.0/12.0, n));
  }

  // ---- Gaussian, exponential
  {
    CHECK(agrees(moments([&]() { return g.gauss(3.0, 2.0); }, n), 3.0, 4.0, n));
    // fraction beyond 2 sigma: 4.55%
    int beyond = 0;
    for (int k=0; k<n; k++) if (std::fabs(g.gauss()) > 2.0) beyond++;
    CHECK(std::fabs(beyond/double(n) - 0.0455) < 5*std::sqrt(0.0455*0.9545/n));
    CHECK(agrees(moments([&]() { return g.exp(2.0); }, n), 2.0, 4.0, n));
    // truncated to [1,3], mean 1: mean = 1 + 1 - 2 e^-2/(1 - e^-2)
    double t = std::exp(-2.0), mean = 2.0 - 2.0*t/(1.0 - t);
    double second = 0;                                                   // E[(x-1)^2] for an exponential truncated at 2
    second = (2.0 - t*(4.0 + 4.0 + 2.0))/(1.0 - t);
    double variance = second - (mean - 1.0)*(mean - 1.0);
    bool within = true;
    Moments me = moments([&]() { double x = g.exp(1.0, 1.0, 3.0); within = within && x >= 1.0 && x <= 3.0; return x; }, n);
    CHECK(within && agrees(me, mean, variance, n));
    CHECK(throws([&]() { g.exp(-1.0); }) && throws([&]() { g.exp(1.0, 2.0, 1.0); }));
  }

  // ---- Poisson, binomial
  {
    for (double mu : { 0.3, 3.5, 29.0, 31.0, 100.5, 2500.0 })
      CHECK(agrees(moments([&]() { return double(g.poisson(mu)); }, n), mu, mu, n));
    int zeros = 0;
    for (int k=0; k<n; k++) if (g.poisson(3.5) == 0) zeros++;
    CHECK(std::fabs(zeros/double(n) - std::exp(-3.5)) < 5*std::sqrt(std::exp(-3.5)/n));
    CHECK(g.poisson(0.0) == 0 && throws([&]() { g.poisson(-1.0); }));
    struct Case { long trials; double p; };
    for (Case c : { Case{ 20, 0.3 }, Case{ 20, 0.9 }, Case{ 1000, 0.4 }, Case{ 100000, 0.001 }, Case{ 1000000, 0.5 } })
      CHECK(agrees(moments([&]() { return double(g.binomial(c.trials, c.p)); }, n), c.trials*c.p, c.trials*c.p*(1 - c.p), n));
    CHECK(g.binomial(10, 0.0) == 0 && g.binomial(10, 1.0) == 10 && g.binomial(0, 0.5) == 0);
    CHECK(throws([&]() { g.binomial(10, 1.5); }) && throws([&]() { g.binomial(-1, 0.5); }));
  }

  // ---- gamma, beta, chi-square
  {
    CHECK(agrees(moments([&]() { return g.gamma(2.5, 2.0); }, n), 5.0, 10.0, n));
    CHECK(agrees(moments([&]() { return g.gamma(0.4); }, n), 0.4, 0.4, n));
    CHECK(agrees(moments([&]() { return g.beta(2.0, 5.0); }, n), 2.0/7.0, 10.0/(49.0*8.0), n));
    CHECK(agrees(moments([&]() { return g.chiSquare(5.0); }, n), 5.0, 10.0, n));
    CHECK(agrees(moments([&]() { return g.chiSquare(1.0); }, n), 1.0, 2.0, n));
    CHECK(throws([&]() { g.gamma(0.0); }) && throws([&]() { g.chiSquare(-2.0); }));
  }

  // ---- polynomials
  {
    // (1 + 3x^2)/2 on [0,1]: mean 5/8, E[x^2] = (1/3 + 3/5)/2 = 7/15
    bool within = true;
    Moments m = moments([&]() { double x = g.polynomial({ 1.0, 0.0, 3.0 }, 0.0, 1.0); within = within && x >= 0 && x <= 1; return x; }, n);
    CHECK(within && agrees(m, 0.625, 7.0/15.0 - 0.625*0.625, n));
    // linear 2 - x on [0,2]: density (2 - x)/2, mean 2/3, variance 2/9
    CHECK(agrees(moments([&]() { return g.polynomial({ 2.0, -1.0 }, 0.0, 2.0); }, n), 2.0/3.0, 2.0/9.0, n));
    CHECK(throws([&]() { g.polynomial({ 1.0, -2.0 }, 0.0, 1.0); }));       // negative beyond x = 0.5
    CHECK(throws([&]() { g.polynomial({}, 0.0, 1.0); }));
  }

  // ---- distributions added from outside
  {
    TabulatedDistribution d([](double x) { return x*x*std::exp(-x); }, 0.0, 60.0);   // gamma of shape 3
    CHECK(std::fabs(d.integral() - 2.0) < 1e-6);
    CHECK(agrees(moments([&]() { return g.sample(d); }, n), 3.0, 3.0, n));
    struct Cauchy : public RandomDistribution
    {
      double generate(RandomGenerator & r) const override { return std::tan(M_PI*(r.random() - 0.5)); }
    };
    int inside = 0;
    for (int k=0; k<n; k++) if (std::fabs(g.sample(Cauchy())) < 1.0) inside++;   // half of a Cauchy within its width
    CHECK(std::fabs(inside/double(n) - 0.5) < 5*std::sqrt(0.25/n));
    CHECK(throws([]() { TabulatedDistribution([](double x) { return x - 1.0; }, 0.0, 2.0); }));
  }

  std::printf("TestRandom: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
