/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
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
  _cumulative.push_back(0.0);
  double left = density(minimum);
  for (int k=0; k<nBins; k++)
    {
    double x = minimum + k*width;
    double middle = density(x + 0.5*width), right = density(x + width);
    if (left < 0.0 || middle < 0.0 || right < 0.0) throw MathException("The density is negative", "TabulatedDistribution");
    _integral += width*(left + 4.0*middle + right)/6.0;
    _cumulative.push_back(_integral);
    left = right;
    }
  if (!(_integral > 0.0)) throw MathException("The integral of the density is not positive", "TabulatedDistribution");
  for (auto & c : _cumulative) c /= _integral;
  }

  double TabulatedDistribution::generate(RandomGenerator & generator) const
  {
  double u = generator.random();
  // the bin where the cumulative distribution reaches u, then a linear interpolation in it
  size_t k = size_t(std::upper_bound(_cumulative.begin(), _cumulative.end(), u) - _cumulative.begin());
  if (k == 0) k = 1;
  if (k >= _cumulative.size()) k = _cumulative.size() - 1;
  double c0 = _cumulative[k-1], c1 = _cumulative[k];
  double width = (_maximum - _minimum)/double(_cumulative.size() - 1);
  double f = c1 > c0 ? (u - c0)/(c1 - c0) : 0.5;
  return _minimum + (double(k - 1) + f)*width;
  }

  } // namespace MATH
} // namespace CAP
