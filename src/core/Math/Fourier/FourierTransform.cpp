/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FourierTransform.cpp
//! @brief Implementation of FourierTransform.
#include <cmath>
#include "FourierTransform.hpp"
#include "MathExceptions.hpp"

namespace Datum
{
  namespace MATH
  {

  FourierTransform::FourierTransform(unsigned int rank, const Evaluator & evaluator, Method method,
                                     const String & description, const String & name)
  :
  Function(name.isNull() ? String("FourierTransform") : String("FT of ") + name,
           name.isNull() ? String("Fourier transform") : String("Fourier transform of ") + name, "FourierTransform", rank),
  _evaluator(evaluator),
  _method(method),
  _description(description),
  _part(kReal),
  _hasDomain(false),
  _low(0.0),
  _high(0.0)
  {
  if (!_evaluator) throw MathException("A Fourier transform needs an evaluator", __FUNCTION__);
  }

  std::complex<double> FourierTransform::valueAt(double k) const
  {
  checkRank(1, __FUNCTION__);
  return _evaluator(std::vector<double>{ k });
  }

  std::complex<double> FourierTransform::valueAt(double kx, double ky) const
  {
  checkRank(2, __FUNCTION__);
  return _evaluator(std::vector<double>{ kx, ky });
  }

  std::complex<double> FourierTransform::valueAt(const std::vector<double> & k) const
  {
  checkRank((unsigned int)(k.size()), __FUNCTION__);
  return _evaluator(k);
  }

  double FourierTransform::partOf(const std::complex<double> & value, Part part)
  {
  switch (part)
    {
    case kImaginary:      return value.imag();
    case kModulus:        return std::abs(value);
    case kModulusSquared: return std::norm(value);
    case kPhase:          return std::arg(value);
    default:              return value.real();
    }
  }

  double FourierTransform::evaluateAt(const std::vector<double> & k) const
  {
  return partOf(valueAt(k), _part);
  }

  double FourierTransform::evaluateAt(double k) const
  {
  return partOf(valueAt(k), _part);
  }

  std::complex<double> FourierTransform::coefficient(int n) const
  {
  if (!_hasDomain || !std::isfinite(_low) || !std::isfinite(_high) || !(_high > _low))
    throw MathException("Fourier series coefficients need a transform on a finite domain", __FUNCTION__);
  double length = _high - _low;
  return valueAt(2.0*3.14159265358979323846*n/length)/length;
  }

  } // namespace MATH
} // namespace Datum
