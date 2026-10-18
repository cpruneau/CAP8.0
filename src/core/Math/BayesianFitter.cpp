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
//! @file BayesianFitter.cpp
//! @brief Implementation of BayesianFitter.
#include <cmath>
#include <limits>
#include "BayesianFitter.hpp"
#include "MathExceptions.hpp"
#include "ParameterizedFunction.hpp"

namespace CAP
{
  namespace MATH
  {

  BayesianFitter::BayesianFitter(Likelihood & likelihood)
  :
  _owned(),
  _likelihood(&likelihood),
  _priors(likelihood.function().nParameters()),
  _sampler(),
  _startFromFit(true),
  _free()
  { }

  BayesianFitter::BayesianFitter(ParameterizedFunction & function, const DataPoints & data, Fitter::Statistic statistic)
  :
  _owned(statistic == Fitter::kPoisson ? static_cast<Likelihood*>(new PoissonLikelihood(function, data))
                                       : static_cast<Likelihood*>(new ChiSquareLikelihood(function, data))),
  _likelihood(nullptr),
  _priors(function.nParameters()),
  _sampler(),
  _startFromFit(true),
  _free()
  {
  _likelihood = _owned.get();
  }

  void BayesianFitter::setPrior(unsigned int index, const Prior & prior)
  {
  if (index >= _priors.size()) throw MathException("Parameter index out of range", __FUNCTION__);
  _priors[index] = prior;
  }

  const Prior & BayesianFitter::prior(unsigned int index) const
  {
  if (index >= _priors.size()) throw MathException("Parameter index out of range", __FUNCTION__);
  return _priors[index];
  }

  double BayesianFitter::logPosterior(const std::vector<double> & values)
  {
  const double minusInfinity = -std::numeric_limits<double>::infinity();
  ParameterizedFunction & f = _likelihood->function();
  double logPrior = 0;
  for (unsigned int j=0; j<_free.size(); j++)
    {
    const Parameter & p = f.parameter(_free[j]);
    double v = values[j];
    if ((p.isLowConstrained() && v < p.minimum()) || (p.isHighConstrained() && v > p.maximum())) return minusInfinity;
    double l = _priors[_free[j]].logDensity(v);
    if (!std::isfinite(l)) return minusInfinity;
    logPrior += l;
    f.setParameterValue(_free[j], v);
    }
  double s = _likelihood->statistic();
  return std::isfinite(s) ? logPrior - 0.5*s : minusInfinity;
  }

  BayesianResult BayesianFitter::sample()
  {
  BayesianResult result;
  ParameterizedFunction & f = _likelihood->function();
  _free = f.freeParameters();
  if (_free.empty()) { result.status = "no free parameter"; return result; }
  _likelihood->prepare();
  const unsigned int m = (unsigned int)(_free.size());
  // start: a fit (its uncertainties size the first proposals), or the current values
  std::vector<double> start(m), scales(m);
  for (unsigned int j=0; j<m; j++)
    {
    start[j] = f.parameter(_free[j]).value();
    scales[j] = 0.1*std::max(std::fabs(start[j]), 1E-3);
    }
  if (_startFromFit)
    {
    Fitter fitter(*_likelihood);
    FitResult fit = fitter.fit();
    for (unsigned int j=0; j<m; j++)
      {
      start[j] = fit.values[_free[j]];
      if (fit.validErrors && fit.errors[_free[j]] > 0) scales[j] = fit.errors[_free[j]];
      }
    }
  if (!std::isfinite(logPosterior(start)))
    { result.status = "the posterior density is zero at the start point (check the priors and the bounds)"; return result; }
  result.chain = _sampler.sample([this](const std::vector<double> & p) { return logPosterior(p); }, start, scales);
  result.valid = true;
  result.freeParameters = _free;
  result.acceptance = result.chain.acceptance;
  result.correlation = result.chain.correlation();
  result.mode = f.parameterValues();
  std::vector<double> mode = result.chain.mode();
  const unsigned int n = f.nParameters();
  result.means = result.standardDeviations = result.medians = result.low68 = result.high68 = result.low95 = result.high95 = f.parameterValues();
  for (unsigned int k=0; k<n; k++) result.standardDeviations[k] = 0;
  for (unsigned int j=0; j<m; j++)
    {
    unsigned int k = _free[j];
    result.means[k] = result.chain.mean(j);
    result.standardDeviations[k] = result.chain.standardDeviation(j);
    result.medians[k] = result.chain.median(j);
    auto i68 = result.chain.interval(j, 0.6827), i95 = result.chain.interval(j, 0.9545);
    result.low68[k] = i68.first; result.high68[k] = i68.second;
    result.low95[k] = i95.first; result.high95[k] = i95.second;
    result.mode[k] = mode[j];
    result.effectiveSampleSizes.push_back(result.chain.effectiveSampleSize(j));
    result.rHats.push_back(result.chain.rHat(j));
    }
  // the function is left at the posterior means (within the bounds)
  std::vector<double> means(m);
  for (unsigned int j=0; j<m; j++) means[j] = result.means[_free[j]];
  logPosterior(means);
  bool mixed = true;
  for (double r : result.rHats) mixed = mixed && !(r > 1.05);
  result.status = mixed ? "sampled" : "sampled, but the chains disagree (R-hat above 1.05): more burn-in or samples are needed";
  return result;
  }

  } // namespace MATH
} // namespace CAP
