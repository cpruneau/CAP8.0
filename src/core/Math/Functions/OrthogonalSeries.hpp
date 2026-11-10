/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file OrthogonalSeries.hpp
//! @brief OrthogonalSeries: series of orthogonal polynomials (Legendre, Chebyshev, Hermite, Laguerre), for fits.
#ifndef CAP__MATH__OrthogonalSeries
#define CAP__MATH__OrthogonalSeries
#include <vector>
#include "ParameterizedFunction.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Series of orthogonal polynomials of degree N: f(x) = c_0 P_0(t) + c_1 P_1(t) + ... + c_N P_N(t), t being x
  //! mapped onto the natural domain of the polynomials.
  //!
  //! The coefficients c_0 ... c_N are the first parameters; the parameters of the mapping follow, fixed by default:
  //!  - LegendreSeries (P_n) and ChebyshevSeries (T_n, first kind): "low" and "high", the range of x mapped onto
  //!    [-1, 1], t = (2x - low - high)/(high - low);
  //!  - HermiteSeries (H_n, physicists'): "x0" and "scale", t = (x - x0)/scale;
  //!  - LaguerreSeries (generalized L_n^alpha): "x0", "scale" and "alpha", t = (x - x0)/scale.
  //!
  //! adaptToRange() sets the fixed mapping parameters for the range of a fit: [low, high] becomes t in [-1, 1]
  //! (Legendre, Chebyshev), [-2, 2] (Hermite) or [0, 4] (Laguerre). The series is linear in its coefficients, and the
  //! polynomials are nearly orthogonal over such a range: fits are fast and well conditioned, much better than with
  //! the powers of x of a Polynomial of high degree. estimate() solves the least squares exactly.
  //!
  //! @code
  //!   CAP::MATH::LegendreSeries f(4);            // c_0 P_0 + ... + c_4 P_4
  //!   f.adaptToRange(0.0, 10.0);                 // x in [0, 10] -> t in [-1, 1]
  //!   CAP::MATH::Fitter::estimate(f, data, 0.0, 10.0);
  //!   CAP::MATH::Fitter(f, data).fit();
  //! @endcode
  //!
  class OrthogonalSeries : public ParameterizedFunction
  {
  public:

  //! @brief Families of polynomials.
  enum Family { kLegendre, kChebyshev, kHermite, kLaguerre };

  //! @brief Series of a family. @param family family @param degree degree N (N+1 coefficients, all 0 but c_0 = 1)
  OrthogonalSeries(Family family, unsigned int degree);
  //! @brief Destructor.
  virtual ~OrthogonalSeries() { }

  //! @brief Family of the polynomials. @return family
  Family family() const { return _family; }
  //! @brief Degree N of the series. @return degree
  unsigned int degree() const { return _nCoefficients - 1; }
  //! @brief Number of coefficients (N+1). @return number
  unsigned int nCoefficients() const { return _nCoefficients; }
  //! @brief Change the degree: coefficients are added (0) or removed at the end; the mapping is kept. @param degree degree
  void setDegree(unsigned int degree);
  //! @brief Number of parameters of the mapping of a family (2 or 3). @param family family @return number
  static unsigned int nMappingParameters(Family family) { return family == kLaguerre ? 3 : 2; }

  //! @brief Mapped variable t of x. @param x x @return t
  double variable(double x) const;
  //! @brief Values of the polynomials P_0 ... P_N at t. @param family family @param degree N @param t variable
  //! @param alpha alpha of the Laguerre polynomials @param values set to the N+1 values
  static void polynomials(Family family, unsigned int degree, double t, double alpha, std::vector<double> & values);

  //! @brief Value of the series. @param x x @return f(x)
  double evaluateAt(double x) const override;
  using ParameterizedFunction::evaluateAt;
  //! @brief Derivative (exact: the derivative series, see derivativeFunction()). @param x x @return f'(x)
  double derivativeAt(double x) const override;
  //! @brief The derivative is analytical. @return true
  bool hasAnalyticalDerivative() const override { return true; }
  //!
  //! @brief Derivative, a series of the same family of degree N-1 and the same mapping: P_n' = sum over k < n, n-k
  //! odd, of (2k+1) P_k (Legendre); T_n' = 2n sum over k < n, n-k odd, of T_k, T_0 counted half (Chebyshev);
  //! H_n' = 2n H_{n-1} (Hermite); L_n^alpha' = -sum over k < n of L_k^alpha (Laguerre); times dt/dx.
  //! @param axis 0 @return new series (owned by the caller)
  //!
  Function * derivativeFunction(unsigned int axis=0) const override;
  //! @brief Coefficients of the derivative series (in t) and dt/dx. @param d set to the coefficients (N of them, at
  //! least one) @return dt/dx
  double derivativeCoefficients(std::vector<double> & d) const;
  //! @brief Set the fixed mapping parameters for a fit over [low, high] (see the class). @param low lower end
  //! @param high upper end
  void adaptToRange(double low, double high) override;
  using ParameterizedFunction::adaptToRange;
  //!
  //! @brief Least-squares values of the free coefficients through points (the mapping and the fixed coefficients
  //! are kept). @param x abscissas @param y values @param w weights (1/sigma^2) @return false if the system is singular
  //!
  bool estimate(const std::vector<double> & x, const std::vector<double> & y, const std::vector<double> & w);

  protected:

  Family _family;   //!< family of the polynomials
  unsigned int _nCoefficients;   //!< N+1
  };

  //! @brief Series of Legendre polynomials P_n, x in [low, high] mapped onto [-1, 1].
  class LegendreSeries : public OrthogonalSeries
  {
  public:
  //! @brief Series. @param degree degree
  explicit LegendreSeries(unsigned int degree=3) : OrthogonalSeries(kLegendre, degree) { }
  //! @brief Copy. @return new series (owned by the caller)
  LegendreSeries * clone() const override { return new LegendreSeries(*this); }
  };

  //! @brief Series of Chebyshev polynomials of the first kind T_n, x in [low, high] mapped onto [-1, 1].
  class ChebyshevSeries : public OrthogonalSeries
  {
  public:
  //! @brief Series. @param degree degree
  explicit ChebyshevSeries(unsigned int degree=3) : OrthogonalSeries(kChebyshev, degree) { }
  //! @brief Copy. @return new series (owned by the caller)
  ChebyshevSeries * clone() const override { return new ChebyshevSeries(*this); }
  };

  //! @brief Series of Hermite polynomials H_n (physicists'), t = (x - x0)/scale.
  class HermiteSeries : public OrthogonalSeries
  {
  public:
  //! @brief Series. @param degree degree
  explicit HermiteSeries(unsigned int degree=3) : OrthogonalSeries(kHermite, degree) { }
  //! @brief Copy. @return new series (owned by the caller)
  HermiteSeries * clone() const override { return new HermiteSeries(*this); }
  };

  //! @brief Series of generalized Laguerre polynomials L_n^alpha, t = (x - x0)/scale.
  class LaguerreSeries : public OrthogonalSeries
  {
  public:
  //! @brief Series. @param degree degree @param alpha alpha (> -1)
  explicit LaguerreSeries(unsigned int degree=3, double alpha=0.0);
  //! @brief Copy. @return new series (owned by the caller)
  LaguerreSeries * clone() const override { return new LaguerreSeries(*this); }
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__OrthogonalSeries
