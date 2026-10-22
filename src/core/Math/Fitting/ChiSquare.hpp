#ifndef CAP_MATH_ChiSquare
#define CAP_MATH_ChiSquare
#include "Aliases.hpp"
#include "ParameterizedFunction.hpp"
#include "DataPoints.hpp"

namespace CAP
{
  namespace MATH
  {

  //!
  //! @brief Chi square of a set of data points (DataPoints) with respect to a function of one variable (ParameterizedFunction of rank 1).
  //!
  //! chi2 = sum_i (y_i - f(x_i))^2 / sigma_i^2, where the uncertainty sigma_i of point i is chosen with setErrorMode():
  //!  - Statistical : sigma^2 = yStatError^2 (default)
  //!  - Total       : sigma^2 = yStatError^2 + ySysError^2
  //!
  //! If x uncertainties are enabled with setUseXErrors(true), the effective variance method is used:
  //! sigma^2 += f'(x)^2 xError^2 - 2 f'(x) cov(x,y), with f'(x) from ParameterizedFunction::derivativeAt().
  //! For data of higher rank: sigma^2 += sum_k (df/dx_k)^2 sigma_k^2, with the gradient from ParameterizedFunction::gradientAt()
  //! and the coordinate uncertainties from DataPoints::coordinateErrorsAt() (assumed uncorrelated).
  //!
  //! The number of degrees of freedom is the number of points minus the number of function
  //! parameters that are not Fixed.
  //!
  //! The function and the data must have the same rank: this is checked by the constructor, setFunction() and setData()
  //! (MathException otherwise), and again by chiSquare(), in case the rank of the function was changed in between.
  //! Data of any rank are supported: for rank 1 the points are read with DataPoints::point(); for higher ranks with
  //! DataPoints::coordinatesAt(), valueAt(), statErrorAt(), sysErrorAt() and coordinateErrorsAt(), and the function
  //! is evaluated with ParameterizedFunction::evaluateAt(const std::vector<double> &).
  //!
  //! The function and the data are not owned: they must outlive this object and are read each time
  //! chiSquare() is called, so it always reflects their current values (e.g. during a fit).
  //!
  //! Example:
  //! @code
  //!   CAP::MATH::Linear f(1.0, 2.0);
  //!   CAP::MATH::DataPointsVector data;
  //!   data.addPoints({0.0, 1.0, 2.0}, {1.1, 2.9, 5.2}, {0.1, 0.1, 0.2});
  //!   CAP::MATH::ChiSquare chi2(f, data);
  //!   double value = chi2.chiSquare();
  //!   double perNdf = chi2.chiSquarePerDegreeOfFreedom();
  //! @endcode
  //!
  class ChiSquare
  {
  public:

  //! @brief Choice of the y uncertainty used for each point.
  enum Chi2ErrorMode
  {
    Statistical,  //!< yStatError only
    Total         //!< yStatError and ySysError added in quadrature
  };

  //! @brief Default constructor: no function and no data (set them with setFunction() and setData()).
  ChiSquare();
  //!
  //! @brief Detailed constructor.
  //! @param function function compared to the data (not owned)
  //! @param data data points (not owned)
  //! @param errorMode y uncertainty used for each point
  //! @param useXErrors whether to include the x uncertainties (effective variance method)
  //! @throws MathException if the function and the data do not have the same rank
  //!
  ChiSquare(const ParameterizedFunction & function,
              const DataPoints & data,
              Chi2ErrorMode errorMode = Statistical,
              bool useXErrors = false);
  //! @brief Copy constructor: the copy refers to the same function and data. @param source object to copy
  ChiSquare(const ChiSquare & source);
  //! @brief Assignment operator: refers to the same function and data as rhs. @param rhs object to copy @return reference to this object
  ChiSquare & operator=(const ChiSquare & rhs);
  //! @brief Destructor.
  virtual ~ChiSquare() {}

  //! @brief Set the function (not owned). @param function function compared to the data
  //! @throws MathException if data is already set and its rank differs from that of the function
  void setFunction(const ParameterizedFunction & function);
  //! @brief Set the data (not owned). @param data data points
  //! @throws MathException if a function is already set and its rank differs from that of the data
  void setData(const DataPoints & data);
  //! @brief Set the y uncertainty used for each point. @param errorMode Statistical or Total
  void setErrorMode(Chi2ErrorMode errorMode) { _errorMode = errorMode; }
  //! @brief Include (or not) the x uncertainties, with the effective variance method. @param useXErrors true to include them
  void setUseXErrors(bool useXErrors) { _useXErrors = useXErrors; }
  //! @brief Compare the data of a histogram to the average of the function over each bin (see binAverage()) rather
  //! than to its value at the center of the bin. @param v average
  void setBinAveraging(bool v) { _binAveraging = v; }
  //! @brief Whether the function is averaged over the bins of a histogram. @return true if so
  bool binAveraging() const { return _binAveraging; }

