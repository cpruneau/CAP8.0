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
//! @file FitRecord.cpp
//! @brief Implementation of FitRecord.
#include <limits>
#include "FitRecord.hpp"
#include "ParameterizedFunction.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  double FitRecord::valueAt(double x) const
  {
  if (!function) return std::numeric_limits<double>::quiet_NaN();
  try { return function->evaluateAt(x); }
  catch (...) { return std::numeric_limits<double>::quiet_NaN(); }
  }

  double FitRecord::valueAt(const std::vector<double> & x) const
  {
  if (!function) return std::numeric_limits<double>::quiet_NaN();
  try { return function->evaluateAt(x); }
  catch (...) { return std::numeric_limits<double>::quiet_NaN(); }
  }

  const FitRecord & FitList::at(unsigned int k) const
  {
  if (k >= _records.size()) throw MathException("Fit index out of range", __FUNCTION__);
  return *_records[k];
  }

  std::shared_ptr<const FitRecord> FitList::pointer(unsigned int k) const
  {
  if (k >= _records.size()) throw MathException("Fit index out of range", __FUNCTION__);
  return _records[k];
  }

  void FitList::remove(unsigned int k)
  {
  if (k >= _records.size()) throw MathException("Fit index out of range", __FUNCTION__);
  _records.erase(_records.begin() + k);
  }

  } // namespace MATH
} // namespace CAP
