/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Integrator.hpp
//! @brief Integrator: base class of the numerical integrators of functions (trapezoid, Simpson, Romberg,
//! Gauss-Legendre, Monte Carlo) and of Integration, which chooses one of them.
//!
//! @image html NumericalTools.png "Interpolation, integration and differentiation: classes and dependencies" width=100%
//! @image latex NumericalTools.pdf "Interpolation, integration and differentiation: classes and dependencies" width=\textwidth
//! @image html IntegralClasses.png "Classes of the folder Integral" width=100%
//! @image latex IntegralClasses.pdf "Classes of the folder Integral" width=\textwidth
#ifndef CAP__MATH__Integrator
#define CAP__MATH__Integrator
#include <functional>
#include <vector>
#include "String.hpp"

namespace CAP
{
  namespace MATH
  {

  class Function;

  //! @brief Integrand of one variable.
  using Integrand1D = std::function<double(double)>;
  //! @brief Integrand of n variables (a point of n coordinates).
  using IntegrandND = std::function<double(const std::vector<double> &)>;

  //!
  //! @brief Result of a numerical integration: the value, an estimate of its error, the number of evaluations of the
  //! integrand, and whether the requested accuracy was reached.
  //!
  struct IntegralResult
  {
  double value = 0.0;        //!< integral
  double error = 0.0;        //!< estimate of the absolute error (a statistical error for Monte Carlo)
  long nEvaluations = 0;     //!< evaluations of the integrand
  bool converged = true;     //!< false if the maximum level (or number of samples) was reached first
  };

  //!
  //! @brief Base class of the numerical integrators: the integral of a function of one variable over [low, high], and
  //! of a function of n variables over a box [low[0], high[0]] x ... x [low[n-1], high[n-1]].
  //!
  //! An integrator integrates a Function (of rank 1, 2 or any n) or a callable (Integrand1D, IntegrandND); the result
  //! (IntegralResult) holds the value, an error estimate and the number of evaluations, and integral() returns the
  //! value only. Reversed bounds change the sign; the bounds must be finite. The accuracy requested is
  //! |error| <= max(absoluteTolerance, relativeTolerance |value|), within maxLevel refinements (the meaning of a level
  //! depends on the method: halvings of the step, depth of the adaptive subdivision, doublings of the samples).
  //!
  //! Derived classes implement integrate() for one variable. The box is integrated one variable at a time by default
  //! (the integral over the remaining variables is the integrand of the first); TrapezoidIntegrator,
  //! GaussLegendreIntegrator and MonteCarloIntegrator integrate the box directly.
  //!
  //! @code
  //!   CAP::MATH::SimpsonIntegrator simpson;                          // adaptive
  //!   CAP::MATH::Normal g(1.0, 0.0, 1.0);
  //!   double s = simpson.integral(g, -1.0, 1.0);                    // 0.6827 (times sqrt(2 pi))
  //!   CAP::MATH::IntegralResult r = simpson.integrate([](double x) { return x*x; }, 0.0, 3.0);   // 9
  //!   CAP::MATH::MonteCarloIntegrator mc(1.0E-3);
  //!   double v = mc.integral([](const std::vector<double> & x) { return x[0]*x[1]*x[2]*x[3]; },
  //!                          {0, 0, 0, 0}, {1, 1, 1, 1});            // 1/16, to 0.1%
  //! @endcode
  //!
  class Integrator
  {
  public:

  //! @brief Integrator. @param relativeTolerance relative accuracy requested @param absoluteTolerance absolute accuracy
  //! requested @param maxLevel maximum number of refinements
  Integrator(double relativeTolerance=1.0E-8, double absoluteTolerance=0.0, unsigned int maxLevel=20);
  //! @brief Destructor.
  virtual ~Integrator() { }
  //! @brief Copy of the integrator, of its own class. @return new integrator (owned by the caller)
  virtual Integrator * clone() const = 0;
  //! @brief Name of the method (e.g. "Simpson"). @return name
  virtual String name() const = 0;

