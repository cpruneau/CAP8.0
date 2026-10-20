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
//! @file ScanSearch.hpp
//! @brief ScanSearch: extremum search by evaluating a regular grid, or every candidate of a discrete problem.
#ifndef CAP__MATH__ScanSearch
#define CAP__MATH__ScanSearch
#include "SearchEngine.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Extremum search by evaluating every point of a regular grid over a bounded domain, or every allowed
  //! candidate of a discrete problem (an exact search of data points).
  //!
  //! The grid has pointsPerAxis() points along each dimension, ends included; 0 (the default) takes as many as the
  //! budget of evaluations allows (at most 1001 per axis). The scan finds the region of the global extremum to the
  //! precision of the grid; a local engine can then refine it (see ExtremumSearch::setPolish()).
  //!
  class ScanSearch : public SearchEngine
  {
  public:

  //! @brief Engine with an automatic number of points.
  ScanSearch() : SearchEngine("scan"), _pointsPerAxis(0) { }
  //! @brief Points along each dimension of the grid. @return number (0: automatic)
  int  pointsPerAxis() const { return _pointsPerAxis; }
  //! @brief Set the points along each dimension of the grid. @param n number (0: automatic, otherwise at least 2)
  void setPointsPerAxis(int n) { _pointsPerAxis = n <= 0 ? 0 : (n < 2 ? 2 : n); }
  //! @brief Scan the grid, or the candidates. @param problem problem @param domain domain (bounded, for a
  //! continuous problem) @param options options @return result
  SearchResult search(const SearchProblem & problem, const SearchDomain & domain, const SearchOptions & options) override;

  protected:

  int _pointsPerAxis;   //!< points along each dimension (0: automatic)
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__ScanSearch
