/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file SignalFunctions.cpp
//! @brief Implementation of the functions of SignalFunctions.hpp and of their Fourier transforms.
#include <algorithm>
#include <cmath>
#include <limits>
#include "SignalFunctions.hpp"
#include "FourierTools.hpp"
#include "MathExceptions.hpp"

namespace Datum
{
  namespace MATH
  {

  namespace
  {
  using Complex = std::complex<double>;
  const double kPi = 3.14159265358979323846;
  const double kInfinity = std::numeric_limits<double>::infinity();
  const Complex kI(0.0, 1.0);

  Parameter positive(const char * name, double value) { return Parameter(name, value, Parameter::Low, 0.0); }

  void requirePositive(double value, const char * what, const char * where)
  {
  if (!(value > 0.0)) throw MathException(String(what) + " must be > 0", where);
  }

  //! e^{-ik x0}
  Complex phaseAt(double k, double x0) { return std::exp(Complex(0.0, -k*x0)); }

  //! Signed integral helper: the integral over [low, high] from one over the ordered range.
  template <typename F>
  double ordered(double low, double high, F integral)
  {
  if (low == high) return 0.0;
  return low < high ? integral(low, high) : -integral(high, low);
  }
  } // namespace

  // ---- Boxcar

  Boxcar::Boxcar(double amplitude, double position, double width)
  :
  SignalFunction({ Parameter("amplitude", amplitude), Parameter("position", position), positive("width", width) })
  {
  requirePositive(width, "The width of a boxcar", __FUNCTION__);
  }

  double Boxcar::evaluateAt(double x) const
  {
  double u = std::fabs(x - position()), half = 0.5*width();
  if (u < half) return amplitude();
  return u == half ? 0.5*amplitude() : 0.0;
  }

  double Boxcar::integralInRange(double low, double high) const
  {
  return ordered(low, high, [this](double a, double b)
    {
    double lo = std::max(a, position() - 0.5*width()), hi = std::min(b, position() + 0.5*width());
    return hi > lo ? amplitude()*(hi - lo) : 0.0;
    });
  }

  Complex Boxcar::lineTransform(double k) const
  {
  return amplitude()*width()*FourierTools::sinc(0.5*k*width())*phaseAt(k, position());
  }

  Complex Boxcar::domainTransform(double k, double low, double high) const
  {
  double lo = std::max(low, position() - 0.5*width()), hi = std::min(high, position() + 0.5*width());
  if (!(hi > lo)) return 0.0;
  return amplitude()*FourierTools::exponentialIntegral(Complex(0.0, -k), lo, hi);
  }

  // ---- Triangle

  Triangle::Triangle(double amplitude, double position, double halfWidth)
  :
  SignalFunction({ Parameter("amplitude", amplitude), Parameter("position", position), positive("halfWidth", halfWidth) })
  {
  requirePositive(halfWidth, "The half width of a triangle", __FUNCTION__);
  }

  double Triangle::evaluateAt(double x) const
  {
  double u = std::fabs(x - position())/halfWidth();
  return u < 1.0 ? amplitude()*(1.0 - u) : 0.0;
  }

  double Triangle::integralInRange(double low, double high) const
  {
  return ordered(low, high, [this](double a, double b)
    {
    double x0 = position(), w = halfWidth(), sum = 0.0;
    for (double edge : { x0 - w, x0 })
      {
      double lo = std::max(a, edge), hi = std::min(b, edge + w);
      if (hi > lo) sum += 0.5*(hi - lo)*(evaluateAt(lo) + evaluateAt(hi));
      }
    return sum;
    });
  }

  Complex Triangle::lineTransform(double k) const
  {
  double s = FourierTools::sinc(0.5*k*halfWidth());
  return amplitude()*halfWidth()*s*s*phaseAt(k, position());
  }

  Complex Triangle::domainTransform(double k, double low, double high) const
  {
  double x0 = position(), w = halfWidth();
  Complex sum = 0.0;
  auto f = [this](double x) { return evaluateAt(x); };
  for (double edge : { x0 - w, x0 })   // two linear pieces
    {
    double lo = std::max(low, edge), hi = std::min(high, edge + w);
    if (hi > lo) sum += FourierTools::polynomialTransform(f, 1, k, lo, hi);
    }
  return sum;
  }

  // ---- Sinc

  Sinc::Sinc(double amplitude, double position, double q)
  :
  SignalFunction({ Parameter("amplitude", amplitude), Parameter("position", position), positive("q", q) })
  {
  requirePositive(q, "q of a sinc", __FUNCTION__);
  }

