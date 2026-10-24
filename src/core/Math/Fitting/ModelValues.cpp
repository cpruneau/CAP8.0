/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file ModelValues.cpp
//! @brief Implementation of the values of a model at data points.
#include <cmath>
#include <limits>
#include <vector>
#include "ModelValues.hpp"
#include "DataPoints.hpp"
#include "Function.hpp"
#include "Histogram.hpp"

namespace CAP
{
  namespace MATH
  {

  double binAverage(const Function & function, const Histogram & histogram, unsigned int index)
  {
  static const double nodes[3]   = { -0.7745966692414834, 0.0, 0.7745966692414834 };   // sqrt(3/5)
  static const double weights[3] = { 5.0/18.0, 8.0/18.0, 5.0/18.0 };                  // sum 1: an average
  unsigned int rank = histogram.rank();
  std::vector<double> center(rank), half(rank);
  unsigned int rest = index;
  for (unsigned int k=0; k<rank; k++)                      // axis 0 varies fastest
    {
    unsigned int n = histogram.nBins(k), i = rest % n;
    rest /= n;
    center[k] = histogram.binCenter(i, k);
    half[k] = 0.5*histogram.binWidth(i, k);
    }
  unsigned int nPoints = 1;
  for (unsigned int k=0; k<rank; k++) nPoints *= 3;
  std::vector<double> x(rank);
  double sum = 0;
  try
    {
    for (unsigned int p=0; p<nPoints; p++)
      {
      double weight = 1;
      unsigned int code = p;
      for (unsigned int k=0; k<rank; k++)
        {
        unsigned int j = code % 3;
        code /= 3;
        x[k] = center[k] + half[k]*nodes[j];
        weight *= weights[j];
        }
      sum += weight*(rank == 1 ? function.evaluateAt(x[0]) : function.evaluateAt(x));
      }
    }
  catch (...) { return std::numeric_limits<double>::quiet_NaN(); }
  return sum;
  }

  double modelValue(const Function & function, const DataPoints & data, unsigned int index, bool binAveraging)
  {
  if (binAveraging)
    if (const Histogram * histogram = dynamic_cast<const Histogram*>(&data)) return binAverage(function, *histogram, index);
  try { return data.rank() == 1 ? function.evaluateAt(double(data.point(index).x())) : function.evaluateAt(data.coordinatesAt(index)); }
  catch (...) { return std::numeric_limits<double>::quiet_NaN(); }
  }

  } // namespace MATH
} // namespace CAP
