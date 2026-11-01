/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FitSelection.hpp
//! @brief FitSelection: the points (bins, events) used by a fit: a box, and regions included and excluded.
#ifndef CAP__MATH__FitSelection
#define CAP__MATH__FitSelection
#include <string>
#include <utility>
#include <vector>
#include "Region.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief The part of the space of the variables that a fit uses: a box (low[k] <= x[k] <= high[k] for each
  //! variable; none: no limit), regions included (Region: intervals along some of the variables), and regions
  //! excluded.
  //!
  //! A point (the center of a bin, an event) is used if it is
  //!   in the box, and (in at least one included region, or there is none), and in no excluded region.
  //! Bounds are included. Without regions, the selection is the box alone (as a plain fit range).
  //!
  //! The box also sets the mapping of the functions defined on a range (series, Polynomial2D) and the range over which
  //! the density of an unbinned fit is normalized; that normalization is the integral of the function over the
  //! selection (see cells()).
  //!
  //! @code
  //!   CAP::MATH::FitSelection selection({ 0.0 }, { 10.0 });
  //!   selection.exclude(0, 4.5, 5.5);                        // leave the peak out: fit the background
  //!   fitter.setSelection(selection);
  //!
  //!   CAP::MATH::FitSelection sidebands;                     // two windows in x, any y
  //!   sidebands.include(0, 1.0, 2.0).include(0, 8.0, 9.0);
  //! @endcode
  //!
  class FitSelection
  {
  public:

  //! @brief Selection of all the points (no box, no region).
  FitSelection() { }
  //! @brief Selection of a box. @param low lower ends @param high upper ends @throws MathException if the sizes differ
  //! or a lower end is above an upper end
  FitSelection(const std::vector<double> & low, const std::vector<double> & high) { setBox(low, high); }

  //! @brief Set the box (empty vectors: no box). @param low lower ends @param high upper ends
  //! @throws MathException if the sizes differ or a lower end is above an upper end
  void setBox(const std::vector<double> & low, const std::vector<double> & high);
  //! @brief Remove the box (no limit).
  void clearBox() { _low.clear(); _high.clear(); }
  //! @brief Whether there is a box. @return true if so
  bool hasBox() const { return !_low.empty(); }
  //! @brief Lower ends of the box (empty: none). @return lower ends
  const std::vector<double> & low() const { return _low; }
  //! @brief Upper ends of the box (empty: none). @return upper ends
  const std::vector<double> & high() const { return _high; }

  //! @brief Include a region (the points used are in at least one included region). @param region region @return this
  FitSelection & include(const Region & region) { _included.push_back(region); return *this; }
  //! @brief Include an interval of one variable. @param k variable @param low lower end @param high upper end @return this
  FitSelection & include(unsigned int k, double low, double high) { return include(Region::interval(k, low, high)); }
  //! @brief Exclude a region. @param region region @return this
  FitSelection & exclude(const Region & region) { _excluded.push_back(region); return *this; }
  //! @brief Exclude an interval of one variable. @param k variable @param low lower end @param high upper end @return this
  FitSelection & exclude(unsigned int k, double low, double high) { return exclude(Region::interval(k, low, high)); }
  //! @brief Regions included. @return regions
  const std::vector<Region> & included() const { return _included; }
  //! @brief Regions excluded. @return regions
  const std::vector<Region> & excluded() const { return _excluded; }
  //! @brief Replace the regions. @param included regions included @param excluded regions excluded
  void setRegions(const std::vector<Region> & included, const std::vector<Region> & excluded) { _included = included; _excluded = excluded; }
  //! @brief Remove all the regions (the box is kept).
  void clearRegions() { _included.clear(); _excluded.clear(); }
  //! @brief Whether there are regions. @return true if so
  bool hasRegions() const { return !_included.empty() || !_excluded.empty(); }
  //! @brief Whether the selection is a box alone (or nothing). @return true if there is no region
  bool isBox() const { return !hasRegions(); }
  //! @brief Whether the selection uses all the points (no box, no region). @return true if so
  bool everything() const { return !hasBox() && !hasRegions(); }

  //! @brief Whether a point is used. @param x coordinates @return true if so
  bool contains(const std::vector<double> & x) const;
  //! @brief Whether a point of rank 1 is used. @param x coordinate @return true if so
  bool contains(double x) const { return contains(std::vector<double>{ x }); }
  //!
  //! @brief Check the selection for rank variables: a box of that size, finite intervals along variables below rank.
  //! @param rank number of variables @throws MathException if not valid
  //!
  void validate(unsigned int rank) const;
  //!
  //! @brief The box split into cells by the ends of the intervals of the regions, keeping the cells used: boxes whose
  //! union is the selection (up to their boundaries). Each cell is wholly in or out of every region, so the integral of
  //! a function over the selection is the sum of its integrals over the cells. @return (lower ends, upper ends) of the cells
  //! @throws MathException if there is no box or there would be more than maxCells cells to test
  //! @param maxCells largest number of cells tested
  //!
  std::vector<std::pair<std::vector<double>,std::vector<double>>> cells(unsigned int maxCells=1000000) const;
  //!
  //! @brief Text of the selection, e.g. "[0, 10]; excluding x in [4.5, 5.5]" ("all points" if everything()).
  //! @param names names of the variables (x, y, z, ... by default) @return text
  //!
  std::string toString(const std::vector<std::string> & names = {}) const;
  //! @brief Same box and regions. @param other selection @return true if so
  bool operator==(const FitSelection & other) const;
  //! @brief Different box or regions. @param other selection @return true if so
  bool operator!=(const FitSelection & other) const { return !(*this == other); }

  protected:

  std::vector<double> _low;   //!< lower ends of the box (empty: none)
  std::vector<double> _high;   //!< upper ends of the box (empty: none)
  std::vector<Region> _included;   //!< regions included (none: the whole box)
  std::vector<Region> _excluded;   //!< regions excluded
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__FitSelection
