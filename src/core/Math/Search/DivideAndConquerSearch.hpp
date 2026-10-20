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
//! @file DivideAndConquerSearch.hpp
//! @brief DivideAndConquerSearch: extremum search by recursive subdivision of the domain.
#ifndef CAP__MATH__DivideAndConquerSearch
#define CAP__MATH__DivideAndConquerSearch
#include "SearchEngine.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Extremum search by divide and conquer: the domain is cut in halves along its widest dimension, each
  //! half is sampled (its center and a few random points, or a few of its candidates), the boxes() most promising
  //! boxes are kept and cut again, until they are smaller than the tolerance.
  //!
  //! Keeping several boxes (a beam) protects against a misleading sample; more boxes make the search more global
  //! and more costly. On functions with very many local extrema (beyond 2 or 3 dimensions), differential evolution
  //! or the genetic algorithm are more reliable. For a discrete problem, a box holds candidates, and the search stops when each kept box holds
  //! one candidate.
  //!
  class DivideAndConquerSearch : public SearchEngine
  {
  public:

  //! @brief Engine with the default settings.
  DivideAndConquerSearch() : SearchEngine("divide and conquer"), _samples(8), _boxes(0) { }
  //! @brief Set the points sampled in each box (the center first). @param n number (at least 1)
  void setSamplesPerBox(int n) { _samples = n < 1 ? 1 : n; }
  //! @brief Points sampled in each box. @return number
  int  samplesPerBox() const { return _samples; }
  //! @brief Set the boxes kept at each subdivision. @param n number (0: 16 per dimension, at least 32)
  void setBoxes(int n) { _boxes = n < 0 ? 0 : n; }
  //! @brief Boxes kept at each subdivision. @return number (0: automatic)
  int  boxes() const { return _boxes; }
  //! @brief Divide the domain (bounded; for a discrete problem, the extent of the candidates bounds the unbounded
  //! dimensions). @param problem problem @param domain domain @param options options @return result
  SearchResult search(const SearchProblem & problem, const SearchDomain & domain, const SearchOptions & options) override;

  protected:

  int _samples;   //!< points sampled per box
  int _boxes;   //!< boxes kept (0: automatic)
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__DivideAndConquerSearch
