/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Integration.hpp
//! @brief Integration: an integrator that integrates with the method chosen by an enum (or automatically).
#ifndef CAP__MATH__Integration
#define CAP__MATH__Integration
#include <memory>
#include "Integrator.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Integrator steering the integrators of CAP: it integrates with the method chosen by an enum, or chooses
  //! the method from the number of variables (Automatic).
  //!
  //! Automatic: adaptive Simpson for one variable, Simpson one variable at a time for 2 or 3, Monte Carlo beyond.
  //! The accuracy (setRelativeTolerance(), setAbsoluteTolerance(), setMaxLevel()) is passed to the integrators; the
  //! settings of a method of its own (e.g. the points of Gauss-Legendre, the seed of Monte Carlo) are those of
  //! integrator(method), which may also be replaced (setIntegrator()). Being an Integrator, an Integration is used
  //! wherever an integrator is.
  //!
  //! @code
  //!   CAP::MATH::Integration integration(CAP::MATH::Integration::Romberg);
  //!   double s = integration.integral(f, 0.0, 1.0);
  //!   integration.setMethod(CAP::MATH::Integration::MonteCarlo);
  //!   double v = integration.integral(g, low, high);                  // g of 6 variables
  //! @endcode
  //!
  class Integration : public Integrator
  {
  public:

  //! @brief Methods.
  enum Method
  {
  Automatic,       //!< Simpson for 1 to 3 variables, Monte Carlo beyond
  Trapezoid,       //!< TrapezoidIntegrator
  Simpson,         //!< SimpsonIntegrator (adaptive)
  Romberg,         //!< RombergIntegrator
  GaussLegendre,   //!< GaussLegendreIntegrator
  MonteCarlo       //!< MonteCarloIntegrator
  };

  //! @brief Integration. @param method method @param relativeTolerance relative accuracy (0: that of the method)
  explicit Integration(Method method=Automatic, double relativeTolerance=0.0);
  //! @brief Copy (the integrators are copied). @param source integration
  Integration(const Integration & source);
  //! @brief Assignment. @param rhs integration @return this
  Integration & operator=(const Integration & rhs);
  //! @brief Destructor.
  ~Integration() override { }
  //! @brief Copy. @return new integration (owned by the caller)
  Integration * clone() const override { return new Integration(*this); }
  //! @brief Name of the method (Automatic: "Automatic"). @return name
  String name() const override;
  //! @brief Integral of g over [low, high]. @param g integrand @param low lower bound @param high upper bound @return result
  IntegralResult integrate(const Integrand1D & g, double low, double high) const override;
  //! @brief Integral of g over a box. @param g integrand @param low lower bounds @param high upper bounds @return result
  IntegralResult integrate(const IntegrandND & g, const std::vector<double> & low,
                           const std::vector<double> & high) const override;
  using Integrator::integrate;

  //! @brief Method. @return method
  Method method() const { return _method; }
  //! @brief Set the method. @param method method
  void setMethod(Method method) { _method = method; }
  //! @brief Name of a method. @param method method @return name
  static String methodName(Method method);
  //! @brief Method used for a number of variables (Automatic resolved). @param rank number of variables @return method
  Method methodFor(std::size_t rank) const;
  //! @brief Integrator of a method (to change its own settings). @param method method (not Automatic) @return integrator
  //! @throws MathException if the method is Automatic
  Integrator & integrator(Method method);
  //! @brief Replace the integrator of a method. @param method method (not Automatic) @param integrator integrator
  //! (owned by the Integration) @throws MathException if the method is Automatic or the integrator is null
  void setIntegrator(Method method, Integrator * integrator);
  //! @brief Set the relative accuracy (of all the integrators). @param tolerance tolerance
  void setRelativeTolerance(double tolerance) override;
  //! @brief Set the absolute accuracy (of all the integrators). @param tolerance tolerance
  void setAbsoluteTolerance(double tolerance) override;
  //! @brief Set the maximum number of refinements (of all the integrators but Monte Carlo). @param levels levels
  void setMaxLevel(unsigned int levels) override;

  protected:

  //! @brief Integrator of a method (const). @param method method @return integrator
  const Integrator & integratorOf(Method method) const;

  Method _method;                                    //!< method
  std::unique_ptr<Integrator> _integrators[5];       //!< integrators of Trapezoid ... MonteCarlo
  };

  } // namespace MATH
} // namespace CAP

#endif // CAP__MATH__Integration
