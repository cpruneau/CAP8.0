/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include <algorithm>
#include "RandomDistribution.hpp"
#include "RandomGenerator.hpp"
#include "MathExceptions.hpp"
#include "MathBasicIntegration.hpp"
#include "InterpolationTools.hpp"

namespace CAP
{
  namespace MATH
  {

  TabulatedDistribution::TabulatedDistribution(const std::function<double(double)> & density, double minimum,
                                               double maximum, int nBins)
  :
  _minimum(minimum),
  _maximum(maximum),
  _integral(0.0),
  _cumulative()
  {
  if (!(maximum > minimum) || nBins < 1) throw MathException("Empty interval, or no bins", "TabulatedDistribution");
  double width = (maximum - minimum)/nBins;
  // the density at the edges and the middles of the bins, then Simpson's rule in each bin
  std::vector<double> values(2*nBins + 1);
  for (int k=0; k<nBins; k++)
    {
    double x = minimum + k*width;
    values[2*k] = k == 0 ? density(x) : values[2*k];
    values[2*k+1] = density(x + 0.5*width);
    values[2*k+2] = density(x + width);
    if (values[2*k] < 0.0 || values[2*k+1] < 0.0 || values[2*k+2] < 0.0)
      throw MathException("The density is negative", "TabulatedDistribution");
    }
  _cumulative = cumulativeSimpsonRule(values, 0.5*width);
  _integral = _cumulative.back();
  if (!(_integral > 0.0)) throw MathException("The integral of the density is not positive", "TabulatedDistribution");
  for (auto & c : _cumulative) c /= _integral;
  _edges.resize(nBins + 1);
  for (int k=0; k<=nBins; k++) _edges[k] = minimum + k*width;
  }

  double TabulatedDistribution::generate(RandomGenerator & generator) const
  {
  // the bin where the cumulative distribution reaches u, then a linear interpolation in it
  return inverseLinearInterpolation(_edges, _cumulative, generator.random());
  }

  } // namespace MATH
} // namespace CAP
