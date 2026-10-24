#include "DataPointsVector.hpp"
#include "MathExceptions.hpp"
#include <algorithm>
#include <iostream>
#include <memory>


namespace CAP
{
  namespace MATH
  {

  namespace
  {
  // Owning temporary used to build new point lists with the strong exception guarantee.
  using PointList = std::vector< std::unique_ptr<DataPoint> >;

  // Largest rank accepted when decoding binary data (sanity limit against corrupted input).
  constexpr uint64_t kMaximumDecodedRank = 1000000;

  PointList clonePoints(const std::vector<DataPoint*> & points)
  {
  PointList copies;
  copies.reserve(points.size());
  for (const DataPoint * p : points) copies.emplace_back(p->clone());
  return copies;
  }

  // Replace the content of target (deleting its points) by the points of list (released to target).
  void adopt(std::vector<DataPoint*> & target, PointList & list)
  {
  std::vector<DataPoint*> fresh;
  fresh.reserve(list.size());
  for (auto & p : list) fresh.push_back(p.release());
  target.swap(fresh);
  for (DataPoint * p : fresh) delete p;
  }
  }

  //////////////////////////////////////////////////////////////////////////
  // Construction / assignment
  //////////////////////////////////////////////////////////////////////////

  DataPointsVector::DataPointsVector()
  :
  DataPoints(),
  _dataPoints()
  { }

  DataPointsVector::DataPointsVector(const std::vector<DataPoint2D> & dataPoints)
  :
  DataPoints(),
  _dataPoints()
  {
  try { addPoints(dataPoints); }
  catch (...) { deletePoints(); throw; }
  }

  DataPointsVector::DataPointsVector(const std::vector<double> & x, const std::vector<double> & y)
  :
  DataPoints(),
  _dataPoints()
  {
  try { addPoints(x,y); }
  catch (...) { deletePoints(); throw; }
  }

  DataPointsVector::DataPointsVector(const std::vector<double> & x,
                                     const std::vector<double> & xError,
                                     const std::vector<double> & y,
                                     const std::vector<double> & yStatError,
                                     const std::vector<double> & ySysError)
  :
  DataPoints(),
  _dataPoints()
  {
  try { addPoints(x,xError,y,yStatError,ySysError); }
  catch (...) { deletePoints(); throw; }
  }

  DataPointsVector::DataPointsVector(const DataPointsVector & source)
  :
  DataPoints(source),
  _dataPoints()
  {
  PointList copies = clonePoints(source._dataPoints);
  adopt(_dataPoints,copies);
  }

  DataPointsVector::DataPointsVector(DataPointsVector && source) noexcept
  :
  DataPoints(source),
  _dataPoints()
  {
  _dataPoints.swap(source._dataPoints);
  }

  DataPointsVector::~DataPointsVector()
  {
  deletePoints();
  }

  DataPointsVector & DataPointsVector::operator=(const DataPointsVector & rhs)
  {
  if (this!=&rhs)
    {
    PointList copies = clonePoints(rhs._dataPoints);   // may throw: this container is then unchanged
    adopt(_dataPoints,copies);
    _fits = rhs._fits;
    }
  return *this;
  }

  DataPointsVector & DataPointsVector::operator=(DataPointsVector && rhs) noexcept
  {
  if (this!=&rhs)
    {
    deletePoints();
    _dataPoints.swap(rhs._dataPoints);
    _fits = std::move(rhs._fits);
    }
  return *this;
  }

  void DataPointsVector::deletePoints()
  {
  for (DataPoint * p : _dataPoints) delete p;
  _dataPoints.clear();
  }

  void DataPointsVector::checkIndex(unsigned int index, const char * source) const
  {
  if (index>=_dataPoints.size()) throw MathException("Point index out of range (index>=size())",source);
  }

  void DataPointsVector::checkRank(unsigned int pointRank, const char * source) const
  {
  if (!_dataPoints.empty() && pointRank!=_dataPoints.front()->rank())
    throw MathException("Point rank differs from the rank of the points in the container",source);
  }

