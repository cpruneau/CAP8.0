/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file SignalFunctions.hpp
//! @brief Functions with closed-form Fourier transforms: Boxcar, Triangle, Sinc, TwoSidedExponential,
//! OneSidedExponential, DampedOscillation, GaussianWavePacket, HyperbolicSecant, HermiteFunction, Voigt, HeavisideStep,
//! DiracComb.
#ifndef DATUM_MATH_SignalFunctions
#define DATUM_MATH_SignalFunctions
#include <complex>
#include "ParameterizedFunction.hpp"

namespace Datum
{
  namespace MATH
  {

  //!
  //! @brief Base of the functions of SignalFunctions.hpp: f(x) = A g((x - x0) ...), of rank 1, whose parameters 0 and 1
  //! are the amplitude A and the position x0. Their transforms on the whole line are in closed form (see
  //! Function::fourierTransform() and docs/Fourier.md; HeavisideStep and DiracComb: on a domain only).
  //!
  class SignalFunction : public ParameterizedFunction
  {
  public:
  //! @brief Function of the given parameters (the first two: amplitude, position). @param parameters parameters
  explicit SignalFunction(const std::vector<Parameter> & parameters) : ParameterizedFunction(parameters, 1) { }
  virtual ~SignalFunction() { }
  double amplitude() const { return parameterValue(0); }   //!< Amplitude A
  double position() const { return parameterValue(1); }   //!< Position x0
  void setAmplitude(double a) { setParameterValue(0, a); }   //!< Set the amplitude
  void setPosition(double x0) { setParameterValue(1, x0); }   //!< Set the position
  bool hasAnalyticalFourierTransform() const override { return true; }
  using ParameterizedFunction::evaluateAt;
  using ParameterizedFunction::integralInRange;
  };

  //! @brief Boxcar (rectangle): A for |x - x0| < w/2, A/2 at the edges, 0 outside. Parameters: amplitude, position,
  //! width w (> 0). Transform: A w sinc(k w/2) e^{-ik x0}; exact on any domain.
  class Boxcar : public SignalFunction
  {
  public:
  Boxcar(double amplitude=1.0, double position=0.0, double width=1.0);   //!< Function
  Boxcar * clone() const override { return new Boxcar(*this); }   //!< Copy
  double width() const { return parameterValue(2); }   //!< Width w
  double evaluateAt(double x) const override;   //!< Value
  double integralInRange(double low, double high) const override;   //!< Exact integral
  bool hasAnalyticalFourierTransformOnDomain() const override { return true; }
  protected:
  std::complex<double> lineTransform(double k) const override;
  std::complex<double> domainTransform(double k, double low, double high) const override;
  };

  //! @brief Triangle: A (1 - |x - x0|/w) for |x - x0| < w, 0 outside. Parameters: amplitude, position, half width w (> 0).
  //! Transform: A w sinc^2(k w/2) e^{-ik x0}; exact on any domain.
  class Triangle : public SignalFunction
  {
  public:
  Triangle(double amplitude=1.0, double position=0.0, double halfWidth=1.0);   //!< Function
  Triangle * clone() const override { return new Triangle(*this); }   //!< Copy
  double halfWidth() const { return parameterValue(2); }   //!< Half width w
  double evaluateAt(double x) const override;   //!< Value
  double integralInRange(double low, double high) const override;   //!< Exact integral
  bool hasAnalyticalFourierTransformOnDomain() const override { return true; }
  protected:
  std::complex<double> lineTransform(double k) const override;
  std::complex<double> domainTransform(double k, double low, double high) const override;
  };

  //! @brief Sinc: A sin(q (x - x0))/(q (x - x0)) (A at x0). Parameters: amplitude, position, q (> 0).
  //! Transform: A (pi/q) e^{-ik x0} for |k| < q (half of it at |k| = q), 0 for |k| > q.
  class Sinc : public SignalFunction
  {
  public:
  Sinc(double amplitude=1.0, double position=0.0, double q=1.0);   //!< Function
  Sinc * clone() const override { return new Sinc(*this); }   //!< Copy
  double q() const { return parameterValue(2); }   //!< q
  double evaluateAt(double x) const override;   //!< Value
  protected:
  std::complex<double> lineTransform(double k) const override;
  };

