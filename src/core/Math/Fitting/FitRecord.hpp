/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
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
#include <utility>
#include <vector>
#include "String.hpp"
#include "FitResult.hpp"

namespace CAP
{
  class BinaryWriter;
  class BinaryReader;

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
    //! @brief Whether a point of any rank is within the range of the fit. @param x coordinates @return true if so
    bool inRange(const std::vector<double> & x) const;
    //! @brief Whether the statistic is a chi square, or behaves as one (binned likelihood): not an unbinned
    //! likelihood. @return true if so
    bool chiSquareLike() const;
    //! @brief Name of parameter k of the function ("p<k>" if it has none). @param k index @return name
    String parameterName(unsigned int k) const;
    //! @brief Value and uncertainty of parameter k: those of the result, or else the value of the function and 0.
    //! @param k index @param value set to the value @param error set to the uncertainty
    void parameterAt(unsigned int k, double & value, double & error) const;
    //!
    //! @brief Quality of the fit as (name, value) rows, as in the fit pane of CapGui: chi square (or -2 ln L), degrees
    //! of freedom, chi square per degree of freedom and p-value (for a chi square), points, convergence, uncertainties,
    //! status, statistic, method, evaluations, iterations, range. @return rows
    //!
    std::vector<std::pair<String,String>> quality() const;
    //!
    //! @brief The fit as tab-separated text: label and class of the function, the quality rows, the parameters
    //! (value, uncertainty, bounds) and the correlations of the free parameters. @return text
    //!
    String text() const;
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

  //! @name Binary encoding (part of the payload of the objects the fits are attached to, in CAP files)
  //!
  //! The fits are written as a format version (1 byte), the number of fits, and each fit as a block preceded by its
  //! length: the name of the class of its function (see FunctionRegistry), the number of parameters, the rank, the
  //! name and title of the function, its parameters (names, values, types and bounds), the label, the FitResult and
  //! the range. A fit whose class is not registered (or cannot be made with that number of parameters and rank) is
  //! skipped when read; the other fits are still read.
  //! @{

  //! @brief Write the fits. @param w writer
  void encodeBinary(BinaryWriter & w) const;
  //!
  //! @brief Replace the fits by those read. @param r reader @return number of fits skipped (class not registered)
  //! @throws Exception if the data is invalid (the fits are then unchanged)
  //!
  unsigned int decodeBinary(BinaryReader & r);

  //! @}

  protected:

  std::vector<std::shared_ptr<const FitRecord>> _records;   //!< fits attached
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__FitRecord
