/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Differentiation.hpp
//! @brief Differentiation: derivatives of functions, analytical when possible or by the derivative of an
//! interpolating polynomial (linear, quadratic, cubic or of higher degree), chosen by an enum.
//!
//! @image html NumericalTools.png "Interpolation, integration and differentiation: classes and dependencies" width=100%
//! @image latex NumericalTools.pdf "Interpolation, integration and differentiation: classes and dependencies" width=\textwidth
#ifndef CAP__MATH__Differentiation
#define CAP__MATH__Differentiation
#include <functional>
#include <vector>
#include "String.hpp"
#include "FiniteDifference.hpp"
#include "VectorND.hpp"

namespace CAP
{
  namespace MATH
  {

  class Function;

  //!
  //! @brief Derivatives of functions and of sampled values, by the method chosen by an enum: the tool behind
  //! Derivative and the numerical defaults of Function (derivativeAt(), gradientAt()).
  //!
  //! Methods:
  //!  - Analytical: the derivatives the function provides: Function::derivativeFunction() (the derivative as a new
  //!    function: polynomials, exponentials, orthogonal series, Fourier decompositions, splines), applied as many
  //!    times as the order, and Function::derivativeAt() or gradientAt() for a last first derivative when
  //!    Function::hasAnalyticalDerivative(); an exception if the function provides neither;
  //!  - Linear, Quadratic, Cubic, Polynomial: numerical, the derivative of the interpolating polynomial of degree 1, 2,
  //!    3 or degree() through equally spaced points around x (see FiniteDifference.hpp), Central by default;
  //!  - Automatic (default): analytical as far as the function allows, then numerical (Polynomial) for the rest.
  //!
  //! The step h is chosen to balance the truncation and rounding errors (finiteDifferenceStep(): 1e-3 max(1, |x|) for
  //! the first derivative with degree 4), or set by setStep() (relative: h = step max(1, |x|)). With setRichardson(),
  //! the estimates with h and h/2 are combined to cancel the leading error. Callables (std::function) are
  //! differentiated numerically; Function objects as the method says. Mixed partial derivatives differentiate along
  //! each axis in turn.
  //!
  //! @code
  //!   CAP::MATH::Differentiation d;                                     // Automatic
  //!   CAP::MATH::Polynomial p{1.0, 0.0, 3.0};                           // 1 + 3x^2
  //!   double d1 = d.derivative(p, 2.0);                                 // 12, analytical
  //!   CAP::MATH::Differentiation cubic(CAP::MATH::Differentiation::Cubic);
  //!   double d2 = cubic.derivative([](double x) { return std::sin(x); }, 1.0, 2);   // -sin(1)
  //!   std::vector<double> s = cubic.derivatives(x, y);                  // of sampled values (degree 3)
  //! @endcode
  //!
  class Differentiation
  {
  public:

  //! @brief Methods.
  enum Method
  {
  Automatic,    //!< analytical as far as possible, then numerical (Polynomial)
  Analytical,   //!< analytical only
  Linear,       //!< derivative of the linear interpolation (2 points)
  Quadratic,    //!< of the parabola through 3 points
  Cubic,        //!< of the cubic through 4 points
  Polynomial    //!< of the polynomial of degree() through degree()+1 points
  };

  //! @brief Differentiation. @param method method @param degree degree of the polynomial of the methods Polynomial
  //! and Automatic @param stencil placement of the points
  explicit Differentiation(Method method=Automatic, unsigned int degree=4, Stencil stencil=Stencil::Central);

