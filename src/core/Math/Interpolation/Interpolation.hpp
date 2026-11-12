/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Interpolation.hpp
//! @brief Interpolation: the interpolation of a table by the method chosen by an enum, as a function.
//!
//! @image html NumericalTools.png "Interpolation, integration and differentiation: classes and dependencies" width=100%
//! @image latex NumericalTools.pdf "Interpolation, integration and differentiation: classes and dependencies" width=\textwidth
//! @image html InterpolationClasses.png "Classes of the folder Interpolation" width=100%
//! @image latex InterpolationClasses.pdf "Classes of the folder Interpolation" width=\textwidth
#ifndef CAP__MATH__Interpolation
#define CAP__MATH__Interpolation
#include <memory>
#include <vector>
#include "Function.hpp"
#include "PiecewisePolynomial.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Interpolation of a table (x_i, y_i), x increasing at any spacing, by the method chosen by an enum: a
  //! function of one variable, steering the interpolation tools of CAP (InterpolationTools.hpp, CubicSpline).
  //!
  //! Methods:
  //!  - Nearest: the value of the nearest point;
  //!  - Linear: linear interpolation between successive points;
  //!  - LocalPolynomial: the polynomial of degree() through the degree()+1 nearest points (Neville);
  //!  - Polynomial: the polynomial through all the points (for few points: it oscillates between many);
  //!  - Spline (default): cubic spline, not-a-knot; NaturalSpline, ClampedSpline (slopes given), MonotoneSpline: the
  //!    other cubic splines (see CubicSpline).
  //! Outside the table, the end pieces are extended (extrapolation), and Nearest holds the end values.
  //!
  //! Linear and the splines are piecewise polynomials: their derivatives and integrals are exact (derivativeAt(),
  //! derivativeFunction(), integralInRange()); the other methods are differentiated and integrated numerically.
  //!
  //! @code
  //!   CAP::MATH::Interpolation f(x, y);                                          // cubic spline
  //!   CAP::MATH::Interpolation g(x, y, CAP::MATH::Interpolation::Linear);
  //!   CAP::MATH::Interpolation h(x, y, CAP::MATH::Interpolation::LocalPolynomial, 5);
  //!   double v = f(2.5), s = g.integralInRange(0.0, 3.0);
  //!   double w = CAP::MATH::Interpolation::value(x, y, 2.5, CAP::MATH::Interpolation::Linear);
  //! @endcode
  //!
  class Interpolation : public Function
  {
  public:

  //! @brief Methods.
  enum Method
  {
  Nearest,           //!< value of the nearest point
  Linear,            //!< linear between successive points
  LocalPolynomial,   //!< polynomial of degree() through the nearest points
  Polynomial,        //!< polynomial through all the points
  Spline,            //!< cubic spline, not-a-knot
  NaturalSpline,     //!< cubic spline, natural
  ClampedSpline,     //!< cubic spline, slopes given at the ends
  MonotoneSpline     //!< monotone cubic (Fritsch-Carlson)
  };

  //! @brief Interpolation of a table. @param x increasing abscissas (2 or more) @param y values @param method method
  //! @param degree degree of LocalPolynomial @param slopeLow slope at the first point (ClampedSpline)
  //! @param slopeHigh slope at the last point (ClampedSpline)
  //! @throws MathException if the sizes differ, there are fewer than 2 points or x does not increase
  Interpolation(const std::vector<double> & x, const std::vector<double> & y, Method method=Spline,
                unsigned int degree=3, double slopeLow=0.0, double slopeHigh=0.0);
  //! @brief Copy (shares the piecewise polynomial, which is not changed). @param source interpolation
  Interpolation(const Interpolation & source) = default;
  //! @brief Assignment. @param rhs interpolation @return this
  Interpolation & operator=(const Interpolation & rhs) = default;
  //! @brief Destructor.
  ~Interpolation() override { }

  //! @name Values
  //! @{
  //! @brief Interpolated value at x. @param x x @return value
  double evaluateAt(double x) const override;
  //! @brief Interpolated value at a point (rank 1). @param x coordinates (one) @return value
  double evaluateAt(const std::vector<double> & x) const override;
  using Function::evaluateAt;
  //! @brief Interpolated values at several points. @param x points @return values
  std::vector<double> values(const std::vector<double> & x) const;
  //! @brief Derivative (exact for Linear and the splines, numerical otherwise). @param x x @return derivative
  double derivativeAt(double x) const override;
  //! @brief Whether the derivative is exact (Linear, splines). @return true if so
  bool hasAnalyticalDerivative() const override { return bool(_piecewise); }
  //! @brief Derivative as a function (Linear, splines: a piecewise polynomial; null otherwise). @param axis 0
  //! @return new function (owned by the caller), or null
  Function * derivativeFunction(unsigned int axis=0) const override;
  //! @brief Integral (exact for Linear and the splines, numerical otherwise). @param low lower bound
  //! @param high upper bound @return integral
  double integralInRange(double low, double high) const override;
  using Function::integralInRange;
  //! @}

  //! @name Table and method
  //! @{
  //! @brief Abscissas. @return x
  const std::vector<double> & x() const { return _x; }
  //! @brief Values. @return y
  const std::vector<double> & y() const { return _y; }
  //! @brief Method. @return method
  Method method() const { return _method; }
  //! @brief Degree of LocalPolynomial. @return degree
  unsigned int degree() const { return _degree; }
  //! @brief Change the method. @param method method @param degree degree of LocalPolynomial
  void setMethod(Method method, unsigned int degree=3);
  //! @brief Change the table. @param x increasing abscissas @param y values @throws MathException as the constructor
  void setPoints(const std::vector<double> & x, const std::vector<double> & y);
  //! @brief The piecewise polynomial of Linear and the splines (null for the other methods). @return function
  const PiecewisePolynomial * piecewise() const { return _piecewise.get(); }
  //! @brief Name of a method. @param method method @return name
  static String methodName(Method method);
  //! @brief Interpolated value of a table at one point, without making an Interpolation for Nearest, Linear,
  //! LocalPolynomial and Polynomial. @param x abscissas @param y values @param x0 point @param method method
  //! @param degree degree of LocalPolynomial @return value
  static double value(const std::vector<double> & x, const std::vector<double> & y, double x0,
                      Method method=Linear, unsigned int degree=3);
  //! @}

  protected:

  //! @brief Make the piecewise polynomial of the method (Linear and the splines).
  void build();

  std::vector<double> _x;                          //!< abscissas
  std::vector<double> _y;                          //!< values
  Method _method;                                  //!< method
  unsigned int _degree;                            //!< degree of LocalPolynomial
  double _slopeLow;                                //!< slope at the first point (ClampedSpline)
  double _slopeHigh;                               //!< slope at the last point (ClampedSpline)
  std::shared_ptr<const PiecewisePolynomial> _piecewise;   //!< Linear and the splines
  };

  } // namespace MATH
} // namespace CAP

#endif // CAP__MATH__Interpolation
