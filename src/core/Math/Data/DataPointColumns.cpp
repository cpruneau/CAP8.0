/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "DataPointColumns.hpp"
#include "DataPoint3D.hpp"
#include "DataPointND.hpp"
#include "MathExceptions.hpp"
#include <algorithm>
#include <iostream>
#include <numeric>

ClassImp(CAP::MATH::DataPointColumns);

namespace CAP
{
  namespace MATH
  {

  namespace
  {
  // Largest rank accepted when decoding binary data (sanity limit against corrupted input).
  constexpr uint64_t kMaximumDecodedRank = 1000000;

  // Check that column has n elements (or is empty when allowEmpty) and, for uncertainties, no negative element.
  void checkColumn(const std::vector<double> & column, std::size_t n, bool allowEmpty, bool isError,
                   const char * what, const char * source)
  {
  if (column.empty() && allowEmpty) return;
  if (column.size()!=n) throw MathException(String("Column size differs from the number of points: ")+what,source);
  if (isError)
    for (double e : column) if (e<0) throw MathException(String("Negative uncertainty in column ")+what,source);
  }

  std::vector<float> toFloat(const std::vector<double> & column, std::size_t n)
  {
  if (column.empty()) return std::vector<float>(n,0.0f);
  return std::vector<float>(column.begin(),column.end());
  }
  }

  //////////////////////////////////////////////////////////////////////////
  // Construction
  //////////////////////////////////////////////////////////////////////////

  DataPointColumns::DataPointColumns(unsigned int rank)
  :
  DataPoints(),
  _rank(1), _x(1), _xError(1), _value(), _stat(), _syst()
  {
  setRank(rank);
  }

  DataPointColumns::DataPointColumns(const std::vector<double> & x, const std::vector<double> & y)
  :
  DataPointColumns(1)
  {
  setColumns({x},{},y,{},{});
  }

  DataPointColumns::DataPointColumns(const std::vector<double> & x,
                                     const std::vector<double> & xError,
                                     const std::vector<double> & y,
                                     const std::vector<double> & yStatError,
                                     const std::vector<double> & ySysError)
  :
  DataPointColumns(1)
  {
  if (xError.empty()) setColumns({x},{},y,yStatError,ySysError);
  else                setColumns({x},{xError},y,yStatError,ySysError);
  }

  DataPointColumns::DataPointColumns(const std::vector< std::vector<double> > & x,
                                     const std::vector< std::vector<double> > & xError,
                                     const std::vector<double> & value,
                                     const std::vector<double> & stat,
                                     const std::vector<double> & syst)
  :
  DataPointColumns(1)
  {
  setColumns(x,xError,value,stat,syst);
  }

  //////////////////////////////////////////////////////////////////////////
  // Checks
  //////////////////////////////////////////////////////////////////////////

  void DataPointColumns::checkIndex(unsigned int index, const char * source) const
  {
  if (index>=_value.size()) throw MathException("Point index out of range (index>=size())",source);
  }

  void DataPointColumns::checkVariable(unsigned int variable, const char * source) const
  {
  if (variable>=_rank) throw MathException("Independent variable index out of range (variable>=rank())",source);
  }

  //////////////////////////////////////////////////////////////////////////
  // DataPoints interface
  //////////////////////////////////////////////////////////////////////////

  DataPoint2D DataPointColumns::point(unsigned int index) const
  {
  checkIndex(index,__FUNCTION__);
  if (_rank!=1) throw MathException("point() requires rank 1; use createPoint()",__FUNCTION__);
  return DataPoint2D(_x[0][index],_xError[0][index],_value[index],_stat[index],_syst[index]);
  }

  std::vector<double> DataPointColumns::coordinatesAt(unsigned int index) const
  {
  checkIndex(index,__FUNCTION__);
  std::vector<double> c(_rank);
  for (unsigned int i=0; i<_rank; i++) c[i] = _x[i][index];
  return c;
  }

  std::vector<double> DataPointColumns::coordinateErrorsAt(unsigned int index) const
  {
  checkIndex(index,__FUNCTION__);
  std::vector<double> c(_rank);
  for (unsigned int i=0; i<_rank; i++) c[i] = _xError[i][index];
  return c;
  }

