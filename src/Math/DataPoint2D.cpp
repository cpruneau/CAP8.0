#include "DataPoint2D.hpp"
#include "MathExceptions.hpp"
#include <algorithm>
#include <iostream>

ClassImp(CAP::MATH::DataPoint2D);

namespace CAP
{
  namespace MATH
  {

  namespace
  {
  // Relative tolerance used for symmetry and compatibility checks.
  constexpr double kRelTolerance = 1.0E-6;

  // Propagate a two-operand operation f(y1,y2) whose partial derivatives are d1 and d2.
  // Points are assumed independent. The result has the mean x and the mean x uncertainty.
  void combine(float & x, float & xError, float & y, float & yStat, float & ySys, float & cov,
               const DataPoint2D & p1, const DataPoint2D & p2,
               double value, double d1, double d2)
  {
  x      = 0.5*(p1.x() + p2.x());
  xError = 0.5*(p1.xError() + p2.xError());
  y      = value;
  double s1 = d1*p1.yStatError(), s2 = d2*p2.yStatError();
  double t1 = d1*p1.ySysError(),  t2 = d2*p2.ySysError();
  yStat  = std::sqrt(s1*s1 + s2*s2);
  ySys   = std::sqrt(t1*t1 + t2*t2);
  cov    = 0.5*(d1*p1.xyCovariance() + d2*p2.xyCovariance());
  }
  }

  //////////////////////////////////////////////////////////////////////////
  // Construction / assignment
  //////////////////////////////////////////////////////////////////////////

  DataPoint2D::DataPoint2D()
  :
  _x(0.0), _xError(0.0), _y(0.0), _yStatError(0.0), _ySysError(0.0), _xyCovariance(0.0)
  { }

  DataPoint2D::DataPoint2D(float x, float y)
  :
  _x(x), _xError(0.0), _y(y), _yStatError(0.0), _ySysError(0.0), _xyCovariance(0.0)
  { }

  DataPoint2D::DataPoint2D(float x, float y, float yStatError)
  :
  _x(x), _xError(0.0), _y(y), _yStatError(yStatError), _ySysError(0.0), _xyCovariance(0.0)
  { }

  DataPoint2D::DataPoint2D(float x, float y, float yStatError, float ySysError)
  :
  _x(x), _xError(0.0), _y(y), _yStatError(yStatError), _ySysError(ySysError), _xyCovariance(0.0)
  { }

  DataPoint2D::DataPoint2D(float x, float xError, float y, float yStatError, float ySysError)
  :
  _x(x), _xError(xError), _y(y), _yStatError(yStatError), _ySysError(ySysError), _xyCovariance(0.0)
  { }

  DataPoint2D::DataPoint2D(const DataPoint2D & src)
  :
  _x(src._x), _xError(src._xError), _y(src._y),
  _yStatError(src._yStatError), _ySysError(src._ySysError), _xyCovariance(src._xyCovariance)
  { }

  DataPoint2D & DataPoint2D::operator= (const DataPoint2D & rhs)
  {
  if (this!=&rhs)
    {
    _x            = rhs._x;
    _xError       = rhs._xError;
    _y            = rhs._y;
    _yStatError   = rhs._yStatError;
    _ySysError    = rhs._ySysError;
    _xyCovariance = rhs._xyCovariance;
    }
  return *this;
  }

  //////////////////////////////////////////////////////////////////////////
  // Values and uncertainties
  //////////////////////////////////////////////////////////////////////////

  float DataPoint2D::correlation() const
  {
  double denominator = double(_xError)*double(_yStatError);
  if (denominator<=0.0) return 0.0;
  return _xyCovariance/denominator;
  }

  void DataPoint2D::set(float x, float xError, float y, float yStatError, float ySysError)
  {
  _x            = x;
  _xError       = xError;
  _y            = y;
  _yStatError   = yStatError;
  _ySysError    = ySysError;
  _xyCovariance = 0.0;
  }

  void DataPoint2D::setPoint(double x, double y)
  {
  set(x,0.0,y,0.0,0.0);
  }

  void DataPoint2D::setPoint(double x, double y, double yStatError)
  {
  set(x,0.0,y,yStatError,0.0);
  }

  void DataPoint2D::setPoint(double x, double y, double yStatError, double ySysError)
  {
  set(x,0.0,y,yStatError,ySysError);
  }

  void DataPoint2D::setXError(double xError)
  {
  if (xError<0) throw MathException("xError<0",__FUNCTION__);
  _xError = xError;
  }

  void DataPoint2D::setYStatError(double yStatError)
  {
  if (yStatError<0) throw MathException("yStatError<0",__FUNCTION__);
  _yStatError = yStatError;
  }

