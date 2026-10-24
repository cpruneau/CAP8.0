/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file BesselFunction.cpp
//! @brief Implementation of BesselFunction.
#include <cmath>
#include <limits>
#include "BesselFunction.hpp"
#include "BesselFunctions.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  namespace
  {
  const double kNaN = std::numeric_limits<double>::quiet_NaN();

  //! J_n(x) for any n >= 0: upward recurrence where |x| > n (stable there), Miller's downward recurrence otherwise.
  double besselJ(unsigned int n, double x)
  {
  if (n == 0) return besselJ0(x);
  if (n == 1) return besselJ1(x);
  double ax = std::fabs(x);
  if (ax == 0.0) return 0.0;
  double result;
  if (ax > double(n))
    {
    double previous = besselJ0(ax), current = besselJ1(ax);
    for (unsigned int k=1; k<n; k++)
      {
      double next = (2.0*k/ax)*current - previous;
      previous = current;
      current = next;
      }
    result = current;
    }
  else
    {
    const double big = 1.0E10, small = 1.0E-10;
    unsigned int m = 2*((n + (unsigned int)(std::sqrt(160.0*n)))/2);
    bool even = false;
    double sum = 0.0, above = 0.0, current = 1.0, answer = 0.0;
    for (unsigned int j=m; j>0; j--)
      {
      double below = j*(2.0/ax)*current - above;
      above = current;
      current = below;
      if (std::fabs(current) > big) { current *= small; above *= small; answer *= small; sum *= small; }
      if (even) sum += current;
      even = !even;
      if (j == n) answer = above;
      }
    sum = 2.0*sum - current;
    result = answer/sum;
    }
  return (x < 0.0 && (n & 1)) ? -result : result;
  }

  //! Y_n(x) for any n >= 0 (x > 0): upward recurrence.
  double besselY(unsigned int n, double x)
  {
  if (!(x > 0.0)) return kNaN;
  if (n == 0) return besselY0(x);
  double previous = besselY0(x), current = besselY1(x);
  for (unsigned int k=1; k<n; k++)
    {
    double next = (2.0*k/x)*current - previous;
    previous = current;
    current = next;
    }
  return current;
  }
  } // namespace

  BesselFunction::BesselFunction(Bessel::BesselType type, unsigned int order, double amplitude, double scale, double shift)
  :
  ParameterizedFunction({ Parameter("amplitude", amplitude), Parameter("scale", scale), Parameter("shift", shift),
                          Parameter("order", double(order), Parameter::Fixed) }, 1),
  _type(type)
  {
  if (type != Bessel::BesselJ && type != Bessel::BesselY && type != Bessel::BesselI && type != Bessel::BesselK)
    throw MathException("BesselFunction: only the Bessel functions J, Y, I and K", __FUNCTION__);
  }

  unsigned int BesselFunction::order() const
  {
  double n = parameterValue(3);
  return n > 0 ? (unsigned int)(std::lround(n)) : 0u;
  }

  double BesselFunction::value(Bessel::BesselType type, unsigned int n, double u)
  {
  switch (type)
    {
    case Bessel::BesselJ: return besselJ(n, u);
    case Bessel::BesselY: return besselY(n, u);
    case Bessel::BesselI: return n == 0 ? besselI0(u) : (n == 1 ? besselI1(u) : besselI(int(n), u));
    case Bessel::BesselK: if (!(u > 0.0)) return kNaN; return n == 0 ? besselK0(u) : (n == 1 ? besselK1(u) : besselK(int(n), u));
    default: return kNaN;
    }
  }

  double BesselFunction::evaluateAt(double x) const
  {
  return parameterValue(0)*value(_type, order(), parameterValue(1)*(x - parameterValue(2)));
  }

  } // namespace MATH
} // namespace CAP