  double DataPointColumns::valueAt(unsigned int index) const     { checkIndex(index,__FUNCTION__); return _value[index]; }
  double DataPointColumns::statErrorAt(unsigned int index) const { checkIndex(index,__FUNCTION__); return _stat[index];  }
  double DataPointColumns::sysErrorAt(unsigned int index) const  { checkIndex(index,__FUNCTION__); return _syst[index];  }

  void DataPointColumns::addPoint(const DataPoint2D & dataPoint)
  {
  addPoint(static_cast<const DataPoint &>(dataPoint));
  }

  void DataPointColumns::addPoint(float x, float xError, float y, float yStatError, float ySysError)
  {
  addPoint(std::vector<double>{x},std::vector<double>{xError},y,yStatError,ySysError);
  }

  void DataPointColumns::print() const
  {
  for (unsigned int k=0; k<size(); k++) std::cout << k << ": " << *createPoint(k) << std::endl;
  }

  //////////////////////////////////////////////////////////////////////////
  // Points of any rank
  //////////////////////////////////////////////////////////////////////////

  void DataPointColumns::addPoint(const DataPoint & dataPoint)
  {
  addPoint(dataPoint.xValues(),dataPoint.xErrors(),dataPoint.value(),dataPoint.stat(),dataPoint.syst());
  }

  void DataPointColumns::addPoint(const std::vector<double> & x, const std::vector<double> & xError,
                                  double value, double stat, double syst)
  {
  if (x.empty()) throw MathException("A point needs at least one independent variable",__FUNCTION__);
  if (!_value.empty() && x.size()!=_rank) throw MathException("Point rank differs from the rank of the container",__FUNCTION__);
  if (!xError.empty() && xError.size()!=x.size()) throw MathException("x and xError sizes differ",__FUNCTION__);
  for (double e : xError) if (e<0) throw MathException("xError<0",__FUNCTION__);
  if (stat<0) throw MathException("stat<0",__FUNCTION__);
  if (syst<0) throw MathException("syst<0",__FUNCTION__);
  if (_value.empty() && x.size()!=_rank) setRank(x.size());
  // Make room in every column first (geometric growth), so that the push_backs below cannot throw part way
  // through and leave columns of different sizes.
  std::size_t n = _value.size();
  bool full = _value.capacity()==n || _stat.capacity()==n || _syst.capacity()==n;
  for (unsigned int i=0; i<_rank && !full; i++) full = _x[i].capacity()==n || _xError[i].capacity()==n;
  if (full) reserve(std::max<std::size_t>(16,2*n));
  for (unsigned int i=0; i<_rank; i++)
    {
    _x[i].push_back(x[i]);
    _xError[i].push_back(xError.empty() ? 0.0f : float(xError[i]));
    }
  _value.push_back(value);
  _stat.push_back(stat);
  _syst.push_back(syst);
  }

  std::unique_ptr<DataPoint> DataPointColumns::createPoint(unsigned int index) const
  {
  checkIndex(index,__FUNCTION__);
  if (_rank==1)
    return std::unique_ptr<DataPoint>(new DataPoint2D(_x[0][index],_xError[0][index],_value[index],_stat[index],_syst[index]));
  if (_rank==2)
    return std::unique_ptr<DataPoint>(new DataPoint3D(_x[0][index],_xError[0][index],_x[1][index],_xError[1][index],
                                                      _value[index],_stat[index],_syst[index]));
  return std::unique_ptr<DataPoint>(new DataPointND(coordinatesAt(index),coordinateErrorsAt(index),
                                                    _value[index],_stat[index],_syst[index]));
  }

  //////////////////////////////////////////////////////////////////////////
  // Columns
  //////////////////////////////////////////////////////////////////////////

  const std::vector<float> & DataPointColumns::xColumn(unsigned int variable) const
  {
  checkVariable(variable,__FUNCTION__);
  return _x[variable];
  }

  const std::vector<float> & DataPointColumns::xErrorColumn(unsigned int variable) const
  {
  checkVariable(variable,__FUNCTION__);
  return _xError[variable];
  }