  //! @name Callables (numerical)
  //! @{
  //! @brief Derivative of order n of g at x. @param g function @param x point @param order n (1 or more) @return derivative
  double derivative(const std::function<double(double)> & g, double x, unsigned int order=1) const;
  //! @brief Partial derivative of order n along an axis. @param g function @param x point @param axis axis
  //! @param order n @return derivative @throws MathException if the axis is out of range
  double partialDerivative(const std::function<double(const std::vector<double> &)> & g, const std::vector<double> & x,
                           unsigned int axis, unsigned int order=1) const;
  //! @brief Mixed partial derivative, one derivative along each axis listed (e.g. {0, 1}: d2g/dx0 dx1).
  //! @param g function @param x point @param axes axes @return derivative
  double mixedDerivative(const std::function<double(const std::vector<double> &)> & g, const std::vector<double> & x,
                         const std::vector<unsigned int> & axes) const;
  //! @brief Gradient. @param g function @param x point @return partial derivatives
  VectorND gradient(const std::function<double(const std::vector<double> &)> & g, const std::vector<double> & x) const;
  //! @}

  //! @name Functions (analytical as the method says)
  //! @{
  //! @brief Derivative of order n of a function of rank 1. @param f function @param x point @param order n @return derivative
  //! @throws MathException if the rank is not 1, or (Analytical) if the function has no analytical derivative
  double derivative(const Function & f, double x, unsigned int order=1) const;
  //! @brief Partial derivative of order n along an axis. @param f function @param x point @param axis axis @param order n
  //! @return derivative @throws MathException as derivative()
  double partialDerivative(const Function & f, const std::vector<double> & x, unsigned int axis, unsigned int order=1) const;
  //! @brief Mixed partial derivative (one derivative along each axis listed). @param f function @param x point
  //! @param axes axes @return derivative
  double mixedDerivative(const Function & f, const std::vector<double> & x, const std::vector<unsigned int> & axes) const;
  //! @brief Gradient. @param f function @param x point @return partial derivatives
  VectorND gradient(const Function & f, const std::vector<double> & x) const;
  //! @}

  //! @brief Derivatives of order n of sampled values, at the samples (local polynomials of degree(); see
  //! derivativesOfSamples()). @param x increasing abscissas @param y values @param order n @return derivatives
  std::vector<double> derivatives(const std::vector<double> & x, const std::vector<double> & y, unsigned int order=1) const;

  //! @name Settings
  //! @{
  //! @brief Method. @return method
  Method method() const { return _method; }
  //! @brief Set the method. @param method method
  void setMethod(Method method) { _method = method; }
  //! @brief Degree of the interpolating polynomial of the numerical derivatives (1, 2, 3 for Linear, Quadratic, Cubic).
  //! @return degree
  unsigned int degree() const;
  //! @brief Set the degree of the methods Polynomial and Automatic. @param degree degree (1 or more)
  void setDegree(unsigned int degree) { _degree = degree > 0 ? degree : 1; }
  //! @brief Placement of the points. @return stencil
  Stencil stencil() const { return _stencil; }
  //! @brief Set the placement of the points. @param stencil stencil
  void setStencil(Stencil stencil) { _stencil = stencil; }
  //! @brief Relative step (0: chosen for the order and degree). @return step
  double step() const { return _step; }
  //! @brief Set the relative step: h = step max(1, |x|) (0: chosen). @param step step
  void setStep(double step) { _step = step; }
  //! @brief Whether the estimates with h and h/2 are extrapolated. @return true if so
  bool richardson() const { return _richardson; }
  //! @brief Extrapolate the estimates with h and h/2 (Richardson). @param on true to do so
  void setRichardson(bool on=true) { _richardson = on; }
  //! @brief Step of a derivative of order n at x. @param order n @param x point @return step
  double stepAt(unsigned int order, double x) const;
  //! @brief Name of a method. @param method method @return name
  static String methodName(Method method);
  //! @}

  protected:

  //! @brief Numerical derivative of order n of g at x along a line (g(t) at x + t). @param g function of the offset
  //! @param x point (for the step) @param order n @return derivative
  double numerical(const std::function<double(double)> & g, double x, unsigned int order) const;

  Method _method;          //!< method
  unsigned int _degree;    //!< degree of Polynomial and Automatic
  Stencil _stencil;        //!< placement of the points
  double _step;            //!< relative step (0: chosen)
  bool _richardson;        //!< extrapolation of h and h/2
  };

  } // namespace MATH
} // namespace CAP

#endif // CAP__MATH__Differentiation
