#ifndef CAP_MATH_Bessel
#define CAP_MATH_Bessel
#include <vector>
#include "Aliases.hpp"
#include "Function.hpp"
#include "ClassDef.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Bessel (or Struve) function of integer order as a Function of rank 1, f(x).
  //!
  //! The function is chosen at construction with a BesselType and an order, and evaluated with the
  //! functions of BesselFunctions.hpp:
  //!  - BesselJ, BesselY : Bessel functions of the first and second kind, J_n and Y_n, orders 0 and 1
  //!  - BesselI, BesselK : modified Bessel functions I_n and K_n, any order n >= 0
  //!  - StruveH, StruveL : Struve function H_n and modified Struve function L_n, orders 0 and 1
  //! Y_n and K_n are defined for x > 0 only (MathException otherwise).
  //!
  //! derivativeAt() and integralInRange() use the recurrence relations of these functions, e.g.
  //! J0' = -J1, I_n' = (I_{n-1} + I_{n+1})/2, integral of K_{n+1} = -2 K_n - integral of K_{n-1}, and the closed forms
  //! integral of J0 from 0 to x = x J0 + (pi x/2)(J1 H0 - J0 H1) (same for Y0) and
  //! integral of K0 from 0 to x = x K0 + (pi x/2)(K1 L0 + K0 L1). The integrals of I_n use their power series (all terms
  //! of the same sign; the recurrence cancels badly for x < n). The integrals of H0 and L0 have no closed form in these
  //! functions and are computed numerically; so are integrals whose antiderivatives at the two bounds nearly cancel.
  //! Their accuracy is that of the underlying functions (see BesselFunctions.hpp), typically 1e-7 to 1e-9.
  //!
  //! Example:
  //! @code
  //!   using namespace CAP::MATH;
  //!   Bessel j0(Bessel::BesselJ);         // J0
  //!   Bessel k2(Bessel::BesselK, 2);      // K2
  //!   double y  = j0.evaluateAt(2.4);     // close to the first zero of J0
  //!   double d  = k2.derivativeAt(1.0);   // -(K1(1) + K3(1))/2
  //!   double s  = j0.integralInRange(0.0, 10.0);
  //! @endcode
  //!
  class Bessel : public Function
  {
  public:

  //! @brief Kind of function.
  enum BesselType
  {
    BesselJ,  //!< Bessel function of the first kind J_n, n = 0, 1
    BesselY,  //!< Bessel function of the second kind Y_n, n = 0, 1 (x > 0)
    BesselI,  //!< modified Bessel function of the first kind I_n, n >= 0
    BesselK,  //!< modified Bessel function of the second kind K_n, n >= 0 (x > 0)
    StruveH,  //!< Struve function H_n, n = 0, 1
    StruveL   //!< modified Struve function L_n, n = 0, 1
  };

  //!
  //! @brief Constructor (also the default constructor: J0).
  //! @param type kind of function @param order order n
  //! @throws MathException if the order is not available for this type
  //!
  Bessel(BesselType type=BesselJ, unsigned int order=0);
  //! @brief Copy constructor. @param source function to copy
  Bessel(const Bessel & source);
  //! @brief Assignment operator. @param rhs function to copy @return reference to this function
  Bessel & operator=(const Bessel & rhs);
  //! @brief Destructor.
  virtual ~Bessel() {}

  //! @brief Kind of function. @return type
  BesselType type() const { return _type; }
  //! @brief Order of the function. @return order n
  unsigned int order() const { return _order; }
  //!
  //! @brief Change the function.
  //! @param type kind of function @param order order n
  //! @throws MathException if the order is not available for this type (the function is then unchanged)
  //!
  void setType(BesselType type, unsigned int order=0);
  //! @brief The rank of a Bessel function is always 1. @param rank must be 1 @throws MathException otherwise
  void setRank(unsigned int rank) override;

  //! @brief Value of the function at x. @param x variable @return f(x)
  //! @throws MathException if x is outside the domain (x <= 0 for Y and K)
  double evaluateAt(double x) const override;
  //! @brief Value of the function at x[0]. @param x vector of one value @return f(x[0])
  //! @throws MathException if x.size() != 1, or x[0] is outside the domain
  double evaluateAt(const std::vector<double> & x) const override;
  using Function::evaluateAt;

  //!
  //! @brief First derivative at x, from the recurrence relations.
  //! @param x variable @return f'(x)
  //! @throws MathException if x is outside the domain
  //!
  double derivativeAt(double x) const;

  //!
  //! @brief Antiderivative F(x) with F(0) = 0 (for H0 and L0: numerical integral from 0 to x).
  //! @param x variable @return F(x)
  //! @throws MathException for Y1 and K_n, n >= 1 (not integrable at 0), or if x is outside the domain
  //!
  double antiderivativeAt(double x) const;

  //!
  //! @brief Integral of the function from low to high (negative if low > high).
  //! @param low lower bound @param high upper bound @return integral
  //! @throws MathException if a bound is not finite or is outside the domain (for Y and K: bounds must be > 0,
  //! except that 0 is allowed for Y0 and K0, whose integral converges at 0)
  //!
  double integralInRange(double low, double high) const;

  protected:

  //! @brief Throw if order is not available for type. @param type kind @param order order @param source name of the calling function
  static void checkOrder(BesselType type, unsigned int order, const char * source);
  //! @brief Value of the function of the same type with order n (n >= 0). @param n order @param x variable @return value
  double valueOfOrder(int n, double x) const;
  //! @brief Antiderivative of I_n with F(0) = 0, from its power series. @param n order @param x variable @return integral of I_n from 0 to x
  static double integralI(unsigned int n, double x);
  //! @brief Antiderivative of K_n (K_0: integral from 0; n >= 1: up to a constant). @param n order @param x variable (> 0) @return antiderivative
  static double integralK(unsigned int n, double x);
  //! @brief Numerical integral of the function from low to high (finite bounds). @return integral
  double numericalIntegral(double low, double high) const;

  BesselType   _type;   //!< kind of function
  unsigned int _order;  //!< order n

  ClassDef(Bessel,1)
  };

  } // namespace MATH
} // namespace CAP

#endif