  //////////////////////////////////////////////////////////////////////////
  // DataPoints interface
  //////////////////////////////////////////////////////////////////////////

  unsigned int DataPointsVector::rank() const
  {
  return _dataPoints.empty() ? DataPoints::rank() : _dataPoints.front()->rank();
  }

  DataPoint2D DataPointsVector::point(unsigned int index) const
  {
  checkIndex(index,__FUNCTION__);
  const DataPoint2D * p = dynamic_cast<const DataPoint2D*>(_dataPoints[index]);
  if (!p) throw MathException("point() requires a DataPoint2D (rank 1); use operator[] or pointAs<>()",__FUNCTION__);
  return *p;
  }

  std::vector<double> DataPointsVector::coordinatesAt(unsigned int index) const
  {
  checkIndex(index,__FUNCTION__);
  return _dataPoints[index]->xValues();
  }

  std::vector<double> DataPointsVector::coordinateErrorsAt(unsigned int index) const
  {
  checkIndex(index,__FUNCTION__);
  return _dataPoints[index]->xErrors();
  }

  double DataPointsVector::valueAt(unsigned int index) const
  {
  checkIndex(index,__FUNCTION__);
  return _dataPoints[index]->value();
  }

  double DataPointsVector::statErrorAt(unsigned int index) const
  {
  checkIndex(index,__FUNCTION__);
  return _dataPoints[index]->stat();
  }

  double DataPointsVector::sysErrorAt(unsigned int index) const
  {
  checkIndex(index,__FUNCTION__);
  return _dataPoints[index]->syst();
  }

  void DataPointsVector::addPoint(const DataPoint & dataPoint)
  {
  checkRank(dataPoint.rank(),__FUNCTION__);
  std::unique_ptr<DataPoint> copy(dataPoint.clone());
  _dataPoints.push_back(copy.get());
  copy.release();
  }

  void DataPointsVector::addPoint(const DataPoint2D & dataPoint)
  {
  addPoint(static_cast<const DataPoint &>(dataPoint));
  }

  void DataPointsVector::addPoint(float x, float xError, float y, float yStatError, float ySysError)
  {
  addPoint(static_cast<const DataPoint &>(DataPoint2D(x,xError,y,yStatError,ySysError)));
  }

  void DataPointsVector::addPoints(const std::vector<DataPoint2D> & dataPoints)
  {
  if (!dataPoints.empty()) checkRank(1,__FUNCTION__);
  _dataPoints.reserve(_dataPoints.size()+dataPoints.size());
  for (const auto & p : dataPoints) addPoint(static_cast<const DataPoint &>(p));
  }

  void DataPointsVector::print() const
  {
  for (unsigned int k=0; k<_dataPoints.size(); k++) std::cout << k << ": " << *_dataPoints[k] << std::endl;
  }

  const DataPoint & DataPointsVector::operator[](unsigned int index) const
  {
  checkIndex(index,__FUNCTION__);
  return *_dataPoints[index];
  }

  DataPoint & DataPointsVector::operator[](unsigned int index)
  {
  checkIndex(index,__FUNCTION__);
  return *_dataPoints[index];
  }

  //////////////////////////////////////////////////////////////////////////
  // Getters
  //////////////////////////////////////////////////////////////////////////

  std::vector<double> DataPointsVector::xValues(unsigned int variable) const
  {
  std::vector<double> v; v.reserve(_dataPoints.size());
  for (const DataPoint * p : _dataPoints) v.push_back(p->x(variable));
  return v;
  }

  std::vector<double> DataPointsVector::xErrors(unsigned int variable) const
  {
  std::vector<double> v; v.reserve(_dataPoints.size());
  for (const DataPoint * p : _dataPoints) v.push_back(p->xError(variable));
  return v;
  }

  std::vector<double> DataPointsVector::yValues() const
  {
  std::vector<double> v; v.reserve(_dataPoints.size());
  for (const DataPoint * p : _dataPoints) v.push_back(p->value());
  return v;
  }

