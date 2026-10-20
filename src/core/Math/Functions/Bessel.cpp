#include "Bessel.hpp"
#include <cmath>
#include <string>
#include <functional>
#include "BesselFunctions.hpp"
#include "MathExceptions.hpp"

ClassImp(CAP::MATH::Bessel);

namespace CAP
{
  namespace MATH
  {

  namespace
  {
  const double twoOverPi = 2.0/M_PI;

  // Recursive step of adaptive Simpson integration on [a,b]
  double adaptiveSimpson(const std::function<double(double)> & g, double a, double b,
                         double fa, double fm, double fb, double whole, double tolerance, int depth)
  {
  double m = 0.5*(a+b), lm = 0.5*(a+m), rm = 0.5*(m+b);
  double flm = g(lm), frm = g(rm);
  double left  = (m-a)*(fa + 4.0*flm + fm)/6.0;
  double right = (b-m)*(fm + 4.0*frm + fb)/6.0;
  double delta = left + right - whole;
  if (depth<=0 || std::fabs(delta) <= 15.0*tolerance) return left + right + delta/15.0;
  return adaptiveSimpson(g,a,m,fa,flm,fm,left, 0.5*tolerance,depth-1)
       + adaptiveSimpson(g,m,b,fm,frm,fb,right,0.5*tolerance,depth-1);
  }
  }

  Bessel::Bessel(BesselType type, unsigned int order)
  :
  Function(1),
  _type(type),
  _order(order)
  {
  checkOrder(type,order,__FUNCTION__);
  }

  Bessel::Bessel(const Bessel & source)
  :
  Function(source),
  _type(source._type),
  _order(source._order)
  {}

  Bessel & Bessel::operator=(const Bessel & rhs)
  {
  if (this!=&rhs)
    {
    Function::operator=(rhs);
    _type  = rhs._type;
    _order = rhs._order;
    }
  return *this;
  }

  void Bessel::checkOrder(BesselType type, unsigned int order, const char * source)
  {
  if ((type==BesselJ || type==BesselY || type==StruveH || type==StruveL) && order>1)
    {
    String message = "Order ";
    message += std::to_string(order).c_str();
    message += " is not available for this function: only orders 0 and 1";
    throw MathException(message,source);
    }
  }

  void Bessel::setType(BesselType type, unsigned int order)
  {
  checkOrder(type,order,__FUNCTION__);
  _type  = type;
  _order = order;
  }

  void Bessel::setRank(unsigned int rank)
  {
  if (rank!=1) throw MathException("A Bessel function has rank 1",__FUNCTION__);
  Function::setRank(rank);
  }

  // ---------------------------------------------------------------- values

  double Bessel::valueOfOrder(int n, double x) const
  {
  switch (_type)
    {
    case BesselJ: return n==0 ? besselJ0(x) : besselJ1(x);
    case BesselY: return n==0 ? besselY0(x) : besselY1(x);
    case BesselI: return besselI(n,x);
    case BesselK: return besselK(n,x);
    case StruveH: return n==0 ? struveH0(x) : struveH1(x);
    case StruveL: return n==0 ? struveL0(x) : struveL1(x);
    }
  throw MathException("Unknown Bessel type",__FUNCTION__);
  }

  double Bessel::evaluateAt(double x) const
  {
  return valueOfOrder(_order,x);
  }

  double Bessel::evaluateAt(const std::vector<double> & x) const
  {
  checkRank(x.size(),__FUNCTION__);
  return valueOfOrder(_order,x[0]);
  }

  // ---------------------------------------------------------------- derivatives

  double Bessel::derivativeAt(double x) const
  {
  const int n = _order;
  switch (_type)
    {
    case BesselJ:   // J0' = -J1 ; J1' = J0 - J1/x  (1/2 at x = 0)
      if (n==0) return -besselJ1(x);
      return (x==0.0) ? 0.5 : besselJ0(x) - besselJ1(x)/x;
    case BesselY:   // Y0' = -Y1 ; Y1' = Y0 - Y1/x
      if (n==0) return -besselY1(x);
      return besselY0(x) - besselY1(x)/x;
    case BesselI:   // I0' = I1 ; In' = (I_{n-1} + I_{n+1})/2
      if (n==0) return besselI1(x);
      return 0.5*(besselI(n-1,x) + besselI(n+1,x));
    case BesselK:   // K0' = -K1 ; Kn' = -(K_{n-1} + K_{n+1})/2
      if (n==0) return -besselK1(x);
      return -0.5*(besselK(n-1,x) + besselK(n+1,x));
    case StruveH:   // H0' = 2/pi - H1 ; H1' = H0 - H1/x  (0 at x = 0)
      if (n==0) return twoOverPi - struveH1(x);
      return (x==0.0) ? 0.0 : struveH0(x) - struveH1(x)/x;
    case StruveL:   // L0' = 2/pi + L1 ; L1' = L0 - L1/x  (0 at x = 0)
      if (n==0) return twoOverPi + struveL1(x);
      return (x==0.0) ? 0.0 : struveL0(x) - struveL1(x)/x;
    }
  throw MathException("Unknown Bessel type",__FUNCTION__);
  }

  // ---------------------------------------------------------------- integrals

  double Bessel::integralI(unsigned int n, double x)
  {
  // integral of I_n from 0 to x = sum_k (x/2)^(2k+n) x / (k! (k+n)! (2k+n+1)): the terms all have the sign of x^(n+1),
  // so there is no cancellation (the recurrence integral of I_{n+1} = 2 I_n - integral of I_{n-1} cancels badly for x < n)
  if (x==0.0) return 0.0;
  double half = 0.5*x;
  double q = half*half;
  double lead = x;                                          // (x/2)^n x / n!, for k = 0
  for (unsigned int k=1; k<=n; k++) lead *= half/double(k);
  double sum = lead/double(n+1);
  double term = lead;                                       // (x/2)^(2k+n) x / (k! (k+n)!)
  for (int k=0; k<10000; k++)
    {
    term *= q/(double(k+1)*double(k+1+n));
    double t = term/double(2*k+n+3);
    sum += t;
    if (std::fabs(t) <= 1.0E-17*std::fabs(sum) || std::isinf(sum)) break;
    }
  return sum;
  }

