/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file RandomDistribution.hpp
//! @brief RandomDistribution: base of the distributions added to those of RandomGenerator; TabulatedDistribution.
#ifndef CAP__RandomDistribution
#define CAP__RandomDistribution
#include <functional>
#include <vector>

namespace CAP
{
  namespace MATH
  {

  class RandomGenerator;

  //!
  //! @brief A distribution of random numbers, drawn from the uniform numbers of any generator: the way to add
  //! distributions to those of RandomGenerator without changing it.
  //!
  //! @code
  //!   class Cauchy : public CAP::MATH::RandomDistribution
  //!   {
  //!   public:
  //!   Cauchy(double x0, double gamma) : _x0(x0), _gamma(gamma) { }
  //!   double generate(CAP::MATH::RandomGenerator & g) const override
  //!   { return _x0 + _gamma*std::tan(M_PI*(g.random() - 0.5)); }
  //!   double _x0, _gamma;
  //!   };
  //!   double x = generator.sample(Cauchy(0.0, 1.0));
  //! @endcode
  //!
  class RandomDistribution
  {
  public:

  //! @brief Destructor.
  virtual ~RandomDistribution() { }
  //! @brief Random number of the distribution. @param generator source of uniform numbers @return number
  virtual double generate(RandomGenerator & generator) const = 0;
  //! @brief Random number of the distribution (see generate()). @param generator generator @return number
  double operator()(RandomGenerator & generator) const { return generate(generator); }
  };

  //!
  //! @brief Distribution of any density f on [minimum, maximum], tabulated: the integral of f is computed on n bins
  //! (Simpson's rule in each), and inverted with a linear interpolation in each bin.
  //!
  //! @code
  //!   CAP::MATH::TabulatedDistribution d([](double x) { return x*x*std::exp(-x); }, 0.0, 20.0);
  //!   double x = generator.sample(d);
  //! @endcode
  //!
  class TabulatedDistribution : public RandomDistribution
  {
  public:

  //! @brief Distribution of a density. @param density density (>= 0; not normalized) @param minimum low edge
  //! @param maximum high edge @param nBins number of bins of the table
  //! @throws MathException if the density is negative or its integral is not positive
  TabulatedDistribution(const std::function<double(double)> & density, double minimum, double maximum, int nBins=10000);
  //! @brief Random number of the distribution. @param generator generator @return number in [minimum, maximum]
  double generate(RandomGenerator & generator) const override;
  //! @brief Integral of the density on [minimum, maximum] (normalization). @return integral
  double integral() const { return _integral; }

  protected:

  double _minimum;   //!< low edge
  double _maximum;   //!< high edge
  double _integral;   //!< integral of the density
  std::vector<double> _cumulative;   //!< integral up to each bin edge, divided by _integral (0 to 1)
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__RandomDistribution
