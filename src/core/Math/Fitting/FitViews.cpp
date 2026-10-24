/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FitViews.cpp
//! @brief Implementation of the views of fits.
#include <algorithm>
#include <cmath>
#include "FitViews.hpp"
#include "DataPoint2D.hpp"
#include "DataPoint3D.hpp"
#include "Distributions.hpp"
#include "FitRecord.hpp"
#include "FunctionRegistry.hpp"
#include "MathExceptions.hpp"
#include "ModelValues.hpp"

namespace CAP
{
  namespace MATH
  {

  namespace
  {
  //! Whether a point is inside the range of a fit (no range: everywhere).
  bool insideFit(const FitRecord & fit, const std::vector<double> & x)
  {
  return fit.low.empty() || fit.inRange(x);
  }
  } // namespace

  Histogram residualMap(const Histogram & data, const FitRecord & fit, Residual kind, bool binAveraging)
  {
  if (!fit.function || fit.function->rank() != data.rank()) throw MathException("The fit and the histogram have different ranks", __FUNCTION__);
  Histogram map(data);
  map.clearFits();
  map.reset();
  map.setName(data.name() + String(kind == Residual::kPull ? "_pulls" : "_residuals"));
  map.setTitle(String(kind == Residual::kPull ? "pulls of " : "residuals of ") + (fit.label.isNull() ? String("the fit") : fit.label));
  for (unsigned int k=0; k<data.nBinsTotal(); k++)
    {
    double sigma = data.statErrorAt(k);
    if (!(sigma > 0) || !insideFit(fit, data.coordinatesAt(k))) continue;
    double f = modelValue(*fit.function, data, k, binAveraging);
    if (!std::isfinite(f)) continue;
    double d = data.valueAt(k) - f;
    if (kind == Residual::kPull) map.setBinAtGlobal(k, 1, d/sigma, 1.0);
    else                         map.setBinAtGlobal(k, 1, d, sigma);
    }
  return map;
  }

  DataPointsVector residuals(const DataPoints & data, const FitRecord & fit, Residual kind, bool binAveraging)
  {
  if (!fit.function || fit.function->rank() != data.rank()) throw MathException("The fit and the data have different ranks", __FUNCTION__);
  if (data.rank() > 2) throw MathException("Residuals of data of rank above 2 are not available", __FUNCTION__);
  DataPointsVector points;
  for (unsigned int k=0; k<data.size(); k++)
    {
    double sigma = data.statErrorAt(k);
    std::vector<double> x = data.coordinatesAt(k);
    if (!(sigma > 0) || !insideFit(fit, x)) continue;
    double f = modelValue(*fit.function, data, k, binAveraging);
    if (!std::isfinite(f)) continue;
    double d = data.valueAt(k) - f, r = kind == Residual::kPull ? d/sigma : d, e = kind == Residual::kPull ? 1.0 : sigma;
    if (data.rank() == 1) points.addPoint(DataPoint2D(x[0], r, e));
    else points.addPoint(DataPoint3D(x[0], x[1], r, e));
    }
  return points;
  }

  Histogram projection(const Histogram & data, unsigned int axis, unsigned int first, unsigned int last)
  {
  Histogram p = data.project2DOnto1D(axis, first, last);
  p.clearFits();
  unsigned int other = 1 - axis, n = data.nBins(other);
  unsigned int f = std::min(first, n - 1), l = std::min(last, n - 1);
  std::vector<double> lowEdges, highEdges;
  for (unsigned int j=f; j<=l; j++) { lowEdges.push_back(data.binLowEdge(j, other)); highEdges.push_back(data.binHighEdge(j, other)); }
  for (unsigned int k=0; k<data.nFits(); k++)
    {
    const FitRecord & fit = data.fit(k);
    if (!fit.function || fit.function->rank() != 2) continue;
    FitRecord r;
    r.label = fit.label + String(axis == 0 ? " (x projection)" : " (y projection)");
    r.function = std::make_shared<ProjectedFunction>(*fit.function, axis, lowEdges, highEdges);
    if (fit.low.size() == 2) { r.low = { fit.low[axis] }; r.high = { fit.high[axis] }; }
    r.result = fit.result;
    r.result.statistic = "chi square of the projection";
    r.result.method = String("projection of: ") + fit.result.method;
    double chi2 = 0;
    int used = 0;
    for (unsigned int i=0; i<p.nBinsTotal(); i++)
      {
      double sigma = p.statErrorAt(i), x = p.coordinatesAt(i)[0];
      if (!(sigma > 0) || (!r.low.empty() && (x < r.low[0] || x > r.high[0]))) continue;
      double g = r.function->evaluateAt(x);
      if (!std::isfinite(g)) continue;
      chi2 += (p.valueAt(i) - g)*(p.valueAt(i) - g)/(sigma*sigma);
      used++;
      }
    r.result.chiSquare = chi2;
    r.result.nPoints = used;
    r.result.nDegreesOfFreedom = used;
    r.result.chiSquarePerDegreeOfFreedom = used > 0 ? chi2/used : 0.0;
    r.result.pValue = used > 0 ? Distributions::chiSquareUpper(chi2, used) : 0.0;
    p.addFit(r);
    }
  return p;
  }

  // ------------------------------------------------------------------------------------------------ ProjectedFunction

  ProjectedFunction::ProjectedFunction(const ParameterizedFunction & function, unsigned int axis, const std::vector<double> & lowEdges,
                                       const std::vector<double> & highEdges)
  :
  ParameterizedFunction(function.parameters(), 1),
  _function(function.clone()),
  _axis(axis),
  _lowEdges(lowEdges),
  _highEdges(highEdges),
  _synchronized()
  {
  if (function.rank() != 2) throw MathException("Only functions of rank 2 are projected", __FUNCTION__);
  if (axis > 1) throw MathException("The axis of a projection is 0 or 1", __FUNCTION__);
  if (lowEdges.size() != highEdges.size()) throw MathException("The bin edges differ in number", __FUNCTION__);
  }

  ProjectedFunction::ProjectedFunction(const ProjectedFunction & source)
  :
  ParameterizedFunction(source),
  _function(source._function->clone()),
  _axis(source._axis),
  _lowEdges(source._lowEdges),
  _highEdges(source._highEdges),
  _synchronized()
  { }

  String ProjectedFunction::registryName() const
  {
  return FunctionRegistry::typeName(*_function) + String(" (projection)");
  }

  const ParameterizedFunction & ProjectedFunction::function() const
  {
  std::vector<double> values = parameterValues();
  if (values != _synchronized)
    {
    for (unsigned int k=0; k<nParameters(); k++) _function->setParameter(k, _parameters[k]);
    _synchronized = values;
    }
  return *_function;
  }

  double ProjectedFunction::evaluateAt(double t) const
  {
  static const double nodes[3]   = { -0.7745966692414834, 0.0, 0.7745966692414834 };
  static const double weights[3] = { 5.0/18.0, 8.0/18.0, 5.0/18.0 };
  const ParameterizedFunction & f = function();
  double sum = 0;
  for (std::size_t j=0; j<_lowEdges.size(); j++)
    {
    double center = 0.5*(_lowEdges[j] + _highEdges[j]), half = 0.5*(_highEdges[j] - _lowEdges[j]);
    for (int q=0; q<3; q++)
      {
      double u = center + half*nodes[q];
      sum += weights[q]*(_axis == 0 ? f.evaluateAt(t, u) : f.evaluateAt(u, t));
      }
    }
  return sum;
  }

  } // namespace MATH
} // namespace CAP