  double Bessel::integralK(unsigned int n, double x)
  {
  double k0 = besselK0(x), k1 = besselK1(x);
  // integral of K0 from 0 to x = x K0 + (pi x/2)(K1 L0 + K0 L1)
  double g0 = x*k0 + 0.5*M_PI*x*(k1*struveL0(x) + k0*struveL1(x));
  if (n==0) return g0;
  double g1 = -k0;
  if (n==1) return g1;
  // integral of K_{n+1} = -2 K_n - integral of K_{n-1}
  double gm = g0, g = g1;
  for (unsigned int k=1; k<n; k++)
    {
    double gp = -2.0*besselK(k,x) - gm;
    gm = g;
    g  = gp;
    }
  return g;
  }

  double Bessel::antiderivativeAt(double x) const
  {
  if (!std::isfinite(x)) throw MathException("Argument is not finite",__FUNCTION__);
  switch (_type)
    {
    case BesselJ:
      if (_order==1) return 1.0 - besselJ0(x);                        // -J0, with F(0) = 0
      return x*besselJ0(x) + 0.5*M_PI*x*(besselJ1(x)*struveH0(x) - besselJ0(x)*struveH1(x));
    case BesselY:
      if (_order==1) throw MathException("Y1 is not integrable at 0: use integralInRange() with bounds > 0",__FUNCTION__);
      if (x==0.0) return 0.0;
      if (x<0.0) throw MathException("Invalid argument x < 0",__FUNCTION__);
      return x*besselY0(x) + 0.5*M_PI*x*(besselY1(x)*struveH0(x) - besselY0(x)*struveH1(x));
    case BesselI:
      return integralI(_order,x);
    case BesselK:
      if (_order>=1) throw MathException("K_n, n >= 1, is not integrable at 0: use integralInRange() with bounds > 0",__FUNCTION__);
      if (x==0.0) return 0.0;
      if (x<0.0) throw MathException("Invalid argument x < 0",__FUNCTION__);
      return integralK(0,x);
    case StruveH:
      if (_order==1) return twoOverPi*x - struveH0(x);
      return numericalIntegral(0.0,x);
    case StruveL:
      if (_order==1) return struveL0(x) - twoOverPi*x;
      return numericalIntegral(0.0,x);
    }
  throw MathException("Unknown Bessel type",__FUNCTION__);
  }

  double Bessel::numericalIntegral(double low, double high) const
  {
  if (low==high) return 0.0;
  if (low>high) return -numericalIntegral(high,low);
  auto g = [this](double t) { return evaluateAt(t); };
  const int nPanels = 16;
  double width = (high-low)/nPanels;
  std::vector<double> fs(2*nPanels+1);
  for (int k=0; k<=2*nPanels; k++) fs[k] = g(low + 0.5*k*width);
  double coarseAbs = 0.0;
  for (int k=0; k<nPanels; k++) coarseAbs += width*(std::fabs(fs[2*k]) + 4.0*std::fabs(fs[2*k+1]) + std::fabs(fs[2*k+2]))/6.0;
  double tolerance = 1.0E-12*std::max(coarseAbs,1.0E-300);
  double sum = 0.0;
  for (int k=0; k<nPanels; k++)
    {
    double a = low + k*width, b = a + width;
    double whole = width*(fs[2*k] + 4.0*fs[2*k+1] + fs[2*k+2])/6.0;
    sum += adaptiveSimpson(g,a,b,fs[2*k],fs[2*k+1],fs[2*k+2],whole,tolerance/nPanels,40);
    }
  return sum;
  }

  double Bessel::integralInRange(double low, double high) const
  {
  if (!std::isfinite(low) || !std::isfinite(high)) throw MathException("Integration bounds must be finite",__FUNCTION__);
  if (low==high) return 0.0;
  if (_type==BesselY || _type==BesselK)
    {
    double lowest = std::min(low,high);
    bool zeroAllowed = (_order==0);
    if (lowest<0.0 || (lowest==0.0 && !zeroAllowed))
      throw MathException("Integration bounds must be > 0 (>= 0 for Y0 and K0)",__FUNCTION__);
    if (_type==BesselY && _order==1) return besselY0(low) - besselY0(high);   // integral of Y1 = -Y0
    if (_type==BesselK && _order>=1)
      {
      double gLow = integralK(_order,low), gHigh = integralK(_order,high);
      double result = gHigh - gLow;
      if (std::fabs(result) < 1.0E-3*std::max(std::fabs(gLow),std::fabs(gHigh))) return numericalIntegral(low,high);
      return result;
      }
    }
  if (_type==StruveH && _order==0) return numericalIntegral(low,high);
  if (_type==StruveL && _order==0) return numericalIntegral(low,high);
  double fLow  = antiderivativeAt(low);
  double fHigh = antiderivativeAt(high);
  double result = fHigh - fLow;
  // when the antiderivatives nearly cancel (e.g. integral of K0 over [10,40]: both close to pi/2), the difference loses
  // most of its digits: integrate numerically instead (the bounds are then away from the singularity of Y0 and K0 at 0)
  if (std::fabs(result) < 1.0E-3*std::max(std::fabs(fLow),std::fabs(fHigh)))
    return numericalIntegral(low,high);
  return result;
  }

  } // namespace MATH
} // namespace CAP
