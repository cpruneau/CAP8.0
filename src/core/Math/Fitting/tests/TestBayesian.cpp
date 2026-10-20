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
//! @file TestBayesian.cpp
//! @brief Test program of Prior, MetropolisHastings and BayesianFitter (run by ctest).
//
// TestBayesian -- tests of the Bayesian fits against exact results: priors (normalizations), the sampler on a known
// correlated gaussian and on a bounded density (moments, intervals, diagnostics, reproducibility), the posterior of a
// straight line with flat priors (the least-squares solution), the conjugate gaussian prior of a constant (exact
// posterior), bounds, log-uniform priors, a Poisson posterior, and errors. Prints each failed check and exits with
// the number of failures (0: all passed).
//
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include "BayesianFitter.hpp"
#include "Constant.hpp"
#include "DataPointsVector.hpp"
#include "Histogram.hpp"
#include "Linear.hpp"
#include "MathExceptions.hpp"
#include "Normal.hpp"

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

bool within(double a, double b, double tolerance) { return std::fabs(a - b) <= tolerance; }

std::string show(const BayesianResult & r)
{
  std::string s = " [" + std::string(r.status.data()) + ";";
  for (std::size_t k=0; k<r.means.size(); k++) { char t[80]; std::snprintf(t, sizeof(t), " %.6g+-%.4g", r.means[k], r.standardDeviations[k]); s += t; }
  char t[96];
  std::snprintf(t, sizeof(t), "; acceptance %.3f]", r.acceptance);
  s += t;
  for (std::size_t k=0; k<r.rHats.size(); k++) { char u[64]; std::snprintf(u, sizeof(u), " R %.4f ESS %.0f", r.rHats[k], r.effectiveSampleSizes[k]); s += u; }
  return s;
}

std::string show(const McmcResult & r)
{
  char t[256];
  std::snprintf(t, sizeof(t), " [means %.4f %.4f, sd %.4f %.4f, acceptance %.3f, R %.4f, ESS %.0f]", r.mean(0), r.dimension > 1 ? r.mean(1) : 0.0,
                r.standardDeviation(0), r.dimension > 1 ? r.standardDeviation(1) : 0.0, r.acceptance, r.rHat(0), r.effectiveSampleSize(0));
  return t;
}

struct Random
{
  unsigned long long seed;
  explicit Random(unsigned long long s) : seed(s) { }
  double uniform() { seed = seed*6364136223846793005ULL + 1442695040888963407ULL; return ((seed >> 11) + 0.5)/9007199254740992.0; }
  double gaussian() { return std::sqrt(-2*std::log(uniform()))*std::cos(2*M_PI*uniform()); }
};
} // namespace

//! @brief Count a check, and report it if the condition is false.
#define CHECK(condition) check((condition), #condition, __LINE__)
//! @brief Count a check about a sampling, and report it if the condition is false.
#define CHECK_SAMPLE(condition, result) check((condition), std::string(#condition) + show(result), __LINE__)

