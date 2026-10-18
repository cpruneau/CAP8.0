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
//! @file Fitter.cpp
//! @brief Implementation of Fitter.
#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include "Fitter.hpp"
#include "DataPoints.hpp"
#include "MathExceptions.hpp"
#include "ParameterizedFunction.hpp"

namespace CAP
{
  namespace MATH
  {

  namespace
  {
  const double kInfinity = std::numeric_limits<double>::infinity();

  //! The chi square as a function of the free parameters, for the search engines.
  class FitProblem : public SearchProblem
  {
  public:
  FitProblem(unsigned int dimension, std::function<double(const std::vector<double>&)> chiSquare)
  : SearchProblem(dimension, kMinimum), _chiSquare(chiSquare) { }
  double value(const std::vector<double> & p) const override { double v = _chiSquare(p); return std::isfinite(v) ? v : std::numeric_limits<double>::quiet_NaN(); }
  private:
  std::function<double(const std::vector<double>&)> _chiSquare;
  };
  } // namespace

  Fitter::Fitter(ParameterizedFunction & function, const DataPoints & data)
  :
  _function(function),
  _data(data),
  _chiSquare(function, data),
  _method(kLevenbergMarquardt),
  _search(),
  _polish(true),
  _maxIterations(500),
  _chiSquareTolerance(1E-10),
  _parameterTolerance(1E-8),
  _scaleErrors(false),
  _free(),
  _used(),
  _scale(),
  _nEvaluations(0),
  _result()
  { }

  void Fitter::setFree(const std::vector<double> & p)
  {
  for (unsigned int j=0; j<_free.size(); j++)
    {
    const Parameter & parameter = _function.parameter(_free[j]);
    double v = p[j];
    if (parameter.isLowConstrained())  v = std::max(v, parameter.minimum());
    if (parameter.isHighConstrained()) v = std::min(v, parameter.maximum());
    _function.setParameterValue(_free[j], v);
    }
  }

  bool Fitter::residuals(std::vector<double> & r)
  {
  _nEvaluations++;
  r.resize(_used.size());
  for (unsigned int i=0; i<_used.size(); i++)
    {
    double pull;
    try { pull = _chiSquare.pullAt(_used[i]); }
    catch (...) { return false; }
    if (!std::isfinite(pull)) return false;
    r[i] = pull;
    }
  return true;
  }

  double Fitter::chiSquareAt(const std::vector<double> & p)
  {
  setFree(p);
  std::vector<double> r;
  if (!residuals(r)) return kInfinity;
  double sum = 0;
  for (double v : r) sum += v*v;
  return sum;
  }

  bool Fitter::jacobian(const std::vector<double> & p, Matrix & J)
  {
  const unsigned int n = (unsigned int)(_used.size()), m = (unsigned int)(_free.size());
  J = Matrix(n, m, 0.0);
  std::vector<double> q(p), up, down;
  for (unsigned int j=0; j<m; j++)
    {
    const Parameter & parameter = _function.parameter(_free[j]);
    double h = 1E-6*std::max(std::fabs(p[j]), _scale[j]);
    // central differences, one-sided where a bound is too close
    bool canUp = !parameter.isHighConstrained() || p[j] + h <= parameter.maximum();
    bool canDown = !parameter.isLowConstrained() || p[j] - h >= parameter.minimum();
    double hUp = canUp ? h : 0.0, hDown = canDown ? h : 0.0;
    if (!canUp && !canDown) return false;
    q[j] = p[j] + hUp;   setFree(q); if (!residuals(up))   { setFree(p); return false; }
    q[j] = p[j] - hDown; setFree(q); if (!residuals(down)) { setFree(p); return false; }
    q[j] = p[j];
    for (unsigned int i=0; i<n; i++) J(i, j) = (up[i] - down[i])/(hUp + hDown);
    }
  setFree(p);
  return true;
  }