  void DataPoint2D::setYSysError(double ySysError)
  {
  if (ySysError<0) throw MathException("ySysError<0",__FUNCTION__);
  _ySysError = ySysError;
  }

  //////////////////////////////////////////////////////////////////////////
  // Error matrix
  //////////////////////////////////////////////////////////////////////////

  void DataPoint2D::setErrorMatrix(double varX, double covXY, double varY)
  {
  if (varX<0) throw MathException("Error matrix: var(x)<0",__FUNCTION__);
  if (varY<0) throw MathException("Error matrix: var(y)<0",__FUNCTION__);
  if (covXY*covXY > varX*varY*(1.0+kRelTolerance))
    throw MathException("Error matrix is not positive semi-definite: cov(x,y)^2 > var(x)*var(y)",__FUNCTION__);
  _xError       = std::sqrt(varX);
  _yStatError   = std::sqrt(varY);
  _xyCovariance = covXY;
  }

  void DataPoint2D::setErrorMatrix(const double matrix[2][2])
  {
  double m01 = matrix[0][1];
  double m10 = matrix[1][0];
  double scale = std::max(std::fabs(m01),std::fabs(m10));
  if (std::fabs(m01-m10) > kRelTolerance*scale)
    throw MathException("Error matrix is not symmetric: matrix[0][1]!=matrix[1][0]",__FUNCTION__);
  setErrorMatrix(matrix[0][0], 0.5*(m01+m10), matrix[1][1]);
  }

  void DataPoint2D::setErrorMatrix(const std::vector< std::vector<double> > & matrix)
  {
  if (matrix.size()!=2 || matrix[0].size()!=2 || matrix[1].size()!=2)
    throw MathException("Error matrix must be 2 by 2",__FUNCTION__);
  double m[2][2] = { {matrix[0][0], matrix[0][1]}, {matrix[1][0], matrix[1][1]} };
  setErrorMatrix(m);
  }

  void DataPoint2D::setErrors(double xError, double yStatError, double correlation)
  {
  if (xError<0)     throw MathException("xError<0",__FUNCTION__);
  if (yStatError<0) throw MathException("yStatError<0",__FUNCTION__);
  if (std::fabs(correlation)>1.0) throw MathException("|correlation|>1",__FUNCTION__);
  _xError       = xError;
  _yStatError   = yStatError;
  _xyCovariance = correlation*xError*yStatError;
  }

  void DataPoint2D::errorMatrix(double matrix[2][2]) const
  {
  matrix[0][0] = double(_xError)*_xError;
  matrix[0][1] = _xyCovariance;
  matrix[1][0] = _xyCovariance;
  matrix[1][1] = double(_yStatError)*_yStatError;
  }

  std::vector< std::vector<double> > DataPoint2D::errorMatrix() const
  {
  double m[2][2];
  errorMatrix(m);
  return { {m[0][0], m[0][1]}, {m[1][0], m[1][1]} };
  }

  //////////////////////////////////////////////////////////////////////////
  // Comparison
  //////////////////////////////////////////////////////////////////////////

  bool DataPoint2D::compatibleX(const DataPoint2D & other, double nSigma) const
  {
  double dx    = std::fabs(double(_x) - other._x);
  double error = std::sqrt(double(_xError)*_xError + double(other._xError)*other._xError);
  double tol   = kRelTolerance*std::max(std::fabs(double(_x)),std::fabs(double(other._x)));
  return dx <= nSigma*error + tol;
  }

  bool  DataPoint2D::operator== (const DataPoint2D & rhs) const
  {
  if (!compatibleX(rhs)) return false;
  double e1    = yError();
  double e2    = rhs.yError();
  double dy    = std::fabs(double(_y) - rhs._y);
  double tol   = kRelTolerance*std::max(std::fabs(double(_y)),std::fabs(double(rhs._y)));
  return dy <= 2.0*std::sqrt(e1*e1+e2*e2) + tol;
  }

  bool  DataPoint2D::operator!= (const DataPoint2D & rhs) const
  {
  return !operator==(rhs);
  }

  //////////////////////////////////////////////////////////////////////////
  // Arithmetic
  //////////////////////////////////////////////////////////////////////////

  DataPoint2D & DataPoint2D::operator+= (const DataPoint2D & rhs)
  {
  if (!compatibleX(rhs)) throw MathException("Operation on DataPoint2D with incompatible x values",__FUNCTION__);
  DataPoint2D p1(*this);
  combine(_x,_xError,_y,_yStatError,_ySysError,_xyCovariance,p1,rhs, double(p1._y)+rhs._y, 1.0, 1.0);
  return *this;
  }

