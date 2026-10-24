#ifndef CAP_MATH_LogNormal
#define CAP_MATH_LogNormal
#include <cmath>
#include "ParameterizedFunction.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Log-normal distribution f(x) = A / (x sigma sqrt(2 pi)) exp(-(ln x - mu)^2 / (2 sigma^2)), for x > 0.
  //!
  //! A variable x is log-normally distributed when ln x follows a normal distribution of mean mu
  //! and standard deviation sigma.
  //!  - A, the amplitude, is the area under the curve (A = 1 gives the probability density);
  //!  - mu, the mean, is the mean of ln x (the median of x is exp(mu));
  //!  - sigma, the width, is the standard deviation of ln x (must be > 0).
  //!
  //! The function is zero for x <= 0. Note that, unlike Normal, the amplitude is the area and not
  //! the peak value, since the 1/x factor makes the peak height depend on mu and sigma.
  //! evaluateAt() uses std::exp and std::log.
  //!
  //! Example:
  //! @code
  //!   CAP::MATH::LogNormal f(1.0, 0.0, 0.5);   // probability density of x with ln x ~ N(0, 0.5^2)
  //!   double y  = f(1.0);                      // 1/(0.5 sqrt(2 pi))
  //!   double xm = f.mode();                    // exp(-0.25), position of the peak
  //! @endcode
  //! Parameters (see ParameterizedFunction::parameters()): 0 amplitude, 1 mean, 2 width.
  //!
  class LogNormal : public ParameterizedFunction
  {
  public:

  //!
  //! @brief Log-normal distribution with the given amplitude (area), mean and width of ln x.
  //! @param amplitude area under the curve A @param mean mean mu of ln x @param width standard deviation sigma of ln x (> 0)
  //! @throws MathException if width <= 0
  //!
  LogNormal(double amplitude=1.0, double mean=0.0, double width=1.0);
  //! @brief Copy constructor. @param source function to copy
  LogNormal(const LogNormal & source) = default;
  //! @brief Copy of the function (see ParameterizedFunction::clone()). @return new function (the caller owns it)
  LogNormal * clone() const override { return new LogNormal(*this); }
  //! @brief Assignment operator. @param rhs function to copy @return reference to this function
  LogNormal & operator=(const LogNormal & rhs) = default;
  //! @brief Destructor.
  virtual ~LogNormal() {}

  //! @brief Keep the evaluateAt() versions of other ranks visible (they throw: the rank is 1).
  using ParameterizedFunction::evaluateAt;
  //! @brief Keep the integralInRange() versions of other ranks visible (they throw: the rank is 1).
  using ParameterizedFunction::integralInRange;

  //!
  //! @brief Value of the distribution at x.
  //! @param x value of the variable
  //! @return A / (x sigma sqrt(2 pi)) exp(-(ln x - mu)^2 / (2 sigma^2)) for x > 0, and 0 for x <= 0
  //!
  double evaluateAt(double x) const override;

  //!
  //! @brief Analytical first derivative at x.
  //! @param x value of the variable
  //! @return -f(x) (1 + (ln x - mu)/sigma^2) / x for x > 0, and 0 for x <= 0
  //!
  double derivativeAt(double x) const override;

  //!
  //! @brief Analytical integral from low to high, using the error function.
  //!
  //! (A/2) [erf((ln high - mu)/(sigma sqrt 2)) - erf((ln low - mu)/(sigma sqrt 2))], where bounds <= 0 are
  //! replaced by 0 since the function vanishes there; erfc is used when both bounds lie in the same tail.
  //! Infinite bounds are allowed.
  //! @param low lower bound @param high upper bound
  //! @return integral of f(x) dx from low to high (negative if low > high)
  //!
  double integralInRange(double low, double high) const override;

  //!
  //! @brief Analytical expectation value of x over [low, high] with weight f(x).
  //!
  //! exp(mu + sigma^2/2) [erf(w_high) - erf(w_low)] / [erf(z_high) - erf(z_low)], with
  //! z = (ln x - mu)/(sigma sqrt 2) and w = (ln x - mu - sigma^2)/(sigma sqrt 2); bounds <= 0 are replaced by 0.
  //! Over (0, +infinity) it equals expectationValue().
  //! @param low lower bound @param high upper bound
  //! @return expectation value of x
  //! @throws MathException if the integral over the range is zero
  //!
  double expectationValue(double low, double high) const override;

  //!
  //! @brief Analytical expectation value of x^power over [low, high] with weight f(x), for any integer power.
  //!
  //! exp(n mu + n^2 sigma^2/2) [erf(w_high) - erf(w_low)] / [erf(z_high) - erf(z_low)], with
  //! z = (ln x - mu)/(sigma sqrt 2) and w = (ln x - mu - n sigma^2)/(sigma sqrt 2); bounds <= 0 are replaced by 0.
  //! Negative powers are allowed even if the range starts at 0, since f vanishes faster than any power there.
  //! Over (0, +infinity) it gives the moments exp(n mu + n^2 sigma^2/2) of the log-normal distribution.
  //! @param low lower bound @param high upper bound @param power power n of x
  //! @return expectation value of x^power
  //! @throws MathException if the integral over the range is zero
  //!
  double expectationValue(double low, double high, int power) const override;

  //! @name Getters
  //! @{

  //! @brief Amplitude, area under the curve. @return A
  double amplitude() const { return parameterValue(0); }
  //! @brief Mean of ln x. @return mu
  double mean() const { return parameterValue(1); }
  //! @brief Width, standard deviation of ln x. @return sigma
  double width() const { return parameterValue(2); }
  //! @brief Area under the curve from 0 to +infinity (same as amplitude()). @return A
  double integral() const { return amplitude(); }
  //! @brief Median of x. @return exp(mu)
  double median() const { return std::exp(mean()); }
  //! @brief Mode of x, position of the peak. @return exp(mu - sigma^2)
  double mode() const { return std::exp(mean() - width()*width()); }
  //! @brief Expectation value of x over the full range (0, +infinity). @return exp(mu + sigma^2/2)
  double expectationValue() const { return std::exp(mean() + 0.5*width()*width()); }
  //! @brief Variance of x. @return (exp(sigma^2) - 1) exp(2 mu + sigma^2)
  double variance() const { double s2 = width()*width(); return (std::exp(s2) - 1.0)*std::exp(2.0*mean() + s2); }

  //! @}

  //! @name Setters
  //! @{

  //! @brief Set the amplitude (area under the curve). @param amplitude new amplitude A
  void setAmplitude(double amplitude) { setParameterValue(0,amplitude); }
  //! @brief Set the mean of ln x. @param mean new mean mu
  void setMean(double mean) { setParameterValue(1,mean); }
  //!
  //! @brief Set the width (standard deviation of ln x).
  //! @param width new width sigma (> 0)
  //! @throws MathException if width <= 0 (the function is then unchanged)
  //!
  void setWidth(double width);
  //!
  //! @brief Set amplitude, mean and width together.
  //! @param amplitude area under the curve A @param mean mean mu of ln x @param width standard deviation sigma of ln x (> 0)
  //! @throws MathException if width <= 0 (the function is then unchanged)
  //!
  void setParameters(double amplitude, double mean, double width);
  //! @brief Set the amplitude to 1, so that the function is the probability density.
  void setNormalized() { setParameterValue(0,1.0); }

  //! @}

  };

  } // namespace MATH
} // namespace CAP

#endif
