/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file HillClimbingSearch.cpp
//! @brief Implementation of HillClimbingSearch.
#include <cmath>
#include <limits>
#include "HillClimbingSearch.hpp"

namespace CAP
{
  namespace MATH
  {

  bool HillClimbingSearch::climb(const SearchProblem & problem, const SearchDomain & domain, const SearchOptions & options,
                                 std::vector<double> x, SearchResult & result, long & iterations)
  {
  const unsigned int d = problem.dimension();
  double f = domain.allowed(x) ? problem.objective(x) : std::numeric_limits<double>::infinity();
  if (!std::isfinite(f)) return false;
  keepBest(result, problem, x, f);
  const int tries = _tries > 0 ? _tries : int(2*d + 2);
  double step = _initialStep;
  int failures = 0;
  std::vector<double> y(d);
  while (!budgetSpent(problem, options, iterations))
    {
    iterations++;
    if (_moves == kCoordinates)
      {
      y = x;
      unsigned int k = (unsigned int)(iterations % d);
      y[k] += (uniform() < 0.5 ? -1.0 : 1.0)*step*domain.width(k);
      }
    else
      {
      double norm = 0;
      std::vector<double> r(d);
      for (unsigned int k=0; k<d; k++) { r[k] = gaussian(); norm += r[k]*r[k]; }
      norm = std::sqrt(norm);
      for (unsigned int k=0; k<d; k++) y[k] = x[k] + step*domain.width(k)*r[k]/(norm > 0 ? norm : 1.0);
      }
    y = domain.clamped(y);
    double g = (y != x && domain.allowed(y)) ? problem.objective(y) : std::numeric_limits<double>::infinity();
    if (g < f)
      {
      x = y; f = g;
      keepBest(result, problem, x, f);
      step = std::min(step*_grow, 0.5);
      failures = 0;
      }
    else if (++failures >= tries)
      {
      step *= _shrink;
      failures = 0;
      bool small = true;
      for (unsigned int k=0; k<d; k++) small = small && step*domain.width(k) <= options.tolerance(k, domain);
      if (small) return true;
      }
    if (options.keepHistory) result.history.push_back(result.value);
    }
  return false;
  }

  SearchResult HillClimbingSearch::search(const SearchProblem & problem, const SearchDomain & domain, const SearchOptions & options)
  {
  begin(problem, options);
  SearchResult result;
  long iterations = 0;
  bool converged = false;
  if (problem.isDiscrete())
    {
    iterations = discreteDescent(problem, domain, options, startCandidate(problem, domain), false, result);
    converged = !budgetSpent(problem, options, iterations);
    std::vector<unsigned int> allowed = allowedCandidates(problem, domain);
    for (int r=0; r<_restarts && !allowed.empty() && !budgetSpent(problem, options, iterations); r++)
      {
      int start = int(allowed[std::uniform_int_distribution<std::size_t>(0, allowed.size() - 1)(_random)]);
      iterations += discreteDescent(problem, domain, options, start, false, result);
      }
    finish(result, problem, iterations, converged, "no better neighbor");
    return result;
    }
  converged = climb(problem, domain, options, startPoint(problem, domain), result, iterations);
  for (int r=0; r<_restarts && !budgetSpent(problem, options, iterations); r++)
    {
    std::vector<double> x;
    if (!randomPoint(domain, x)) break;            // restarts need a bounded domain
    converged = climb(problem, domain, options, x, result, iterations) && converged;
    }
  finish(result, problem, iterations, converged, converged ? "steps below the tolerance" : "budget spent");
  return result;
  }

  } // namespace MATH
} // namespace CAP
