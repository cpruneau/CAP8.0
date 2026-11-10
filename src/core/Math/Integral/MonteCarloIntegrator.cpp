/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file MonteCarloIntegrator.cpp
//! @brief Implementation of MonteCarloIntegrator.
#include <algorithm>
#include <cmath>
#include "MonteCarloIntegrator.hpp"
#include "MersenneTwister.hpp"

namespace CAP
{
  namespace MATH
  {

  MonteCarloIntegrator::MonteCarloIntegrator(double relativeTolerance, long maxSamples, std::uint64_t seed)
  :
  Integrator(relativeTolerance, 0.0, 30),
  _samples(0),
  _initialSamples(1000),
  _maxSamples(maxSamples),
  _generator(new MersenneTwister(seed))
  { }

  MonteCarloIntegrator::MonteCarloIntegrator(const MonteCarloIntegrator & source)
  :
  Integrator(source),
  _samples(source._samples),
  _initialSamples(source._initialSamples),
  _maxSamples(source._maxSamples),
  _generator(new MersenneTwister(*source._generator))
  { }

  MonteCarloIntegrator & MonteCarloIntegrator::operator=(const MonteCarloIntegrator & rhs)
  {
  if (this != &rhs)
    {
    Integrator::operator=(rhs);
    _samples = rhs._samples;
    _initialSamples = rhs._initialSamples;
    _maxSamples = rhs._maxSamples;
    _generator.reset(new MersenneTwister(*rhs._generator));
    }
  return *this;
  }

  MonteCarloIntegrator::~MonteCarloIntegrator() { }

  void MonteCarloIntegrator::setSeed(std::uint64_t seed)
  {
  _generator->setSeed(seed);
  }

  IntegralResult MonteCarloIntegrator::integrate(const Integrand1D & g, double low, double high) const
  {
  return integrate(IntegrandND([&g](const std::vector<double> & x) { return g(x[0]); }),
                   std::vector<double>{low}, std::vector<double>{high});
  }

  IntegralResult MonteCarloIntegrator::integrate(const IntegrandND & g, const std::vector<double> & low,
                                                 const std::vector<double> & high) const
  {
  checkBounds(low, high, __FUNCTION__);
  IntegralResult result;
  const std::size_t rank = low.size();
  double volume = 1.0;
  for (std::size_t k=0; k<rank; k++) volume *= high[k] - low[k];
  if (volume == 0.0) return result;
  std::vector<double> x(rank);
  // running mean and sum of squared deviations (Welford), stable for any number of samples
  double mean = 0.0, m2 = 0.0;
  long n = 0;
  auto draw = [&](long count)
    {
    for (long i=0; i<count; i++)
      {
      for (std::size_t k=0; k<rank; k++) x[k] = low[k] + (high[k] - low[k])*_generator->random();
      double v = g(x);
      n++;
      double delta = v - mean;
      mean += delta/double(n);
      m2 += delta*(v - mean);
      }
    };
  auto error = [&]() { return n > 1 ? std::fabs(volume)*std::sqrt(m2/double(n - 1)/double(n)) : 0.0; };
  if (_samples > 0)
    draw(_samples);
  else
    {
    draw(std::min(_initialSamples, _maxSamples));
    result.converged = accepted(error(), volume*mean);
    while (!result.converged && n < _maxSamples)
      {
      draw(std::min(n, _maxSamples - n));      // doubling
      result.converged = accepted(error(), volume*mean);
      }
    }
  result.value = volume*mean;
  result.error = error();
  result.nEvaluations = n;
  if (_samples > 0) result.converged = true;
  return result;
  }

  } // namespace MATH
} // namespace CAP