  std::vector<double> DataPointsVector::yStatErrors() const
  {
  std::vector<double> v; v.reserve(_dataPoints.size());
  for (const DataPoint * p : _dataPoints) v.push_back(p->stat());
  return v;
  }

  std::vector<double> DataPointsVector::ySysErrors() const
  {
  std::vector<double> v; v.reserve(_dataPoints.size());
  for (const DataPoint * p : _dataPoints) v.push_back(p->syst());
  return v;
  }

  std::vector<double> DataPointsVector::yErrors() const
  {
  std::vector<double> v; v.reserve(_dataPoints.size());
  for (const DataPoint * p : _dataPoints) v.push_back(p->error());
  return v;
  }

  //////////////////////////////////////////////////////////////////////////
  // Setters
  //////////////////////////////////////////////////////////////////////////

  void DataPointsVector::setPoint(unsigned int index, const DataPoint & dataPoint)
  {
  checkIndex(index,__FUNCTION__);
  if (_dataPoints.size()>1 && dataPoint.rank()!=rank())
    throw MathException("Point rank differs from the rank of the points in the container",__FUNCTION__);
  DataPoint * copy = dataPoint.clone();
  delete _dataPoints[index];
  _dataPoints[index] = copy;
  }

  void DataPointsVector::setPoints(const std::vector<DataPoint2D> & dataPoints)
  {
  DataPointsVector replacement(dataPoints);
  _dataPoints.swap(replacement._dataPoints);
  }

  void DataPointsVector::setPoints(const std::vector<double> & x, const std::vector<double> & y)
  {
  DataPointsVector replacement(x,y);          // throws before anything is changed
  _dataPoints.swap(replacement._dataPoints);
  }

  void DataPointsVector::setPoints(const std::vector<double> & x,
                                   const std::vector<double> & xError,
                                   const std::vector<double> & y,
                                   const std::vector<double> & yStatError,
                                   const std::vector<double> & ySysError)
  {
  DataPointsVector replacement(x,xError,y,yStatError,ySysError);  // throws before anything is changed
  _dataPoints.swap(replacement._dataPoints);
  }

  void DataPointsVector::removePoint(unsigned int index)
  {
  checkIndex(index,__FUNCTION__);
  delete _dataPoints[index];
  _dataPoints.erase(_dataPoints.begin()+index);
  }

  void DataPointsVector::sortByX(unsigned int variable)
  {
  if (_dataPoints.empty()) return;
  if (variable>=rank()) throw MathException("variable>=rank()",__FUNCTION__);
  std::stable_sort(_dataPoints.begin(),_dataPoints.end(),
                   [variable](const DataPoint * a, const DataPoint * b) { return a->x(variable) < b->x(variable); });
  }

  //////////////////////////////////////////////////////////////////////////
  // Arithmetic
  //////////////////////////////////////////////////////////////////////////

  DataPointsVector & DataPointsVector::operator+=(const DataPointsVector & rhs)
  {
  if (rhs._dataPoints.size()!=_dataPoints.size()) throw MathException("Data sets have different numbers of points",__FUNCTION__);
  PointList result = clonePoints(_dataPoints);
  for (std::size_t i=0; i<result.size(); i++) result[i]->add(*rhs._dataPoints[i]);
  adopt(_dataPoints,result);
  return *this;
  }

  DataPointsVector & DataPointsVector::operator-=(const DataPointsVector & rhs)
  {
  if (rhs._dataPoints.size()!=_dataPoints.size()) throw MathException("Data sets have different numbers of points",__FUNCTION__);
  PointList result = clonePoints(_dataPoints);
  for (std::size_t i=0; i<result.size(); i++) result[i]->subtract(*rhs._dataPoints[i]);
  adopt(_dataPoints,result);
  return *this;
  }

  DataPointsVector operator+(const DataPointsVector & left, const DataPointsVector & right) { DataPointsVector r(left); r += right; return r; }
  DataPointsVector operator-(const DataPointsVector & left, const DataPointsVector & right) { DataPointsVector r(left); r -= right; return r; }

