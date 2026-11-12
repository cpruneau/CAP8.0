/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file CalculateFourierDecomposition.cpp
//! @brief Implementation of CalculateFourierDecomposition.
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <string>
#include <vector>
#include "CalculateFourierDecomposition.hpp"
#include "ChiSquare.hpp"
#include "Fitter.hpp"
#include "Function.hpp"
#include "MathExceptions.hpp"
#include "ModelValues.hpp"

namespace CAP
{
  namespace MATH
  {

  namespace
  {
  String number(unsigned int n) { return String(std::to_string(n).c_str()); }

  //! Basis function j of form 1 at x: 1, cos(x), sin(x), cos(2x), sin(2x), ...
  double basis(unsigned int j, double x)
  {
  if (j == 0) return 1.0;
  unsigned int n = (j + 1)/2;
  return j%2 ? std::cos(n*x) : std::sin(n*x);
  }

  //! sin(t)/t
  double sinc(double t) { return std::fabs(t) < 1.0E-8 ? 1.0 - t*t/6.0 : std::sin(t)/t; }
  } // namespace

  CalculateFourierDecomposition::CalculateFourierDecomposition(unsigned int nMax, Form form, Method method)
  :
  _nMax(1),
  _form(form),
  _method(method),
  _nSamples(0),
  _error(1.0),
  _low(-M_PI),
  _binAveraging(false),
  _result(),
  _samples(),
  _maxDeviation(0.0),
  _rmsDeviation(0.0)
  {
  setNMax(nMax);
  }

  void CalculateFourierDecomposition::setNMax(unsigned int nMax)
  {
  if (nMax == 0) throw MathException("A Fourier decomposition has at least one harmonic", __FUNCTION__);
  _nMax = nMax;
  }

  void CalculateFourierDecomposition::setError(double sigma)
  {
  if (!(sigma > 0.0)) throw MathException("The uncertainty of the samples must be positive", __FUNCTION__);
  _error = sigma;
  }

  FourierDecomposition CalculateFourierDecomposition::calculate(const Function & f)
  {
  if (f.rank() != 1) throw MathException("The Fourier decomposition of a function of rank 1 only", __FUNCTION__);
  unsigned int n = _nSamples > 0 ? _nSamples : std::max(360u, 8*(_nMax + 1));
  double h = 2.0*M_PI/n;
  std::vector<double> x(n), y(n), sigma(n, _error);
  _samples = DataPointsVector();
  for (unsigned int i=0; i<n; i++)
    {
    x[i] = _low + (i + 0.5)*h;
    y[i] = f.evaluateAt(x[i]);
    _samples.addPoint(DataPoint2D(float(x[i]), float(y[i]), float(_error)));
    }
  bool binAveraging = _binAveraging;          // samples are values of the function, not averages
  _binAveraging = false;
  try
    {
    FourierDecomposition d = decompose(x, y, sigma, _samples, true);
    _binAveraging = binAveraging;
    return d;
    }
  catch (...) { _binAveraging = binAveraging; throw; }
  }

  FourierDecomposition CalculateFourierDecomposition::calculate(const DataPoints & data)
  {
  if (data.rank() != 1) throw MathException("The Fourier decomposition of data of rank 1 only", __FUNCTION__);
  std::vector<double> x, y, sigma;
  for (unsigned int i=0; i<data.size(); i++)
    {
    DataPoint2D p = data.point(i);
    if (!std::isfinite(p.x()) || !std::isfinite(p.y())) continue;
    if (!(p.yStatError() > 0.0))
      throw MathException(String("Point ") + number(i) + " has no uncertainty: no chi square can be computed", __FUNCTION__);
    x.push_back(p.x());
    y.push_back(p.y());
    sigma.push_back(p.yStatError());
    }
  return decompose(x, y, sigma, data, false);
  }

