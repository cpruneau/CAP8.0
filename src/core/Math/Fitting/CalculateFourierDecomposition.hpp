/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file CalculateFourierDecomposition.hpp
//! @brief CalculateFourierDecomposition: the Fourier decomposition of a function or of data points of an angle, up to
//! a chosen order, by averages or by a fit, with its chi square.
#ifndef CAP_MATH_CalculateFourierDecomposition
#define CAP_MATH_CalculateFourierDecomposition
#include <vector>
#include "DataPointsVector.hpp"
#include "FitResult.hpp"
#include "FourierDecomposition.hpp"

namespace CAP
{
  namespace MATH
  {

  class Function;

  //!
  //! @brief Fourier decomposition (FourierDecomposition) of a function of rank 1, or of data points of rank 1 (e.g. a
  //! histogram of an angle), up to the order nMax chosen by the caller, in either form, and how well it describes them.
  //!
  //! The coefficients of form 1 (k, c_n, s_n) are obtained by one of two methods (setMethod()):
  //!  - kAverage: the averages k = <f>, c_n = 2 <f cos(n x)>, s_n = 2 <f sin(n x)> over N points equally spaced over
  //!    one period (exact for N > 2 nMax + 1: they are then the coefficients of least chi square for equal
  //!    uncertainties). Their uncertainties and covariance are propagated from those of the points (sigma_i), which
  //!    do not weigh the averages. Data points must be equally spaced and cover one period (a histogram of the
  //!    angle over 2 pi), otherwise MathException.
  //!  - kFit: a chi square fit (Fitter, Levenberg-Marquardt) of form 1 to the points, which weighs each point by its
  //!    uncertainty: points need not be equally spaced nor cover a whole period. Uncertainties and covariance from the
  //!    fit.
  //! Those of form 2 follow from form 1 by the trigonometric relations v_n = sqrt(c_n^2 + s_n^2)/(2 k),
  //! psi_n = atan2(s_n, c_n)/n, the covariance propagated (Jacobian of the relations).
  //!
  //! A function is sampled at N points (at the centers of N equal intervals of the period [low, low + 2 pi],
  //! x_i = low + (i + 1/2) 2 pi/N), each given the same uncertainty sigma (setError()).
  //!
  //! For a histogram, the decomposition is compared to the contents either at the centers of the bins (the default)
  //! or integrated over each bin (setBinAveraging(true): the contents are then averages of the function over the
  //! bins). Integrated, the fit and the chi square use the integral of the decomposition over each bin divided by its
  //! width (Fitter::setBinAveraging(), binAverage(), FourierDecomposition::integralInRange(), analytical); the
  //! averages give the same coefficients, as c_n and s_n of the averages divided by sin(n h/2)/(n h/2), the ratio of
  //! the integral of cos(n x) over a bin of width h to h cos(n x) at its center.
  //!
  //! The quality of the decomposition is that of a fit (FitResult, see result()), its chi square computed by ChiSquare
  //! from the decomposition and the points: chi square, number of points, degrees of freedom N - (2 nMax + 1), chi
  //! square per degree of freedom, p-value (ChiSquare::probability()), the coefficients with their uncertainties,
  //! covariance and correlations. The largest and the rms difference at the points are also given.
  //!
  //! @code
  //!   CAP::MATH::CalculateFourierDecomposition calculator(4, CAP::MATH::FourierDecomposition::kFlow);
  //!   calculator.setMethod(CAP::MATH::CalculateFourierDecomposition::kFit);
  //!   CAP::MATH::FourierDecomposition d = calculator.calculate(histogram);   // or a function: calculate(f)
  //!   const CAP::MATH::FitResult & r = calculator.result();
  //!   std::cout << "v2 = " << d.flow(2) << " +- " << r.errors[d.harmonicIndex(2)]
  //!             << ", chi2/ndf = " << r.chiSquarePerDegreeOfFreedom << ", p = " << r.pValue;
  //! @endcode
  //!
  class CalculateFourierDecomposition
  {
  public:

