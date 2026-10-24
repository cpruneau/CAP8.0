/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file HillClimbingSearch.hpp
//! @brief HillClimbingSearch: extremum search by stochastic hill climbing with adaptive steps and restarts.
#ifndef CAP__MATH__HillClimbingSearch
#define CAP__MATH__HillClimbingSearch
#include "SearchEngine.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Extremum search by hill climbing: random moves around the current point, the first that improves the
  //! objective is taken (first improvement); the step grows after a success and shrinks after a series of failures,
  //! until it is below the tolerance. Restarts from random points (in a bounded domain) look for other extrema; the
  //! best is kept.
  //!
  //! Moves are along random directions (the default) or along one coordinate at a time. No gradient is needed: the
  //! method suits functions that are not smooth.
  //!
  //! For a discrete problem, it moves to a better neighbor taken at random until none is better.
  //!
  class HillClimbingSearch : public SearchEngine
  {
  public:

  //! @brief Directions of the moves.
  enum Moves { kRandomDirections, kCoordinates };
  //! @brief Engine with the default settings.
  HillClimbingSearch() : SearchEngine("hill climbing"), _initialStep(0.1), _shrink(0.5), _grow(1.5), _tries(0), _restarts(0), _moves(kRandomDirections) { }
  //! @brief Set the first step. @param step fraction of the widths of the domain
  void setInitialStep(double step) { _initialStep = step > 0 ? step : 0.1; }
  //! @brief First step, as a fraction of the widths of the domain. @return step
  double initialStep() const { return _initialStep; }
  //! @brief Set the factors applied to the step after a series of failures and after a success.
  //! @param shrink factor below 1 @param grow factor at least 1
  void setStepFactors(double shrink, double grow) { _shrink = (shrink > 0 && shrink < 1) ? shrink : 0.5; _grow = grow >= 1 ? grow : 1.0; }
  //! @brief Set the failures in a row after which the step shrinks. @param n number (0: 2 per dimension + 2)
  void setTries(int n) { _tries = n < 0 ? 0 : n; }
  //! @brief Set the number of restarts from random points. @param n number
  void setRestarts(int n) { _restarts = n < 0 ? 0 : n; }
  //! @brief Number of restarts from random points. @return number
  int  restarts() const { return _restarts; }
  //! @brief Set the directions of the moves. @param moves random directions or coordinates
  void setMoves(Moves moves) { _moves = moves; }
  //! @brief Climb from the start point, then from random points. @param problem problem @param domain domain
  //! @param options options @return result
  SearchResult search(const SearchProblem & problem, const SearchDomain & domain, const SearchOptions & options) override;

  protected:

  //! @brief One climb from a point. @param problem problem @param domain domain @param options options
  //! @param x start point @param result result (updated) @param iterations iterations (updated) @return true if converged
  bool climb(const SearchProblem & problem, const SearchDomain & domain, const SearchOptions & options,
             std::vector<double> x, SearchResult & result, long & iterations);

  double _initialStep;   //!< first step (fraction of the widths)
  double _shrink;   //!< factor of the step after failures
  double _grow;   //!< factor of the step after a success
  int    _tries;   //!< failures in a row before the step shrinks (0: automatic)
  int    _restarts;   //!< restarts from random points
  Moves  _moves;   //!< directions of the moves
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__HillClimbingSearch