  double Sinc::evaluateAt(double x) const
  {
  return amplitude()*FourierTools::sinc(q()*(x - position()));
  }

  Complex Sinc::lineTransform(double k) const
  {
  double a = std::fabs(k);
  if (a > q()) return 0.0;
  double weight = a == q() ? 0.5 : 1.0;
  return weight*amplitude()*(kPi/q())*phaseAt(k, position());
  }

  // ---- TwoSidedExponential

  TwoSidedExponential::TwoSidedExponential(double amplitude, double position, double tau)
  :
  SignalFunction({ Parameter("amplitude", amplitude), Parameter("position", position), positive("tau", tau) })
  {
  requirePositive(tau, "tau of an exponential", __FUNCTION__);
  }

  double TwoSidedExponential::evaluateAt(double x) const
  {
  return amplitude()*std::exp(-std::fabs(x - position())/tau());
  }

  double TwoSidedExponential::integralInRange(double low, double high) const
  {
  return domainTransform(0.0, std::min(low, high), std::max(low, high)).real()*(low <= high ? 1.0 : -1.0);
  }

  Complex TwoSidedExponential::lineTransform(double k) const
  {
  double t = tau();
  return 2.0*amplitude()*t/(1.0 + k*k*t*t)*phaseAt(k, position());
  }

  Complex TwoSidedExponential::domainTransform(double k, double low, double high) const
  {
  // in u = x - x0: e^{u/tau} below 0, e^{-u/tau} above
  double x0 = position(), a = low - x0, b = high - x0, t = tau();
  Complex sum = 0.0;
  if (a < 0.0) sum += FourierTools::exponentialIntegral(Complex(1.0/t, -k), a, std::min(b, 0.0));
  if (b > 0.0) sum += FourierTools::exponentialIntegral(Complex(-1.0/t, -k), std::max(a, 0.0), b);
  return amplitude()*phaseAt(k, x0)*sum;
  }

  // ---- OneSidedExponential

  OneSidedExponential::OneSidedExponential(double amplitude, double position, double tau)
  :
  SignalFunction({ Parameter("amplitude", amplitude), Parameter("position", position), positive("tau", tau) })
  {
  requirePositive(tau, "tau of an exponential", __FUNCTION__);
  }

  double OneSidedExponential::evaluateAt(double x) const
  {
  return x >= position() ? amplitude()*std::exp(-(x - position())/tau()) : 0.0;
  }

  double OneSidedExponential::integralInRange(double low, double high) const
  {
  return domainTransform(0.0, std::min(low, high), std::max(low, high)).real()*(low <= high ? 1.0 : -1.0);
  }

  Complex OneSidedExponential::lineTransform(double k) const
  {
  double t = tau();
  return amplitude()*t/Complex(1.0, k*t)*phaseAt(k, position());
  }

  Complex OneSidedExponential::domainTransform(double k, double low, double high) const
  {
  double x0 = position(), a = std::max(low - x0, 0.0), b = high - x0;
  if (!(b > a)) return 0.0;
  return amplitude()*phaseAt(k, x0)*FourierTools::exponentialIntegral(Complex(-1.0/tau(), -k), a, b);
  }

  // ---- DampedOscillation

  DampedOscillation::DampedOscillation(double amplitude, double position, double tau, double omega, double phi)
  :
  SignalFunction({ Parameter("amplitude", amplitude), Parameter("position", position), positive("tau", tau),
                   Parameter("omega", omega), Parameter("phi", phi) })
  {
  requirePositive(tau, "tau of a damped oscillation", __FUNCTION__);
  }

  double DampedOscillation::evaluateAt(double x) const
  {
  double u = x - position();
  if (u < 0.0) return 0.0;
  return amplitude()*std::exp(-u/tau())*std::sin(omega()*u + phase());
  }

  Complex DampedOscillation::lineTransform(double k) const
  {
  return domainTransform(k, position(), kInfinity);
  }

  Complex DampedOscillation::domainTransform(double k, double low, double high) const
  {
  double x0 = position(), a = std::max(low - x0, 0.0), b = high - x0;
  if (!(b > a)) return 0.0;
  double g = 1.0/tau(), w = omega(), phi = phase();
  // sin(w u + phi) = (e^{i(w u + phi)} - e^{-i(w u + phi)})/(2i)
  Complex plus  = std::exp(Complex(0.0, phi))*FourierTools::exponentialIntegral(Complex(-g, w - k), a, b);
  Complex minus = std::exp(Complex(0.0, -phi))*FourierTools::exponentialIntegral(Complex(-g, -w - k), a, b);
  return amplitude()*phaseAt(k, x0)*(plus - minus)/(2.0*kI);
  }

