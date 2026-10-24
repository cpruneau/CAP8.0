#ifndef CAP_MATH_Parabolic
#define CAP_MATH_Parabolic
#include "Polynomial.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Parabolic (quadratic) function f(x) = c0 + c1 x + c2 x^2: a Polynomial of fixed degree 2.
  //!
  //! Setters inherited from Polynomial that would change the degree throw MathException.
  //!
  //! Example:
  //! @code
  //!   CAP::MATH::Parabolic f(1.0, -4.0, 2.0);   // 1 - 4x + 2x^2
  //!   double y  = f(3.0);                       // 7
  //!   double xe = f.extremumX();                // 1  (minimum, since c2>0)
  //!   double ye = f.extremumValue();            // -1
  //! @endcode
  //!
  class Parabolic : public Polynomial
  {
  public:

  //! @brief Quadratic function c0 + c1 x + c2 x^2. @param c0 constant term @param c1 linear coefficient @param c2 quadratic coefficient
  Parabolic(double c0=0.0, double c1=0.0, double c2=0.0);
  //! @brief Copy constructor. @param source function to copy
  Parabolic(const Parabolic & source) = default;
  //! @brief Copy of the function (see ParameterizedFunction::clone()). @return new function (the caller owns it)
  Parabolic * clone() const override { return new Parabolic(*this); }
  //! @brief Assignment operator. @param rhs function to copy @return reference to this function
  Parabolic & operator=(const Parabolic & rhs) = default;
  //! @brief Destructor.
  virtual ~Parabolic() {}

  //! @brief Value of the function at x. @param x value of the independent variable @return c0 + c1 x + c2 x^2
  //! @brief Keep the evaluateAt() versions of other ranks visible (they throw: the rank is 1).
  using Polynomial::evaluateAt;

  double evaluateAt(double x) const override { return parameterValue(0) + x*(parameterValue(1) + x*parameterValue(2)); }

  //! @brief Analytical first derivative at x. @param x value of the independent variable @return c1 + 2 c2 x
  double derivativeAt(double x) const override { return parameterValue(1) + 2.0*parameterValue(2)*x; }

  //! @brief Set the three coefficients. @param c0 constant term @param c1 linear coefficient @param c2 quadratic coefficient
  void setCoefficients(double c0, double c1, double c2) { setParameterValue(0,c0); setParameterValue(1,c1); setParameterValue(2,c2); }
  using Polynomial::setCoefficients; // keep the vector version visible

  //!
  //! @brief Position of the extremum (vertex) of the parabola: a minimum if c2>0, a maximum if c2<0.
  //! @return -c1/(2 c2)
  //! @throws MathException if c2 is zero
  //!
  double extremumX() const;

  //!
  //! @brief Value of the function at its extremum.
  //! @return f(extremumX())
  //! @throws MathException if c2 is zero
  //!
  double extremumValue() const { return evaluateAt(extremumX()); }

  };

  } // namespace MATH
} // namespace CAP

#endif
