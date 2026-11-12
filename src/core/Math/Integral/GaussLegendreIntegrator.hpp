/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file GaussLegendreIntegrator.hpp
//! @brief GaussLegendreIntegrator: Gauss-Legendre quadrature, composite, in 1, 2 or n dimensions.
#ifndef CAP__MATH__GaussLegendreIntegrator
#define CAP__MATH__GaussLegendreIntegrator
#include "Integrator.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Gauss-Legendre quadrature: m points per panel at the roots of the Legendre polynomial P_m, exact for
  //! polynomials of degree 2m-1 or less.
  //!
  //! The range is cut into panels (1 by default); the number of panels is doubled until two successive estimates
  //! agree within the tolerance (maxLevel() doublings), the error being their difference. With setPanels(n), the rule is
  //! applied once on n panels. A box is integrated by the product rule (m^n points per cell).
  //!
  //! rule() gives the nodes and weights on [-1, 1] (computed once per m, by Newton's method), and quadrature() and
  //! productRule() apply the rule without refinement: the functions of CAP with polynomial pieces (Legendre, Laguerre,
  //! BezierCurve) and the averages of a function over the bins of a histogram (Fitter) use them.
  //!
  class GaussLegendreIntegrator : public Integrator
  {
  public:

  //! @brief Integrator. @param nPoints points per panel (1 or more) @param relativeTolerance relative accuracy
  //! @param maxLevel maximum number of doublings of the panels
  GaussLegendreIntegrator(unsigned int nPoints=5, double relativeTolerance=1.0E-10, unsigned int maxLevel=16);
  //! @brief Copy. @return new integrator (owned by the caller)
  GaussLegendreIntegrator * clone() const override { return new GaussLegendreIntegrator(*this); }
  //! @brief Name. @return "GaussLegendre"
  String name() const override { return "GaussLegendre"; }
  //! @brief Integral of g over [low, high]. @param g integrand @param low lower bound @param high upper bound
  //! @return result @throws MathException if a bound is not finite
  IntegralResult integrate(const Integrand1D & g, double low, double high) const override;
  //! @brief Integral of g over a box, by the product rule. @param g integrand @param low lower bounds
  //! @param high upper bounds @return result @throws MathException if the sizes differ or a bound is not finite
  IntegralResult integrate(const IntegrandND & g, const std::vector<double> & low,
                           const std::vector<double> & high) const override;
  using Integrator::integrate;

  //! @brief Points per panel. @return m
  unsigned int nPoints() const { return _nPoints; }
  //! @brief Set the points per panel. @param m points (1 or more)
  void setPoints(unsigned int m) { _nPoints = m > 0 ? m : 1; }
  //! @brief Use a fixed number of panels per variable (0: doubling to the tolerance). @param n panels
  void setPanels(unsigned int n) { _panels = n; }
  //! @brief Fixed number of panels (0: doubling). @return panels
  unsigned int panels() const { return _panels; }

  //! @brief Nodes and weights of the m-point rule on [-1, 1] (computed once, kept). @param m points (1 or more)
  //! @param nodes nodes (increasing) @param weights weights @throws MathException if m is 0
  static void rule(unsigned int m, std::vector<double> & nodes, std::vector<double> & weights);
  //! @brief The m-point rule on n panels of [low, high]. @param g integrand @param low lower bound @param high upper bound
  //! @param m points per panel @param n panels @return estimate
  static double quadrature(const Integrand1D & g, double low, double high, unsigned int m, unsigned int n=1);
  //! @brief The product of m-point rules on n panels per variable over a box. @param g integrand @param low lower bounds
  //! @param high upper bounds @param m points per panel @param n panels per variable @return estimate
  static double productRule(const IntegrandND & g, const std::vector<double> & low, const std::vector<double> & high,
                            unsigned int m, unsigned int n=1);

  protected:

  unsigned int _nPoints;   //!< points per panel
  unsigned int _panels;    //!< fixed number of panels (0: doubling)
  };

  } // namespace MATH
} // namespace CAP

#endif // CAP__MATH__GaussLegendreIntegrator
