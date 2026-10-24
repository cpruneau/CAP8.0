#include "VectorLorentz.hpp"
#include "MathExceptions.hpp"
#include "PrintHelpers.hpp"
#include <iostream>



namespace CAP
{
namespace MATH
{
VectorLorentz::VectorLorentz()
:
_x0(0),
_x1(0),
_x2(0),
_x3(0)
{ }

VectorLorentz::VectorLorentz(const double & x0, const double & x1, const double & x2, const double & x3)
:
_x0(x0),
_x1(x1),
_x2(x2),
_x3(x3)
{  }

VectorLorentz::VectorLorentz(const float values[4])
:
_x0(values[0]),
_x1(values[1]),
_x2(values[2]),
_x3(values[3])
{  }

VectorLorentz::VectorLorentz(const double values[4])
:
_x0(values[0]),
_x1(values[1]),
_x2(values[2]),
_x3(values[3])
{  }

VectorLorentz::VectorLorentz(const std::vector<float> & values)
:
_x0(0),
_x1(0),
_x2(0),
_x3(0)
{
  setTXYZ(values);
}

VectorLorentz::VectorLorentz(const std::vector<double> & values)
:
_x0(0),
_x1(0),
_x2(0),
_x3(0)
{
  setTXYZ(values);
}

VectorLorentz::VectorLorentz(const VectorLorentz & source)
:
_x0(source._x0),
_x1(source._x1),
_x2(source._x2),
_x3(source._x3)
{  }


//!
//! Returns the component stored at the given index: 0 for t, 1 for x, 2 for y, 3 for z.
//!
double VectorLorentz::operator[] (unsigned int index) const
{
  switch (index)
    {
      case 0: return _x0;
      case 1: return _x1;
      case 2: return _x2;
      case 3: return _x3;
      default: throw MathException("index must be 0, 1, 2 or 3",__FUNCTION__);
    }
}

void VectorLorentz::setTXYZ(const double & x0, const double & x1, const double & x2, const double & x3)
{
  _x0 = x0;
  _x1 = x1;
  _x2 = x2;
  _x3 = x3;
}

void VectorLorentz::setTXYZ(const float values[4])
{
  _x0 = values[0];
  _x1 = values[1];
  _x2 = values[2];
  _x3 = values[3];
}

void VectorLorentz::setTXYZ(const double values[4])
{
  _x0 = values[0];
  _x1 = values[1];
  _x2 = values[2];
  _x3 = values[3];
}

void VectorLorentz::setTXYZ(const std::vector<float> & values)
{
  if (values.size()<4) throw MathException("values must contain at least 4 elements",__FUNCTION__);
  _x0 = values[0];
  _x1 = values[1];
  _x2 = values[2];
  _x3 = values[3];
}

void VectorLorentz::setTXYZ(const std::vector<double> & values)
{
  if (values.size()<4) throw MathException("values must contain at least 4 elements",__FUNCTION__);
  _x0 = values[0];
  _x1 = values[1];
  _x2 = values[2];
  _x3 = values[3];
}

void VectorLorentz::setTXYZ(const VectorLorentz & source)
{
  _x0 = source._x0;
  _x1 = source._x1;
  _x2 = source._x2;
  _x3 = source._x3;
}

void VectorLorentz::setTPhiThetaR(const double & t, const double & phi, const double & theta, const double & r)
{
  double rSinTheta = r*std::sin(theta);
  _x0 = t;
  _x1 = rSinTheta*std::cos(phi);
  _x2 = rSinTheta*std::sin(phi);
  _x3 = r*std::cos(theta);
}


void VectorLorentz::setTPhiRhoZ(const double & t, const double & phi, const double & rho, const double & z)
{
  _x0 = t;
  _x1 = rho*std::cos(phi);
  _x2 = rho*std::sin(phi);
  _x3 = z;
}

void VectorLorentz::setMXYZ(const double & m, const double & x, const double & y, const double & z)
{
  _x0 = std::sqrt(m*m + x*x + y*y + z*z);
  _x1 = x;
  _x2 = y;
  _x3 = z;
}

void VectorLorentz::setMPhiRhoZ(const double & m, const double & phi, const double & rho, const double & z)
{
  _x1 = rho*std::cos(phi);
  _x2 = rho*std::sin(phi);
  _x3 = z;
  _x0 = std::sqrt(m*m + _x1*_x1 + _x2*_x2 + _x3*_x3);
}

void VectorLorentz::setMPhiPtY(const double & m, const double & phi, const double & pt, const double & y)
{
  double mt = std::sqrt(m*m + pt*pt);
  _x0 = mt*std::cosh(y);
  _x1 = pt*std::cos(phi);
  _x2 = pt*std::sin(phi);
  _x3 = mt*std::sinh(y);
}

//!
//! Return the azimuthal angle between this and the other vector, phi - other.phi,
//! mapped onto the range [0, 2pi).
double  VectorLorentz::deltaPhi(const VectorLorentz & other) const
{
  double dPhi = std::atan2(_x2,_x1) - std::atan2(other._x2,other._x1);
  while (dPhi>=twoPi()) dPhi -= twoPi();
  while (dPhi<0) dPhi += twoPi();
  return dPhi;
}

double VectorLorentz::modulus() const
{
  double s = _x0*_x0 - (_x1*_x1+_x2*_x2+_x3*_x3);
  if (s<0) throw MathException("t*t - (x*x+y*y+z*z) < 0: space-like vector",__FUNCTION__);
  return std::sqrt(s);
}

Vector3D VectorLorentz::xyz() const
{
  return  Vector3D( _x1, _x2, _x3);
}

//!
//! Invariant mass. Small negative values of the mass squared caused by rounding
//! (e.g., for massless particles) are treated as zero.
//!
double VectorLorentz::mass() const
{
  double mSq = modulusSquare();
  if (mSq<0)
    {
    if (-mSq <= 1.0E-12*_x0*_x0) return 0.0; // rounding: massless particle
    throw MathException("modulusSquare()<0",__FUNCTION__);
    }
  return std::sqrt(mSq);
}

double VectorLorentz::betaX() const
{
  if (_x0<=0) throw MathException("t<=0",__FUNCTION__);
  return _x1/_x0;
}

double VectorLorentz::betaY() const
{
  if (_x0<=0) throw MathException("t<=0",__FUNCTION__);
  return _x2/_x0;
}

double VectorLorentz::betaZ() const
{
  if (_x0<=0) throw MathException("t<=0",__FUNCTION__);
  return _x3/_x0;
}

double VectorLorentz::beta() const
{
  if (_x0<=0) throw MathException("t<=0",__FUNCTION__);
  return std::sqrt(_x1*_x1+_x2*_x2+_x3*_x3)/_x0;
}

double VectorLorentz::betaSquare() const
{
  if (_x0<=0) throw MathException("t<=0",__FUNCTION__);
  return (_x1*_x1+_x2*_x2+_x3*_x3)/(_x0*_x0);
}

Vector3D VectorLorentz::betaVector() const
{
  if (_x0<=0) throw MathException("t<=0",__FUNCTION__);
  return Vector3D(_x1/_x0,_x2/_x0,_x3/_x0);
}

double VectorLorentz::gammaSquare() const
{
  if (_x0<=0) throw MathException("t<=0",__FUNCTION__);
  double bSq = (_x1*_x1+_x2*_x2+_x3*_x3)/(_x0*_x0);
  if (bSq>=1) throw MathException("betaSq()>=1",__FUNCTION__);
  return 1.0/(1.0-bSq);
}

double VectorLorentz::gamma() const
{
  if (_x0<=0) throw MathException("t<=0",__FUNCTION__);
  double bSq = (_x1*_x1+_x2*_x2+_x3*_x3)/(_x0*_x0);
  if (bSq>=1) throw MathException("betaSq()>=1",__FUNCTION__);
  return 1.0/std::sqrt(1.0-bSq);
}

double VectorLorentz::pseudorapidity() const
{
  double ct = cosTheta();
  if (ct*ct < 1) return -0.5* std::log( (1.0-ct)/(1.0+ct) );
  return (_x3 == 0) ? 0 : ((_x3>0) ? 1E10 : -1E10);
}

//!
//!Transverse mass square
//!
double VectorLorentz::transverseMassSq() const
{
  return _x0*_x0 - _x3*_x3;
}

//!
//!Transverse mass
//!
double VectorLorentz::transverseMass() const
{
  double mtSq = _x0*_x0 - _x3*_x3;
  if (mtSq<0)
    {
    if (-mtSq <= 1.0E-12*_x0*_x0) return 0.0; // rounding
    throw MathException("t*t - z*z < 0",__FUNCTION__);
    }
  return std::sqrt(mtSq);
}

VectorLorentz & VectorLorentz::operator= (const VectorLorentz & rhs)
{
  if (this!=&rhs)
    {
    _x0 = rhs._x0;
    _x1 = rhs._x1;
    _x2 = rhs._x2;
    _x3 = rhs._x3;
    }
  return *this;
}

bool  VectorLorentz::operator== (const VectorLorentz & rhs) const
{
  return (_x0==rhs._x0)  && 
  (_x1==rhs._x1)  && 
  (_x2==rhs._x2)  && 
  (_x3==rhs._x3);
}

bool  VectorLorentz::operator!= (const VectorLorentz & rhs) const
{
  return (_x0!=rhs._x0) ||
  (_x1!=rhs._x1) ||
  (_x2!=rhs._x2) ||
  (_x3!=rhs._x3);
}

VectorLorentz & VectorLorentz::operator+= (const VectorLorentz & rhs)
{
  _x0 += rhs._x0;
  _x1 += rhs._x1;
  _x2 += rhs._x2;
  _x3 += rhs._x3;
  return *this;
}

VectorLorentz & VectorLorentz::operator-= (const VectorLorentz & rhs)
{
  _x0 -= rhs._x0;
  _x1 -= rhs._x1;
  _x2 -= rhs._x2;
  _x3 -= rhs._x3;
  return *this;
}

//! Unary minus: all four components are negated, so that v + (-v) is the null vector.
VectorLorentz VectorLorentz::operator- () const
{
  return VectorLorentz(-_x0,-_x1,-_x2,-_x3);
}

//! produce a 3D unit vector in the direction of this vector
Vector3D VectorLorentz::unit() const
{
  double r = std::sqrt(_x1*_x1+_x2*_x2+_x3*_x3);
  if (r==0) throw MathException("Cannot produce unit vector from null vector",__FUNCTION__);
  return Vector3D(_x1/r,_x2/r,_x3/r);
}

//! Active rotation of this VectorLorentz by the given angle relative to the x-axis
void VectorLorentz::rotateX(const double & angle)
{
  double s = std::sin(angle);
  double c = std::cos(angle);
  double y = _x2;
  double z = _x3;
  _x2 = c*y - s*z;
  _x3 = s*y + c*z;
}

//! Active rotation of this VectorLorentz by the given angle relative to the y-axis
void VectorLorentz::rotateY(const double & angle)
{
  double s = std::sin(angle);
  double c = std::cos(angle);
  double x = _x1;
  double z = _x3;
  _x1 = c*x + s*z;
  _x3 = c*z - s*x;
}

//! Active rotation of this VectorLorentz by the given angle relative to the z-axis
void VectorLorentz::rotateZ(const double & angle)
{
  double s = std::sin(angle);
  double c = std::cos(angle);
  double x = _x1;
  double y = _x2;
  _x1 = c*x - s*y;
  _x2 = s*x + c*y;
}

//! Boost this vector by velocity beta along the x axis: t' = gamma(t + beta x), x' = gamma(x + beta t)
void VectorLorentz::boostX(const double & beta)
{
  if (std::fabs(beta)>=1.0)
    throw  MathException("std::fabs(beta)>=1.0",__FUNCTION__);
  double gamma = 1.0/std::sqrt(1.0 - beta*beta);
  double t = _x0;
  double s = _x1;
  _x0 = gamma*(t + beta*s);
  _x1  = gamma*(s + beta*t);
}

//! Boost this vector by velocity beta along the y axis: t' = gamma(t + beta y), y' = gamma(y + beta t)
void VectorLorentz::boostY(const double & beta)
{
  if (std::fabs(beta)>=1.0)
    throw  MathException("std::fabs(beta)>=1.0",__FUNCTION__);
  double gamma = 1.0/std::sqrt(1.0 - beta*beta);
  double t = _x0;
  double s = _x2;
  _x0 = gamma*(t + beta*s);
  _x2  = gamma*(s + beta*t);
}

//! Boost this vector by velocity beta along the z axis: t' = gamma(t + beta z), z' = gamma(z + beta t)
void VectorLorentz::boostZ(const double & beta)
{
  if (std::fabs(beta)>=1.0)
    throw  MathException("std::fabs(beta)>=1.0",__FUNCTION__);
  double gamma = 1.0/std::sqrt(1.0 - beta*beta);
  double t = _x0;
  double s = _x3;
  _x0 = gamma*(t + beta*s);
  _x3  = gamma*(s + beta*t);
}

//!
//!Boost this Lorentz vector by the given velocity vector (betaX,betaY,betaZ), |beta|<1
//!
void VectorLorentz::boost(const double & betaX,const double & betaY,const double & betaZ)
{
  if (std::fabs(betaX)>=1.0) throw  MathException("std::fabs(betaX)>=1.0",__FUNCTION__);
  if (std::fabs(betaY)>=1.0) throw  MathException("std::fabs(betaY)>=1.0",__FUNCTION__);
  if (std::fabs(betaZ)>=1.0) throw  MathException("std::fabs(betaZ)>=1.0",__FUNCTION__);
  double  betaSq = betaX*betaX + betaY*betaY + betaZ*betaZ;
  if (std::fabs(betaSq)>=1.0) throw  MathException("std::fabs(betaSq)>=1.0",__FUNCTION__);
  double  gamma = 1.0 / std::sqrt(1.0 - betaSq);
  double  bp = betaX*_x1 + betaY*_x2 + betaZ*_x3;
  double  gamma2 = betaSq > 0 ? (gamma - 1.0)/betaSq : 0.0;
  double  x0p = _x0;
  _x0 = gamma*(_x0+bp);
  _x1 += betaX*(gamma2*bp+gamma*x0p);      //gamma2*bp*betaX+gamma*betaX*x0p; // ;
  _x2 += betaY*(gamma2*bp+gamma*x0p);      //gamma2*bp*betaY+gamma*betaY*x0p;
  _x3 += betaZ*(gamma2*bp+gamma*x0p);      //gamma2*bp*betaZ+gamma*betaZ*x0p;
}

//!
//!Boost this Lorentz vector by the given velocity vector beta, |beta|<1
//!
void VectorLorentz::boost(const Vector3D & beta)
{
  boost(beta.x(),beta.y(),beta.z());
}

//!
//!Compute and return the rapidity of this vector (wrt the z-axis)
//!
double VectorLorentz::rapidity() const
{
  double num = _x0+_x3;
  double den = _x0-_x3;
  if (num<=0 || den<=0) throw MathException("rapidity requires t>|z|",__FUNCTION__);
  return 0.5*std::log(num/den);
}

//!
//! Return the rapidity difference between this and the other vector
double  VectorLorentz::deltaRapidity(const VectorLorentz & other) const
{
  return rapidity() - other.rapidity();
}

//!
//! Return the pseudorapidity difference between this and the other vector
//!
double  VectorLorentz::deltaPseudoRapidity(const VectorLorentz & other) const
{
  return pseudorapidity() - other.pseudorapidity();
}


void VectorLorentz::print() const
{
  std::cout << "(" << _x0 << "," << _x1 << "," << _x2 << "," << _x3 << ") " << std::endl;
}

//!
//! Add the two vectors and return a new vector
//!
VectorLorentz operator+ (const VectorLorentz & left, const VectorLorentz & right)
{
  return VectorLorentz(left._x0+right._x0, left._x1+right._x1, left._x2+right._x2, left._x3+right._x3);
}

VectorLorentz operator- (const VectorLorentz & left, const VectorLorentz & right)
{
  return VectorLorentz(left._x0-right._x0, left._x1-right._x1, left._x2-right._x2, left._x3-right._x3);
}


//!
//! SR (4D) scalar product of two left and right vectors
//!
double operator* (const VectorLorentz & left, const VectorLorentz & right)
  {
  return left._x0*right._x0 - left._x1*right._x1 - left._x2*right._x2 - left._x3*right._x3;
  }


//!
//! Scaling of the left vector by a scalar value from the right
//!
VectorLorentz operator* (const VectorLorentz & v, double a)
  {
  return VectorLorentz(a*v._x0, a*v._x1, a*v._x2, a*v._x3);
  }

//!
//! Scaling of the right vector by a scalar value from the left
//!
VectorLorentz operator* (double a, const VectorLorentz & v)
  {
  return VectorLorentz(a*v._x0, a*v._x1, a*v._x2, a*v._x3);
  }

//!
//! Print out the given vector to the given ostream.
//!
std::ostream& operator<<(std::ostream& out, const VectorLorentz & v)
{
  out << "(" << v.t() << "," << v.x() << "," << v.y() << "," << v.z() << ")" ;
  return out;
}

double VectorLorentz::distanceT(const VectorLorentz & other) const
{
  return _x0 - other._x0;
}

double VectorLorentz::distanceX(const VectorLorentz & other) const
{
  return _x1 - other._x1;
}

double VectorLorentz::distanceY(const VectorLorentz & other) const
{
  return _x2 - other._x2;
}

double VectorLorentz::distanceZ(const VectorLorentz & other) const
{
  return _x3 - other._x3;
}

//!
//!Square Distance in the transverse plane xy
//!
double VectorLorentz::distance2DSq(const VectorLorentz & other) const
{
  double dx1 = _x1 - other._x1;
  double dx2 = _x2 - other._x2;
  return dx1*dx1+dx2*dx2;
}

//!
//!Distance in the transverse plane xy
//!
double VectorLorentz::distance2D(const VectorLorentz & other) const
{
  double dx1 = _x1 - other._x1;
  double dx2 = _x2 - other._x2;
  return std::sqrt(dx1*dx1+dx2*dx2);
}

double VectorLorentz::distance3DSq(const VectorLorentz & other) const
{
  double dx1 = _x1 - other._x1;
  double dx2 = _x2 - other._x2;
  double dx3 = _x3 - other._x3;
  return dx1*dx1+dx2*dx2+dx3*dx3;
}

double VectorLorentz::distance3D(const VectorLorentz & other) const
{
  double dx1 = _x1 - other._x1;
  double dx2 = _x2 - other._x2;
  double dx3 = _x3 - other._x3;
  return std::sqrt(dx1*dx1+dx2*dx2+dx3*dx3);
}

double VectorLorentz::distanceSq(const VectorLorentz & other) const
{
  double dx0 = _x0 - other._x0;
  double dx1 = _x1 - other._x1;
  double dx2 = _x2 - other._x2;
  double dx3 = _x3 - other._x3;
  return dx0*dx0 -(dx1*dx1+dx2*dx2+dx3*dx3);
}

double VectorLorentz::distance(const VectorLorentz & other) const
{
  double dx0 = _x0 - other._x0;
  double dx1 = _x1 - other._x1;
  double dx2 = _x2 - other._x2;
  double dx3 = _x3 - other._x3;
  double dsq = dx0*dx0 -(dx1*dx1+dx2*dx2+dx3*dx3);
  if (dsq>=0)
    return std::sqrt(dsq);
  throw MathDomainException(0.0,1.E100,dsq,"std::sqrt(arg) w/ arg<0",__FUNCTION__);
}

