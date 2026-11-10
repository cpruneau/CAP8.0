/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file SpecialFunctions.cpp
//! @brief Implementation of GammaFunction, BetaFunction and IncompleteGammaFunction.
#include <cmath>
#include "SpecialFunctions.hpp"
#include "Distributions.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  // ---- GammaFunction

  GammaFunction::GammaFunction()
  :
  Function(String("Gamma"), String("Gamma function"), String("GammaFunction"), 1)
  { }

  double GammaFunction::evaluateAt(double x) const
  {
  return std::tgamma(x);
  }

  double GammaFunction::evaluateAt(const std::vector<double> & x) const
  {
  if (x.size() != 1) throw MathException("The gamma function has one variable", __FUNCTION__);
  return evaluateAt(x[0]);
  }

  double GammaFunction::logGamma(double x)
  {
  return std::lgamma(x);
  }

  // ---- BetaFunction

  BetaFunction::BetaFunction()
  :
  Function(String("Beta"), String("Beta function"), String("BetaFunction"), 2)
  { }

  double BetaFunction::evaluateAt(double a, double b) const
  {
  return beta(a, b);
  }

  double BetaFunction::evaluateAt(const std::vector<double> & x) const
  {
  if (x.size() != 2) throw MathException("The beta function has two variables", __FUNCTION__);
  return beta(x[0], x[1]);
  }

  double BetaFunction::beta(double a, double b)
  {
  if (a > 0.0 && b > 0.0) return std::exp(logBeta(a, b));
  return std::tgamma(a)*std::tgamma(b)/std::tgamma(a + b);
  }

  double BetaFunction::logBeta(double a, double b)
  {
  return std::lgamma(a) + std::lgamma(b) - std::lgamma(a + b);
  }

  // ---- IncompleteGammaFunction

  IncompleteGammaFunction::IncompleteGammaFunction(Kind kind)
  :
  Function(String("IncompleteGamma"), String("Incomplete gamma function"), String("IncompleteGammaFunction"), 2),
  _kind(kind)
  { }

  double IncompleteGammaFunction::evaluateAt(double a, double x) const
  {
  switch (_kind)
    {
    case kLowerRegularized: return Distributions::regularizedGammaP(a, x);
    case kUpperRegularized: return Distributions::regularizedGammaQ(a, x);
    case kLower:            return Distributions::regularizedGammaP(a, x)*std::tgamma(a);
    case kUpper:            return Distributions::regularizedGammaQ(a, x)*std::tgamma(a);
    }
  return 0.0;
  }

  double IncompleteGammaFunction::evaluateAt(const std::vector<double> & x) const
  {
  if (x.size() != 2) throw MathException("The incomplete gamma function has two variables", __FUNCTION__);
  return evaluateAt(x[0], x[1]);
  }

  } // namespace MATH
} // namespace CAP