  DataPoint2D & DataPoint2D::operator-= (const DataPoint2D & rhs)
  {
  if (!compatibleX(rhs)) throw MathException("Operation on DataPoint2D with incompatible x values",__FUNCTION__);
  DataPoint2D p1(*this);
  combine(_x,_xError,_y,_yStatError,_ySysError,_xyCovariance,p1,rhs, double(p1._y)-rhs._y, 1.0, -1.0);
  return *this;
  }

  DataPoint2D & DataPoint2D::operator*= (const DataPoint2D & rhs)
  {
  if (!compatibleX(rhs)) throw MathException("Operation on DataPoint2D with incompatible x values",__FUNCTION__);
  DataPoint2D p1(*this);
  double y1 = p1._y, y2 = rhs._y;
  combine(_x,_xError,_y,_yStatError,_ySysError,_xyCovariance,p1,rhs, y1*y2, y2, y1);
  return *this;
  }

  DataPoint2D & DataPoint2D::operator/= (const DataPoint2D & rhs)
  {
  if (!compatibleX(rhs)) throw MathException("Operation on DataPoint2D with incompatible x values",__FUNCTION__);
  if (rhs._y==0) throw MathException("Attempting division by a point with y==0",__FUNCTION__);
  DataPoint2D p1(*this);
  double y1 = p1._y, y2 = rhs._y;
  combine(_x,_xError,_y,_yStatError,_ySysError,_xyCovariance,p1,rhs, y1/y2, 1.0/y2, -y1/(y2*y2));
  return *this;
  }

  DataPoint2D & DataPoint2D::operator+= (double rhs)
  {
  _y += rhs;
  return *this;
  }

  DataPoint2D & DataPoint2D::operator-= (double rhs)
  {
  _y -= rhs;
  return *this;
  }

  DataPoint2D & DataPoint2D::operator*= (double rhs)
  {
  scale(rhs);
  return *this;
  }

  DataPoint2D & DataPoint2D::operator/= (double rhs)
  {
  if (rhs==0) throw MathException("Attempting division by zero",__FUNCTION__);
  scale(1.0/rhs);
  return *this;
  }

  void DataPoint2D::scale(double coefficient)
  {
  double a = std::fabs(coefficient);
  _y            *= coefficient;
  _yStatError   *= a;
  _ySysError    *= a;
  _xyCovariance *= coefficient;
  }

  void DataPoint2D::inverse()
  {
  if (_y==0) throw MathException("Cannot invert a point with y==0",__FUNCTION__);
  double y   = _y;
  double ySq = y*y;
  _y            = 1.0/y;
  _yStatError   = _yStatError/ySq;
  _ySysError    = _ySysError/ySq;
  _xyCovariance = -_xyCovariance/ySq;
  }

  void DataPoint2D::setAsSumOf(const DataPoint2D & p1, const DataPoint2D & p2,
                               double coefficient1, double coefficient2)
  {
  DataPoint2D a(p1), b(p2);
  a.scale(coefficient1);
  b.scale(coefficient2);
  a += b;
  *this = a;
  }

  void DataPoint2D::setAsSumOf(const std::vector<const DataPoint2D *> & points,
                               const std::vector<double> & coefficients)
  {
  if (points.empty()) throw MathException("No points given",__FUNCTION__);
  if (points.size()!=coefficients.size()) throw MathException("points and coefficients sizes differ",__FUNCTION__);
  for (auto p : points) if (!p) throw MathException("Null point pointer",__FUNCTION__);
  DataPoint2D result(*points[0]);
  result.scale(coefficients[0]);
  for (unsigned int k=1; k<points.size(); k++)
    {
    DataPoint2D term(*points[k]);
    term.scale(coefficients[k]);
    result += term;
    }
  *this = result;
  }

  void DataPoint2D::setAsProductOf(const DataPoint2D & p1, const DataPoint2D & p2,
                                   double coefficient1, double coefficient2)
  {
  DataPoint2D a(p1), b(p2);
  a.scale(coefficient1);
  b.scale(coefficient2);
  a *= b;
  *this = a;
  }

  void DataPoint2D::setAsProductOf(const std::vector<const DataPoint2D *> & points,
                                   const std::vector<double> & coefficients)
  {
  if (points.empty()) throw MathException("No points given",__FUNCTION__);
  if (points.size()!=coefficients.size()) throw MathException("points and coefficients sizes differ",__FUNCTION__);
  for (auto p : points) if (!p) throw MathException("Null point pointer",__FUNCTION__);
  DataPoint2D result(*points[0]);
  result.scale(coefficients[0]);
  for (unsigned int k=1; k<points.size(); k++)
    {
    DataPoint2D factor(*points[k]);
    factor.scale(coefficients[k]);
    result *= factor;
    }
  *this = result;
  }

