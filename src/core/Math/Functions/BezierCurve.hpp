/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file BezierCurve.hpp
//! @brief BezierCurve: a smooth curve through knots, made of cubic Bezier segments (Bernstein polynomials), drawn
//! and edited with the mouse (e.g. efficiencies and resolutions of the detector maps of CapGui).
#ifndef CAP_MATH_BezierCurve
#define CAP_MATH_BezierCurve
#include <vector>
#include "ParameterizedFunction.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief A function y(x) through n >= 2 knots (x_k, y_k), each segment between two knots a cubic Bezier curve:
  //! y(t) = (1-t)^3 P0 + 3(1-t)^2 t P1 + 3(1-t) t^2 P2 + t^3 P3, with P0 = y_k, P3 = y_k+1, and x = x_k + t h
  //! (h = x_k+1 - x_k). The inner control points sit at the thirds of the segment in x, so that x is linear in t:
  //! every segment is a cubic in x and the curve is single valued (a function of x), whatever the knots.
  //!
  //! The slopes at the knots are set automatically (Fritsch-Butland, monotonicity preserving): zero at a local
  //! extremum of the knots, and no larger than 3 times the slopes of the segments, so that the control points P1 and
  //! P2 of a segment lie between its two knots. A Bezier segment lies within the range of its control points: the
  //! curve never goes beyond its knots (no overshoot), and stays within bounds [lower, upper] whenever the knots do.
  //! The curve is continuous with a continuous derivative. Outside [x_0, x_n-1] its value is that of the end knot.
  //!
  //! Parameters: x0, y0, x1, y1, ... (2n of them); the bounds are those of the parameters y_k (e.g. [0, 1] for an
  //! efficiency, >= 0 for a resolution), kept by the files with the parameters. The knots are kept in increasing x.
  //! The class is registered in FunctionRegistry as "BezierCurve" (made with 2n parameters: n knots); a product of
  //! curves of several variables is a ParameterizedComposite (kProduct), e.g. eff(pT) f(eta) g(phi).
  //!
  //! @code
  //!   CAP::MATH::BezierCurve efficiency({0.0, 0.5, 2.0, 10.0}, {0.0, 0.6, 0.9, 0.95}, 0.0, 1.0);
  //!   double e = efficiency(1.0);          // in [0, 1]
  //!   int k = efficiency.insertKnot(5.0);   // a knot on the curve (its shape barely changes)
  //!   efficiency.moveKnot(k, 5.0, 0.93);
  //! @endcode
  //!
  class BezierCurve : public ParameterizedFunction
  {
  public:

  //! @brief Straight line from (0, 0) to (1, 1), unbounded.
  BezierCurve();
  //! @brief Curve with nKnots knots on y = 0 for x in [0, 1] (the registry makes it with 2 nKnots parameters).
  //! @param nKnots number of knots (2 or more) @throws MathException if fewer than 2
  explicit BezierCurve(unsigned int nKnots);
  //! @brief Curve through knots, values bounded (each y clamped to [lower, upper]).
  //! @param x abscissas (sorted here; distinct) @param y values @param lower lower bound @param upper upper bound
  //! @throws MathException if fewer than 2 knots, sizes differ, two x are equal, or lower > upper
  BezierCurve(const std::vector<double> & x, const std::vector<double> & y,
              double lower=Parameter::defaultMinimum, double upper=Parameter::defaultMaximum);
  BezierCurve(const BezierCurve & source) = default;   //!< Copy
  BezierCurve & operator=(const BezierCurve & rhs) = default;   //!< Assignment
  virtual ~BezierCurve() { }
  //! @brief Copy. @return new curve (owned by the caller)
  BezierCurve * clone() const override { return new BezierCurve(*this); }

  //! @name Values
  //! @{
  //! @brief Value at x (that of the end knot outside the knots). @param x x @return value
  double evaluateAt(double x) const override;
  using ParameterizedFunction::evaluateAt;
  //! @brief Derivative at x (0 outside the knots). @param x x @return derivative
  double derivativeAt(double x) const override;
  //! @brief The derivative is analytical. @return true
  bool hasAnalyticalDerivative() const override { return true; }
  //! @brief Exact integral from low to high (the end values held outside the knots). @param low lower end
  //! @param high upper end @return integral
  double integralInRange(double low, double high) const override;
  using ParameterizedFunction::integralInRange;
  //! @}

  //! @name Knots
  //! @{
  //! @brief Number of knots. @return number
  unsigned int nKnots() const { return nParameters()/2; }
  //! @brief Abscissa of knot k. @param k index @return x
  double knotX(unsigned int k) const { return parameterValue(2*k); }
  //! @brief Value of knot k. @param k index @return y
  double knotY(unsigned int k) const { return parameterValue(2*k+1); }
  //! @brief Control points of segment k (between knots k and k+1), in y (P0, P1, P2, P3; their x are at the thirds).
  //! @param k segment (0 to nKnots()-2) @return the 4 values
  std::vector<double> controlPoints(unsigned int k) const;
  //! @brief Lower bound of the values. @return bound (Parameter::defaultMinimum if none)
  double lower() const;
  //! @brief Upper bound of the values. @return bound (Parameter::defaultMaximum if none)
  double upper() const;
  //! @brief Bound the values (the knots are clamped to the bounds). @param lower lower bound @param upper upper bound
  //! @throws MathException if lower > upper
  void setBounds(double lower, double upper);
  //! @brief Replace the knots (sorted; values clamped to the bounds). @param x abscissas @param y values
  //! @throws MathException if fewer than 2, sizes differ or two x are equal
  void setKnots(const std::vector<double> & x, const std::vector<double> & y);
  //! @brief Add a knot on the curve at x (the shape barely changes). @param x abscissa (not that of a knot)
  //! @return index of the new knot, or -1 if a knot is there already
  int insertKnot(double x);
  //! @brief Remove a knot (at least 2 stay). @param k index @return false if it cannot be removed
  bool removeKnot(unsigned int k);
  //!
  //! @brief Move a knot: its x kept strictly between those of its neighbors (the order of the knots does not change),
  //! its y clamped to the bounds. @param k index @param x wanted abscissa @param y wanted value
  //!
  void moveKnot(unsigned int k, double x, double y);
  //! @brief Knot nearest to x. @param x abscissa @return index
  unsigned int nearestKnot(double x) const;
  //! @}

  protected:

  //! @brief Rebuild the parameters from knots (sorted, clamped). @param x abscissas @param y values
  void build(std::vector<double> x, std::vector<double> y, double lower, double upper);
  //! @brief Slopes at the knots and the knots in order, recomputed when the parameters changed.
  void update() const;
  //! @brief Segment holding x (x within the knots). @param x abscissa @return index of its first knot
  unsigned int segment(double x) const;

  mutable std::vector<double> _cachedParameters;   //!< parameters when the slopes were computed
  mutable std::vector<double> _x;   //!< knots, in order of x
  mutable std::vector<double> _y;   //!< values of the knots
  mutable std::vector<double> _slopes;   //!< slopes at the knots
  };

  } // namespace MATH
} // namespace CAP

#endif // CAP_MATH_BezierCurve