  void VectorLorentz::encodeBinary(BinaryWriter & w) const
  {
  w.writeFloat64(_x0);
  w.writeFloat64(_x1);
  w.writeFloat64(_x2);
  w.writeFloat64(_x3);
  }

  void VectorLorentz::decodeBinary(BinaryReader & r, const String & /*name*/, unsigned int /*version*/)
  {
  _x0 = r.readFloat64();
  _x1 = r.readFloat64();
  _x2 = r.readFloat64();
  _x3 = r.readFloat64();
  }

  void VectorLorentz::writeBinary(std::ostream & out) const                     { writeBinaryObject(out,*this); }
  bool VectorLorentz::readBinary(std::istream & in)                             { return readBinaryObject(in,*this); }
  void VectorLorentz::saveBinary(const String & fileName, bool append) const    { saveBinaryObject(fileName,*this,append); }
  void VectorLorentz::loadBinary(const String & fileName, unsigned int index)    { loadBinaryObjectAt(fileName,*this,index); }
  void VectorLorentz::saveBinary(const String & fileName, const std::vector<VectorLorentz> & objects) { saveBinaryObjects(fileName,objects); }
  std::vector<VectorLorentz> VectorLorentz::loadAllBinary(const String & fileName)       { return loadAllBinaryObjects<VectorLorentz>(fileName); }

} // namespace MATH
} // namespace CAP


