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
//! @file GreedySearch.cpp
//! @brief Implementation of GreedySearch.
#include <cmath>
#include <limits>
#include "GreedySearch.hpp"

namespace CAP
{
  namespace MATH
  {

  SearchResult GreedySearch::search(const SearchProblem & problem, const SearchDomain & domain, const SearchOptions & options)
  {
  begin(problem, options);
  SearchResult result;
  long iterations = 0;
  if (problem.isDiscrete())
    {
    iterations = discreteDescent(problem, domain, options, startCandidate(problem, domain), true, result);
    finish(result, problem, iterations, !budgetSpent(problem, options, iterations), "no better neighbor");
    return result;
    }
  const unsigned int d = problem.dimension();
  // the offsets of the neighbors: along the axes, and the diagonals if enabled
  std::vector<std::vector<int>> offsets;
  bool diagonals = _diagonals == 1 || (_diagonals == -1 && d <= 3);
  if (diagonals)
    {
    long n = 1;
    for (unsigned int k=0; k<d; k++) n *= 3;
    for (long code=0; code<n; code++)
      {
      std::vector<int> o(d);
      long rest = code;
      bool zero = true;
      for (unsigned int k=0; k<d; k++) { o[k] = int(rest % 3) - 1; rest /= 3; zero = zero && o[k] == 0; }
      if (!zero) offsets.push_back(o);
      }
    }
  else
    for (unsigned int k=0; k<d; k++)
      for (int s : {-1, 1}) { std::vector<int> o(d, 0); o[k] = s; offsets.push_back(o); }
  std::vector<double> x = startPoint(problem, domain);
  double f = domain.allowed(x) ? problem.objective(x) : std::numeric_limits<double>::infinity();
  if (!std::isfinite(f)) { finish(result, problem, 0, false, "the objective cannot be evaluated at the start point"); return result; }
  keepBest(result, problem, x, f);
  double spacing = _initialSpacing;
  bool converged = false;
  std::vector<double> y(d);
  while (!budgetSpent(problem, options, iterations))
    {
    iterations++;
    std::vector<double> best;
    double bestObjective = f;
    for (const auto & o : offsets)
      {
      for (unsigned int k=0; k<d; k++) y[k] = x[k] + o[k]*spacing*domain.width(k);
      y = domain.clamped(y);
      if (y == x || !domain.allowed(y)) continue;
      double g = problem.objective(y);
      if (g < bestObjective) { bestObjective = g; best = y; }
      if (budgetSpent(problem, options, iterations)) break;
      }
    if (!best.empty())
      {
      x = best; f = bestObjective;
      keepBest(result, problem, x, f);
      }
    else
      {
      spacing *= _refine;
      bool small = true;
      for (unsigned int k=0; k<d; k++) small = small && spacing*domain.width(k) <= options.tolerance(k, domain);
      if (small) { converged = true; break; }
      }
    if (options.keepHistory) result.history.push_back(result.value);
    }
  finish(result, problem, iterations, converged, converged ? "lattice spacing below the tolerance" : "budget spent");
  return result;
  }

  } // namespace MATH
} // namespace CAP
