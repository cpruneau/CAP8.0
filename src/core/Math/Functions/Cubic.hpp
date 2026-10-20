#ifndef CAP_MATH_Cubic
#define CAP_MATH_Cubic
#include "Polynomial.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Cubic function f(x) = c0 + c1 x + c2 x^2 + c3 x^3: a Polynomial of fixed degree 3.
  //!
  //! Setters inherited from Polynomial that would change the degree throw MathException.
  //!
  //! Example:
  //! @code
  //!   CAP::MATH::Cubic f(1.0, 0.0, -3.0, 1.0);   // 1 - 3x^2 + x^3
  //!   double y  = f(2.0);                        // -3
  //!   double xi = f.inflectionX();               // 1
  //! @endcode
  //!
  class Cubic : public Polynomial
  {
  public:

  //! @brief Cubic function c0 + c1 x + c2 x^2 + c3 x^3.
  //! @param c0 constant term @param c1 coefficient of x @param c2 coefficient of x^2 @param c3 coefficient of x^3
  Cubic(double c0=0.0, double c1=0.0, double c2=0.0, double c3=0.0);
  //! @brief Copy constructor. @param source function to copy
  Cubic(const Cubic & source) = default;
  //! @brief Copy of the function (see ParameterizedFunction::clone()). @return new function (the caller owns it)
  Cubic * clone() const override { return new Cubic(*this); }
  //! @brief Assignment operator. @param rhs function to copy @return reference to this function
  Cubic & operator=(const Cubic & rhs) = default;
  //! @brief Destructor.
  virtual ~Cubic() {}

  //! @brief Value of the function at x (Horner's method). @param x value of the independent variable @return c0 + c1 x + c2 x^2 + c3 x^3
  //! @brief Keep the evaluateAt() versions of other ranks visible (they throw: the rank is 1).
  using Polynomial::evaluateAt;

  double evaluateAt(double x) const override
  {
  return parameterValue(0) + x*(parameterValue(1) + x*(parameterValue(2) + x*parameterValue(3)));
  }

  //! @brief Analytical first derivative at x. @param x value of the independent variable @return c1 + 2 c2 x + 3 c3 x^2
  double derivativeAt(double x) const override
  {
  return parameterValue(1) + x*(2.0*parameterValue(2) + x*3.0*parameterValue(3));
  }

  //! @brief Set the four coefficients.
  //! @param c0 constant term @param c1 coefficient of x @param c2 coefficient of x^2 @param c3 coefficient of x^3
  void setCoefficients(double c0, double c1, double c2, double c3)
  {
  setParameterValue(0,c0); setParameterValue(1,c1); setParameterValue(2,c2); setParameterValue(3,c3);
  }
  using Polynomial::setCoefficients; // keep the vector version visible

  //!
  //! @brief Position of the inflection point, where the second derivative vanishes.
  //! @return -c2/(3 c3)
  //! @throws MathException if c3 is zero
  //!
  double inflectionX() const;

  ClassDef(Cubic,1)
  };

  } // namespace MATH
} // namespace CAP

#endif
