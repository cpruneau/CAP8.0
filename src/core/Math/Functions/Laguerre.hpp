#ifndef CAP_MATH_Laguerre
#define CAP_MATH_Laguerre
#include <vector>
#include "Aliases.hpp"
#include "Function.hpp"
#include "ClassDef.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Generalized (associated) Laguerre polynomial L_n^(alpha)(x) as a Function of rank 1.
  //!
  //! With alpha = 0 (the default) these are the ordinary Laguerre polynomials L_n(x):
  //! L_0 = 1, L_1 = 1 - x, L_2 = (x^2 - 4x + 2)/2, ...
  //!
  //! - Value: three-term recurrence (k+1) L_{k+1} = (2k+1+alpha-x) L_k - (k+alpha) L_{k-1}, with L_0 = 1 and
  //!   L_1 = 1 + alpha - x (stable, n steps).
  //! - Derivative: d/dx L_n^(alpha) = -L_{n-1}^(alpha+1).
  //! - Integral: an antiderivative of L_n^(alpha) is -L_{n+1}^(alpha-1); when its values at the two bounds nearly cancel
  //!   (short intervals), Gauss-Legendre quadrature with n/2+1 points is used instead (exact for a polynomial of degree n).
  //!
  //! The polynomials are defined for any real alpha; they are orthogonal on [0, infinity) with weight
  //! x^alpha e^(-x) when alpha > -1 (see isOrthogonal(), weightAt() and norm()).
  //!
  //! Example:
  //! @code
  //!   using namespace CAP::MATH;
  //!   Laguerre l3(3);                       // L_3(x) = (-x^3 + 9x^2 - 18x + 6)/6
  //!   Laguerre a(2, 0.5);                   // L_2^(1/2)(x)
  //!   double y = l3.evaluateAt(1.0);        // (-1 + 9 - 18 + 6)/6 = -2/3
  //!   double d = l3.derivativeAt(1.0);      // -L_2^(1)(1)
  //!   double s = a.integralInRange(0.0, 2.0);
  //! @endcode
  //!
  class Laguerre : public Function
  {
  public:

  //!
  //! @brief Constructor (also the default constructor: L_0 = 1).
  //! @param n degree of the polynomial @param alpha parameter alpha (0 for the ordinary Laguerre polynomials)
  //! @throws MathException if alpha is not finite
  //!
  Laguerre(unsigned int n=0, double alpha=0.0);
  //! @brief Copy constructor. @param source polynomial to copy
  Laguerre(const Laguerre & source);
  //! @brief Assignment operator. @param rhs polynomial to copy @return reference to this polynomial
  Laguerre & operator=(const Laguerre & rhs);
  //! @brief Destructor.
  virtual ~Laguerre() {}

  //! @brief Degree n. @return degree
  unsigned int degree() const { return _degree; }
  //! @brief Parameter alpha. @return alpha
  double alpha() const { return _alpha; }
  //! @brief Set the degree. @param n new degree
  void setDegree(unsigned int n) { _degree = n; }
  //! @brief Set alpha. @param alpha new value @throws MathException if alpha is not finite
  void setAlpha(double alpha);
  //! @brief The rank of a Laguerre polynomial is always 1. @param rank must be 1 @throws MathException otherwise
  void setRank(unsigned int rank) override;

  //! @brief Value L_n^(alpha)(x). @param x variable @return value
  double evaluateAt(double x) const override;
  //! @brief Value L_n^(alpha)(x[0]). @param x vector of one value @return value @throws MathException if x.size() != 1
  double evaluateAt(const std::vector<double> & x) const override;
  using Function::evaluateAt;

  //! @brief First derivative, -L_{n-1}^(alpha+1)(x) (0 for n = 0). @param x variable @return derivative
  double derivativeAt(double x) const;
  //! @brief Antiderivative -L_{n+1}^(alpha-1)(x). @param x variable @return antiderivative (defined up to a constant)
  double antiderivativeAt(double x) const;
  //! @brief Integral from low to high (negative if low > high). @param low lower bound @param high upper bound @return integral
  //! @throws MathException if a bound is not finite
  double integralInRange(double low, double high) const;

  //! @brief Whether the polynomials of this alpha are orthogonal on [0, infinity), i.e. alpha > -1. @return true if orthogonal
  bool isOrthogonal() const { return _alpha > -1.0; }
  //! @brief Weight of the orthogonality relation, x^alpha e^(-x). @param x variable (>= 0) @return weight
  //! @throws MathException if x < 0
  double weightAt(double x) const;
  //!
  //! @brief Norm: integral from 0 to infinity of [L_n^(alpha)]^2 x^alpha e^(-x) dx = Gamma(n+alpha+1)/n!.
  //! @return norm
  //! @throws MathException if alpha <= -1 (the integral diverges)
  //!
  double norm() const;

  //!
  //! @brief Value of L_n^(alpha)(x) for any degree and alpha, by the three-term recurrence.
  //! @param n degree @param alpha parameter @param x variable @return value
  //!
  static double value(unsigned int n, double alpha, double x);

  protected:

  unsigned int _degree;  //!< degree n
  double _alpha;         //!< parameter alpha

  ClassDef(Laguerre,1)
  };

  } // namespace MATH
} // namespace CAP

#endif
