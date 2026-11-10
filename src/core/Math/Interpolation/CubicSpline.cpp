/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file CubicSpline.cpp
//! @brief Implementation of CubicSpline.
#include <cmath>
#include "CubicSpline.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  CubicSpline::CubicSpline()
  :
  PiecewisePolynomial(),
  _kind(NotAKnot)
  {
  setName("CubicSpline");
  setTitle("CubicSpline");
  }

  CubicSpline::CubicSpline(const std::vector<double> & x, const std::vector<double> & y, Kind kind,
                           double slopeLow, double slopeHigh)
  :
  PiecewisePolynomial(),
  _kind(kind)
  {
  setName("CubicSpline");
  setTitle("CubicSpline");
  setPoints(x, y, kind, slopeLow, slopeHigh);
  }

  String CubicSpline::kindName(Kind kind)
  {
  switch (kind)
    {
    case Natural:  return "Natural";
    case Clamped:  return "Clamped";
    case NotAKnot: return "NotAKnot";
    case Monotone: return "Monotone";
    }
  return "Unknown";
  }

  namespace
  {
  // Thomas algorithm: a[i] s[i-1] + b[i] s[i] + c[i] s[i+1] = d[i]
  std::vector<double> tridiagonal(std::vector<double> a, std::vector<double> b, std::vector<double> c, std::vector<double> d)
  {
  std::size_t n = b.size();
  for (std::size_t i=1; i<n; i++)
    {
    double m = a[i]/b[i-1];
    b[i] -= m*c[i-1];
    d[i] -= m*d[i-1];
    }
  std::vector<double> s(n);
  s[n-1] = d[n-1]/b[n-1];
  for (std::size_t i=n-1; i-->0; ) s[i] = (d[i] - c[i]*s[i+1])/b[i];
  return s;
  }

  // end slope of pchip (three-point formula, shape preserving)
  double pchipEnd(double h0, double h1, double d0, double d1)
  {
  double s = ((2.0*h0 + h1)*d0 - h0*d1)/(h0 + h1);
  if (s*d0 <= 0.0) return 0.0;
  if (d0*d1 < 0.0 && std::fabs(s) > 3.0*std::fabs(d0)) return 3.0*d0;
  return s;
  }
  } // namespace

  std::vector<double> CubicSpline::slopes(const std::vector<double> & x, const std::vector<double> & y, Kind kind,
                                          double slopeLow, double slopeHigh)
  {
  const std::size_t n = x.size();
  std::vector<double> h(n - 1), delta(n - 1);
  for (std::size_t i=0; i+1<n; i++) { h[i] = x[i+1] - x[i]; delta[i] = (y[i+1] - y[i])/h[i]; }
  std::vector<double> s(n);
  if (n == 2)
    {
    if (kind == Clamped) { s[0] = slopeLow; s[1] = slopeHigh; }
    else s[0] = s[1] = delta[0];
    return s;
    }
  if (kind == Monotone)
    {
    for (std::size_t i=1; i+1<n; i++)
      {
      if (delta[i-1]*delta[i] <= 0.0) { s[i] = 0.0; continue; }
      double w1 = 2.0*h[i] + h[i-1], w2 = h[i] + 2.0*h[i-1];      // weighted harmonic mean
      s[i] = (w1 + w2)/(w1/delta[i-1] + w2/delta[i]);
      }
    s[0]   = pchipEnd(h[0], h[1], delta[0], delta[1]);
    s[n-1] = pchipEnd(h[n-2], h[n-3], delta[n-2], delta[n-3]);
    return s;
    }
  if (kind == NotAKnot && n == 3)
    {
    // the parabola through the three points
    double c = (delta[1] - delta[0])/(h[0] + h[1]);
    s[0] = delta[0] - c*h[0];
    s[1] = delta[0] + c*h[0];
    s[2] = delta[1] + c*h[1];
    return s;
    }
  // continuity of the second derivative at the interior points, and the end conditions
  std::vector<double> a(n, 0.0), b(n, 0.0), c(n, 0.0), d(n, 0.0);
  for (std::size_t i=1; i+1<n; i++)
    {
    a[i] = h[i];
    b[i] = 2.0*(h[i-1] + h[i]);
    c[i] = h[i-1];
    d[i] = 3.0*(h[i]*delta[i-1] + h[i-1]*delta[i]);
    }
  switch (kind)
    {
    case Natural:
      b[0] = 2.0; c[0] = 1.0; d[0] = 3.0*delta[0];
      a[n-1] = 1.0; b[n-1] = 2.0; d[n-1] = 3.0*delta[n-2];
      break;
    case Clamped:
      b[0] = 1.0; c[0] = 0.0; d[0] = slopeLow;
      a[n-1] = 0.0; b[n-1] = 1.0; d[n-1] = slopeHigh;
      break;
    default:   // NotAKnot, 4 points or more
      {
      double hs = h[0] + h[1];
      b[0] = h[1]; c[0] = hs;
      d[0] = ((h[0] + 2.0*hs)*h[1]*delta[0] + h[0]*h[0]*delta[1])/hs;
      double he = h[n-2] + h[n-3];
      a[n-1] = he; b[n-1] = h[n-3];
      d[n-1] = (h[n-2]*h[n-2]*delta[n-3] + (2.0*he + h[n-2])*h[n-3]*delta[n-2])/he;
      }
      break;
    }
  return tridiagonal(a, b, c, d);
  }

  void CubicSpline::setPoints(const std::vector<double> & x, const std::vector<double> & y, Kind kind,
                              double slopeLow, double slopeHigh)
  {
  if (x.size() != y.size()) throw MathException("The sizes of x and y differ", __FUNCTION__);
  if (x.size() < 2) throw MathException("Fewer than 2 points", __FUNCTION__);
  for (std::size_t i=0; i+1<x.size(); i++)
    if (!(x[i+1] > x[i])) throw MathException("The abscissas do not increase", __FUNCTION__);
  _kind = kind;
  std::vector<double> s = slopes(x, y, kind, slopeLow, slopeHigh);
  // Hermite form of each piece, as local coefficients
  std::vector<std::vector<double>> coefficients(x.size() - 1);
  for (std::size_t i=0; i+1<x.size(); i++)
    {
    double h = x[i+1] - x[i], delta = (y[i+1] - y[i])/h;
    coefficients[i] = { y[i], s[i], (3.0*delta - 2.0*s[i] - s[i+1])/h, (s[i] + s[i+1] - 2.0*delta)/(h*h) };
    }
  setPieces(x, coefficients);
  }

  } // namespace MATH
} // namespace CAP
