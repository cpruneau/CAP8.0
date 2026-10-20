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
//! @file TestLikelihood.cpp
//! @brief Test program of the maximum likelihood fits: binned Poisson and unbinned likelihoods (run by ctest).
//
// TestLikelihood -- tests of CAP::MATH::PoissonLikelihood and UnbinnedLikelihood through Fitter, against exact
// properties: the Baker-Cousins statistic, the sum of a Poisson fit with a free amplitude, the unbinned estimators of
// a Normal (sample mean and standard deviation, their uncertainties), the extended likelihood (number of events),
// and the likelihood equation of a truncated exponential. Prints each failed check and exits with the number of
// failures (0: all passed).
//
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include "DataPointsVector.hpp"
#include "Exponential.hpp"
#include "Fitter.hpp"
#include "Histogram.hpp"
#include "Likelihood.hpp"
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

bool close(double a, double b, double relative) { return std::fabs(a - b) <= relative*std::max(1.0, std::max(std::fabs(a), std::fabs(b))); }

std::string show(const FitResult & r)
{
  std::string s = " [" + std::string(r.method.data()) + ", " + r.statistic.data() + ": " + r.status.data() + ";";
  for (std::size_t k=0; k<r.values.size(); k++) { char t[64]; std::snprintf(t, sizeof(t), " %.8g+-%.4g", r.values[k], r.errors[k]); s += t; }
  char t[96];
  std::snprintf(t, sizeof(t), "; statistic %.6g, %d points, p %.3g]", r.chiSquare, r.nPoints, r.pValue);
  return s + t;
}

//! Random numbers of a fixed sequence.
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
//! @brief Count a check about a fit, and report the fit if the condition is false.
#define CHECK_FIT(condition, result) check((condition), std::string(#condition) + show(result), __LINE__)