  FourierDecomposition CalculateFourierDecomposition::decompose(const std::vector<double> & x, const std::vector<double> & y,
                                                                const std::vector<double> & sigma, const DataPoints & data,
                                                                bool equallySpaced)
  {
  const unsigned int nParameters = 2*_nMax + 1, n = (unsigned int)(x.size());
  if (n <= nParameters)
    throw MathException(String("A decomposition of order ") + number(_nMax) + " needs more than " + number(nParameters)
                        + " points (" + number(n) + " given)", __FUNCTION__);
  std::vector<double> values(nParameters, 0.0);
  Matrix covariance(nParameters, nParameters, 0.0);
  FitResult r;

  if (_method == kAverage)
    {
    double h = 2.0*M_PI/n;
    if (!equallySpaced)
      {
      // the points must be the centers of n equal intervals of one period (e.g. the bins of a histogram over 2 pi)
      std::vector<double> sorted = x;
      std::sort(sorted.begin(), sorted.end());
      double spacing = (sorted.back() - sorted.front())/(n - 1);
      bool equal = std::fabs(n*spacing - 2.0*M_PI) < 1.0E-3*2.0*M_PI;
      for (unsigned int i=1; equal && i<n; i++) equal = std::fabs(sorted[i] - sorted[i-1] - spacing) < 1.0E-3*spacing;
      if (!equal)
        throw MathException("Averages need points equally spaced over one period (2 pi): use the fit (kFit)", __FUNCTION__);
      h = spacing;
      }
    // averages over the points: values_j = sum_i a_j(x_i) y_i, covariance from the uncertainties of the points
    std::vector<std::vector<double>> a(nParameters, std::vector<double>(n));
    for (unsigned int j=0; j<nParameters; j++)
      {
      unsigned int harmonic = (j + 1)/2;
      double factor = (j == 0 ? 1.0 : 2.0)/n;
      if (_binAveraging && j > 0) factor /= sinc(harmonic*h/2.0);   // contents: averages of the function over the bins
      for (unsigned int i=0; i<n; i++) { a[j][i] = factor*basis(j, x[i]); values[j] += a[j][i]*y[i]; }
      }
    for (unsigned int j=0; j<nParameters; j++)
      for (unsigned int l=j; l<nParameters; l++)
        {
        double sum = 0.0;
        for (unsigned int i=0; i<n; i++) sum += a[j][i]*a[l][i]*sigma[i]*sigma[i];
        covariance(j, l) = covariance(l, j) = sum;
        }
    r.converged    = true;
    r.validErrors  = true;
    r.status       = "decomposed";
    r.method       = String("averages over ") + number(n) + " points";
    r.nEvaluations = long(n);
    }
  else
    {
    // chi square fit of form 1 (linear in its parameters), from the least squares estimates
    FourierDecomposition model(_nMax, FourierDecomposition::kCosineSine);
    std::vector<double> w(n), start;
    for (unsigned int i=0; i<n; i++) w[i] = 1.0/(sigma[i]*sigma[i]);
    if (leastSquares(x, y, w, _nMax, start)) model.setParameterValues(start);
    Fitter fitter(model, data);
    fitter.setBinAveraging(_binAveraging);
    FitResult fit = fitter.fit();
    if (!fit.validErrors || fit.freeParameters.size() != nParameters)
      throw MathException(String("The fit of the Fourier decomposition failed: ") + fit.status, __FUNCTION__);
    values = fit.values;
    covariance = fit.covariance;
    r.converged    = fit.converged;
    r.validErrors  = true;
    r.status       = fit.status;
    r.method       = String("chi square fit (") + fit.method + ")";
    r.nEvaluations = fit.nEvaluations;
    r.nIterations  = fit.nIterations;
    }

  // the quality of the decomposition: chi square of form 1 (the same function in both forms) to the points
  FourierDecomposition d1(_nMax, FourierDecomposition::kCosineSine);
  d1.setParameterValues(values);
  ChiSquare chi2(d1, data);
  chi2.setBinAveraging(_binAveraging);
  r.statistic   = "chi square";
  r.chiSquare   = chi2.chiSquare();
  r.nPoints     = int(chi2.nPoints());
  r.nDegreesOfFreedom = chi2.nDegreesOfFreedom();
  r.chiSquarePerDegreeOfFreedom = r.nDegreesOfFreedom > 0 ? chi2.chiSquarePerDegreeOfFreedom() : 0.0;
  r.pValue      = r.nDegreesOfFreedom > 0 ? ChiSquare::probability(r.chiSquare, r.nDegreesOfFreedom) : 0.0;
  double sum2 = 0.0;
  _maxDeviation = 0.0;
  for (unsigned int i=0; i<data.size(); i++)
    {
    DataPoint2D p = data.point(i);
    if (!std::isfinite(p.x()) || !std::isfinite(p.y())) continue;
    double difference = std::fabs(p.y() - modelValue(d1, data, i, _binAveraging));
    _maxDeviation = std::max(_maxDeviation, difference);
    sum2 += difference*difference;
    }
  _rmsDeviation = std::sqrt(sum2/n);

  // form 2 from form 1
  if (_form == FourierDecomposition::kFlow)
    {
    double largest = 0.0;
    for (double value : y) largest = std::max(largest, std::fabs(value));
    if (!(std::fabs(values[0]) > 1.0E-12*largest) || !toFlow(values, &covariance))
      throw MathException("Flow coefficients of a function whose average is 0", __FUNCTION__);
    }
  FourierDecomposition d(_nMax, _form);
  d.setParameterValues(values);
  r.values = values;
  r.covariance = covariance;
  r.atLimit.assign(nParameters, false);
  r.correlation = Matrix(nParameters, nParameters, 0.0);
  for (unsigned int i=0; i<nParameters; i++)
    {
    r.freeParameters.push_back(i);
    r.errors.push_back(std::sqrt(covariance(i, i)));
    for (unsigned int j=0; j<nParameters; j++)
      {
      double norm = std::sqrt(covariance(i, i)*covariance(j, j));
      r.correlation(i, j) = i == j ? 1.0 : (std::isfinite(norm) && norm > 0.0 ? covariance(i, j)/norm : 0.0);
      }
    }
  _result = r;
  return d;
  }

