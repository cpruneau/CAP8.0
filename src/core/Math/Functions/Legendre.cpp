#include "Legendre.hpp"
#include <cmath>
#include <algorithm>
#include "MathExceptions.hpp"


namespace CAP
{
  namespace MATH
  {

  Legendre::Legendre(unsigned int n)
  :
  Function(1),
  _degree(n)
  {}

  Legendre::Legendre(const Legendre & source)
  :
  Function(source),
  _degree(source._degree)
  {}

  Legendre & Legendre::operator=(const Legendre & rhs)
  {
  if (this!=&rhs)
    {
    Function::operator=(rhs);
    _degree = rhs._degree;
    }
  return *this;
  }

  void Legendre::setRank(unsigned int rank)
  {
  if (rank!=1) throw MathException("A Legendre polynomial has rank 1",__FUNCTION__);
  Function::setRank(rank);
  }

  double Legendre::value(unsigned int n, double x)
  {
  if (n==0) return 1.0;
  double previous = 1.0;   // P_0
  double current  = x;     // P_1
  for (unsigned int k=1; k<n; k++)
    {
    double next = ((2.0*k + 1.0)*x*current - k*previous)/(k + 1.0);
    previous = current;
    current  = next;
    }
  return current;
  }

  double Legendre::derivative(unsigned int n, double x)
  {
  if (n==0) return 0.0;
  // P'_{k+1} = P'_{k-1} + (2k+1) P_k, with P'_0 = 0 and P'_1 = 1, together with the recurrence for P_k
  double pPrevious = 1.0, p = x;         // P_0, P_1
  double dPrevious = 0.0, d = 1.0;       // P'_0, P'_1
  for (unsigned int k=1; k<n; k++)
    {
    double dNext = dPrevious + (2.0*k + 1.0)*p;
    double pNext = ((2.0*k + 1.0)*x*p - k*pPrevious)/(k + 1.0);
    dPrevious = d;  d = dNext;
    pPrevious = p;  p = pNext;
    }
  return d;
  }

  double Legendre::evaluateAt(double x) const
  {
  return value(_degree,x);
  }

  double Legendre::evaluateAt(const std::vector<double> & x) const
  {
  checkRank(x.size(),__FUNCTION__);
  return value(_degree,x[0]);
  }

  double Legendre::derivativeAt(double x) const
  {
  return derivative(_degree,x);
  }

  double Legendre::antiderivativeAt(double x) const
  {
  if (_degree==0) return x;
  return (value(_degree+1,x) - value(_degree-1,x))/(2.0*_degree + 1.0);
  }

  void Legendre::gaussLegendre(unsigned int m, std::vector<double> & nodes, std::vector<double> & weights)
  {
  if (m==0) throw MathException("Number of points must be at least 1",__FUNCTION__);
  nodes.assign(m,0.0);
  weights.assign(m,0.0);
  for (unsigned int i=0; i<(m+1)/2; i++)   // roots are symmetric: compute the positive ones
    {
    double z  = std::cos(M_PI*(i + 0.75)/(m + 0.5));   // initial guess for root i
    double dp = 1.0;
    for (int iteration=0; iteration<100; iteration++)
      {
      double pm = value(m,z);
      dp = derivative(m,z);
      double dz = pm/dp;
      z -= dz;
      if (std::fabs(dz) <= 1.0E-16) break;
      }
    dp = derivative(m,z);
    double w = 2.0/((1.0 - z*z)*dp*dp);
    nodes[i]       =  z;   weights[i]       = w;
    nodes[m-1-i]   = -z;   weights[m-1-i]   = w;
    }
  if (m%2==1) nodes[m/2] = 0.0;
  }

  double Legendre::integralInRange(double low, double high) const
  {
  if (!std::isfinite(low) || !std::isfinite(high)) throw MathException("Integration bounds must be finite",__FUNCTION__);
  if (low==high) return 0.0;
  double fLow  = antiderivativeAt(low);
  double fHigh = antiderivativeAt(high);
  double result = fHigh - fLow;
  if (std::fabs(result) >= 1.0E-3*std::max(std::fabs(fLow),std::fabs(fHigh))) return result;
  // the antiderivatives nearly cancel: Gauss-Legendre quadrature with n/2+1 points, exact for a polynomial of degree n
  std::vector<double> nodes, weights;
  gaussLegendre(_degree/2+1,nodes,weights);
  double mid = 0.5*(high+low), half = 0.5*(high-low), sum = 0.0;
  for (unsigned int i=0; i<nodes.size(); i++) sum += weights[i]*evaluateAt(mid + half*nodes[i]);
  return half*sum;
  }

  } // namespace MATH
} // namespace CAP
