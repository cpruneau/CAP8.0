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
//! @file ParameterizedComposite.cpp
//! @brief Implementation of ParameterizedComposite.
#include "ParameterizedComposite.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  ParameterizedComposite::ParameterizedComposite()
  :
  ParameterizedFunction(1),
  _components(),
  _labels(),
  _first(),
  _registryName(),
  _synchronized()
  { }

  ParameterizedComposite::ParameterizedComposite(const ParameterizedComposite & source)
  :
  ParameterizedFunction(source),
  _components(),
  _labels(source._labels),
  _first(source._first),
  _registryName(source._registryName),
  _synchronized()
  {
  for (const auto & component : source._components) _components.emplace_back(component->clone());
  }

  ParameterizedComposite & ParameterizedComposite::operator=(const ParameterizedComposite & rhs)
  {
  if (this == &rhs) return *this;
  std::vector<std::unique_ptr<ParameterizedFunction>> components;
  for (const auto & component : rhs._components) components.emplace_back(component->clone());
  ParameterizedFunction::operator=(rhs);
  _components.swap(components);
  _labels = rhs._labels;
  _first = rhs._first;
  _registryName = rhs._registryName;
  _synchronized.clear();
  return *this;
  }

  void ParameterizedComposite::add(const String & label, const ParameterizedFunction & function)
  {
  if (label.isNull()) throw MathException("A component of a composite needs a label", __FUNCTION__);
  for (const auto & existing : _labels)
    if (existing == label) throw MathException(String("The label ") + label + String(" is already used"), __FUNCTION__);
  if (!_components.empty() && function.rank() != rank()) throw MathException("The components of a composite must have the same rank", __FUNCTION__);
  std::unique_ptr<ParameterizedFunction> copy(function.clone());
  if (_components.empty()) setRank(function.rank());
  _first.push_back(nParameters());
  for (unsigned int k=0; k<copy->nParameters(); k++)
    {
    Parameter p = copy->parameter(k);
    p.setName(label + String(".") + p.name());
    addParameter(p);
    }
  _labels.push_back(label);
  _components.push_back(std::move(copy));
  _synchronized.clear();
  }

  const ParameterizedFunction & ParameterizedComposite::component(unsigned int k) const
  {
  if (k >= _components.size()) throw MathException("Component index out of range", __FUNCTION__);
  synchronize();
  return *_components[k];
  }

  const String & ParameterizedComposite::label(unsigned int k) const
  {
  if (k >= _labels.size()) throw MathException("Component index out of range", __FUNCTION__);
  return _labels[k];
  }

  unsigned int ParameterizedComposite::firstParameter(unsigned int k) const
  {
  if (k >= _first.size()) throw MathException("Component index out of range", __FUNCTION__);
  return _first[k];
  }

  void ParameterizedComposite::synchronize() const
  {
  std::vector<double> values = parameterValues();
  if (values == _synchronized) return;
  for (std::size_t c=0; c<_components.size(); c++)
    for (unsigned int k=0; k<_components[c]->nParameters(); k++)
      _components[c]->setParameter(k, _parameters[_first[c] + k]);   // value, type and bounds of the composite
  _synchronized = values;
  }

  double ParameterizedComposite::evaluateAt(double x) const
  {
  checkRank(1, __FUNCTION__);
  synchronize();
  double sum = 0;
  for (const auto & component : _components) sum += component->evaluateAt(x);
  return sum;
  }

  double ParameterizedComposite::evaluateAt(const std::vector<double> & x) const
  {
  checkRank((unsigned int)(x.size()), __FUNCTION__);
  synchronize();
  double sum = 0;
  for (const auto & component : _components) sum += component->evaluateAt(x);
  return sum;
  }

  double ParameterizedComposite::derivativeAt(double x) const
  {
  checkRank(1, __FUNCTION__);
  synchronize();
  double sum = 0;
  for (const auto & component : _components) sum += component->derivativeAt(x);
  return sum;
  }

  double ParameterizedComposite::integralInRange(double low, double high) const
  {
  checkRank(1, __FUNCTION__);
  synchronize();
  double sum = 0;
  for (const auto & component : _components) sum += component->integralInRange(low, high);
  return sum;
  }

  void ParameterizedComposite::adaptToRange(double low, double high)
  {
  synchronize();
  for (std::size_t c=0; c<_components.size(); c++)
    {
    _components[c]->adaptToRange(low, high);
    for (unsigned int k=0; k<_components[c]->nParameters(); k++)
      {
      Parameter & p = _parameters[_first[c] + k];
      if (p.type() == Parameter::Fixed) p.setValue(_components[c]->parameter(k).value());
      }
    }
  _synchronized.clear();
  }

  } // namespace MATH
} // namespace CAP
