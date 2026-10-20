#ifndef CAP_MATH_Linear
#define CAP_MATH_Linear
#include "Polynomial.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Linear function f(x) = c0 + c1 x: a Polynomial of fixed degree 1.
  //!
  //! c0 is the intercept and c1 the slope. Setters inherited from Polynomial that would
  //! change the degree throw MathException.
  //!
  //! Example:
  //! @code
  //!   CAP::MATH::Linear f(1.0, 2.0);   // 1 + 2x
  //!   double y = f(3.0);               // 7
  //!   double x0 = f.root();            // -0.5
  //! @endcode
  //!
  class Linear : public Polynomial
  {
  public:

  //! @brief Linear function c0 + c1 x. @param c0 intercept @param c1 slope
  Linear(double c0=0.0, double c1=0.0);
  //! @brief Copy constructor. @param source function to copy
  Linear(const Linear & source) = default;
  //! @brief Copy of the function (see ParameterizedFunction::clone()). @return new function (the caller owns it)
  Linear * clone() const override { return new Linear(*this); }
  //! @brief Assignment operator. @param rhs function to copy @return reference to this function
  Linear & operator=(const Linear & rhs) = default;
  //! @brief Destructor.
  virtual ~Linear() {}

  //! @brief Value of the function at x. @param x value of the independent variable @return c0 + c1 x
  //! @brief Keep the evaluateAt() versions of other ranks visible (they throw: the rank is 1).
  using Polynomial::evaluateAt;

  double evaluateAt(double x) const override { return parameterValue(0) + parameterValue(1)*x; }

  //! @brief Analytical first derivative, the slope (independent of x). @return c1
  double derivativeAt(double) const override { return parameterValue(1); }

  //! @brief Intercept, value at x=0. @return c0
  double intercept() const { return parameterValue(0); }
  //! @brief Slope. @return c1
  double slope() const { return parameterValue(1); }
  //! @brief Set the intercept. @param c0 new intercept
  void setIntercept(double c0) { setParameterValue(0,c0); }
  //! @brief Set the slope. @param c1 new slope
  void setSlope(double c1) { setParameterValue(1,c1); }
  //! @brief Set both coefficients. @param c0 intercept @param c1 slope
  void setCoefficients(double c0, double c1) { setParameterValue(0,c0); setParameterValue(1,c1); }
  using Polynomial::setCoefficients; // keep the vector version visible

  //!
  //! @brief Value of x where the function is zero.
  //! @return -c0/c1
  //! @throws MathException if the slope is zero
  //!
  double root() const;

  ClassDef(Linear,1)
  };

  } // namespace MATH
} // namespace CAP

#endif
