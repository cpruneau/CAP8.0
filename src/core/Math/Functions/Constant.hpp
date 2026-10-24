#ifndef CAP_MATH_Constant
#define CAP_MATH_Constant
#include "Polynomial.hpp"
#include "ClassDef.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Constant function f(x) = c0: a Polynomial of fixed degree 0.
  //!
  //! Setters inherited from Polynomial that would change the degree throw MathException.
  //!
  //! Example:
  //! @code
  //!   CAP::MATH::Constant f(2.5);
  //!   double y = f(10.0);   // 2.5
  //! @endcode
  //!
  class Constant : public Polynomial
  {
  public:

  //! @brief Constant function of the given value. @param c0 value of the function
  Constant(double c0=0.0);
  //! @brief Copy constructor. @param source function to copy
  Constant(const Constant & source) = default;
  //! @brief Copy of the function (see ParameterizedFunction::clone()). @return new function (the caller owns it)
  Constant * clone() const override { return new Constant(*this); }
  //! @brief Assignment operator. @param rhs function to copy @return reference to this function
  Constant & operator=(const Constant & rhs) = default;
  //! @brief Destructor.
  virtual ~Constant() {}

  //! @brief Value of the function, independent of x. @return c0
  //! @brief Keep the evaluateAt() versions of other ranks visible (they throw: the rank is 1).
  using Polynomial::evaluateAt;

  double evaluateAt(double) const override { return parameterValue(0); }

  //! @brief Analytical first derivative, zero everywhere. @return 0
  double derivativeAt(double) const override { return 0.0; }

  //! @brief Value of the constant. @return c0
  double value() const { return parameterValue(0); }
  //! @brief Set the value of the constant. @param c0 new value
  void setValue(double c0) { setParameterValue(0,c0); }

  ClassDef(Constant,1)
  };

  } // namespace MATH
} // namespace CAP

#endif
