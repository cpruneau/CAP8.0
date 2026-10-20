#include "VectorND.hpp"
#include "MathExceptions.hpp"
#include <iostream>

ClassImp(CAP::MATH::VectorND);


namespace CAP
{
namespace MATH
{

VectorND::VectorND()
:
_x()
{ }

VectorND::VectorND(unsigned int n, double value)
:
_x(n,value)
{ }

VectorND::VectorND(const float values[], unsigned int n)
:
_x(values, values+n)
{ }

VectorND::VectorND(const double values[], unsigned int n)
:
_x(values, values+n)
{ }

VectorND::VectorND(const std::vector<float> & values)
:
_x(values.begin(), values.end())
{ }

VectorND::VectorND(const std::vector<double> & values)
:
_x(values)
{ }

VectorND::VectorND(const VectorND & source)
:
_x(source._x)
{ }

void VectorND::checkDimension(const VectorND & other, const char * source) const
{
  if (_x.size()!=other._x.size())
    throw MathException("Vectors have different dimensions",source);
}

double VectorND::operator[] (unsigned int index) const
{
  if (index>=_x.size()) throw MathException("index out of range (index>=dimension)",__FUNCTION__);
  return _x[index];
}

double & VectorND::operator[] (unsigned int index)
{
  if (index>=_x.size()) throw MathException("index out of range (index>=dimension)",__FUNCTION__);
  return _x[index];
}

void VectorND::setValue(unsigned int index, double value)
{
  if (index>=_x.size()) throw MathException("index out of range (index>=dimension)",__FUNCTION__);
  _x[index] = value;
}

void VectorND::setValues(const std::vector<float> & values)
{
  _x.assign(values.begin(),values.end());
}

void VectorND::setValues(const std::vector<double> & values)
{
  _x = values;
}

void VectorND::setValues(const float values[], unsigned int n)
{
  _x.assign(values,values+n);
}

void VectorND::setValues(const double values[], unsigned int n)
{
  _x.assign(values,values+n);
}

void VectorND::setValues(const VectorND & source)
{
  _x = source._x;
}

void VectorND::fill(double value)
{
  for (auto & v : _x) v = value;
}

void VectorND::resize(unsigned int n, double value)
{
  _x.resize(n,value);
}

double VectorND::modulusSquare() const
{
  double sum = 0.0;
  for (auto v : _x) sum += v*v;
  return sum;
}

double VectorND::modulus() const
{
  return std::sqrt(modulusSquare());
}

double VectorND::scalarProduct(const VectorND & rhs) const
{
  checkDimension(rhs,__FUNCTION__);
  double sum = 0.0;
  for (unsigned int k=0; k<_x.size(); k++) sum += _x[k]*rhs._x[k];
  return sum;
}

VectorND & VectorND::operator= (const VectorND & rhs)
{
  if (this!=&rhs) _x = rhs._x;
  return *this;
}

bool  VectorND::operator== (const VectorND & rhs) const
{
  return _x==rhs._x;
}

bool  VectorND::operator!= (const VectorND & rhs) const
{
  return _x!=rhs._x;
}

VectorND & VectorND::operator+= (const VectorND & rhs)
{
  checkDimension(rhs,__FUNCTION__);
  for (unsigned int k=0; k<_x.size(); k++) _x[k] += rhs._x[k];
  return *this;
}

VectorND & VectorND::operator-= (const VectorND & rhs)
{
  checkDimension(rhs,__FUNCTION__);
  for (unsigned int k=0; k<_x.size(); k++) _x[k] -= rhs._x[k];
  return *this;
}

VectorND & VectorND::operator*= (double a)
{
  for (auto & v : _x) v *= a;
  return *this;
}

VectorND & VectorND::operator/= (double a)
{
  if (a==0.0) throw MathException("Attempting division by zero",__FUNCTION__);
  for (auto & v : _x) v /= a;
  return *this;
}

VectorND VectorND::operator- () const
{
  VectorND result(*this);
  for (auto & v : result._x) v = -v;
  return result;
}

double VectorND::distanceSq(const VectorND & other) const
{
  checkDimension(other,__FUNCTION__);
  double sum = 0.0;
  for (unsigned int k=0; k<_x.size(); k++)
    {
    double d = _x[k] - other._x[k];
    sum += d*d;
    }
  return sum;
}

double VectorND::distance(const VectorND & other) const
{
  return std::sqrt(distanceSq(other));
}

VectorND VectorND::unit() const
{
  double r = modulus();
  if (r==0) throw MathException("Cannot produce unit vector from null vector",__FUNCTION__);
  VectorND result(*this);
  result /= r;
  return result;
}

void VectorND::print() const
{
  std::cout << *this << " " << std::endl;
}

VectorND operator+ (const VectorND & left, const VectorND & right)
{
  VectorND result(left);
  result += right;
  return result;
}

VectorND operator- (const VectorND & left, const VectorND & right)
{
  VectorND result(left);
  result -= right;
  return result;
}

double operator* (const VectorND & left, const VectorND & right)
{
  return left.scalarProduct(right);
}

VectorND operator* (const VectorND & v, double a)
{
  VectorND result(v);
  result *= a;
  return result;
}

VectorND operator* (double a, const VectorND & v)
{
  VectorND result(v);
  result *= a;
  return result;
}

std::ostream& operator<<(std::ostream& out, const VectorND & v)
{
  out << "(";
  for (unsigned int k=0; k<v._x.size(); k++)
    {
    if (k>0) out << ",";
    out << v._x[k];
    }
  out << ")";
  return out;
}
  void VectorND::encodeBinary(BinaryWriter & w) const
  {
  writeDoubles(w,_x);
  }

  void VectorND::decodeBinary(BinaryReader & r, const String & /*name*/, unsigned int /*version*/)
  {
  _x = readDoubles(r);
  }

  void VectorND::writeBinary(std::ostream & out) const                     { writeBinaryObject(out,*this); }
  bool VectorND::readBinary(std::istream & in)                             { return readBinaryObject(in,*this); }
  void VectorND::saveBinary(const String & fileName, bool append) const    { saveBinaryObject(fileName,*this,append); }
  void VectorND::loadBinary(const String & fileName, unsigned int index)    { loadBinaryObjectAt(fileName,*this,index); }
  void VectorND::saveBinary(const String & fileName, const std::vector<VectorND> & objects) { saveBinaryObjects(fileName,objects); }
  std::vector<VectorND> VectorND::loadAllBinary(const String & fileName)       { return loadAllBinaryObjects<VectorND>(fileName); }


} // namespace MATH
} // namespace CAP
