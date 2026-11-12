/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file CubicSpline.hpp
//! @brief CubicSpline: cubic interpolation of points at any spacing (natural, clamped, not-a-knot, monotone).
#ifndef CAP__MATH__CubicSpline
#define CAP__MATH__CubicSpline
#include <vector>
#include "PiecewisePolynomial.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Cubic interpolation of points (x_i, y_i), x increasing at any spacing: a cubic polynomial between
  //! successive points, the function and its slope continuous.
  //!
  //! Kinds:
  //!  - Natural: the second derivative continuous too, and zero at the ends;
  //!  - Clamped: the second derivative continuous, the slopes at the ends given;
  //!  - NotAKnot (default): the second derivative continuous, the third continuous at the second and next-to-last
  //!    points (exact for cubic polynomials);
  //!  - Monotone: the slopes limited so that the interpolation does not overshoot (Fritsch-Carlson, as pchip):
  //!    monotonic data give a monotonic function; the second derivative is not continuous.
  //! With 2 points the interpolation is linear; with 3, NotAKnot is the parabola through them.
  //!
  //! A CubicSpline is a PiecewisePolynomial: its derivatives (to any order) and its integrals are exact, and it is
  //! a Function (evaluated, plotted, integrated, differentiated as any).
  //!
  //! @code
  //!   std::vector<double> x = {0, 1, 2, 3}, y = {0, 1, 8, 27};
  //!   CAP::MATH::CubicSpline s(x, y);                     // not-a-knot: x^3 exactly
  //!   double v = s(1.5), d = s.derivativeAt(1.5);          // 3.375, 6.75
  //!   CAP::MATH::CubicSpline n(x, y, CAP::MATH::CubicSpline::Natural);
  //! @endcode
  //!
  class CubicSpline : public PiecewisePolynomial
  {
  public:

  //! @brief Kinds of splines.
  enum Kind
  {
  Natural,    //!< second derivative zero at the ends
  Clamped,    //!< slopes given at the ends
  NotAKnot,   //!< third derivative continuous at the second and next-to-last points
  Monotone    //!< no overshoot (Fritsch-Carlson)
  };

  //! @brief Empty spline.
  CubicSpline();
  //! @brief Spline through points. @param x increasing abscissas (2 or more) @param y values @param kind kind
  //! @param slopeLow slope at the first point (Clamped) @param slopeHigh slope at the last point (Clamped)
  //! @throws MathException if the sizes differ, there are fewer than 2 points, or x does not increase
  CubicSpline(const std::vector<double> & x, const std::vector<double> & y, Kind kind=NotAKnot,
              double slopeLow=0.0, double slopeHigh=0.0);
  //! @brief Destructor.
  ~CubicSpline() override { }
  //! @brief Make the spline through new points. @param x abscissas @param y values @param kind kind
  //! @param slopeLow slope at the first point (Clamped) @param slopeHigh slope at the last point (Clamped)
  //! @throws MathException as the constructor
  void setPoints(const std::vector<double> & x, const std::vector<double> & y, Kind kind=NotAKnot,
                 double slopeLow=0.0, double slopeHigh=0.0);
  //! @brief Kind. @return kind
  Kind kind() const { return _kind; }
  //! @brief Name of a kind. @param kind kind @return name
  static String kindName(Kind kind);
  //! @brief Slopes of the spline at the points (Hermite form). @param x abscissas @param y values @param kind kind
  //! @param slopeLow slope at the first point (Clamped) @param slopeHigh slope at the last point (Clamped)
  //! @return slopes
  static std::vector<double> slopes(const std::vector<double> & x, const std::vector<double> & y, Kind kind,
                                    double slopeLow=0.0, double slopeHigh=0.0);

  protected:

  Kind _kind;   //!< kind
  };

  } // namespace MATH
} // namespace CAP

#endif // CAP__MATH__CubicSpline
