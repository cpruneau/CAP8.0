#ifndef CAP_MATH_Legendre
#define CAP_MATH_Legendre
#include <vector>
#include "String.hpp"
#include "Function.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Legendre polynomial P_n(x) as a Function of rank 1.
  //!
  //! P_0 = 1, P_1 = x, P_2 = (3x^2 - 1)/2, P_3 = (5x^3 - 3x)/2, ...
  //!
  //! - Value: three-term recurrence (k+1) P_{k+1} = (2k+1) x P_k - k P_{k-1} (stable, n steps).
  //! - Derivative: recurrence P'_{k+1} = P'_{k-1} + (2k+1) P_k, accurate also at and near x = +-1
  //!   (where the usual form n (x P_n - P_{n-1})/(x^2 - 1) is 0/0).
  //! - Integral: antiderivative (P_{n+1} - P_{n-1})/(2n+1) (x for n = 0); when its values at the two bounds nearly
  //!   cancel (short intervals), Gauss-Legendre quadrature, exact for a polynomial of degree n, is used instead.
  //!
  //! The polynomials are defined for any real x and are orthogonal on [-1, 1] with weight 1:
  //! integral from -1 to 1 of P_n P_m dx = 2/(2n+1) if n = m, 0 otherwise (see norm()).
  //! gaussLegendre() gives the nodes (roots of P_n) and weights of Gauss-Legendre quadrature.
  //!
  //! Example:
  //! @code
  //!   using namespace CAP::MATH;
  //!   Legendre p2(2);
  //!   double y = p2.evaluateAt(0.5);                // (3*0.25 - 1)/2 = -0.125
  //!   double d = p2.derivativeAt(0.5);              // 3x = 1.5
  //!   double s = p2.integralInRange(0.0, 1.0);      // 0
  //!   std::vector<double> nodes, weights;
  //!   Legendre::gaussLegendre(5, nodes, weights);   // 5-point rule on [-1, 1]
  //! @endcode
  //!
  class Legendre : public Function
  {
  public:

  //! @brief Constructor (also the default constructor: P_0 = 1). @param n degree of the polynomial
  Legendre(unsigned int n=0);
  //! @brief Copy constructor. @param source polynomial to copy
  Legendre(const Legendre & source);
  //! @brief Assignment operator. @param rhs polynomial to copy @return reference to this polynomial
  Legendre & operator=(const Legendre & rhs);
  //! @brief Destructor.
  virtual ~Legendre() {}

  //! @brief Degree n. @return degree
  unsigned int degree() const { return _degree; }
  //! @brief Set the degree. @param n new degree
  void setDegree(unsigned int n) { _degree = n; }
  //! @brief The rank of a Legendre polynomial is always 1. @param rank must be 1 @throws MathException otherwise
  void setRank(unsigned int rank) override;

  //! @brief Value P_n(x). @param x variable @return value
  double evaluateAt(double x) const override;
  //! @brief Value P_n(x[0]). @param x vector of one value @return value @throws MathException if x.size() != 1
  double evaluateAt(const std::vector<double> & x) const override;
  using Function::evaluateAt;

  //! @brief First derivative P_n'(x). @param x variable @return derivative
  double derivativeAt(double x) const;
  //! @brief Antiderivative (P_{n+1}(x) - P_{n-1}(x))/(2n+1), or x for n = 0; zero at x = -1 and x = 1 for n >= 1.
  //! @param x variable @return antiderivative
  double antiderivativeAt(double x) const;
  //! @brief Integral from low to high (negative if low > high). @param low lower bound @param high upper bound @return integral
  //! @throws MathException if a bound is not finite
  double integralInRange(double low, double high) const;

  //! @brief Norm: integral from -1 to 1 of P_n^2 dx = 2/(2n+1). @return norm
  double norm() const { return 2.0/(2.0*_degree + 1.0); }

  //! @brief Value of P_n(x) for any degree, by the three-term recurrence. @param n degree @param x variable @return value
  static double value(unsigned int n, double x);
  //! @brief Derivative P_n'(x) for any degree. @param n degree @param x variable @return derivative
  static double derivative(unsigned int n, double x);
  //!
  //! @brief Nodes and weights of the m-point Gauss-Legendre quadrature rule on [-1, 1].
  //!
  //! The nodes are the m roots of P_m, in decreasing order; the rule integrates polynomials of degree <= 2m-1 exactly:
  //! integral from -1 to 1 of f(x) dx is approximately sum_i weights[i] f(nodes[i]).
  //! @param m number of points (>= 1) @param nodes output: the m nodes @param weights output: the m weights
  //! @throws MathException if m is 0
  //!
  static void gaussLegendre(unsigned int m, std::vector<double> & nodes, std::vector<double> & weights);

  protected:

  unsigned int _degree;  //!< degree n

  };

  } // namespace MATH
} // namespace CAP

#endif
