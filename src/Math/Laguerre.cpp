#include "Laguerre.hpp"
#include <cmath>
#include <algorithm>
#include "MathExceptions.hpp"

ClassImp(CAP::MATH::Laguerre);

namespace CAP
{
  namespace MATH
  {

  Laguerre::Laguerre(unsigned int n, double alpha)
  :
  Function(1),
  _degree(n),
  _alpha(0.0)
  {
  setAlpha(alpha);
  }

  Laguerre::Laguerre(const Laguerre & source)
  :
  Function(source),
  _degree(source._degree),
  _alpha(source._alpha)
  {}

  Laguerre & Laguerre::operator=(const Laguerre & rhs)
  {
  if (this!=&rhs)
    {
    Function::operator=(rhs);
    _degree = rhs._degree;
    _alpha  = rhs._alpha;
    }
  return *this;
  }

  void Laguerre::setAlpha(double alpha)
  {
  if (!std::isfinite(alpha)) throw MathException("alpha must be finite",__FUNCTION__);
  _alpha = alpha;
  }

  void Laguerre::setRank(unsigned int rank)
  {
  if (rank!=1) throw MathException("A Laguerre polynomial has rank 1",__FUNCTION__);
  Function::setRank(rank);
  }

  double Laguerre::value(unsigned int n, double alpha, double x)
  {
  if (n==0) return 1.0;
  double previous = 1.0;              // L_0
  double current  = 1.0 + alpha - x;  // L_1
  for (unsigned int k=1; k<n; k++)
    {
    double next = ((2.0*k + 1.0 + alpha - x)*current - (k + alpha)*previous)/(k + 1.0);
    previous = current;
    current  = next;
    }
  return current;
  }

  double Laguerre::evaluateAt(double x) const
  {
  return value(_degree,_alpha,x);
  }

  double Laguerre::evaluateAt(const std::vector<double> & x) const
  {
  checkRank(x.size(),__FUNCTION__);
  return value(_degree,_alpha,x[0]);
  }

  double Laguerre::derivativeAt(double x) const
  {
  if (_degree==0) return 0.0;
  return -value(_degree-1,_alpha+1.0,x);
  }

  double Laguerre::antiderivativeAt(double x) const
  {
  return -value(_degree+1,_alpha-1.0,x);
  }

  namespace
  {
  // Gauss-Legendre quadrature with m points on [low, high]: exact for polynomials of degree <= 2m-1.
  // Nodes are the roots of the Legendre polynomial P_m, found by Newton's method.
  double gaussLegendre(const Laguerre & f, double low, double high, unsigned int m)
  {
  double mid = 0.5*(high+low), half = 0.5*(high-low);
  double sum = 0.0;
  for (unsigned int i=0; i<m; i++)
    {
    double z = std::cos(M_PI*(i+0.75)/(m+0.5));   // initial guess for root i
    double dp = 1.0;
    for (int iteration=0; iteration<100; iteration++)
      {
      double p0 = 1.0, p1 = z;                      // P_0, P_1, then upward recurrence to P_m
      for (unsigned int k=2; k<=m; k++)
        {
        double p2 = ((2.0*k-1.0)*z*p1 - (k-1.0)*p0)/k;
        p0 = p1;
        p1 = p2;
        }
      if (m==1) p0 = 1.0;
      double pm = (m==1) ? z : p1;
      dp = m*(z*pm - p0)/(z*z - 1.0);               // P_m'(z)
      double dz = pm/dp;
      z -= dz;
      if (std::fabs(dz) <= 1.0E-16) break;
      }
    double weight = 2.0/((1.0 - z*z)*dp*dp);
    sum += weight*f.evaluateAt(mid + half*z);
    }
  return half*sum;
  }
  }

  double Laguerre::integralInRange(double low, double high) const
  {
  if (!std::isfinite(low) || !std::isfinite(high)) throw MathException("Integration bounds must be finite",__FUNCTION__);
  if (low==high) return 0.0;
  double fLow  = antiderivativeAt(low);
  double fHigh = antiderivativeAt(high);
  double result = fHigh - fLow;
  // when the antiderivatives nearly cancel (short intervals), use Gauss-Legendre quadrature, exact for a polynomial of degree n
  if (std::fabs(result) < 1.0E-3*std::max(std::fabs(fLow),std::fabs(fHigh)))
    return gaussLegendre(*this,low,high,_degree/2+1);
  return result;
  }

  double Laguerre::weightAt(double x) const
  {
  if (x<0.0) throw MathException("Weight is defined for x >= 0",__FUNCTION__);
  return std::pow(x,_alpha)*std::exp(-x);
  }

  double Laguerre::norm() const
  {
  if (!isOrthogonal()) throw MathException("Norm diverges for alpha <= -1",__FUNCTION__);
  // Gamma(n+alpha+1)/n!, computed with logarithms to avoid overflow
  return std::exp(std::lgamma(_degree + _alpha + 1.0) - std::lgamma(_degree + 1.0));
  }

  } // namespace MATH
} // namespace CAP
