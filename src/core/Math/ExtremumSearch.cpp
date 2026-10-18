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
  _refineData(true),
  _options(),
  _scan(),
  _gradient(),
  _hillClimbing(),
  _greedy()
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
    default:            return "none";
    }
  }

  ExtremumSearch::Method ExtremumSearch::method(const String & name)
  {
  // compared without case and blanks: "Hill Climbing", "hillclimbing", "hill_climbing"
  auto squeeze = [](const char * s) { std::string r; for (; *s; s++) if (std::isalnum((unsigned char)*s)) r += char(std::tolower((unsigned char)*s)); return r; };
  std::string wanted = squeeze(name.data());
  for (Method m : { kAutomatic, kScan, kGradient, kHillClimbing, kGreedy, kNone })
    if (squeeze(ExtremumSearch::name(m)) == wanted) return m;
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
    default:            return _gradient;
    }
  }

  ExtremumSearch::Method ExtremumSearch::automaticMethod(const SearchProblem & problem, const SearchDomain & domain)
  {
  if (problem.isDiscrete()) return kScan;
  return domain.isBounded() ? kScan : kGradient;
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
    // automatic: a coarse scan finds the region of the extremum, the gradient method refines it
    second = kGradient;
    options.maxEvaluations = std::min<long>(_options.maxEvaluations/4, 20000);
    }
  SearchResult result = engine(first).search(problem, domain, options);
  result.engine = name(first);
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
