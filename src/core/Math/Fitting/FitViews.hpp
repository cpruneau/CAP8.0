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
//! @file FitViews.hpp
//! @brief Views of fits: maps of residuals, projections of fits of rank 2 (ProjectedFunction).
#ifndef CAP__MATH__FitViews
#define CAP__MATH__FitViews
#include <memory>
#include <vector>
#include "DataPointsVector.hpp"
#include "Histogram.hpp"
#include "ParameterizedFunction.hpp"

namespace CAP
{
  namespace MATH
  {

  struct FitRecord;

  //! @brief What a residual is: data - fit, or the pull (data - fit)/(statistical uncertainty).
  enum class Residual { kDifference, kPull };

  //!
  //! @brief Residuals of a histogram (any rank) to a fit, as a histogram of the same bins: each bin with a statistical
  //! uncertainty and inside the range of the fit holds data - fit (with the uncertainty of the data) or the pull
  //! (with an uncertainty of 1); the other bins are empty. Its name is that of the data followed by "_residuals"
  //! (or "_pulls"); it has no fit.
  //! @param data histogram @param fit fit of the rank of the histogram @param kind difference or pull
  //! @param binAveraging compare to the average of the fit over each bin (see binAverage())
  //! @return histogram of the residuals @throws MathException if the ranks differ
  //!
  Histogram residualMap(const Histogram & data, const FitRecord & fit, Residual kind=Residual::kPull, bool binAveraging=false);

  //!
  //! @brief Residuals of data points (rank 1 or 2) to a fit: points (x, r) or (x, y, r), r being data - fit (with the
  //! uncertainty of the data) or the pull (with an uncertainty of 1), for the points with a statistical uncertainty
  //! inside the range of the fit. @param data data points @param fit fit @param kind difference or pull
  //! @param binAveraging average the fit over the bins of a histogram @return points
  //! @throws MathException if the ranks differ or are above 2
  //!
  DataPointsVector residuals(const DataPoints & data, const FitRecord & fit, Residual kind=Residual::kPull, bool binAveraging=false);

  //!
  //! @brief Projection of a histogram of rank 2 onto one of its axes, with its fits of rank 2 projected the same way
  //! and attached (a ProjectedFunction each): the 1D tools for fits (curves, residuals panel, fit pane) then apply to
  //! the projection. Bins [first, last] of the other axis are summed (Histogram::project2DOnto1D()). Each projected fit
  //! keeps the parameters, uncertainties and covariance of the fit, its label followed by " (x projection)" or
  //! " (y projection)", and the range of the fit along the axis kept; its chi square, degrees of freedom (the bins
  //! compared, no parameter being fitted to the projection) and p-value are those of the projection, over the bins with
  //! an uncertainty inside that range. Projected fits are views: their class is not registered, so they are not read
  //! back from files. @param data histogram of rank 2 @param axis axis kept (0: x, 1: y) @param first first bin of
  //! the other axis @param last last bin of the other axis (Histogram::allBins: the last one) @return projection
  //! @throws Exception if the rank is not 2 or the axis or range is invalid
  //!
  Histogram projection(const Histogram & data, unsigned int axis, unsigned int first=0, unsigned int last=Histogram::allBins);

  //!
  //! @brief Projection of a function of rank 2 onto one variable, for histograms: g(t) = sum over the bins j of the
  //! other axis of the average of f over bin j (3-point Gauss-Legendre rule), t being the variable kept. Its parameters
  //! are those of f (copied, and given to f before each evaluation). Made by projection().
  //!
  class ProjectedFunction : public ParameterizedFunction
  {
  public:

  //!
  //! @brief Projection of a function. @param function function of rank 2 (copied) @param axis variable kept (0 or 1)
  //! @param lowEdges lower edges of the bins of the other variable @param highEdges upper edges
  //! @throws MathException if the function is not of rank 2, the axis is not 0 or 1, or the edges differ in number
  //!
  ProjectedFunction(const ParameterizedFunction & function, unsigned int axis, const std::vector<double> & lowEdges,
                    const std::vector<double> & highEdges);
  //! @brief Copy (the function is copied). @param source projection
  ProjectedFunction(const ProjectedFunction & source);
  //! @brief Destructor.
  virtual ~ProjectedFunction() { }
  //! @brief Copy. @return new projection (owned by the caller)
  ProjectedFunction * clone() const override { return new ProjectedFunction(*this); }
  //! @brief Value of the projection. @param t variable kept @return g(t)
  double evaluateAt(double t) const override;
  using ParameterizedFunction::evaluateAt;
  //! @brief The function projected (with the parameters of the projection). @return function
  const ParameterizedFunction & function() const;
  //! @brief Variable kept (0 or 1). @return axis
  unsigned int axis() const { return _axis; }

  protected:

  std::unique_ptr<ParameterizedFunction> _function;   //!< function projected
  unsigned int _axis;   //!< variable kept
  std::vector<double> _lowEdges;   //!< bins of the other variable
  std::vector<double> _highEdges;   //!< bins of the other variable
  mutable std::vector<double> _synchronized;   //!< parameters last given to the function
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__FitViews
