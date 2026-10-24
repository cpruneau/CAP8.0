#include "DataPoint2D.hpp"
#include "MathExceptions.hpp"
#include <algorithm>
#include <iostream>


namespace CAP
{
  namespace MATH
  {

  namespace
  {
  // Relative tolerance used for symmetry checks.
  constexpr double kRelTolerance = 1.0E-6;
  }

  //////////////////////////////////////////////////////////////////////////
  // Construction / assignment
  //////////////////////////////////////////////////////////////////////////

  DataPoint2D::DataPoint2D()
  :
  DataPoint(), _x(0.0), _xError(0.0), _xyCovariance(0.0)
  { }

  DataPoint2D::DataPoint2D(float x, float y)
  :
  DataPoint(y), _x(x), _xError(0.0), _xyCovariance(0.0)
  { }

  DataPoint2D::DataPoint2D(float x, float y, float yStatError)
  :
  DataPoint(y,yStatError), _x(x), _xError(0.0), _xyCovariance(0.0)
  { }

  DataPoint2D::DataPoint2D(float x, float y, float yStatError, float ySysError)
  :
  DataPoint(y,yStatError,ySysError), _x(x), _xError(0.0), _xyCovariance(0.0)
  { }

  DataPoint2D::DataPoint2D(float x, float xError, float y, float yStatError, float ySysError)
  :
  DataPoint(y,yStatError,ySysError), _x(x), _xError(xError), _xyCovariance(0.0)
  { }

  DataPoint2D::DataPoint2D(const DataPoint2D & src)
  :
  DataPoint(src), _x(src._x), _xError(src._xError), _xyCovariance(src._xyCovariance)
  { }

  DataPoint2D & DataPoint2D::operator= (const DataPoint2D & rhs)
  {
  if (this!=&rhs)
    {
    DataPoint::operator=(rhs);
    _x            = rhs._x;
    _xError       = rhs._xError;
    _xyCovariance = rhs._xyCovariance;
    }
  return *this;
  }

  //////////////////////////////////////////////////////////////////////////
  // DataPoint interface
  //////////////////////////////////////////////////////////////////////////

  float DataPoint2D::x(unsigned int index) const
  {
  checkIndex(index,__FUNCTION__);
  return _x;
  }

  float DataPoint2D::xError(unsigned int index) const
  {
  checkIndex(index,__FUNCTION__);
  return _xError;
  }

  void DataPoint2D::setX(unsigned int index, double value)
  {
  checkIndex(index,__FUNCTION__);
  _x = value;
  }

  void DataPoint2D::setXError(unsigned int index, double error)
  {
  checkIndex(index,__FUNCTION__);
  setXError(error);
  }

  //////////////////////////////////////////////////////////////////////////
  // Values and uncertainties
  //////////////////////////////////////////////////////////////////////////

  float DataPoint2D::correlation() const
  {
  double denominator = double(_xError)*double(_stat);
  if (denominator<=0.0) return 0.0;
  return _xyCovariance/denominator;
  }

  void DataPoint2D::set(float x, float xError, float y, float yStatError, float ySysError)
  {
  _x            = x;
  _xError       = xError;
  _value        = y;
  _stat         = yStatError;
  _syst         = ySysError;
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
  _stat         = std::sqrt(varY);
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
  _stat         = yStatError;
  _xyCovariance = correlation*xError*yStatError;
  }

  void DataPoint2D::errorMatrix(double matrix[2][2]) const
  {
  matrix[0][0] = double(_xError)*_xError;
  matrix[0][1] = _xyCovariance;
  matrix[1][0] = _xyCovariance;
  matrix[1][1] = double(_stat)*_stat;
  }

  std::vector< std::vector<double> > DataPoint2D::errorMatrix() const
  {
  double m[2][2];
  errorMatrix(m);
  return { {m[0][0], m[0][1]}, {m[1][0], m[1][1]} };
  }

  //////////////////////////////////////////////////////////////////////////
  // Arithmetic
  //////////////////////////////////////////////////////////////////////////

  void DataPoint2D::combine(const DataPoint & p1, const DataPoint & p2, double value, double d1, double d2)
  {
  // Read the covariances before DataPoint::combine() overwrites this point (p1 or p2 may be this point).
  const DataPoint2D * q1 = dynamic_cast<const DataPoint2D*>(&p1);
  const DataPoint2D * q2 = dynamic_cast<const DataPoint2D*>(&p2);
  double cov1 = q1 ? q1->_xyCovariance : 0.0;
  double cov2 = q2 ? q2->_xyCovariance : 0.0;
  DataPoint::combine(p1,p2,value,d1,d2);
  _xyCovariance = 0.5*(d1*cov1 + d2*cov2);
  }

  DataPoint2D & DataPoint2D::operator/= (double rhs)
  {
  if (rhs==0) throw MathException("Attempting division by zero",__FUNCTION__);
  scale(1.0/rhs);
  return *this;
  }

  void DataPoint2D::scale(double coefficient)
  {
  DataPoint::scale(coefficient);
  _xyCovariance *= coefficient;
  }

  void DataPoint2D::inverse()
  {
  double y = _value;
  DataPoint::inverse();  // throws if y==0
  _xyCovariance = -_xyCovariance/(y*y);
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

  void DataPoint2D::print(std::ostream & out) const
  {
  out << "x: " << _x << " +- " << _xError
      << "  y: " << _value << " +- " << _stat << " (stat) +- " << _syst << " (syst)"
      << "  cov(x,y): " << _xyCovariance;
  }

  //////////////////////////////////////////////////////////////////////////
  // Binary I/O
  //////////////////////////////////////////////////////////////////////////

  void DataPoint2D::encodeBinary(BinaryWriter & w) const
  {
  const float v[6] = {_x,_xError,_value,_stat,_syst,_xyCovariance};
  uint8_t mask = 0;
  for (int i=0; i<6; i++) { uint32_t u; std::memcpy(&u,&v[i],4); if (u!=0) mask |= uint8_t(1u<<i); }
  w.writeUInt8(mask);
  for (int i=0; i<6; i++) if (mask & (1u<<i)) w.writeFloat32(v[i]);
  }

  void DataPoint2D::decodeBinary(BinaryReader & r)
  {
  uint8_t mask = r.readUInt8();
  if (mask & 0xC0) throw Exception("Invalid data point mask","DataPoint2D::decodeBinary()");
  float * v[6] = {&_x,&_xError,&_value,&_stat,&_syst,&_xyCovariance};
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
