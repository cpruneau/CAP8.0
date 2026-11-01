/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Region.hpp
//! @brief Region: a box made of intervals along some of the dimensions of a space.
#ifndef CAP__MATH__Region
#define CAP__MATH__Region
#include <string>
#include <utility>
#include <vector>

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief A region of a space of any dimension: intervals along some dimensions, the others not restricted (a band,
  //! a slab, or a box). A point is in the region if it is in all its intervals (bounds included).
  //!
  //! Used to leave regions out of an extremum search (SearchDomain::exclude()) and to choose the points of a fit
  //! (FitSelection: regions included and excluded).
  //!
  //! @code
  //!   Region band = Region::interval(0, 1.2, 1.4);               // 1.2 <= x <= 1.4, any y
  //!   Region box = Region().add(0, 0.0, 1.0).add(1, 2.0, 3.0);   // the box [0,1] x [2,3]
  //! @endcode
  //!
  class Region
  {
  public:

  //! @brief Region without intervals (it contains every point; add intervals to restrict it).
  Region() { }
  //! @brief Add (or replace) the interval of a dimension (ends in any order). @param dimension dimension (0 for x)
  //! @param low lower end @param high upper end @return this region
  Region & add(unsigned int dimension, double low, double high);
  //! @brief Region of one interval. @param dimension dimension @param low lower end @param high upper end @return region
  static Region interval(unsigned int dimension, double low, double high) { return Region().add(dimension, low, high); }
  //! @brief Box over all the dimensions. @param low lower ends @param high upper ends @return region
  static Region box(const std::vector<double> & low, const std::vector<double> & high);
  //! @brief Intervals of the region: (dimension, (low, high)). @return intervals
  const std::vector<std::pair<unsigned int,std::pair<double,double>>> & intervals() const { return _intervals; }
  //! @brief Whether the region restricts a dimension. @param dimension dimension @return true if it has an interval along it
  bool restricts(unsigned int dimension) const;
  //! @brief Interval along a dimension. @param dimension dimension @param low lower end (set) @param high upper end (set)
  //! @return false if the region does not restrict that dimension (low and high unchanged)
  bool intervalOf(unsigned int dimension, double & low, double & high) const;
  //! @brief Remove the interval of a dimension (the region no longer restricts it). @param dimension dimension
  //! @return this region
  Region & remove(unsigned int dimension);
  //! @brief Whether the region has no interval (it contains every point). @return true if so
  bool unrestricted() const { return _intervals.empty(); }
  //! @brief Whether the point is in the region. @param x point @return true if it is in all the intervals
  bool contains(const std::vector<double> & x) const;
  //! @brief Whether all the intervals are finite and along dimensions below rank. @param rank number of dimensions
  //! @return true if so
  bool isValid(unsigned int rank) const;
  //!
  //! @brief Text of the region, e.g. "x in [2, 3]" or "x in [0, 1] and y in [2, 3]" ("everywhere" without intervals).
  //! @param names names of the dimensions (x, y, z, then x3, x4, ... by default) @return text
  //!
  std::string toString(const std::vector<std::string> & names = {}) const;
  //! @brief Name of a dimension: x, y, z, then x3, x4, ... @param dimension dimension @return name
  static std::string dimensionName(unsigned int dimension);
  //! @brief Same intervals (in any order). @param other region @return true if so
  bool operator==(const Region & other) const;
  //! @brief Different intervals. @param other region @return true if so
  bool operator!=(const Region & other) const { return !(*this == other); }

  protected:

  std::vector<std::pair<unsigned int,std::pair<double,double>>> _intervals;   //!< (dimension, (low, high))
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__Region