  //! @name Integration
  //! @{
  //! @brief Integral of g over [low, high]. @param g integrand @param low lower bound @param high upper bound
  //! @return result @throws MathException if a bound is not finite
  virtual IntegralResult integrate(const Integrand1D & g, double low, double high) const = 0;
  //! @brief Integral of g over a box (by default one variable at a time). @param g integrand @param low lower bounds
  //! @param high upper bounds @return result @throws MathException if the sizes differ or a bound is not finite
  virtual IntegralResult integrate(const IntegrandND & g, const std::vector<double> & low,
                                   const std::vector<double> & high) const;
  //! @brief Integral of a function of rank 1. @param f function @param low lower bound @param high upper bound
  //! @return result @throws MathException if the rank is not 1
  IntegralResult integrate(const Function & f, double low, double high) const;
  //! @brief Integral of a function of rank 2 over a rectangle. @param f function @param x1Low lower bound of x1
  //! @param x1High upper bound of x1 @param x2Low lower bound of x2 @param x2High upper bound of x2 @return result
  //! @throws MathException if the rank is not 2
  IntegralResult integrate(const Function & f, double x1Low, double x1High, double x2Low, double x2High) const;
  //! @brief Integral of a function of any rank over a box. @param f function @param low lower bounds
  //! @param high upper bounds @return result @throws MathException if the sizes differ from the rank
  IntegralResult integrate(const Function & f, const std::vector<double> & low, const std::vector<double> & high) const;
  //! @brief Value of the integral of g over [low, high]. @param g integrand @param low lower bound @param high upper bound @return integral
  double integral(const Integrand1D & g, double low, double high) const { return integrate(g, low, high).value; }
  //! @brief Value of the integral of g over a box. @param g integrand @param low lower bounds @param high upper bounds @return integral
  double integral(const IntegrandND & g, const std::vector<double> & low, const std::vector<double> & high) const
  { return integrate(g, low, high).value; }
  //! @brief Value of the integral of a function of rank 1. @param f function @param low lower bound @param high upper bound @return integral
  double integral(const Function & f, double low, double high) const { return integrate(f, low, high).value; }
  //! @brief Value of the integral of a function of rank 2. @param f function @param x1Low lower bound of x1
  //! @param x1High upper bound of x1 @param x2Low lower bound of x2 @param x2High upper bound of x2 @return integral
  double integral(const Function & f, double x1Low, double x1High, double x2Low, double x2High) const
  { return integrate(f, x1Low, x1High, x2Low, x2High).value; }
  //! @brief Value of the integral of a function over a box. @param f function @param low lower bounds @param high upper bounds @return integral
  double integral(const Function & f, const std::vector<double> & low, const std::vector<double> & high) const
  { return integrate(f, low, high).value; }
  //! @}

  //! @name Accuracy
  //! @{
  //! @brief Relative accuracy requested. @return tolerance
  double relativeTolerance() const { return _relativeTolerance; }
  //! @brief Absolute accuracy requested. @return tolerance
  double absoluteTolerance() const { return _absoluteTolerance; }
  //! @brief Maximum number of refinements. @return levels
  unsigned int maxLevel() const { return _maxLevel; }
  //! @brief Set the relative accuracy requested. @param tolerance tolerance (>= 0)
  virtual void setRelativeTolerance(double tolerance) { _relativeTolerance = tolerance; }
  //! @brief Set the absolute accuracy requested. @param tolerance tolerance (>= 0)
  virtual void setAbsoluteTolerance(double tolerance) { _absoluteTolerance = tolerance; }
  //! @brief Set the maximum number of refinements. @param levels levels
  virtual void setMaxLevel(unsigned int levels) { _maxLevel = levels; }
  //! @}

  protected:

  //! @brief Throw if a bound is not finite. @param low lower bound @param high upper bound @param source caller
  static void checkBounds(double low, double high, const char * source);
  //! @brief Throw if the sizes of the bounds differ or are 0, or a bound is not finite. @param low lower bounds
  //! @param high upper bounds @param source caller
  static void checkBounds(const std::vector<double> & low, const std::vector<double> & high, const char * source);
  //! @brief Whether a change (an error estimate) is within the accuracy requested for a value. @param change change
  //! @param value value @return true if so
  bool accepted(double change, double value) const;

  double _relativeTolerance;   //!< relative accuracy requested
  double _absoluteTolerance;   //!< absolute accuracy requested
  unsigned int _maxLevel;      //!< maximum number of refinements
  };

  } // namespace MATH
} // namespace CAP

#endif // CAP__MATH__Integrator