  //! @brief Two-sided exponential (Laplace): A e^{-|x - x0|/tau}. Parameters: amplitude, position, tau (> 0).
  //! Transform: 2 A tau/(1 + k^2 tau^2) e^{-ik x0}; exact on any domain.
  class TwoSidedExponential : public SignalFunction
  {
  public:
  TwoSidedExponential(double amplitude=1.0, double position=0.0, double tau=1.0);   //!< Function
  TwoSidedExponential * clone() const override { return new TwoSidedExponential(*this); }   //!< Copy
  double tau() const { return parameterValue(2); }   //!< tau
  double evaluateAt(double x) const override;   //!< Value
  double integralInRange(double low, double high) const override;   //!< Exact integral
  bool hasAnalyticalFourierTransformOnDomain() const override { return true; }
  protected:
  std::complex<double> lineTransform(double k) const override;
  std::complex<double> domainTransform(double k, double low, double high) const override;
  };

  //! @brief One-sided exponential (decay): A e^{-(x - x0)/tau} for x >= x0, 0 below. Parameters: amplitude, position,
  //! tau (> 0). Transform: A tau e^{-ik x0}/(1 + ik tau); exact on any domain.
  class OneSidedExponential : public SignalFunction
  {
  public:
  OneSidedExponential(double amplitude=1.0, double position=0.0, double tau=1.0);   //!< Function
  OneSidedExponential * clone() const override { return new OneSidedExponential(*this); }   //!< Copy
  double tau() const { return parameterValue(2); }   //!< tau
  double evaluateAt(double x) const override;   //!< Value
  double integralInRange(double low, double high) const override;   //!< Exact integral
  bool hasAnalyticalFourierTransformOnDomain() const override { return true; }
  protected:
  std::complex<double> lineTransform(double k) const override;
  std::complex<double> domainTransform(double k, double low, double high) const override;
  };

  //! @brief Damped oscillation: A e^{-(x - x0)/tau} sin(omega (x - x0) + phi) for x >= x0, 0 below. Parameters:
  //! amplitude, position, tau (> 0), omega, phi. Transform: A e^{-ik x0} [e^{i phi}/(1/tau + i(k - omega)) - e^{-i phi}/
  //! (1/tau + i(k + omega))]/(2i) (phi = 0: A omega e^{-ik x0}/((1/tau + ik)^2 + omega^2)); exact on any domain.
  class DampedOscillation : public SignalFunction
  {
  public:
  DampedOscillation(double amplitude=1.0, double position=0.0, double tau=1.0, double omega=6.283185307179586, double phi=0.0);   //!< Function
  DampedOscillation * clone() const override { return new DampedOscillation(*this); }   //!< Copy
  double tau() const { return parameterValue(2); }   //!< tau
  double omega() const { return parameterValue(3); }   //!< Angular frequency omega
  double phase() const { return parameterValue(4); }   //!< Phase phi
  double evaluateAt(double x) const override;   //!< Value
  bool hasAnalyticalFourierTransformOnDomain() const override { return true; }
  protected:
  std::complex<double> lineTransform(double k) const override;
  std::complex<double> domainTransform(double k, double low, double high) const override;
  };

  //! @brief Gaussian wave packet: A e^{-(x - x0)^2/(2 sigma^2)} cos(k0 (x - x0) + phi). Parameters: amplitude, position,
  //! sigma (> 0), k0, phi. Transform: A sigma sqrt(2 pi)/2 e^{-ik x0} [e^{i phi} e^{-sigma^2 (k - k0)^2/2} + e^{-i phi}
  //! e^{-sigma^2 (k + k0)^2/2}]; exact on any domain (Faddeeva function).
  class GaussianWavePacket : public SignalFunction
  {
  public:
  GaussianWavePacket(double amplitude=1.0, double position=0.0, double sigma=1.0, double k0=5.0, double phi=0.0);   //!< Function
  GaussianWavePacket * clone() const override { return new GaussianWavePacket(*this); }   //!< Copy
  double sigma() const { return parameterValue(2); }   //!< sigma
  double waveNumber() const { return parameterValue(3); }   //!< k0
  double phase() const { return parameterValue(4); }   //!< phi
  double evaluateAt(double x) const override;   //!< Value
  bool hasAnalyticalFourierTransformOnDomain() const override { return true; }
  protected:
  std::complex<double> lineTransform(double k) const override;
  std::complex<double> domainTransform(double k, double low, double high) const override;
  };

  //! @brief Hyperbolic secant: A sech((x - x0)/s). Parameters: amplitude, position, s (> 0).
  //! Transform: A pi s sech(pi s k/2) e^{-ik x0}.
  class HyperbolicSecant : public SignalFunction
  {
  public:
  HyperbolicSecant(double amplitude=1.0, double position=0.0, double scale=1.0);   //!< Function
  HyperbolicSecant * clone() const override { return new HyperbolicSecant(*this); }   //!< Copy
  double scale() const { return parameterValue(2); }   //!< s
  double evaluateAt(double x) const override;   //!< Value
  double integralInRange(double low, double high) const override;   //!< Exact integral
  protected:
  std::complex<double> lineTransform(double k) const override;
  };

