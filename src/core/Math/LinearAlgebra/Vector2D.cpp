#include "Vector2D.hpp"
#include "MathExceptions.hpp"
#include "MathConstants.hpp"
#include <iostream>



namespace CAP
{
namespace MATH
{
Vector2D::Vector2D()
:
_x1(0),
_x2(0)
{ }

Vector2D::Vector2D(const double & x1, const double & x2)
:
_x1(x1),
_x2(x2)
{  }

Vector2D::Vector2D(const float values[2])
:
_x1(values[0]),
_x2(values[1])
{  }

Vector2D::Vector2D(const double values[2])
:
_x1(values[0]),
_x2(values[1])
{  }

Vector2D::Vector2D(const std::vector<float> & values)
:
_x1(0),
_x2(0)
{
  setXY(values);
}

Vector2D::Vector2D(const std::vector<double> & values)
:
_x1(0),
_x2(0)
{
  setXY(values);
}

Vector2D::Vector2D(const Vector2D & source)
:
_x1(source._x1),
_x2(source._x2)
{  }

// Component at the given index: 0 for x, 1 for y.
double Vector2D::operator[] (unsigned int index) const
{
  switch (index)
    {
      case 0: return _x1;
      case 1: return _x2;
      default: throw MathException("index must be 0 or 1",__FUNCTION__);
    }
}

void Vector2D::setXY(const double & x1, const double & x2)
{
  _x1 = x1;
  _x2 = x2;
}

void Vector2D::setXY(const float values[2])
{
  _x1 = values[0];
  _x2 = values[1];
}

void Vector2D::setXY(const double values[2])
{
  _x1 = values[0];
  _x2 = values[1];
}

void Vector2D::setXY(const std::vector<float> & values)
{
  if (values.size()<2) throw MathException("values must contain at least 2 elements",__FUNCTION__);
  _x1 = values[0];
  _x2 = values[1];
}

void Vector2D::setXY(const std::vector<double> & values)
{
  if (values.size()<2) throw MathException("values must contain at least 2 elements",__FUNCTION__);
  _x1 = values[0];
  _x2 = values[1];
}

void Vector2D::setXY(const Vector2D & source)
{
  _x1 = source._x1;
  _x2 = source._x2;
}

void Vector2D::setPhiR(const double & phi, const double & r)
{
  _x1 = r*std::cos(phi);
  _x2 = r*std::sin(phi);
}

// Azimuthal angle between this and the other vector, phi - other.phi, mapped onto [0, 2pi).
double  Vector2D::deltaPhi(const Vector2D & other) const
{
  double dPhi = std::atan2(_x2,_x1) - std::atan2(other._x2,other._x1);
  while (dPhi>=twoPi()) dPhi -= twoPi();
  while (dPhi<0) dPhi += twoPi();
  return dPhi;
}

Vector2D & Vector2D::operator= (const Vector2D & rhs)
{
  if (this!=&rhs)
    {
    _x1 = rhs._x1;
    _x2 = rhs._x2;
    }
  return *this;
}

bool  Vector2D::operator== (const Vector2D & rhs) const
{
  return (_x1==rhs._x1) && (_x2==rhs._x2);
}

bool  Vector2D::operator!= (const Vector2D & rhs) const
{
  return (_x1!=rhs._x1) || (_x2!=rhs._x2);
}

Vector2D & Vector2D::operator+= (const Vector2D & rhs)
{
  _x1 += rhs._x1;
  _x2 += rhs._x2;
  return *this;
}

Vector2D & Vector2D::operator-= (const Vector2D & rhs)
{
  _x1 -= rhs._x1;
  _x2 -= rhs._x2;
  return *this;
}

Vector2D Vector2D::operator- () const
{
  return Vector2D(-_x1,-_x2);
}

double Vector2D::distanceX(const Vector2D & other) const
{
  return _x1 - other._x1;
}

double Vector2D::distanceY(const Vector2D & other) const
{
  return _x2 - other._x2;
}

double Vector2D::distanceSq(const Vector2D & other) const
{
  double d_x1 = _x1 - other._x1;
  double d_x2 = _x2 - other._x2;
  return d_x1*d_x1+d_x2*d_x2;
}

double Vector2D::distance(const Vector2D & other) const
{
  return std::sqrt(distanceSq(other));
}

Vector2D Vector2D::unit() const
{
  double r = std::sqrt(_x1*_x1+_x2*_x2);
  if (r==0) throw MathException("Cannot produce unit vector from null vector",__FUNCTION__);
  return Vector2D(_x1/r,_x2/r);
}

// Active (counter-clockwise) rotation of this vector by the given angle.
void Vector2D::rotate(const double & angle)
{
  double s = std::sin(angle);
  double c = std::cos(angle);
  double x = _x1;
  double y = _x2;
  _x1 = c*x - s*y;
  _x2 = s*x + c*y;
}

void Vector2D::print() const
{
  std::cout << "(" << _x1 << "," << _x2 << ") " << std::endl;
}

Vector2D operator+ (const Vector2D & left, const Vector2D & right)
{
  return Vector2D(left._x1+right._x1, left._x2+right._x2);
}

Vector2D operator- (const Vector2D & left, const Vector2D & right)
{
  return Vector2D(left._x1-right._x1, left._x2-right._x2);
}

double operator* (const Vector2D & left, const Vector2D & right)
{
  return left._x1*right._x1 + left._x2*right._x2;
}

Vector2D operator* (const Vector2D & v, double a)
{
  return Vector2D(a*v._x1, a*v._x2);
}

Vector2D operator* (double a, const Vector2D & v)
{
  return Vector2D(a*v._x1, a*v._x2);
}

std::ostream& operator<<(std::ostream& out, const Vector2D & v)
{
  out << "(" << v.x() << "," << v.y() << ")" ;
  return out;
}

  void Vector2D::encodeBinary(BinaryWriter & w) const
  {
  w.writeFloat64(_x1);
  w.writeFloat64(_x2);
  }

  void Vector2D::decodeBinary(BinaryReader & r, const String & /*name*/, unsigned int /*version*/)
  {
  _x1 = r.readFloat64();
  _x2 = r.readFloat64();
  }

  void Vector2D::writeBinary(std::ostream & out) const                     { writeBinaryObject(out,*this); }
  bool Vector2D::readBinary(std::istream & in)                             { return readBinaryObject(in,*this); }
  void Vector2D::saveBinary(const String & fileName, bool append) const    { saveBinaryObject(fileName,*this,append); }
  void Vector2D::loadBinary(const String & fileName, unsigned int index)    { loadBinaryObjectAt(fileName,*this,index); }
  void Vector2D::saveBinary(const String & fileName, const std::vector<Vector2D> & objects) { saveBinaryObjects(fileName,objects); }
  std::vector<Vector2D> Vector2D::loadAllBinary(const String & fileName)       { return loadAllBinaryObjects<Vector2D>(fileName); }

} // namespace MATH
} // namespace CAP
