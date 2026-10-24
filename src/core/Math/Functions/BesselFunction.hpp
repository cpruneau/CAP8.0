/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file BesselFunction.hpp
//! @brief BesselFunction: Bessel functions with an amplitude, a scale and a shift, for fits.
#ifndef CAP__MATH__BesselFunction
#define CAP__MATH__BesselFunction
#include "ParameterizedFunction.hpp"
#include "Bessel.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Bessel function of integer order n, with parameters, for fits: f(x) = A Z_n(k (x - x0)).
  //!
  //! Z is J (first kind), Y (second kind), I (modified, first kind) or K (modified, second kind), set by the class:
  //! BesselJFunction, BesselYFunction, BesselIFunction, BesselKFunction. The parameters are "amplitude" (A),
  //! "scale" (k), "shift" (x0), and "order" (n, fixed: it is rounded to an integer >= 0, and should stay fixed in a
  //! fit). Y and K are defined for k (x - x0) > 0 only (NaN elsewhere: such points are left out of a fit).
  //! Unlike Bessel (orders 0 and 1 for J and Y), any order is available: J_n by Miller's downward recurrence where
  //! the argument is below n, upward recurrence otherwise; Y_n by upward recurrence; I_n and K_n as in Bessel.
  //!
  //! @code
  //!   CAP::MATH::BesselJFunction f(2, 3.0, 1.5);   // 3 J_2(1.5 x)
  //!   double y = f.evaluateAt(1.0);
  //! @endcode
  //!
  class BesselFunction : public ParameterizedFunction
  {
  public:

  //! @brief Bessel function. @param type kind (BesselJ, BesselY, BesselI or BesselK) @param order order n
  //! @param amplitude A @param scale k @param shift x0 @throws MathException for a Struve type
  BesselFunction(Bessel::BesselType type, unsigned int order=0, double amplitude=1.0, double scale=1.0, double shift=0.0);
  //! @brief Destructor.
  virtual ~BesselFunction() { }

  //! @brief Kind of the function. @return kind
  Bessel::BesselType type() const { return _type; }
  //! @brief Order n (the parameter "order", rounded, at least 0). @return order
  unsigned int order() const;
  //! @brief Value. @param x x @return A Z_n(k (x - x0)), or NaN where it is not defined
  double evaluateAt(double x) const override;
  using ParameterizedFunction::evaluateAt;
  //! @brief Z_n(u) for any integer order. @param type kind @param n order @param u argument @return value, NaN where
  //! it is not defined (u <= 0 for Y and K)
  static double value(Bessel::BesselType type, unsigned int n, double u);

  protected:

  Bessel::BesselType _type;   //!< kind
  };

  //! @brief A J_n(k (x - x0)): Bessel function of the first kind.
  class BesselJFunction : public BesselFunction
  {
  public:
  //! @brief Function. @param order n @param amplitude A @param scale k @param shift x0
  explicit BesselJFunction(unsigned int order=0, double amplitude=1.0, double scale=1.0, double shift=0.0)
  : BesselFunction(Bessel::BesselJ, order, amplitude, scale, shift) { }
  //! @brief Copy. @return new function (owned by the caller)
  BesselJFunction * clone() const override { return new BesselJFunction(*this); }
  };

  //! @brief A Y_n(k (x - x0)): Bessel function of the second kind.
  class BesselYFunction : public BesselFunction
  {
  public:
  //! @brief Function. @param order n @param amplitude A @param scale k @param shift x0
  explicit BesselYFunction(unsigned int order=0, double amplitude=1.0, double scale=1.0, double shift=0.0)
  : BesselFunction(Bessel::BesselY, order, amplitude, scale, shift) { }
  //! @brief Copy. @return new function (owned by the caller)
  BesselYFunction * clone() const override { return new BesselYFunction(*this); }
  };

  //! @brief A I_n(k (x - x0)): modified Bessel function of the first kind.
  class BesselIFunction : public BesselFunction
  {
  public:
  //! @brief Function. @param order n @param amplitude A @param scale k @param shift x0
  explicit BesselIFunction(unsigned int order=0, double amplitude=1.0, double scale=1.0, double shift=0.0)
  : BesselFunction(Bessel::BesselI, order, amplitude, scale, shift) { }
  //! @brief Copy. @return new function (owned by the caller)
  BesselIFunction * clone() const override { return new BesselIFunction(*this); }
  };

  //! @brief A K_n(k (x - x0)): modified Bessel function of the second kind.
  class BesselKFunction : public BesselFunction
  {
  public:
  //! @brief Function. @param order n @param amplitude A @param scale k @param shift x0
  explicit BesselKFunction(unsigned int order=0, double amplitude=1.0, double scale=1.0, double shift=0.0)
  : BesselFunction(Bessel::BesselK, order, amplitude, scale, shift) { }
  //! @brief Copy. @return new function (owned by the caller)
  BesselKFunction * clone() const override { return new BesselKFunction(*this); }
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__BesselFunction
