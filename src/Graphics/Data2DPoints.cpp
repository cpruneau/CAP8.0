#include "Data2DPoints.hpp"
#include "Folder.hpp"
#include "MathExceptions.hpp"

ClassImp(CAP::GRAPHICS::Data2DPoints);

namespace CAP
{
  namespace GRAPHICS
  {

  Data2DPoints::Data2DPoints()
  :
  Data2D(),
  _points()
  {
  setType(kRow);
  }

  Data2DPoints::Data2DPoints(const Data2DPoints & src)
  :
  Data2D(src),
  _points(src._points)
  {  }

  Data2DPoints & Data2DPoints::operator=(const Data2DPoints & rhs)
  {
  if (this!=&rhs)
    {
    Data2D::operator=(rhs);
    _points = rhs._points;
    }
  return *this;
  }

  void Data2DPoints::addPoint(float x, float xError, float y, float yStatError, float ySysError)
  {
  _points.addPoint(x,xError,y,yStatError,ySysError);
  }

  void Data2DPoints::addPoint(const DataPoint2D & dataPoint)
  {
  _points.addPoint(dataPoint);
  }

  void Data2DPoints::addPoints(const std::vector<DataPoint2D> & dataPoints)
  {
  _points.addPoints(dataPoints);
  }

  // Extrema: delegate to the Math container; keep the historical Graphics values when empty.
  float Data2DPoints::xMinimum()    const { return _points.empty() ?  1.0E30f : _points.xMinimum();    }
  float Data2DPoints::xMaximum()    const { return _points.empty() ? -1.0E30f : _points.xMaximum();    }
  float Data2DPoints::xAtYMaximum() const { return _points.empty() ?  1.0E30f : _points.xAtYMaximum(); }
  float Data2DPoints::xAtYMinimum() const { return _points.empty() ?  1.0E30f : _points.xAtYMinimum(); }
  float Data2DPoints::yMinimum()    const { return _points.empty() ?  1.0E30f : _points.yMinimum();    }
  float Data2DPoints::yMaximum()    const { return _points.empty() ? -1.0E30f : _points.yMaximum();    }
  float Data2DPoints::yAtXMaximum() const { return _points.empty() ?  1.0E30f : _points.yAtXMaximum(); }
  float Data2DPoints::yAtXMinimum() const { return _points.empty() ?  1.0E30f : _points.yAtXMinimum(); }

  std::vector<DataPoint2D> Data2DPoints::dataPoints() const
  {
  std::vector<DataPoint2D> result;
  result.reserve(_points.size());
  for (unsigned int k=0; k<_points.size(); k++) result.push_back(_points.point(k));
  return result;
  }

  void Data2DPoints::encodeBinary(CAP::BinaryWriter & w) const
  {
  encodeBinaryDescription(w);
  _points.encodeBinary(w);
  }

  void Data2DPoints::decodeBinary(CAP::BinaryReader & r, const String & dataName, unsigned int version)
  {
  decodeBinaryDescription(r,dataName);
  _points.decodeBinary(r,"",version);
  }

  void Data2DPoints::writeBinary(std::ostream & out) const                     { writeBinaryObject(out,*this); }
  bool Data2DPoints::readBinary(std::istream & in)                             { return readBinaryObject(in,*this); }
  void Data2DPoints::saveBinary(const String & fileName, bool append) const    { saveBinaryObject(fileName,*this,append); }
  void Data2DPoints::loadBinary(const String & fileName, const String & name)   { loadBinaryObject(fileName,*this,name); }
  void Data2DPoints::saveBinary(const String & fileName, const std::vector<Data2DPoints> & objects) { saveBinaryObjects(fileName,objects); }
  std::vector<Data2DPoints> Data2DPoints::loadAllBinary(const String & fileName)       { return loadAllBinaryObjects<Data2DPoints>(fileName); }

  namespace
  {
  // make Data2DPoints objects loadable from binary files of folders (CAP::Folder)
  [[maybe_unused]] const bool registeredInFolder = (CAP::Folder::registerType<Data2DPoints>(), true);
  }

  namespace
  {
  Data2DPoints combinePoints(const Data2DPoints & left, const Data2DPoints & right, bool subtract)
  {
  if (left.size()!=right.size()) throw CAP::MATH::MathException("Data sets have different numbers of points","Data2DPoints");
  Data2DPoints result(left);
  if (subtract) result.points() -= right.points(); else result.points() += right.points();
  return result;
  }
  }

  Data2DPoints operator+(const Data2DPoints & left, const Data2DPoints & right) { return combinePoints(left,right,false); }
  Data2DPoints operator-(const Data2DPoints & left, const Data2DPoints & right) { return combinePoints(left,right,true); }

  } //  namespace GRAPHICS
} // namespace CAP
