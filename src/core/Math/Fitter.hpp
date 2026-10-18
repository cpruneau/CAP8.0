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
//! @brief Fitter: fit of a ParameterizedFunction to data by least squares or maximum likelihood, and FitResult.
#ifndef CAP__MATH__Fitter
#define CAP__MATH__Fitter
#include <memory>
#include <vector>
#include "Aliases.hpp"
#include "ChiSquare.hpp"
#include "Likelihood.hpp"
#include "ExtremumSearch.hpp"
#include "DataPointsVector.hpp"
#include "FitRecord.hpp"
#include "FitResult.hpp"
#include "Matrix.hpp"

namespace CAP
{
  namespace MATH
  {

  class ParameterizedFunction;
  class DataPoints;

  //!
  //! @brief Fit of a ParameterizedFunction to data: the parameters that minimize a statistic, -2 ln L (see Likelihood):
  //!  - the chi square of DataPoints of the same rank (the default; see ChiSquare: statistical or total
  //!    uncertainties, optional uncertainties on the coordinates): least squares;
  //!  - the Poisson likelihood of counts (kPoisson; see PoissonLikelihood): binned maximum likelihood, for histograms
  //!    with few entries per bin;
  //!  - any Likelihood, e.g. an UnbinnedLikelihood of individual events: unbinned maximum likelihood.
  //!
  //! Methods:
  //!  - kLevenbergMarquardt (the default): Gauss-Newton steps with an adaptive damping, the standard for least
  //!    squares; fast and precise from reasonable starting values (the current values of the parameters).
  //!  - kSearch: any engine of ExtremumSearch on the chi square as a function of the free parameters (search() sets
  //!    the method, its precision and budget): e.g. differential evolution or simulated annealing when the starting
  //!    values are poor or the chi square has several minima. Global engines need bounds on the free parameters (see
  //!    ParameterizedFunction::setParameterLimits()). The result is then refined by Levenberg-Marquardt (setPolish()).
  //!
  //! Levenberg-Marquardt works on the residuals of a statistic made of residuals (chi square, Poisson likelihood); for
  //! the others (unbinned likelihood) it becomes a damped Newton method on the numerical gradient and curvature of the
  //! statistic. The search engines work on the free parameters scaled by their typical sizes (their ranges, or their
  //! starting values).
  //!
  //! Fixed parameters keep their values; bounded parameters (Low, High, LowHigh) stay within their bounds. Points
  //! with a zero uncertainty (e.g. the empty bins of a histogram) are left out. The uncertainties of the parameters
  //! come from the covariance C = (J^T J)^-1, J being the derivatives of the residuals with respect to the free
  //! parameters at the minimum; without residuals, from C = 2 H^-1, H being the curvature (second derivatives) of
  //! -2 ln L. They can be scaled by chi square / degrees of freedom (setScaleErrors()) when the uncertainties of the
  //! data are not known absolutely.
  //!
  //! The fit leaves the function at the best parameters. record() makes a FitRecord of the last fit (a copy of the
  //! function, the result, the range), to attach to the data (DataPoints::addFit()) or to the function that was
  //! fitted (Function::addFit(); a function is fitted through samples of it, see samples()).
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
  //! @brief Statistics of DataPoints.
  enum Statistic { kChiSquare, kPoisson };

  //! @brief Fitter. @param function function fitted (not owned; its parameters are changed by the fit)
  //! @param data data points (not owned) @param statistic chi square or Poisson likelihood
  //! @throws MathException if the ranks differ
  Fitter(ParameterizedFunction & function, const DataPoints & data, Statistic statistic=kChiSquare);
  //! @brief Fitter of any likelihood (e.g. an UnbinnedLikelihood). @param likelihood likelihood (not owned; its
  //! function's parameters are changed by the fit)
  explicit Fitter(Likelihood & likelihood);

