#include "Exponential.hpp"

#include <algorithm>
#include "MathExceptions.hpp"

ClassImp(CAP::MATH::Exponential);

namespace CAP
{
  namespace MATH
  {

  Exponential::Exponential(double a, double b)
  :
  ParameterizedFunction({Parameter("amplitude",a),Parameter("exponent",b)})
  { }

  double Exponential::integralInRange(double low, double high) const
  {
  if (low==high) return 0.0;
  if (exponent()==0.0) return amplitude()*(high-low);
  if (low>high) return -integralInRange(high,low);
  // a/b exp(b low) (exp(b (high-low)) - 1); expm1 keeps precision when b (high-low) is small.
  // Factor out the larger of the two exponentials' arguments' starting point so that
  // infinite bounds give finite results when the integral converges.
  if (exponent()>0.0)
    return  amplitude()/exponent()*std::exp(exponent()*high)*(-std::expm1(-exponent()*(high-low)));
  else
    return  amplitude()/exponent()*std::exp(exponent()*low)*std::expm1(exponent()*(high-low));
  }

  double Exponential::expectationValue(double low, double high) const
  {
  if (amplitude()==0.0) throw MathException("Integral of the function over the range is zero (amplitude is zero)",__FUNCTION__);
  if (low>high) std::swap(low,high);
  if (std::isinf(low) || std::isinf(high))
    {
    if (std::isinf(high) && !std::isinf(low) && exponent()<0.0) return low  - 1.0/exponent();
    if (std::isinf(low) && !std::isinf(high) && exponent()>0.0) return high - 1.0/exponent();
    throw MathException("Integral diverges over the range",__FUNCTION__);
    }
  double d = high - low;
  if (d==0.0) return low;
  double t = exponent()*d;
  // g(t) = 1/(1 - exp(-t)) - 1/t ; g(0) = 1/2
  double g;
  if (std::fabs(t)<1.0E-3)
    g = 0.5 + t/12.0 - t*t*t/720.0;
  else
    g = -1.0/std::expm1(-t) - 1.0/t;
  return low + d*g;
  }

  namespace
  {
  // K_j = integral from 0 to d of u^j exp(-lambda u) du, with lambda >= 0 (d may be infinite if lambda > 0).
  // K_j = j!/lambda^(j+1) P(j+1, lambda d), P = regularized lower incomplete gamma function.
  double decayingMoment(int j, double lambda, double d)
  {
  double factorial = std::tgamma(j+1.0);
  int a = j+1;
  if (lambda==0.0) return std::pow(d,a)/a;
  double y = lambda*d;
  if (!std::isinf(y) && y < a+1.0)
    {
    // series: P(a,y) = exp(-y) y^a / Gamma(a) sum_k y^k / (a (a+1) ... (a+k)), so K_j = exp(-y) d^a sum
    double term = 1.0/a, sum = term;
    for (int k=1; k<500; k++)
      {
      term *= y/(a+k);
      sum  += term;
      if (term < 1.0E-17*sum) break;
      }
    return std::exp(-y)*std::pow(d,a)*sum;
    }
  // complement: P(a,y) = 1 - exp(-y) sum_{i<=j} y^i/i!  (P close to 1 here, no cancellation)
  double q = 0.0;
  if (!std::isinf(y))
    {
    double term = 1.0, sum = 1.0;
    for (int i=1; i<=j; i++) { term *= y/i; sum += term; }
    q = std::exp(-y)*sum;
    }
  return factorial/std::pow(lambda,a)*(1.0 - q);
  }
  }

  double Exponential::expectationValue(double low, double high, int power) const
  {
  if (power==1) return expectationValue(low,high);
  if (power<0)  return ParameterizedFunction::expectationValue(low,high,power);
  if (amplitude()==0.0) throw MathException("Integral of the function over the range is zero (amplitude is zero)",__FUNCTION__);
  if (low>high) std::swap(low,high);
  if (low==high) return std::pow(low,power);
  // Reference point at the end where the weight is largest; weight exp(-lambda u) decays with u >= 0.
  double ref, dir, lambda;
  if (exponent()<=0.0) { ref = low;  dir =  1.0; lambda = -exponent(); }
  else                { ref = high; dir = -1.0; lambda =  exponent(); }
  double d = high - low;
  if (std::isinf(ref) || (std::isinf(d) && lambda==0.0))
    throw MathException("Integral diverges over the range",__FUNCTION__);
  if (power==0) return 1.0;
  // E[x^n] = sum_j C(n,j) ref^(n-j) dir^j K_j / K_0
  double k0 = decayingMoment(0,lambda,d);
  double result = 0.0, binomial = 1.0;
  for (int j=0; j<=power; j++)
    {
    if (j>0) binomial *= double(power-j+1)/j;
    double kj = (j==0) ? k0 : decayingMoment(j,lambda,d);
    result += binomial*std::pow(ref,power-j)*std::pow(dir,j)*kj/k0;
    }
  return result;
  }

  } // namespace MATH
} // namespace CAP
