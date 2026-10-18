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

  Fitter::Fitter(ParameterizedFunction & function, const DataPoints & data, Statistic statistic)
  :
  _function(function),
  _owned(statistic == kPoisson ? static_cast<Likelihood*>(new PoissonLikelihood(function, data))
                               : static_cast<Likelihood*>(new ChiSquareLikelihood(function, data))),
  _likelihood(nullptr),
  _method(kLevenbergMarquardt),
  _search(),
  _polish(true),
  _maxIterations(500),
  _chiSquareTolerance(1E-10),
  _parameterTolerance(1E-8),
  _scaleErrors(false),
  _free(),
  _scale(),
  _nEvaluations(0),
  _result()
  {
  _likelihood = _owned.get();
  }

  Fitter::Fitter(Likelihood & likelihood)
  :
  _function(likelihood.function()),
  _owned(),
  _likelihood(&likelihood),
  _method(kLevenbergMarquardt),
  _search(),
  _polish(true),
  _maxIterations(500),
  _chiSquareTolerance(1E-10),
  _parameterTolerance(1E-8),
  _scaleErrors(false),
  _free(),
  _scale(),
  _nEvaluations(0),
  _result()
  { }

  ChiSquare & Fitter::chiSquare()
  {
  ChiSquareLikelihood * chi2 = dynamic_cast<ChiSquareLikelihood*>(_likelihood);
  if (!chi2) throw MathException("This fit does not minimize a chi square", __FUNCTION__);
  return chi2->chiSquare();
  }

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
  return _likelihood->residuals(r);
  }

  double Fitter::chiSquareAt(const std::vector<double> & p)
  {
  setFree(p);
  _nEvaluations++;
  double s = _likelihood->statistic();
  return std::isfinite(s) ? s : kInfinity;
  }

  bool Fitter::hessian(const std::vector<double> & p, Matrix & H)
  {
  // central second differences, steps of 1E-4 of the size of the parameters (curvatures need larger steps than
  // slopes); the parameters are left at p
  const unsigned int m = (unsigned int)(_free.size());
  H = Matrix(m, m, 0.0);
  std::vector<double> h(m), q(p);
  for (unsigned int a=0; a<m; a++) h[a] = 1E-4*std::max(std::fabs(p[a]), 1E-3*_scale[a]);
  double center = chiSquareAt(p);
  if (!std::isfinite(center)) return false;
  for (unsigned int a=0; a<m; a++)
    {
    q[a] = p[a] + h[a]; double up = chiSquareAt(q);
    q[a] = p[a] - h[a]; double down = chiSquareAt(q);
    q[a] = p[a];
    if (!std::isfinite(up) || !std::isfinite(down)) { setFree(p); return false; }
    H(a, a) = (up - 2*center + down)/(h[a]*h[a]);
    for (unsigned int b=0; b<a; b++)
      {
      double v[4];
      int k = 0;
      for (int sa : {1, -1}) for (int sb : {1, -1})
        { q[a] = p[a] + sa*h[a]; q[b] = p[b] + sb*h[b]; v[k++] = chiSquareAt(q); }
      q[a] = p[a]; q[b] = p[b];
      for (double x : v) if (!std::isfinite(x)) { setFree(p); return false; }
      H(a, b) = H(b, a) = (v[0] - v[1] - v[2] + v[3])/(4*h[a]*h[b]);
      }
    }
  setFree(p);
  return true;
  }

  bool Fitter::covarianceAt(Matrix & covariance)
  {
  if (_free.empty()) prepare();
  const unsigned int m = (unsigned int)(_free.size());
  std::vector<double> p(m);
  for (unsigned int j=0; j<m; j++) p[j] = _function.parameter(_free[j]).value();
  try
    {
    Matrix curvature;                                    // J^T J, or H/2: covariance = curvature^-1
    if (_likelihood->hasResiduals())
      {
      Matrix J;
      if (!jacobian(p, J)) return false;
      curvature = J.transpose()*J;
      }
    else
      {
      if (!hessian(p, curvature)) return false;
      for (unsigned int a=0; a<m; a++) for (unsigned int b=0; b<m; b++) curvature(a, b) *= 0.5;
      }
    // a parameter that the statistic hardly depends on (relative to the others, in units of their typical sizes)
    // is not determined: no covariance
    double largest = 0;
    for (unsigned int a=0; a<m; a++) largest = std::max(largest, curvature(a, a)*_scale[a]*_scale[a]);
    for (unsigned int a=0; a<m; a++) if (!(curvature(a, a)*_scale[a]*_scale[a] > 1E-8*largest)) return false;
    covariance = curvature.inverse();
    }
  catch (const MathException &) { return false; }
  return true;
  }

  bool Fitter::jacobian(const std::vector<double> & p, Matrix & J)
  {
  const unsigned int m = (unsigned int)(_free.size());
  std::vector<double> q(p), up, down;
  setFree(p);
  if (!residuals(up)) return false;
  const unsigned int n = (unsigned int)(up.size());
  J = Matrix(n, m, 0.0);
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
  const unsigned int m = (unsigned int)(_free.size());
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
    const unsigned int n = (unsigned int)(r.size());
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

  bool Fitter::gradient(const std::vector<double> & p, std::vector<double> & g)
  {
  const unsigned int m = (unsigned int)(_free.size());
  g.assign(m, 0.0);
  std::vector<double> q(p);
  for (unsigned int a=0; a<m; a++)
    {
    double h = 1E-6*std::max(std::fabs(p[a]), 1E-3*_scale[a]);
    q[a] = p[a] + h; double up = chiSquareAt(q);
    q[a] = p[a] - h; double down = chiSquareAt(q);
    q[a] = p[a];
    if (!std::isfinite(up) || !std::isfinite(down)) { setFree(p); return false; }
    g[a] = (up - down)/(2*h);
    }
  setFree(p);
  return true;
  }

  bool Fitter::newton(FitResult & result)
  {
  // minimize S by steps solving (H + lambda diag H) delta = -g, the damping growing until S decreases; where the
  // curvature is not positive, the diagonal is replaced by its magnitude (a gradient step)
  const unsigned int m = (unsigned int)(_free.size());
  std::vector<double> p(m);
  for (unsigned int j=0; j<m; j++) p[j] = _function.parameter(_free[j]).value();
  double s = chiSquareAt(p);
  if (!std::isfinite(s)) { result.status = "the statistic cannot be computed at the starting values"; return false; }
  double lambda = 1E-3;
  for (int iteration=0; iteration<_maxIterations; iteration++)
    {
    result.nIterations++;
    std::vector<double> g;
    Matrix H;
    if (!gradient(p, g) || !hessian(p, H)) { result.status = "the derivatives cannot be computed"; return false; }
    bool accepted = false;
    std::vector<double> next(m), step(m);
    double sNext = s;
    while (lambda < 1E16)
      {
      Matrix M = H;
      for (unsigned int a=0; a<m; a++)
        {
        double d = std::fabs(H(a, a)) > 0 ? std::fabs(H(a, a)) : 1.0/(_scale[a]*_scale[a]);
        M(a, a) = std::max(H(a, a), 0.0) + lambda*d + (H(a, a) > 0 ? 0.0 : d);
        }
      std::vector<double> minusG(m);
      for (unsigned int a=0; a<m; a++) minusG[a] = -g[a];
      try { step = M.solve(minusG); }
      catch (const MathException &) { lambda *= 10; continue; }
      for (unsigned int a=0; a<m; a++) next[a] = p[a] + step[a];
      sNext = chiSquareAt(next);
      for (unsigned int a=0; a<m; a++) next[a] = _function.parameter(_free[a]).value();
      if (sNext <= s) { accepted = true; break; }
      lambda *= 10;
      }
    if (!accepted)
      {
      setFree(p);
      result.status = "the statistic no longer decreases (minimum within the precision of the derivatives)";
      return true;
      }
    double decrease = s - sNext;
    bool smallStep = true;
    for (unsigned int a=0; a<m; a++)
      smallStep = smallStep && std::fabs(next[a] - p[a]) <= _parameterTolerance*(std::fabs(p[a]) + _parameterTolerance*_scale[a]);
    p = next;
    s = sNext;
    lambda = std::max(lambda/10, 1E-12);
    if (decrease <= _chiSquareTolerance*(std::fabs(s) + 1E-300) && smallStep) { result.status = "converged"; return true; }
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
  result.statistic = _likelihood->name();
  result.nPoints = int(_likelihood->nPoints());
  result.nDegreesOfFreedom = result.nPoints - int(m);
  result.chiSquarePerDegreeOfFreedom = result.nDegreesOfFreedom > 0 ? result.chiSquare/result.nDegreesOfFreedom : 0.0;
  result.pValue = (_likelihood->goodnessOfFit() && result.nDegreesOfFreedom > 0 && std::isfinite(result.chiSquare))
                ? ChiSquare::probability(result.chiSquare, result.nDegreesOfFreedom) : 0.0;
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
  // covariance at the minimum: (J^T J)^-1, or 2 H^-1
  Matrix C;
  result.validErrors = false;
  if (covarianceAt(C))
    {
      {
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
    }
  if (!result.validErrors) result.status += " (no uncertainties: the curvature matrix is singular)";
  result.nEvaluations = _nEvaluations;
  }

  void Fitter::prepare()
  {
  _free = _function.freeParameters();
  if (_free.empty()) throw MathException("The function has no free parameter to fit", __FUNCTION__);
  _likelihood->prepare();
  // typical sizes of the free parameters: their ranges, otherwise their starting values
  _scale.clear();
  for (unsigned int j : _free)
    {
    const Parameter & parameter = _function.parameter(j);
    double s = (parameter.isLowConstrained() && parameter.isHighConstrained()) ? parameter.maximum() - parameter.minimum()
             : std::max(std::fabs(parameter.value()), 1.0);
    _scale.push_back(s > 0 ? s : 1.0);
    }
  }

  FitResult Fitter::fit()
  {
  FitResult result;
  _nEvaluations = 0;
  prepare();
  const bool residualsAvailable = _likelihood->hasResiduals();
  if (_method == kSearch)
    {
    // the search works on q = (p - p0)/scale: parameters of different sizes are treated alike
    const unsigned int m = (unsigned int)(_free.size());
    std::vector<double> origin(m);
    for (unsigned int j=0; j<m; j++) origin[j] = _function.parameter(_free[j]).value();
    auto toParameters = [this, origin, m](const std::vector<double> & q)
      { std::vector<double> p(m); for (unsigned int j=0; j<m; j++) p[j] = origin[j] + q[j]*_scale[j]; return p; };
    FitProblem problem(m, [this, toParameters](const std::vector<double> & q) { return chiSquareAt(toParameters(q)); });
    SearchDomain domain(m);
    for (unsigned int j=0; j<m; j++)
      {
      const Parameter & parameter = _function.parameter(_free[j]);
      domain.setBounds(j, parameter.isLowConstrained() ? (parameter.minimum() - origin[j])/_scale[j] : -kInfinity,
                          parameter.isHighConstrained() ? (parameter.maximum() - origin[j])/_scale[j] : kInfinity);
      }
    domain.setStart(std::vector<double>(m, 0.0));
    SearchResult found = _search.search(problem, domain);
    result.method = found.engine;
    result.nIterations = found.nIterations;
    if (found.found) setFree(toParameters(found.point));
    else { result.status = String("the search found no point: ") + found.stopReason; finish(result); _result = result; return result; }
    result.converged = found.converged;
    result.status = found.stopReason;
    if (_polish)
      {
      result.converged = residualsAvailable ? levenbergMarquardt(result) : newton(result);
      result.method = String(result.method.str() + (residualsAvailable ? " + Levenberg-Marquardt" : " + Newton"));
      }
    }
  else if (residualsAvailable)
    {
    result.method = "Levenberg-Marquardt";
    result.converged = levenbergMarquardt(result);
    }
  else
    {
    result.method = "Newton";
    result.converged = newton(result);
    }
  finish(result);
  _result = result;
  return result;
  }

  } // namespace MATH
} // namespace CAP