  // ---- GaussianWavePacket

  GaussianWavePacket::GaussianWavePacket(double amplitude, double position, double sigma, double k0, double phi)
  :
  SignalFunction({ Parameter("amplitude", amplitude), Parameter("position", position), positive("sigma", sigma),
                   Parameter("k0", k0), Parameter("phi", phi) })
  {
  requirePositive(sigma, "sigma of a wave packet", __FUNCTION__);
  }

  double GaussianWavePacket::evaluateAt(double x) const
  {
  double u = (x - position())/sigma();
  return amplitude()*std::exp(-0.5*u*u)*std::cos(waveNumber()*(x - position()) + phase());
  }

  Complex GaussianWavePacket::lineTransform(double k) const
  {
  double x0 = position(), s = sigma(), k0 = waveNumber(), phi = phase();
  Complex plus  = std::exp(Complex(0.0, phi - k0*x0))*FourierTools::gaussianLine(k - k0, x0, s);
  Complex minus = std::exp(Complex(0.0, -phi + k0*x0))*FourierTools::gaussianLine(k + k0, x0, s);
  return 0.5*amplitude()*(plus + minus);
  }

  Complex GaussianWavePacket::domainTransform(double k, double low, double high) const
  {
  double x0 = position(), s = sigma(), k0 = waveNumber(), phi = phase();
  Complex plus  = std::exp(Complex(0.0, phi - k0*x0))*FourierTools::gaussianDomain(k - k0, x0, s, low, high);
  Complex minus = std::exp(Complex(0.0, -phi + k0*x0))*FourierTools::gaussianDomain(k + k0, x0, s, low, high);
  return 0.5*amplitude()*(plus + minus);
  }

  // ---- HyperbolicSecant

  HyperbolicSecant::HyperbolicSecant(double amplitude, double position, double scale)
  :
  SignalFunction({ Parameter("amplitude", amplitude), Parameter("position", position), positive("scale", scale) })
  {
  requirePositive(scale, "The scale of a hyperbolic secant", __FUNCTION__);
  }

  double HyperbolicSecant::evaluateAt(double x) const
  {
  return amplitude()/std::cosh((x - position())/scale());
  }

  double HyperbolicSecant::integralInRange(double low, double high) const
  {
  // antiderivative s gd(u) = 2 s atan(tanh(u/2))
  auto g = [this](double x) { return 2.0*scale()*std::atan(std::tanh(0.5*(x - position())/scale())); };
  return amplitude()*(g(high) - g(low));
  }

  Complex HyperbolicSecant::lineTransform(double k) const
  {
  double s = scale();
  return amplitude()*kPi*s/std::cosh(0.5*kPi*s*k)*phaseAt(k, position());
  }

  // ---- HermiteFunction

  HermiteFunction::HermiteFunction(unsigned int order, double amplitude, double position, double scale)
  :
  SignalFunction({ Parameter("amplitude", amplitude), Parameter("position", position), positive("scale", scale),
                   Parameter("order", double(order), Parameter::Fixed) })
  {
  requirePositive(scale, "The scale of a Hermite function", __FUNCTION__);
  }

  unsigned int HermiteFunction::order() const
  {
  double n = std::round(parameterValue(3));
  return n > 0.0 ? (unsigned int)(n) : 0u;
  }

  double HermiteFunction::psi(unsigned int n, double t)
  {
  double p0 = std::pow(kPi, -0.25)*std::exp(-0.5*t*t);
  if (n == 0) return p0;
  double p1 = std::sqrt(2.0)*t*p0;
  for (unsigned int m=1; m<n; m++)
    {
    double p2 = std::sqrt(2.0/(m + 1.0))*t*p1 - std::sqrt(double(m)/(m + 1.0))*p0;
    p0 = p1; p1 = p2;
    }
  return p1;
  }

  double HermiteFunction::evaluateAt(double x) const
  {
  return amplitude()*psi(order(), (x - position())/scale());
  }

  Complex HermiteFunction::lineTransform(double k) const
  {
  unsigned int n = order();
  Complex phase = 1.0;   // (-i)^n
  for (unsigned int m=0; m<n%4; m++) phase *= -kI;
  double s = scale();
  return amplitude()*s*std::sqrt(2.0*kPi)*phase*psi(n, s*k)*phaseAt(k, position());
  }

