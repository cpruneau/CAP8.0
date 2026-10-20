#ifndef CAP_MATH_Quartic
#define CAP_MATH_Quartic
#include "Polynomial.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Quartic function f(x) = c0 + c1 x + c2 x^2 + c3 x^3 + c4 x^4: a Polynomial of fixed degree 4.
  //!
  //! Setters inherited from Polynomial that would change the degree throw MathException.
  //!
  //! Example:
  //! @code
  //!   CAP::MATH::Quartic f(0.0, 0.0, -2.0, 0.0, 1.0);   // x^4 - 2x^2
  //!   double y = f(2.0);                                // 8
  //! @endcode
  //!
  class Quartic : public Polynomial
  {
  public:

  //! @brief Quartic function c0 + c1 x + c2 x^2 + c3 x^3 + c4 x^4.
  //! @param c0 constant term @param c1 coefficient of x @param c2 coefficient of x^2
  //! @param c3 coefficient of x^3 @param c4 coefficient of x^4
  Quartic(double c0=0.0, double c1=0.0, double c2=0.0, double c3=0.0, double c4=0.0);
  //! @brief Copy constructor. @param source function to copy
  Quartic(const Quartic & source) = default;
  //! @brief Copy of the function (see ParameterizedFunction::clone()). @return new function (the caller owns it)
  Quartic * clone() const override { return new Quartic(*this); }
  //! @brief Assignment operator. @param rhs function to copy @return reference to this function
  Quartic & operator=(const Quartic & rhs) = default;
  //! @brief Destructor.
  virtual ~Quartic() {}

  //! @brief Value of the function at x (Horner's method). @param x value of the independent variable @return c0 + c1 x + c2 x^2 + c3 x^3 + c4 x^4
  //! @brief Keep the evaluateAt() versions of other ranks visible (they throw: the rank is 1).
  using Polynomial::evaluateAt;

  double evaluateAt(double x) const override
  {
  return parameterValue(0) + x*(parameterValue(1) + x*(parameterValue(2) + x*(parameterValue(3) + x*parameterValue(4))));
  }

  //! @brief Analytical first derivative at x. @param x value of the independent variable @return c1 + 2 c2 x + 3 c3 x^2 + 4 c4 x^3
  double derivativeAt(double x) const override
  {
  return parameterValue(1) + x*(2.0*parameterValue(2) + x*(3.0*parameterValue(3) + x*4.0*parameterValue(4)));
  }

  //! @brief Set the five coefficients.
  //! @param c0 constant term @param c1 coefficient of x @param c2 coefficient of x^2
  //! @param c3 coefficient of x^3 @param c4 coefficient of x^4
  void setCoefficients(double c0, double c1, double c2, double c3, double c4)
  {
  setParameterValue(0,c0); setParameterValue(1,c1); setParameterValue(2,c2); setParameterValue(3,c3); setParameterValue(4,c4);
  }
  using Polynomial::setCoefficients; // keep the vector version visible

  ClassDef(Quartic,1)
  };

  } // namespace MATH
} // namespace CAP

#endif
