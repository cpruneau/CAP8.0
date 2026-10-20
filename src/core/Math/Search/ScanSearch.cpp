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
//! @file ScanSearch.cpp
//! @brief Implementation of ScanSearch.
#include <cmath>
#include <cstdio>
#include "ScanSearch.hpp"

namespace CAP
{
  namespace MATH
  {

  SearchResult ScanSearch::search(const SearchProblem & problem, const SearchDomain & domain, const SearchOptions & options)
  {
  begin(problem, options);
  SearchResult result;
  long iterations = 0;
  if (problem.isDiscrete())
    {
    for (unsigned int n : allowedCandidates(problem, domain))
      {
      if (budgetSpent(problem, options, iterations)) { finish(result, problem, iterations, false, "budget spent"); return result; }
      iterations++;
      if (keepBest(result, problem, problem.candidate(n), problem.candidateObjective(n))) result.candidate = int(n);
      if (options.keepHistory) result.history.push_back(result.value);
      }
    finish(result, problem, iterations, true, "all the candidates scanned");
    return result;
    }
  if (!domain.isBounded()) { finish(result, problem, 0, false, "a scan needs a bounded domain"); return result; }
  const unsigned int d = problem.dimension();
  long n = _pointsPerAxis;
  if (n == 0)
    {
    n = long(std::floor(std::pow(double(options.maxEvaluations), 1.0/d) + 1E-9));
    n = n < 2 ? 2 : (n > 1001 ? 1001 : n);
    }
  std::vector<long> index(d, 0);
  std::vector<double> x(d);
  bool done = false;
  while (!done)
    {
    if (budgetSpent(problem, options, iterations)) { finish(result, problem, iterations, false, "budget spent"); return result; }
    for (unsigned int k=0; k<d; k++) x[k] = domain.lower(k) + (domain.upper(k) - domain.lower(k))*index[k]/double(n - 1);
    iterations++;
    if (domain.allowed(x)) keepBest(result, problem, x, problem.objective(x));
    if (options.keepHistory && result.found) result.history.push_back(result.value);
    // next point of the grid (dimension 0 fastest)
    unsigned int k = 0;
    while (k < d && ++index[k] == n) { index[k] = 0; k++; }
    done = k == d;
    }
  char reason[64];
  std::snprintf(reason, sizeof(reason), "grid of %ld points per axis scanned", n);
  finish(result, problem, iterations, true, reason);
  return result;
  }

  } // namespace MATH
} // namespace CAP
