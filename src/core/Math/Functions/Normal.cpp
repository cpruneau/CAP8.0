#include "Normal.hpp"
#include "MathConstants.hpp"
#include "MathExceptions.hpp"

#include <algorithm>
#include <vector>


namespace CAP
{
  namespace MATH
  {

  Normal::Normal(double amplitude, double mean, double width)
  :
  ParameterizedFunction({Parameter("amplitude",amplitude),Parameter("mean",mean),Parameter("width",1.0)})
  {
  setWidth(width);
  }

  double Normal::integral() const
  {
  return amplitude()*width()*sqrtTwoPi();
  }

  double Normal::fwhm() const
  {
  return 2.0*std::sqrt(2.0*std::log(2.0))*width();
  }

  void Normal::setWidth(double width)
  {
  if (!(width>0.0)) throw MathException("Width (standard deviation) must be > 0",__FUNCTION__);
  setParameterValue(2,width);
  }

  void Normal::setParameters(double amplitude, double mean, double width)
  {
  setWidth(width);          // checked first, so nothing changes if it throws
  setParameterValue(0,amplitude);
  setParameterValue(1,mean);
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
  }

  double Normal::integralInRange(double low, double high) const
  {
  if (low==high) return 0.0;
  double scale = 1.0/(width()*std::sqrt(2.0));
  return 0.5*integral()*erfDifference((low-mean())*scale,(high-mean())*scale);
  }

  double Normal::expectationValue(double low, double high) const
  {
  double norm = integralInRange(low,high);
  if (norm==0.0) throw MathException("Integral of the function over the range is zero",__FUNCTION__);
  double zl = (low  - mean())/width();
  double zh = (high - mean())/width();
  double firstCentralMoment = amplitude()*width()*width()*(std::exp(-0.5*zl*zl) - std::exp(-0.5*zh*zh));
  return mean() + firstCentralMoment/norm;
  }

  double Normal::expectationValue(double low, double high, int power) const
  {
  if (power==1) return expectationValue(low,high);
  if (power<0)  return ParameterizedFunction::expectationValue(low,high,power);
  double norm = integralInRange(low,high);
  if (norm==0.0) throw MathException("Integral of the function over the range is zero",__FUNCTION__);
  if (power==0) return 1.0;
  if (low>high) std::swap(low,high);
  double zl = (low  - mean())/width();
  double zh = (high - mean())/width();
  // boundary term z^m exp(-z^2/2), zero at infinity
  auto boundary = [](double z, int m) { return std::isinf(z) ? 0.0 : std::pow(z,m)*std::exp(-0.5*z*z); };
  std::vector<double> J(power+1);
  J[0] = std::sqrt(0.5*M_PI)*erfDifference(zl/std::sqrt(2.0),zh/std::sqrt(2.0));
  if (power>=1) J[1] = boundary(zl,0) - boundary(zh,0);
  for (int j=2; j<=power; j++) J[j] = (j-1)*J[j-2] - (boundary(zh,j-1) - boundary(zl,j-1));
  if (J[0]==0.0) throw MathException("Integral of the function over the range underflows",__FUNCTION__);
  // E[x^n] = sum_j C(n,j) mu^(n-j) sigma^j J_j / J_0
  double result = 0.0, binomial = 1.0;
  for (int j=0; j<=power; j++)
    {
    if (j>0) binomial *= double(power-j+1)/j;
    result += binomial*std::pow(mean(),power-j)*std::pow(width(),j)*J[j]/J[0];
    }
  return result;
  }

  void Normal::setNormalized()
  {
  setParameterValue(0,1.0/(width()*sqrtTwoPi()));
  }

  } // namespace MATH
} // namespace CAP
