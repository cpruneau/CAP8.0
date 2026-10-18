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
//! @file FitRecord.hpp
//! @brief FitRecord: a fit attached to the data points or the function it was made to.
#ifndef CAP__MATH__FitRecord
#define CAP__MATH__FitRecord
#include <memory>
#include <vector>
#include "Aliases.hpp"
#include "FitResult.hpp"

namespace CAP
{
  namespace MATH
  {

  class ParameterizedFunction;

  //!
  //! @brief A fit attached to the object it was made to (DataPoints::addFit(), Function::addFit()): its own copy of the
  //! fitted function at the best parameters, the FitResult, a label, and the range of the fit.
  //!
  //! The function is a copy (see ParameterizedFunction::clone()): changing or fitting the model again leaves the
  //! record unchanged. Records are made by Fitter::record() and shared (std::shared_ptr) by the copies of the object
  //! they are attached to.
  //!
  struct FitRecord
  {
    String label;   //!< label of the fit (e.g. "gaussian"), shown in legends and tabs
    std::shared_ptr<const ParameterizedFunction> function;   //!< copy of the fitted function, at the best parameters
    FitResult result;   //!< parameters, uncertainties, covariance, chi square, degrees of freedom, p-value
    std::vector<double> low;   //!< lower ends of the range of the fit, one per variable
    std::vector<double> high;   //!< upper ends of the range of the fit, one per variable

    //! @brief Value of the fitted function at a point of rank 1. @param x x @return value (NaN if it cannot be computed)
    double valueAt(double x) const;
    //! @brief Value of the fitted function at a point. @param x coordinates @return value (NaN if it cannot be computed)
    double valueAt(const std::vector<double> & x) const;
    //! @brief Whether a point of rank 1 is within the range of the fit. @param x x @return true if so
    bool inRange(double x) const { return !low.empty() && x >= low[0] && x <= high[0]; }
  };

  //!
  //! @brief The fits attached to an object (see DataPoints::addFit(), Function::addFit()): shared records, in the
  //! order they were added. Copies of the list share the records.
  //!
  class FitList
  {
  public:

  //! @brief Attach a copy of a record. @param record record
  void add(const FitRecord & record) { _records.push_back(std::make_shared<const FitRecord>(record)); }
  //! @brief Attach a record (shared). @param record record (ignored if null)
  void add(const std::shared_ptr<const FitRecord> & record) { if (record) _records.push_back(record); }
  //! @brief Number of fits. @return number
  unsigned int size() const { return (unsigned int)(_records.size()); }
  //! @brief Fit k. @param k index @return record @throws MathException if k is out of range
  const FitRecord & at(unsigned int k) const;
  //! @brief Fit k, shared. @param k index @return record @throws MathException if k is out of range
  std::shared_ptr<const FitRecord> pointer(unsigned int k) const;
  //! @brief Detach fit k. @param k index @throws MathException if k is out of range
  void remove(unsigned int k);
  //! @brief Detach all the fits.
  void clear() { _records.clear(); }

  protected:

  std::vector<std::shared_ptr<const FitRecord>> _records;   //!< fits attached
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__FitRecord
