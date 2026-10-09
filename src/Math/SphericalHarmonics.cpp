#include "SphericalHarmonics.hpp"
#include <cmath>
#include <string>
#include "MathExceptions.hpp"

ClassImp(CAP::MATH::SphericalHarmonics);

namespace CAP
{
  namespace MATH
  {

  SphericalHarmonics::SphericalHarmonics(unsigned int l, int m, Part part)
  :
  Function(2),
  _l(0),
  _m(0),
  _part(part)
  {
  setDegreeAndOrder(l,m);
  }

  SphericalHarmonics::SphericalHarmonics(const SphericalHarmonics & source)
  :
  Function(source),
  _l(source._l),
  _m(source._m),
  _part(source._part)
  {}

  SphericalHarmonics & SphericalHarmonics::operator=(const SphericalHarmonics & rhs)
  {
  if (this!=&rhs)
    {
    Function::operator=(rhs);
    _l    = rhs._l;
    _m    = rhs._m;
    _part = rhs._part;
    }
  return *this;
  }

  void SphericalHarmonics::setDegreeAndOrder(unsigned int l, int m)
  {
  if ((unsigned int)std::abs(m) > l)
    {
    String message = "Order m=";
    message += std::to_string(m).c_str();
    message += " must satisfy |m| <= l=";
    message += std::to_string(l).c_str();
    throw MathException(message,__FUNCTION__);
    }
  _l = l;
  _m = m;
  }

  void SphericalHarmonics::setRank(unsigned int rank)
  {
  if (rank!=2) throw MathException("A spherical harmonic has rank 2 (theta, phi)",__FUNCTION__);
  Function::setRank(rank);
  }

  double SphericalHarmonics::normalizedLegendre(unsigned int l, unsigned int m, double x)
  {
  if (m>l) throw MathException("Order m must satisfy m <= l",__FUNCTION__);
  if (!(std::fabs(x)<=1.0)) throw MathException("Argument must satisfy |x| <= 1",__FUNCTION__);
  return normalizedLegendre(l,m,x,(1.0-x)*(1.0+x));
  }

  double SphericalHarmonics::normalizedLegendre(unsigned int l, unsigned int m, double x, double oneMinusX2)
  {
  // Start from the normalized P_m^m:  sqrt((2m+1)/(4 pi) (2m-1)!!^2/(2m)!) (1-x^2)^(m/2), built as a product of
  // factors of order 1 to avoid overflow; (-1)^m is the Condon-Shortley phase.
  double pmm = 1.0;
  double fact = 1.0;
  for (unsigned int i=1; i<=m; i++)
    {
    pmm  *= oneMinusX2*fact/(fact+1.0);
    fact += 2.0;
    }
  pmm = std::sqrt((2.0*m+1.0)*pmm/(4.0*M_PI));
  if (m%2==1) pmm = -pmm;
  if (l==m) return pmm;
  // normalized P_{m+1}^m = x sqrt(2m+3) P_m^m, then the normalized three-term recurrence in l
  double pmmp1 = x*std::sqrt(2.0*m+3.0)*pmm;
  if (l==m+1) return pmmp1;
  double oldFactor = std::sqrt(2.0*m+3.0);
  double pll = 0.0;
  for (unsigned int ll=m+2; ll<=l; ll++)
    {
    double factor = std::sqrt((4.0*ll*ll-1.0)/(double(ll)*ll-double(m)*m));
    pll   = (x*pmmp1 - pmm/oldFactor)*factor;
    oldFactor = factor;
    pmm   = pmmp1;
    pmmp1 = pll;
    }
  return pll;
  }

  std::complex<double> SphericalHarmonics::value(double theta, double phi) const
  {
  if (!std::isfinite(theta) || !std::isfinite(phi)) throw MathException("Angles must be finite",__FUNCTION__);
  unsigned int am = (unsigned int)std::abs(_m);
  double s = std::sin(theta);
  double p = normalizedLegendre(_l,am,std::cos(theta),s*s);
  std::complex<double> y = p*std::polar(1.0,double(am)*phi);
  if (_m<0)
    {
    y = std::conj(y);               // Y_l^(-m) = (-1)^m conj(Y_l^m)
    if (am%2==1) y = -y;
    }
  return y;
  }

  double SphericalHarmonics::evaluateAt(double theta, double phi) const
  {
  switch (_part)
    {
    case RealPart:      return value(theta,phi).real();
    case ImaginaryPart: return value(theta,phi).imag();
    case RealHarmonic:
      {
      if (!std::isfinite(theta) || !std::isfinite(phi)) throw MathException("Angles must be finite",__FUNCTION__);
      unsigned int am = (unsigned int)std::abs(_m);
      double s = std::sin(theta);
      double p = normalizedLegendre(_l,am,std::cos(theta),s*s);
      if (am%2==1) p = -p;          // remove the Condon-Shortley phase
      if (_m==0) return p;
      if (_m>0)  return std::sqrt(2.0)*p*std::cos(am*phi);
      return std::sqrt(2.0)*p*std::sin(am*phi);
      }
    }
  throw MathException("Unknown part",__FUNCTION__);
  }

  double SphericalHarmonics::evaluateAt(const std::vector<double> & x) const
  {
  checkRank(x.size(),__FUNCTION__);
  return evaluateAt(x[0],x[1]);
  }

  } // namespace MATH
} // namespace CAP
