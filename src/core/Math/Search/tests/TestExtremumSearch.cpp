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
//! @file TestExtremumSearch.cpp
//! @brief Test program of the extremum searches (run by ctest).
//
// TestExtremumSearch -- tests of CAP::MATH::ExtremumSearch and its engines: domains and exclusions, the scan,
// gradient, hill climbing and greedy engines on standard functions with known extrema (sphere, Rosenbrock,
// Himmelblau), maxima, data points (histograms, points on a grid, scattered points), seeds and budgets. Prints each
// failed check and exits with the number of failures (0: all passed).
//
#include <cmath>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>
#include "DataPoint3D.hpp"
#include "DataPointsVector.hpp"
#include "ExtremumSearch.hpp"
#include "Function.hpp"
#include "Histogram.hpp"
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

//! A function of any rank given by a lambda, counting nothing itself (the problems count).
class Lambda : public Function
{
public:
  Lambda(unsigned int rank, std::function<double(const std::vector<double>&)> f) : Function(rank), _f(f) { }
  double evaluateAt(const std::vector<double> & x) const override { checkRank(x.size(), __FUNCTION__); return _f(x); }
  using Function::evaluateAt;
private:
  std::function<double(const std::vector<double>&)> _f;
};

double distance(const std::vector<double> & a, const std::vector<double> & b)
{
  double d = 0;
  for (std::size_t k=0; k<a.size() && k<b.size(); k++) d += (a[k] - b[k])*(a[k] - b[k]);
  return std::sqrt(d);
}

std::string show(const SearchResult & r)
{
  char text[256];
  std::snprintf(text, sizeof(text), " [%s: (%g, %g) value %g, %ld evaluations, %s]", r.engine.data(),
                r.point.size() > 0 ? r.point[0] : 0.0, r.point.size() > 1 ? r.point[1] : 0.0, r.value, r.nEvaluations, r.stopReason.data());
  return text;
}
} // namespace

//! @brief Count a check, and report it if the condition is false.
#define CHECK(condition) check((condition), #condition, __LINE__)
//! @brief Count a check about a search result, and report the result if the condition is false.
#define CHECK_RESULT(condition, result) check((condition), std::string(#condition) + show(result), __LINE__)