  bool Fitter::levenbergMarquardt(FitResult & result)
  {
  const unsigned int m = (unsigned int)(_free.size()), n = (unsigned int)(_used.size());
  std::vector<double> p(m);
  for (unsigned int j=0; j<m; j++) p[j] = _function.parameter(_free[j]).value();
  double chi2 = chiSquareAt(p);
  if (!std::isfinite(chi2)) { result.status = "the chi square cannot be computed at the starting values"; return false; }
  std::vector<double> r;
  double lambda = 1E-3;
  for (int iteration=0; iteration<_maxIterations; iteration++)
    {
    result.nIterations++;
    Matrix J;
    if (!jacobian(p, J)) { result.status = "the derivatives cannot be computed"; return false; }
    residuals(r);
    // curvature A = J^T J and gradient g = J^T r (half the gradient of the chi square)
    Matrix A(m, m, 0.0);
    std::vector<double> g(m, 0.0);
    for (unsigned int a=0; a<m; a++)
      {
      for (unsigned int i=0; i<n; i++) g[a] += J(i, a)*r[i];
      for (unsigned int b=a; b<m; b++)
        {
        double s = 0;
        for (unsigned int i=0; i<n; i++) s += J(i, a)*J(i, b);
        A(a, b) = s; A(b, a) = s;
        }
      }
    // damped steps: larger damping until the chi square decreases
    bool accepted = false;
    std::vector<double> next(m), step(m);
    double chi2Next = chi2;
    while (lambda < 1E16)
      {
      Matrix M = A;
      for (unsigned int a=0; a<m; a++) M(a, a) += lambda*(A(a, a) > 0 ? A(a, a) : 1.0);
      std::vector<double> minusG(m);
      for (unsigned int a=0; a<m; a++) minusG[a] = -g[a];
      try { step = M.solve(minusG); }
      catch (const MathException &) { lambda *= 10; continue; }
      for (unsigned int a=0; a<m; a++) next[a] = p[a] + step[a];
      chi2Next = chiSquareAt(next);
      for (unsigned int a=0; a<m; a++) next[a] = _function.parameter(_free[a]).value();   // clamped to the bounds
      if (chi2Next <= chi2) { accepted = true; break; }
      lambda *= 10;
      }
    if (!accepted)
      {
      setFree(p);
      result.status = "the chi square no longer decreases (minimum within the precision of the derivatives)";
      return true;
      }
    double decrease = chi2 - chi2Next;
    bool smallStep = true;
    for (unsigned int a=0; a<m; a++)
      smallStep = smallStep && std::fabs(next[a] - p[a]) <= _parameterTolerance*(std::fabs(p[a]) + _parameterTolerance*_scale[a]);
    p = next;
    chi2 = chi2Next;
    lambda = std::max(lambda/10, 1E-12);
    if (decrease <= _chiSquareTolerance*(chi2 + 1E-300) && smallStep) { result.status = "converged"; return true; }
    if (chi2 == 0) { result.status = "converged (the function goes through all the points)"; return true; }
    }
  result.status = "too many iterations";
  return false;
  }

