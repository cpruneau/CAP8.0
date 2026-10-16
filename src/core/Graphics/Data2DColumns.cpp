#include "Data2DColumns.hpp"
#include "Folder.hpp"
#include "MathExceptions.hpp"

ClassImp(CAP::GRAPHICS::Data2DColumns);

namespace CAP
{
  namespace GRAPHICS
  {

  Data2DColumns::Data2DColumns()
  :
  Data2D(),
  _x(),
  _xError(),
  _y(),
  _yStatError(),
  _ySysError()
  {
  setType(kColumn);
  }

  Data2DColumns::Data2DColumns(std::vector<float> & x,
                               std::vector<float> & y)
  :
  Data2D(),
  _x(x),
  _xError(),
  _y(y),
  _yStatError(),
  _ySysError()
  {
  if (_xError.empty())     _xError.assign(_x.size(), 0.0f);
  if (_yStatError.empty()) _yStatError.assign(_x.size(), 0.0f);
  if (_ySysError.empty())  _ySysError.assign(_x.size(), 0.0f);
  }

  Data2DColumns::Data2DColumns(std::vector<float> & x,
                               std::vector<float> & y,
                               std::vector<float> & yStatError)
  :
  Data2D(),
  _x(x),
  _xError(),
  _y(y),
  _yStatError(yStatError),
  _ySysError()
  {
  if (_xError.empty())     _xError.assign(_x.size(), 0.0f);
  if (_yStatError.empty()) _yStatError.assign(_x.size(), 0.0f);
  if (_ySysError.empty())  _ySysError.assign(_x.size(), 0.0f);
  }

  Data2DColumns::Data2DColumns(std::vector<float> & x,
                               std::vector<float> & y,
                               std::vector<float> & yStatError,
                               std::vector<float> & ySystError)
  :
  Data2D(),
  _x(x),
  _xError(),
  _y(y),
  _yStatError(yStatError),
  _ySysError(ySystError)
  {
  if (_xError.empty())     _xError.assign(_x.size(), 0.0f);
  if (_yStatError.empty()) _yStatError.assign(_x.size(), 0.0f);
  if (_ySysError.empty())  _ySysError.assign(_x.size(), 0.0f);
  }

  Data2DColumns::Data2DColumns(std::vector<float> & x,
                               std::vector<float> & xError,
                               std::vector<float> & y,
                               std::vector<float> & yStatError,
                               std::vector<float> & ySystError)
  :
  Data2D(),
  _x(x),
  _xError(xError),
  _y(y),
  _yStatError(yStatError),
  _ySysError(ySystError)
  {
  if (_xError.empty())     _xError.assign(_x.size(), 0.0f);
  if (_yStatError.empty()) _yStatError.assign(_x.size(), 0.0f);
  if (_ySysError.empty())  _ySysError.assign(_x.size(), 0.0f);
  }

  Data2DColumns::Data2DColumns(const Data2DColumns & src)
  :
  Data2D(src),
  _x(src._x),
  _xError(src._xError),
  _y(src._y),
  _yStatError(src._yStatError),
  _ySysError(src._ySysError)
  {  }

  Data2DColumns  & Data2DColumns::operator=(const Data2DColumns & rhs)
  {
  if (this!=&rhs)
    {
    Data2D::operator=(rhs);
    _x      = rhs._x;
    _xError = rhs._xError;
    _y      = rhs._y;
    _yStatError = rhs._yStatError;
    _ySysError  = rhs._ySysError;
    }
  return *this;
  }

  float Data2DColumns::xMinimum() const
  {
  float min = 1.0E30;
  for (auto value : _x)
    {
    if (value<min) min = value;
    }
  return min;
  }

  float Data2DColumns::xMaximum() const
  {
  float max = -1.0E30;
  for (auto value : _x)
    {
    if (value > max) max = value;
    }
  return max;
  }

  float Data2DColumns::xAtYMaximum() const
  {
  float max = -1.0E30;
  float x = 1.0E30;
  for (unsigned int k=0; k<_y.size(); k++)
    {
    float y = _y[k];
    if (y > max)
      {
      max = y;
      x = _x[k];
      }
    }
  return x;
  }

  float Data2DColumns::xAtYMinimum() const
  {
  float min = 1.0E30;
  float x = 1.0E30;
  for (unsigned int k=0; k<_y.size(); k++)
    {
    float y = _y[k];
    if (y<min)
      {
      min = y;
      x = _x[k];
      }
    }
  return x;
  }

  float Data2DColumns::yMinimum() const
  {
  float min = 1.0E30;
  for (auto value : _y)
    {
    if (value<min) min = value;
    }
  return min;
  }

  float Data2DColumns::yMaximum() const
  {
  float max = -1.0E30;
  for (auto value : _y)
    {
    if (value > max) max = value;
    }
  return max;
  }

  float Data2DColumns::yAtXMaximum() const
  {
  float max = -1.0E30;
  float y = 1.0E30;
  for (unsigned int k=0; k<_x.size(); k++)
    {
    float x = _x[k];
    if (x > max)
      {
      max = x;
      y = _y[k];
      }
    }
  return y;
  }


