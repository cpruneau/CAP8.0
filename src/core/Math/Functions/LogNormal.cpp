#include "LogNormal.hpp"
#include "MathConstants.hpp"
#include "MathExceptions.hpp"


namespace CAP
{
  namespace MATH
  {

  LogNormal::LogNormal(double amplitude, double mean, double width)
  :
  ParameterizedFunction({Parameter("amplitude",amplitude),Parameter("mean",mean),Parameter("width",1.0)})
  {
  setWidth(width);
  }

  double LogNormal::evaluateAt(double x) const
  {
  if (x<=0.0) return 0.0;
  double z = (std::log(x) - mean())/width();
  return amplitude()*std::exp(-0.5*z*z)/(x*width()*sqrtTwoPi());
  }

  // d/dx ln f = -1/x - (ln x - mu)/(sigma^2 x)
  double LogNormal::derivativeAt(double x) const
  {
  if (x<=0.0) return 0.0;
  double z = (std::log(x) - mean())/width();
  return -evaluateAt(x)*(1.0 + z/width())/x;
  }

  namespace
  {
  // erf(b) - erf(a), using erfc in the tails to avoid cancellation
  double erfDifference(double a, double b)
  {
  if (a>=0.0 && b>=0.0) return std::erfc(a) - std::erfc(b);
  if (a<=0.0 && b<=0.0) return std::erfc(-b) - std::erfc(-a);
  return std::erf(b) - std::erf(a);
  }

  // (ln x - mu)/(sigma sqrt 2), with -infinity for x <= 0
  double logArgument(double x, double mean, double width)
  {
  if (x<=0.0) return -HUGE_VAL;
  return (std::log(x) - mean)/(width*std::sqrt(2.0));
  }
  }

  double LogNormal::integralInRange(double low, double high) const
  {
  if (low==high) return 0.0;
  return 0.5*amplitude()*erfDifference(logArgument(low,mean(),width()),logArgument(high,mean(),width()));
  }

  double LogNormal::expectationValue(double low, double high) const
  {
  double norm = erfDifference(logArgument(low,mean(),width()),logArgument(high,mean(),width()));
  if (norm==0.0 || amplitude()==0.0) throw MathException("Integral of the function over the range is zero",__FUNCTION__);
  double shiftedMean = mean() + width()*width();
  double moment = erfDifference(logArgument(low,shiftedMean,width()),logArgument(high,shiftedMean,width()));
  return std::exp(mean() + 0.5*width()*width())*moment/norm;
  }

  double LogNormal::expectationValue(double low, double high, int power) const
  {
  double norm = erfDifference(logArgument(low,mean(),width()),logArgument(high,mean(),width()));
  if (norm==0.0 || amplitude()==0.0) throw MathException("Integral of the function over the range is zero",__FUNCTION__);
  if (power==0) return 1.0;
  double n = power;
  double shiftedMean = mean() + n*width()*width();
  double moment = erfDifference(logArgument(low,shiftedMean,width()),logArgument(high,shiftedMean,width()));
  return std::exp(n*mean() + 0.5*n*n*width()*width())*moment/norm;
  }

  void LogNormal::setWidth(double width)
  {
  if (!(width>0.0)) throw MathException("Width (standard deviation of ln x) must be > 0",__FUNCTION__);
  setParameterValue(2,width);
  }

  void LogNormal::setParameters(double amplitude, double mean, double width)
  {
  setWidth(width);          // checked first, so nothing changes if it throws
  setParameterValue(0,amplitude);
  setParameterValue(1,mean);
  }

  } // namespace MATH
} // namespace CAP