  void Fitter::finish(FitResult & result)
  {
  const unsigned int m = (unsigned int)(_free.size());
  std::vector<double> p(m);
  for (unsigned int j=0; j<m; j++) p[j] = _function.parameter(_free[j]).value();
  result.chiSquare = chiSquareAt(p);
  result.nPoints = int(_used.size());
  result.nDegreesOfFreedom = int(_used.size()) - int(m);
  result.chiSquarePerDegreeOfFreedom = result.nDegreesOfFreedom > 0 ? result.chiSquare/result.nDegreesOfFreedom : 0.0;
  result.pValue = (result.nDegreesOfFreedom > 0 && std::isfinite(result.chiSquare)) ? ChiSquare::probability(result.chiSquare, result.nDegreesOfFreedom) : 0.0;
  result.values = _function.parameterValues();
  result.errors.assign(_function.nParameters(), 0.0);
  result.atLimit.assign(_function.nParameters(), false);
  result.freeParameters = _free;
  for (unsigned int j=0; j<m; j++)
    {
    const Parameter & parameter = _function.parameter(_free[j]);
    double tolerance = 1E-9*std::max(std::fabs(parameter.value()), _scale[j]);
    result.atLimit[_free[j]] = (parameter.isLowConstrained() && parameter.value() <= parameter.minimum() + tolerance)
                            || (parameter.isHighConstrained() && parameter.value() >= parameter.maximum() - tolerance);
    }
  // covariance (J^T J)^-1 at the minimum
  Matrix J;
  result.validErrors = false;
  if (jacobian(p, J))
    {
    Matrix A = J.transpose()*J;
    try
      {
      Matrix C = A.inverse();
      double factor = (_scaleErrors && result.nDegreesOfFreedom > 0) ? result.chiSquarePerDegreeOfFreedom : 1.0;
      for (unsigned int a=0; a<m; a++) for (unsigned int b=0; b<m; b++) C(a, b) *= factor;
      result.covariance = C;
      result.correlation = Matrix(m, m, 0.0);
      bool positive = true;
      for (unsigned int a=0; a<m; a++) positive = positive && C(a, a) > 0;
      for (unsigned int a=0; a<m; a++)
        {
        result.errors[_free[a]] = C(a, a) > 0 ? std::sqrt(C(a, a)) : 0.0;
        for (unsigned int b=0; b<m; b++)
          result.correlation(a, b) = positive ? C(a, b)/std::sqrt(C(a, a)*C(b, b)) : 0.0;
        }
      result.validErrors = positive;
      }
    catch (const MathException &) { }
    }
  if (!result.validErrors) result.status += " (no uncertainties: the curvature matrix is singular)";
  result.nEvaluations = _nEvaluations;
  }

  FitResult Fitter::fit()
  {
  FitResult result;
  _nEvaluations = 0;
  _free = _function.freeParameters();
  if (_free.empty()) throw MathException("The function has no free parameter to fit", __FUNCTION__);
  if (_data.rank() != _function.rank()) throw MathException("The function and the data have different ranks", __FUNCTION__);
  // the points used: those whose uncertainty is not zero
  _used.clear();
  for (unsigned int i=0; i<_data.size(); i++)
    {
    try { if (std::isfinite(_chiSquare.pullAt(i))) _used.push_back(i); }
    catch (...) { }
    }
  if (_used.empty()) throw MathException("No data point has a nonzero uncertainty", __FUNCTION__);
  // typical sizes of the free parameters: their ranges, otherwise their starting values
  _scale.clear();
  for (unsigned int j : _free)
    {
    const Parameter & parameter = _function.parameter(j);
    double s = (parameter.isLowConstrained() && parameter.isHighConstrained()) ? parameter.maximum() - parameter.minimum()
             : std::max(std::fabs(parameter.value()), 1.0);
    _scale.push_back(s > 0 ? s : 1.0);
    }
  if (_method == kSearch)
    {
    const unsigned int m = (unsigned int)(_free.size());
    FitProblem problem(m, [this](const std::vector<double> & p) { return chiSquareAt(p); });
    SearchDomain domain(m);
    std::vector<double> start;
    for (unsigned int j=0; j<m; j++)
      {
      const Parameter & parameter = _function.parameter(_free[j]);
      domain.setBounds(j, parameter.isLowConstrained() ? parameter.minimum() : -kInfinity,
                          parameter.isHighConstrained() ? parameter.maximum() : kInfinity);
      start.push_back(parameter.value());
      }
    domain.setStart(start);
    SearchResult found = _search.search(problem, domain);
    result.method = found.engine;
    result.nIterations = found.nIterations;
    if (found.found) setFree(found.point);
    else { result.status = String("the search found no point: ") + found.stopReason; finish(result); _result = result; return result; }
    result.converged = found.converged;
    result.status = found.stopReason;
    if (_polish)
      {
      result.converged = levenbergMarquardt(result);
      result.method = String(result.method.str() + " + Levenberg-Marquardt");
      }
    }
  else
    {
    result.method = "Levenberg-Marquardt";
    result.converged = levenbergMarquardt(result);
    }
  finish(result);
  _result = result;
  return result;
  }

  } // namespace MATH
} // namespace CAP
