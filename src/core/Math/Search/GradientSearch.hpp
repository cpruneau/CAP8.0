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
//! @file GradientSearch.hpp
//! @brief GradientSearch: extremum search by gradient descent (ascent for a maximum) with a line search.
#ifndef CAP__MATH__GradientSearch
#define CAP__MATH__GradientSearch
#include "SearchEngine.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Extremum search by gradient descent (gradient ascent for a maximum): steps against the gradient of the
  //! objective, each one as long as a backtracking line search allows (Armijo condition).
  //!
  //! The gradient is that of the problem (analytic, see SearchProblem::hasGradient()) or central differences.
  //! Coordinates are scaled by the widths of the domain, so that variables of different sizes step alike. Steps are
  //! projected on the bounds, and shortened if they land in an exclusion region. An optional momentum (heavy ball)
  //! speeds the descent along long valleys. The search stops when the steps are below the tolerance, or the gradient
  //! vanishes.
  //!
  //! For a discrete problem, it moves to the best neighbor until none is better (steepest descent).
  //!
  class GradientSearch : public SearchEngine
  {
  public:

  //! @brief Engine with the default settings.
  GradientSearch() : SearchEngine("gradient"), _initialStep(0.1), _momentum(0.0), _gradientTolerance(1E-12), _maxLineSearch(50) { }
  //! @brief First step, as a fraction of the widths of the domain. @return step
  double initialStep() const { return _initialStep; }
  //! @brief Set the first step. @param step fraction of the widths of the domain
  void   setInitialStep(double step) { _initialStep = step > 0 ? step : 0.1; }
  //! @brief Fraction of the previous step added to the next one (0: none, below 1). @return momentum
  double momentum() const { return _momentum; }
  //! @brief Set the momentum. @param momentum fraction in [0,1)
  void   setMomentum(double momentum) { _momentum = momentum < 0 ? 0 : (momentum > 0.99 ? 0.99 : momentum); }
  //! @brief Size of the scaled gradient below which it vanishes, relative to the objective. @return tolerance
  double gradientTolerance() const { return _gradientTolerance; }
  //! @brief Set the size of the gradient below which it vanishes. @param tolerance tolerance
  void   setGradientTolerance(double tolerance) { _gradientTolerance = tolerance; }
  //! @brief Halvings of a step tried by the line search. @param n number
  void   setMaxLineSearch(int n) { _maxLineSearch = n < 1 ? 1 : n; }
  //! @brief Descend from the start point (see SearchEngine::startPoint()). @param problem problem @param domain
  //! domain @param options options @return result
  SearchResult search(const SearchProblem & problem, const SearchDomain & domain, const SearchOptions & options) override;

  protected:

  double _initialStep;   //!< first step (fraction of the widths)
  double _momentum;   //!< fraction of the previous step added
  double _gradientTolerance;   //!< size of the gradient below which it vanishes
  int    _maxLineSearch;   //!< halvings of a step tried
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__GradientSearch
