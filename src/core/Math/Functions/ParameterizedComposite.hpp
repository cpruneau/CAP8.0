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
//! @file ParameterizedComposite.hpp
//! @brief ParameterizedComposite: sum of parameterized functions with all their parameters, for fits.
#ifndef CAP__MATH__ParameterizedComposite
#define CAP__MATH__ParameterizedComposite
#include <memory>
#include <vector>
#include "ParameterizedFunction.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Sum of parameterized functions, f(x) = f_0(x; p_0) + f_1(x; p_1) + ..., whose parameters are those of all
  //! its components: a model to fit, e.g. a signal over a background.
  //!
  //! Each component is a copy (ParameterizedFunction::clone()) of the function given to add(), with a label; its
  //! parameters are appended to those of the composite, named "label.name" (e.g. "signal.mean"), with their values,
  //! types and bounds. The parameters of the composite are those that count: they are given to the components before
  //! each evaluation. derivativeAt() and integralInRange() are the sums of those of the components (analytical when
  //! they are); adaptToRange() is passed on to the components.
  //!
  //! A composite is a sum (kSum, the default: all components have the rank of the composite) or a product (kProduct:
  //! each component acts on its own variables, in the order the components were added, and the rank is the sum of
  //! theirs; e.g. f(x) g(y), a function of two variables made of two of one). The integral of a product over a box is
  //! the product of those of its components.
  //!
  //! Unlike CompositeFunction (a sum of functions without parameters, which it does not own), a composite owns its
  //! components and can be fitted, copied, stored with a fit in a CAP file, and registered under a name with
  //! FunctionRegistry::registerComposite(), which makes it available in the Fit window of CapGui.
  //!
  //! @code
  //!   CAP::MATH::ParameterizedComposite model;
  //!   model.add("signal", CAP::MATH::Normal(100.0, 0.0, 1.0));
  //!   model.add("background", CAP::MATH::Linear(10.0, 0.0));
  //!   CAP::MATH::FunctionRegistry::registerComposite("Gauss+Linear", model);
  //! @endcode
  //!
  class ParameterizedComposite : public ParameterizedFunction
  {
  public:

  //! @brief How the components are combined.
  enum Operation { kSum, kProduct };

  //! @brief Empty composite (rank 1 until a component is added). @param operation sum or product
  explicit ParameterizedComposite(Operation operation=kSum);
  //! @brief Product f(x) g(y) of two functions of one variable each (copied). @param labelX label of f
  //! @param f function of x @param labelY label of g @param g function of y @return composite of rank 2
  static ParameterizedComposite product(const String & labelX, const ParameterizedFunction & f,
                                        const String & labelY, const ParameterizedFunction & g);
  //! @brief Copy (the components are copied). @param source composite
  ParameterizedComposite(const ParameterizedComposite & source);
  //! @brief Assignment (the components are copied). @param rhs composite @return this composite
  ParameterizedComposite & operator=(const ParameterizedComposite & rhs);
  //! @brief Destructor.
  virtual ~ParameterizedComposite() { }
  //! @brief Copy. @return new composite (owned by the caller)
  ParameterizedComposite * clone() const override { return new ParameterizedComposite(*this); }

  //!
  //! @brief Add a copy of a function to the sum, its parameters appended (named "label.name").
  //! @param label label of the component (not empty, unique) @param function function (copied; in a product, it
  //! acts on the next variables) @throws MathException if the label is empty or already used, the rank differs from
  //! that of the other components of a sum, or the function cannot be copied
  //!
  void add(const String & label, const ParameterizedFunction & function);
  //! @brief How the components are combined. @return sum or product
  Operation operation() const { return _operation; }
  //! @brief Number of components. @return number
  unsigned int nComponents() const { return (unsigned int)(_components.size()); }
  //! @brief Component k (with the parameters of the composite). @param k index @return component
  //! @throws MathException if k is out of range
  const ParameterizedFunction & component(unsigned int k) const;
  //! @brief Label of component k. @param k index @return label @throws MathException if k is out of range
  const String & label(unsigned int k) const;
  //! @brief Index of the first parameter of component k among those of the composite. @param k index @return index
  unsigned int firstParameter(unsigned int k) const;

  //! @brief Value of a composite of rank 1. @param x x @return sum of the components
  double evaluateAt(double x) const override;
  //! @brief Value at a point of any rank. @param x coordinates @return sum of the components
  double evaluateAt(const std::vector<double> & x) const override;
  using ParameterizedFunction::evaluateAt;
  //! @brief Derivative (rank 1): sum of those of the components. @param x x @return derivative
  double derivativeAt(double x) const override;
  //! @brief Integral (rank 1): sum of those of the components. @param low lower end @param high upper end @return integral
  double integralInRange(double low, double high) const override;
  //! @brief Integral over a box: sum (or product, for a product) of those of the components.
  //! @param low lower ends @param high upper ends @return integral
  double integralInRange(const std::vector<double> & low, const std::vector<double> & high) const override;
  using ParameterizedFunction::integralInRange;
  //! @brief Pass the range on to the components (fixed parameters they change are changed in the composite).
  //! @param low lower end @param high upper end
  void adaptToRange(double low, double high) override;
  //! @brief Pass a box on to the components (in a product, each its own variables).
  //! @param low lower ends @param high upper ends
  void adaptToRange(const std::vector<double> & low, const std::vector<double> & high) override;

  //! @brief Name under which the composite is registered (see FunctionRegistry::registerComposite()). @return name
  String registryName() const override { return _registryName; }
  //! @brief Set the name under which the composite is registered. @param name name
  void setRegistryName(const String & name) { _registryName = name; }

  protected:

  //! @brief Give the parameters of the composite to the components (when they changed).
  void synchronize() const;
  //! @brief The variables of component k in a product. @param x all the variables @param k component @return its variables
  std::vector<double> slice(const std::vector<double> & x, unsigned int k) const;
  //! @brief Fixed parameters changed in the components (e.g. by adaptToRange()) are changed in the composite.
  void pullFixedParameters();

  Operation _operation;   //!< sum or product
  std::vector<unsigned int> _offsets;   //!< in a product, index of the first variable of each component

  std::vector<std::unique_ptr<ParameterizedFunction>> _components;   //!< components (owned)
  std::vector<String> _labels;   //!< labels of the components
  std::vector<unsigned int> _first;   //!< index of the first parameter of each component
  String _registryName;   //!< name in FunctionRegistry (empty if not registered)
  mutable std::vector<double> _synchronized;   //!< values last given to the components
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__ParameterizedComposite