  //! @brief Hermite function: A psi_n((x - x0)/s), psi_n(t) = H_n(t) e^{-t^2/2}/sqrt(2^n n! sqrt(pi)) (normalized:
  //! integral of psi_n^2 = 1). Parameters: amplitude, position, s (> 0), order n (fixed). Transform:
  //! A s sqrt(2 pi) (-i)^n psi_n(s k) e^{-ik x0} (the Hermite functions are eigenfunctions of the transform).
  class HermiteFunction : public SignalFunction
  {
  public:
  HermiteFunction(unsigned int order=0, double amplitude=1.0, double position=0.0, double scale=1.0);   //!< Function
  HermiteFunction * clone() const override { return new HermiteFunction(*this); }   //!< Copy
  double scale() const { return parameterValue(2); }   //!< s
  unsigned int order() const;   //!< Order n
  double evaluateAt(double x) const override;   //!< Value
  //! @brief psi_n(t), by the stable recurrence of the normalized functions. @param n order @param t t @return value
  static double psi(unsigned int n, double t);
  protected:
  std::complex<double> lineTransform(double k) const override;
  };

  //! @brief Voigt profile: convolution of a Gaussian of width sigma and a Cauchy (Lorentzian) of half width gamma, of
  //! area A: A Re w((x - x0 + i gamma)/(sigma sqrt 2))/(sigma sqrt(2 pi)), w the Faddeeva function. Parameters:
  //! amplitude (the area), position, sigma (> 0), gamma (>= 0). Transform: A e^{-ik x0} e^{-sigma^2 k^2/2 - gamma |k|}.
  class Voigt : public SignalFunction
  {
  public:
  Voigt(double amplitude=1.0, double position=0.0, double sigma=1.0, double gamma=1.0);   //!< Function
  Voigt * clone() const override { return new Voigt(*this); }   //!< Copy
  double sigma() const { return parameterValue(2); }   //!< sigma
  double gamma() const { return parameterValue(3); }   //!< gamma
  double evaluateAt(double x) const override;   //!< Value
  protected:
  std::complex<double> lineTransform(double k) const override;
  };

  //! @brief Heaviside step: A for x > x0, A/2 at x0, 0 below. Parameters: amplitude, position. Its transform on the
  //! line is a distribution (pi A delta(k) - i A/k): only the transform on a domain is given (exact).
  class HeavisideStep : public SignalFunction
  {
  public:
  HeavisideStep(double amplitude=1.0, double position=0.0);   //!< Function
  HeavisideStep * clone() const override { return new HeavisideStep(*this); }   //!< Copy
  double evaluateAt(double x) const override;   //!< Value
  double integralInRange(double low, double high) const override;   //!< Exact integral
  bool hasAnalyticalFourierTransform() const override { return false; }
  bool hasAnalyticalFourierTransformOnDomain() const override { return true; }
  protected:
  std::complex<double> domainTransform(double k, double low, double high) const override;
  };

  //!
  //! @brief Dirac comb: sum over m of A delta(x - x0 - m L). Parameters: weight A, position x0, period L (> 0).
  //!
  //! A distribution, not a function: evaluateAt() is 0, integralInRange() is A times the number of teeth in the range
  //! (half for a tooth on a bound). Its transform on a domain is the finite sum of A e^{-ik x_m} over the teeth (exact,
  //! half weight on the bounds), so that its Fourier series coefficients are c_n = (A/L) e^{-2 pi i n x0/L}; its
  //! transform on the line is the comb sum_n transformWeight(n) delta(k - 2 pi n/L), given by transformPeriod() and
  //! transformWeight().
  //!
  class DiracComb : public SignalFunction
  {
  public:
  DiracComb(double weight=1.0, double position=0.0, double period=1.0);   //!< Comb
  DiracComb * clone() const override { return new DiracComb(*this); }   //!< Copy
  double period() const { return parameterValue(2); }   //!< Period L
  double evaluateAt(double x) const override { (void) x; return 0.0; }   //!< 0 (a distribution)
  double integralInRange(double low, double high) const override;   //!< A times the number of teeth
  bool hasAnalyticalFourierTransform() const override { return false; }
  bool hasAnalyticalFourierTransformOnDomain() const override { return true; }
  //! @brief Period of the comb of the transform, 2 pi/L. @return period in k
  double transformPeriod() const;
  //! @brief Weight of the tooth n of the transform, (2 pi A/L) e^{-2 pi i n x0/L}. @param n index @return weight
  std::complex<double> transformWeight(int n) const;
  protected:
  std::complex<double> domainTransform(double k, double low, double high) const override;
  };

  } // namespace MATH
} // namespace Datum

#endif // DATUM_MATH_SignalFunctions