  //! @brief The statistic minimized. @return likelihood
  Likelihood & likelihood() { return *_likelihood; }
  //! @brief The chi square (to set its error mode and the use of coordinate uncertainties), for a chi square fit.
  //! @return chi square @throws MathException if the statistic is not the chi square
  ChiSquare & chiSquare();
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
  //! @throws MathException if there is no free parameter or no point can be used
  FitResult fit();
  //!
  //! @brief Covariance of the free parameters at their current values: (J^T J)^-1 for a statistic made of residuals,
  //! otherwise 2 H^-1. @param covariance set to the covariance @return false if it cannot be computed (singular)
  //!
  bool covarianceAt(Matrix & covariance);
  //! @brief Result of the last fit. @return result
  const FitResult & result() const { return _result; }
  //!
  //! @brief Record of the last fit, to attach to the data or the function fitted: a copy of the function (at the best
  //! parameters), the result, the range covered by the points used, and a label.
  //! @param label label (empty: the class of the function and "fit", e.g. "Normal fit")
  //! @return record
  //! @throws MathException if no fit was made, or the function cannot be copied (see ParameterizedFunction::clone())
  //!
  FitRecord record(const String & label="") const;
  //!
  //! @brief Samples of a function of rank 1, to fit another function to it: n points equally spaced over [low, high]
  //! (ends included), each with the same uncertainty (the chi square is then the sum of the squared differences
  //! divided by error^2). @param function function sampled @param low lower end @param high upper end @param n number
  //! of points (at least 2) @param error uncertainty given to each point @return points
  //! @throws MathException if the function is not of rank 1, n < 2 or error <= 0
  //!
  static DataPointsVector samples(const Function & function, double low, double high, unsigned int n, double error=1.0);

  protected:

  //! @brief Set the free parameters (clamped to their bounds). @param p values of the free parameters
  void setFree(const std::vector<double> & p);
  //! @brief Residuals of the points used. @param r residuals @return false if they cannot be computed
  bool residuals(std::vector<double> & r);
  //! @brief Statistic at free parameters p (the parameters are left at p). @param p values @return statistic, or
  //! +infinity if it cannot be computed
  double chiSquareAt(const std::vector<double> & p);
  //! @brief Second derivatives of the statistic with respect to the free parameters. @param p values
  //! @param hessian m x m matrix @return false if they cannot be computed
  bool hessian(const std::vector<double> & p, Matrix & hessian);
  //! @brief Prepare a fit: free parameters, points used, scales. @throws MathException as fit()
  void prepare();
  //! @brief Derivatives of the residuals with respect to the free parameters (central differences, one-sided at a
  //! bound). @param p values of the free parameters @param jacobian n x m matrix @return false if they cannot be computed
  bool jacobian(const std::vector<double> & p, Matrix & jacobian);
  //! @brief Levenberg-Marquardt from the current values. @param result result (updated) @return true if converged
  bool levenbergMarquardt(FitResult & result);
  //! @brief Damped Newton method (Levenberg-Marquardt on the gradient and curvature of the statistic), for a statistic
  //! without residuals. @param result result (updated) @return true if converged
  bool newton(FitResult & result);
  //! @brief Numerical gradient of the statistic. @param p values of the free parameters @param gradient gradient
  //! @return false if it cannot be computed
  bool gradient(const std::vector<double> & p, std::vector<double> & gradient);
  //! @brief Fill the covariance, errors and quality of the result at the current values. @param result result
  void finish(FitResult & result);

  ParameterizedFunction & _function;   //!< function fitted
  std::unique_ptr<Likelihood> _owned;   //!< statistic made by the fitter (DataPoints constructor)
  Likelihood * _likelihood;   //!< statistic minimized
  Method _method;   //!< method
  ExtremumSearch _search;   //!< engine of kSearch
  bool   _polish;   //!< kSearch refined by Levenberg-Marquardt
  int    _maxIterations;   //!< Levenberg-Marquardt iterations allowed
  double _chiSquareTolerance;   //!< relative change of the chi square at convergence
  double _parameterTolerance;   //!< relative change of the parameters at convergence
  bool   _scaleErrors;   //!< errors scaled by sqrt(chi2/ndf)
  std::vector<unsigned int> _free;   //!< indices of the free parameters
  std::vector<double> _scale;   //!< typical size of each free parameter (for the derivatives)
  long   _nEvaluations;   //!< evaluations of all the residuals
  bool   _fitted = false;   //!< a fit was made
  FitResult _result;   //!< result of the last fit
  };

  } // namespace MATH
} // namespace CAP

#endif // !CAP__MATH__Fitter