  //! @brief Function compared to the data. @return pointer to the function (nullptr if not set)
  const ParameterizedFunction * function() const { return _function; }
  //! @brief Data points. @return pointer to the data (nullptr if not set)
  const DataPoints * data() const { return _data; }
  //! @brief Y uncertainty used for each point. @return error mode
  Chi2ErrorMode errorMode() const { return _errorMode; }
  //! @brief Whether the x uncertainties are included. @return true if included
  bool useXErrors() const { return _useXErrors; }

  //!
  //! @brief Chi square of the data with respect to the function.
  //! @return sum over points of (y - f(x))^2 / sigma^2
  //! @throws NullPointerException if the function or the data is not set; MathException if their ranks differ
  //! @throws MathException if the uncertainty of a point is zero (or negative, with x uncertainties)
  //!
  double chiSquare() const;

  //!
  //! @brief Contribution of one point to the chi square.
  //! @param index index of the point
  //! @return (y - f(x))^2 / sigma^2
  //! @throws NullPointerException if the function or the data is not set; MathException if their ranks differ
  //! @throws MathException if index is out of range or the uncertainty of the point is zero
  //!
  double chiSquareAt(unsigned int index) const;

  //!
  //! @brief Pull (normalized residual) of one point.
  //! @param index index of the point
  //! @return (y - f(x)) / sigma
  //! @throws NullPointerException if the function or the data is not set; MathException if their ranks differ
  //! @throws MathException if index is out of range or the uncertainty of the point is zero
  //!
  double pullAt(unsigned int index) const;

  //! @brief Number of data points. @return number of points (0 if no data is set)
  unsigned int nPoints() const { return _data ? _data->size() : 0; }
  //! @brief Number of function parameters that are not Fixed. @return number of free parameters (0 if no function is set)
  unsigned int nFreeParameters() const;
  //! @brief Number of degrees of freedom: nPoints() - nFreeParameters(). @return degrees of freedom (may be negative)
  int nDegreesOfFreedom() const { return int(nPoints()) - int(nFreeParameters()); }

  //!
  //! @brief Chi square divided by the number of degrees of freedom.
  //! @return chiSquare()/nDegreesOfFreedom()
  //! @throws MathException if the number of degrees of freedom is not positive
  //!
  double chiSquarePerDegreeOfFreedom() const;

  //!
  //! @brief Probability that a chi square of ndf degrees of freedom exceeds the given value (the p-value of a fit):
  //! the regularized upper incomplete gamma function Q(ndf/2, chiSquare/2), computed by its series or its continued
  //! fraction. @param chiSquare chi square (>= 0) @param ndf degrees of freedom (> 0)
  //! @return probability in [0,1] @throws MathException if ndf is not positive or chiSquare is negative
  //!
  static double probability(double chiSquare, int ndf);
  //! @brief Regularized upper incomplete gamma function Q(a,x) = Gamma(a,x)/Gamma(a). @param a a (> 0) @param x x
  //! (>= 0) @return Q(a,x) @throws MathException if a <= 0 or x < 0
  static double regularizedGammaQ(double a, double x);

  protected:

  //! @brief Throw NullPointerException if the function or the data is not set, MathException if their ranks differ. @param source name of the calling function
  void checkReady(const char * source) const;
  //!
  //! @brief Throw if the function and the data do not have the same rank.
  //! @param function function @param data data points @param source name of the calling function
  //! @throws MathException if function.rank() != data.rank()
  //!
  static void checkRanks(const ParameterizedFunction & function, const DataPoints & data, const char * source);
  //! @brief Variance sigma^2 used for a point of rank 1, according to the error mode and x uncertainty option. @param point data point @return variance
  double variance(const DataPoint2D & point) const;
  //! @brief Variance sigma^2 used for point index of any rank, according to the error mode and coordinate uncertainty option.
  //! @param index index of the point @param x coordinates of the point @return variance
  double variance(unsigned int index, const std::vector<double> & x) const;

  const ParameterizedFunction * _function;  //!<! function compared to the data (not owned, not streamed)
  const DataPoints * _data;            //!<! data points (not owned, not streamed)
  Chi2ErrorMode _errorMode;                   //!< y uncertainty used for each point
  bool _useXErrors;                       //!< whether the x uncertainties are included
  bool _binAveraging;                     //!< the function is averaged over the bins of a histogram

  ClassDef(ChiSquare,2)
  };

  } // namespace MATH
} // namespace CAP

#endif
