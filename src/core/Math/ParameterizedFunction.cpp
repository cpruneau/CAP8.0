#include "ParameterizedFunction.hpp"

#include <cmath>
#include <vector>
#include <algorithm>
#include <functional>
#include "MathExceptions.hpp"

ClassImp(CAP::MATH::ParameterizedFunction);

namespace CAP
{
  namespace MATH
  {

  ParameterizedFunction::ParameterizedFunction(unsigned int rank)
  :
  Function(rank),
  _parameters()
  {}

  ParameterizedFunction::ParameterizedFunction(const std::vector<Parameter> & parameters, unsigned int rank)
  :
  Function(rank),
  _parameters(parameters)
  {}

  ParameterizedFunction::ParameterizedFunction(const ParameterizedFunction & source)
  :
  Function(source),
  _parameters(source._parameters)
  {}

  ParameterizedFunction::~ParameterizedFunction()
  {}

  ParameterizedFunction & ParameterizedFunction::operator=(const ParameterizedFunction & rhs)
  {
  if (this != &rhs)
    {
    Function::operator=(rhs);
    _parameters = rhs._parameters;
    }
  return *this;
  }

  void ParameterizedFunction::addParameter(const Parameter & parameter)
  {
  _parameters.push_back(parameter);
  }

  const Parameter & ParameterizedFunction::parameter(unsigned int index) const
  {
  if (index >= _parameters.size())
    throw MathException("Parameter index out of range",__FUNCTION__);
  return _parameters[index];
  }

  ParameterizedFunction * ParameterizedFunction::clone() const
  {
  throw MathException(String("clone() is not implemented by the class ") + className(), __FUNCTION__);
  }

  void ParameterizedFunction::setParameterValue(unsigned int index, double value)
  {
  if (index >= _parameters.size()) throw MathException("Parameter index out of range",__FUNCTION__);
  _parameters[index].setValue(value);
  }

  void ParameterizedFunction::setParameterValues(const std::vector<double> & values)
  {
  if (values.size() != _parameters.size()) throw MathException("One value per parameter is needed",__FUNCTION__);
  for (unsigned int k=0; k<values.size(); k++) _parameters[k].setValue(values[k]);
  }

  std::vector<double> ParameterizedFunction::parameterValues() const
  {
  std::vector<double> values;
  for (const auto & p : _parameters) values.push_back(p.value());
  return values;
  }

  std::vector<unsigned int> ParameterizedFunction::freeParameters() const
  {
  std::vector<unsigned int> indices;
  for (unsigned int k=0; k<_parameters.size(); k++) if (_parameters[k].type() != Parameter::Fixed) indices.push_back(k);
  return indices;
  }

  void ParameterizedFunction::fixParameter(unsigned int index, bool fixed)
  {
  if (index >= _parameters.size()) throw MathException("Parameter index out of range",__FUNCTION__);
  Parameter & p = _parameters[index];
  if (fixed) { p.setType(Parameter::Fixed); return; }
  bool low = p.minimum() > Parameter::defaultMinimum, high = p.maximum() < Parameter::defaultMaximum;
  p.setType(low ? (high ? Parameter::LowHigh : Parameter::Low) : (high ? Parameter::High : Parameter::Free));
  }

  void ParameterizedFunction::setParameterLimits(unsigned int index, double minimum, double maximum)
  {
  if (index >= _parameters.size()) throw MathException("Parameter index out of range",__FUNCTION__);
  if (minimum > maximum) throw MathException("The minimum is above the maximum",__FUNCTION__);
  Parameter & p = _parameters[index];
  bool fixed = p.type() == Parameter::Fixed;
  p.setType(Parameter::Free);                         // the value is moved within the new bounds first
  double v = std::min(std::max(p.value(), minimum), maximum);
  p.setValue(v);
  p.setMinimum(minimum);
  p.setMaximum(maximum);
  if (fixed) p.setType(Parameter::Fixed);
  }

  namespace
  {
  // Function currently being evaluated by the vector version of evaluateAt() (per thread). The vector version
  // forwards to the 1, 2 or 3 argument versions, and these forward back to the vector version: this flag
  // detects the loop when a derived class implements neither, and turns it into an exception.
  thread_local const ParameterizedFunction * evaluatingVector = nullptr;

  struct EvaluatingVectorGuard
  {
  const ParameterizedFunction * saved;
  EvaluatingVectorGuard(const ParameterizedFunction * f) : saved(evaluatingVector) { evaluatingVector = f; }
  ~EvaluatingVectorGuard() { evaluatingVector = saved; }
  };

  const char * notImplementedMessage = "evaluateAt() is not implemented by the derived class";
  }

  double ParameterizedFunction::evaluateAt(double x) const
  {
  checkRank(1,__FUNCTION__);
  if (evaluatingVector==this) throw MathException(notImplementedMessage,__FUNCTION__);
  return evaluateAt(std::vector<double>{x});
  }

