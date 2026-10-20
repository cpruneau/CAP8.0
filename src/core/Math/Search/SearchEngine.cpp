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
//! @file SearchEngine.cpp
//! @brief Implementation of SearchEngine.
#include <algorithm>
#include <cmath>
#include <limits>
#include "SearchEngine.hpp"

namespace CAP
{
  namespace MATH
  {

  SearchEngine::SearchEngine(const String & name)
  :
  _name(name),
  _random(12345)
  { }

  double SearchEngine::uniform(double low, double high)
  {
  return std::uniform_real_distribution<double>(low, high)(_random);
  }

  double SearchEngine::gaussian()
  {
  return std::normal_distribution<double>(0.0, 1.0)(_random);
  }

  bool SearchEngine::randomPoint(const SearchDomain & domain, std::vector<double> & point)
  {
  if (!domain.isBounded()) return false;
  point.assign(domain.dimension(), 0.0);
  for (int attempt=0; attempt<10000; attempt++)
    {
    for (unsigned int k=0; k<domain.dimension(); k++) point[k] = uniform(domain.lower(k), domain.upper(k));
    if (domain.allowed(point)) return true;
    }
  return false;
  }

  std::vector<double> SearchEngine::startPoint(const SearchProblem & problem, const SearchDomain & domain)
  {
  std::vector<double> x(problem.dimension(), 0.0);
  if (domain.hasStart()) x = domain.clamped(domain.start());
  else
    for (unsigned int k=0; k<problem.dimension(); k++)
      x[k] = domain.isBounded(k) ? 0.5*(domain.lower(k) + domain.upper(k))
           : (std::isfinite(domain.lower(k)) ? domain.lower(k) + 1.0 : (std::isfinite(domain.upper(k)) ? domain.upper(k) - 1.0 : 0.0));
  if (!domain.allowed(x))
    {
    std::vector<double> y;
    if (randomPoint(domain, y)) x = y;
    }
  return x;
  }

  std::vector<unsigned int> SearchEngine::allowedCandidates(const SearchProblem & problem, const SearchDomain & domain) const
  {
  std::vector<unsigned int> allowed;
  for (unsigned int n=0; n<problem.nCandidates(); n++) if (domain.allowed(problem.candidate(n))) allowed.push_back(n);
  return allowed;
  }

  int SearchEngine::startCandidate(const SearchProblem & problem, const SearchDomain & domain)
  {
  if (domain.hasStart())
    {
    int nearest = problem.nearestCandidate(domain.start());
    if (nearest >= 0 && domain.allowed(problem.candidate(nearest))) return nearest;
    // the nearest allowed one
    double best = std::numeric_limits<double>::infinity();
    int index = -1;
    for (unsigned int n : allowedCandidates(problem, domain))
      {
      std::vector<double> c = problem.candidate(n);
      double d = 0;
      for (unsigned int k=0; k<c.size(); k++) { double t = (c[k] - domain.start()[k])/domain.width(k); d += t*t; }
      if (d < best) { best = d; index = int(n); }
      }
    return index;
    }
  std::vector<unsigned int> allowed = allowedCandidates(problem, domain);
  if (allowed.empty()) return -1;
  return int(allowed[std::uniform_int_distribution<std::size_t>(0, allowed.size() - 1)(_random)]);
  }

  bool SearchEngine::budgetSpent(const SearchProblem & problem, const SearchOptions & options, long iterations) const
  {
  return evaluations(problem) >= options.maxEvaluations || iterations >= options.maxIterations;
  }

  bool SearchEngine::keepBest(SearchResult & result, const SearchProblem & problem, const std::vector<double> & x, double objective) const
  {
  if (!std::isfinite(objective)) return false;
  double bestObjective = problem.goal() == SearchProblem::kMaximum ? -result.value : result.value;
  if (result.found && objective >= bestObjective) return false;
  result.found = true;
  result.point = x;
  result.value = problem.valueOf(objective);
  return true;
  }

  void SearchEngine::finish(SearchResult & result, const SearchProblem & problem, long iterations, bool converged, const String & reason) const
  {
  result.nEvaluations = evaluations(problem);
  result.nIterations = iterations;
  result.converged = converged && result.found;
  result.engine = _name;
  result.stopReason = (result.found || !converged) ? reason : String("no allowed point could be evaluated");
  }

  long SearchEngine::discreteDescent(const SearchProblem & problem, const SearchDomain & domain, const SearchOptions & options,
                                     int start, bool best, SearchResult & result)
  {
  if (start < 0) return 0;
  int current = start;
  double currentObjective = problem.candidateObjective(current);
  if (keepBest(result, problem, problem.candidate(current), currentObjective)) result.candidate = current;
  long iterations = 0;
  while (!budgetSpent(problem, options, iterations))
    {
    iterations++;
    std::vector<unsigned int> neighbors = problem.neighbors(current);
    if (!best) std::shuffle(neighbors.begin(), neighbors.end(), _random);
    int next = -1;
    double nextObjective = currentObjective;
    for (unsigned int n : neighbors)
      {
      if (!domain.allowed(problem.candidate(n))) continue;
      double o = problem.candidateObjective(n);
      if (o < nextObjective)
        {
        next = int(n);
        nextObjective = o;
        if (!best) break;                     // first improvement
        }
      }
    if (options.keepHistory) result.history.push_back(problem.valueOf(currentObjective));
    if (next < 0) break;                      // no better neighbor: a local extremum
    current = next;
    currentObjective = nextObjective;
    if (keepBest(result, problem, problem.candidate(current), currentObjective)) result.candidate = current;
    }
  return iterations;
  }

  } // namespace MATH
} // namespace CAP
