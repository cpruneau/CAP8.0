#ifndef CAP_MATH_Polynomial
#define CAP_MATH_Polynomial
#include <vector>
#include <initializer_list>
#include <ostream>
#include "Aliases.hpp"
#include "ParameterizedFunction.hpp"
#include "ClassDef.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Polynomial of one variable, p(x) = c0 + c1 x + c2 x^2 + ... + cn x^n.
  //!
  //! Coefficients are stored as the parameters of the function (see ParameterizedFunction::parameters()),
  //! named c0, c1, ..., in increasing powers of x: coefficient(k), i.e. parameter k, multiplies x^k.
  //! A polynomial with no coefficients is the zero polynomial. evaluateAt() uses
  //! Horner's method, which needs n multiplications and n additions for degree n.
  //!
  //! Example:
  //! @code
  //!   CAP::MATH::Polynomial p({1.0, -2.0, 3.0});   // 1 - 2x + 3x^2
  //!   double y = p.evaluateAt(2.0);                // 9
  //!   p.setCoefficient(3, 1.0);                    // now 1 - 2x + 3x^2 + x^3
  //!   unsigned int n = p.degree();                 // 3
  //! @endcode
  //!
  class Polynomial : public ParameterizedFunction
  {
  protected:

  bool _fixedDegree;                   //!< If true, setters may not change the degree (used by Constant, Linear, Parabolic)

  //! @brief Tag type selecting the fixed-degree constructor (a distinct type, so that a brace list such as {3,4} cannot select it).
  struct FixedDegree {};

  //!
  //! @brief Constructor for derived classes of fixed degree: all coefficients zero, degree locked.
  //!
  //! Setters that would change the degree then throw MathException.
  //! @param tag selects this constructor (FixedDegree())
  //! @param degree degree n of the polynomial (n+1 coefficients)
  //!
  Polynomial(FixedDegree tag, unsigned int degree);

  //!
  //! @brief Throw if the degree is fixed and newNCoefficients differs from the current number of coefficients.
  //! @param newNCoefficients number of coefficients requested @param source name of the calling function, reported in the exception
  //! @throws MathException if the degree is fixed and would change
  //!
  void checkDegreeChange(unsigned int newNCoefficients, const char * source) const;

  //! @brief Name of the parameter holding the coefficient of x^k. @param k power of x @return "ck"
  static String coefficientName(unsigned int k);

  //! @brief Parameters c0 ... cn holding the given coefficients. @param coefficients coefficients in increasing powers of x @return parameters
  static std::vector<Parameter> coefficientParameters(const std::vector<double> & coefficients);

  //! @brief Resize the list of coefficients, adding zero coefficients if needed (no fixed-degree check). @param n new number of coefficients
  void resizeCoefficients(unsigned int n);

  public:

  //! @name Construction and assignment
  //! @{

  //! @brief Default constructor: zero polynomial (no coefficients).
  Polynomial();

  //!
  //! @brief Polynomial of the given degree with all coefficients set to zero.
  //! @param degree degree n of the polynomial (n+1 coefficients)
  //!
  explicit Polynomial(unsigned int degree);

  //!
  //! @brief Polynomial with the given coefficients.
  //! @param coefficients coefficients c0 ... cn, in increasing powers of x
  //!
  Polynomial(const std::vector<double> & coefficients);

  //!
  //! @brief Polynomial with the coefficients given as a brace list, e.g. Polynomial({1, -2, 3}) or Polynomial p{4}.
  //!
  //! Ensures that a brace list is always read as coefficients: Polynomial({4}) is the constant 4,
  //! whereas Polynomial(4) is a polynomial of degree 4 with zero coefficients.
  //! @param coefficients coefficients c0 ... cn, in increasing powers of x
  //!
  Polynomial(std::initializer_list<double> coefficients);

  //! @brief Copy constructor. @param source polynomial to copy
  Polynomial(const Polynomial & source);
  //! @brief Copy of the function (see ParameterizedFunction::clone()). @return new function (the caller owns it)
  Polynomial * clone() const override { return new Polynomial(*this); }

  //! @brief Destructor.
  virtual ~Polynomial() {}

  //!
  //! @brief Assignment operator: copies the coefficients.
  //!
  //! The fixed-degree property belongs to the object and is not copied.
  //! @param rhs polynomial to copy @return reference to this polynomial
  //! @throws MathException if this polynomial has a fixed degree and rhs has a different number of coefficients
  //!
  Polynomial & operator=(const Polynomial & rhs);

  //! @}

  //! @brief Keep the evaluateAt() versions of other ranks visible (they throw: the rank is 1).
  using ParameterizedFunction::evaluateAt;
  //! @brief Keep the integralInRange() versions of other ranks visible (they throw: the rank is 1).
  using ParameterizedFunction::integralInRange;

  //!
  //! @brief Value of the polynomial at x, computed with Horner's method.
  //! @param x value of the independent variable
  //! @return p(x); 0 for the zero polynomial
  //!
  double evaluateAt(double x) const override;

  //!
  //! @brief Analytical first derivative at x: c1 + 2 c2 x + ... + n cn x^(n-1), computed with Horner's method.
  //! @param x value of the independent variable
  //! @return p'(x); 0 for a constant or for the zero polynomial
  //!
  double derivativeAt(double x) const override;

  //!
  //! @brief Analytical integral from low to high: P(high) - P(low), with P(x) = c0 x + c1 x^2/2 + ... + cn x^(n+1)/(n+1).
  //!
  //! Inherited by Constant, Linear, Parabolic, Cubic and Quartic.
  //! @param low lower bound @param high upper bound
  //! @return integral of p(x) dx from low to high (negative if low > high)
  //!
  double integralInRange(double low, double high) const override;

  //!
  //! @brief Antiderivative P(x) = c0 x + c1 x^2/2 + ... + cn x^(n+1)/(n+1), with P(0) = 0, computed with Horner's method.
  //! @param x value of the independent variable @return P(x)
  //!
  double antiderivativeAt(double x) const;

  //!
  //! @brief Analytical expectation value of x over [low, high] with weight p(x).
  //!
  //! Ratio of Q(high) - Q(low), with Q(x) = c0 x^2/2 + c1 x^3/3 + ... + cn x^(n+2)/(n+2), to the integral
  //! of p over the range. Inherited by Constant, Linear, Parabolic, Cubic and Quartic.
  //! @param low lower bound @param high upper bound
  //! @return expectation value of x
  //! @throws MathException if the integral of p over the range is zero
  //!
  double expectationValue(double low, double high) const override;

  //!
  //! @brief Analytical expectation value of x^power over [low, high] with weight p(x).
  //!
  //! Integrates c_k x^(k+power) term by term (ln|x| for the term x^-1). Inherited by Constant,
  //! Linear, Parabolic, Cubic and Quartic.
  //! @param low lower bound @param high upper bound @param power power of x (negative only if 0 is outside the range)
  //! @return expectation value of x^power
  //! @throws MathException if the integral of p over the range is zero, or if power < 0 and 0 is in the range
  //!
  double expectationValue(double low, double high, int power) const override;

  //! @name Getters
  //! @{

  //!
  //! @brief Degree of the polynomial, i.e., number of coefficients minus one.
  //!
  //! The degree is set by the number of stored coefficients, even if the highest ones are zero.
  //! @return degree (0 for a constant or for the zero polynomial)
  //!
  unsigned int degree() const { return _parameters.empty() ? 0 : _parameters.size()-1; }

  //! @brief Number of stored coefficients (degree()+1, or 0 for the zero polynomial). @return number of coefficients
  unsigned int nCoefficients() const { return _parameters.size(); }

  //!
  //! @brief Coefficient of x^k.
  //! @param k power of x
  //! @return coefficient c_k; 0 if k>degree()
  //!
  double coefficient(unsigned int k) const { return k<_parameters.size() ? _parameters[k].value() : 0.0; }

  //! @brief Values of all coefficients, in increasing powers of x. @return vector of coefficients (a copy)
  std::vector<double> coefficients() const;

  //! @brief Whether the degree of this polynomial is fixed (true for Constant, Linear, Parabolic). @return true if the degree cannot change
  bool hasFixedDegree() const { return _fixedDegree; }

  //! @}

  //! @name Setters
  //! For a polynomial of fixed degree (Constant, Linear, Parabolic), a setter that would change
  //! the degree throws MathException and leaves the polynomial unchanged.
  //! @{

  //!
  //! @brief Set the coefficient of x^k.
  //!
  //! If k is larger than degree(), the polynomial is extended with zero coefficients up to x^k
  //! (not allowed for a polynomial of fixed degree).
  //! @param k power of x
  //! @param value new value of the coefficient
  //! @throws MathException if the degree is fixed and k>degree()
  //!
  void setCoefficient(unsigned int k, double value);

  //! @brief Replace all coefficients. @param coefficients coefficients c0 ... cn, in increasing powers of x
  //! @throws MathException if the degree is fixed and the number of coefficients differs
  void setCoefficients(const std::vector<double> & coefficients);

  //!
  //! @brief Change the degree: higher coefficients are removed, or new ones are added with value zero.
  //! @param degree new degree n (n+1 coefficients)
  //! @throws MathException if the degree is fixed and differs from the current one
  //!
  void setDegree(unsigned int degree);

  //!
  //! @brief Make this the zero polynomial: coefficients are removed, or set to zero if the degree is fixed.
  //!
  void clear() override;

  //! @}

  //! @brief Print the polynomial to std::cout.
  virtual void print() const override;

  //!
  //! @brief Write the polynomial as c0 + c1*x + c2*x^2 + ... to a stream.
  //! @param out output stream @param p polynomial @return the output stream
  //!
  friend std::ostream& operator<<(std::ostream& out, const Polynomial & p);

  ClassDef(Polynomial,1)
  };

  //! @relates Polynomial
  //! @brief Write the polynomial as c0 + c1*x + c2*x^2 + ... to a stream. @param out output stream @param p polynomial @return the output stream
  std::ostream& operator<<(std::ostream& out, const Polynomial & p);

  } // namespace MATH
} // namespace CAP

#endif
