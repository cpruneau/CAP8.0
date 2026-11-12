/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Derivative.hpp
//! @brief Derivative: the derivative of a function, as a function.
#ifndef CAP__MATH__Derivative
#define CAP__MATH__Derivative
#include <memory>
#include <vector>
#include "Function.hpp"
#include "Differentiation.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Derivative of a function, as a function of the same rank: the derivative of order n of f (rank 1), the
  //! partial derivative of order n along an axis, or a mixed partial derivative (one derivative along each axis
  //! listed). It is evaluated, plotted, integrated or differentiated again like any function.
  //!
  //! The derivative is computed as the Differentiation given says (Automatic by default): analytically when f
  //! provides it (Function::derivativeFunction(), applied once when the Derivative is made, and
  //! Function::derivativeAt()), numerically otherwise, by the derivative of an interpolating polynomial (Linear,
  //! Quadratic, Cubic, or of any degree). isAnalytical() tells which.
  //!
  //! f is referred to, not copied: it must outlive the Derivative, and changes to it (e.g. new parameter values after
  //! a fit) are seen, except by the analytical derivative made once (made again by update()). The constructors taking
  //! a std::shared_ptr share the ownership of f instead.
  //!
  //! @code
  //!   CAP::MATH::Normal g(1.0, 0.0, 1.0);
  //!   CAP::MATH::Derivative dg(g);                                            // g', analytical
  //!   double s = dg(1.0);                                                     // -exp(-1/2)/sqrt(2 pi)
  //!   CAP::MATH::Derivative d2(g, 2, 0, CAP::MATH::Differentiation(CAP::MATH::Differentiation::Cubic));   // g''
  //!   CAP::MATH::Derivative dxy(field, {0, 1});                               // d2f/dx dy
  //! @endcode
  //!
  class Derivative : public Function
  {
  public:

  //! @brief Derivative of order n along an axis. @param f function (referred to) @param order n (1 or more)
  //! @param axis axis (0 for rank 1) @param differentiation method
  //! @throws MathException if the axis is out of range, or the method is Analytical and f has no analytical derivative
  Derivative(const Function & f, unsigned int order=1, unsigned int axis=0,
             const Differentiation & differentiation=Differentiation());
  //! @brief Mixed partial derivative, one derivative along each axis listed. @param f function (referred to)
  //! @param axes axes @param differentiation method @throws MathException as the other constructor
  Derivative(const Function & f, const std::vector<unsigned int> & axes,
             const Differentiation & differentiation=Differentiation());
  //! @brief Derivative of order n along an axis, sharing the ownership of f. @param f function @param order n
  //! @param axis axis @param differentiation method
  Derivative(std::shared_ptr<const Function> f, unsigned int order=1, unsigned int axis=0,
             const Differentiation & differentiation=Differentiation());
  //! @brief Copy (shares f and the analytical derivative). @param source derivative
  Derivative(const Derivative & source) = default;
  //! @brief Assignment. @param rhs derivative @return this
  Derivative & operator=(const Derivative & rhs) = default;
  //! @brief Destructor.
  ~Derivative() override { }

  //! @name Values
  //! @{
  //! @brief Value at x (rank 1). @param x x @return derivative
  double evaluateAt(double x) const override;
  //! @brief Value at a point. @param x coordinates @return derivative
  double evaluateAt(const std::vector<double> & x) const override;
  using Function::evaluateAt;
  //! @brief Whether its own derivative is analytical. @return true if the derivative is analytical and so is its own
  bool hasAnalyticalDerivative() const override;
  //! @brief Derivative of the derivative (one more axis). @param axis axis @return new Derivative (owned by the caller)
  Function * derivativeFunction(unsigned int axis=0) const override;
  //! @}

  //! @name Properties
  //! @{
  //! @brief The function differentiated. @return f
  const Function & function() const { return *_f; }
  //! @brief Axes of the derivatives, in order. @return axes
  const std::vector<unsigned int> & axes() const { return _axes; }
  //! @brief Order of the derivative (number of axes). @return order
  unsigned int order() const { return unsigned(_axes.size()); }
  //! @brief Method. @return differentiation
  const Differentiation & differentiation() const { return _differentiation; }
  //! @brief Change the method (the analytical derivative is made again). @param differentiation method
  void setDifferentiation(const Differentiation & differentiation);
  //! @brief Whether the derivative is computed analytically (fully). @return true if so
  bool isAnalytical() const;
  //! @brief Make the analytical derivative again, from the current f (e.g. after its parameters changed).
  void update();
  //! @}

  protected:

  //! @brief Check the axes and make the analytical part.
  void initialize();

  std::shared_ptr<const Function> _owner;     //!< f, when the Derivative shares its ownership
  const Function * _f;                        //!< the function differentiated
  std::vector<unsigned int> _axes;            //!< axes of the derivatives
  Differentiation _differentiation;           //!< method
  std::shared_ptr<const Function> _analytic;  //!< f differentiated analytically along the first axes (null: none)
  std::vector<unsigned int> _rest;            //!< axes left to the last analytical derivative or numerical ones
  };

  } // namespace MATH
} // namespace CAP

#endif // CAP__MATH__Derivative
