/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file PiecewisePolynomial.cpp
//! @brief Implementation of PiecewisePolynomial.
#include <cmath>
#include "PiecewisePolynomial.hpp"
#include "InterpolationTools.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  PiecewisePolynomial::PiecewisePolynomial()
  :
  Function("PiecewisePolynomial", "PiecewisePolynomial", "CAP::MATH::PiecewisePolynomial", 1)
  { }

  PiecewisePolynomial::PiecewisePolynomial(const std::vector<double> & breakpoints,
                                           const std::vector<std::vector<double>> & coefficients)
  :
  Function("PiecewisePolynomial", "PiecewisePolynomial", "CAP::MATH::PiecewisePolynomial", 1)
  {
  setPieces(breakpoints, coefficients);
  }

  void PiecewisePolynomial::setPieces(const std::vector<double> & breakpoints,
                                      const std::vector<std::vector<double>> & coefficients)
  {
  if (breakpoints.size() != coefficients.size() + 1 || coefficients.empty())
    throw MathException("There must be one breakpoint more than pieces, and one piece or more", __FUNCTION__);
  for (std::size_t i=0; i+1<breakpoints.size(); i++)
    if (!(breakpoints[i+1] > breakpoints[i])) throw MathException("The breakpoints do not increase", __FUNCTION__);
  _breakpoints = breakpoints;
  _coefficients = coefficients;
  _cumulative.assign(_breakpoints.size(), 0.0);
  for (std::size_t i=0; i<_coefficients.size(); i++)
    _cumulative[i+1] = _cumulative[i] + pieceIntegral(i, _breakpoints[i+1]);
  }

  std::size_t PiecewisePolynomial::piece(double x) const
  {
  if (_coefficients.size() <= 1) return 0;
  return findInterval(_breakpoints, x);
  }

  double PiecewisePolynomial::evaluateAt(double x) const
  {
  if (_coefficients.empty()) return 0.0;
  std::size_t i = piece(x);
  const std::vector<double> & c = _coefficients[i];
  double t = x - _breakpoints[i], value = 0.0;
  for (std::size_t k=c.size(); k-->0; ) value = value*t + c[k];
  return value;
  }

  double PiecewisePolynomial::evaluateAt(const std::vector<double> & x) const
  {
  checkRank(x.size(), __FUNCTION__);
  return evaluateAt(x[0]);
  }

  double PiecewisePolynomial::derivativeAt(double x, unsigned int order) const
  {
  if (_coefficients.empty()) return 0.0;
  std::size_t i = piece(x);
  const std::vector<double> & c = _coefficients[i];
  double t = x - _breakpoints[i], value = 0.0;
  for (std::size_t k=c.size(); k-->order; )
    {
    double factor = 1.0;                         // k (k-1) ... (k-order+1)
    for (unsigned int j=0; j<order; j++) factor *= double(k - j);
    value = value*t + factor*c[k];
    }
  return value;
  }

  double PiecewisePolynomial::derivativeAt(double x) const
  {
  return derivativeAt(x, 1);
  }

  Function * PiecewisePolynomial::derivativeFunction(unsigned int axis) const
  {
  if (axis != 0 || _coefficients.empty()) return nullptr;
  std::vector<std::vector<double>> d(_coefficients.size());
  for (std::size_t i=0; i<_coefficients.size(); i++)
    {
    const std::vector<double> & c = _coefficients[i];
    for (std::size_t k=1; k<c.size(); k++) d[i].push_back(double(k)*c[k]);
    if (d[i].empty()) d[i].push_back(0.0);
    }
  PiecewisePolynomial * p = new PiecewisePolynomial(_breakpoints, d);
  p->setName(String("D") + name());
  return p;
  }

  double PiecewisePolynomial::pieceIntegral(std::size_t i, double x) const
  {
  const std::vector<double> & c = _coefficients[i];
  double t = x - _breakpoints[i], value = 0.0;
  for (std::size_t k=c.size(); k-->0; ) value = value*t + c[k]/double(k + 1);
  return value*t;
  }

  double PiecewisePolynomial::antiderivativeAt(double x) const
  {
  if (_coefficients.empty()) return 0.0;
  std::size_t i = piece(x);
  return _cumulative[i] + pieceIntegral(i, x);
  }

  double PiecewisePolynomial::integralInRange(double low, double high) const
  {
  if (!std::isfinite(low) || !std::isfinite(high)) throw MathException("Integration bounds must be finite", __FUNCTION__);
  return antiderivativeAt(high) - antiderivativeAt(low);
  }

  } // namespace MATH
} // namespace CAP
