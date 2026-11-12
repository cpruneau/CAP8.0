/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file Derivative.cpp
//! @brief Implementation of Derivative.
#include "Derivative.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  Derivative::Derivative(const Function & f, unsigned int order, unsigned int axis, const Differentiation & differentiation)
  :
  Function(f.rank()),
  _owner(),
  _f(&f),
  _axes(order, axis),
  _differentiation(differentiation)
  {
  initialize();
  }

  Derivative::Derivative(const Function & f, const std::vector<unsigned int> & axes, const Differentiation & differentiation)
  :
  Function(f.rank()),
  _owner(),
  _f(&f),
  _axes(axes),
  _differentiation(differentiation)
  {
  initialize();
  }

  Derivative::Derivative(std::shared_ptr<const Function> f, unsigned int order, unsigned int axis,
                         const Differentiation & differentiation)
  :
  Function(f ? f->rank() : 1),
  _owner(f),
  _f(f.get()),
  _axes(order, axis),
  _differentiation(differentiation)
  {
  if (!_f) throw MathException("Null function", __FUNCTION__);
  initialize();
  }

  void Derivative::initialize()
  {
  for (unsigned int a : _axes) if (a >= _f->rank()) throw MathException("No such axis", "Derivative");
  setName(String("D") + _f->name());
  setTitle(String("Derivative of ") + _f->name());
  update();
  if (_differentiation.method() == Differentiation::Analytical && !isAnalytical())
    throw MathException("The function has no analytical derivative of this order", "Derivative");
  }

  void Derivative::update()
  {
  _analytic.reset();
  _rest = _axes;
  Differentiation::Method method = _differentiation.method();
  if (method != Differentiation::Automatic && method != Differentiation::Analytical) return;
  const Function * g = _f;
  std::size_t consumed = 0;
  while (consumed < _axes.size())
    {
    Function * d = g->derivativeFunction(_axes[consumed]);
    if (!d) break;
    _analytic.reset(d);
    g = d;
    consumed++;
    }
  _rest.assign(_axes.begin() + consumed, _axes.end());
  }

  void Derivative::setDifferentiation(const Differentiation & differentiation)
  {
  _differentiation = differentiation;
  update();
  }

  bool Derivative::isAnalytical() const
  {
  if (_rest.empty()) return true;
  const Function * g = _analytic ? _analytic.get() : _f;
  return _rest.size() == 1 && g->hasAnalyticalDerivative()
         && _differentiation.method() != Differentiation::Linear && _differentiation.method() != Differentiation::Quadratic
         && _differentiation.method() != Differentiation::Cubic && _differentiation.method() != Differentiation::Polynomial;
  }

  double Derivative::evaluateAt(double x) const
  {
  checkRank(1, __FUNCTION__);
  const Function * g = _analytic ? _analytic.get() : _f;
  if (_rest.empty()) return g->evaluateAt(x);
  return _differentiation.mixedDerivative(*g, std::vector<double>{x}, _rest);
  }

  double Derivative::evaluateAt(const std::vector<double> & x) const
  {
  checkRank(x.size(), __FUNCTION__);
  const Function * g = _analytic ? _analytic.get() : _f;
  if (_rest.empty()) return g->evaluateAt(x);
  return _differentiation.mixedDerivative(*g, x, _rest);
  }

  bool Derivative::hasAnalyticalDerivative() const
  {
  if (!_rest.empty()) return false;
  const Function * g = _analytic ? _analytic.get() : _f;
  return g->hasAnalyticalDerivative();
  }

  Function * Derivative::derivativeFunction(unsigned int axis) const
  {
  if (axis >= rank()) return nullptr;
  std::vector<unsigned int> axes(_axes);
  axes.push_back(axis);
  Derivative * d = new Derivative(*this);
  d->_axes = axes;
  d->update();
  // numerical derivatives of a numerical derivative lose accuracy: only when the result is analytical
  if (!d->isAnalytical()) { delete d; return nullptr; }
  return d;
  }

  } // namespace MATH
} // namespace CAP
