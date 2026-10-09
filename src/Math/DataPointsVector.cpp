#include "DataPointsVector.hpp"
#include "MathExceptions.hpp"
#include <algorithm>

ClassImp(CAP::MATH::DataPointsVector);

namespace CAP
{
  namespace MATH
  {

  DataPointsVector::DataPointsVector()
  :
  DataPoints(),
  _dataPoints()
  { }

  DataPointsVector::DataPointsVector(const std::vector<DataPoint2D> & dataPoints)
  :
  DataPoints(),
  _dataPoints(dataPoints)
  { }

  DataPointsVector::DataPointsVector(const std::vector<double> & x, const std::vector<double> & y)
  :
  DataPoints(),
  _dataPoints()
  {
  addPoints(x,y);
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
  addPoints(x,xError,y,yStatError,ySysError);
  }

  DataPointsVector::DataPointsVector(const DataPointsVector & source)
  :
  DataPoints(source),
  _dataPoints(source._dataPoints)
  { }

  DataPointsVector & DataPointsVector::operator=(const DataPointsVector & rhs)
  {
  if (this!=&rhs) _dataPoints = rhs._dataPoints;
  return *this;
  }

  void DataPointsVector::checkIndex(unsigned int index, const char * source) const
  {
  if (index>=_dataPoints.size()) throw MathException("Point index out of range (index>=size())",source);
  }

  DataPoint2D DataPointsVector::point(unsigned int index) const
  {
  checkIndex(index,__FUNCTION__);
  return _dataPoints[index];
  }

  void DataPointsVector::addPoint(const DataPoint2D & dataPoint)
  {
  _dataPoints.push_back(dataPoint);
  }

  void DataPointsVector::addPoint(float x, float xError, float y, float yStatError, float ySysError)
  {
  _dataPoints.push_back(DataPoint2D(x,xError,y,yStatError,ySysError));
  }

  void DataPointsVector::addPoints(const std::vector<DataPoint2D> & dataPoints)
  {
  _dataPoints.insert(_dataPoints.end(),dataPoints.begin(),dataPoints.end());
  }

  const DataPoint2D & DataPointsVector::operator[](unsigned int index) const
  {
  checkIndex(index,__FUNCTION__);
  return _dataPoints[index];
  }

  DataPoint2D & DataPointsVector::operator[](unsigned int index)
  {
  checkIndex(index,__FUNCTION__);
  return _dataPoints[index];
  }

  std::vector<double> DataPointsVector::xValues() const
  {
  std::vector<double> v; v.reserve(_dataPoints.size());
  for (const auto & p : _dataPoints) v.push_back(p.x());
  return v;
  }

  std::vector<double> DataPointsVector::xErrors() const
  {
  std::vector<double> v; v.reserve(_dataPoints.size());
  for (const auto & p : _dataPoints) v.push_back(p.xError());
  return v;
  }

  std::vector<double> DataPointsVector::yValues() const
  {
  std::vector<double> v; v.reserve(_dataPoints.size());
  for (const auto & p : _dataPoints) v.push_back(p.y());
  return v;
  }

  std::vector<double> DataPointsVector::yStatErrors() const
  {
  std::vector<double> v; v.reserve(_dataPoints.size());
  for (const auto & p : _dataPoints) v.push_back(p.yStatError());
  return v;
  }

  std::vector<double> DataPointsVector::ySysErrors() const
  {
  std::vector<double> v; v.reserve(_dataPoints.size());
  for (const auto & p : _dataPoints) v.push_back(p.ySysError());
  return v;
  }

  std::vector<double> DataPointsVector::yErrors() const
  {
  std::vector<double> v; v.reserve(_dataPoints.size());
  for (const auto & p : _dataPoints) v.push_back(p.yError());
  return v;
  }

  void DataPointsVector::setPoint(unsigned int index, const DataPoint2D & dataPoint)
  {
  checkIndex(index,__FUNCTION__);
  _dataPoints[index] = dataPoint;
  }

  void DataPointsVector::setPoints(const std::vector<DataPoint2D> & dataPoints)
  {
  _dataPoints = dataPoints;
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
  _dataPoints.erase(_dataPoints.begin()+index);
  }

  void DataPointsVector::sortByX()
  {
  std::stable_sort(_dataPoints.begin(),_dataPoints.end(),
                   [](const DataPoint2D & a, const DataPoint2D & b) { return a.x() < b.x(); });
  }
  void DataPointsVector::encodeBinary(BinaryWriter & w) const
  {
  w.writeVarUInt(_dataPoints.size());
  for (const auto & p : _dataPoints) p.encodeBinary(w);
  }

  void DataPointsVector::decodeBinary(BinaryReader & r, const String & /*name*/, unsigned int /*version*/)
  {
  uint64_t n = r.readVarUInt();
  if (n > r.remaining()) throw Exception("More points than data","DataPointsVector::decodeBinary()");
  _dataPoints.assign(std::size_t(n),DataPoint2D());
  for (auto & p : _dataPoints) p.decodeBinary(r);
  }

  void DataPointsVector::writeBinary(std::ostream & out) const                     { writeBinaryObject(out,*this); }
  bool DataPointsVector::readBinary(std::istream & in)                             { return readBinaryObject(in,*this); }
  void DataPointsVector::saveBinary(const String & fileName, bool append) const    { saveBinaryObject(fileName,*this,append); }
  void DataPointsVector::loadBinary(const String & fileName, unsigned int index)    { loadBinaryObjectAt(fileName,*this,index); }
  void DataPointsVector::saveBinary(const String & fileName, const std::vector<DataPointsVector> & objects) { saveBinaryObjects(fileName,objects); }
  std::vector<DataPointsVector> DataPointsVector::loadAllBinary(const String & fileName)       { return loadAllBinaryObjects<DataPointsVector>(fileName); }


  DataPointsVector & DataPointsVector::operator+=(const DataPointsVector & rhs)
  {
  if (rhs._dataPoints.size()!=_dataPoints.size()) throw MathException("Data sets have different numbers of points",__FUNCTION__);
  std::vector<DataPoint2D> result(_dataPoints);
  for (std::size_t i=0; i<result.size(); i++) result[i] += rhs._dataPoints[i];
  _dataPoints.swap(result);
  return *this;
  }

  DataPointsVector & DataPointsVector::operator-=(const DataPointsVector & rhs)
  {
  if (rhs._dataPoints.size()!=_dataPoints.size()) throw MathException("Data sets have different numbers of points",__FUNCTION__);
  std::vector<DataPoint2D> result(_dataPoints);
  for (std::size_t i=0; i<result.size(); i++) result[i] -= rhs._dataPoints[i];
  _dataPoints.swap(result);
  return *this;
  }

  DataPointsVector operator+(const DataPointsVector & left, const DataPointsVector & right) { DataPointsVector r(left); r += right; return r; }
  DataPointsVector operator-(const DataPointsVector & left, const DataPointsVector & right) { DataPointsVector r(left); r -= right; return r; }

  } // namespace MATH
} // namespace CAP
