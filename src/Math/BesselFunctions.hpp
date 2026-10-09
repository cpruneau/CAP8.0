#ifndef CAP_BesselFunctions
#define CAP_BesselFunctions
#include "MathBasicFunctions.hpp"

//!
//! Bessel and Struve functions of integer order.
//!
//! Accuracy: these use the polynomial and rational approximations of Abramowitz & Stegun and Numerical Recipes,
//! and Chebyshev series (CERNLIB) for the Struve functions. Relative accuracy is about 1e-7 (I0, I1, K0, K1, Y0, and
//! I_n, K_n which are computed from them), 1e-8 (J0, J1, Y1) and 1e-9 (H0, H1), 1e-12 (L0, L1); not full double precision.
//! Functions defined only for x > 0 (K, Y) throw MathException otherwise.

namespace CAP
{
namespace MATH
{

//! Bessel and Struve functions
double besselI(int n,double x) ;
double besselK(int n,double x) ;
double besselI0(double x);
double besselK0(double x) ;
double besselI1(double x);
double besselK1(double x) ;
double besselJ0(double x);
double besselJ1(double x);
double besselY0(double x);
double besselY1(double x);
double struveH0(double x);
double struveH1(double x);
double struveL0(double x);
double struveL1(double x);

} // namespace MATH
} // namespace CAP

#endif
