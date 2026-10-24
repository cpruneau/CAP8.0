/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file AnnealingSearch.cpp
//! @brief Implementation of AnnealingSearch.
#include <algorithm>
#include <cmath>
#include <limits>
#include "AnnealingSearch.hpp"

namespace CAP
{
  namespace MATH
  {

  AnnealingSearch::AnnealingSearch()
  :
  SearchEngine("simulated annealing"),
  _initialTemperature(0),
  _cooling(0.95),
  _finalRatio(1E-6),
  _movesPerStage(0),
  _initialStep(0.2),
  _targetAcceptance(0.4)
  { }

  SearchResult AnnealingSearch::search(const SearchProblem & problem, const SearchDomain & domain, const SearchOptions & options)
  {
  begin(problem, options);
  SearchResult result;
  const bool discrete = problem.isDiscrete();
  const unsigned int d = problem.dimension();
  long iterations = 0;
  // the current point (or candidate) and its objective
  std::vector<double> x;
  int current = -1;
  double f = std::numeric_limits<double>::infinity();
  if (discrete)
    {
    current = startCandidate(problem, domain);
    if (current >= 0) { x = problem.candidate(current); f = problem.candidateObjective(current); }
    }
  else
    {
    if (!domain.hasStart() && !randomPoint(domain, x)) x = startPoint(problem, domain);
    if (domain.hasStart()) x = startPoint(problem, domain);
    if (domain.allowed(x)) f = problem.objective(x);
    }
  if (!std::isfinite(f)) { finish(result, problem, 0, false, "the objective cannot be evaluated at the start point"); return result; }
  if (keepBest(result, problem, x, f) && discrete) result.candidate = current;
  // the first temperature: the spread of the objective over random points (or candidates)
  double temperature = _initialTemperature;
  if (temperature <= 0)
    {
    std::vector<double> sample;
    std::vector<unsigned int> allowed = discrete ? allowedCandidates(problem, domain) : std::vector<unsigned int>();
    for (int k=0; k<20; k++)
      {
      double v = std::numeric_limits<double>::infinity();
      if (discrete && !allowed.empty()) v = problem.candidateObjective(allowed[std::uniform_int_distribution<std::size_t>(0, allowed.size()-1)(_random)]);
      else
        {
        std::vector<double> y;
        if (!randomPoint(domain, y))
          { y = x; for (unsigned int j=0; j<d; j++) y[j] += _initialStep*domain.width(j)*gaussian(); }
        if (domain.allowed(y)) v = problem.objective(y);
        }
      if (std::isfinite(v)) sample.push_back(v);
      }
    double mean = 0, spread = 0;
    for (double v : sample) mean += v/sample.size();
    for (double v : sample) spread += (v - mean)*(v - mean)/sample.size();
    temperature = std::sqrt(spread);
    if (!(temperature > 0)) temperature = std::max(1.0, std::fabs(f));
    }
  const double lastTemperature = temperature*_finalRatio, coolTemperature = 0.1*temperature;
  const int moves = _movesPerStage > 0 ? _movesPerStage : int(30*d);
  double step = _initialStep;
  bool converged = false;
  String reason = "budget spent";
  std::vector<double> y(d);
  while (!budgetSpent(problem, options, iterations))
    {
    int accepted = 0, tried = 0;               // moves of the adaptive step
    for (int m=0; m<moves && !budgetSpent(problem, options, iterations); m++)
      {
      iterations++;
      double g;
      int next = -1;
      if (discrete)
        {
        std::vector<unsigned int> neighbors = problem.neighbors(current);
        if (neighbors.empty()) break;
        next = int(neighbors[std::uniform_int_distribution<std::size_t>(0, neighbors.size()-1)(_random)]);
        if (!domain.allowed(problem.candidate(next))) continue;
        g = problem.candidateObjective(next);
        }
      bool large = false;
      if (!discrete)
        {
        // a fifth of the moves keep the first (large) step: jumps between basins stay possible as it cools
        large = uniform() < 0.2;
        double s = large ? std::max(step, _initialStep) : step;
        for (unsigned int k=0; k<d; k++) y[k] = x[k] + s*domain.width(k)*gaussian();
        y = domain.clamped(y);
        if (!domain.allowed(y)) continue;
        g = problem.objective(y);
        }
      if (!large) tried++;
      // Metropolis: better always, worse with probability exp(-increase/T)
      if (g <= f || (std::isfinite(g) && uniform() < std::exp(-(g - f)/temperature)))
        {
        if (!large) accepted++;
        f = g;
        if (discrete) { current = next; x = problem.candidate(current); } else x = y;
        if (keepBest(result, problem, x, f) && discrete) result.candidate = current;
        }
      }
    if (options.keepHistory) result.history.push_back(result.value);
    // once cooler, back to the best point when the walk has strayed above it by more than the temperature (it
    // settles in the best region visited rather than wherever it happens to be; hot, it crosses valleys freely)
    double bestObjective = problem.goal() == SearchProblem::kMaximum ? -result.value : result.value;
    if (temperature < coolTemperature && f > bestObjective + temperature)
      {
      x = result.point;
      f = bestObjective;
      if (discrete) current = result.candidate;
      }
    // the step follows the fraction of accepted moves; the temperature decreases
    if (!discrete && tried > 0)
      {
      double fraction = double(accepted)/tried;
      step *= fraction > _targetAcceptance ? 1.2 : 0.8;
      step = std::min(std::max(step, 1E-4), 0.5);    // keeps moving until it has cooled
      }
    temperature *= _cooling;
    if (temperature <= lastTemperature) { converged = true; reason = "cooled to the last temperature"; break; }
    }
  finish(result, problem, iterations, converged, reason);
  return result;
  }

  } // namespace MATH
} // namespace CAP
