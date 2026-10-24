/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file EvolutionarySearch.cpp
//! @brief Implementation of EvolutionarySearch.
#include <algorithm>
#include <cmath>
#include <limits>
#include "EvolutionarySearch.hpp"

namespace CAP
{
  namespace MATH
  {

  EvolutionarySearch::EvolutionarySearch(Strategy strategy)
  :
  SearchEngine(strategy == kGenetic ? "genetic" : "differential evolution"),
  _strategy(strategy),
  _population(0),
  _crossover(0.9),
  _weight(0.7),
  _mutationRate(0),
  _mutationWidth(0.1),
  _elites(2),
  _tournament(3),
  _stall(strategy == kGenetic ? 60 : 150)
  { }

  void EvolutionarySearch::setStrategy(Strategy strategy)
  {
  _strategy = strategy;
  _name = strategy == kGenetic ? "genetic" : "differential evolution";
  _stall = strategy == kGenetic ? 60 : 150;
  }

  SearchResult EvolutionarySearch::search(const SearchProblem & problem, const SearchDomain & domain, const SearchOptions & options)
  {
  begin(problem, options);
  SearchResult result;
  const bool discrete = problem.isDiscrete();
  const unsigned int d = problem.dimension();
  // the box where points live: the domain, bounded by the extent of the candidates for a discrete problem
  SearchDomain box = domain;
  if (discrete)
    {
    SearchDomain natural = problem.naturalDomain();
    for (unsigned int k=0; k<d; k++)
      if (!box.isBounded(k)) box.setBounds(k, std::max(box.lower(k), natural.lower(k)), std::min(box.upper(k), natural.upper(k)));
    }
  if (!box.isBounded()) { finish(result, problem, 0, false, "evolution needs a bounded domain"); return result; }
  // a point is evaluated where it is, or at the allowed candidate nearest to it (which it then becomes)
  auto evaluate = [&](std::vector<double> & x, int & candidate) -> double
    {
    if (!discrete) return domain.allowed(x) ? problem.objective(x) : std::numeric_limits<double>::infinity();
    candidate = problem.nearestCandidate(x);
    if (candidate < 0 || !domain.allowed(problem.candidate(candidate))) return std::numeric_limits<double>::infinity();
    x = problem.candidate(candidate);
    return problem.candidateObjective(candidate);
    };
  auto keep = [&](const std::vector<double> & x, double f, int candidate)
    {
    if (keepBest(result, problem, x, f) && discrete) result.candidate = candidate;
    };
  const int n = _population > 0 ? _population : std::max(40, int(10*d));
  std::vector<std::vector<double>> points(n);
  std::vector<double> objectives(n, std::numeric_limits<double>::infinity());
  std::vector<int> candidates(n, -1);
  long iterations = 0;
  for (int i=0; i<n && !budgetSpent(problem, options, iterations); i++)
    {
    if (!randomPoint(box, points[i])) { finish(result, problem, 0, false, "no allowed point found in the domain"); return result; }
    objectives[i] = evaluate(points[i], candidates[i]);
    keep(points[i], objectives[i], candidates[i]);
    }
  const double mutationRate = _mutationRate > 0 ? _mutationRate : 1.0/d;
  double best = *std::min_element(objectives.begin(), objectives.end());   // of the first population
  int stalled = 0;
  bool converged = false;
  String reason = "budget spent";
  std::vector<double> trial(d);
  // a coordinate outside the box is drawn between the parent and the bound it crossed
  auto bounce = [&](double v, double parent, unsigned int k)
    {
    if (v < box.lower(k)) return box.lower(k) + uniform()*(parent - box.lower(k));
    if (v > box.upper(k)) return box.upper(k) - uniform()*(box.upper(k) - parent);
    return v;
    };
  while (!budgetSpent(problem, options, iterations))
    {
    iterations++;
    if (_strategy == kDifferentialEvolution)
      {
      for (int i=0; i<n && !budgetSpent(problem, options, iterations); i++)
        {
        int a, b, c;
        do a = int(uniform(0, n)); while (a == i || a >= n);
        do b = int(uniform(0, n)); while (b == i || b == a || b >= n);
        do c = int(uniform(0, n)); while (c == i || c == a || c == b || c >= n);
        unsigned int forced = (unsigned int)(uniform(0, d)) % d;
        for (unsigned int k=0; k<d; k++)
          {
          double v = (k == forced || uniform() < _crossover) ? points[a][k] + _weight*(points[b][k] - points[c][k]) : points[i][k];
          trial[k] = bounce(v, points[i][k], k);
          }
        int candidate = -1;
        double f = evaluate(trial, candidate);
        if (f <= objectives[i]) { points[i] = trial; objectives[i] = f; candidates[i] = candidate; keep(trial, f, candidate); }
        }
      }
    else
      {
      // genetic: elites, then children of parents chosen by tournaments
      std::vector<int> order(n);
      for (int i=0; i<n; i++) order[i] = i;
      std::sort(order.begin(), order.end(), [&](int p, int q) { return objectives[p] < objectives[q]; });
      std::vector<std::vector<double>> nextPoints;
      std::vector<double> nextObjectives;
      std::vector<int> nextCandidates;
      for (int e=0; e<std::min(_elites, n); e++)
        { nextPoints.push_back(points[order[e]]); nextObjectives.push_back(objectives[order[e]]); nextCandidates.push_back(candidates[order[e]]); }
      auto tournament = [&]()
        {
        int winner = int(uniform(0, n)) % n;
        for (int t=1; t<_tournament; t++) { int other = int(uniform(0, n)) % n; if (objectives[other] < objectives[winner]) winner = other; }
        return winner;
        };
      while (int(nextPoints.size()) < n && !budgetSpent(problem, options, iterations))
        {
        const std::vector<double> & mother = points[tournament()], & father = points[tournament()];
        for (unsigned int k=0; k<d; k++)
          {
          double v = mother[k];
          if (uniform() < _crossover)
            {
            const double alpha = 0.5;
            double low = std::min(mother[k], father[k]), high = std::max(mother[k], father[k]), span = high - low;
            v = uniform(low - alpha*span, high + alpha*span + 1E-300);
            }
          if (uniform() < mutationRate) v += _mutationWidth*box.width(k)*gaussian();
          trial[k] = bounce(v, mother[k], k);
          }
        int candidate = -1;
        double f = evaluate(trial, candidate);
        keep(trial, f, candidate);
        nextPoints.push_back(trial); nextObjectives.push_back(f); nextCandidates.push_back(candidate);
        }
      if (int(nextPoints.size()) == n) { points = nextPoints; objectives = nextObjectives; candidates = nextCandidates; }
      }
    if (options.keepHistory) result.history.push_back(result.value);
    // stop: the population gathered within the tolerance, or no improvement for a while
    double bestNow = *std::min_element(objectives.begin(), objectives.end());
    bool improved = !std::isfinite(best) ? std::isfinite(bestNow) : bestNow < best - options.fTolerance*(std::fabs(best) + options.fTolerance);
    if (improved) { best = bestNow; stalled = 0; }
    else if (++stalled >= _stall) { converged = true; reason = "no improvement for many generations"; break; }
    bool gathered = true;
    for (unsigned int k=0; k<d && gathered; k++)
      {
      double low = std::numeric_limits<double>::infinity(), high = -low;
      for (int i=0; i<n; i++) if (std::isfinite(objectives[i])) { low = std::min(low, points[i][k]); high = std::max(high, points[i][k]); }
      gathered = (high - low) <= options.tolerance(k, domain);
      }
    if (gathered) { converged = true; reason = "the population gathered within the tolerance"; break; }
    }
  finish(result, problem, iterations, converged, reason);
  return result;
  }

  } // namespace MATH
} // namespace CAP
