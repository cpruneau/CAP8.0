/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Normal2D.cpp
//! @brief Implementation of Normal2D.
#include <cmath>
#include "Normal2D.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  Normal2D::Normal2D(double amplitude, double meanX, double meanY, double widthX, double widthY, double correlation)
  :
  ParameterizedFunction({ Parameter("amplitude", amplitude), Parameter("meanX", meanX), Parameter("meanY", meanY),
                          Parameter("widthX", widthX), Parameter("widthY", widthY),
                          Parameter("correlation", correlation, Parameter::LowHigh, -0.999, 0.999) }, 2)
  {
  if (widthX == 0.0 || widthY == 0.0) throw MathException("The widths of a Normal2D must not be 0", __FUNCTION__);
  if (!(std::fabs(correlation) < 1.0)) throw MathException("The correlation of a Normal2D must be in (-1, 1)", __FUNCTION__);
  }

  double Normal2D::evaluateAt(double x, double y) const
  {
  double sx = parameterValue(3), sy = parameterValue(4), rho = parameterValue(5);
  double u = (x - parameterValue(1))/sx, v = (y - parameterValue(2))/sy;
  return parameterValue(0)*std::exp(-(u*u - 2.0*rho*u*v + v*v)/(2.0*(1.0 - rho*rho)));
  }

  double Normal2D::volume() const
  {
  double rho = parameterValue(5);
  return 2.0*M_PI*parameterValue(0)*std::fabs(parameterValue(3)*parameterValue(4))*std::sqrt(1.0 - rho*rho);
  }

  double Normal2D::integralInRange(double x1Low, double x1High, double x2Low, double x2High) const
  {
  if (parameterValue(5) != 0.0) return ParameterizedFunction::integralInRange(x1Low, x1High, x2Low, x2High);
  double sx = std::fabs(parameterValue(3))*std::sqrt(2.0), sy = std::fabs(parameterValue(4))*std::sqrt(2.0);
  double mx = parameterValue(1), my = parameterValue(2);
  double fx = 0.5*(std::erf((x1High - mx)/sx) - std::erf((x1Low - mx)/sx));
  double fy = 0.5*(std::erf((x2High - my)/sy) - std::erf((x2Low - my)/sy));
  return volume()*fx*fy;
  }

  } // namespace MATH
} // namespace CAP
