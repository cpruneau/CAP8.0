/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file GradientSearch.cpp
//! @brief Implementation of GradientSearch.
#include <cmath>
#include <limits>
#include "GradientSearch.hpp"

namespace CAP
{
  namespace MATH
  {

  SearchResult GradientSearch::search(const SearchProblem & problem, const SearchDomain & domain, const SearchOptions & options)
  {
  begin(problem, options);
  SearchResult result;
  if (problem.isDiscrete())
    {
    long iterations = discreteDescent(problem, domain, options, startCandidate(problem, domain), true, result);
    finish(result, problem, iterations, !budgetSpent(problem, options, iterations), "no better neighbor (steepest descent)");
    return result;
    }
  const unsigned int d = problem.dimension();
  std::vector<double> width(d), tolerance(d), steps(d);
  for (unsigned int k=0; k<d; k++) { width[k] = domain.width(k); tolerance[k] = options.tolerance(k, domain); }
  std::vector<double> x = startPoint(problem, domain);
  double f = domain.allowed(x) ? problem.objective(x) : std::numeric_limits<double>::infinity();
  if (!std::isfinite(f)) { finish(result, problem, 0, false, "the objective cannot be evaluated at the start point"); return result; }
  keepBest(result, problem, x, f);
  std::vector<double> velocity(d, 0.0);      // previous step, in scaled coordinates
  double alpha = _initialStep;
  long iterations = 0;
  bool converged = false;
  String reason = "budget spent";
  while (!budgetSpent(problem, options, iterations))
    {
    iterations++;
    for (unsigned int k=0; k<d; k++) steps[k] = 1E-6*width[k]*std::max(1.0, domain.isBounded(k) ? 1.0 : std::fabs(x[k]));
    std::vector<double> g = problem.objectiveGradient(x, steps);
    // gradient in the coordinates scaled by the widths, and the direction of descent (with the momentum)
    double norm = 0;
    for (unsigned int k=0; k<d; k++) { g[k] *= width[k]; norm += g[k]*g[k]; }
    norm = std::sqrt(norm);
    if (!(norm > _gradientTolerance*(1.0 + std::fabs(f)))) { converged = true; reason = "the gradient vanishes"; break; }
    std::vector<double> direction(d);
    double slope = 0;
    for (unsigned int k=0; k<d; k++) { direction[k] = -g[k]/norm + _momentum*velocity[k]; slope += g[k]*direction[k]; }
    if (slope >= 0) { for (unsigned int k=0; k<d; k++) direction[k] = -g[k]/norm; slope = -norm; }   // momentum against the gradient: drop it
    // backtracking line search (Armijo), steps projected on the bounds, shortened out of the exclusions
    double t = alpha, fNew = f;
    std::vector<double> xNew(d);
    bool accepted = false;
    for (int trial=0; trial<_maxLineSearch && !budgetSpent(problem, options, iterations); trial++, t *= 0.5)
      {
      for (unsigned int k=0; k<d; k++) xNew[k] = x[k] + t*direction[k]*width[k];
      xNew = domain.clamped(xNew);
      if (!domain.allowed(xNew)) continue;
      fNew = problem.objective(xNew);
      if (fNew <= f + 1E-4*t*slope) { accepted = true; break; }
      }
    bool small = true;
    for (unsigned int k=0; k<d; k++) small = small && std::fabs((accepted ? xNew[k] : x[k] + t*direction[k]*width[k]) - x[k]) <= tolerance[k];
    if (!accepted)
      {
      if (small) { converged = true; reason = "no descent along the gradient within the tolerance"; }
      else reason = "the line search failed";
      break;
      }
    double change = std::fabs(f - fNew);
    for (unsigned int k=0; k<d; k++) { velocity[k] = (xNew[k] - x[k])/width[k]; small = small && std::fabs(xNew[k] - x[k]) <= tolerance[k]; }
    x = xNew;
    f = fNew;
    keepBest(result, problem, x, f);
    if (options.keepHistory) result.history.push_back(result.value);
    if (small) { converged = true; reason = "steps below the tolerance"; break; }
    if (change <= options.fTolerance*(std::fabs(f) + options.fTolerance) && change > 0)
      { converged = true; reason = "the objective no longer changes"; break; }
    alpha = std::min(2.0*t, 1.0);             // the next step may be longer
    }
  finish(result, problem, iterations, converged, reason);
  return result;
  }

  } // namespace MATH
} // namespace CAP
