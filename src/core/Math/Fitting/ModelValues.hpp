/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file ModelValues.hpp
//! @brief Values of a model at data points: at their coordinates, or averaged over the bins of a histogram.
#ifndef CAP__MATH__ModelValues
#define CAP__MATH__ModelValues

namespace CAP
{
  namespace MATH
  {

  class Function;
  class DataPoints;
  class Histogram;

  //!
  //! @brief Average of a function over bin index of a histogram (any rank): the 3-point Gauss-Legendre rule along each
  //! axis (3^rank evaluations), exact for polynomials of degree 5 in each variable.
  //! @param function function (of the rank of the histogram) @param histogram histogram @param index global bin index
  //! @return average, NaN if the function cannot be evaluated
  //!
  double binAverage(const Function & function, const Histogram & histogram, unsigned int index);

  //!
  //! @brief Value of a model for point index of data: the function at the coordinates of the point, or, if
  //! binAveraging and the data are a histogram, its average over the bin (binAverage()). Used by the chi square and
  //! the Poisson likelihood (see Fitter::setBinAveraging()). @param function function @param data data
  //! @param index point @param binAveraging average over the bins of a histogram @return value, NaN if it cannot be computed
  //!
  double modelValue(const Function & function, const DataPoints & data, unsigned int index, bool binAveraging);

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__ModelValues
