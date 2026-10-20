#ifndef CAP_MATH_Exponential
#define CAP_MATH_Exponential
#include <cmath>
#include "ParameterizedFunction.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Exponential function f(x) = a exp(b x).
  //!
  //! a is the amplitude (value at x=0) and b the exponent coefficient (slope of ln f).
  //! With the default values a=1 and b=1, f(x) = exp(x). A negative b gives an exponential
  //! decay with decay constant -b, e.g., a exp(-x/tau) with b = -1/tau.
  //! evaluateAt() uses std::exp.
  //!
  //! Example:
  //! @code
  //!   CAP::MATH::Exponential f(2.0, -0.5);   // 2 exp(-x/2)
  //!   double y = f(2.0);                     // 2/e
  //! @endcode
  //! Parameters (see ParameterizedFunction::parameters()): 0 amplitude, 1 exponent.
  //!
  class Exponential : public ParameterizedFunction
  {
  public:

  //! @brief Exponential function a exp(b x). @param a amplitude (value at x=0) @param b coefficient of x in the exponent
  Exponential(double a=1.0, double b=1.0);
  //! @brief Copy constructor. @param source function to copy
  Exponential(const Exponential & source) = default;
  //! @brief Copy of the function (see ParameterizedFunction::clone()). @return new function (the caller owns it)
  Exponential * clone() const override { return new Exponential(*this); }
  //! @brief Assignment operator. @param rhs function to copy @return reference to this function
  Exponential & operator=(const Exponential & rhs) = default;
  //! @brief Destructor.
  virtual ~Exponential() {}

  //! @brief Value of the function at x. @param x value of the independent variable @return a exp(b x)
  //! @brief Keep the evaluateAt() versions of other ranks visible (they throw: the rank is 1).
  using ParameterizedFunction::evaluateAt;
  //! @brief Keep the integralInRange() versions of other ranks visible (they throw: the rank is 1).
  using ParameterizedFunction::integralInRange;

  double evaluateAt(double x) const override { return amplitude()*std::exp(exponent()*x); }

  //! @brief Analytical first derivative at x. @param x value of the independent variable @return a b exp(b x)
  double derivativeAt(double x) const override { return exponent()*amplitude()*std::exp(exponent()*x); }

  //!
  //! @brief Analytical integral from low to high: (a/b) (exp(b high) - exp(b low)), or a (high - low) if b = 0.
  //!
  //! Infinite bounds are allowed where the integral converges, e.g. high = +infinity with b < 0.
  //! @param low lower bound @param high upper bound
  //! @return integral of a exp(b x) dx from low to high (negative if low > high)
  //!
  double integralInRange(double low, double high) const override;

  //!
  //! @brief Analytical expectation value of x over [low, high] with weight a exp(b x).
  //!
  //! E[x] = low + d (1/(1 - exp(-b d)) - 1/(b d)), with d = high - low. This tends to (low + high)/2 for b -> 0,
  //! and is evaluated with a series when |b d| is small to avoid cancellation.
  //! One infinite bound is allowed where the integral converges: E[x] = low - 1/b for high = +infinity
  //! and b < 0, and high - 1/b for low = -infinity and b > 0.
  //! @param low lower bound @param high upper bound
  //! @return expectation value of x
  //! @throws MathException if the integral diverges or the amplitude is zero
  //!
  double expectationValue(double low, double high) const override;

  //!
  //! @brief Expectation value of x^power over [low, high] with weight a exp(b x).
  //!
  //! Analytical for power >= 0: with u the distance from the end of the range where the weight is
  //! largest, the moments of u are incomplete gamma functions, evaluated by series or by their
  //! complement so that no precision is lost; E[x^n] then follows from the binomial expansion.
  //! One infinite bound is allowed where the integral converges (as for expectationValue(low,high)).
  //! Negative powers use the numerical default of ParameterizedFunction (finite bounds, 0 outside the range).
  //! @param low lower bound @param high upper bound @param power power of x
  //! @return expectation value of x^power
  //! @throws MathException if the integral diverges or is zero, or if power < 0 and 0 is in the range
  //!
  double expectationValue(double low, double high, int power) const override;

  //! @brief Amplitude, value of the function at x=0. @return a
  double amplitude() const { return parameterValue(0); }
  //! @brief Coefficient of x in the exponent. @return b
  double exponent() const { return parameterValue(1); }
  //! @brief Set the amplitude. @param a new amplitude
  void setAmplitude(double a) { setParameterValue(0,a); }
  //! @brief Set the coefficient of x in the exponent. @param b new coefficient
  void setExponent(double b) { setParameterValue(1,b); }
  //! @brief Set both parameters. @param a amplitude @param b coefficient of x in the exponent
  void setParameters(double a, double b) { setParameterValue(0,a); setParameterValue(1,b); }

  ClassDef(Exponential,1)
  };

  } // namespace MATH
} // namespace CAP

#endif
