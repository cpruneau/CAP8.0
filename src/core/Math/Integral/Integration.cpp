/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Integration.cpp
//! @brief Implementation of Integration.
#include "Integration.hpp"
#include "TrapezoidIntegrator.hpp"
#include "SimpsonIntegrator.hpp"
#include "RombergIntegrator.hpp"
#include "GaussLegendreIntegrator.hpp"
#include "MonteCarloIntegrator.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  Integration::Integration(Method method, double relativeTolerance)
  :
  Integrator(),
  _method(method)
  {
  _integrators[0].reset(new TrapezoidIntegrator());
  _integrators[1].reset(new SimpsonIntegrator());
  _integrators[2].reset(new RombergIntegrator());
  _integrators[3].reset(new GaussLegendreIntegrator());
  _integrators[4].reset(new MonteCarloIntegrator());
  if (relativeTolerance > 0.0) setRelativeTolerance(relativeTolerance);
  }

  Integration::Integration(const Integration & source)
  :
  Integrator(source),
  _method(source._method)
  {
  for (int k=0; k<5; k++) _integrators[k].reset(source._integrators[k]->clone());
  }

  Integration & Integration::operator=(const Integration & rhs)
  {
  if (this != &rhs)
    {
    Integrator::operator=(rhs);
    _method = rhs._method;
    for (int k=0; k<5; k++) _integrators[k].reset(rhs._integrators[k]->clone());
    }
  return *this;
  }

  String Integration::methodName(Method method)
  {
  switch (method)
    {
    case Automatic:     return "Automatic";
    case Trapezoid:     return "Trapezoid";
    case Simpson:       return "Simpson";
    case Romberg:       return "Romberg";
    case GaussLegendre: return "GaussLegendre";
    case MonteCarlo:    return "MonteCarlo";
    }
  return "Unknown";
  }

  String Integration::name() const
  {
  return methodName(_method);
  }

  Integration::Method Integration::methodFor(std::size_t rank) const
  {
  if (_method != Automatic) return _method;
  return rank <= 3 ? Simpson : MonteCarlo;
  }

  const Integrator & Integration::integratorOf(Method method) const
  {
  if (method == Automatic) throw MathException("No integrator for the method Automatic", __FUNCTION__);
  return *_integrators[int(method) - 1];
  }

  Integrator & Integration::integrator(Method method)
  {
  if (method == Automatic) throw MathException("No integrator for the method Automatic", __FUNCTION__);
  return *_integrators[int(method) - 1];
  }

  void Integration::setIntegrator(Method method, Integrator * integrator)
  {
  if (method == Automatic) throw MathException("No integrator for the method Automatic", __FUNCTION__);
  if (!integrator) throw MathException("Null integrator", __FUNCTION__);
  _integrators[int(method) - 1].reset(integrator);
  }

  void Integration::setRelativeTolerance(double tolerance)
  {
  Integrator::setRelativeTolerance(tolerance);
  for (auto & integrator : _integrators) integrator->setRelativeTolerance(tolerance);
  }

  void Integration::setAbsoluteTolerance(double tolerance)
  {
  Integrator::setAbsoluteTolerance(tolerance);
  for (auto & integrator : _integrators) integrator->setAbsoluteTolerance(tolerance);
  }

  void Integration::setMaxLevel(unsigned int levels)
  {
  Integrator::setMaxLevel(levels);
  for (int k=0; k<4; k++) _integrators[k]->setMaxLevel(levels);
  }

  IntegralResult Integration::integrate(const Integrand1D & g, double low, double high) const
  {
  return integratorOf(methodFor(1)).integrate(g, low, high);
  }

  IntegralResult Integration::integrate(const IntegrandND & g, const std::vector<double> & low,
                                        const std::vector<double> & high) const
  {
  return integratorOf(methodFor(low.size())).integrate(g, low, high);
  }

  } // namespace MATH
} // namespace CAP