  void DataPointColumns::setColumns(const std::vector< std::vector<double> > & x,
                                    const std::vector< std::vector<double> > & xError,
                                    const std::vector<double> & value,
                                    const std::vector<double> & stat,
                                    const std::vector<double> & syst)
  {
  if (x.empty()) throw MathException("At least one independent variable column is required",__FUNCTION__);
  if (!xError.empty() && xError.size()!=x.size()) throw MathException("x and xError have different numbers of columns",__FUNCTION__);
  std::size_t n = value.size();
  for (const auto & c : x)      checkColumn(c,n,false,false,"x",__FUNCTION__);
  for (const auto & c : xError) checkColumn(c,n,true,true,"xError",__FUNCTION__);
  checkColumn(stat,n,true,true,"stat",__FUNCTION__);
  checkColumn(syst,n,true,true,"syst",__FUNCTION__);
  // Build the new columns, then swap them in: the container is unchanged if anything above or below throws.
  std::vector< std::vector<float> > newX, newXError;
  newX.reserve(x.size());
  newXError.reserve(x.size());
  for (std::size_t i=0; i<x.size(); i++)
    {
    newX.push_back(toFloat(x[i],n));
    newXError.push_back(toFloat(xError.empty() ? std::vector<double>() : xError[i],n));
    }
  std::vector<float> newValue = toFloat(value,n);
  std::vector<float> newStat  = toFloat(stat,n);
  std::vector<float> newSyst  = toFloat(syst,n);
  _rank = x.size();
  _x.swap(newX);
  _xError.swap(newXError);
  _value.swap(newValue);
  _stat.swap(newStat);
  _syst.swap(newSyst);
  }

  //////////////////////////////////////////////////////////////////////////
  // Setters
  //////////////////////////////////////////////////////////////////////////

  void DataPointColumns::setRank(unsigned int rank)
  {
  if (rank==0) throw MathException("rank must be at least 1",__FUNCTION__);
  if (!_value.empty()) throw MathException("The rank can only be changed while the container is empty",__FUNCTION__);
  _x.assign(rank,std::vector<float>());
  _xError.assign(rank,std::vector<float>());
  _rank = rank;
  }

  void DataPointColumns::storePoint(unsigned int index, const DataPoint & dataPoint)
  {
  for (unsigned int i=0; i<_rank; i++)
    {
    _x[i][index]      = dataPoint.x(i);
    _xError[i][index] = dataPoint.xError(i);
    }
  _value[index] = dataPoint.value();
  _stat[index]  = dataPoint.stat();
  _syst[index]  = dataPoint.syst();
  }

  void DataPointColumns::setPoint(unsigned int index, const DataPoint & dataPoint)
  {
  checkIndex(index,__FUNCTION__);
  if (dataPoint.rank()!=_rank) throw MathException("Point rank differs from the rank of the container",__FUNCTION__);
  storePoint(index,dataPoint);
  }

  void DataPointColumns::removePoint(unsigned int index)
  {
  checkIndex(index,__FUNCTION__);
  for (unsigned int i=0; i<_rank; i++)
    {
    _x[i].erase(_x[i].begin()+index);
    _xError[i].erase(_xError[i].begin()+index);
    }
  _value.erase(_value.begin()+index);
  _stat.erase(_stat.begin()+index);
  _syst.erase(_syst.begin()+index);
  }

  void DataPointColumns::clear()
  {
  for (unsigned int i=0; i<_rank; i++) { _x[i].clear(); _xError[i].clear(); }
  _value.clear();
  _stat.clear();
  _syst.clear();
  }

  void DataPointColumns::reserve(unsigned int n)
  {
  for (unsigned int i=0; i<_rank; i++) { _x[i].reserve(n); _xError[i].reserve(n); }
  _value.reserve(n);
  _stat.reserve(n);
  _syst.reserve(n);
  }

  void DataPointColumns::sortByX(unsigned int variable)
  {
  checkVariable(variable,__FUNCTION__);
  std::vector<std::size_t> order(_value.size());
  std::iota(order.begin(),order.end(),std::size_t(0));
  const std::vector<float> & key = _x[variable];
  std::stable_sort(order.begin(),order.end(),[&key](std::size_t a, std::size_t b) { return key[a] < key[b]; });
  auto permute = [&order](std::vector<float> & column)
    {
    std::vector<float> sorted(column.size());
    for (std::size_t k=0; k<order.size(); k++) sorted[k] = column[order[k]];
    column.swap(sorted);
    };
  for (unsigned int i=0; i<_rank; i++) { permute(_x[i]); permute(_xError[i]); }
  permute(_value);
  permute(_stat);
  permute(_syst);
  }

