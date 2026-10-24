/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "DataPoint3D.hpp"
#include "MathExceptions.hpp"

ClassImp(CAP::MATH::DataPoint3D);

namespace CAP
{
  namespace MATH
  {

  DataPoint3D::DataPoint3D()
  :
  DataPoint(), _x(0.0), _xError(0.0), _y(0.0), _yError(0.0)
  { }

  DataPoint3D::DataPoint3D(double x, double y, double z)
  :
  DataPoint(z), _x(x), _xError(0.0), _y(y), _yError(0.0)
  { }

  DataPoint3D::DataPoint3D(double x, double y, double z, double zStatError, double zSysError)
  :
  DataPoint(z,zStatError,zSysError), _x(x), _xError(0.0), _y(y), _yError(0.0)
  { }

  DataPoint3D::DataPoint3D(double x, double xError, double y, double yError, double z, double zStatError, double zSysError)
  :
  DataPoint(z,zStatError,zSysError), _x(x), _xError(0.0), _y(y), _yError(0.0)
  {
  setXError(xError);
  setYError(yError);
  }

  DataPoint3D::DataPoint3D(const DataPoint3D & source)
  :
  DataPoint(source), _x(source._x), _xError(source._xError), _y(source._y), _yError(source._yError)
  { }

  DataPoint3D & DataPoint3D::operator=(const DataPoint3D & rhs)
  {
  if (this!=&rhs)
    {
    DataPoint::operator=(rhs);
    _x      = rhs._x;
    _xError = rhs._xError;
    _y      = rhs._y;
    _yError = rhs._yError;
    }
  return *this;
  }

  float DataPoint3D::x(unsigned int index) const
  {
  checkIndex(index,__FUNCTION__);
  return index==0 ? _x : _y;
  }

  float DataPoint3D::xError(unsigned int index) const
  {
  checkIndex(index,__FUNCTION__);
  return index==0 ? _xError : _yError;
  }

  void DataPoint3D::setX(unsigned int index, double value)
  {
  checkIndex(index,__FUNCTION__);
  if (index==0) _x = value; else _y = value;
  }

  void DataPoint3D::setXError(unsigned int index, double error)
  {
  checkIndex(index,__FUNCTION__);
  if (index==0) setXError(error); else setYError(error);
  }

  void DataPoint3D::set(double x, double xError, double y, double yError, double z, double zStatError, double zSysError)
  {
  if (xError<0) throw MathException("xError<0",__FUNCTION__);
  if (yError<0) throw MathException("yError<0",__FUNCTION__);
  setValue(z,zStatError,zSysError);
  _x      = x;
  _xError = xError;
  _y      = y;
  _yError = yError;
  }

  void DataPoint3D::setPoint(double x, double y, double z)
  {
  set(x,0.0,y,0.0,z,0.0,0.0);
  }

  void DataPoint3D::setXError(double xError)
  {
  if (xError<0) throw MathException("xError<0",__FUNCTION__);
  _xError = xError;
  }

  void DataPoint3D::setYError(double yError)
  {
  if (yError<0) throw MathException("yError<0",__FUNCTION__);
  _yError = yError;
  }

  DataPoint3D & DataPoint3D::operator/=(double rhs)
  {
  if (rhs==0) throw MathException("Attempting division by zero",__FUNCTION__);
  scale(1.0/rhs);
  return *this;
  }

  void DataPoint3D::print(std::ostream & out) const
  {
  out << "x: " << _x << " +- " << _xError
      << "  y: " << _y << " +- " << _yError
      << "  z: " << _value << " +- " << _stat << " (stat) +- " << _syst << " (syst)";
  }

  void DataPoint3D::encodeBinary(BinaryWriter & w) const
  {
  const float v[7] = {_x,_xError,_y,_yError,_value,_stat,_syst};
  uint8_t mask = 0;
  for (int i=0; i<7; i++) { uint32_t u; std::memcpy(&u,&v[i],4); if (u!=0) mask |= uint8_t(1u<<i); }
  w.writeUInt8(mask);
  for (int i=0; i<7; i++) if (mask & (1u<<i)) w.writeFloat32(v[i]);
  }

  void DataPoint3D::decodeBinary(BinaryReader & r)
  {
  uint8_t mask = r.readUInt8();
  if (mask & 0x80) throw Exception("Invalid data point mask","DataPoint3D::decodeBinary()");
  float * v[7] = {&_x,&_xError,&_y,&_yError,&_value,&_stat,&_syst};
  for (int i=0; i<7; i++) *v[i] = (mask & (1u<<i)) ? r.readFloat32() : 0.0f;
  }

  void DataPoint3D::decodeBinary(BinaryReader & r, const String & /*name*/, unsigned int /*version*/)
  {
  decodeBinary(r);
  }

  void DataPoint3D::writeBinary(std::ostream & out) const                     { writeBinaryObject(out,*this); }
  bool DataPoint3D::readBinary(std::istream & in)                             { return readBinaryObject(in,*this); }
  void DataPoint3D::saveBinary(const String & fileName, bool append) const    { saveBinaryObject(fileName,*this,append); }
  void DataPoint3D::loadBinary(const String & fileName, unsigned int index)   { loadBinaryObjectAt(fileName,*this,index); }
  void DataPoint3D::saveBinary(const String & fileName, const std::vector<DataPoint3D> & objects) { saveBinaryObjects(fileName,objects); }
  std::vector<DataPoint3D> DataPoint3D::loadAllBinary(const String & fileName)      { return loadAllBinaryObjects<DataPoint3D>(fileName); }

  } // namespace MATH
} // namespace CAP