//! @brief Run the checks. @return number of failed checks (0: all passed)
int main()
{
  // ---- priors
  {
    Prior flat;
    CHECK(flat.type() == Prior::kFlat && flat.logDensity(1E10) == 0);
    Prior u = Prior::uniform(1.0, 3.0);
    CHECK(within(u.logDensity(2.0), -std::log(2.0), 1E-15) && std::isinf(u.logDensity(3.5)) && u.name() == "uniform(1, 3)");
    Prior g = Prior::gaussian(1.0, 0.5);
    CHECK(within(g.logDensity(1.0), -std::log(0.5*std::sqrt(2*M_PI)), 1E-15) && within(g.logDensity(2.0) - g.logDensity(1.0), -2.0, 1E-12));
    // the log-uniform density integrates to 1 over its range
    Prior l = Prior::logUniform(0.1, 10.0);
    double integral = 0;
    for (int k=0; k<100000; k++) { double x = 0.1 + (k + 0.5)*9.9/100000; integral += std::exp(l.logDensity(x))*9.9/100000; }
    CHECK(within(integral, 1.0, 1E-6) && std::isinf(l.logDensity(-1.0)));
    Prior c = Prior::custom([](double x) { return -std::fabs(x); }, "laplace");
    CHECK(c.type() == Prior::kCustom && c.logDensity(-2.0) == -2.0 && c.name() == "laplace");
    bool thrown = false;
    try { Prior::gaussian(0.0, -1.0); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);
  }

  // ---- the sampler on a correlated gaussian: means (1, -2), widths (1, 2), correlation 0.8
  {
    const double rho = 0.8, sx = 1.0, sy = 2.0;
    auto logDensity = [&](const std::vector<double> & v)
      {
      double x = (v[0] - 1)/sx, y = (v[1] + 2)/sy;
      return -0.5*(x*x - 2*rho*x*y + y*y)/(1 - rho*rho);
      };
    MetropolisHastings sampler;
    McmcResult r = sampler.sample(logDensity, {0.0, 0.0}, {1.0, 1.0});
    double essX = r.effectiveSampleSize(0), essY = r.effectiveSampleSize(1);
    CHECK_SAMPLE(r.samples.size() == 4*5000 && r.dimension == 2 && r.nChains == 4, r);
    CHECK_SAMPLE(within(r.mean(0), 1.0, 4*sx/std::sqrt(essX)) && within(r.mean(1), -2.0, 4*sy/std::sqrt(essY)), r);
    CHECK_SAMPLE(within(r.standardDeviation(0), sx, 0.05*sx) && within(r.standardDeviation(1), sy, 0.05*sy), r);
    CHECK_SAMPLE(within(r.correlation()(0, 1), rho, 0.03), r);
    CHECK_SAMPLE(r.rHat(0) < 1.02 && r.rHat(1) < 1.02 && essX > 1000 && r.acceptance > 0.15 && r.acceptance < 0.5, r);
    auto i68 = r.interval(0, 0.6827);
    CHECK_SAMPLE(within(i68.first, 1.0 - sx, 0.08) && within(i68.second, 1.0 + sx, 0.08) && within(r.median(1), -2.0, 0.1), r);
    std::vector<double> mode = r.mode();
    CHECK_SAMPLE(within(mode[0], 1.0, 0.2) && within(mode[1], -2.0, 0.4), r);
    McmcResult again = sampler.sample(logDensity, {0.0, 0.0}, {1.0, 1.0});
    CHECK(again.samples == r.samples);                                      // the same seed: the same samples
    sampler.setSeed(99);
    CHECK(sampler.sample(logDensity, {0.0, 0.0}, {1.0, 1.0}).samples != r.samples);
    // a bounded density: uniform over [0, 1]
    MetropolisHastings bounded;
    bounded.setChains(2);
    McmcResult b = bounded.sample([](const std::vector<double> & v) { return (v[0] >= 0 && v[0] <= 1) ? 0.0 : -std::numeric_limits<double>::infinity(); }, {0.5}, {0.3});
    bool inside = true;
    for (const auto & x : b.samples) inside = inside && x[0] >= 0 && x[0] <= 1;
    CHECK_SAMPLE(inside && within(b.mean(0), 0.5, 0.02) && within(b.standardDeviation(0), 1/std::sqrt(12.0), 0.02), b);
    bool thrown = false;
    try { bounded.sample([](const std::vector<double> &) { return -std::numeric_limits<double>::infinity(); }, {0.5}, {0.3}); }
    catch (const MathException &) { thrown = true; }
    CHECK(thrown);                                                          // nowhere positive
  }

  // ---- a straight line with flat priors: the posterior is the least-squares gaussian
  {
    DataPointsVector data;
    data.addPoints({0.0, 1.0, 2.0, 3.0, 4.0, 5.0}, {1.1, 2.9, 5.2, 6.8, 9.1, 11.3}, {0.1, 0.2, 0.1, 0.3, 0.2, 0.4});
    Linear line(0.0, 1.0);
    FitResult ls = Fitter(line, data).fit();
    Linear model(0.0, 1.0);
    BayesianFitter bayes(model, data);
    BayesianResult r = bayes.sample();
    CHECK_SAMPLE(r.valid && r.status == "sampled" && r.freeParameters.size() == 2, r);
    for (unsigned int k=0; k<2; k++)
      {
      check(within(r.means[k], ls.values[k], 0.1*ls.errors[k]), "posterior mean = least squares" + show(r), __LINE__);
      check(within(r.standardDeviations[k], ls.errors[k], 0.05*ls.errors[k]), "posterior width = least-squares error" + show(r), __LINE__);
      check(within(r.medians[k], ls.values[k], 0.1*ls.errors[k]) && r.low68[k] < ls.values[k] && r.high68[k] > ls.values[k]
            && r.low95[k] < r.low68[k] && r.high95[k] > r.high68[k], "credible intervals" + show(r), __LINE__);
      }
    CHECK_SAMPLE(within(r.correlation(0, 1), ls.correlation(0, 1), 0.03) && r.rHats[0] < 1.02 && r.effectiveSampleSizes[0] > 1000, r);
    CHECK(model.parameterValues()[0] == r.means[0]);                        // the function is left at the posterior means
  }

  // ---- a constant with a gaussian prior: the exact (conjugate) posterior
  {
    DataPointsVector data;
    Random random(5);
    for (int k=0; k<8; k++) data.addPoint(DataPoint2D(double(k), 3.0 + 0.5*random.gaussian(), 0.5));
    double sum = 0, weights = 0;
    for (unsigned int k=0; k<data.size(); k++) { double w = 1.0/(0.5*0.5); weights += w; sum += w*data.point(k).y(); }
    const double mu0 = 2.0, tau = 0.3;
    double precision = weights + 1/(tau*tau), mean = (sum + mu0/(tau*tau))/precision, sigma = 1/std::sqrt(precision);
    Constant c(1.0);
    BayesianFitter bayes(c, data);
    bayes.setPrior(0, Prior::gaussian(mu0, tau));
    BayesianResult r = bayes.sample();
    CHECK_SAMPLE(within(r.means[0], mean, 4*sigma/std::sqrt(r.effectiveSampleSizes[0])) && within(r.standardDeviations[0], sigma, 0.04*sigma), r);
    bayes.setStartFromFit(false);                                           // from the current values (the posterior mean)
    c.setParameterValue(0, 0.0);
    r = bayes.sample();
    CHECK_SAMPLE(r.valid && within(r.means[0], mean, 4*sigma/std::sqrt(r.effectiveSampleSizes[0])), r);
  }

  // ---- bounds and log-uniform priors; a Poisson posterior
  {
    Histogram h("h", "h", 40, -4.0, 4.0);
    Random random(17);
    for (int k=0; k<300; k++) h.fill(0.2 + 0.9*random.gaussian());
    Normal model(10.0, 0.0, 1.0);
    FitResult mle = Fitter(model, h, Fitter::kPoisson).fit();
    Normal bayesModel(10.0, 0.0, 1.0);
    BayesianFitter bayes(bayesModel, h, Fitter::kPoisson);
    bayes.setPrior(2, Prior::logUniform(0.05, 20.0));
    BayesianResult r = bayes.sample();
    CHECK_SAMPLE(r.valid && within(r.means[1], mle.values[1], 0.3*mle.errors[1]) && within(r.standardDeviations[1], mle.errors[1], 0.2*mle.errors[1]), r);
    bool positive = true;
    for (const auto & s : r.chain.samples) positive = positive && s[2] >= 0.05 && s[2] <= 20.0;
    CHECK_SAMPLE(positive && r.rHats[1] < 1.05, r);
    // a mean bounded just above the estimate: the posterior is cut at the bound
    Normal limited(10.0, 0.0, 1.0);
    limited.setParameterLimits(1, -1.0, mle.values[1] + 0.5*mle.errors[1]);
    BayesianFitter cut(limited, h, Fitter::kPoisson);
    BayesianResult rc = cut.sample();
    double highest = -1E9;
    for (const auto & s : rc.chain.samples) highest = std::max(highest, s[1]);
    CHECK_SAMPLE(rc.valid && highest <= mle.values[1] + 0.5*mle.errors[1] && rc.means[1] < mle.values[1], rc);
    // a start where the prior is zero, without a fit: no sampling
    Normal off(10.0, 0.0, 1.0);
    BayesianFitter impossible(off, h, Fitter::kPoisson);
    impossible.setPrior(1, Prior::uniform(5.0, 6.0));
    impossible.setStartFromFit(false);
    BayesianResult ri = impossible.sample();
    CHECK(!ri.valid && ri.status.contains("zero"));
  }

  std::printf("TestBayesian: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
