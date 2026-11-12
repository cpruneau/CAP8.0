/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file MultivariateDistributions.cpp
//! @brief Implementation of MultinomialDistribution and DirichletDistribution.
#include <cmath>
#include <limits>
#include <string>
#include "MultivariateDistributions.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  namespace
  {
  const double kInfinity = std::numeric_limits<double>::infinity();

  String indexed(const char * name, unsigned int i) { return String((std::string(name) + std::to_string(i)).c_str()); }

  std::vector<Parameter> multinomialParameters(double n, const std::vector<double> & p, double amplitude)
  {
  if (p.size() < 2) throw MathException("A multinomial distribution has at least 2 categories", __FUNCTION__);
  if (n < 0.0) throw MathException("The number of trials of a multinomial distribution is >= 0", __FUNCTION__);
  double sum = 0.0;
  for (double q : p)
    {
    if (q < 0.0 || q > 1.0) throw MathException("The probabilities of a multinomial distribution are in [0, 1]", __FUNCTION__);
    sum += q;
    }
  if (std::fabs(sum - 1.0) > 1.0E-9) throw MathException("The probabilities of a multinomial distribution add up to 1", __FUNCTION__);
  Parameter trials("n", n, Parameter::Low, 0.0);
  trials.setType(Parameter::Fixed);
  std::vector<Parameter> parameters = {Parameter("amplitude", amplitude), trials};
  for (std::size_t i=0; i+1<p.size(); i++) parameters.push_back(Parameter(indexed("p", unsigned(i + 1)), p[i], Parameter::LowHigh, 0.0, 1.0));
  return parameters;
  }

  std::vector<Parameter> dirichletParameters(const std::vector<double> & alpha, double amplitude)
  {
  if (alpha.size() < 2) throw MathException("A Dirichlet distribution has at least 2 categories", __FUNCTION__);
  std::vector<Parameter> parameters = {Parameter("amplitude", amplitude)};
  for (std::size_t i=0; i<alpha.size(); i++)
    {
    if (!(alpha[i] > 0.0)) throw MathException("The concentrations of a Dirichlet distribution are > 0", __FUNCTION__);
    parameters.push_back(Parameter(indexed("alpha", unsigned(i + 1)), alpha[i], Parameter::Low, 0.0));
    }
  return parameters;
  }
  } // namespace

  // ---- MultinomialDistribution

  MultinomialDistribution::MultinomialDistribution(double n, const std::vector<double> & p, double amplitude)
  :
  ParameterizedFunction(multinomialParameters(n, p, amplitude))
  {
  setRank((unsigned int)(p.size() - 1));
  }

  MultinomialDistribution::MultinomialDistribution(unsigned int m)
  :
  ParameterizedFunction(multinomialParameters(10.0, std::vector<double>(m < 2 ? 2 : m, 1.0/(m < 2 ? 2 : m)), 1.0))
  {
  setRank(m < 2 ? 1 : m - 1);
  }

  double MultinomialDistribution::probability(unsigned int i) const
  {
  unsigned int m = nCategories();
  if (i >= m) throw MathException("Category out of range", __FUNCTION__);
  if (i + 1 < m) return parameterValue(2 + i);
  double last = 1.0;
  for (unsigned int j=0; j+1<m; j++) last -= parameterValue(2 + j);
  return last;
  }

  double MultinomialDistribution::covariance(unsigned int i, unsigned int j) const
  {
  double pi = probability(i), pj = probability(j);
  return i == j ? trials()*pi*(1.0 - pi) : -trials()*pi*pj;
  }

  double MultinomialDistribution::evaluateAt(const std::vector<double> & k) const
  {
  checkRank((unsigned int)(k.size()), __FUNCTION__);
  unsigned int m = nCategories();
  double n = trials(), last = n, logP = std::lgamma(n + 1.0);
  for (unsigned int i=0; i<=m-1; i++)
    {
    double ki = i + 1 < m ? k[i] : last, pi = probability(i);
    if (ki < 0.0 || pi < -1.0E-12) return 0.0;
    if (i + 1 < m) last -= ki;
    if (pi <= 0.0)
      {
      if (ki > 0.0) return 0.0;   // 0^0 = 1
      continue;
      }
    logP += ki*std::log(pi) - std::lgamma(ki + 1.0);
    }
  return amplitude()*std::exp(logP);
  }

  double MultinomialDistribution::evaluateAt(double k) const
  {
  return evaluateAt(std::vector<double>{k});
  }

  double MultinomialDistribution::evaluateAt(double k1, double k2) const
  {
  return evaluateAt(std::vector<double>{k1, k2});
  }

  // ---- DirichletDistribution

  DirichletDistribution::DirichletDistribution(const std::vector<double> & alpha, double amplitude)
  :
  ParameterizedFunction(dirichletParameters(alpha, amplitude))
  {
  setRank((unsigned int)(alpha.size() - 1));
  }

  DirichletDistribution::DirichletDistribution(unsigned int K)
  :
  ParameterizedFunction(dirichletParameters(std::vector<double>(K < 2 ? 2 : K, 1.0), 1.0))
  {
  setRank(K < 2 ? 1 : K - 1);
  }

  double DirichletDistribution::mean(unsigned int i) const
  {
  double sum = 0.0;
  for (unsigned int j=0; j<nCategories(); j++) sum += concentration(j);
  return concentration(i)/sum;
  }

  double DirichletDistribution::covariance(unsigned int i, unsigned int j) const
  {
  double a0 = 0.0;
  for (unsigned int l=0; l<nCategories(); l++) a0 += concentration(l);
  double ai = concentration(i), aj = concentration(j);
  return ((i == j ? ai/a0 : 0.0) - ai*aj/(a0*a0))/(a0 + 1.0);
  }

  double DirichletDistribution::evaluateAt(const std::vector<double> & x) const
  {
  checkRank((unsigned int)(x.size()), __FUNCTION__);
  unsigned int K = nCategories();
  double a0 = 0.0, last = 1.0, logP = 0.0;
  for (unsigned int i=0; i<K; i++)
    {
    double a = concentration(i);
    if (!(a > 0.0)) return 0.0;
    a0 += a;
    double xi = i + 1 < K ? x[i] : last;
    if (i + 1 < K) last -= xi;
    if (xi < 0.0 || (i + 1 == K && xi < -1.0E-15)) return 0.0;
    if (xi <= 0.0)
      {
      if (a < 1.0) return kInfinity;
      if (a > 1.0) return 0.0;
      logP -= std::lgamma(a);
      continue;
      }
    logP += (a - 1.0)*std::log(xi) - std::lgamma(a);
    }
  return amplitude()*std::exp(logP + std::lgamma(a0));
  }

  double DirichletDistribution::evaluateAt(double x) const
  {
  return evaluateAt(std::vector<double>{x});
  }

  double DirichletDistribution::evaluateAt(double x1, double x2) const
  {
  return evaluateAt(std::vector<double>{x1, x2});
  }

  } // namespace MATH
} // namespace CAP