//! @brief Run the checks. @return number of failed checks (0: all passed)
int main()
{
  // ---- the Baker-Cousins statistic and its residuals, by hand
  {
    DataPointsVector counts;
    counts.addPoints({0.0, 1.0, 2.0}, {0.0, 4.0, 9.0});
    Normal flat(1.0, 0.0, 1E9);                                              // practically constant: 1 everywhere
    PoissonLikelihood poisson(flat, counts);
    poisson.prepare();
    double expected = 2*1.0 + 2*(1 - 4 + 4*std::log(4.0)) + 2*(1 - 9 + 9*std::log(9.0));
    CHECK(poisson.nPoints() == 3 && close(poisson.statistic(), expected, 1E-9));   // the empty bin counts: 2 f
    std::vector<double> r;
    CHECK(poisson.residuals(r) && r.size() == 3 && r[0] < 0 && r[1] > 0 && close(r[0]*r[0] + r[1]*r[1] + r[2]*r[2], expected, 1E-9));
    CHECK(poisson.name() == "Poisson likelihood" && poisson.hasResiduals() && poisson.goodnessOfFit());
  }

  // ---- binned Poisson fit of a sparse histogram, compared with the chi square
  Random random(21);
  {
    Histogram h("h", "h", 60, -6.0, 6.0);
    for (int k=0; k<400; k++) h.fill(0.5 + 1.2*random.gaussian());
    double entries = 0;
    unsigned int empty = 0;
    for (unsigned int b=0; b<60; b++) { entries += h.valueAt(b); if (h.valueAt(b) == 0) empty++; }
    Normal poissonModel(10.0, 0.0, 1.0), chi2Model(10.0, 0.0, 1.0);
    FitResult p = Fitter(poissonModel, h, Fitter::kPoisson).fit();
    FitResult c = Fitter(chi2Model, h).fit();
    double sum = 0, chi2Sum = 0;
    for (unsigned int b=0; b<60; b++) { sum += poissonModel.evaluateAt(h.binCenter(b, 0)); chi2Sum += chi2Model.evaluateAt(h.binCenter(b, 0)); }
    CHECK_FIT(p.converged && p.validErrors && p.statistic == "Poisson likelihood" && p.nPoints == 60 && empty > 10, p);
    CHECK_FIT(close(sum, entries, 1E-6), p);                                // free amplitude: the fit sums to the entries
    CHECK_FIT(std::fabs(p.values[1] - 0.5) < 3*p.errors[1] && std::fabs(std::fabs(p.values[2]) - 1.2) < 3*p.errors[2], p);
    CHECK_FIT(p.pValue > 1E-3 && p.method == "Levenberg-Marquardt", p);
    CHECK_FIT(c.nPoints == int(60 - empty) && chi2Sum < 0.97*entries, c);  // the chi square underestimates sparse counts
    Normal searched(10.0, 0.0, 1.0);
    searched.setParameterLimits(0, 1.0, 100.0); searched.setParameterLimits(1, -3.0, 3.0); searched.setParameterLimits(2, 0.3, 4.0);
    Fitter fitter(searched, h, Fitter::kPoisson);
    fitter.setMethod(Fitter::kSearch);
    FitResult s = fitter.fit();
    CHECK_FIT(close(s.values[1], p.values[1], 1E-5) && close(s.chiSquare, p.chiSquare, 1E-7), s);
  }

  // ---- unbinned Normal: the estimators are the sample mean and the (population) standard deviation
  {
    const int n = 2000;
    std::vector<double> events;
    double mean = 0, spread = 0;
    for (int k=0; k<n; k++) { events.push_back(0.4 + 1.3*random.gaussian()); mean += events.back()/n; }
    for (double x : events) spread += (x - mean)*(x - mean)/n;
    spread = std::sqrt(spread);
    Normal model(1.0, 0.0, 1.0);
    model.fixParameter(0);                                                  // not extended: the amplitude does not matter
    UnbinnedLikelihood likelihood(model, events, -15.0, 15.0);
    FitResult r = Fitter(likelihood).fit();
    CHECK_FIT(r.validErrors && r.statistic == "unbinned likelihood" && r.nPoints == n && r.pValue == 0, r);
    CHECK_FIT(close(r.values[1], mean, 1E-5) && close(std::fabs(r.values[2]), spread, 1E-5), r);
    CHECK_FIT(close(r.errors[1], spread/std::sqrt(double(n)), 0.03) && close(r.errors[2], spread/std::sqrt(2.0*n), 0.03), r);
    CHECK_FIT(std::fabs(r.correlation(0, 1)) < 0.05, r);                    // mean and width are uncorrelated

    // extended: the integral of the function is the number of events, known to sqrt(n)
    Normal counted(500.0, 0.0, 1.0);
    UnbinnedLikelihood extended(counted, events, -15.0, 15.0, true);
    FitResult e = Fitter(extended).fit();
    double integral = counted.integralInRange(-15.0, 15.0);
    CHECK_FIT(extended.name() == "extended unbinned likelihood" && close(integral, n, 1E-4) && close(e.values[1], mean, 1E-5), e);
    double relative = e.errors[0]/e.values[0];                             // amplitude ~ n/(width sqrt(2 pi)): sqrt(1/n + ...)
    CHECK_FIT(relative > 0.9/std::sqrt(double(n)) && relative < 1.5/std::sqrt(double(n)), e);

    // a free amplitude without extension: no uncertainties (the amplitude is not determined)
    Normal free(1.0, 0.0, 1.0);
    UnbinnedLikelihood undetermined(free, events, -15.0, 15.0);
    FitResult u = Fitter(undetermined).fit();
    CHECK_FIT(!u.validErrors && close(u.values[1], mean, 1E-4), u);
    // events outside the range are left out
    UnbinnedLikelihood narrow(model, events, -1.0, 2.0);
    narrow.prepare();
    CHECK(narrow.nPoints() > 0 && narrow.nPoints() < unsigned(n));
  }

  // ---- unbinned exponential over [0, 5]: the likelihood equation, solved here by bisection
  {
    const double L = 5.0;
    std::vector<double> events;
    double mean = 0;
    while (events.size() < 1500)
      {
      double t = -1.4*std::log(random.uniform());
      if (t <= L) { events.push_back(t); mean += t; }
      }
    mean /= events.size();
    // mean of a density proportional to exp(b t) over [0, L]: L e^{bL}/(e^{bL} - 1) - 1/b, increasing with b
    auto expectedMean = [L](double b) { return L*std::exp(b*L)/(std::exp(b*L) - 1) - 1/b; };
    double low = -10, high = -1E-6;
    for (int k=0; k<200; k++) { double middle = 0.5*(low + high); (expectedMean(middle) < mean ? low : high) = middle; }
    double exact = 0.5*(low + high);
    Exponential model(1.0, -1.0);
    model.fixParameter(0);
    UnbinnedLikelihood likelihood(model, events, 0.0, L);
    FitResult r = Fitter(likelihood).fit();
    CHECK_FIT(close(r.values[1], exact, 1E-5) && r.validErrors && r.errors[1] > 0, r);
  }

  // ---- errors
  {
    Normal model(1.0, 0.0, 1.0);
    bool thrown = false;
    try { UnbinnedLikelihood u(model, std::vector<std::vector<double>>{{0.0, 1.0}}, {0.0, 0.0}, {1.0, 1.0}); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);                                                          // a range of rank 2 for a function of rank 1
    UnbinnedLikelihood outside(model, std::vector<double>{10.0, 11.0}, -1.0, 1.0);
    thrown = false;
    try { Fitter(outside).fit(); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);                                                          // no event in the range
    Fitter fitter(outside);
    thrown = false;
    try { fitter.chiSquare(); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);                                                          // not a chi square fit
  }

  std::printf("TestLikelihood: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