  double ParameterizedFunction::evaluateAt(double x1, double x2) const
  {
  checkRank(2,__FUNCTION__);
  if (evaluatingVector==this) throw MathException(notImplementedMessage,__FUNCTION__);
  return evaluateAt(std::vector<double>{x1,x2});
  }

  double ParameterizedFunction::evaluateAt(double x1, double x2, double x3) const
  {
  checkRank(3,__FUNCTION__);
  if (evaluatingVector==this) throw MathException(notImplementedMessage,__FUNCTION__);
  return evaluateAt(std::vector<double>{x1,x2,x3});
  }

  double ParameterizedFunction::evaluateAt(const std::vector<double> & x) const
  {
  checkRank(x.size(),__FUNCTION__);
  EvaluatingVectorGuard guard(this);
  switch (_rank)
    {
    case 1: return evaluateAt(x[0]);
    case 2: return evaluateAt(x[0],x[1]);
    case 3: return evaluateAt(x[0],x[1],x[2]);
    default: throw MathException(notImplementedMessage,__FUNCTION__);
    }
  }

  // Five-point central difference; error of order h^4 f^(5)(x).
  double ParameterizedFunction::derivativeAt(double x) const
  {
  checkRank(1,__FUNCTION__);
  double h = 1.0E-3*std::max(1.0,std::fabs(x));
  return (evaluateAt(x-2.0*h) - 8.0*evaluateAt(x-h) + 8.0*evaluateAt(x+h) - evaluateAt(x+2.0*h))/(12.0*h);
  }

  namespace
  {
  // Recursive step of adaptive Simpson integration on [a,b], with g evaluated at a, m=(a+b)/2, b.
  double adaptiveSimpson(const std::function<double(double)> & g, double a, double b,
                         double fa, double fm, double fb, double whole,
                         double tolerance, int depth)
  {
  double m   = 0.5*(a+b);
  double lm  = 0.5*(a+m),  rm  = 0.5*(m+b);
  double flm = g(lm), frm = g(rm);
  double left  = (m-a)*(fa + 4.0*flm + fm)/6.0;
  double right = (b-m)*(fm + 4.0*frm + fb)/6.0;
  double delta = left + right - whole;
  if (depth<=0 || std::fabs(delta) <= 15.0*tolerance)
    return left + right + delta/15.0;   // Richardson extrapolation
  return adaptiveSimpson(g,a,m,fa,flm,fm,left, 0.5*tolerance,depth-1)
       + adaptiveSimpson(g,m,b,fm,frm,fb,right,0.5*tolerance,depth-1);
  }

  // Integral of g from low to high (finite bounds), by adaptive Simpson integration started on 8 panels so that
  // narrow features are not missed by the first estimate. The tolerance is relative to the integral of |g|,
  // so that integrals close to zero (e.g., odd functions over symmetric ranges) converge quickly.
  double integrate(const std::function<double(double)> & g, double low, double high,
                   double relativeTolerance, int maxDepth, const char * source)
  {
  if (!std::isfinite(low) || !std::isfinite(high))
    throw MathException("Numerical integration requires finite bounds",source);
  if (low==high) return 0.0;
  if (low>high)  return -integrate(g,high,low,relativeTolerance,maxDepth,source);
  const int nPanels = 8;
  double width = (high-low)/nPanels;
  double coarseAbs = 0.0;
  std::vector<double> fs(2*nPanels+1);
  for (int k=0; k<=2*nPanels; k++) fs[k] = g(low + 0.5*k*width);
  for (int k=0; k<nPanels; k++) coarseAbs += width*(std::fabs(fs[2*k]) + 4.0*std::fabs(fs[2*k+1]) + std::fabs(fs[2*k+2]))/6.0;
  double tolerance = relativeTolerance*std::max(coarseAbs,1.0E-300);
  double sum = 0.0;
  for (int k=0; k<nPanels; k++)
    {
    double a = low + k*width, b = a + width;
    double whole = width*(fs[2*k] + 4.0*fs[2*k+1] + fs[2*k+2])/6.0;
    sum += adaptiveSimpson(g,a,b,fs[2*k],fs[2*k+1],fs[2*k+2],whole,tolerance/nPanels,maxDepth);
    }
  return sum;
  }

  // Iterated integral over axes axis ... rank-1 of f, with the variables of the lower axes already set in x.
  double integrateAxes(const ParameterizedFunction & f, std::vector<double> & x, unsigned int axis,
                       const std::vector<double> & low, const std::vector<double> & high, const char * source)
  {
  const double relativeTolerance = 1.0E-8;
  const int    maxDepth          = 20;
  bool last = (axis+1==x.size());
  auto g = [&](double t)
    {
    x[axis] = t;
    return last ? f.evaluateAt(x) : integrateAxes(f,x,axis+1,low,high,source);
    };
  return integrate(g,low[axis],high[axis],relativeTolerance,maxDepth,source);
  }
  }

