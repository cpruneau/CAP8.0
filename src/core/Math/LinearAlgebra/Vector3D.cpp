#include "Vector3D.hpp"
#include <iostream>
#include "MathExceptions.hpp"
#include "MathConstants.hpp"



namespace CAP
{
namespace MATH
{
Vector3D::Vector3D()
:
_x1(0),
_x2(0),
_x3(0)
{ }

Vector3D::Vector3D(const double & x1, const double & x2, const double & x3)
:
_x1(x1),
_x2(x2),
_x3(x3)
{  }

Vector3D::Vector3D(const float values[3])
:
_x1(values[0]),
_x2(values[1]),
_x3(values[2])
{  }

Vector3D::Vector3D(const double values[3])
:
_x1(values[0]),
_x2(values[1]),
_x3(values[2])
{  }

Vector3D::Vector3D(const std::vector<float> & values)
:
_x1(0),
_x2(0),
_x3(0)
{
  setXYZ(values);
}

Vector3D::Vector3D(const std::vector<double> & values)
:
_x1(0),
_x2(0),
_x3(0)
{
  setXYZ(values);
}

Vector3D::Vector3D(const Vector3D & source)
:
_x1(source._x1),
_x2(source._x2),
_x3(source._x3)
{  }


//!
//! Returns the component stored at the given index: 0 for x, 1 for y, 2 for z.
//!
double Vector3D::operator[] (unsigned int index) const
{
  switch (index)
    {
      case 0: return _x1;
      case 1: return _x2;
      case 2: return _x3;
      default: throw MathException("index must be 0, 1 or 2",__FUNCTION__);
    }
}

void Vector3D::setXYZ(const double & x1, const double & x2, const double & x3)
{
  _x1 = x1;
  _x2 = x2;
  _x3 = x3;
}

void Vector3D::setXYZ(const float values[3])
{
  _x1 = values[0];
  _x2 = values[1];
  _x3 = values[2];
}

void Vector3D::setXYZ(const double values[3])
{
  _x1 = values[0];
  _x2 = values[1];
  _x3 = values[2];
}

void Vector3D::setXYZ(const std::vector<float> & values)
{
  if (values.size()<3) throw MathException("values must contain at least 3 elements",__FUNCTION__);
  _x1 = values[0];
  _x2 = values[1];
  _x3 = values[2];
}

void Vector3D::setXYZ(const std::vector<double> & values)
{
  if (values.size()<3) throw MathException("values must contain at least 3 elements",__FUNCTION__);
  _x1 = values[0];
  _x2 = values[1];
  _x3 = values[2];
}

void Vector3D::setXYZ(const Vector3D & source)
{
  _x1 = source._x1;
  _x2 = source._x2;
  _x3 = source._x3;
}

void Vector3D::setPhiThetaR(const double & phi, const double & theta, const double & r)
{
  double rSinTheta = r*std::sin(theta);
  _x1 = rSinTheta*std::cos(phi);
  _x2 = rSinTheta*std::sin(phi);
  _x3 = r*std::cos(theta);
}


void Vector3D::setPhiRhoZ(const double & phi, const double & rho, const double & z)
{
  _x1 = rho*std::cos(phi);
  _x2 = rho*std::sin(phi);
  _x3 = z;
}


//!
//! Return the azimuthal angle between this and the other vector, phi - other.phi,
//! mapped onto the range [0, 2pi).
double  Vector3D::deltaPhi(const Vector3D & other) const
{
  double dPhi = std::atan2(_x2,_x1) - std::atan2(other._x2,other._x1);
  while (dPhi>=twoPi()) dPhi -= twoPi();
  while (dPhi<0) dPhi += twoPi();
  return dPhi;
}

Vector3D & Vector3D::operator= (const Vector3D & rhs)
{
  if (this!=&rhs)
    {
    _x1 = rhs._x1;
    _x2 = rhs._x2;
    _x3 = rhs._x3;
    }
  return *this;
}

bool  Vector3D::operator== (const Vector3D & rhs) const
{
  return (_x1==rhs._x1)  && 
  (_x2==rhs._x2)  && 
  (_x3==rhs._x3);
}

bool  Vector3D::operator!= (const Vector3D & rhs) const
{
  return (_x1!=rhs._x1) ||
  (_x2!=rhs._x2) ||
  (_x3!=rhs._x3);
}

Vector3D & Vector3D::operator+= (const Vector3D & rhs)
{
  _x1 += rhs._x1;
  _x2 += rhs._x2;
  _x3 += rhs._x3;
  return *this;
}

Vector3D & Vector3D::operator-= (const Vector3D & rhs)
{
  _x1 -= rhs._x1;
  _x2 -= rhs._x2;
  _x3 -= rhs._x3;
  return *this;
}

//! Unary minus.
Vector3D Vector3D::operator- () const
{
  return Vector3D(-_x1,-_x2,-_x3);
}

double Vector3D::distanceX(const Vector3D & other) const
{
  return _x1 - other._x1;
}

double Vector3D::distanceY(const Vector3D & other) const
{
  return _x2 - other._x2;
}

double Vector3D::distanceZ(const Vector3D & other) const
{
  return _x3 - other._x3;
}

double Vector3D::distanceSq(const Vector3D & other) const
{
  double d_x1 = _x1 - other._x1;
  double d_x2 = _x2 - other._x2;
  double d_x3 = _x3 - other._x3;
  return d_x1*d_x1+d_x2*d_x2+d_x3*d_x3;
}

double Vector3D::distance(const Vector3D & other) const
{
  double d_x1 = _x1 - other._x1;
  double d_x2 = _x2 - other._x2;
  double d_x3 = _x3 - other._x3;
  return std::sqrt(d_x1*d_x1+d_x2*d_x2+d_x3*d_x3);
}

//! produce a 3D unit vector in the direction of this vector
Vector3D Vector3D::unit() const
{
  double r = std::sqrt(_x1*_x1+_x2*_x2+_x3*_x3);
  if (r==0) throw MathException("Cannot produce unit vector from null vector",__FUNCTION__);
  return Vector3D(_x1/r,_x2/r,_x3/r);
}

//! Active rotation of this Vector3D by the given angle relative to the x-axis
void Vector3D::rotateX(const double & angle)
{
  double s = std::sin(angle);
  double c = std::cos(angle);
  double y = _x2;
  double z = _x3;
  _x2 = c*y - s*z;
  _x3 = s*y + c*z;
}

//! Active rotation of this Vector3D by the given angle relative to the y-axis
void Vector3D::rotateY(const double & angle)
{
  double s = std::sin(angle);
  double c = std::cos(angle);
  double x = _x1;
  double z = _x3;
  _x1 = c*x + s*z;
  _x3 = c*z - s*x;
}

//! Active rotation of this Vector3D by the given angle relative to the z-axis
void Vector3D::rotateZ(const double & angle)
{
  double s = std::sin(angle);
  double c = std::cos(angle);
  double x = _x1;
  double y = _x2;
  _x1 = c*x - s*y;
  _x2 = c*y + s*x;
}

void Vector3D::print() const
{
  std::cout << "(" << _x1 << "," << _x2 << "," << _x3 << ") " << std::endl;
}

//!
//! Add the two vectors and return a new vector
//!
Vector3D operator+ (const Vector3D & left, const Vector3D & right)
{
  return Vector3D(left._x1+right._x1, left._x2+right._x2, left._x3+right._x3);
}

Vector3D operator- (const Vector3D & left, const Vector3D & right)
{
  return Vector3D(left._x1-right._x1, left._x2-right._x2, left._x3-right._x3);
}

//!
//! SR (3D) scalar product of two left and right vectors
//!
double operator* (const Vector3D & left, const Vector3D & right)
  {
  return left._x1*right._x1 + left._x2*right._x2 + left._x3*right._x3;
  }


//!
//! Scaling of the left vector by a scalar value from the right
//!
Vector3D operator* (const Vector3D & v, double a)
  {
  return Vector3D(a*v._x1, a*v._x2, a*v._x3);
  }

//!
//! Scaling of the right vector by a scalar value from the left
//!
Vector3D operator* (double a, const Vector3D & v)
  {
  return Vector3D(a*v._x1, a*v._x2, a*v._x3);
  }

//!
//! Print out the given vector to the given ostream.
//!
std::ostream& operator<<(std::ostream& out, const Vector3D & v)
{
  out << "(" << v.x() << "," << v.y() << "," << v.z() << ")" ;
  return out;
}

  void Vector3D::encodeBinary(BinaryWriter & w) const
  {
  w.writeFloat64(_x1);
  w.writeFloat64(_x2);
  w.writeFloat64(_x3);
  }

  void Vector3D::decodeBinary(BinaryReader & r, const String & /*name*/, unsigned int /*version*/)
  {
  _x1 = r.readFloat64();
  _x2 = r.readFloat64();
  _x3 = r.readFloat64();
  }

  void Vector3D::writeBinary(std::ostream & out) const                     { writeBinaryObject(out,*this); }
  bool Vector3D::readBinary(std::istream & in)                             { return readBinaryObject(in,*this); }
  void Vector3D::saveBinary(const String & fileName, bool append) const    { saveBinaryObject(fileName,*this,append); }
  void Vector3D::loadBinary(const String & fileName, unsigned int index)    { loadBinaryObjectAt(fileName,*this,index); }
  void Vector3D::saveBinary(const String & fileName, const std::vector<Vector3D> & objects) { saveBinaryObjects(fileName,objects); }
  std::vector<Vector3D> Vector3D::loadAllBinary(const String & fileName)       { return loadAllBinaryObjects<Vector3D>(fileName); }

} // namespace MATH
} // namespace CAP