  float Data2DColumns::yAtXMinimum() const
  {
  float min = 1.0E30;
  float y = 1.0E30;
  for (unsigned int k=0; k<_x.size(); k++)
    {
    float x = _x[k];
    if (x < min)
      {
      min = x;
      y = _y[k];
      }
    }
  return y;
  }


  void Data2DColumns::addPoint(float x, float xError, float y, float yStatError, float ySysError)
  {
  _x.push_back(x);
  _xError.push_back(xError);
  _y.push_back(y);
  _yStatError.push_back(yStatError);
  _ySysError.push_back(ySysError);
  }

  void Data2DColumns::addPoint(const DataPoint2D & dataPoint)
  {
  _x.push_back(dataPoint.x());
  _xError.push_back(dataPoint.xError());
  _y.push_back(dataPoint.y());
  _yStatError.push_back(dataPoint.yStatError());
  _ySysError.push_back(dataPoint.ySysError());
  }

  void Data2DColumns::encodeBinary(CAP::BinaryWriter & w) const
  {
  encodeBinaryDescription(w);
  const std::vector<float> * columns[5] = {&_x,&_xError,&_y,&_yStatError,&_ySysError};
  std::size_t n = _x.size();
  for (auto c : columns) if (c->size()!=n) throw Exception("Columns of different lengths","Data2DColumns::encodeBinary()");
  w.writeVarUInt(n);
  // per point: a mask byte telling which of the five values are non-zero, then those values
  for (std::size_t i=0; i<n; i++)
    {
    uint8_t mask = 0;
    for (int k=0; k<5; k++) { uint32_t u; std::memcpy(&u,&(*columns[k])[i],4); if (u!=0) mask |= uint8_t(1u<<k); }
    w.writeUInt8(mask);
    for (int k=0; k<5; k++) if (mask & (1u<<k)) w.writeFloat32((*columns[k])[i]);
    }
  }

  void Data2DColumns::decodeBinary(CAP::BinaryReader & r, const String & dataName, unsigned int /*version*/)
  {
  decodeBinaryDescription(r,dataName);
  uint64_t n = r.readVarUInt();
  if (n > r.remaining()) throw Exception("More points than data","Data2DColumns::decodeBinary()");
  std::vector<float> * columns[5] = {&_x,&_xError,&_y,&_yStatError,&_ySysError};
  for (auto c : columns) c->assign(std::size_t(n),0.0f);
  for (std::size_t i=0; i<n; i++)
    {
    uint8_t mask = r.readUInt8();
    if (mask & 0xE0) throw Exception("Invalid point mask","Data2DColumns::decodeBinary()");
    for (int k=0; k<5; k++) if (mask & (1u<<k)) (*columns[k])[i] = r.readFloat32();
    }
  }

  void Data2DColumns::writeBinary(std::ostream & out) const                     { writeBinaryObject(out,*this); }
  bool Data2DColumns::readBinary(std::istream & in)                             { return readBinaryObject(in,*this); }
  void Data2DColumns::saveBinary(const String & fileName, bool append) const    { saveBinaryObject(fileName,*this,append); }
  void Data2DColumns::loadBinary(const String & fileName, const String & name)   { loadBinaryObject(fileName,*this,name); }
  void Data2DColumns::saveBinary(const String & fileName, const std::vector<Data2DColumns> & objects) { saveBinaryObjects(fileName,objects); }
  std::vector<Data2DColumns> Data2DColumns::loadAllBinary(const String & fileName)       { return loadAllBinaryObjects<Data2DColumns>(fileName); }

  namespace
  {
  // make Data2DColumns objects loadable from binary files of folders (CAP::Folder)
  [[maybe_unused]] const bool registeredInFolder = (CAP::Folder::registerType<Data2DColumns>(), true);
  }

  namespace
  {
  Data2DColumns combineColumns(const Data2DColumns & left, const Data2DColumns & right, bool subtract)
  {
  if (left.size()!=right.size()) throw CAP::MATH::MathException("Data sets have different numbers of points","Data2DColumns");
  std::vector<DataPoint2D> points;
  for (unsigned int i=0; i<left.size(); i++) points.push_back(subtract ? left.point(i)-right.point(i) : left.point(i)+right.point(i));
  Data2DColumns result(left);      // keeps the description of left
  result.setColumns(points);
  return result;
  }
  }

  Data2DColumns operator+(const Data2DColumns & left, const Data2DColumns & right) { return combineColumns(left,right,false); }
  Data2DColumns operator-(const Data2DColumns & left, const Data2DColumns & right) { return combineColumns(left,right,true); }

  void Data2DColumns::setColumns(const std::vector<DataPoint2D> & points)
  {
  _x.clear(); _xError.clear(); _y.clear(); _yStatError.clear(); _ySysError.clear();
  for (const auto & p : points) addPoint(p);
  }

  } //  namespace GRAPHICS
} // namespace CAP
