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

  ParameterizedComposite::ParameterizedComposite(Operation operation)
  :
  ParameterizedFunction(1),
  _operation(operation),
  _offsets(),
  _components(),
  _labels(),
  _first(),
  _registryName(),
  _synchronized()
  { }

  ParameterizedComposite::ParameterizedComposite(const ParameterizedComposite & source)
  :
  ParameterizedFunction(source),
  _operation(source._operation),
  _offsets(source._offsets),
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
  _operation = rhs._operation;
  _offsets = rhs._offsets;
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
  if (_operation == kSum && !_components.empty() && function.rank() != rank())
    throw MathException("The components of a sum must have the same rank", __FUNCTION__);
  std::unique_ptr<ParameterizedFunction> copy(function.clone());
  if (_operation == kProduct)
    {
    _offsets.push_back(_components.empty() ? 0u : rank());
    setRank(_components.empty() ? function.rank() : rank() + function.rank());
    }
  else if (_components.empty()) setRank(function.rank());
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
  if (_operation == kProduct)
    {
    double product = 1;
    for (const auto & component : _components) product *= component->evaluateAt(x);
    return product;
    }
  double sum = 0;
  for (const auto & component : _components) sum += component->evaluateAt(x);
  return sum;
  }

  std::vector<double> ParameterizedComposite::slice(const std::vector<double> & x, unsigned int k) const
  {
  return std::vector<double>(x.begin() + _offsets[k], x.begin() + _offsets[k] + _components[k]->rank());
  }

  double ParameterizedComposite::evaluateAt(const std::vector<double> & x) const
  {
  checkRank((unsigned int)(x.size()), __FUNCTION__);
  synchronize();
  if (_operation == kProduct)
    {
    double product = 1;
    for (unsigned int k=0; k<_components.size(); k++)
      {
      const ParameterizedFunction & c = *_components[k];
      product *= c.rank() == 1 ? c.evaluateAt(x[_offsets[k]]) : c.evaluateAt(slice(x, k));
      }
    return product;
    }
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

  double ParameterizedComposite::integralInRange(const std::vector<double> & low, const std::vector<double> & high) const
  {
  checkRank((unsigned int)(low.size()), __FUNCTION__);
  checkRank((unsigned int)(high.size()), __FUNCTION__);
  synchronize();
  if (_operation == kProduct)
    {
    double product = 1;
    for (unsigned int k=0; k<_components.size(); k++) product *= _components[k]->integralInRange(slice(low, k), slice(high, k));
    return product;
    }
  double sum = 0;
  for (const auto & component : _components) sum += component->integralInRange(low, high);
  return sum;
  }

  void ParameterizedComposite::pullFixedParameters()
  {
  for (std::size_t c=0; c<_components.size(); c++)
    for (unsigned int k=0; k<_components[c]->nParameters(); k++)
      {
      Parameter & p = _parameters[_first[c] + k];
      if (p.type() == Parameter::Fixed) p.setValue(_components[c]->parameter(k).value());
      }
  _synchronized.clear();
  }

  void ParameterizedComposite::adaptToRange(double low, double high)
  {
  synchronize();
  for (auto & component : _components) component->adaptToRange(low, high);
  pullFixedParameters();
  }

  void ParameterizedComposite::adaptToRange(const std::vector<double> & low, const std::vector<double> & high)
  {
  if (low.size() != rank() || high.size() != rank()) return;
  synchronize();
  for (unsigned int k=0; k<_components.size(); k++)
    {
    if (_operation == kProduct) _components[k]->adaptToRange(slice(low, k), slice(high, k));
    else _components[k]->adaptToRange(low, high);
    }
  pullFixedParameters();
  }

  ParameterizedComposite ParameterizedComposite::product(const String & labelX, const ParameterizedFunction & f,
                                                         const String & labelY, const ParameterizedFunction & g)
  {
  ParameterizedComposite c(kProduct);
  c.add(labelX, f);
  c.add(labelY, g);
  return c;
  }

  } // namespace MATH
} // namespace CAP