  using Form = FourierDecomposition::Form;   //!< form of the decomposition

  //! @brief How the coefficients are obtained.
  enum Method
  {
    kAverage,   //!< averages over points equally spaced over one period
    kFit        //!< chi square fit, each point weighed by its uncertainty
  };

  //! @brief Calculator of decompositions of order nMax. @param nMax highest harmonic (1 or more) @param form form
  //! @param method method @throws MathException if nMax is 0
  CalculateFourierDecomposition(unsigned int nMax, Form form=FourierDecomposition::kCosineSine, Method method=kAverage);
  //! @brief Destructor.
  virtual ~CalculateFourierDecomposition() { }

  //! @name Settings
  //! @{
  //! @brief Highest harmonic. @return nMax
  unsigned int nMax() const { return _nMax; }
  //! @brief Set the highest harmonic. @param nMax highest harmonic (1 or more) @throws MathException if 0
  void setNMax(unsigned int nMax);
  //! @brief Form of the decompositions. @return form
  Form form() const { return _form; }
  //! @brief Set the form of the decompositions. @param form form
  void setForm(Form form) { _form = form; }
  //! @brief Method. @return method
  Method method() const { return _method; }
  //! @brief Set the method. @param method kAverage or kFit
  void setMethod(Method method) { _method = method; }
  //! @brief Number of samples N of a function over the period (0: automatic, max(360, 8 (nMax + 1))). @return N as set
  unsigned int nSamples() const { return _nSamples; }
  //! @brief Set the number of samples of a function. @param n samples (0: automatic; otherwise more than 2 nMax + 1)
  void setNSamples(unsigned int n) { _nSamples = n; }
  //! @brief Uncertainty sigma given to each sample of a function. @return sigma
  double error() const { return _error; }
  //! @brief Set the uncertainty given to each sample of a function. @param sigma uncertainty (> 0)
  //! @throws MathException if not positive
  void setError(double sigma);
  //! @brief Lower end of the period over which a function is sampled. @return low
  double low() const { return _low; }
  //! @brief Set the period [low, low + 2 pi] over which a function is sampled (by default [-pi, pi]). @param low lower end
  void setLow(double low) { _low = low; }
  //! @brief Whether the points are averages of the function over bins (a histogram). @return true if so
  bool binAveraging() const { return _binAveraging; }
  //! @brief Treat the points as averages over bins of the function (a histogram). @param v true to do so
  void setBinAveraging(bool v) { _binAveraging = v; }
  //! @}

