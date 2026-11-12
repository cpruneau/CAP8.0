/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file SpecialFunctions.hpp
//! @brief The gamma, beta and incomplete gamma functions as Function objects: GammaFunction, BetaFunction,
//! IncompleteGammaFunction.
#ifndef CAP_MATH_SpecialFunctions
#define CAP_MATH_SpecialFunctions
#include <vector>
#include "Function.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief The gamma function, Gamma(x) = integral from 0 to infinity of t^(x-1) e^(-t) dt (Gamma(n) = (n-1)! for an
  //! integer n >= 1), for real x (std::tgamma; infinite or NaN at the poles x = 0, -1, -2, ...). Rank 1.
  //!
  //! @code
  //!   CAP::MATH::GammaFunction gamma;
  //!   double g = gamma.evaluateAt(5.0);                        // 24
  //!   double l = CAP::MATH::GammaFunction::logGamma(171.5);    // ln Gamma, no overflow
  //! @endcode
  //!
  class GammaFunction : public Function
  {
  public:

  //! @brief Gamma function (rank 1).
  GammaFunction();
  //! @brief Destructor.
  virtual ~GammaFunction() { }
  //! @brief Gamma(x). @param x x @return value
  double evaluateAt(double x) const override;
  //! @brief Gamma(x[0]). @param x one value @return value @throws MathException if x is not of size 1
  double evaluateAt(const std::vector<double> & x) const override;
  using Function::evaluateAt;
  //! @brief ln |Gamma(x)| (std::lgamma). @param x x @return value
  static double logGamma(double x);
  };

  //!
  //! @brief The beta function, B(a, b) = Gamma(a) Gamma(b)/Gamma(a + b) = integral from 0 to 1 of
  //! t^(a-1) (1-t)^(b-1) dt, a function of two variables (a, b). For a, b > 0 it is computed from ln Gamma (no
  //! overflow); otherwise from Gamma (infinite or NaN at the poles).
  //!
  //! @code
  //!   CAP::MATH::BetaFunction beta;
  //!   double b = beta.evaluateAt(2.0, 3.0);     // 1/12
  //! @endcode
  //!
  class BetaFunction : public Function
  {
  public:

  //! @brief Beta function (rank 2).
  BetaFunction();
  //! @brief Destructor.
  virtual ~BetaFunction() { }
  //! @brief B(a, b). @param a a @param b b @return value
  double evaluateAt(double a, double b) const override;
  //! @brief B(x[0], x[1]). @param x a, b @return value @throws MathException if x is not of size 2
  double evaluateAt(const std::vector<double> & x) const override;
  using Function::evaluateAt;
  //! @brief B(a, b). @param a a @param b b @return value
  static double beta(double a, double b);
  //! @brief ln B(a, b), for a, b > 0. @param a a @param b b @return value
  static double logBeta(double a, double b);
  };

  //!
  //! @brief The incomplete gamma functions of two variables (a, x), a > 0, x >= 0, in one of four kinds:
  //!  - kLowerRegularized: P(a, x) = gamma(a, x)/Gamma(a) (the default; the cumulative distribution of a gamma
  //!    distribution, e.g. P(k/2, x/2) that of a chi square of k degrees of freedom);
  //!  - kUpperRegularized: Q(a, x) = 1 - P(a, x) = Gamma(a, x)/Gamma(a);
  //!  - kLower: gamma(a, x) = integral from 0 to x of t^(a-1) e^(-t) dt;
  //!  - kUpper: Gamma(a, x) = integral from x to infinity of t^(a-1) e^(-t) dt.
  //! Computed by series or continued fraction (Distributions::regularizedGammaP(), regularizedGammaQ()).
  //!
  //! @code
  //!   CAP::MATH::IncompleteGammaFunction p;                       // P(a, x)
  //!   double v = p.evaluateAt(1.0, 2.0);                          // 1 - exp(-2)
  //!   CAP::MATH::IncompleteGammaFunction upper(CAP::MATH::IncompleteGammaFunction::kUpper);
  //! @endcode
  //!
  class IncompleteGammaFunction : public Function
  {
  public:

  //! @brief Kind of incomplete gamma function.
  enum Kind
  {
    kLowerRegularized,   //!< P(a, x)
    kUpperRegularized,   //!< Q(a, x)
    kLower,              //!< gamma(a, x)
    kUpper               //!< Gamma(a, x)
  };

  //! @brief Incomplete gamma function of a kind (rank 2: a, x). @param kind kind
  explicit IncompleteGammaFunction(Kind kind=kLowerRegularized);
  //! @brief Destructor.
  virtual ~IncompleteGammaFunction() { }
  //! @brief Kind. @return kind
  Kind kind() const { return _kind; }
  //! @brief Set the kind. @param kind kind
  void setKind(Kind kind) { _kind = kind; }
  //! @brief Value at (a, x). @param a a (> 0) @param x x (>= 0) @return value
  //! @throws MathException if a <= 0 or x < 0
  double evaluateAt(double a, double x) const override;
  //! @brief Value at (x[0], x[1]). @param x a, x @return value @throws MathException if x is not of size 2, a <= 0 or x < 0
  double evaluateAt(const std::vector<double> & x) const override;
  using Function::evaluateAt;

  protected:

  Kind _kind;   //!< kind
  };

  } // namespace MATH
} // namespace CAP

#endif // CAP_MATH_SpecialFunctions