  //////////////////////////////////////////////////////////////////////////
  // Binary I/O
  //////////////////////////////////////////////////////////////////////////

  void DataPointsVector::encodeBinary(BinaryWriter & w) const
  {
  w.writeVarUInt(_dataPoints.size());
  unsigned int n = _dataPoints.empty() ? 0 : rank();
  if (n>0) w.writeVarUInt(n);
  for (const DataPoint * p : _dataPoints)
    {
    if (n==1)
      {
      const DataPoint2D * q = dynamic_cast<const DataPoint2D*>(p);
      if (!q) throw Exception("Rank 1 point is not a DataPoint2D","DataPointsVector::encodeBinary()");
      q->encodeBinary(w);
      }
    else if (n==2)
      {
      const DataPoint3D * q = dynamic_cast<const DataPoint3D*>(p);
      if (!q) throw Exception("Rank 2 point is not a DataPoint3D","DataPointsVector::encodeBinary()");
      q->encodeBinary(w);
      }
    else
      {
      const DataPointND * q = dynamic_cast<const DataPointND*>(p);
      if (!q) throw Exception("Rank>2 point is not a DataPointND","DataPointsVector::encodeBinary()");
      q->encodeBinary(w);
      }
    }
  _fits.encodeBinary(w);   // format version 3: fits attached
  }

  void DataPointsVector::decodeBinary(BinaryReader & r, const String & /*name*/, unsigned int version)
  {
  uint64_t n = r.readVarUInt();
  if (n > r.remaining()) throw Exception("More points than data","DataPointsVector::decodeBinary()");
  PointList points;
  points.reserve(std::size_t(n));
  if (version<=1)
    {
    // Version 1: rank 1 points only, no rank field.
    for (uint64_t k=0; k<n; k++)
      {
      std::unique_ptr<DataPoint2D> p(new DataPoint2D());
      p->decodeBinary(r);
      points.emplace_back(p.release());
      }
    }
  else if (n>0)
    {
    uint64_t pointRank = r.readVarUInt();
    if (pointRank==0 || pointRank>kMaximumDecodedRank) throw Exception("Invalid point rank","DataPointsVector::decodeBinary()");
    for (uint64_t k=0; k<n; k++)
      {
      if (pointRank==1)
        {
        std::unique_ptr<DataPoint2D> p(new DataPoint2D());
        p->decodeBinary(r);
        points.emplace_back(p.release());
        }
      else if (pointRank==2)
        {
        std::unique_ptr<DataPoint3D> p(new DataPoint3D());
        p->decodeBinary(r);
        points.emplace_back(p.release());
        }
      else
        {
        std::unique_ptr<DataPointND> p(new DataPointND());
        p->decodeBinary(r);
        if (p->rank()!=pointRank) throw Exception("Point rank differs from the container rank","DataPointsVector::decodeBinary()");
        points.emplace_back(p.release());
        }
      }
    }
  if (version>=3) _fits.decodeBinary(r);
  adopt(_dataPoints,points);
  }

  void DataPointsVector::writeBinary(std::ostream & out) const                     { writeBinaryObject(out,*this); }
  bool DataPointsVector::readBinary(std::istream & in)                             { return readBinaryObject(in,*this); }
  void DataPointsVector::saveBinary(const String & fileName, bool append) const    { saveBinaryObject(fileName,*this,append); }
  void DataPointsVector::loadBinary(const String & fileName, unsigned int index)   { loadBinaryObjectAt(fileName,*this,index); }
  void DataPointsVector::saveBinary(const String & fileName, const std::vector<DataPointsVector> & objects) { saveBinaryObjects(fileName,objects); }
  std::vector<DataPointsVector> DataPointsVector::loadAllBinary(const String & fileName)       { return loadAllBinaryObjects<DataPointsVector>(fileName); }

  } // namespace MATH
} // namespace CAP
