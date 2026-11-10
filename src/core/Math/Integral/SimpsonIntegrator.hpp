/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file SimpsonIntegrator.hpp
//! @brief SimpsonIntegrator: Simpson's rule, adaptive (default) or on a fixed number of panels.
#ifndef CAP__MATH__SimpsonIntegrator
#define CAP__MATH__SimpsonIntegrator
#include "Integrator.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Integration by Simpson's rule, exact for polynomials of degree 3 or less.
  //!
  //! Adaptive (default): the range is cut into initialPanels() panels (8 by default, so that narrow features are not
  //! missed by the first estimate); each panel is halved recursively until the two halves and the whole agree within
  //! the tolerance, the difference correcting the result (Richardson: error of order h^6). The tolerance is relative to
  //! the integral of |g| (estimated on the first panels), so that integrals close to zero (an odd function over a
  //! symmetric range) converge quickly; maxLevel() is the maximum depth of the subdivision.
  //!
  //! Fixed: Simpson's rule on setPanels(n) panels (2n intervals of width (high - low)/(2n), error of order h^4); the
  //! error is estimated from the rule on n/2 panels (when n is even).
  //!
  //! A box is integrated one variable at a time. This is the integrator of the numerical defaults of Function
  //! (integralInRange(), expectationValue()).
  //!
  class SimpsonIntegrator : public Integrator
  {
  public:

  //! @brief Adaptive integrator. @param relativeTolerance relative accuracy @param maxLevel maximum depth of the subdivision
  //! @param initialPanels panels of the first estimate
  SimpsonIntegrator(double relativeTolerance=1.0E-8, unsigned int maxLevel=20, unsigned int initialPanels=8);
  //! @brief Copy. @return new integrator (owned by the caller)
  SimpsonIntegrator * clone() const override { return new SimpsonIntegrator(*this); }
  //! @brief Name. @return "Simpson"
  String name() const override { return "Simpson"; }
  //! @brief Integral of g over [low, high]. @param g integrand @param low lower bound @param high upper bound
  //! @return result @throws MathException if a bound is not finite
  IntegralResult integrate(const Integrand1D & g, double low, double high) const override;
  using Integrator::integrate;

  //! @brief Use a fixed number of panels (0: adaptive). @param n panels (each of two intervals)
  void setPanels(unsigned int n) { _panels = n; }
  //! @brief Fixed number of panels (0: adaptive). @return panels
  unsigned int panels() const { return _panels; }
  //! @brief Set the number of panels of the first estimate of the adaptive integration. @param n panels (1 or more)
  void setInitialPanels(unsigned int n) { _initialPanels = n > 0 ? n : 1; }
  //! @brief Panels of the first estimate of the adaptive integration. @return panels
  unsigned int initialPanels() const { return _initialPanels; }

  protected:

  unsigned int _panels;          //!< fixed number of panels (0: adaptive)
  unsigned int _initialPanels;   //!< panels of the first estimate (adaptive)
  };

  } // namespace MATH
} // namespace CAP

#endif // CAP__MATH__SimpsonIntegrator
