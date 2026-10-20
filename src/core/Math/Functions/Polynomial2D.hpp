/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Polynomial2D.hpp
//! @brief Polynomial2D: polynomial of two variables, for fits.
#ifndef CAP__MATH__Polynomial2D
#define CAP__MATH__Polynomial2D
#include <vector>
#include "ParameterizedFunction.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Polynomial of two variables of total degree N: f(x,y) = sum over i + j <= N of c_ij t^i s^j, with
  //! t = (x - x0)/xScale and s = (y - y0)/yScale.
  //!
  //! The coefficients come first, by total degree and then by decreasing power of t: c00; c10, c01; c20, c11, c02;
  //! ... ("cij" is the coefficient of t^i s^j), (N+1)(N+2)/2 of them; then the fixed parameters of the mapping "x0",
  //! "xScale", "y0", "yScale" (0, 1, 0, 1 by default: a plain polynomial of x and y). adaptToRange() maps a box of x
  //! and y onto [-1, 1] x [-1, 1], which keeps fits well conditioned. The degree is at most 9.
  //!
  //! @code
  //!   CAP::MATH::Polynomial2D background(2);           // 6 coefficients
  //!   background.adaptToRange({ 0.0, -5.0 }, { 10.0, 5.0 });
  //! @endcode
  //!
  class Polynomial2D : public ParameterizedFunction
  {
  public:

  //! @brief Polynomial. @param degree total degree N (0 to 9; all coefficients 0 but c00 = 1)
  //! @throws MathException if the degree is above 9
  explicit Polynomial2D(unsigned int degree=2);
  //! @brief Destructor.
  virtual ~Polynomial2D() { }
  //! @brief Copy. @return new polynomial (owned by the caller)
  Polynomial2D * clone() const override { return new Polynomial2D(*this); }

  //! @brief Total degree N. @return degree
  unsigned int degree() const { return _degree; }
  //! @brief Number of coefficients, (N+1)(N+2)/2. @return number
  unsigned int nCoefficients() const { return nCoefficients(_degree); }
  //! @brief Number of coefficients of a degree. @param degree degree @return (degree+1)(degree+2)/2
  static unsigned int nCoefficients(unsigned int degree) { return (degree + 1)*(degree + 2)/2; }
  //! @brief Change the degree: the coefficients of the degrees kept are kept, new ones are 0; the mapping is kept.
  //! @param degree new degree (0 to 9) @throws MathException if above 9
  void setDegree(unsigned int degree);
  //! @brief Index of the coefficient of t^i s^j. @param i power of t @param j power of s @return index
  static unsigned int index(unsigned int i, unsigned int j);

  //! @brief Value. @param x x @param y y @return f(x,y)
  double evaluateAt(double x, double y) const override;
  using ParameterizedFunction::evaluateAt;
  //! @brief Map the box [low[0], high[0]] x [low[1], high[1]] onto [-1, 1] x [-1, 1] (fixed mapping parameters).
  //! @param low lower ends @param high upper ends
  void adaptToRange(const std::vector<double> & low, const std::vector<double> & high) override;
  using ParameterizedFunction::adaptToRange;
  //! @brief Values of the monomials t^i s^j at a point, in the order of the coefficients. @param x x @param y y
  //! @param values set to the values
  void monomials(double x, double y, std::vector<double> & values) const;
  //!
  //! @brief Least-squares values of the free coefficients through points (the mapping and the fixed coefficients
  //! are kept). @param x abscissas @param y ordinates @param z values @param w weights (1/sigma^2)
  //! @return false if the system is singular
  //!
  bool estimate(const std::vector<double> & x, const std::vector<double> & y, const std::vector<double> & z, const std::vector<double> & w);

  protected:

  unsigned int _degree;   //!< total degree
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__Polynomial2D