//! @brief Run the checks. @return number of failed checks (0: all passed)
int main()
{
  // ---- domains and exclusions
  {
    ExclusionRegion band = ExclusionRegion::interval(0, 1.2, 1.4);
    CHECK(band.contains({1.3, 100.0}) && band.contains({1.2, -5.0}) && !band.contains({1.5, 0.0}));
    ExclusionRegion box = ExclusionRegion().add(0, 0.0, 1.0).add(1, 3.0, 2.0);   // bounds in any order
    CHECK(box.contains({0.5, 2.5}) && !box.contains({0.5, 3.5}) && !box.contains({1.5, 2.5}));
    SearchDomain domain({-5.0, -5.0}, {5.0, 5.0});
    CHECK(domain.dimension() == 2 && domain.isBounded() && domain.width(0) == 10.0);
    domain.exclude(band);
    CHECK(domain.allowed({0.0, 0.0}) && !domain.allowed({1.3, 0.0}) && !domain.allowed({6.0, 0.0}) && !domain.allowed({0.0, std::nan("")}));
    CHECK(domain.clamped({7.0, -8.0}) == std::vector<double>({5.0, -5.0}));
    SearchDomain open(2);
    CHECK(!open.isBounded() && open.width(0) == 1.0 && open.allowed({1E30, -1E30}) && !open.hasStart());
    open.setStart({1.0, 2.0});
    CHECK(open.hasStart() && open.start()[1] == 2.0);
  }

  // ---- every engine on a sphere: minimum at (1, -2)
  Lambda sphere(2, [](const std::vector<double> & x) { return (x[0] - 1)*(x[0] - 1) + (x[1] + 2)*(x[1] + 2) + 3.0; });
  {
    SearchDomain domain({-5.0, -5.0}, {5.0, 5.0});
    for (ExtremumSearch::Method method : { ExtremumSearch::kAutomatic, ExtremumSearch::kGradient, ExtremumSearch::kHillClimbing, ExtremumSearch::kGreedy })
      {
      ExtremumSearch search;
      search.setMethod(method);
      SearchResult r = search.minimum(sphere, domain);
      CHECK_RESULT(r.found && r.converged && distance(r.point, {1.0, -2.0}) < 1E-4 && std::fabs(r.value - 3.0) < 1E-8, r);
      }
    ExtremumSearch search;
    search.setMethod(ExtremumSearch::kScan);
    search.scan().setPointsPerAxis(11);                                     // nodes at whole numbers: (1, -2) is one
    SearchResult r = search.minimum(sphere, domain);
    CHECK_RESULT(r.found && r.nEvaluations == 121 && distance(r.point, {1.0, -2.0}) < 1E-12, r);
    search.setPolish(ExtremumSearch::kGradient);
    search.scan().setPointsPerAxis(7);                                      // coarse grid, refined by the gradient
    r = search.minimum(sphere, domain);
    CHECK_RESULT(r.engine == "scan + gradient" && distance(r.point, {1.0, -2.0}) < 1E-5, r);
    SearchDomain unbounded(2);
    unbounded.setStart({10.0, 10.0});
    search.setMethod(ExtremumSearch::kAutomatic);
    search.setPolish(ExtremumSearch::kNone);
    r = search.minimum(sphere, unbounded);                                  // automatic: the gradient method
    CHECK_RESULT(r.engine == "gradient" && distance(r.point, {1.0, -2.0}) < 1E-4, r);
    search.setMethod(ExtremumSearch::kScan);
    r = search.minimum(sphere, unbounded);
    CHECK(!r.found && r.stopReason == "a scan needs a bounded domain");
  }

  // ---- Rosenbrock: a curved valley, minimum at (1, 1)
  {
    Lambda rosenbrock(2, [](const std::vector<double> & x) { return 100*(x[1] - x[0]*x[0])*(x[1] - x[0]*x[0]) + (1 - x[0])*(1 - x[0]); });
    SearchDomain domain({-2.0, -1.0}, {2.0, 3.0});
    domain.setStart({-1.2, 1.0});
    ExtremumSearch search;
    search.setMethod(ExtremumSearch::kGradient);
    search.gradient().setMomentum(0.5);
    SearchResult r = search.minimum(rosenbrock, domain);
    CHECK_RESULT(distance(r.point, {1.0, 1.0}) < 1E-2, r);
    search.setMethod(ExtremumSearch::kGreedy);
    r = search.minimum(rosenbrock, domain);
    CHECK_RESULT(r.converged && distance(r.point, {1.0, 1.0}) < 1E-3, r);
    search.setMethod(ExtremumSearch::kHillClimbing);
    r = search.minimum(rosenbrock, domain);
    CHECK_RESULT(distance(r.point, {1.0, 1.0}) < 1E-2, r);
  }

  // ---- Himmelblau: four minima of value 0; exclusions leave out some of them
  {
    Lambda himmelblau(2, [](const std::vector<double> & x)
      { double a = x[0]*x[0] + x[1] - 11, b = x[0] + x[1]*x[1] - 7; return a*a + b*b; });
    std::vector<std::vector<double>> minima = { {3.0, 2.0}, {-2.805118, 3.131312}, {-3.779310, -3.283186}, {3.584428, -1.848126} };
    auto nearest = [&](const std::vector<double> & x) { double best = 1E9; for (const auto & m : minima) best = std::min(best, distance(x, m)); return best; };
    SearchDomain domain({-5.0, -5.0}, {5.0, 5.0});
    ExtremumSearch search;
    SearchResult r = search.minimum(himmelblau, domain);
    CHECK_RESULT(r.converged && r.value < 1E-8 && nearest(r.point) < 1E-4, r);
    domain.exclude(0, 2.0, 5.0);                                            // leaves out (3, 2) and (3.58, -1.85)
    r = search.minimum(himmelblau, domain);
    CHECK_RESULT(r.value < 1E-8 && nearest(r.point) < 1E-4 && r.point[0] < 0, r);
    domain.exclude(ExclusionRegion().add(0, -5.0, 0.0).add(1, 0.0, 5.0));    // and (-2.8, 3.13)
    r = search.minimum(himmelblau, domain);
    CHECK_RESULT(r.value < 1E-8 && distance(r.point, minima[2]) < 1E-4, r);
    search.setMethod(ExtremumSearch::kHillClimbing);
    search.hillClimbing().setRestarts(5);
    r = search.minimum(himmelblau, domain);
    CHECK_RESULT(r.value < 1E-8 && distance(r.point, minima[2]) < 1E-3 && domain.allowed(r.point), r);
  }

  // ---- a minimum on an exclusion boundary; a maximum with an analytic gradient
  {
    Lambda parabola(1, [](const std::vector<double> & x) { return (x[0] - 1)*(x[0] - 1); });
    SearchDomain domain({-3.0}, {3.0});
    domain.exclude(0, 0.5, 1.5);
    ExtremumSearch search;
    SearchResult r = search.minimum(parabola, domain);
    CHECK_RESULT(std::fabs(r.value - 0.25) < 1E-4 && (r.point[0] < 0.5 || r.point[0] > 1.5), r);   // at the edge of the excluded interval
    Normal normal(2.0, 0.7, 0.3);
    FunctionProblem peak(normal, SearchProblem::kMaximum);
    CHECK(peak.hasGradient() && peak.dimension() == 1);
    SearchDomain line({-5.0}, {5.0});
    line.setStart({0.0});
    search.setMethod(ExtremumSearch::kGradient);
    r = search.search(peak, line);
    CHECK_RESULT(std::fabs(r.point[0] - 0.7) < 1E-5 && r.value > 0 && peak.nGradients() > 0, r);
    r = search.maximum(normal, line);
    CHECK_RESULT(std::fabs(r.point[0] - 0.7) < 1E-5, r);
  }

  // ---- data: histograms
  {
    Histogram h("h", "h", 50, -5.0, 5.0);
    unsigned long seed = 99;
    auto uniform = [&seed]() { seed = seed*6364136223846793005ULL + 1442695040888963407ULL; return ((seed >> 11) + 0.5)/9007199254740992.0; };
    // a smooth peak at 0.73: bin contents of a Gaussian (no noise), so the refinement can be checked
    for (unsigned int i=0; i<50; i++) { double x = h.binCenter(i, 0); h.fill(x, 100*std::exp(-0.5*(x - 0.73)*(x - 0.73)/0.64)); }
    ExtremumSearch search;
    SearchResult r = search.maximum(h);
    CHECK_RESULT(r.found && r.converged && r.engine == "scan" && r.candidate >= 0 && std::fabs(r.point[0] - 0.7) < 1E-6, r);   // the bin of 0.7 (coordinates are floats)
    CHECK(r.refined && std::fabs(r.refinedPoint[0] - 0.73) < std::fabs(r.point[0] - 0.73) && r.refinedValue >= r.value);
    SearchDomain domain({-5.0}, {5.0});
    domain.exclude(0, 0.0, 2.0);                                            // the peak left out
    r = search.maximum(h, domain);
    CHECK_RESULT(r.point[0] < 0.0 || r.point[0] > 2.0, r);
    search.setMethod(ExtremumSearch::kHillClimbing);                        // local moves among the bins from the left
    SearchDomain fromLeft({-5.0}, {5.0});
    fromLeft.setStart({-4.0});
    r = search.maximum(h, fromLeft);
    CHECK_RESULT(std::fabs(r.point[0] - 0.7) < 1E-6, r);
    // rank 2, with empty bins: the minimum is among the filled bins (unless the empty ones are searched)
    Histogram h2("h2", "h2", 10, 0.0, 10.0, 10, 0.0, 10.0);
    for (int k=0; k<3000; k++) h2.fill(5 + 2*(uniform() + uniform() + uniform() - 1.5), 5 + 2*(uniform() + uniform() - 1.0), 1.0);
    search.setMethod(ExtremumSearch::kAutomatic);
    DataProblem lowest(h2, SearchProblem::kMinimum);
    CHECK(lowest.isGrid() && lowest.nCandidates() < 100);
    r = search.search(lowest, lowest.naturalDomain());
    CHECK_RESULT(r.value > 0, r);
    DataProblem withEmpty(h2, SearchProblem::kMinimum, true);
    r = search.search(withEmpty, withEmpty.naturalDomain());
    CHECK_RESULT(r.value == 0 && withEmpty.nCandidates() == 100, r);
    r = search.maximum(h2);
    double highest = 0;
    for (unsigned int n=0; n<100; n++) highest = std::max(highest, h2.valueAt(n));
    CHECK_RESULT(r.value == highest, r);
    search.setMethod(ExtremumSearch::kGreedy);
    SearchDomain corner({0.0, 0.0}, {10.0, 10.0});
    corner.setStart({5.0, 5.0});
    r = search.maximum(h2, corner);
    CHECK_RESULT(r.converged && r.value >= 0.8*highest, r);                 // a local maximum near the center
  }

  // ---- data: points on a grid, scattered points
  {
    DataPointsVector grid, scattered;
    for (int j=0; j<5; j++) for (int i=0; i<6; i++)
      grid.addPoint(DataPoint3D(0.5*i, 1.0*j, -((0.5*i - 1.2)*(0.5*i - 1.2) + (j - 2.4)*(j - 2.4))));
    DataProblem gridded(grid, SearchProblem::kMaximum);
    CHECK(gridded.isGrid() && gridded.nCandidates() == 30);
    ExtremumSearch search;
    SearchResult r = search.search(gridded, gridded.naturalDomain());
    CHECK_RESULT(distance(r.point, {1.0, 2.0}) < 1E-6, r);
    CHECK(r.refined && distance(r.refinedPoint, {1.2, 2.4}) < 1E-5);         // a parabola is fitted exactly (float data)
    unsigned long seed = 7;
    auto uniform = [&seed]() { seed = seed*6364136223846793005ULL + 1442695040888963407ULL; return ((seed >> 11) + 0.5)/9007199254740992.0; };
    double best = -1E9;
    std::vector<double> bestPoint;
    for (int k=0; k<200; k++)
      {
      double x = 4*uniform() - 2, y = 4*uniform() - 2, z = std::exp(-x*x - y*y);
      scattered.addPoint(DataPoint3D(x, y, z));
      if (z > best) { best = z; bestPoint = {x, y}; }
      }
    DataProblem points(scattered, SearchProblem::kMaximum);
    CHECK(!points.isGrid() && points.neighbors(0).size() == 8);
    r = search.search(points, points.naturalDomain());
    CHECK_RESULT(std::fabs(r.value - best) < 1E-6 && distance(r.point, bestPoint) < 1E-6 && !r.refined, r);
    search.setMethod(ExtremumSearch::kGreedy);                              // from neighbor to neighbor: a local maximum
    r = search.search(points, points.naturalDomain());
    bool local = r.candidate >= 0;
    for (unsigned int n : points.neighbors((unsigned int)(r.candidate))) local = local && points.candidateValue(n) <= r.value;
    CHECK_RESULT(r.converged && local, r);
    SearchDomain nearPeak = points.naturalDomain();
    nearPeak.setStart({0.3, -0.2});
    r = search.search(points, nearPeak);
    CHECK_RESULT(std::fabs(r.value - best) < 1E-6, r);                      // from near the peak: the peak
  }

  // ---- multimodal functions: the global methods (each refined by the gradient method)
  const std::vector<ExtremumSearch::Method> globals = { ExtremumSearch::kAnnealing, ExtremumSearch::kGenetic,
    ExtremumSearch::kDifferentialEvolution, ExtremumSearch::kDivideAndConquer, ExtremumSearch::kExploreExploit };
  {
    // shifted Rastrigin: a lattice of local minima, the global one (0) at (1.3, -0.7)
    Lambda rastrigin(2, [](const std::vector<double> & x)
      {
      double z0 = x[0] - 1.3, z1 = x[1] + 0.7;
      return 20 + z0*z0 - 10*std::cos(2*M_PI*z0) + z1*z1 - 10*std::cos(2*M_PI*z1);
      });
    SearchDomain domain({-5.12, -5.12}, {5.12, 5.12});
    for (ExtremumSearch::Method method : globals)
      {
      ExtremumSearch search;
      search.setMethod(method);
      search.setPolish(ExtremumSearch::kGradient);
      SearchResult r = search.minimum(rastrigin, domain);
      CHECK_RESULT(r.found && r.value < 1E-6 && distance(r.point, {1.3, -0.7}) < 1E-4, r);
      }
    ExtremumSearch search;                                                  // automatic: differential evolution + gradient
    SearchResult r = search.minimum(rastrigin, domain);
    CHECK_RESULT(r.engine == "differential evolution + gradient" && r.value < 1E-6, r);
    search.setMethod(ExtremumSearch::kGradient);                            // a single descent: a local minimum
    domain.setStart({-3.9, 4.1});
    SearchResult single = search.minimum(rastrigin, domain);
    search.setMultiStart(40);                                               // many descents: a better one (about 100 basins)
    r = search.minimum(rastrigin, domain);
    CHECK_RESULT(single.value > 1 && r.value < single.value && r.value < 1.0 && r.engine == "gradient (40 starts)"
                 && r.nEvaluations <= search.options().maxEvaluations, r);
    search.setMultiStart(1);
  }
  {
    // Himmelblau with two of its minima excluded: the global methods stay out of the exclusions
    Lambda himmelblau(2, [](const std::vector<double> & x)
      { double a = x[0]*x[0] + x[1] - 11, b = x[0] + x[1]*x[1] - 7; return a*a + b*b; });
    SearchDomain domain({-5.0, -5.0}, {5.0, 5.0});
    domain.exclude(0, 2.0, 5.0);
    for (ExtremumSearch::Method method : globals)
      {
      ExtremumSearch search;
      search.setMethod(method);
      search.setPolish(ExtremumSearch::kGradient);
      SearchResult r = search.minimum(himmelblau, domain);
      CHECK_RESULT(r.value < 1E-6 && domain.allowed(r.point) && r.point[0] < 0, r);
      }
  }
  {
    // a histogram with two peaks: the higher one (100) at (7, 7), the lower one (60) at (2, 2)
    Histogram peaks("peaks", "peaks", 20, 0.0, 10.0, 20, 0.0, 10.0);
    double highest = 0;
    for (unsigned int j=0; j<20; j++)
      for (unsigned int i=0; i<20; i++)
        {
        double x = peaks.binCenter(i, 0), y = peaks.binCenter(j, 1);
        double v = 100*std::exp(-((x - 7)*(x - 7) + (y - 7)*(y - 7))/2) + 60*std::exp(-((x - 2)*(x - 2) + (y - 2)*(y - 2))/2) + 1;
        peaks.fill(x, y, v);
        highest = std::max(highest, peaks.valueAt(i + 20*j));
        }
    SearchDomain nearLower({0.0, 0.0}, {10.0, 10.0});
    nearLower.setStart({2.0, 2.5});
    ExtremumSearch search;
    search.setMethod(ExtremumSearch::kGreedy);
    SearchResult r = search.maximum(peaks, nearLower);
    CHECK_RESULT(r.value < 70, r);                                          // greedy: the lower peak
    for (ExtremumSearch::Method method : globals)
      {
      search.setMethod(method);
      r = search.maximum(peaks, nearLower);
      CHECK_RESULT(std::fabs(r.value - highest) < 1E-9 && distance(r.point, {7.0, 7.0}) < 0.5, r);
      }
  }
  {
    // budgets, seeds and names of the global methods
    Lambda bowl(3, [](const std::vector<double> & x) { return x[0]*x[0] + 2*x[1]*x[1] + 3*x[2]*x[2] + std::sin(5*x[0]); });
    SearchDomain domain({-3.0, -3.0, -3.0}, {3.0, 3.0, 3.0});
    for (ExtremumSearch::Method method : globals)
      {
      ExtremumSearch search;
      search.setMethod(method);
      search.options().maxEvaluations = 500;
      SearchResult a = search.minimum(bowl, domain), b = search.minimum(bowl, domain);
      CHECK_RESULT(a.found && a.nEvaluations <= 500 + 10, a);
      CHECK_RESULT(a.point == b.point && a.nEvaluations == b.nEvaluations, a);   // the same seed, the same search
      }
    ExtremumSearch search;
    CHECK(search.setMethod("Simulated Annealing") && search.method() == ExtremumSearch::kAnnealing);
    CHECK(search.setMethod("annealing") && search.setMethod("differential-evolution") && search.method() == ExtremumSearch::kDifferentialEvolution);
    CHECK(search.setMethod("DE") && search.setMethod("Genetic") && search.method() == ExtremumSearch::kGenetic);
    CHECK(search.setMethod("divide and conquer") && search.method() == ExtremumSearch::kDivideAndConquer);
    CHECK(search.setMethod("explore exploit") && search.method() == ExtremumSearch::kExploreExploit);
    SearchDomain open(3);
    for (ExtremumSearch::Method method : { ExtremumSearch::kGenetic, ExtremumSearch::kDifferentialEvolution, ExtremumSearch::kDivideAndConquer, ExtremumSearch::kExploreExploit })
      {
      search.setMethod(method);
      SearchResult r = search.minimum(bowl, open);
      CHECK_RESULT(!r.found && r.stopReason.contains("bounded"), r);        // global methods need bounds
      }
  }

  // ---- seeds, budgets, names, errors
  {
    Lambda wavy(2, [](const std::vector<double> & x) { return std::sin(3*x[0])*std::cos(2*x[1]) + 0.05*(x[0]*x[0] + x[1]*x[1]); });
    SearchDomain domain({-4.0, -4.0}, {4.0, 4.0});
    ExtremumSearch search;
    search.setMethod(ExtremumSearch::kHillClimbing);
    search.hillClimbing().setRestarts(3);
    SearchResult a = search.minimum(wavy, domain), b = search.minimum(wavy, domain);
    CHECK(a.point == b.point && a.nEvaluations == b.nEvaluations);          // the same seed: the same search
    search.options().maxEvaluations = 60;
    SearchResult c = search.minimum(wavy, domain);
    CHECK_RESULT(!c.converged && c.nEvaluations <= 60 && c.stopReason == "budget spent", c);
    search.options().maxEvaluations = 200000;
    search.setMethod(ExtremumSearch::kGradient);
    search.options().maxEvaluations = 25;
    c = search.minimum(wavy, domain);
    CHECK_RESULT(c.nEvaluations <= 25 + 5, c);                             // a line search may finish its step
    search.options().maxEvaluations = 200000;
    CHECK(search.setMethod("Hill Climbing") && search.method() == ExtremumSearch::kHillClimbing);
    CHECK(search.setMethod("greedy") && search.method() == ExtremumSearch::kGreedy);
    CHECK(!search.setMethod("nonsense") && search.method() == ExtremumSearch::kGreedy);
    CHECK(ExtremumSearch::method("SCAN") == ExtremumSearch::kScan && std::string(ExtremumSearch::name(ExtremumSearch::kGradient)) == "gradient");
    bool thrown = false;
    try { search.minimum(wavy, SearchDomain({0.0}, {1.0})); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);                                                          // dimensions differ
    SearchDomain closed({-1.0, -1.0}, {1.0, 1.0});
    closed.exclude(ExclusionRegion::box({-1.0, -1.0}, {1.0, 1.0}));         // nothing allowed
    search.setMethod(ExtremumSearch::kGradient);
    c = search.minimum(wavy, closed);
    CHECK(!c.found);
  }

  std::printf("TestExtremumSearch: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