  // ---- Voigt

  Voigt::Voigt(double amplitude, double position, double sigma, double gamma)
  :
  SignalFunction({ Parameter("amplitude", amplitude), Parameter("position", position), positive("sigma", sigma),
                   Parameter("gamma", gamma, Parameter::Low, 0.0) })
  {
  requirePositive(sigma, "sigma of a Voigt profile", __FUNCTION__);
  if (gamma < 0.0) throw MathException("gamma of a Voigt profile must be >= 0", __FUNCTION__);
  }

  double Voigt::evaluateAt(double x) const
  {
  double s = sigma()*std::sqrt(2.0);
  Complex z((x - position())/s, gamma()/s);
  return amplitude()*FourierTools::faddeeva(z).real()/(sigma()*std::sqrt(2.0*kPi));
  }

  Complex Voigt::lineTransform(double k) const
  {
  double s = sigma();
  return amplitude()*std::exp(-0.5*s*s*k*k - gamma()*std::fabs(k))*phaseAt(k, position());
  }

  // ---- HeavisideStep

  HeavisideStep::HeavisideStep(double amplitude, double position)
  :
  SignalFunction({ Parameter("amplitude", amplitude), Parameter("position", position) })
  { }

  double HeavisideStep::evaluateAt(double x) const
  {
  if (x > position()) return amplitude();
  return x == position() ? 0.5*amplitude() : 0.0;
  }

  double HeavisideStep::integralInRange(double low, double high) const
  {
  return ordered(low, high, [this](double a, double b)
    {
    double lo = std::max(a, position());
    return b > lo ? amplitude()*(b - lo) : 0.0;
    });
  }

  Complex HeavisideStep::domainTransform(double k, double low, double high) const
  {
  double lo = std::max(low, position());
  if (!(high > lo)) return 0.0;
  return amplitude()*FourierTools::exponentialIntegral(Complex(0.0, -k), lo, high);
  }

  // ---- DiracComb

  DiracComb::DiracComb(double weight, double position, double period)
  :
  SignalFunction({ Parameter("weight", weight), Parameter("position", position), positive("period", period) })
  {
  requirePositive(period, "The period of a Dirac comb", __FUNCTION__);
  }

  double DiracComb::integralInRange(double low, double high) const
  {
  return domainTransform(0.0, std::min(low, high), std::max(low, high)).real()*(low <= high ? 1.0 : -1.0);
  }

  double DiracComb::transformPeriod() const
  {
  return 2.0*kPi/period();
  }

  Complex DiracComb::transformWeight(int n) const
  {
  double L = period();
  return 2.0*kPi*amplitude()/L*std::exp(Complex(0.0, -2.0*kPi*n*position()/L));
  }

  Complex DiracComb::domainTransform(double k, double low, double high) const
  {
  if (!std::isfinite(low) || !std::isfinite(high)) throw MathException("A Dirac comb has a transform on a finite domain only", __FUNCTION__);
  double x0 = position(), L = period();
  double m1 = std::ceil((low - x0)/L), m2 = std::floor((high - x0)/L);
  if (m1 > m2) return 0.0;
  double n = m2 - m1 + 1.0;
  // sum over m of e^{-ik(x0 + m L)} = e^{-ik x0} sum e^{-i theta m}, theta = k L reduced to (-pi, pi]
  double theta = std::remainder(k*L, 2.0*kPi);
  Complex first = std::exp(Complex(0.0, -k*x0))*std::exp(Complex(0.0, -theta*m1));
  // sum of e^{-i theta m} over n terms: e^{-i theta (n-1)/2} sin(n theta/2)/sin(theta/2)
  double half = std::sin(0.5*theta);
  Complex ratio = half == 0.0 ? Complex(n, 0.0)
                : std::exp(Complex(0.0, -0.5*theta*(n - 1.0)))*(std::sin(0.5*n*theta)/half);
  Complex sum = first*ratio;
  // a tooth on a bound counts half
  double tolerance = 1e-12*L;
  if (std::fabs(x0 + m1*L - low) <= tolerance) sum -= 0.5*std::exp(Complex(0.0, -k*(x0 + m1*L)));
  if (std::fabs(x0 + m2*L - high) <= tolerance) sum -= 0.5*std::exp(Complex(0.0, -k*(x0 + m2*L)));
  return amplitude()*sum;
  }

  } // namespace MATH
} // namespace Datum
