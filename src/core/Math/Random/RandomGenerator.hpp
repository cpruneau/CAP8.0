/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file RandomGenerator.hpp
//! @brief RandomGenerator: random numbers of the common distributions, from any engine of uniform numbers.
#ifndef CAP__RandomGenerator
#define CAP__RandomGenerator
#include <cstdint>
#include <limits>
#include <vector>

namespace CAP
{
  namespace MATH
  {

  class RandomDistribution;

  //!
  //! @brief Random numbers of the common distributions, drawn from the uniform numbers of an engine (independent of ROOT).
  //!
  //! An engine (e.g. MersenneTwister) implements random(), a double uniformly distributed in [0,1), and setSeed().
  //! Every distribution is written in terms of random(), so a new engine gets all of them, and a new distribution
  //! works with every engine: add a member function here, or derive a RandomDistribution and use sample().
  //!
  //! @code
  //!   CAP::MATH::MersenneTwister generator(12345);
  //!   double x = generator.random();               // [0,1)
  //!   double y = generator.gauss(0.0, 2.0);        // mean 0, rms 2
  //!   long   n = generator.poisson(7.5);
  //!   double t = generator.exp(1.0, 0.0, 5.0);     // exponential of mean 1, truncated to [0,5]
  //! @endcode
  //!
  //! Methods:
  //!  - gauss(): Box-Muller (the second number of each pair is kept for the next call);
  //!  - exp(): inversion of the (truncated) exponential distribution;
  //!  - poisson(): multiplication of uniform numbers for means below 30, transformed rejection (Hoermann's PTRS) above;
  //!  - binomial(): inversion when n min(p,1-p) < 30, otherwise Knuth's reduction through beta numbers first;
  //!  - gamma(): Marsaglia and Tsang; beta() and chiSquare() from gamma numbers;
  //!  - polynomial(): inversion of the integral of the polynomial (a probability density on [minimum, maximum]).
  //!
  class RandomGenerator
  {
  public:

  //! @brief Generator with no Gaussian number kept.
  RandomGenerator() : _hasGauss(false), _gauss(0.0) { }
  //! @brief Destructor.
  virtual ~RandomGenerator() { }

  //! @brief Random number uniformly distributed in [0,1). @return number
  virtual double random() = 0;
  //! @brief Start the sequence again from a seed (0: a seed of its own, different at each call). @param seed seed
  virtual void setSeed(std::uint64_t seed) = 0;

  //! @brief Random number uniformly distributed in [low, high). @param low low edge @param high high edge @return number
  double uniform(double low, double high) { return low + (high - low)*random(); }
  //! @brief Random integer uniformly distributed in [low, high] (both included). @param low lowest @param high highest
  //! @return number
  long uniformInteger(long low, long high);
  //! @brief Random number in (0,1): never 0 (for logarithms). @return number
  double positive();

  //! @brief Gaussian random number (Box-Muller). @param mean mean @param rms standard deviation @return number
  double gauss(double mean=0.0, double rms=1.0);
  //! @brief Exponential random number, of density exp(-x/mean) restricted to [minimum, maximum].
  //! @param mean mean of the exponential (before the restriction; > 0) @param minimum lowest value
  //! @param maximum highest value (infinite: no upper limit) @return number
  //! @throws MathException if mean <= 0 or maximum <= minimum
  double exp(double mean, double minimum=0.0, double maximum=std::numeric_limits<double>::infinity());
  //! @brief Poisson random number. @param mean mean (>= 0) @return number @throws MathException if mean < 0
  long poisson(double mean);
  //! @brief Binomial random number: successes in n trials of probability p. @param n trials (>= 0)
  //! @param p probability of success, in [0,1] @return number @throws MathException if n < 0 or p is outside [0,1]
  long binomial(long n, double p);
  //! @brief Gamma random number, of density x^(shape-1) exp(-x/scale). @param shape shape (> 0) @param scale scale (> 0)
  //! @return number @throws MathException if shape or scale <= 0
  double gamma(double shape, double scale=1.0);
  //! @brief Beta random number in [0,1], of density x^(a-1) (1-x)^(b-1). @param a first parameter (> 0)
  //! @param b second parameter (> 0) @return number
  double beta(double a, double b);
  //! @brief Chi-square random number. @param ndf number of degrees of freedom (> 0) @return number
  double chiSquare(double ndf);
  //!
  //! @brief Random number of density proportional to the polynomial c0 + c1 x + ... + cn x^n on [minimum, maximum].
  //! @param coefficients c0, c1, ..., cn @param minimum low edge @param maximum high edge @return number
  //! @throws MathException if the polynomial is negative somewhere on the interval, or its integral is not positive
  //!
  double polynomial(const std::vector<double> & coefficients, double minimum, double maximum);
  //! @brief Random number of any distribution (see RandomDistribution). @param distribution distribution @return number
  double sample(const RandomDistribution & distribution);

  protected:

  bool   _hasGauss;   //!< whether the second Gaussian number of the last pair is kept
  double _gauss;   //!< second Gaussian number of the last pair (standard normal)

  //! @brief Clear what the distributions keep between calls (to call when the engine is seeded).
  void resetDistributions() { _hasGauss = false; }
  //! @brief Binomial number by inversion (p <= 0.5, n p small). @param n trials @param p probability @return number
  long binomialInversion(long n, double p);
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__RandomGenerator