  void DataPoint2D::setAsRatioOf(const DataPoint2D & p1, const DataPoint2D & p2,
                                 double coefficient1, double coefficient2)
  {
  DataPoint2D a(p1), b(p2);
  a.scale(coefficient1);
  b.scale(coefficient2);
  a /= b;
  *this = a;
  }

  void DataPoint2D::clear()
  {
  set(0.0,0.0,0.0,0.0,0.0);
  }

  void DataPoint2D::reset()
  {
  clear();
  }

  void DataPoint2D::print() const
  {
  std::cout << *this << std::endl;
  }

  //////////////////////////////////////////////////////////////////////////
  // Free operators
  //////////////////////////////////////////////////////////////////////////

  DataPoint2D operator+(const DataPoint2D & left, const DataPoint2D & right) { DataPoint2D r(left); r += right; return r; }
  DataPoint2D operator-(const DataPoint2D & left, const DataPoint2D & right) { DataPoint2D r(left); r -= right; return r; }
  DataPoint2D operator*(const DataPoint2D & left, const DataPoint2D & right) { DataPoint2D r(left); r *= right; return r; }
  DataPoint2D operator/(const DataPoint2D & left, const DataPoint2D & right) { DataPoint2D r(left); r /= right; return r; }

  DataPoint2D operator+(const DataPoint2D & left, double right) { DataPoint2D r(left);  r += right; return r; }
  DataPoint2D operator+(double left, const DataPoint2D & right) { DataPoint2D r(right); r += left;  return r; }
  DataPoint2D operator-(const DataPoint2D & left, double right) { DataPoint2D r(left);  r -= right; return r; }
  DataPoint2D operator-(double left, const DataPoint2D & right) { DataPoint2D r(right); r.scale(-1.0); r += left; return r; }
  DataPoint2D operator*(const DataPoint2D & left, double right) { DataPoint2D r(left);  r.scale(right); return r; }
  DataPoint2D operator*(double left, const DataPoint2D & right) { DataPoint2D r(right); r.scale(left);  return r; }
  DataPoint2D operator/(const DataPoint2D & left, double right) { DataPoint2D r(left);  r /= right; return r; }
  DataPoint2D operator/(double left, const DataPoint2D & right) { DataPoint2D r(right); r.inverse(); r.scale(left); return r; }

  DataPoint2D operator+(const DataPoint2D & right) { return DataPoint2D(right); }
  DataPoint2D operator-(const DataPoint2D & right) { DataPoint2D r(right); r.scale(-1.0); return r; }

  std::ostream& operator<<(std::ostream& out, const DataPoint2D & p)
  {
  out << "x: " << p._x << " +- " << p._xError
      << "  y: " << p._y << " +- " << p._yStatError << " (stat) +- " << p._ySysError << " (syst)"
      << "  cov(x,y): " << p._xyCovariance;
  return out;
  }
  void DataPoint2D::encodeBinary(BinaryWriter & w) const
  {
  const float v[6] = {_x,_xError,_y,_yStatError,_ySysError,_xyCovariance};
  uint8_t mask = 0;
  for (int i=0; i<6; i++) { uint32_t u; std::memcpy(&u,&v[i],4); if (u!=0) mask |= uint8_t(1u<<i); }
  w.writeUInt8(mask);
  for (int i=0; i<6; i++) if (mask & (1u<<i)) w.writeFloat32(v[i]);
  }

  void DataPoint2D::decodeBinary(BinaryReader & r)
  {
  uint8_t mask = r.readUInt8();
  if (mask & 0xC0) throw Exception("Invalid data point mask","DataPoint2D::decodeBinary()");
  float * v[6] = {&_x,&_xError,&_y,&_yStatError,&_ySysError,&_xyCovariance};
  for (int i=0; i<6; i++) *v[i] = (mask & (1u<<i)) ? r.readFloat32() : 0.0f;
  }


  void DataPoint2D::decodeBinary(BinaryReader & r, const String & /*name*/, unsigned int /*version*/)
  {
  decodeBinary(r);
  }

  void DataPoint2D::writeBinary(std::ostream & out) const                     { writeBinaryObject(out,*this); }
  bool DataPoint2D::readBinary(std::istream & in)                             { return readBinaryObject(in,*this); }
  void DataPoint2D::saveBinary(const String & fileName, bool append) const    { saveBinaryObject(fileName,*this,append); }
  void DataPoint2D::loadBinary(const String & fileName, unsigned int index)    { loadBinaryObjectAt(fileName,*this,index); }
  void DataPoint2D::saveBinary(const String & fileName, const std::vector<DataPoint2D> & objects) { saveBinaryObjects(fileName,objects); }
  std::vector<DataPoint2D> DataPoint2D::loadAllBinary(const String & fileName)       { return loadAllBinaryObjects<DataPoint2D>(fileName); }

  } // namespace MATH
} // namespace CAP
