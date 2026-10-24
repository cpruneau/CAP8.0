#ifndef CAP_MATH_Normal
#define CAP_MATH_Normal
#include <cmath>
#include "ParameterizedFunction.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Gaussian (normal) function f(x) = A exp(-(x - mu)^2 / (2 sigma^2)).
  //!
  //!  - A, the amplitude, is the peak value f(mu);
  //!  - mu, the mean, is the position of the peak;
  //!  - sigma, the width, is the standard deviation (must be > 0).
  //!
  //! The area under the curve is A sigma sqrt(2 pi) (see integral()); with A = 1/(sigma sqrt(2 pi))
  //! the function is the normal probability density (see setNormalized()).
  //! evaluateAt() uses std::exp.
  //!
  //! Example:
  //! @code
  //!   CAP::MATH::Normal g(10.0, 1.0, 0.5);   // amplitude 10, mean 1, width 0.5
  //!   double peak = g(1.0);                  // 10
  //!   double y    = g(1.5);                  // 10 exp(-1/2), one sigma from the mean
  //!   g.setNormalized();                     // unit area: standard normal density shape
  //! @endcode
  //! Parameters (see ParameterizedFunction::parameters()): 0 amplitude, 1 mean, 2 width.
  //!
  class Normal : public ParameterizedFunction
  {
  public:

  //!
  //! @brief Gaussian function with the given amplitude, mean and width.
  //! @param amplitude peak value A @param mean position of the peak mu @param width standard deviation sigma (> 0)
  //! @throws MathException if width <= 0
  //!
  Normal(double amplitude=1.0, double mean=0.0, double width=1.0);
  //! @brief Copy constructor. @param source function to copy
  Normal(const Normal & source) = default;
  //! @brief Copy of the function (see ParameterizedFunction::clone()). @return new function (the caller owns it)
  Normal * clone() const override { return new Normal(*this); }
  //! @brief Assignment operator. @param rhs function to copy @return reference to this function
  Normal & operator=(const Normal & rhs) = default;
  //! @brief Destructor.
  virtual ~Normal() {}

  //! @brief Keep the evaluateAt() versions of other ranks visible (they throw: the rank is 1).
  using ParameterizedFunction::evaluateAt;
  //! @brief Keep the integralInRange() versions of other ranks visible (they throw: the rank is 1).
  using ParameterizedFunction::integralInRange;

  //!
  //! @brief Value of the function at x.
  //! @param x value of the independent variable
  //! @return A exp(-(x - mu)^2 / (2 sigma^2))
  //!
  double evaluateAt(double x) const override
  {
  double z = (x - mean())/width();
  return amplitude()*std::exp(-0.5*z*z);
  }

  //!
  //! @brief Analytical first derivative at x.
  //! @param x value of the independent variable
  //! @return -(x - mu)/sigma^2 f(x)
  //!
  double derivativeAt(double x) const override
  {
  double z = (x - mean())/width();
  return -(z/width())*amplitude()*std::exp(-0.5*z*z);
  }

  //!
  //! @brief Analytical integral from low to high, using the error function.
  //!
  //! A sigma sqrt(pi/2) [erf((high - mu)/(sigma sqrt 2)) - erf((low - mu)/(sigma sqrt 2))]; erfc is used
  //! when both bounds lie in the same tail, to keep full precision. Infinite bounds are allowed.
  //! @param low lower bound @param high upper bound
  //! @return integral of f(x) dx from low to high (negative if low > high)
  //!
  double integralInRange(double low, double high) const override;

  //!
  //! @brief Analytical expectation value of x over [low, high] with weight f(x).
  //!
  //! mu + A sigma^2 [exp(-z_low^2/2) - exp(-z_high^2/2)] / integralInRange(low, high), with z = (x - mu)/sigma.
  //! Equal to mu for a range symmetric about mu, including (-infinity, +infinity).
  //! @param low lower bound @param high upper bound
  //! @return expectation value of x
  //! @throws MathException if the integral over the range is zero (amplitude zero, or range so far in a tail that it underflows)
  //!
  double expectationValue(double low, double high) const override;

  //!
  //! @brief Expectation value of x^power over [low, high] with weight f(x).
  //!
  //! Analytical for power >= 0: moments J_j of the truncated standard normal follow from the recursion
  //! J_j = (j-1) J_(j-2) - [z^(j-1) exp(-z^2/2)] between the bounds, and E[x^n] from the binomial
  //! expansion of (mu + sigma z)^n. Infinite bounds are allowed (e.g. E[x^2] over the full range is mu^2 + sigma^2).
  //! Negative powers use the numerical default of ParameterizedFunction (finite bounds, 0 outside the range).
  //! @param low lower bound @param high upper bound @param power power of x
  //! @return expectation value of x^power
  //! @throws MathException if the integral over the range is zero, or if power < 0 and 0 is in the range
  //!
  double expectationValue(double low, double high, int power) const override;

  //! @name Getters
  //! @{

  //! @brief Amplitude, peak value of the function at x = mean. @return A
  double amplitude() const { return parameterValue(0); }
  //! @brief Mean, position of the peak. @return mu
  double mean() const { return parameterValue(1); }
  //! @brief Width, standard deviation of the gaussian. @return sigma
  double width() const { return parameterValue(2); }
  //! @brief Area under the curve from -infinity to +infinity. @return A sigma sqrt(2 pi)
  double integral() const;
  //! @brief Full width at half maximum. @return 2 sqrt(2 ln 2) sigma (about 2.3548 sigma)
  double fwhm() const;

  //! @}

  //! @name Setters
  //! @{

  //! @brief Set the amplitude (peak value). @param amplitude new amplitude A
  void setAmplitude(double amplitude) { setParameterValue(0,amplitude); }
  //! @brief Set the mean (position of the peak). @param mean new mean mu
  void setMean(double mean) { setParameterValue(1,mean); }
  //!
  //! @brief Set the width (standard deviation).
  //! @param width new width sigma (> 0)
  //! @throws MathException if width <= 0 (the function is then unchanged)
  //!
  void setWidth(double width);
  //!
  //! @brief Set amplitude, mean and width together.
  //! @param amplitude peak value A @param mean position of the peak mu @param width standard deviation sigma (> 0)
  //! @throws MathException if width <= 0 (the function is then unchanged)
  //!
  void setParameters(double amplitude, double mean, double width);
  //! @brief Set the amplitude so that the area under the curve is 1 (normal probability density): A = 1/(sigma sqrt(2 pi)).
  void setNormalized();

  //! @}

  };

  } // namespace MATH
} // namespace CAP

#endif
