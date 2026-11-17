#include "MathBasicGeometry.hpp"


template bool Datum::MATH::isInside<double> (double  xp, double  yp, int  np, double  *x, double  *y);
template double Datum::MATH::modulus<double> (const double  v[3]);
template double Datum::MATH::modulusSq<double> (const double  v[3]);
template double Datum::MATH::normalize<double> (double  v[3]);
template double Datum::MATH::scalarProduct<double>    (const double  v1[3], const double  v2[3]);
template double *Datum::MATH::crossProduct<double>    (const double  v1[3], const double  v2[3], double  out[3]);
template double Datum::MATH::normCrossProduct<double> (const double  v1[3], const double  v2[3], double  out[3]);
template double *Datum::MATH::normalToPlane<double>   (const double  v1[3], const double  v2[3], const double  v3[3], double normal[3]);
