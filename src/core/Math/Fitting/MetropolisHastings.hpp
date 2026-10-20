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
//! @file MetropolisHastings.hpp
//! @brief MetropolisHastings: Markov chain Monte Carlo sampler of a density; McmcResult: its samples and summaries.
#ifndef CAP__MATH__MetropolisHastings
#define CAP__MATH__MetropolisHastings
#include <functional>
#include <utility>
#include <vector>
#include "Matrix.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Samples of a density drawn by MetropolisHastings, and their summaries: mean, spread, quantiles, credible
  //! intervals, mode, covariance, and the diagnostics of the chains (acceptance, effective sample size, R-hat).
  //!
  struct McmcResult
  {
    unsigned int dimension = 0;   //!< number of variables
    unsigned int nChains = 0;   //!< number of chains
    std::vector<std::vector<double>> samples;   //!< samples, chain after chain (burn-in excluded, thinned)
    std::vector<double> logDensities;   //!< log density of each sample
    std::vector<unsigned int> chains;   //!< chain of each sample
    double acceptance = 0;   //!< fraction of accepted moves after the burn-in

    //! @brief Mean of variable k. @param k variable @return mean
    double mean(unsigned int k) const;
    //! @brief Standard deviation of variable k. @param k variable @return standard deviation
    double standardDeviation(unsigned int k) const;
    //! @brief Quantile of variable k. @param k variable @param q fraction in [0,1] @return value below which a fraction q
    //! of the samples lies
    double quantile(unsigned int k, double q) const;
    //! @brief Median of variable k. @param k variable @return median
    double median(unsigned int k) const { return quantile(k, 0.5); }
    //! @brief Central credible interval of variable k. @param k variable @param probability probability (e.g. 0.683)
    //! @return (lower, upper) ends
    std::pair<double,double> interval(unsigned int k, double probability) const;
    //! @brief The sample of highest density (an estimate of the mode, the maximum a posteriori). @return sample
    std::vector<double> mode() const;
    //! @brief Covariance of the variables. @return dimension x dimension matrix
    Matrix covariance() const;
    //! @brief Correlations of the variables. @return dimension x dimension matrix
    Matrix correlation() const;
    //! @brief Effective number of independent samples of variable k (from the autocorrelation of the chains, summed
    //! up to the first negative pair of autocorrelations, after Geyer). @param k variable @return effective size
    double effectiveSampleSize(unsigned int k) const;
    //! @brief Gelman-Rubin statistic of variable k: close to 1 when the chains agree. @param k variable
    //! @return R-hat (NaN with a single chain)
    double rHat(unsigned int k) const;
    //! @brief Values of variable k in one chain. @param k variable @param chain chain @return values
    std::vector<double> chainValues(unsigned int k, unsigned int chain) const;
  };

  //!
  //! @brief Markov chain Monte Carlo sampler of a density known up to a constant (given by its logarithm), by the
  //! Metropolis-Hastings algorithm with gaussian proposals.
  //!
  //! Several chains start near the start point (spread by the scales). During the burn-in, the size of the proposals
  //! adapts to a target acceptance (0.25 by default), and halfway through it their covariance becomes that of the
  //! samples drawn so far (adaptive Metropolis): correlated variables are then sampled efficiently. After the
  //! burn-in, the proposals are fixed (the chain is then a proper Markov chain) and samples are kept, one every
  //! thinning() moves. A sample outside the support (log density -infinity) is never accepted.
  //!
  class MetropolisHastings
  {
  public:

  //! @brief Sampler with the default settings: 4 chains, 2000 moves of burn-in, 5000 samples per chain.
  MetropolisHastings() : _chains(4), _burnIn(2000), _samples(5000), _thinning(1), _seed(12345), _target(0.25) { }
  //! @brief Set the number of chains. @param n number (at least 1; 2 or more for R-hat)
  void setChains(int n) { _chains = n < 1 ? 1 : n; }
  //! @brief Set the moves of burn-in of each chain. @param n number
  void setBurnIn(int n) { _burnIn = n < 0 ? 0 : n; }
  //! @brief Set the samples kept per chain. @param n number (at least 10)
  void setSamples(int n) { _samples = n < 10 ? 10 : n; }
  //! @brief Set the moves between kept samples. @param n number (at least 1)
  void setThinning(int n) { _thinning = n < 1 ? 1 : n; }
  //! @brief Set the seed of the random numbers. @param seed seed
  void setSeed(unsigned long long seed) { _seed = seed; }
  //! @brief Set the acceptance the proposals adapt to during the burn-in. @param fraction fraction in (0,1)
  void setTargetAcceptance(double fraction) { _target = (fraction > 0 && fraction < 1) ? fraction : 0.25; }
  //! @brief Number of chains. @return number
  int chains() const { return _chains; }
  //! @brief Samples kept per chain. @return number
  int samples() const { return _samples; }
  //!
  //! @brief Sample a density. @param logDensity log of the density (up to a constant; -infinity outside its support)
  //! @param start start point (with a finite log density) @param scales typical spread of each variable (first sizes
  //! of the proposals) @return samples and summaries
  //! @throws MathException if the sizes differ or the log density is not finite at the start point
  //!
  McmcResult sample(const std::function<double(const std::vector<double>&)> & logDensity,
                    const std::vector<double> & start, const std::vector<double> & scales) const;

  protected:

  int _chains;   //!< chains
  int _burnIn;   //!< moves of burn-in per chain
  int _samples;   //!< samples kept per chain
  int _thinning;   //!< moves between kept samples
  unsigned long long _seed;   //!< seed
  double _target;   //!< target acceptance during the burn-in
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__MetropolisHastings
