/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file MonteCarloIntegrator.hpp
//! @brief MonteCarloIntegrator: Monte Carlo integration over a box of any dimension.
#ifndef CAP__MATH__MonteCarloIntegrator
#define CAP__MATH__MonteCarloIntegrator
#include <cstdint>
#include <memory>
#include "Integrator.hpp"

namespace CAP
{
  namespace MATH
  {

  class MersenneTwister;

  //!
  //! @brief Monte Carlo integration: the integral over a box of volume V is V times the mean of the integrand at points
  //! drawn uniformly in the box, with the statistical error V sigma/sqrt(N).
  //!
  //! The error falls as 1/sqrt(N) whatever the number of variables, whereas the product rules (trapezoid, Simpson,
  //! Gauss-Legendre) need a number of points growing as N^n: Monte Carlo is the method of choice beyond 3 or 4
  //! variables, or for integrands that are not smooth (with steps, e.g. the indicator of a region).
  //!
  //! Adaptive (default): batches of samples (initialSamples(), then doubling) are drawn until the statistical error is
  //! within the tolerance, or the number of samples reaches maxSamples() (converged is then false). With setSamples(n),
  //! n samples are drawn once. Functions of rank 1, 2 or n are integrated the same way. The random numbers come from a
  //! Mersenne twister of its own, seeded by setSeed() (each integrate() continues its sequence; a copy has its own).
  //!
  //! @code
  //!   CAP::MATH::MonteCarloIntegrator mc(1.0E-3);                    // 0.1% statistical accuracy
  //!   mc.setSeed(12345);
  //!   CAP::MATH::IntegralResult r = mc.integrate(
  //!     [](const std::vector<double> & x) { double s = 0; for (double v : x) s += v*v; return s < 1 ? 1.0 : 0.0; },
  //!     std::vector<double>(5, -1.0), std::vector<double>(5, 1.0));   // volume of the 5-ball, 8 pi^2/15
  //! @endcode
  //!
  class MonteCarloIntegrator : public Integrator
  {
  public:

  //! @brief Integrator. @param relativeTolerance relative statistical accuracy @param maxSamples maximum number of samples
  //! @param seed seed of the random numbers
  MonteCarloIntegrator(double relativeTolerance=1.0E-3, long maxSamples=100000000, std::uint64_t seed=5489);
  //! @brief Copy (with a copy of the generator, at the same point of its sequence). @param source integrator
  MonteCarloIntegrator(const MonteCarloIntegrator & source);
  //! @brief Assignment. @param rhs integrator @return this
  MonteCarloIntegrator & operator=(const MonteCarloIntegrator & rhs);
  //! @brief Destructor.
  ~MonteCarloIntegrator() override;
  //! @brief Copy. @return new integrator (owned by the caller)
  MonteCarloIntegrator * clone() const override { return new MonteCarloIntegrator(*this); }
  //! @brief Name. @return "MonteCarlo"
  String name() const override { return "MonteCarlo"; }
  //! @brief Integral of g over [low, high]. @param g integrand @param low lower bound @param high upper bound
  //! @return result (error: statistical, one standard deviation) @throws MathException if a bound is not finite
  IntegralResult integrate(const Integrand1D & g, double low, double high) const override;
  //! @brief Integral of g over a box. @param g integrand @param low lower bounds @param high upper bounds
  //! @return result (error: statistical) @throws MathException if the sizes differ or a bound is not finite
  IntegralResult integrate(const IntegrandND & g, const std::vector<double> & low,
                           const std::vector<double> & high) const override;
  using Integrator::integrate;

  //! @brief Seed the random numbers. @param seed seed
  void setSeed(std::uint64_t seed);
  //! @brief Draw a fixed number of samples (0: adaptive). @param n samples
  void setSamples(long n) { _samples = n; }
  //! @brief Fixed number of samples (0: adaptive). @return samples
  long samples() const { return _samples; }
  //! @brief Set the samples of the first batch (adaptive). @param n samples
  void setInitialSamples(long n) { _initialSamples = n > 1 ? n : 2; }
  //! @brief Samples of the first batch. @return samples
  long initialSamples() const { return _initialSamples; }
  //! @brief Set the maximum number of samples (adaptive). @param n samples
  void setMaxSamples(long n) { _maxSamples = n; }
  //! @brief Maximum number of samples. @return samples
  long maxSamples() const { return _maxSamples; }

  protected:

  long _samples;          //!< fixed number of samples (0: adaptive)
  long _initialSamples;   //!< samples of the first batch
  long _maxSamples;       //!< maximum number of samples
  std::unique_ptr<MersenneTwister> _generator;   //!< random numbers (advanced by integrate())
  };

  } // namespace MATH
} // namespace CAP

#endif // CAP__MATH__MonteCarloIntegrator
