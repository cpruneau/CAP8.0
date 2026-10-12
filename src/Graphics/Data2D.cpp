#include "Data2D.hpp"
#include "Exceptions.hpp"

ClassImp(CAP::GRAPHICS::Data2D);

namespace CAP
{
  namespace GRAPHICS
  {

  Data2D::Data2D()
  :
  CAP::MATH::DataPoints("","","Data2D"),
  _type(kColumn),
  _xName(),
  _yName(),
  _xTitle(),
  _yTitle(),
  _labels()
  {  }

  Data2D::Data2D(DataType type)
  :
  CAP::MATH::DataPoints("","","Data2D"),
  _type(type),
  _xName(),
  _yName(),
  _xTitle(),
  _yTitle(),
  _labels()
  {  }

  Data2D::Data2D(const Data2D & src)
  :
  CAP::MATH::DataPoints(src),
  _type(src._type),
  _xName(src._xName),
  _yName(src._yName),
  _xTitle(src._xTitle),
  _yTitle(src._yTitle),
  _labels(src._labels)
  {  }

  Data2D  & Data2D::operator=(const Data2D & rhs)
  {
  if (this!= &rhs)
    {
    _type = rhs._type;
    CAP::MATH::DataPoints::operator=(rhs);
    _xName = rhs._xName;
    _yName = rhs._yName;
    _xTitle = rhs._xTitle;
    _yTitle = rhs._yTitle;
    _labels = rhs._labels;
    }
  return *this;
  }

  //!
  //! Replace the label at position index. Throws if index is out of range.
  //!
  void Data2D::setLabel(unsigned int index, const String & s)
  {
  if (index >= _labels.size()) throw Exception("Label index out of range", __FUNCTION__);
  _labels[index] = s;
  }

  //!
  //! Return the label at position index. Throws if index is out of range.
  //!
  const String & Data2D::label(unsigned int index) const
  {
  if (index >= _labels.size()) throw Exception("Label index out of range", __FUNCTION__);
  return _labels[index];
  }

  void Data2D::addPoints(std::vector<float> & x,
                         std::vector<float> & xError,
                         std::vector<float> & y,
                         std::vector<float> & yStatError,
                         std::vector<float> & ySystError)
  {
  if (xError.size()!= x.size() && xError.size()!=0) throw Exception("Incompatible sizes for x and xError", __FUNCTION__);
  if (y.size() != x.size()) throw Exception("Incompatible sizes for x and y", __FUNCTION__);
  if (yStatError.size()!= x.size() && yStatError.size()!=0) throw Exception("Incompatible sizes for x and yStatError", __FUNCTION__);
  if (ySystError.size()!= x.size() && ySystError.size()!=0) throw Exception("Incompatible sizes for x and ySystError", __FUNCTION__);
  for (unsigned int k=0; k<x.size(); k++)
    {
    float dx     = xError.empty()     ? 0.0f : xError[k];
    float dyStat = yStatError.empty() ? 0.0f : yStatError[k];
    float dySys  = ySystError.empty() ? 0.0f : ySystError[k];
    addPoint(x[k], dx, y[k], dyStat, dySys);
    }
  }


  void Data2D::encodeBinaryDescription(CAP::BinaryWriter & w) const
  {
  w.writeVarUInt((unsigned int)_type);
  w.writeString(_xName);
  w.writeString(_yName);
  w.writeString(_title);
  w.writeString(_xTitle);
  w.writeString(_yTitle);
  w.writeVarUInt(_labels.size());
  for (const auto & label : _labels) w.writeString(label);
  }

  void Data2D::decodeBinaryDescription(CAP::BinaryReader & r, const String & dataName)
  {
  uint64_t type = r.readVarUInt();
  if (type > (uint64_t)kRow) throw Exception("Invalid Data2D type","Data2D::decodeBinaryDescription()");
  _type   = DataType(type);
  _name   = dataName;
  _xName  = r.readString();
  _yName  = r.readString();
  _title  = r.readString();
  _xTitle = r.readString();
  _yTitle = r.readString();
  uint64_t nLabels = r.readVarUInt();
  if (nLabels > r.remaining()) throw Exception("More labels than data","Data2D::decodeBinaryDescription()");
  _labels.clear();
  for (uint64_t i=0; i<nLabels; i++) _labels.push_back(r.readString());
  }

  } //  namespace GRAPHICS
} // namespace CAP
