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
#include "GaussLegendreIntegrator.hpp"
#include "ModelValues.hpp"
#include "DataPoints.hpp"
#include "Function.hpp"
#include "Histogram.hpp"
#include "ParameterizedFunction.hpp"

namespace CAP
{
  namespace MATH
  {

  double binAverage(const Function & function, const Histogram & histogram, unsigned int index)
  {
  // a function of one variable with parameters: its integral over the bin (analytical when the class has one, e.g.
  // FourierDecomposition, Normal, the polynomials; adaptive Simpson otherwise) divided by the width of the bin
  if (histogram.rank() == 1)
    if (const ParameterizedFunction * f = dynamic_cast<const ParameterizedFunction*>(&function))
      {
      unsigned int i = index % histogram.nBins(0);
      double center = histogram.binCenter(i, 0), width = histogram.binWidth(i, 0);
      if (width > 0.0)
        {
        try { return f->integralInRange(center - 0.5*width, center + 0.5*width)/width; }
        catch (...) { return std::numeric_limits<double>::quiet_NaN(); }
        }
      }
  // otherwise the average over the bin by the product of 3-point Gauss-Legendre rules (exact for cubics in each variable)
  unsigned int rank = histogram.rank();
  std::vector<double> low(rank), high(rank);
  unsigned int rest = index;
  double volume = 1.0;
  for (unsigned int k=0; k<rank; k++)                      // axis 0 varies fastest
    {
    unsigned int n = histogram.nBins(k), i = rest % n;
    rest /= n;
    double center = histogram.binCenter(i, k), half = 0.5*histogram.binWidth(i, k);
    low[k] = center - half;
    high[k] = center + half;
    volume *= 2.0*half;
    }
  try
    {
    double integral = GaussLegendreIntegrator::productRule([&](const std::vector<double> & x)
                        { return rank == 1 ? function.evaluateAt(x[0]) : function.evaluateAt(x); }, low, high, 3);
    return integral/volume;
    }
  catch (...) { return std::numeric_limits<double>::quiet_NaN(); }
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
