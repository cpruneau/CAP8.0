/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Interpolation.cpp
//! @brief Implementation of Interpolation.
#include <cmath>
#include "Interpolation.hpp"
#include "InterpolationTools.hpp"
#include "CubicSpline.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  Interpolation::Interpolation(const std::vector<double> & x, const std::vector<double> & y, Method method,
                               unsigned int degree, double slopeLow, double slopeHigh)
  :
  Function("Interpolation", "Interpolation", "CAP::MATH::Interpolation", 1),
  _x(),
  _y(),
  _method(method),
  _degree(degree > 0 ? degree : 1),
  _slopeLow(slopeLow),
  _slopeHigh(slopeHigh)
  {
  setPoints(x, y);
  }

  String Interpolation::methodName(Method method)
  {
  switch (method)
    {
    case Nearest:         return "Nearest";
    case Linear:          return "Linear";
    case LocalPolynomial: return "LocalPolynomial";
    case Polynomial:      return "Polynomial";
    case Spline:          return "Spline";
    case NaturalSpline:   return "NaturalSpline";
    case ClampedSpline:   return "ClampedSpline";
    case MonotoneSpline:  return "MonotoneSpline";
    }
  return "Unknown";
  }

  void Interpolation::setPoints(const std::vector<double> & x, const std::vector<double> & y)
  {
  if (x.size() != y.size()) throw MathException("The sizes of x and y differ", __FUNCTION__);
  if (x.size() < 2) throw MathException("Fewer than 2 points", __FUNCTION__);
  for (std::size_t i=0; i+1<x.size(); i++)
    if (!(x[i+1] > x[i])) throw MathException("The abscissas do not increase", __FUNCTION__);
  _x = x;
  _y = y;
  build();
  }

  void Interpolation::setMethod(Method method, unsigned int degree)
  {
  _method = method;
  _degree = degree > 0 ? degree : 1;
  build();
  }

  void Interpolation::build()
  {
  _piecewise.reset();
  switch (_method)
    {
    case Linear:
      {
      std::vector<std::vector<double>> c(_x.size() - 1);
      for (std::size_t i=0; i+1<_x.size(); i++) c[i] = { _y[i], (_y[i+1] - _y[i])/(_x[i+1] - _x[i]) };
      _piecewise.reset(new PiecewisePolynomial(_x, c));
      }
      break;
    case Spline:         _piecewise.reset(new CubicSpline(_x, _y, CubicSpline::NotAKnot)); break;
    case NaturalSpline:  _piecewise.reset(new CubicSpline(_x, _y, CubicSpline::Natural)); break;
    case ClampedSpline:  _piecewise.reset(new CubicSpline(_x, _y, CubicSpline::Clamped, _slopeLow, _slopeHigh)); break;
    case MonotoneSpline: _piecewise.reset(new CubicSpline(_x, _y, CubicSpline::Monotone)); break;
    default: break;
    }
  }

  double Interpolation::value(const std::vector<double> & x, const std::vector<double> & y, double x0,
                              Method method, unsigned int degree)
  {
  if (x.size() != y.size()) throw MathException("The sizes of x and y differ", __FUNCTION__);
  if (x.size() < 2) throw MathException("Fewer than 2 points", __FUNCTION__);
  switch (method)
    {
    case Nearest:
      {
      std::size_t i = findInterval(x, x0);
      return std::fabs(x0 - x[i]) <= std::fabs(x[i+1] - x0) ? y[i] : y[i+1];
      }
    case Linear:
      {
      std::size_t i = findInterval(x, x0);
      return y[i] + (y[i+1] - y[i])*(x0 - x[i])/(x[i+1] - x[i]);
      }
    case LocalPolynomial: return interpolateLocalPolynomial(x, y, x0, degree);
    case Polynomial:      return interpolatePolynomial(x, y, x0);
    default:              return Interpolation(x, y, method, degree).evaluateAt(x0);
    }
  }

  double Interpolation::evaluateAt(double x) const
  {
  if (_piecewise) return _piecewise->evaluateAt(x);
  return value(_x, _y, x, _method, _degree);
  }

  double Interpolation::evaluateAt(const std::vector<double> & x) const
  {
  checkRank(x.size(), __FUNCTION__);
  return evaluateAt(x[0]);
  }

  std::vector<double> Interpolation::values(const std::vector<double> & x) const
  {
  std::vector<double> v(x.size());
  for (std::size_t i=0; i<x.size(); i++) v[i] = evaluateAt(x[i]);
  return v;
  }

  double Interpolation::derivativeAt(double x) const
  {
  if (_piecewise) return _piecewise->derivativeAt(x);
  return Function::derivativeAt(x);
  }

  Function * Interpolation::derivativeFunction(unsigned int axis) const
  {
  if (!_piecewise) return nullptr;
  return _piecewise->derivativeFunction(axis);
  }

  double Interpolation::integralInRange(double low, double high) const
  {
  if (_piecewise) return _piecewise->integralInRange(low, high);
  return Function::integralInRange(low, high);
  }

  } // namespace MATH
} // namespace CAP