  namespace
  {
  // x f(x), used to compute the first moment with the numerical integrator
  class XTimesFunction : public ParameterizedFunction
  {
  public:
  XTimesFunction(const ParameterizedFunction & f) : _f(f) {}
  double evaluateAt(double x) const override { return x*_f.evaluateAt(x); }
  using ParameterizedFunction::evaluateAt;
  private:
  const ParameterizedFunction & _f;
  };

  // x^n f(x), used to compute moments with the numerical integrator
  class XPowerFunction : public ParameterizedFunction
  {
  public:
  XPowerFunction(const ParameterizedFunction & f, int power) : _f(f), _power(power) {}
  double evaluateAt(double x) const override { return std::pow(x,_power)*_f.evaluateAt(x); }
  using ParameterizedFunction::evaluateAt;
  private:
  const ParameterizedFunction & _f;
  int _power;
  };
  }

  void ParameterizedFunction::checkPowerRange(double low, double high, int power, const char * source)
  {
  if (power<0 && std::min(low,high)<=0.0 && std::max(low,high)>=0.0)
    throw MathException("Negative power of x is singular at x=0, which is in the range",source);
  }

  double ParameterizedFunction::expectationValue(double low, double high, int power) const
  {
  checkRank(1,__FUNCTION__);
  if (power==1) return expectationValue(low,high);
  checkPowerRange(low,high,power,__FUNCTION__);
  double norm = ParameterizedFunction::integralInRange(low,high);
  if (norm==0.0) throw MathException("Integral of the function over the range is zero",__FUNCTION__);
  if (power==0) return 1.0;
  XPowerFunction xf(*this,power);
  return xf.ParameterizedFunction::integralInRange(low,high)/norm;
  }

  double ParameterizedFunction::expectationValue(double low, double high) const
  {
  checkRank(1,__FUNCTION__);
  double norm = ParameterizedFunction::integralInRange(low,high);
  if (norm==0.0) throw MathException("Integral of the function over the range is zero",__FUNCTION__);
  XTimesFunction xf(*this);
  return xf.ParameterizedFunction::integralInRange(low,high)/norm;
  }

  double ParameterizedFunction::integralInRange(double low, double high) const
  {
  checkRank(1,__FUNCTION__);
  auto g = [this](double x) { return evaluateAt(x); };
  return integrate(g,low,high,1.0E-10,40,__FUNCTION__);
  }

  double ParameterizedFunction::integralInRange(double x1Low, double x1High, double x2Low, double x2High) const
  {
  checkRank(2,__FUNCTION__);
  return integralInRange(std::vector<double>{x1Low,x2Low},std::vector<double>{x1High,x2High});
  }

  double ParameterizedFunction::integralInRange(double x1Low, double x1High, double x2Low, double x2High, double x3Low, double x3High) const
  {
  checkRank(3,__FUNCTION__);
  return integralInRange(std::vector<double>{x1Low,x2Low,x3Low},std::vector<double>{x1High,x2High,x3High});
  }

  double ParameterizedFunction::integralInRange(const std::vector<double> & low, const std::vector<double> & high) const
  {
  checkRank(low.size(),__FUNCTION__);
  checkRank(high.size(),__FUNCTION__);
  if (_rank==1) return integralInRange(low[0],high[0]);
  std::vector<double> x(_rank);
  return integrateAxes(*this,x,0,low,high,__FUNCTION__);
  }

  VectorND ParameterizedFunction::gradientAt(const std::vector<double> & x) const
  {
  checkRank(x.size(),__FUNCTION__);
  if (_rank==1) return VectorND(1,derivativeAt(x[0]));
  VectorND gradient(_rank);
  std::vector<double> y(x);
  for (unsigned int i=0; i<_rank; i++)
    {
    // five-point central difference along axis i; error of order h^4
    double h = 1.0E-3*std::max(1.0,std::fabs(x[i]));
    y[i] = x[i]-2.0*h; double f1 = evaluateAt(y);
    y[i] = x[i]-h;     double f2 = evaluateAt(y);
    y[i] = x[i]+h;     double f3 = evaluateAt(y);
    y[i] = x[i]+2.0*h; double f4 = evaluateAt(y);
    y[i] = x[i];
    gradient[i] = (f1 - 8.0*f2 + 8.0*f3 - f4)/(12.0*h);
    }
  return gradient;
  }

  VectorND ParameterizedFunction::gradientAt(double x) const
  {
  checkRank(1,__FUNCTION__);
  return gradientAt(std::vector<double>{x});
  }

  VectorND ParameterizedFunction::gradientAt(double x1, double x2) const
  {
  checkRank(2,__FUNCTION__);
  return gradientAt(std::vector<double>{x1,x2});
  }

  VectorND ParameterizedFunction::gradientAt(double x1, double x2, double x3) const
  {
  checkRank(3,__FUNCTION__);
  return gradientAt(std::vector<double>{x1,x2,x3});
  }

  } // namespace MATH
} // namespace CAP
