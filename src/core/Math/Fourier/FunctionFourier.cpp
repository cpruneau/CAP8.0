/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FunctionFourier.cpp
//! @brief The Fourier transforms of Function: public interface and defaults (the classes: FunctionTransforms.cpp).
#include <cmath>
#include <cstdio>
#include <memory>
#include "Function.hpp"
#include "FourierTools.hpp"
#include "FourierTransform.hpp"
#include "MathExceptions.hpp"

namespace Datum
{
  namespace MATH
  {

  namespace
  {
  const double kTwoPi = 6.28318530717958647693;

  String nameOf(const Function & f)
  {
  return f.name().isNull() ? String(f.className()) : String(f.name());
  }
  } // namespace

  std::complex<double> Function::fourierTransformAt(double k) const
  {
  checkRank(1, __FUNCTION__);
  return lineTransformND(std::vector<double>{ k });   // by default lineTransform(k); composites override lineTransformND()
  }

  std::complex<double> Function::fourierTransformAt(const std::vector<double> & k) const
  {
  checkRank((unsigned int)(k.size()), __FUNCTION__);
  return lineTransformND(k);
  }

  std::complex<double> Function::fourierTransformAt(double k, double low, double high) const
  {
  checkRank(1, __FUNCTION__);
  if (low == high) return 0.0;
  if (low > high) return -domainTransform(k, high, low);
  return domainTransform(k, low, high);
  }

  std::complex<double> Function::fourierCoefficient(int n, double x0, double period) const
  {
  if (!(period > 0.0) || !std::isfinite(period)) throw MathException("The period must be > 0 and finite", __FUNCTION__);
  return fourierTransformAt(kTwoPi*n/period, x0, x0 + period)/period;
  }

  FourierTransform * Function::fourierTransform() const
  {
  return newLineTransform();
  }

  FourierTransform * Function::fourierTransform(double low, double high) const
  {
  checkRank(1, __FUNCTION__);
  if (!(high > low)) throw MathException("The domain must have low < high", __FUNCTION__);
  return newDomainTransform(low, high);
  }

  std::complex<double> Function::lineTransform(double k) const
  {
  (void) k;
  throw MathException(String("No Fourier transform on the whole line in closed form for ") + nameOf(*this)
                      + String(": give a domain"), __FUNCTION__);
  }

  std::complex<double> Function::lineTransformND(const std::vector<double> & k) const
  {
  if (rank() == 1) return lineTransform(k[0]);
  throw MathException(String("No Fourier transform on the whole line in closed form for ") + nameOf(*this), __FUNCTION__);
  }

  std::complex<double> Function::domainTransform(double k, double low, double high) const
  {
  auto f = [this](double x) { return evaluateAt(x); };
  int degree = polynomialDegree();
  if (degree >= 0) return FourierTools::polynomialTransform(f, (unsigned int)(degree), k, low, high);
  return FourierTools::legendreTransform(FourierTools::adaptiveLegendreCoefficients(f, low, high), k, low, high);
  }

  FourierTransform * Function::newLineTransform() const
  {
  if (!hasAnalyticalFourierTransform())
    throw MathException(String("No Fourier transform on the whole line in closed form for ") + nameOf(*this)
                        + String(": give a domain, fourierTransform(low, high)"), __FUNCTION__);
  std::shared_ptr<const Function> copy(copyFunction());
  if (!copy) throw MathException(String("The class of ") + nameOf(*this) + String(" cannot be copied (copyFunction())"), __FUNCTION__);
  return new FourierTransform(rank(), [copy](const std::vector<double> & k) { return copy->fourierTransformAt(k); },
                              FourierTransform::kClosedForm, "closed form", nameOf(*this));
  }

  FourierTransform * Function::newDomainTransform(double low, double high) const
  {
  int degree = polynomialDegree();
  if (degree >= 0)
    {
    // exact: the Legendre series of the polynomial, computed once
    std::vector<double> c = FourierTools::legendreCoefficients([this](double x) { return evaluateAt(x); }, low, high,
                                                                (unsigned int)(degree));
    char text[96];
    std::snprintf(text, sizeof(text), "exact on [%g, %g] (polynomial of degree %d)", low, high, degree);
    FourierTransform * t = new FourierTransform(1, [c, low, high](const std::vector<double> & k)
                                                { return FourierTools::legendreTransform(c, k[0], low, high); },
                                                FourierTransform::kExactOnDomain, text, nameOf(*this));
    t->setDomain(low, high);
    return t;
    }
  if (hasAnalyticalFourierTransformOnDomain())
    {
    std::shared_ptr<const Function> copy(copyFunction());
    if (copy)
      {
      char text[64];
      std::snprintf(text, sizeof(text), "exact on [%g, %g]", low, high);
      FourierTransform * t = new FourierTransform(1, [copy, low, high](const std::vector<double> & k)
                                                  { return copy->fourierTransformAt(k[0], low, high); },
                                                  FourierTransform::kExactOnDomain, text, nameOf(*this));
      t->setDomain(low, high);
      return t;
      }
    }
  return approximateDomainTransform(low, high);
  }

  FourierTransform * Function::approximateDomainTransform(double low, double high) const
  {
  double tail = 0.0;
  std::vector<double> c = FourierTools::adaptiveLegendreCoefficients([this](double x) { return evaluateAt(x); }, low, high,
                                                                      1e-13, 2048, &tail);
  char text[160];
  std::snprintf(text, sizeof(text), "Legendre approximation of degree %u on [%g, %g] (neglected coefficients %.1e of the largest)",
                (unsigned int)(c.size() - 1), low, high, tail);
  FourierTransform * t = new FourierTransform(1, [c, low, high](const std::vector<double> & k)
                                              { return FourierTools::legendreTransform(c, k[0], low, high); },
                                              FourierTransform::kApproximation, text, nameOf(*this));
  t->setDomain(low, high);
  return t;
  }

  } // namespace MATH
} // namespace Datum
