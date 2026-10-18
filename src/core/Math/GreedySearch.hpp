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
//! @file GreedySearch.hpp
//! @brief GreedySearch: extremum search by greedy moves to the best neighbor on a refining lattice.
#ifndef CAP__MATH__GreedySearch
#define CAP__MATH__GreedySearch
#include "SearchEngine.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Extremum search by greedy moves: all the neighbors of the current point on a lattice (along the axes,
  //! and the diagonals if enabled) are evaluated and the best is taken (best improvement); when none is better, the
  //! lattice is refined, until its spacing is below the tolerance.
  //!
  //! More evaluations per step than hill climbing (3^n - 1 neighbors with the diagonals, 2n without), but each move
  //! is the best one available. For a discrete problem, it moves to the best neighbor until none is better.
  //!
  class GreedySearch : public SearchEngine
  {
  public:

  //! @brief Engine with the default settings (diagonals in up to 3 dimensions).
  GreedySearch() : SearchEngine("greedy"), _initialSpacing(0.1), _refine(0.5), _diagonals(-1) { }
  //! @brief Set the first spacing of the lattice. @param spacing fraction of the widths of the domain
  void setInitialSpacing(double spacing) { _initialSpacing = spacing > 0 ? spacing : 0.1; }
  //! @brief First spacing of the lattice, as a fraction of the widths of the domain. @return spacing
  double initialSpacing() const { return _initialSpacing; }
  //! @brief Set the factor applied to the spacing when no neighbor is better. @param factor factor below 1
  void setRefinement(double factor) { _refine = (factor > 0 && factor < 1) ? factor : 0.5; }
  //! @brief Set whether the diagonal neighbors are tried. @param v yes or no
  void setDiagonals(bool v) { _diagonals = v ? 1 : 0; }
  //! @brief Move from the start point. @param problem problem @param domain domain @param options options
  //! @return result
  SearchResult search(const SearchProblem & problem, const SearchDomain & domain, const SearchOptions & options) override;

  protected:

  double _initialSpacing;   //!< first spacing (fraction of the widths)
  double _refine;   //!< factor of the spacing when no neighbor is better
  int    _diagonals;   //!< diagonals tried: 1 yes, 0 no, -1 in up to 3 dimensions
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__GreedySearch
