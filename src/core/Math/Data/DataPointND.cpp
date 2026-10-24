/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "DataPointND.hpp"
#include "MathExceptions.hpp"


namespace CAP
{
  namespace MATH
  {

  namespace
  {
  // Smallest rank handled by DataPointND, and a sanity limit used when decoding binary data.
  constexpr unsigned int kMinimumRank = 3;
  constexpr uint64_t     kMaximumDecodedRank = 1000000;
  }

  DataPointND::DataPointND()
  :
  DataPoint(), _x(kMinimumRank,0.0f), _xError(kMinimumRank,0.0f)
  { }

  DataPointND::DataPointND(unsigned int rank)
  :
  DataPoint(), _x(), _xError()
  {
  if (rank<kMinimumRank) throw MathException("DataPointND requires rank>=3 (use DataPoint2D or DataPoint3D)",__FUNCTION__);
  _x.assign(rank,0.0f);
  _xError.assign(rank,0.0f);
  }

  DataPointND::DataPointND(const std::vector<double> & x, double value, double stat, double syst)
  :
  DataPoint(), _x(), _xError()
  {
  set(x,std::vector<double>(x.size(),0.0),value,stat,syst);
  }

  DataPointND::DataPointND(const std::vector<double> & x, const std::vector<double> & xError, double value, double stat, double syst)
  :
  DataPoint(), _x(), _xError()
  {
  set(x,xError,value,stat,syst);
  }

  DataPointND::DataPointND(const DataPointND & source)
  :
  DataPoint(source), _x(source._x), _xError(source._xError)
  { }

  DataPointND & DataPointND::operator=(const DataPointND & rhs)
  {
  if (this!=&rhs)
    {
    DataPoint::operator=(rhs);
    _x      = rhs._x;
    _xError = rhs._xError;
    }
  return *this;
  }

  float DataPointND::x(unsigned int index) const
  {
  checkIndex(index,__FUNCTION__);
  return _x[index];
  }

  float DataPointND::xError(unsigned int index) const
  {
  checkIndex(index,__FUNCTION__);
  return _xError[index];
  }

  void DataPointND::setX(unsigned int index, double value)
  {
  checkIndex(index,__FUNCTION__);
  _x[index] = value;
  }

  void DataPointND::setXError(unsigned int index, double error)
  {
  checkIndex(index,__FUNCTION__);
  if (error<0) throw MathException("xError<0",__FUNCTION__);
  _xError[index] = error;
  }

  void DataPointND::set(const std::vector<double> & x, const std::vector<double> & xError, double value, double stat, double syst)
  {
  if (x.size()<kMinimumRank)   throw MathException("DataPointND requires rank>=3 (use DataPoint2D or DataPoint3D)",__FUNCTION__);
  if (xError.size()!=x.size()) throw MathException("x and xError sizes differ",__FUNCTION__);
  for (double e : xError) if (e<0) throw MathException("xError<0",__FUNCTION__);
  setValue(value,stat,syst);
  _x.assign(x.begin(),x.end());
  _xError.assign(xError.begin(),xError.end());
  }

  DataPointND & DataPointND::operator/=(double rhs)
  {
  if (rhs==0) throw MathException("Attempting division by zero",__FUNCTION__);
  scale(1.0/rhs);
  return *this;
  }

  void DataPointND::encodeBinary(BinaryWriter & w) const
  {
  w.writeVarUInt(_x.size());
  for (unsigned int i=0; i<_x.size(); i++)
    {
    w.writeFloat32(_x[i]);
    w.writeFloat32(_xError[i]);
    }
  w.writeFloat32(_value);
  w.writeFloat32(_stat);
  w.writeFloat32(_syst);
  }

  void DataPointND::decodeBinary(BinaryReader & r)
  {
  uint64_t n = r.readVarUInt();
  if (n<kMinimumRank || n>kMaximumDecodedRank || 4*(2*n+3)>r.remaining())
    throw Exception("Invalid data point rank","DataPointND::decodeBinary()");
  std::vector<float> x(n), xError(n);
  for (uint64_t i=0; i<n; i++)
    {
    x[i]      = r.readFloat32();
    xError[i] = r.readFloat32();
    }
  _value  = r.readFloat32();
  _stat   = r.readFloat32();
  _syst   = r.readFloat32();
  _x.swap(x);
  _xError.swap(xError);
  }

  void DataPointND::decodeBinary(BinaryReader & r, const String & /*name*/, unsigned int /*version*/)
  {
  decodeBinary(r);
  }

  void DataPointND::writeBinary(std::ostream & out) const                     { writeBinaryObject(out,*this); }
  bool DataPointND::readBinary(std::istream & in)                             { return readBinaryObject(in,*this); }
  void DataPointND::saveBinary(const String & fileName, bool append) const    { saveBinaryObject(fileName,*this,append); }
  void DataPointND::loadBinary(const String & fileName, unsigned int index)   { loadBinaryObjectAt(fileName,*this,index); }
  void DataPointND::saveBinary(const String & fileName, const std::vector<DataPointND> & objects) { saveBinaryObjects(fileName,objects); }
  std::vector<DataPointND> DataPointND::loadAllBinary(const String & fileName)      { return loadAllBinaryObjects<DataPointND>(fileName); }

  } // namespace MATH
} // namespace CAP
