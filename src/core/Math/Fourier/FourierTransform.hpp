/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FourierTransform.hpp
//! @brief FourierTransform: the Fourier transform of a function, as a function of k.
#ifndef DATUM_MATH_FourierTransform
#define DATUM_MATH_FourierTransform
#include <complex>
#include <functional>
#include <vector>
#include "Function.hpp"

namespace Datum
{
  namespace MATH
  {

  //!
  //! @brief Fourier transform F(k) = integral of f(x) e^{-ikx} dx of a function f, as a function of k (made by
  //! Function::fourierTransform()).
  //!
  //! F is complex: valueAt() gives it; evaluateAt() (so that the transform can be drawn, fitted, integrated as any
  //! Function) gives the part chosen with setPart(): real part (the default), imaginary part, modulus, squared modulus
  //! (the power spectrum) or phase.
  //!
  //! How the transform was obtained (method()):
  //!  - kClosedForm: the transform on the whole line, in closed form (possibly with special functions: Faddeeva, Bessel);
  //!  - kExactOnDomain: the transform of f restricted to a domain [low, high], exact (polynomials, exponentials, ...);
  //!  - kApproximation: the exact transform of a Legendre series (a polynomial) approximating f on the domain; its error
  //!    is at most the integral of |f - series| over the domain, for every k (description() gives the degree and the
  //!    size of the neglected coefficients).
  //!
  //! With a domain [low, high] of length L, coefficient(n) gives the Fourier series coefficient c_n = F(2 pi n/L)/L of
  //! the periodic function equal to f on the domain: f(x) = sum c_n e^{2 pi i n x/L}.
  //!
  //! The transform is that of f when fourierTransform() was called: it does not follow later changes of its parameters.
  //!
  //! @code
  //!   Datum::MATH::Normal g(1.0, 0.0, 2.0);
  //!   std::unique_ptr<Datum::MATH::FourierTransform> G(g.fourierTransform());
  //!   std::complex<double> v = G->valueAt(0.5);       // 2 sqrt(2 pi) e^{-0.5}
  //!   G->setPart(Datum::MATH::FourierTransform::kModulus);
  //!   double m = G->evaluateAt(0.5);
  //! @endcode
  //!
  class FourierTransform : public Function
  {
  public:

  //! @brief How the transform was obtained (see the class).
  enum Method { kClosedForm, kExactOnDomain, kApproximation };
  //! @brief Value returned by evaluateAt().
  enum Part { kReal, kImaginary, kModulus, kModulusSquared, kPhase };
  //! @brief Computes F at a point k (rank() values).
  using Evaluator = std::function<std::complex<double>(const std::vector<double> & k)>;

  //!
  //! @brief Transform given by an evaluator.
  //! @param rank number of variables @param evaluator F(k) @param method how it was obtained @param description text
  //! (e.g. "closed form", "Legendre approximation of degree 64") @param name name of the function transformed
  //!
  FourierTransform(unsigned int rank, const Evaluator & evaluator, Method method, const String & description,
                   const String & name="");
  //! @brief Copy. @param source transform
  FourierTransform(const FourierTransform & source) = default;
  //! @brief Assignment. @param rhs transform @return this transform
  FourierTransform & operator=(const FourierTransform & rhs) = default;
  //! @brief Destructor.
  virtual ~FourierTransform() { }

  //! @brief F(k) of a transform of rank 1. @param k k @return F(k) @throws MathException if the rank is not 1
  std::complex<double> valueAt(double k) const;
  //! @brief F(kx, ky) of a transform of rank 2. @param kx first @param ky second @return F @throws MathException if the rank is not 2
  std::complex<double> valueAt(double kx, double ky) const;
  //! @brief F(k) for any rank. @param k rank() values @return F(k) @throws MathException if the size differs from the rank
  std::complex<double> valueAt(const std::vector<double> & k) const;

  //! @brief The part of F(k) chosen by setPart(). @param k rank() values @return value
  double evaluateAt(const std::vector<double> & k) const override;
  //! @brief The part of F(k) chosen by setPart(), rank 1. @param k k @return value
  double evaluateAt(double k) const override;
  using Function::evaluateAt;

  Part part() const { return _part; }   //!< Part returned by evaluateAt()
  void setPart(Part part) { _part = part; }   //!< Choose the part returned by evaluateAt()
  Method method() const { return _method; }   //!< How the transform was obtained
  //! @brief Whether the transform is analytical (closed form or exact on its domain). @return true unless an approximation
  bool isAnalytical() const { return _method != kApproximation; }
  const String & description() const { return _description; }   //!< How it was obtained, in words

  //! @brief Set the domain of the function transformed (kExactOnDomain, kApproximation). @param low lower bound @param high upper bound
  void setDomain(double low, double high) { _hasDomain = true; _low = low; _high = high; }
  bool hasDomain() const { return _hasDomain; }   //!< Whether the transform is that of f on a domain
  double low() const { return _low; }   //!< Lower bound of the domain
  double high() const { return _high; }   //!< Upper bound of the domain
  //!
  //! @brief Fourier series coefficient c_n = F(2 pi n/L)/L of the periodic function equal to f on the domain (length L).
  //! @param n index (any sign) @return c_n @throws MathException if the transform has no finite domain or is not of rank 1
  //!
  std::complex<double> coefficient(int n) const;
  //! @brief Value of the part of a complex number chosen by a Part. @param value value @param part part @return value
  static double partOf(const std::complex<double> & value, Part part);

  protected:

  Evaluator _evaluator;   //!< F(k)
  Method _method;         //!< how it was obtained
  String _description;    //!< how it was obtained, in words
  Part _part;             //!< part returned by evaluateAt()
  bool _hasDomain;        //!< transform on a domain
  double _low;            //!< lower bound of the domain
  double _high;           //!< upper bound of the domain
  };

  } // namespace MATH
} // namespace Datum

#endif // DATUM_MATH_FourierTransform
