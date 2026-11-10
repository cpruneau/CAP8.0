/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file RombergIntegrator.hpp
//! @brief RombergIntegrator: the trapezoidal rule with step halving and Richardson extrapolation.
#ifndef CAP__MATH__RombergIntegrator
#define CAP__MATH__RombergIntegrator
#include "Integrator.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Romberg integration: the trapezoidal estimates T(h), T(h/2), T(h/4), ... are extrapolated to h = 0
  //! (Richardson): R(k, j) = R(k, j-1) + (R(k, j-1) - R(k-1, j-1))/(4^j - 1).
  //!
  //! Level k uses 2^k panels; R(k, k) is exact for polynomials of degree 2k+1 and converges very fast for smooth
  //! integrands. The integration stops when two successive diagonal values agree within the tolerance (from level 3
  //! on), or after maxLevel() levels; the error is estimated as their difference. A box is integrated one variable at
  //! a time.
  //!
  class RombergIntegrator : public Integrator
  {
  public:

  //! @brief Integrator. @param relativeTolerance relative accuracy @param maxLevel maximum number of levels (2^maxLevel panels)
  RombergIntegrator(double relativeTolerance=1.0E-10, unsigned int maxLevel=20);
  //! @brief Copy. @return new integrator (owned by the caller)
  RombergIntegrator * clone() const override { return new RombergIntegrator(*this); }
  //! @brief Name. @return "Romberg"
  String name() const override { return "Romberg"; }
  //! @brief Integral of g over [low, high]. @param g integrand @param low lower bound @param high upper bound
  //! @return result @throws MathException if a bound is not finite
  IntegralResult integrate(const Integrand1D & g, double low, double high) const override;
  using Integrator::integrate;
  };

  } // namespace MATH
} // namespace CAP

#endif // CAP__MATH__RombergIntegrator
