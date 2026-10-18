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
//! @file Fitter.hpp
//! @brief Fitter: least-squares fit of a ParameterizedFunction to DataPoints (chi square), and FitResult.
#ifndef CAP__MATH__Fitter
#define CAP__MATH__Fitter
#include <vector>
#include "Aliases.hpp"
#include "ChiSquare.hpp"
#include "ExtremumSearch.hpp"
#include "Matrix.hpp"

namespace CAP
{
  namespace MATH
  {

  class ParameterizedFunction;
  class DataPoints;

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
    double chiSquare = 0;   //!< chi square at the best parameters
    int    nPoints = 0;   //!< points used (those with a nonzero uncertainty)
    int    nDegreesOfFreedom = 0;   //!< points used minus free parameters
    double chiSquarePerDegreeOfFreedom = 0;   //!< chi square / degrees of freedom (0 if none)
    double pValue = 0;   //!< probability of a larger chi square (see ChiSquare::probability()), 0 if no degree of freedom
    std::vector<double> values;   //!< best values of all the parameters
    std::vector<double> errors;   //!< uncertainties of all the parameters (0 for fixed ones)
    std::vector<bool>   atLimit;   //!< parameters that ended at one of their bounds (their errors are then unreliable)
    std::vector<unsigned int> freeParameters;   //!< indices of the free parameters
    Matrix covariance;   //!< covariance of the free parameters
    Matrix correlation;   //!< correlations of the free parameters
    long   nEvaluations = 0;   //!< evaluations of the chi square (or of all the residuals)
    long   nIterations = 0;   //!< iterations
  };

  //!
  //! @brief Least-squares fit of a ParameterizedFunction to DataPoints of the same rank: the parameters that minimize
  //! the chi square (see ChiSquare: statistical or total uncertainties, optional uncertainties on the coordinates).
  //!
  //! Methods:
  //!  - kLevenbergMarquardt (the default): Gauss-Newton steps with an adaptive damping, the standard for least
  //!    squares; fast and precise from reasonable starting values (the current values of the parameters).
  //!  - kSearch: any engine of ExtremumSearch on the chi square as a function of the free parameters (search() sets
  //!    the method, its precision and budget): e.g. differential evolution or simulated annealing when the starting
  //!    values are poor or the chi square has several minima. Global engines need bounds on the free parameters (see
  //!    ParameterizedFunction::setParameterLimits()). The result is then refined by Levenberg-Marquardt (setPolish()).
  //!
  //! Fixed parameters keep their values; bounded parameters (Low, High, LowHigh) stay within their bounds. Points
  //! with a zero uncertainty (e.g. the empty bins of a histogram) are left out. The uncertainties of the parameters
  //! come from the covariance C = (J^T J)^-1, J being the derivatives of the normalized residuals (y - f)/sigma with
  //! respect to the free parameters at the minimum (twice the inverse of the curvature of the chi square); they can be
  //! scaled by chi square / degrees of freedom (setScaleErrors()) when the uncertainties of the data are not known
  //! absolutely.
  //!
  //! The fit leaves the function at the best parameters.
  //!
  //! @code
  //!   CAP::MATH::Normal model(100.0, 0.0, 1.0);          // starting values
  //!   CAP::MATH::Fitter fitter(model, histogram);
  //!   CAP::MATH::FitResult r = fitter.fit();
  //!   if (r.converged) std::cout << r.values[1] << " +- " << r.errors[1] << "  chi2/ndf " << r.chiSquarePerDegreeOfFreedom;
  //! @endcode
  //!
  class Fitter
  {
  public:

  //! @brief Methods of fit.
  enum Method { kLevenbergMarquardt, kSearch };

  //! @brief Fitter. @param function function fitted (not owned; its parameters are changed by the fit)
  //! @param data data points (not owned) @throws MathException if the ranks differ
  Fitter(ParameterizedFunction & function, const DataPoints & data);

  //! @brief The chi square (to set its error mode and the use of coordinate uncertainties). @return chi square
  ChiSquare & chiSquare() { return _chiSquare; }
  //! @brief Method. @return method
  Method method() const { return _method; }
  //! @brief Set the method. @param method method
  void setMethod(Method method) { _method = method; }
  //! @brief The search engine of kSearch (method, options). @return engine
  ExtremumSearch & search() { return _search; }
  //! @brief Whether the result of kSearch is refined by Levenberg-Marquardt. @return true if so
  bool polish() const { return _polish; }
  //! @brief Refine the result of kSearch by Levenberg-Marquardt, or not. @param v refine
  void setPolish(bool v) { _polish = v; }
  //! @brief Set the maximum number of Levenberg-Marquardt iterations. @param n number
  void setMaxIterations(int n) { _maxIterations = n < 1 ? 1 : n; }
  //! @brief Set the tolerances: relative change of the chi square, and relative change of the parameters, below
  //! which the fit has converged. @param chiSquare tolerance on the chi square @param parameters tolerance on the parameters
  void setTolerances(double chiSquare, double parameters) { _chiSquareTolerance = chiSquare; _parameterTolerance = parameters; }
  //! @brief Whether the errors are scaled by sqrt(chi square / degrees of freedom). @return true if so
  bool scaleErrors() const { return _scaleErrors; }
  //! @brief Scale the errors by sqrt(chi square / degrees of freedom), or not. @param v scale
  void setScaleErrors(bool v) { _scaleErrors = v; }

  //! @brief Fit: the function is left at the best parameters. @return result
  //! @throws MathException if there is no free parameter or no point with a nonzero uncertainty
  FitResult fit();
  //! @brief Result of the last fit. @return result
  const FitResult & result() const { return _result; }

  protected:

  //! @brief Set the free parameters (clamped to their bounds). @param p values of the free parameters
  void setFree(const std::vector<double> & p);
  //! @brief Normalized residuals of the points used. @param r residuals @return false if one cannot be computed
  bool residuals(std::vector<double> & r);
  //! @brief Chi square at free parameters p (the parameters are left at p). @param p values @return chi square, or
  //! +infinity if it cannot be computed
  double chiSquareAt(const std::vector<double> & p);
  //! @brief Derivatives of the residuals with respect to the free parameters (central differences, one-sided at a
  //! bound). @param p values of the free parameters @param jacobian n x m matrix @return false if they cannot be computed
  bool jacobian(const std::vector<double> & p, Matrix & jacobian);
  //! @brief Levenberg-Marquardt from the current values. @param result result (updated) @return true if converged
  bool levenbergMarquardt(FitResult & result);
  //! @brief Fill the covariance, errors and quality of the result at the current values. @param result result
  void finish(FitResult & result);

  ParameterizedFunction & _function;   //!< function fitted
  const DataPoints & _data;   //!< data fitted
  ChiSquare _chiSquare;   //!< chi square of the data and the function
  Method _method;   //!< method
  ExtremumSearch _search;   //!< engine of kSearch
  bool   _polish;   //!< kSearch refined by Levenberg-Marquardt
  int    _maxIterations;   //!< Levenberg-Marquardt iterations allowed
  double _chiSquareTolerance;   //!< relative change of the chi square at convergence
  double _parameterTolerance;   //!< relative change of the parameters at convergence
  bool   _scaleErrors;   //!< errors scaled by sqrt(chi2/ndf)
  std::vector<unsigned int> _free;   //!< indices of the free parameters
  std::vector<unsigned int> _used;   //!< indices of the points used
  std::vector<double> _scale;   //!< typical size of each free parameter (for the derivatives)
  long   _nEvaluations;   //!< evaluations of all the residuals
  FitResult _result;   //!< result of the last fit
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__Fitter
