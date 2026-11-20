/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FourierTools.hpp
//! @brief FourierTools: special functions and building blocks of the Fourier transforms of the functions of Datum.
#ifndef DATUM_MATH_FourierTools
#define DATUM_MATH_FourierTools
#include <complex>
#include <functional>
#include <vector>

namespace Datum
{
  namespace MATH
  {

  //!
  //! @brief Special functions and building blocks of the analytical Fourier transforms (see Function::fourierTransform()
  //! and docs/Fourier.md). Convention: F(k) = integral of f(x) e^{-ikx} dx.
  //!
  //! - faddeeva(): the Faddeeva function w(z) = e^{-z^2} erfc(-iz) (Weideman's rational approximation, relative accuracy
  //!   about 1e-13 in the upper half plane), from which the transforms of the Gaussian on a domain and of the half-line
  //!   Gaussian moments (Maxwell-Boltzmann, Rayleigh) are computed;
  //! - exponentialIntegral(): the integral of e^{cx} over [a, b] for a complex c (bounds may be infinite);
  //! - gaussianDomain(), halfGaussianMoment(): transforms of Gaussians on a domain and on [0, infinity);
  //! - sphericalBesselJ(): j_0(x) ... j_n(x), from which the transform of a Legendre series on a domain is computed;
  //! - besselKNormalized(): z^nu K_nu(z)/(Gamma(nu) 2^(nu-1)) for a real order nu (the characteristic function of the
  //!   Student distribution);
  //! - legendreCoefficients(), legendreTransform(): a function on [a, b] as a Legendre series (exact for polynomials of
  //!   the degree used, an approximation otherwise) and the exact transform of that series.
  //!
  namespace FourierTools
  {
  using Complex = std::complex<double>;   //!< complex numbers of the transforms

  //! @brief Faddeeva function w(z) = e^{-z^2} erfc(-iz). @param z argument @return w(z)
  Complex faddeeva(Complex z);
  //! @brief e^{-v^2} erf(u + iv), bounded for all real u, v (infinite u allowed). @param u real part @param v imaginary part
  //! @return value
  Complex scaledErf(double u, double v);
  //! @brief Integral of e^{c x} from a to b (infinite bounds allowed where it converges). @param c coefficient @param a
  //! lower bound @param b upper bound @return integral @throws MathException if it diverges
  Complex exponentialIntegral(Complex c, double a, double b);
  //! @brief Integral of e^{-(x - mu)^2/(2 sigma^2)} e^{-ikx} over the whole line. @param k k @param mu mean
  //! @param sigma width @return transform
  Complex gaussianLine(double k, double mu, double sigma);
  //! @brief Integral of e^{-(x - mu)^2/(2 sigma^2)} e^{-ikx} from a to b (infinite bounds allowed). @param k k
  //! @param mu mean @param sigma width @param a lower bound @param b upper bound @return transform
  Complex gaussianDomain(double k, double mu, double sigma, double a, double b);
  //! @brief Integral of x^m e^{-x^2/(2 s^2)} e^{-ikx} from 0 to infinity, m = 0, 1 or 2. @param m power @param k k
  //! @param s width @return transform @throws MathException if m > 2
  Complex halfGaussianMoment(unsigned int m, double k, double s);
  //! @brief Spherical Bessel functions j_0(x) ... j_nMax(x) (forward recurrence for x > nMax, Miller's backward
  //! recurrence otherwise). @param nMax largest order @param x argument @param j set to the nMax+1 values
  void sphericalBesselJ(unsigned int nMax, double x, std::vector<double> & j);
  //! @brief Spherical Bessel function j_n(x). @param n order @param x argument @return j_n(x)
  double sphericalBesselJ(unsigned int n, double x);
  //! @brief z^nu K_nu(z) / (Gamma(nu) 2^(nu - 1)) for nu > 0 and z >= 0 (1 at z = 0), computed in logarithms so that it
  //! neither overflows nor underflows. @param nu order @param z argument @return value
  double besselKNormalized(double nu, double z);
  //! @brief sin(u)/u (1 at 0). @param u argument @return value
  double sinc(double u);

  //!
  //! @brief Legendre coefficients c_0 ... c_N of a function on [a, b]: f(x) = sum c_n P_n(t), t = (2x - a - b)/(b - a),
  //! from the Gauss-Legendre rule of N+1 points (exact for a polynomial of degree N or less; for other functions, the
  //! polynomial interpolating f at the nodes).
  //! @param f function @param a lower bound @param b upper bound @param degree N @return N+1 coefficients
  //!
  std::vector<double> legendreCoefficients(const std::function<double(double)> & f, double a, double b, unsigned int degree);
  //!
  //! @brief Legendre coefficients of a function on [a, b] of a degree raised (16, 32, ... maxDegree) until the last
  //! eighth of the coefficients is below tolerance times the largest one.
  //! @param f function @param a lower bound @param b upper bound @param tolerance relative size of the neglected
  //! coefficients @param maxDegree largest degree @param tail set to the largest coefficient of the last eighth, relative
  //! to the largest coefficient (may be null) @return coefficients
  //!
  std::vector<double> adaptiveLegendreCoefficients(const std::function<double(double)> & f, double a, double b,
                                                   double tolerance=1e-13, unsigned int maxDegree=2048, double * tail=nullptr);
  //!
  //! @brief Exact transform on [a, b] of the Legendre series of coefficients c: (b - a)/2 e^{-ikm} sum c_n 2 (-i)^n
  //! j_n(k (b - a)/2), m = (a + b)/2. @param c coefficients @param k k @param a lower bound @param b upper bound
  //! @return transform
  //!
  Complex legendreTransform(const std::vector<double> & c, double k, double a, double b);
  //! @brief Transform on [a, b] of a function that is a polynomial of degree N there (exact). @param f function
  //! @param degree N @param k k @param a lower bound @param b upper bound @return transform
  Complex polynomialTransform(const std::function<double(double)> & f, unsigned int degree, double k, double a, double b);

  //!
  //! @brief A function approximated by a Legendre series on each segment of a partition (piecewise polynomial), and the
  //! exact transform of that approximation: for functions of very different scales along x (peaked densities with long
  //! tails), the segments follow the function (e.g. quantiles of a distribution).
  //!
  struct PiecewiseLegendre
  {
    std::vector<double> edges;   //!< edges of the segments (increasing)
    std::vector<std::vector<double>> coefficients;   //!< Legendre coefficients on each segment
    double tail = 0.0;           //!< largest relative size of the neglected coefficients
    unsigned int largestDegree = 0;   //!< largest degree used
    //! @brief Approximate f on the segments (adaptiveLegendreCoefficients() on each). @param f function @param edges
    //! edges (at least 2, increasing) @param tolerance tolerance @param maxDegree largest degree @return approximation
    static PiecewiseLegendre build(const std::function<double(double)> & f, const std::vector<double> & edges,
                                   double tolerance=1e-13, unsigned int maxDegree=512);
    //! @brief Transform of the approximation. @param k k @return transform
    Complex transform(double k) const;
  };
  } // namespace FourierTools

  } // namespace MATH
} // namespace Datum

#endif // DATUM_MATH_FourierTools
