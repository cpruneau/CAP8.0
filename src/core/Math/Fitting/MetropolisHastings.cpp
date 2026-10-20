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
//! @file MetropolisHastings.cpp
//! @brief Implementation of MetropolisHastings and McmcResult.
#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include "MetropolisHastings.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  namespace
  {
  //! Lower triangular L with L L^T = A (A symmetric positive definite). @return false if A is not
  bool cholesky(const std::vector<std::vector<double>> & A, std::vector<std::vector<double>> & L)
  {
  const std::size_t n = A.size();
  L.assign(n, std::vector<double>(n, 0.0));
  for (std::size_t i=0; i<n; i++)
    for (std::size_t j=0; j<=i; j++)
      {
      double s = A[i][j];
      for (std::size_t k=0; k<j; k++) s -= L[i][k]*L[j][k];
      if (i == j) { if (!(s > 0)) return false; L[i][i] = std::sqrt(s); }
      else L[i][j] = s/L[j][j];
      }
  return true;
  }
  } // namespace

  // ------------------------------------------------------------------------------------------------ McmcResult

  double McmcResult::mean(unsigned int k) const
  {
  double s = 0;
  for (const auto & x : samples) s += x[k];
  return samples.empty() ? 0.0 : s/samples.size();
  }

  double McmcResult::standardDeviation(unsigned int k) const
  {
  if (samples.size() < 2) return 0.0;
  double m = mean(k), s = 0;
  for (const auto & x : samples) s += (x[k] - m)*(x[k] - m);
  return std::sqrt(s/(samples.size() - 1));
  }

  double McmcResult::quantile(unsigned int k, double q) const
  {
  if (samples.empty()) return 0.0;
  std::vector<double> v;
  for (const auto & x : samples) v.push_back(x[k]);
  std::sort(v.begin(), v.end());
  q = std::min(std::max(q, 0.0), 1.0);
  double position = q*(v.size() - 1);
  std::size_t i = std::size_t(position);
  if (i + 1 >= v.size()) return v.back();
  return v[i] + (position - i)*(v[i+1] - v[i]);
  }

  std::pair<double,double> McmcResult::interval(unsigned int k, double probability) const
  {
  return std::make_pair(quantile(k, 0.5*(1 - probability)), quantile(k, 0.5*(1 + probability)));
  }

  std::vector<double> McmcResult::mode() const
  {
  if (samples.empty()) return std::vector<double>();
  return samples[std::max_element(logDensities.begin(), logDensities.end()) - logDensities.begin()];
  }

  Matrix McmcResult::covariance() const
  {
  Matrix C(dimension, dimension, 0.0);
  if (samples.size() < 2) return C;
  std::vector<double> m(dimension);
  for (unsigned int k=0; k<dimension; k++) m[k] = mean(k);
  for (const auto & x : samples)
    for (unsigned int a=0; a<dimension; a++)
      for (unsigned int b=0; b<dimension; b++) C(a, b) += (x[a] - m[a])*(x[b] - m[b]);
  for (unsigned int a=0; a<dimension; a++) for (unsigned int b=0; b<dimension; b++) C(a, b) /= (samples.size() - 1);
  return C;
  }

  Matrix McmcResult::correlation() const
  {
  Matrix C = covariance(), R(dimension, dimension, 0.0);
  for (unsigned int a=0; a<dimension; a++)
    for (unsigned int b=0; b<dimension; b++)
      R(a, b) = (C(a, a) > 0 && C(b, b) > 0) ? C(a, b)/std::sqrt(C(a, a)*C(b, b)) : (a == b ? 1.0 : 0.0);
  return R;
  }

  std::vector<double> McmcResult::chainValues(unsigned int k, unsigned int chain) const
  {
  std::vector<double> v;
  for (std::size_t i=0; i<samples.size(); i++) if (chains[i] == chain) v.push_back(samples[i][k]);
  return v;
  }

  double McmcResult::effectiveSampleSize(unsigned int k) const
  {
  double total = 0;
  for (unsigned int c=0; c<nChains; c++)
    {
    std::vector<double> v = chainValues(k, c);
    const std::size_t n = v.size();
    if (n < 4) { total += n; continue; }
    double m = 0;
    for (double x : v) m += x/n;
    double variance = 0;
    for (double x : v) variance += (x - m)*(x - m)/n;
    if (!(variance > 0)) { total += n; continue; }
    auto rho = [&](std::size_t lag)
      {
      double s = 0;
      for (std::size_t i=0; i+lag<n; i++) s += (v[i] - m)*(v[i+lag] - m);
      return s/(n*variance);
      };
    // Geyer: sum the autocorrelations while the sums of consecutive pairs stay positive
    double sum = 0;
    for (std::size_t lag=1; lag+1<n/2; lag+=2)
      {
      double pair = rho(lag) + rho(lag + 1);
      if (pair < 0) break;
      sum += pair;
      }
    total += n/(1 + 2*sum);
    }
  return total;
  }

  double McmcResult::rHat(unsigned int k) const
  {
  if (nChains < 2) return std::numeric_limits<double>::quiet_NaN();
  std::vector<double> means, variances;
  std::size_t n = 0;
  for (unsigned int c=0; c<nChains; c++)
    {
    std::vector<double> v = chainValues(k, c);
    n = v.size();
    if (n < 2) return std::numeric_limits<double>::quiet_NaN();
    double m = 0, s = 0;
    for (double x : v) m += x/n;
    for (double x : v) s += (x - m)*(x - m)/(n - 1);
    means.push_back(m); variances.push_back(s);
    }
  double W = 0, grand = 0, B = 0;
  for (unsigned int c=0; c<nChains; c++) { W += variances[c]/nChains; grand += means[c]/nChains; }
  for (unsigned int c=0; c<nChains; c++) B += n*(means[c] - grand)*(means[c] - grand)/(nChains - 1);
  if (!(W > 0)) return 1.0;
  double pooled = (n - 1.0)/n*W + B/n;
  return std::sqrt(pooled/W);
  }

  // ------------------------------------------------------------------------------------------------ sampler

  McmcResult MetropolisHastings::sample(const std::function<double(const std::vector<double>&)> & logDensity,
                                        const std::vector<double> & start, const std::vector<double> & scales) const
  {
  const unsigned int d = (unsigned int)(start.size());
  if (d == 0 || scales.size() != d) throw MathException("The start point and the scales must have the same, nonzero, size", __FUNCTION__);
  if (!std::isfinite(logDensity(start))) throw MathException("The log density is not finite at the start point", __FUNCTION__);
  McmcResult result;
  result.dimension = d;
  result.nChains = (unsigned int)(_chains);
  std::mt19937_64 random(_seed);
  std::normal_distribution<double> gaussian(0.0, 1.0);
  std::uniform_real_distribution<double> uniform(0.0, 1.0);
  long accepted = 0, moves = 0;
  for (int chain=0; chain<_chains; chain++)
    {
    // start near the start point (spread by a fraction of the scales), where the density is finite
    std::vector<double> x(start);
    double lp = logDensity(x);
    for (int attempt=0; attempt<100 && chain > 0; attempt++)
      {
      std::vector<double> y(start);
      for (unsigned int k=0; k<d; k++) y[k] += 0.5*scales[k]*gaussian(random);
      double ly = logDensity(y);
      if (std::isfinite(ly)) { x = y; lp = ly; break; }
      }
    // proposals: L z, scaled by s; L starts diagonal (the scales), then the covariance of the burn-in samples
    std::vector<std::vector<double>> L(d, std::vector<double>(d, 0.0));
    for (unsigned int k=0; k<d; k++) L[k][k] = scales[k] > 0 ? scales[k] : 1.0;
    double s = 2.38/std::sqrt(double(d));
    std::vector<std::vector<double>> history;
    int windowAccepted = 0, window = 0;
    std::vector<double> y(d), z(d);
    auto move = [&]()
      {
      for (unsigned int k=0; k<d; k++) z[k] = gaussian(random);
      for (unsigned int i=0; i<d; i++)
        {
        double v = 0;
        for (unsigned int j=0; j<=i; j++) v += L[i][j]*z[j];
        y[i] = x[i] + s*v;
        }
      double ly = logDensity(y);
      bool accept = std::isfinite(ly) && (ly >= lp || uniform(random) < std::exp(ly - lp));
      if (accept) { x = y; lp = ly; }
      return accept;
      };
    for (int t=0; t<_burnIn; t++)
      {
      if (move()) windowAccepted++;
      window++;
      if (t >= _burnIn/4) history.push_back(x);
      if (window == 100)
        {
        // the size of the proposals follows the acceptance
        double rate = double(windowAccepted)/window;
        s *= std::exp(rate - _target);
        windowAccepted = 0; window = 0;
        }
      if (t == _burnIn/2 && history.size() > 10*d)
        {
        // the covariance of the samples so far shapes the proposals
        std::vector<double> m(d, 0.0);
        for (const auto & h : history) for (unsigned int k=0; k<d; k++) m[k] += h[k]/history.size();
        std::vector<std::vector<double>> C(d, std::vector<double>(d, 0.0)), newL;
        for (const auto & h : history)
          for (unsigned int a=0; a<d; a++) for (unsigned int b=0; b<d; b++) C[a][b] += (h[a] - m[a])*(h[b] - m[b])/(history.size() - 1);
        for (unsigned int a=0; a<d; a++) C[a][a] += 1E-12*(C[a][a] + 1E-300);
        if (cholesky(C, newL)) { L = newL; s = 2.38/std::sqrt(double(d)); }
        }
      }
    for (int kept=0; kept<_samples; kept++)
      {
      for (int t=0; t<_thinning; t++) { moves++; if (move()) accepted++; }
      result.samples.push_back(x);
      result.logDensities.push_back(lp);
      result.chains.push_back((unsigned int)(chain));
      }
    }
  result.acceptance = moves > 0 ? double(accepted)/moves : 0.0;
  return result;
  }

  } // namespace MATH
} // namespace CAP
