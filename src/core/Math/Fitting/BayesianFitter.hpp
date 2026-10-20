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
//! @file BayesianFitter.hpp
//! @brief BayesianFitter: posterior distribution of the parameters of a fit (likelihood and priors), by MCMC.
#ifndef CAP__MATH__BayesianFitter
#define CAP__MATH__BayesianFitter
#include <memory>
#include <vector>
#include "Fitter.hpp"
#include "MetropolisHastings.hpp"
#include "Prior.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Posterior distribution of the parameters: the samples (of the free parameters) and their summaries, given
  //! for all the parameters (a fixed parameter has its value and a zero width).
  //!
  struct BayesianResult
  {
    bool   valid = false;   //!< the sampling ran
    String status;   //!< how it went (or why it did not run)
    McmcResult chain;   //!< samples of the free parameters (in the order of freeParameters)
    std::vector<unsigned int> freeParameters;   //!< indices of the free parameters
    std::vector<double> means;   //!< posterior means
    std::vector<double> standardDeviations;   //!< posterior standard deviations
    std::vector<double> medians;   //!< posterior medians
    std::vector<double> low68;   //!< lower end of the central 68.3% credible interval
    std::vector<double> high68;   //!< upper end of the central 68.3% credible interval
    std::vector<double> low95;   //!< lower end of the central 95.4% credible interval
    std::vector<double> high95;   //!< upper end of the central 95.4% credible interval
    std::vector<double> mode;   //!< sample of highest posterior density (maximum a posteriori)
    Matrix correlation;   //!< correlations of the free parameters
    std::vector<double> effectiveSampleSizes;   //!< effective numbers of samples of the free parameters
    std::vector<double> rHats;   //!< Gelman-Rubin statistics of the free parameters
    double acceptance = 0;   //!< fraction of accepted moves
  };

  //!
  //! @brief Bayesian fit: the posterior distribution of the free parameters of a function, proportional to the
  //! likelihood of the data times the priors, sampled by Markov chain Monte Carlo (MetropolisHastings).
  //!
  //! The likelihood is that of a Likelihood: exp(-statistic/2) (chi square, Poisson, unbinned). Each parameter has a
  //! Prior (setPrior()); by default it is flat, within the bounds of the parameter if it has some. By default the
  //! chains start from a fit (see Fitter), whose uncertainties give the first sizes of the proposals; otherwise from
  //! the current values. The samples give the posterior means, standard deviations, medians, credible intervals,
  //! mode and correlations; the diagnostics of the chains (acceptance, effective sample sizes, R-hat) tell whether
  //! the sampling can be trusted (R-hat close to 1, effective sizes in the hundreds at least).
  //!
  //! The function is left at the posterior means.
  //!
  //! @code
  //!   CAP::MATH::Normal model(100.0, 0.0, 1.0);
  //!   CAP::MATH::BayesianFitter bayes(model, histogram, CAP::MATH::Fitter::kPoisson);
  //!   bayes.setPrior(2, CAP::MATH::Prior::gaussian(1.1, 0.05));     // a width known from elsewhere
  //!   CAP::MATH::BayesianResult r = bayes.sample();
  //!   std::cout << r.means[1] << " in [" << r.low68[1] << ", " << r.high68[1] << "]";
  //! @endcode
  //!
  class BayesianFitter
  {
  public:

  //! @brief Bayesian fit of any likelihood. @param likelihood likelihood (not owned)
  explicit BayesianFitter(Likelihood & likelihood);
  //! @brief Bayesian fit of DataPoints. @param function function (not owned) @param data data (not owned)
  //! @param statistic chi square or Poisson likelihood
  BayesianFitter(ParameterizedFunction & function, const DataPoints & data, Fitter::Statistic statistic=Fitter::kChiSquare);

  //! @brief Set the prior of a parameter. @param index index of the parameter @param prior prior
  //! @throws MathException if index is out of range
  void setPrior(unsigned int index, const Prior & prior);
  //! @brief Prior of a parameter. @param index index of the parameter @return prior
  const Prior & prior(unsigned int index) const;
  //! @brief The sampler (chains, burn-in, samples, thinning, seed). @return sampler
  MetropolisHastings & sampler() { return _sampler; }
  //! @brief Whether the chains start from a fit. @return true if so
  bool startFromFit() const { return _startFromFit; }
  //! @brief Start the chains from a fit, or from the current values. @param v from a fit
  void setStartFromFit(bool v) { _startFromFit = v; }
  //! @brief Log of the posterior density (up to a constant) at values of the free parameters; the parameters are
  //! left at these values. @param values values of the free parameters @return log posterior (-infinity outside the
  //! bounds or the supports of the priors)
  double logPosterior(const std::vector<double> & values);
  //! @brief Sample the posterior. @return result
  BayesianResult sample();

  protected:

  std::unique_ptr<Likelihood> _owned;   //!< likelihood made by this object
  Likelihood * _likelihood;   //!< likelihood
  std::vector<Prior> _priors;   //!< priors of the parameters
  MetropolisHastings _sampler;   //!< sampler
  bool _startFromFit;   //!< chains start from a fit
  std::vector<unsigned int> _free;   //!< free parameters
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__BayesianFitter
