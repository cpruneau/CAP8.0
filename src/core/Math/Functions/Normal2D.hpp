/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Normal2D.hpp
//! @brief Normal2D: Gaussian of two variables, with a correlation, for fits.
#ifndef CAP__MATH__Normal2D
#define CAP__MATH__Normal2D
#include "ParameterizedFunction.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Gaussian of two variables: f(x,y) = A exp(-[u^2 - 2 rho u v + v^2]/(2 (1 - rho^2))), with
  //! u = (x - meanX)/widthX and v = (y - meanY)/widthY.
  //!
  //! The parameters are "amplitude" (A, the value at the peak), "meanX", "meanY", "widthX", "widthY" (standard
  //! deviations; their signs do not matter) and "correlation" (rho, bounded to [-0.999, 0.999]). Its integral over the
  //! plane is volume() = 2 pi A widthX widthY sqrt(1 - rho^2); integralInRange() over a box is computed from it
  //! when rho = 0 (product of error functions), numerically otherwise.
  //!
  //! @code
  //!   CAP::MATH::Normal2D f(100.0, 0.0, 1.0, 0.5, 2.0, 0.3);
  //!   double peak = f.evaluateAt(0.0, 1.0);   // 100
  //! @endcode
  //!
  class Normal2D : public ParameterizedFunction
  {
  public:

  //! @brief Gaussian. @param amplitude A @param meanX mean of x @param meanY mean of y @param widthX standard
  //! deviation of x @param widthY standard deviation of y @param correlation rho, in (-1, 1)
  //! @throws MathException if a width is 0 or |rho| >= 1
  Normal2D(double amplitude=1.0, double meanX=0.0, double meanY=0.0, double widthX=1.0, double widthY=1.0, double correlation=0.0);
  //! @brief Destructor.
  virtual ~Normal2D() { }
  //! @brief Copy. @return new function (owned by the caller)
  Normal2D * clone() const override { return new Normal2D(*this); }

  //! @brief Value. @param x x @param y y @return f(x,y)
  double evaluateAt(double x, double y) const override;
  using ParameterizedFunction::evaluateAt;
  //! @brief Integral over the plane: 2 pi A widthX widthY sqrt(1 - rho^2). @return volume
  double volume() const;
  //! @brief Integral over the box [x1Low, x1High] x [x2Low, x2High]: analytical when rho = 0.
  //! @param x1Low left @param x1High right @param x2Low bottom @param x2High top @return integral
  double integralInRange(double x1Low, double x1High, double x2Low, double x2High) const override;
  using ParameterizedFunction::integralInRange;
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__Normal2D
