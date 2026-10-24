/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Prior.cpp
//! @brief Implementation of Prior.
#include <cmath>
#include <cstdio>
#include <limits>
#include "Prior.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  Prior Prior::uniform(double low, double high)
  {
  if (!(high > low)) throw MathException("A uniform prior needs low < high", __FUNCTION__);
  Prior p; p._type = kUniform; p._a = low; p._b = high;
  char name[96]; std::snprintf(name, sizeof(name), "uniform(%g, %g)", low, high); p._name = name;
  return p;
  }

  Prior Prior::gaussian(double mean, double sigma)
  {
  if (!(sigma > 0)) throw MathException("A gaussian prior needs a positive width", __FUNCTION__);
  Prior p; p._type = kGaussian; p._a = mean; p._b = sigma;
  char name[96]; std::snprintf(name, sizeof(name), "gaussian(%g, %g)", mean, sigma); p._name = name;
  return p;
  }

  Prior Prior::logUniform(double low, double high)
  {
  if (!(low > 0) || !(high > low)) throw MathException("A log-uniform prior needs 0 < low < high", __FUNCTION__);
  Prior p; p._type = kLogUniform; p._a = low; p._b = high;
  char name[96]; std::snprintf(name, sizeof(name), "log-uniform(%g, %g)", low, high); p._name = name;
  return p;
  }

  Prior Prior::custom(std::function<double(double)> logDensity, const String & name)
  {
  Prior p; p._type = kCustom; p._custom = logDensity; p._name = name;
  return p;
  }

  double Prior::logDensity(double x) const
  {
  const double minusInfinity = -std::numeric_limits<double>::infinity();
  if (!std::isfinite(x)) return minusInfinity;
  switch (_type)
    {
    case kUniform:    return (x >= _a && x <= _b) ? -std::log(_b - _a) : minusInfinity;
    case kGaussian:   { double z = (x - _a)/_b; return -0.5*z*z - std::log(_b*std::sqrt(2*M_PI)); }
    case kLogUniform: return (x >= _a && x <= _b) ? -std::log(x) - std::log(std::log(_b/_a)) : minusInfinity;
    case kCustom:     return _custom ? _custom(x) : 0.0;
    default:          return 0.0;
    }
  }

  } // namespace MATH
} // namespace CAP
