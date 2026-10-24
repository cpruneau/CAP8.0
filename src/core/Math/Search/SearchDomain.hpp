/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file SearchDomain.hpp
//! @brief SearchDomain and ExclusionRegion: where an extremum search may look.
#ifndef CAP__MATH__SearchDomain
#define CAP__MATH__SearchDomain
#include <limits>
#include <utility>
#include <vector>

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief A region left out of a search: intervals in one or more dimensions (the other dimensions are not
  //! restricted). A point is in the region if it is in all its intervals (bounds included).
  //!
  //! @code
  //!   ExclusionRegion band = ExclusionRegion::interval(0, 1.2, 1.4);               // 1.2 <= x <= 1.4, any y
  //!   ExclusionRegion box = ExclusionRegion().add(0, 0.0, 1.0).add(1, 2.0, 3.0);   // the box [0,1] x [2,3]
  //! @endcode
  //!
  class ExclusionRegion
  {
  public:

  //! @brief Region without intervals (it contains every point; add intervals to restrict it).
  ExclusionRegion() { }
  //! @brief Add (or replace) the interval of a dimension. @param dimension dimension (0 for x)
  //! @param low lower end @param high upper end @return this region
  ExclusionRegion & add(unsigned int dimension, double low, double high);
  //! @brief Region of one interval. @param dimension dimension @param low lower end @param high upper end @return region
  static ExclusionRegion interval(unsigned int dimension, double low, double high) { return ExclusionRegion().add(dimension, low, high); }
  //! @brief Box over all the dimensions. @param low lower ends @param high upper ends @return region
  static ExclusionRegion box(const std::vector<double> & low, const std::vector<double> & high);
  //! @brief Intervals of the region: (dimension, (low, high)). @return intervals
  const std::vector<std::pair<unsigned int,std::pair<double,double>>> & intervals() const { return _intervals; }
  //! @brief Whether the point is in the region. @param x point @return true if it is in all the intervals
  bool contains(const std::vector<double> & x) const;

  protected:

  std::vector<std::pair<unsigned int,std::pair<double,double>>> _intervals;   //!< (dimension, (low, high))
  };

  //!
  //! @brief Where an extremum search may look: bounds in each dimension (infinite by default), regions left out
  //! (ExclusionRegion), and an optional start point.
  //!
  //! Global engines need finite bounds; local engines can start from a point in an unbounded domain. A point is
  //! allowed if it is within the bounds and in no exclusion region.
  //!
  //! @code
  //!   CAP::MATH::SearchDomain domain({-5.0, -5.0}, {5.0, 5.0});
  //!   domain.exclude(0, -1.0, 1.0);          // leave out -1 <= x <= 1
  //!   domain.setStart({3.0, 3.0});
  //! @endcode
  //!
  class SearchDomain
  {
  public:

  //! @brief Unbounded domain. @param dimension number of dimensions
  explicit SearchDomain(unsigned int dimension=1);
  //! @brief Bounded domain. @param lower lower bounds @param upper upper bounds (same size)
  SearchDomain(const std::vector<double> & lower, const std::vector<double> & upper);

  //! @brief Number of dimensions. @return number
  unsigned int dimension() const { return (unsigned int)(_lower.size()); }
  //! @brief Set the bounds of a dimension. @param k dimension @param low lower bound @param high upper bound
  void setBounds(unsigned int k, double low, double high);
  //! @brief Lower bound of a dimension. @param k dimension @return bound (-infinity if none)
  double lower(unsigned int k) const { return _lower[k]; }
  //! @brief Upper bound of a dimension. @param k dimension @return bound (+infinity if none)
  double upper(unsigned int k) const { return _upper[k]; }
  //! @brief Whether a dimension has finite bounds. @param k dimension @return true if both are finite
  bool isBounded(unsigned int k) const;
  //! @brief Whether all the dimensions have finite bounds. @return true if so
  bool isBounded() const;
  //! @brief Width of a dimension: upper - lower, or 1 if it is not bounded (the scale of steps and tolerances).
  //! @param k dimension @return width
  double width(unsigned int k) const;

  //! @brief Leave a region out of the search. @param region region
  void exclude(const ExclusionRegion & region) { _exclusions.push_back(region); }
  //! @brief Leave an interval of one dimension out of the search. @param k dimension @param low lower end
  //! @param high upper end
  void exclude(unsigned int k, double low, double high) { exclude(ExclusionRegion::interval(k, low, high)); }
  //! @brief Regions left out. @return regions
  const std::vector<ExclusionRegion> & exclusions() const { return _exclusions; }
  //! @brief Remove all the exclusion regions.
  void clearExclusions() { _exclusions.clear(); }

  //! @brief Whether a point is within the bounds. @param x point @return true if so
  bool inBounds(const std::vector<double> & x) const;
  //! @brief Whether a point is in an exclusion region. @param x point @return true if so
  bool excluded(const std::vector<double> & x) const;
  //! @brief Whether a point may be evaluated: within the bounds and not excluded. @param x point @return true if so
  bool allowed(const std::vector<double> & x) const { return inBounds(x) && !excluded(x); }
  //! @brief The point moved into the bounds (each coordinate clamped). @param x point @return point
  std::vector<double> clamped(const std::vector<double> & x) const;

  //! @brief Set the start point of local searches. @param x start point
  void setStart(const std::vector<double> & x) { _start = x; }
  //! @brief Whether a start point is set. @return true if so
  bool hasStart() const { return _start.size() == dimension(); }
  //! @brief Start point (empty if none). @return start point
  const std::vector<double> & start() const { return _start; }
  //! @brief Forget the start point.
  void clearStart() { _start.clear(); }

  protected:

  std::vector<double> _lower;   //!< lower bounds
  std::vector<double> _upper;   //!< upper bounds
  std::vector<ExclusionRegion> _exclusions;   //!< regions left out
  std::vector<double> _start;   //!< start point (empty if none)
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__SearchDomain
