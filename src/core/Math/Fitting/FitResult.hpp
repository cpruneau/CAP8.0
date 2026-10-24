/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FitResult.hpp
//! @brief FitResult: the outcome of a fit (see Fitter).
#ifndef CAP__MATH__FitResult
#define CAP__MATH__FitResult
#include <vector>
#include "String.hpp"
#include "Matrix.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Outcome of a fit: the parameters with their uncertainties and covariance, and the quality of the fit.
  //!
  //! Values and errors are given for all the parameters (errors are 0 for Fixed ones); the covariance and correlation
  //! matrices are those of the free parameters, in the order of freeParameters.
  //!
  struct FitResult
  {
    bool   converged = false;   //!< the minimization converged within the tolerances
    bool   validErrors = false;   //!< the covariance could be computed (the curvature matrix is not singular)
    String status;   //!< how the fit ended
    String method;   //!< method used (e.g. "Levenberg-Marquardt", "differential evolution + Levenberg-Marquardt")
    String statistic;   //!< statistic minimized (see Likelihood::name())
    double chiSquare = 0;   //!< value of the statistic at the best parameters: the chi square, or -2 ln L
    int    nPoints = 0;   //!< points (bins, events) used
    int    nDegreesOfFreedom = 0;   //!< points used minus free parameters
    double chiSquarePerDegreeOfFreedom = 0;   //!< chi square / degrees of freedom (0 if none)
    double pValue = 0;   //!< probability of a larger chi square (see ChiSquare::probability()); 0 if no degree of
                         //!< freedom, or for a statistic that is not chi square distributed (unbinned likelihood)
    std::vector<double> values;   //!< best values of all the parameters
    std::vector<double> errors;   //!< uncertainties of all the parameters (0 for fixed ones)
    std::vector<bool>   atLimit;   //!< parameters that ended at one of their bounds (their errors are then unreliable)
    std::vector<unsigned int> freeParameters;   //!< indices of the free parameters
    Matrix covariance;   //!< covariance of the free parameters
    Matrix correlation;   //!< correlations of the free parameters
    long   nEvaluations = 0;   //!< evaluations of the chi square (or of all the residuals)
    long   nIterations = 0;   //!< iterations
  };
  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__FitResult