  bool CalculateFourierDecomposition::leastSquares(const std::vector<double> & x, const std::vector<double> & y,
                                                   const std::vector<double> & w, unsigned int nMax,
                                                   std::vector<double> & values, Matrix * covariance)
  {
  const unsigned int m = 2*nMax + 1;
  if (nMax == 0 || x.size() < m || y.size() != x.size() || w.size() != x.size()) return false;
  Matrix curvature(m, m, 0.0);
  std::vector<double> rhs(m, 0.0), b(m);
  for (std::size_t i=0; i<x.size(); i++)
    {
    for (unsigned int j=0; j<m; j++) b[j] = basis(j, x[i]);
    for (unsigned int j=0; j<m; j++)
      {
      rhs[j] += w[i]*b[j]*y[i];
      for (unsigned int l=0; l<m; l++) curvature(j, l) += w[i]*b[j]*b[l];
      }
    }
  try
    {
    std::vector<double> solution = curvature.solve(rhs);
    for (double v : solution) if (!std::isfinite(v)) return false;
    if (covariance) *covariance = curvature.inverse();
    values = solution;
    return true;
    }
  catch (...) { return false; }
  }

  bool CalculateFourierDecomposition::toFlow(std::vector<double> & values, Matrix * covariance)
  {
  const unsigned int m = (unsigned int)(values.size());
  if (m < 3 || m%2 == 0) return false;
  double k = values[0];
  if (k == 0.0) return false;
  // Jacobian of (k, v_n, psi_n) with respect to (k, c_n, s_n)
  std::vector<std::vector<double>> jacobian(m, std::vector<double>(m, 0.0));
  std::vector<double> flow(m);
  std::vector<bool> vanishing(m, false);
  flow[0] = k;
  jacobian[0][0] = 1.0;
  for (unsigned int n=1; 2*n<m; n++)
    {
    unsigned int ic = 2*n - 1, is = 2*n;
    double c = values[ic], s = values[is], a = std::hypot(c, s);
    flow[ic] = a/(2.0*k);
    flow[is] = a > 0.0 ? std::atan2(s, c)/n : 0.0;
    if (a > 0.0)
      {
      jacobian[ic][0]  = -a/(2.0*k*k);
      jacobian[ic][ic] = c/(2.0*k*a);
      jacobian[ic][is] = s/(2.0*k*a);
      jacobian[is][ic] = -s/(n*a*a);
      jacobian[is][is] = c/(n*a*a);
      }
    else
      {
      jacobian[ic][ic] = 1.0/(2.0*std::fabs(k));    // a harmonic that vanishes: the uncertainty of its amplitude
      vanishing[is] = true;
      }
    }
  if (covariance)
    {
    if (covariance->nRows() != m || covariance->nColumns() != m) return false;
    Matrix propagated(m, m, 0.0);
    for (unsigned int i=0; i<m; i++)
      for (unsigned int j=0; j<m; j++)
        {
        double sum = 0.0;
        for (unsigned int p=0; p<m; p++)
          {
          if (jacobian[i][p] == 0.0) continue;
          for (unsigned int q=0; q<m; q++) sum += jacobian[i][p]*(*covariance)(p, q)*jacobian[j][q];
          }
        propagated(i, j) = sum;
        }
    for (unsigned int i=0; i<m; i++)
      if (vanishing[i]) propagated(i, i) = std::numeric_limits<double>::infinity();   // its phase is undefined
    *covariance = propagated;
    }
  values = flow;
  return true;
  }

  FourierDecomposition CalculateFourierDecomposition::decompose(const Function & f, unsigned int nMax, Form form,
                                                                Method method, FitResult * result, double error)
  {
  CalculateFourierDecomposition calculator(nMax, form, method);
  calculator.setError(error);
  FourierDecomposition d = calculator.calculate(f);
  if (result) *result = calculator.result();
  return d;
  }

  } // namespace MATH
} // namespace CAP
