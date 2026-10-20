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
//! @file SearchEngine.hpp
//! @brief SearchEngine: base of the extremum search engines; SearchOptions and SearchResult.
#ifndef CAP__MATH__SearchEngine
#define CAP__MATH__SearchEngine
#include <random>
#include <vector>
#include "Aliases.hpp"
#include "SearchDomain.hpp"
#include "SearchProblem.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Precision and budget of a search.
  //!
  //! The search stops when the points it compares are closer than the tolerance in every dimension, when the
  //! objective changes by less than fTolerance (relative), or when a budget is spent. The tolerance in dimension k
  //! is xTolerance times the width of the domain in that dimension (1 if it is unbounded), or absoluteTolerances[k]
  //! if given.
  //!
  struct SearchOptions
  {
    double xTolerance = 1E-6;   //!< tolerance on the position, relative to the width of the domain
    std::vector<double> absoluteTolerances;   //!< absolute tolerances, one per dimension (empty: relative)
    double fTolerance = 1E-12;   //!< tolerance on the change of the objective, relative to its size
    long   maxEvaluations = 200000;   //!< evaluations of the objective allowed
    long   maxIterations = 100000;   //!< iterations allowed (the meaning depends on the engine)
    unsigned long long seed = 12345;   //!< seed of the random numbers (the same seed gives the same search)
    bool   keepHistory = false;   //!< record the best value after each iteration

    //! @brief Tolerance on the position in one dimension. @param k dimension @param domain domain @return tolerance
    double tolerance(unsigned int k, const SearchDomain & domain) const
    { return k < absoluteTolerances.size() ? absoluteTolerances[k] : xTolerance*domain.width(k); }
  };

  //!
  //! @brief Outcome of a search: the best point found, its value, and how the search went.
  //!
  struct SearchResult
  {
    bool   found = false;   //!< an allowed point was evaluated
    bool   converged = false;   //!< stopped because the tolerance was reached (not by a budget)
    std::vector<double> point;   //!< best point
    double value = 0;   //!< value of the problem at the best point (not the objective: a maximum is positive)
    int    candidate = -1;   //!< index of the best candidate of a discrete problem (-1: continuous)
    bool   refined = false;   //!< refinedPoint and refinedValue are set (see SearchProblem::refine())
    std::vector<double> refinedPoint;   //!< extremum placed between the candidates
    double refinedValue = 0;   //!< value at refinedPoint
    long   nEvaluations = 0;   //!< evaluations of the objective
    long   nIterations = 0;   //!< iterations
    String engine;   //!< engine (or engines, e.g. "scan + gradient") that found it
    String stopReason;   //!< why the search stopped
    std::vector<double> history;   //!< best value after each iteration (if SearchOptions::keepHistory)
  };

  //!
  //! @brief Base of the extremum search engines (ScanSearch, GradientSearch, HillClimbingSearch, GreedySearch, ...).
  //!
  //! An engine minimizes the objective of a SearchProblem (see SearchProblem::objective()) over the allowed points of
  //! a SearchDomain, within the precision and budget of SearchOptions, and returns a SearchResult. Engines never
  //! evaluate a point outside the bounds or in an exclusion region. Each engine handles continuous problems and
  //! discrete ones (only their candidates, moving from neighbor to neighbor).
  //!
  //! The base class provides what engines share: random numbers (seeded from the options, so a search can be
  //! repeated), random and start points, the bookkeeping of the best point, and the descent among the neighbors of a
  //! discrete problem.
  //!
  class SearchEngine
  {
  public:

  //! @brief Engine. @param name name of the engine
  explicit SearchEngine(const String & name);
  //! @brief Destructor.
  virtual ~SearchEngine() { }
  //! @brief Name of the engine. @return name
  const String & name() const { return _name; }
  //!
  //! @brief Search the extremum. @param problem problem @param domain domain (bounds, exclusions, start point)
  //! @param options precision and budget @return result
  //!
  virtual SearchResult search(const SearchProblem & problem, const SearchDomain & domain, const SearchOptions & options) = 0;

  protected:

  //! @brief Start a search: seed the random numbers, note the evaluations already made (by an earlier stage).
  //! @param problem problem @param options options
  void begin(const SearchProblem & problem, const SearchOptions & options) { _random.seed(options.seed); _firstEvaluation = problem.nEvaluations(); }
  //! @brief Evaluations made since begin(). @param problem problem @return number
  long evaluations(const SearchProblem & problem) const { return problem.nEvaluations() - _firstEvaluation; }
  //! @brief Uniform random number. @param low lower end @param high upper end @return number
  double uniform(double low=0.0, double high=1.0);
  //! @brief Gaussian random number of mean 0 and width 1. @return number
  double gaussian();
  //! @brief Random allowed point of a bounded domain. @param domain domain @param point set to the point
  //! @return false if no allowed point was found (in many tries) or the domain is not bounded
  bool randomPoint(const SearchDomain & domain, std::vector<double> & point);
  //!
  //! @brief Where a local search starts: the start point of the domain, otherwise its center (or the origin of the
  //! unbounded dimensions); moved to a random allowed point if it is not allowed.
  //! @param problem problem @param domain domain @return start point
  //!
  std::vector<double> startPoint(const SearchProblem & problem, const SearchDomain & domain);
  //! @brief Candidates of a discrete problem that are allowed in the domain. @param problem problem
  //! @param domain domain @return indices
  std::vector<unsigned int> allowedCandidates(const SearchProblem & problem, const SearchDomain & domain) const;
  //! @brief Candidate where a local search of a discrete problem starts: the allowed candidate nearest to the
  //! start point of the domain, otherwise a random allowed one. @param problem problem @param domain domain
  //! @return index, or -1 if none is allowed
  int startCandidate(const SearchProblem & problem, const SearchDomain & domain);
  //! @brief Whether the budget of evaluations or iterations is spent. @param problem problem @param options options
  //! @param iterations iterations so far @return true if spent
  bool budgetSpent(const SearchProblem & problem, const SearchOptions & options, long iterations) const;
  //! @brief Keep a point if it is the best so far. @param result result @param problem problem @param x point
  //! @param objective objective at x @return true if it is the best so far
  bool keepBest(SearchResult & result, const SearchProblem & problem, const std::vector<double> & x, double objective) const;
  //! @brief Fill the end of a result: evaluations, engine, stop reason. @param result result @param problem problem
  //! @param iterations iterations @param converged converged @param reason stop reason
  void finish(SearchResult & result, const SearchProblem & problem, long iterations, bool converged, const String & reason) const;
  //!
  //! @brief Local descent among the neighbors of a discrete problem, from a candidate: move to the best neighbor
  //! (best improvement) or to the first better one in random order (first improvement), until no neighbor is better.
  //! @param problem problem @param domain domain @param options options @param start start candidate @param best
  //! best improvement, otherwise first improvement @param result result (updated) @return iterations
  //!
  long discreteDescent(const SearchProblem & problem, const SearchDomain & domain, const SearchOptions & options,
                       int start, bool best, SearchResult & result);

  String _name;   //!< name of the engine
  std::mt19937_64 _random;   //!< random numbers
  long _firstEvaluation = 0;   //!< evaluations of the problem when the search began
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__SearchEngine