  //////////////////////////////////////////////////////////////////////////
  // Arithmetic
  //////////////////////////////////////////////////////////////////////////

  namespace
  {
  DataPointColumns combineColumns(const DataPointColumns & left, const DataPointColumns & right, bool subtract, const char * source)
  {
  if (left.rank()!=right.rank()) throw MathException("Data sets have different ranks",source);
  if (left.size()!=right.size()) throw MathException("Data sets have different numbers of points",source);
  DataPointColumns result(left);
  for (unsigned int k=0; k<left.size(); k++)
    {
    std::unique_ptr<DataPoint> p = left.createPoint(k);
    std::unique_ptr<DataPoint> q = right.createPoint(k);
    if (subtract) p->subtract(*q); else p->add(*q);
    result.setPoint(k,*p);
    }
  return result;
  }
  }

  DataPointColumns & DataPointColumns::operator+=(const DataPointColumns & rhs)
  {
  DataPointColumns result = combineColumns(*this,rhs,false,__FUNCTION__);
  *this = std::move(result);
  return *this;
  }

  DataPointColumns & DataPointColumns::operator-=(const DataPointColumns & rhs)
  {
  DataPointColumns result = combineColumns(*this,rhs,true,__FUNCTION__);
  *this = std::move(result);
  return *this;
  }

  DataPointColumns operator+(const DataPointColumns & left, const DataPointColumns & right) { DataPointColumns r(left); r += right; return r; }
  DataPointColumns operator-(const DataPointColumns & left, const DataPointColumns & right) { DataPointColumns r(left); r -= right; return r; }

  //////////////////////////////////////////////////////////////////////////
  // Binary I/O
  //////////////////////////////////////////////////////////////////////////

  void DataPointColumns::encodeBinary(BinaryWriter & w) const
  {
  w.writeVarUInt(_rank);
  w.writeVarUInt(_value.size());
  auto writeColumn = [&w](const std::vector<float> & column) { for (float v : column) w.writeFloat32(v); };
  for (unsigned int i=0; i<_rank; i++) { writeColumn(_x[i]); writeColumn(_xError[i]); }
  writeColumn(_value);
  writeColumn(_stat);
  writeColumn(_syst);
  _fits.encodeBinary(w);   // format version 2: fits attached
  }

  void DataPointColumns::decodeBinary(BinaryReader & r, const String & /*name*/, unsigned int version)
  {
  uint64_t rank = r.readVarUInt();
  uint64_t n    = r.readVarUInt();
  if (rank==0 || rank>kMaximumDecodedRank) throw Exception("Invalid rank","DataPointColumns::decodeBinary()");
  if (n>r.remaining() || 4*n*(2*rank+3)>r.remaining()) throw Exception("More points than data","DataPointColumns::decodeBinary()");
  auto readColumn = [&r,n]() { std::vector<float> column(n); for (auto & v : column) v = r.readFloat32(); return column; };
  std::vector< std::vector<float> > x, xError;
  for (uint64_t i=0; i<rank; i++) { x.push_back(readColumn()); xError.push_back(readColumn()); }
  std::vector<float> value = readColumn();
  std::vector<float> stat  = readColumn();
  std::vector<float> syst  = readColumn();
  if (version>=2) _fits.decodeBinary(r);
  _rank = rank;
  _x.swap(x);
  _xError.swap(xError);
  _value.swap(value);
  _stat.swap(stat);
  _syst.swap(syst);
  }

  void DataPointColumns::writeBinary(std::ostream & out) const                     { writeBinaryObject(out,*this); }
  bool DataPointColumns::readBinary(std::istream & in)                             { return readBinaryObject(in,*this); }
  void DataPointColumns::saveBinary(const String & fileName, bool append) const    { saveBinaryObject(fileName,*this,append); }
  void DataPointColumns::loadBinary(const String & fileName, unsigned int index)   { loadBinaryObjectAt(fileName,*this,index); }
  void DataPointColumns::saveBinary(const String & fileName, const std::vector<DataPointColumns> & objects) { saveBinaryObjects(fileName,objects); }
  std::vector<DataPointColumns> DataPointColumns::loadAllBinary(const String & fileName)       { return loadAllBinaryObjects<DataPointColumns>(fileName); }

  } // namespace MATH
} // namespace CAP
