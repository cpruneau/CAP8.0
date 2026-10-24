/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file DivideAndConquerSearch.cpp
//! @brief Implementation of DivideAndConquerSearch.
#include <algorithm>
#include <cmath>
#include <limits>
#include "DivideAndConquerSearch.hpp"

namespace CAP
{
  namespace MATH
  {

  namespace
  {
  //! A box of the domain, its best sample, and (discrete problems) the candidates it holds.
  struct Box
  {
    std::vector<double> low, high;
    double objective = std::numeric_limits<double>::infinity();
    std::vector<unsigned int> candidates;
  };
  } // namespace

  SearchResult DivideAndConquerSearch::search(const SearchProblem & problem, const SearchDomain & domain, const SearchOptions & options)
  {
  begin(problem, options);
  SearchResult result;
  const bool discrete = problem.isDiscrete();
  const unsigned int d = problem.dimension();
  SearchDomain box = domain;
  if (discrete)
    {
    SearchDomain natural = problem.naturalDomain();
    for (unsigned int k=0; k<d; k++)
      if (!box.isBounded(k)) box.setBounds(k, std::max(box.lower(k), natural.lower(k)), std::min(box.upper(k), natural.upper(k)));
    }
  if (!box.isBounded()) { finish(result, problem, 0, false, "divide and conquer needs a bounded domain"); return result; }
  long iterations = 0;
  // sample a box: its center and random points (continuous), or some of its candidates (discrete)
  auto sample = [&](Box & b)
    {
    if (discrete)
      {
      if (b.candidates.empty()) return;
      std::vector<unsigned int> chosen;
      // the candidate nearest to the center first, then random ones
      unsigned int nearest = b.candidates[0];
      double bestDistance = std::numeric_limits<double>::infinity();
      for (unsigned int c : b.candidates)
        {
        std::vector<double> x = problem.candidate(c);
        double dist = 0;
        for (unsigned int k=0; k<d; k++) { double t = (x[k] - 0.5*(b.low[k] + b.high[k]))/box.width(k); dist += t*t; }
        if (dist < bestDistance) { bestDistance = dist; nearest = c; }
        }
      chosen.push_back(nearest);
      for (int s=1; s<_samples && s<int(b.candidates.size()); s++)
        chosen.push_back(b.candidates[std::uniform_int_distribution<std::size_t>(0, b.candidates.size()-1)(_random)]);
      for (unsigned int c : chosen)
        {
        double f = problem.candidateObjective(c);
        b.objective = std::min(b.objective, f);
        if (keepBest(result, problem, problem.candidate(c), f)) result.candidate = int(c);
        }
      return;
      }
    std::vector<double> x(d);
    for (int s=0, tries=0; s<_samples && tries<20*_samples; tries++)
      {
      for (unsigned int k=0; k<d; k++) x[k] = (s == 0 && tries == 0) ? 0.5*(b.low[k] + b.high[k]) : uniform(b.low[k], b.high[k]);
      if (!domain.allowed(x)) continue;
      s++;
      double f = problem.objective(x);
      b.objective = std::min(b.objective, f);
      keepBest(result, problem, x, f);
      }
    };
  // halves of a box along its widest dimension (relative to the domain)
  auto split = [&](const Box & b, Box & first, Box & second)
    {
    unsigned int widest = 0;
    double size = -1;
    for (unsigned int k=0; k<d; k++)
      {
      double s = (b.high[k] - b.low[k])/box.width(k);
      if (discrete)
        {
        // the spread of the candidates, not of the box
        double low = std::numeric_limits<double>::infinity(), high = -low;
        for (unsigned int c : b.candidates) { double v = problem.candidate(c)[k]; low = std::min(low, v); high = std::max(high, v); }
        s = (high - low)/box.width(k);
        }
      if (s > size) { size = s; widest = k; }
      }
    double middle = 0.5*(b.low[widest] + b.high[widest]);
    if (discrete)
      {
      // cut between the candidates: at the median of their coordinates
      std::vector<double> values;
      for (unsigned int c : b.candidates) values.push_back(problem.candidate(c)[widest]);
      std::sort(values.begin(), values.end());
      double low = values[(values.size() - 1)/2], high = values[values.size()/2];
      middle = (low == high) ? low : 0.5*(low + high);
      if (values.size() == 2) middle = 0.5*(values[0] + values[1]);
      }
    first = b; second = b;
    first.high[widest] = middle;
    second.low[widest] = middle;
    first.objective = second.objective = std::numeric_limits<double>::infinity();
    first.candidates.clear(); second.candidates.clear();
    for (unsigned int c : b.candidates) (problem.candidate(c)[widest] < middle ? first : second).candidates.push_back(c);
    if (discrete && (first.candidates.empty() || second.candidates.empty()))
      {
      // all on one side (ties): divide the list itself
      first.candidates.assign(b.candidates.begin(), b.candidates.begin() + b.candidates.size()/2);
      second.candidates.assign(b.candidates.begin() + b.candidates.size()/2, b.candidates.end());
      }
    };
  Box whole;
  for (unsigned int k=0; k<d; k++) { whole.low.push_back(box.lower(k)); whole.high.push_back(box.upper(k)); }
  if (discrete) whole.candidates = allowedCandidates(problem, domain);
  sample(whole);
  std::vector<Box> beam{ whole };
  const int kept = _boxes > 0 ? _boxes : std::max(32, int(16*d));
  bool converged = false;
  String reason = "budget spent";
  while (!budgetSpent(problem, options, iterations))
    {
    iterations++;
    // the boxes still to divide: larger than the tolerance, or holding several candidates
    std::vector<Box> next;
    bool divided = false;
    for (const Box & b : beam)
      {
      bool small = true;
      if (discrete) small = b.candidates.size() <= 1;
      else for (unsigned int k=0; k<d; k++) small = small && (b.high[k] - b.low[k]) <= options.tolerance(k, domain);
      if (small) { next.push_back(b); continue; }
      Box first, second;
      split(b, first, second);
      sample(first);
      sample(second);
      next.push_back(first);
      next.push_back(second);
      divided = true;
      if (budgetSpent(problem, options, iterations)) break;
      }
    // keep the most promising boxes
    std::sort(next.begin(), next.end(), [](const Box & p, const Box & q) { return p.objective < q.objective; });
    while (!next.empty() && !std::isfinite(next.back().objective) && next.size() > 1) next.pop_back();
    if (int(next.size()) > kept) next.resize(kept);
    beam = next;
    if (options.keepHistory) result.history.push_back(result.value);
    if (!divided) { converged = true; reason = discrete ? "each box holds one candidate" : "boxes smaller than the tolerance"; break; }
    }
  finish(result, problem, iterations, converged, reason);
  return result;
  }

  } // namespace MATH
} // namespace CAP
