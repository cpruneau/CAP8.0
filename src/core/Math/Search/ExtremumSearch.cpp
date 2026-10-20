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
//! @file ExtremumSearch.cpp
//! @brief Implementation of ExtremumSearch.
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <random>
#include "ExtremumSearch.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  ExtremumSearch::ExtremumSearch()
  :
  _method(kAutomatic),
  _polish(kNone),
  _multiStart(1),
  _refineData(true),
  _options(),
  _scan(),
  _gradient(),
  _hillClimbing(),
  _greedy(),
  _annealing(),
  _genetic(EvolutionarySearch::kGenetic),
  _differential(EvolutionarySearch::kDifferentialEvolution),
  _divide(),
  _explore()
  { }

  const char * ExtremumSearch::name(Method method)
  {
  switch (method)
    {
    case kAutomatic:    return "automatic";
    case kScan:         return "scan";
    case kGradient:     return "gradient";
    case kHillClimbing: return "hill climbing";
    case kGreedy:       return "greedy";
    case kAnnealing:    return "simulated annealing";
    case kGenetic:      return "genetic";
    case kDifferentialEvolution: return "differential evolution";
    case kDivideAndConquer:      return "divide and conquer";
    case kExploreExploit:        return "explore-exploit";
    default:            return "none";
    }
  }

  ExtremumSearch::Method ExtremumSearch::method(const String & name)
  {
  // compared without case and blanks: "Hill Climbing", "hillclimbing", "hill_climbing"
  auto squeeze = [](const char * s) { std::string r; for (; *s; s++) if (std::isalnum((unsigned char)*s)) r += char(std::tolower((unsigned char)*s)); return r; };
  std::string wanted = squeeze(name.data());
  for (Method m : { kAutomatic, kScan, kGradient, kHillClimbing, kGreedy, kAnnealing, kGenetic, kDifferentialEvolution,
                    kDivideAndConquer, kExploreExploit, kNone })
    if (squeeze(ExtremumSearch::name(m)) == wanted) return m;
  if (wanted == "annealing") return kAnnealing;
  if (wanted == "evolutionary" || wanted == "de") return kDifferentialEvolution;
  if (wanted == "ga") return kGenetic;
  return kNone;
  }

  bool ExtremumSearch::setMethod(const String & name)
  {
  Method m = method(name);
  if (m == kNone) return false;
  _method = m;
  return true;
  }

  SearchEngine & ExtremumSearch::engine(Method method)
  {
  switch (method)
    {
    case kScan:         return _scan;
    case kHillClimbing: return _hillClimbing;
    case kGreedy:       return _greedy;
    case kAnnealing:    return _annealing;
    case kGenetic:      return _genetic;
    case kDifferentialEvolution: return _differential;
    case kDivideAndConquer:      return _divide;
    case kExploreExploit:        return _explore;
    default:            return _gradient;
    }
  }

  ExtremumSearch::Method ExtremumSearch::automaticMethod(const SearchProblem & problem, const SearchDomain & domain)
  {
  if (problem.isDiscrete()) return kScan;
  return domain.isBounded() ? kDifferentialEvolution : kGradient;
  }

  SearchResult ExtremumSearch::search(const SearchProblem & problem, const SearchDomain & domain)
  {
  if (domain.dimension() != problem.dimension())
    throw MathException("The domain and the problem have different dimensions", __FUNCTION__);
  problem.resetCounts();
  Method first = _method == kAutomatic ? automaticMethod(problem, domain) : _method;
  Method second = _polish == kAutomatic ? kGradient : _polish;
  SearchOptions options = _options;
  if (_method == kAutomatic && !problem.isDiscrete() && domain.isBounded())
    {
    // automatic: differential evolution finds the region of the global extremum, the gradient method refines it
    second = kGradient;
    options.maxEvaluations = std::max<long>(_options.maxEvaluations/2, 1);
    }
  SearchResult result = run(first, problem, domain, options);
  if (second != kNone && second != first && result.found && !problem.isDiscrete())
    {
    SearchDomain from = domain;
    from.setStart(result.point);
    SearchOptions rest = _options;
    rest.maxEvaluations = std::max<long>(_options.maxEvaluations - result.nEvaluations, 1);
    SearchResult refined = engine(second).search(problem, from, rest);
    if (refined.found && (problem.goal() == SearchProblem::kMaximum ? refined.value >= result.value : refined.value <= result.value))
      {
      result.point = refined.point;
      result.value = refined.value;
      result.converged = refined.converged;
      result.stopReason = refined.stopReason;
      }
    result.nEvaluations += refined.nEvaluations;
    result.nIterations += refined.nIterations;
    result.history.insert(result.history.end(), refined.history.begin(), refined.history.end());
    result.engine = String(std::string(name(first)) + " + " + name(second));
    }
  if (_refineData && problem.isDiscrete() && result.candidate >= 0)
    result.refined = problem.refine((unsigned int)(result.candidate), result.refinedPoint, result.refinedValue);
  return result;
  }

  SearchResult ExtremumSearch::run(Method method, const SearchProblem & problem, const SearchDomain & domain, const SearchOptions & options)
  {
  SearchEngine & searcher = engine(method);
  if (_multiStart <= 1)
    {
    SearchResult result = searcher.search(problem, domain, options);
    result.engine = name(method);
    return result;
    }
  // several runs: from the start point of the domain (or its center), then from random allowed points (around the
  // start point in an unbounded domain); the budget is shared, the best result kept
  std::mt19937_64 random(options.seed);
  SearchResult best;
  long evaluations = 0, iterations = 0;
  for (int run=0; run<_multiStart && evaluations < options.maxEvaluations; run++)
    {
    SearchDomain from = domain;
    if (run > 0)
      {
      std::vector<double> x(problem.dimension());
      bool placed = false;
      for (int attempt=0; attempt<1000 && !placed; attempt++)
        {
        for (unsigned int k=0; k<problem.dimension(); k++)
          {
          if (domain.isBounded(k)) x[k] = std::uniform_real_distribution<double>(domain.lower(k), domain.upper(k))(random);
          else x[k] = (domain.hasStart() ? domain.start()[k] : 0.0) + std::normal_distribution<double>(0.0, 1.0)(random);
          }
        placed = domain.allowed(x);
        }
      if (!placed) break;
      from.setStart(x);
      }
    SearchOptions share = options;
    share.maxEvaluations = std::max<long>((options.maxEvaluations - evaluations)/(_multiStart - run), 1);
    share.seed = options.seed + run;
    SearchResult result = searcher.search(problem, from, share);
    evaluations += result.nEvaluations;
    iterations += result.nIterations;
    bool better = result.found && (!best.found || (problem.goal() == SearchProblem::kMaximum ? result.value > best.value : result.value < best.value));
    if (better) { std::vector<double> history = best.history; best = result; best.history.insert(best.history.begin(), history.begin(), history.end()); }
    else best.history.insert(best.history.end(), result.history.begin(), result.history.end());
    }
  best.nEvaluations = evaluations;
  best.nIterations = iterations;
  char label[96];
  std::snprintf(label, sizeof(label), "%s (%d starts)", name(method), _multiStart);
  best.engine = label;
  if (!best.found) best.stopReason = "no allowed point could be evaluated";
  return best;
  }

  SearchResult ExtremumSearch::minimum(const Function & function, const SearchDomain & domain)
  {
  return search(FunctionProblem(function, SearchProblem::kMinimum), domain);
  }

  SearchResult ExtremumSearch::maximum(const Function & function, const SearchDomain & domain)
  {
  return search(FunctionProblem(function, SearchProblem::kMaximum), domain);
  }

  SearchResult ExtremumSearch::minimum(const DataPoints & data, const SearchDomain & domain)
  {
  return search(DataProblem(data, SearchProblem::kMinimum), domain);
  }

  SearchResult ExtremumSearch::maximum(const DataPoints & data, const SearchDomain & domain)
  {
  return search(DataProblem(data, SearchProblem::kMaximum), domain);
  }

  SearchResult ExtremumSearch::minimum(const DataPoints & data)
  {
  DataProblem problem(data, SearchProblem::kMinimum);
  return search(problem, problem.naturalDomain());
  }

  SearchResult ExtremumSearch::maximum(const DataPoints & data)
  {
  DataProblem problem(data, SearchProblem::kMaximum);
  return search(problem, problem.naturalDomain());
  }

  } // namespace MATH
} // namespace CAP
