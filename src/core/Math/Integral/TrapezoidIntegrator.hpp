/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TrapezoidIntegrator.hpp
//! @brief TrapezoidIntegrator: the trapezoidal rule in 1, 2 or n dimensions, with step halving or fixed panels.
#ifndef CAP__MATH__TrapezoidIntegrator
#define CAP__MATH__TrapezoidIntegrator
#include "Integrator.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Integration by the trapezoidal rule: the integrand is replaced by its linear interpolation between
  //! equally spaced points (error of order h^2; exact for linear functions).
  //!
  //! One variable: the number of panels is doubled (1, 2, 4, ...; the new points only are evaluated) until two
  //! successive estimates T(h) and T(h/2) agree within the tolerance, or maxLevel() halvings; the error is estimated
  //! as |T(h/2) - T(h)|/3. With setPanels(n), the rule is applied once on n panels.
  //!
  //! A box of n variables: the product rule on a grid of N^n points (the corners of N^n cells weighted by 1/2 per
  //! boundary they lie on), N doubled from 2 until convergence; the number of evaluations grows as N^n, so the
  //! maximum number of evaluations (setMaxEvaluations()) stops the refinement. Smooth functions of more than 3 or 4
  //! variables are better integrated by Monte Carlo (MonteCarloIntegrator).
  //!
  class TrapezoidIntegrator : public Integrator
  {
  public:

  //! @brief Integrator. @param relativeTolerance relative accuracy @param maxLevel maximum number of halvings
  TrapezoidIntegrator(double relativeTolerance=1.0E-6, unsigned int maxLevel=20);
  //! @brief Copy. @return new integrator (owned by the caller)
  TrapezoidIntegrator * clone() const override { return new TrapezoidIntegrator(*this); }
  //! @brief Name. @return "Trapezoid"
  String name() const override { return "Trapezoid"; }
  //! @brief Integral of g over [low, high]. @param g integrand @param low lower bound @param high upper bound
  //! @return result @throws MathException if a bound is not finite
  IntegralResult integrate(const Integrand1D & g, double low, double high) const override;
  //! @brief Integral of g over a box, by the product rule. @param g integrand @param low lower bounds
  //! @param high upper bounds @return result @throws MathException if the sizes differ or a bound is not finite
  IntegralResult integrate(const IntegrandND & g, const std::vector<double> & low,
                           const std::vector<double> & high) const override;
  using Integrator::integrate;

  //! @brief Use a fixed number of panels per variable (0: refinement to the tolerance). @param n panels
  void setPanels(unsigned int n) { _panels = n; }
  //! @brief Fixed number of panels per variable (0: refinement). @return panels
  unsigned int panels() const { return _panels; }
  //! @brief Set the maximum number of evaluations of the integrand of a box. @param n evaluations
  void setMaxEvaluations(long n) { _maxEvaluations = n; }
  //! @brief Maximum number of evaluations of the integrand of a box. @return evaluations
  long maxEvaluations() const { return _maxEvaluations; }

  //! @brief Trapezoidal rule on a grid of n panels per variable over a box. @param g integrand @param low lower bounds
  //! @param high upper bounds @param n panels per variable @param nEvaluations incremented by the evaluations
  //! @return estimate
  static double productRule(const IntegrandND & g, const std::vector<double> & low, const std::vector<double> & high,
                            unsigned int n, long & nEvaluations);

  protected:

  unsigned int _panels;    //!< fixed number of panels (0: refinement)
  long _maxEvaluations;    //!< maximum number of evaluations of the integrand of a box
  };

  } // namespace MATH
} // namespace CAP

#endif // CAP__MATH__TrapezoidIntegrator
