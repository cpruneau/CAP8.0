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
//! @file ExtremumSearch.hpp
//! @brief ExtremumSearch: the master engine of extremum searches, for Function and DataPoints objects.
#ifndef CAP__MATH__ExtremumSearch
#define CAP__MATH__ExtremumSearch
#include "AnnealingSearch.hpp"
#include "DivideAndConquerSearch.hpp"
#include "EvolutionarySearch.hpp"
#include "ExploreExploitSearch.hpp"
#include "GradientSearch.hpp"
#include "GreedySearch.hpp"
#include "HillClimbingSearch.hpp"
#include "ScanSearch.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Master engine of the extremum searches: the minimum or maximum of any Function or DataPoints, over a
  //! domain with exclusion regions, by the method chosen.
  //!
  //! The method is set with setMethod() (by value or by name), or chosen automatically (kAutomatic, the default):
  //!  - data points: a scan of all the allowed points (exact);
  //!  - a function over a bounded domain: differential evolution (global), then the gradient method from the best
  //!    point;
  //!  - a function over an unbounded domain: the gradient method from the start point.
  //!
  //! Local methods: gradient, hill climbing, greedy. Global methods: scan, simulated annealing, genetic, differential
  //! evolution, divide and conquer, explore-exploit. A second method can refine the result of the first
  //! (setPolish()), e.g. simulated annealing followed by the gradient method; the method can also be run from
  //! several starting points (setMultiStart(): the start point of the domain, then random points), keeping the best. For data on a grid (histograms), the best point is also placed between the bins by a parabolic
  //! fit (setRefineData(), on by default; see SearchResult::refinedPoint).
  //!
  //! The precision and the budget are those of options(); the settings of each engine are reached through scan(),
  //! gradient(), hillClimbing() and greedy().
  //!
  //! @code
  //!   CAP::MATH::ExtremumSearch search;
  //!   CAP::MATH::SearchDomain domain({-5.0, -5.0}, {5.0, 5.0});
  //!   domain.exclude(0, -1.0, 1.0);                       // not where -1 <= x <= 1
  //!   search.setMethod("hill climbing");
  //!   search.options().xTolerance = 1E-8;
  //!   CAP::MATH::SearchResult r = search.minimum(f, domain);
  //!   if (r.converged) ... r.point, r.value ...
  //!   CAP::MATH::SearchResult peak = search.maximum(histogram);   // the highest bin, refined between bins
  //! @endcode
  //!
  class ExtremumSearch
  {
  public:

  //! @brief Methods of search.
  enum Method { kAutomatic, kScan, kGradient, kHillClimbing, kGreedy, kAnnealing, kGenetic, kDifferentialEvolution,
                kDivideAndConquer, kExploreExploit, kNone };

  //! @brief Master engine: automatic choice, no polish, data refined, default options.
  ExtremumSearch();

  //! @brief Method used. @return method
  Method method() const { return _method; }
  //! @brief Set the method. @param method method (kNone is not a method: ignored)
  void setMethod(Method method) { if (method != kNone) _method = method; }
  //! @brief Set the method by name ("automatic", "scan", "gradient", "hill climbing", "greedy", "simulated
  //! annealing" or "annealing", "genetic", "differential evolution", "divide and conquer", "explore exploit"; case,
  //! blanks and dashes ignored). @param name name @return false if the name is unknown (the method is unchanged)
  bool setMethod(const String & name);
  //! @brief Method that refines the result of the first one (kNone: none). @return method
  Method polish() const { return _polish; }
  //! @brief Set the method that refines the result of the first one. @param method method (kNone: none; kAutomatic:
  //! the gradient method)
  void setPolish(Method method) { _polish = method; }
  //! @brief Number of runs of the method, from the start point of the domain then from random points. @return number
  int  multiStart() const { return _multiStart; }
  //! @brief Set the number of runs of the method (the best is kept; the budget is shared). @param n number (1: one run)
  void setMultiStart(int n) { _multiStart = n < 1 ? 1 : n; }
  //! @brief Whether the best point of data on a grid is placed between the bins. @return true if so
  bool refineData() const { return _refineData; }
  //! @brief Place the best point of data on a grid between the bins, or not. @param v refine
  void setRefineData(bool v) { _refineData = v; }

  //! @brief Precision and budget. @return options
  SearchOptions & options() { return _options; }
  const SearchOptions & options() const { return _options; }   //!< Precision and budget
  //! @brief Set the precision and budget. @param options options
  void setOptions(const SearchOptions & options) { _options = options; }

  ScanSearch & scan() { return _scan; }   //!< The scan engine, for its settings
  GradientSearch & gradient() { return _gradient; }   //!< The gradient engine, for its settings
  HillClimbingSearch & hillClimbing() { return _hillClimbing; }   //!< The hill climbing engine, for its settings
  GreedySearch & greedy() { return _greedy; }   //!< The greedy engine, for its settings
  AnnealingSearch & annealing() { return _annealing; }   //!< The simulated annealing engine, for its settings
  EvolutionarySearch & genetic() { return _genetic; }   //!< The genetic engine, for its settings
  EvolutionarySearch & differentialEvolution() { return _differential; }   //!< The differential evolution engine, for its settings
  DivideAndConquerSearch & divideAndConquer() { return _divide; }   //!< The divide and conquer engine, for its settings
  ExploreExploitSearch & exploreExploit() { return _explore; }   //!< The explore-exploit engine, for its settings
  //! @brief The engine of a method. @param method method (not kAutomatic or kNone) @return engine
  SearchEngine & engine(Method method);

  //! @brief Name of a method. @param method method @return name
  static const char * name(Method method);
  //! @brief Method of a name (see setMethod(const String&)). @param name name @return method, kNone if unknown
  static Method method(const String & name);
  //! @brief The method kAutomatic chooses for a problem and a domain. @param problem problem @param domain domain
  //! @return method
  static Method automaticMethod(const SearchProblem & problem, const SearchDomain & domain);

  //!
  //! @brief Search: the method (or the automatic choice, possibly from several starts), then the polish, then the
  //! refinement of data.
  //! @param problem problem @param domain domain (same dimension as the problem) @return result
  //! @throws MathException if the dimensions differ
  //!
  SearchResult search(const SearchProblem & problem, const SearchDomain & domain);
  //! @brief Minimum of a function. @param function function @param domain domain @return result
  SearchResult minimum(const Function & function, const SearchDomain & domain);
  //! @brief Maximum of a function. @param function function @param domain domain @return result
  SearchResult maximum(const Function & function, const SearchDomain & domain);
  //! @brief Minimum of data points over a domain. @param data data @param domain domain @return result
  SearchResult minimum(const DataPoints & data, const SearchDomain & domain);
  //! @brief Maximum of data points over a domain. @param data data @param domain domain @return result
  SearchResult maximum(const DataPoints & data, const SearchDomain & domain);
  //! @brief Minimum of data points (over all of them). @param data data @return result
  SearchResult minimum(const DataPoints & data);
  //! @brief Maximum of data points (over all of them). @param data data @return result
  SearchResult maximum(const DataPoints & data);

  protected:

  Method _method;   //!< method
  Method _polish;   //!< method refining the result (kNone: none)
  int    _multiStart;   //!< runs of the method
  bool   _refineData;   //!< best point of gridded data placed between the bins
  SearchOptions _options;   //!< precision and budget
  ScanSearch _scan;   //!< scan engine
  GradientSearch _gradient;   //!< gradient engine
  HillClimbingSearch _hillClimbing;   //!< hill climbing engine
  GreedySearch _greedy;   //!< greedy engine
  AnnealingSearch _annealing;   //!< simulated annealing engine
  EvolutionarySearch _genetic;   //!< genetic engine
  EvolutionarySearch _differential;   //!< differential evolution engine
  DivideAndConquerSearch _divide;   //!< divide and conquer engine
  ExploreExploitSearch _explore;   //!< explore-exploit engine

  //! @brief Run a method, from several starts if asked. @param method method @param problem problem
  //! @param domain domain @param options options @return best result
  SearchResult run(Method method, const SearchProblem & problem, const SearchDomain & domain, const SearchOptions & options);
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__ExtremumSearch
