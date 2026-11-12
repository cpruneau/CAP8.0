/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file PiecewisePolynomial.hpp
//! @brief PiecewisePolynomial: a function of one variable made of polynomial pieces between breakpoints.
#ifndef CAP__MATH__PiecewisePolynomial
#define CAP__MATH__PiecewisePolynomial
#include <vector>
#include "Function.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Function of one variable made of polynomials between breakpoints b_0 < b_1 < ... < b_m: on [b_i, b_i+1],
  //! f(x) = c_i0 + c_i1 (x - b_i) + c_i2 (x - b_i)^2 + ... (local coefficients). Below b_0 and above b_m, the first and
  //! last pieces are extended.
  //!
  //! Its derivatives and integrals are exact: derivativeAt(), derivativeAt(x, n), derivativeFunction() (a piecewise
  //! polynomial of one degree less), antiderivativeAt() and integralInRange(). The interpolating splines (CubicSpline)
  //! and linear interpolation (Interpolation) are piecewise polynomials.
  //!
  //! @code
  //!   CAP::MATH::PiecewisePolynomial p({0.0, 1.0, 2.0}, {{0.0, 1.0}, {1.0, -1.0}});   // a tent: x, then 2 - x
  //!   double v = p(1.5);                                                              // 0.5
  //!   double s = p.integralInRange(0.0, 2.0);                                         // 1
  //! @endcode
  //!
  class PiecewisePolynomial : public Function
  {
  public:

  //! @brief Empty function (no piece; evaluates to 0).
  PiecewisePolynomial();
  //! @brief Function of pieces. @param breakpoints m+1 increasing breakpoints @param coefficients m lists of local
  //! coefficients (increasing powers of x - b_i; of any lengths) @throws MathException if the sizes do not match or the
  //! breakpoints do not increase
  PiecewisePolynomial(const std::vector<double> & breakpoints, const std::vector<std::vector<double>> & coefficients);
  //! @brief Copy. @param source function
  PiecewisePolynomial(const PiecewisePolynomial & source) = default;
  //! @brief Assignment. @param rhs function @return this
  PiecewisePolynomial & operator=(const PiecewisePolynomial & rhs) = default;
  //! @brief Destructor.
  ~PiecewisePolynomial() override { }

  //! @name Values
  //! @{
  //! @brief Value at x. @param x x @return f(x)
  double evaluateAt(double x) const override;
  //! @brief Value at a point (rank 1). @param x coordinates (one) @return f(x[0])
  double evaluateAt(const std::vector<double> & x) const override;
  using Function::evaluateAt;
  //! @brief First derivative (exact). @param x x @return f'(x)
  double derivativeAt(double x) const override;
  //! @brief Derivative of order n (exact). @param x x @param order n @return f^(n)(x)
  double derivativeAt(double x, unsigned int order) const;
  //! @brief The derivatives are analytical. @return true
  bool hasAnalyticalDerivative() const override { return true; }
  //! @brief Derivative, a piecewise polynomial. @param axis 0 @return new function (owned by the caller), null if axis is not 0
  Function * derivativeFunction(unsigned int axis=0) const override;
  //! @brief Antiderivative, 0 at the first breakpoint. @param x x @return integral from b_0 to x
  double antiderivativeAt(double x) const;
  //! @brief Integral from low to high (exact). @param low lower bound @param high upper bound @return integral
  double integralInRange(double low, double high) const override;
  using Function::integralInRange;
  //! @}

  //! @name Pieces
  //! @{
  //! @brief Breakpoints. @return breakpoints
  const std::vector<double> & breakpoints() const { return _breakpoints; }
  //! @brief Local coefficients of the pieces. @return coefficients
  const std::vector<std::vector<double>> & coefficients() const { return _coefficients; }
  //! @brief Number of pieces. @return pieces
  std::size_t nPieces() const { return _coefficients.size(); }
  //! @brief Index of the piece used at x (the end pieces are extended). @param x x @return index
  std::size_t piece(double x) const;
  //! @brief Set the pieces. @param breakpoints breakpoints @param coefficients local coefficients
  //! @throws MathException as the constructor
  void setPieces(const std::vector<double> & breakpoints, const std::vector<std::vector<double>> & coefficients);
  //! @}

  protected:

  //! @brief Antiderivative of piece i from b_i to x. @param i piece @param x x @return integral
  double pieceIntegral(std::size_t i, double x) const;

  std::vector<double> _breakpoints;                 //!< breakpoints b_0 ... b_m
  std::vector<std::vector<double>> _coefficients;   //!< local coefficients of the m pieces
  std::vector<double> _cumulative;                  //!< integral from b_0 to b_i
  };

  } // namespace MATH
} // namespace CAP

#endif // CAP__MATH__PiecewisePolynomial