  //!
  //! @brief Fourier decomposition of a function (sampled, see the class description), and its quality (result()).
  //! @param f function of rank 1 (of an angle, in radians) @return decomposition of order nMax() in form form()
  //! @throws MathException if f is not of rank 1, the number of samples is not above 2 nMax + 1, the fit fails, or
  //! (form 2) the average of f is 0
  //!
  FourierDecomposition calculate(const Function & f);
  //!
  //! @brief Fourier decomposition of data points (e.g. a histogram), and its quality (result()).
  //! @param data points of rank 1 (x: an angle, in radians), with uncertainties
  //! @return decomposition of order nMax() in form form()
  //! @throws MathException if the data are not of rank 1, have no more than 2 nMax + 1 points, are not equally spaced
  //! over one period (kAverage), a point has no uncertainty, the fit fails, or (form 2) the average k is 0
  //!
  FourierDecomposition calculate(const DataPoints & data);
  //!
  //! @brief Fourier decomposition of a function, in one call. @param f function of rank 1 @param nMax highest
  //! harmonic @param form form @param method method @param result quality of the decomposition (if not null)
  //! @param error uncertainty of each sample @return decomposition @throws MathException as calculate()
  //!
  static FourierDecomposition decompose(const Function & f, unsigned int nMax, Form form=FourierDecomposition::kCosineSine,
                                        Method method=kAverage, FitResult * result=nullptr, double error=1.0);
  //!
  //! @brief Weighted linear least squares of form 1: the k, c_n, s_n of least sum_i w_i (y_i - f(x_i))^2.
  //! @param x abscissas @param y values @param w weights (1/sigma_i^2) @param nMax highest harmonic
  //! @param values k, c1, s1, ..., c_nMax, s_nMax @param covariance (if not null) their covariance, the inverse of
  //! the curvature matrix @return false if there are too few points or the system is singular
  //!
  static bool leastSquares(const std::vector<double> & x, const std::vector<double> & y, const std::vector<double> & w,
                           unsigned int nMax, std::vector<double> & values, Matrix * covariance=nullptr);
  //!
  //! @brief Coefficients of form 1 changed into those of form 2 (v_n = sqrt(c_n^2 + s_n^2)/(2 k),
  //! psi_n = atan2(s_n, c_n)/n), with their covariance propagated. @param values k, c1, s1, ... changed into k, v1,
  //! psi1, ... @param covariance (if not null) covariance changed accordingly (the phase of a harmonic that vanishes
  //! gets an infinite variance) @return false if k is 0 (nothing changed)
  //!
  static bool toFlow(std::vector<double> & values, Matrix * covariance=nullptr);

  //! @name Quality of the last decomposition
  //! @{
  //! @brief Quality of the last decomposition, as that of a fit (chi square, degrees of freedom, chi square per
  //! degree of freedom, p-value, coefficients with their uncertainties and covariance). @return result
  const FitResult & result() const { return _result; }
  //! @brief Chi square. @return chi square
  double chiSquare() const { return _result.chiSquare; }
  //! @brief Degrees of freedom: N - (2 nMax + 1). @return degrees of freedom
  int nDegreesOfFreedom() const { return _result.nDegreesOfFreedom; }
  //! @brief Chi square per degree of freedom. @return chi square / degrees of freedom
  double chiSquarePerDegreeOfFreedom() const { return _result.chiSquarePerDegreeOfFreedom; }
  //! @brief Probability of a larger chi square. @return p-value
  double pValue() const { return _result.pValue; }
  //! @brief Largest |y - decomposition| at the points. @return difference
  double maxDeviation() const { return _maxDeviation; }
  //! @brief Root mean square of y - decomposition at the points. @return difference
  double rmsDeviation() const { return _rmsDeviation; }
  //! @brief Samples of the function of the last decomposition of a function. @return points (x, f(x), sigma)
  const DataPointsVector & samples() const { return _samples; }
  //! @}

  protected:

  //!
  //! @brief The decomposition of points (x_i, y_i, sigma_i). @param x abscissas @param y values (double precision)
  //! @param sigma uncertainties @param data the same points, for the chi square and the fit @param equallySpaced
  //! whether the points are known to be the centers of equal intervals of one period (samples of a function)
  //! @return decomposition
  //!
  FourierDecomposition decompose(const std::vector<double> & x, const std::vector<double> & y,
                                 const std::vector<double> & sigma, const DataPoints & data, bool equallySpaced);

  unsigned int _nMax;          //!< highest harmonic
  Form _form;                  //!< form of the decompositions
  Method _method;              //!< method
  unsigned int _nSamples;      //!< samples of a function over the period (0: automatic)
  double _error;               //!< uncertainty of each sample of a function
  double _low;                 //!< lower end of the period of a function
  bool _binAveraging;          //!< the points are averages over bins
  FitResult _result;           //!< quality of the last decomposition
  DataPointsVector _samples;   //!< samples of the last decomposition of a function
  double _maxDeviation;        //!< largest difference at the points
  double _rmsDeviation;        //!< rms difference at the points
  };

  } // namespace MATH
} // namespace CAP

#endif // CAP_MATH_CalculateFourierDecomposition
