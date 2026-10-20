#include "ChiSquare.hpp"
#include <algorithm>
#include <cmath>
#include <string>
#include "MathExceptions.hpp"

ClassImp(CAP::MATH::ChiSquare);

namespace CAP
{
  namespace MATH
  {

  ChiSquare::ChiSquare()
  :
  _function(nullptr),
  _data(nullptr),
  _errorMode(Statistical),
  _useXErrors(false)
  {}

  ChiSquare::ChiSquare(const ParameterizedFunction & function,
                           const DataPoints & data,
                           Chi2ErrorMode errorMode,
                           bool useXErrors)
  :
  _function(&function),
  _data(&data),
  _errorMode(errorMode),
  _useXErrors(useXErrors)
  {
  checkRanks(function,data,__FUNCTION__);
  }

  ChiSquare::ChiSquare(const ChiSquare & source)
  :
  _function(source._function),
  _data(source._data),
  _errorMode(source._errorMode),
  _useXErrors(source._useXErrors)
  {}

  ChiSquare & ChiSquare::operator=(const ChiSquare & rhs)
  {
  if (this != &rhs)
    {
    _function   = rhs._function;
    _data       = rhs._data;
    _errorMode  = rhs._errorMode;
    _useXErrors = rhs._useXErrors;
    }
  return *this;
  }

  void ChiSquare::checkReady(const char * source) const
  {
  if (!_function) throw NullPointerException("Function is not set",source);
  if (!_data)     throw NullPointerException("Data is not set",source);
  checkRanks(*_function,*_data,source);
  }

  void ChiSquare::checkRanks(const ParameterizedFunction & function, const DataPoints & data, const char * source)
  {
  if (function.rank()!=data.rank())
    {
    String message = "Rank of the function (";
    message += std::to_string(function.rank()).c_str();
    message += ") differs from the rank of the data (";
    message += std::to_string(data.rank()).c_str();
    message += ")";
    throw MathException(message,source);
    }
  }

  void ChiSquare::setFunction(const ParameterizedFunction & function)
  {
  if (_data) checkRanks(function,*_data,__FUNCTION__);
  _function = &function;
  }

  void ChiSquare::setData(const DataPoints & data)
  {
  if (_function) checkRanks(*_function,data,__FUNCTION__);
  _data = &data;
  }

  double ChiSquare::variance(const DataPoint2D & point) const
  {
  double sigmaY = point.yStatError();
  double v = sigmaY*sigmaY;
  if (_errorMode==Total)
    {
    double sigmaSys = point.ySysError();
    v += sigmaSys*sigmaSys;
    }
  if (_useXErrors)
    {
    double slope  = _function->derivativeAt(point.x());
    double sigmaX = point.xError();
    v += slope*slope*sigmaX*sigmaX - 2.0*slope*point.xyCovariance();
    }
  return v;
  }

  double ChiSquare::variance(unsigned int index, const std::vector<double> & x) const
  {
  double sigma = _data->statErrorAt(index);
  double v = sigma*sigma;
  if (_errorMode==Total)
    {
    double sigmaSys = _data->sysErrorAt(index);
    v += sigmaSys*sigmaSys;
    }
  if (_useXErrors)
    {
    // effective variance: sum over axes of (df/dx_k sigma_k)^2, coordinates assumed uncorrelated
    std::vector<double> sigmaX = _data->coordinateErrorsAt(index);
    bool any = false;
    for (double s : sigmaX) if (s!=0.0) { any = true; break; }
    if (any)
      {
      VectorND gradient = _function->gradientAt(x);
      for (unsigned int k=0; k<x.size(); k++) v += gradient[k]*gradient[k]*sigmaX[k]*sigmaX[k];
      }
    }
  return v;
  }

  double ChiSquare::chiSquareAt(unsigned int index) const
  {
  double pull = pullAt(index);
  return pull*pull;
  }

  double ChiSquare::pullAt(unsigned int index) const
  {
  checkReady(__FUNCTION__);
  if (index >= _data->size()) throw MathException("Point index out of range",__FUNCTION__);
  if (_data->rank()==1)
    {
    DataPoint2D point = _data->point(index);
    double v = variance(point);
    if (!(v>0.0)) throw MathException("Uncertainty of a data point is zero or negative",__FUNCTION__);
    return (point.y() - _function->evaluateAt(point.x()))/std::sqrt(v);
    }
  std::vector<double> x = _data->coordinatesAt(index);
  double v = variance(index,x);
  if (!(v>0.0)) throw MathException("Uncertainty of a data point is zero or negative",__FUNCTION__);
  return (_data->valueAt(index) - _function->evaluateAt(x))/std::sqrt(v);
  }

  double ChiSquare::chiSquare() const
  {
  checkReady(__FUNCTION__);
  double sum = 0.0;
  for (unsigned int i=0; i<_data->size(); i++) sum += chiSquareAt(i);
  return sum;
  }

  unsigned int ChiSquare::nFreeParameters() const
  {
  if (!_function) return 0;
  unsigned int n = 0;
  for (const auto & parameter : _function->parameters())
    if (parameter.type()!=Parameter::Fixed) n++;
  return n;
  }

  double ChiSquare::chiSquarePerDegreeOfFreedom() const
  {
  int ndf = nDegreesOfFreedom();
  if (ndf<=0) throw MathException("Number of degrees of freedom is not positive",__FUNCTION__);
  return chiSquare()/double(ndf);
  }

  double ChiSquare::regularizedGammaQ(double a, double x)
  {
  if (!(a > 0) || x < 0) throw MathException("Invalid arguments of the incomplete gamma function",__FUNCTION__);
  if (x == 0) return 1.0;
  const double logPrefactor = -x + a*std::log(x) - std::lgamma(a);
  if (x < a + 1)
    {
    // series of P(a,x) = x^a e^-x / Gamma(a) sum x^n / (a (a+1) ... (a+n)); Q = 1 - P
    double term = 1.0/a, sum = term;
    for (int n=1; n<1000; n++)
      {
      term *= x/(a + n);
      sum += term;
      if (std::fabs(term) < std::fabs(sum)*1E-16) break;
      }
    return std::max(0.0, std::min(1.0, 1.0 - sum*std::exp(logPrefactor)));
    }
  // continued fraction of Q(a,x) (modified Lentz)
  const double tiny = 1E-300;
  double b = x + 1 - a, c = 1/tiny, d = 1/b, h = d;
  for (int n=1; n<1000; n++)
    {
    double an = -n*(n - a);
    b += 2;
    d = an*d + b; if (std::fabs(d) < tiny) d = tiny;
    c = b + an/c; if (std::fabs(c) < tiny) c = tiny;
    d = 1/d;
    double delta = d*c;
    h *= delta;
    if (std::fabs(delta - 1) < 1E-16) break;
    }
  return std::max(0.0, std::min(1.0, std::exp(logPrefactor)*h));
  }

  double ChiSquare::probability(double chiSquare, int ndf)
  {
  if (ndf <= 0) throw MathException("The number of degrees of freedom must be positive",__FUNCTION__);
  if (chiSquare < 0) throw MathException("A chi square cannot be negative",__FUNCTION__);
  return regularizedGammaQ(0.5*ndf, 0.5*chiSquare);
  }

  } // namespace MATH
} // namespace CAP
