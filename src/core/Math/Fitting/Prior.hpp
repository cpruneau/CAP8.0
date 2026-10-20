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
//! @file Prior.hpp
//! @brief Prior: the prior density of a parameter in a Bayesian fit.
#ifndef CAP__MATH__Prior
#define CAP__MATH__Prior
#include <functional>
#include "Aliases.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Prior density of one parameter in a Bayesian fit (see BayesianFitter), given by its logarithm.
  //!
  //!  - flat(): constant (improper); within the bounds of the parameter if it has some (the default);
  //!  - uniform(low, high): constant over [low, high], zero outside;
  //!  - gaussian(mean, sigma): e.g. the result of an earlier measurement;
  //!  - logUniform(low, high): uniform in log(x) over [low, high] (x > 0), for a scale whose order of magnitude is
  //!    unknown;
  //!  - custom(logDensity): any log density.
  //!
  class Prior
  {
  public:

  //! @brief Types of prior.
  enum Type { kFlat, kUniform, kGaussian, kLogUniform, kCustom };

  //! @brief Flat prior.
  Prior() : _type(kFlat), _a(0), _b(0), _custom(), _name("flat") { }
  //! @brief Flat prior (improper). @return prior
  static Prior flat() { return Prior(); }
  //! @brief Uniform prior over [low, high]. @param low lower end @param high upper end @return prior
  static Prior uniform(double low, double high);
  //! @brief Gaussian prior. @param mean mean @param sigma width (> 0) @return prior
  static Prior gaussian(double mean, double sigma);
  //! @brief Log-uniform prior over [low, high] (0 < low < high). @param low lower end @param high upper end @return prior
  static Prior logUniform(double low, double high);
  //! @brief Prior of any log density. @param logDensity log of the (possibly unnormalized) density @param name name
  //! @return prior
  static Prior custom(std::function<double(double)> logDensity, const String & name="custom");

  //! @brief Type. @return type
  Type type() const { return _type; }
  //! @brief Name (e.g. "gaussian(1, 0.5)"). @return name
  const String & name() const { return _name; }
  //! @brief Log of the density at a value. @param x value @return log density (-infinity outside the support)
  double logDensity(double x) const;

  protected:

  Type   _type;   //!< type
  double _a;   //!< low, or mean
  double _b;   //!< high, or sigma
  std::function<double(double)> _custom;   //!< custom log density
  String _name;   //!< name
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__Prior
