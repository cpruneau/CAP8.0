#include "Parameter.hpp"
#include "MathExceptions.hpp"

ClassImp(CAP::MATH::Parameter);

namespace CAP
{
  namespace MATH
  {

  Parameter::Parameter()
  :
  _name(),
  _value(0.0),
  _type(Free),
  _minimum(defaultMinimum),
  _maximum(defaultMaximum)
  {}

  Parameter::Parameter(const String & name,
                       double value,
                       ParameterType type,
                       double minimum,
                       double maximum)
  :
  _name(name),
  _value(value),
  _type(type),
  _minimum(minimum),
  _maximum(maximum)
  {
  if (isLowConstrained() && isHighConstrained() && _minimum>_maximum)
    throw InvalidBoundsException(_minimum,_maximum,"Minimum larger than maximum for parameter "+_name,__FUNCTION__);
  if ((isLowConstrained() && _value<_minimum) || (isHighConstrained() && _value>_maximum))
    throw MathDomainException(_minimum,_maximum,_value,"Value outside the bounds of parameter "+_name,__FUNCTION__);
  }

  Parameter::Parameter(const Parameter & source)
  :
  _name(source._name),
  _value(source._value),
  _type(source._type),
  _minimum(source._minimum),
  _maximum(source._maximum)
  {}

  Parameter & Parameter::operator=(const Parameter & rhs)
  {
  if (this != &rhs)
    {
    _name    = rhs._name;
    _value   = rhs._value;
    _type    = rhs._type;
    _minimum = rhs._minimum;
    _maximum = rhs._maximum;
    }
  return *this;
  }

  void Parameter::setValue(double value)
  {
  if ((isLowConstrained() && value<_minimum) || (isHighConstrained() && value>_maximum))
    throw MathDomainException(_minimum,_maximum,value,"Value outside the bounds of parameter "+_name,__FUNCTION__);
  _value = value;
  }

  void Parameter::setMinimum(double minimum)
  {
  if (_type==Fixed)
    throw MathException("Cannot set the minimum of fixed parameter "+_name,__FUNCTION__);
  if (isHighConstrained() && minimum>_maximum)
    throw InvalidBoundsException(minimum,_maximum,"Minimum larger than maximum for parameter "+_name,__FUNCTION__);
  if (_value<minimum)
    throw MathDomainException(minimum,_maximum,_value,"Current value below the new minimum of parameter "+_name,__FUNCTION__);
  _minimum = minimum;
  if      (_type==Free) _type = Low;
  else if (_type==High) _type = LowHigh;
  }

  void Parameter::setMaximum(double maximum)
  {
  if (_type==Fixed)
    throw MathException("Cannot set the maximum of fixed parameter "+_name,__FUNCTION__);
  if (isLowConstrained() && maximum<_minimum)
    throw InvalidBoundsException(_minimum,maximum,"Maximum smaller than minimum for parameter "+_name,__FUNCTION__);
  if (_value>maximum)
    throw MathDomainException(_minimum,maximum,_value,"Current value above the new maximum of parameter "+_name,__FUNCTION__);
  _maximum = maximum;
  if      (_type==Free) _type = High;
  else if (_type==Low)  _type = LowHigh;
  }

  bool Parameter::smaller(const Parameter & other) const
  {
  return _value < other._value;
  }

  bool Parameter::greater(const Parameter & other) const
  {
  return _value > other._value;
  }

  bool Parameter::equal(const Parameter & other) const
  {
  return _value == other._value;
  }

  bool Parameter::notEqual(const Parameter & other) const
  {
  return !equal(other);
  }

  void Parameter::encodeBinary(BinaryWriter & w) const
  {
  w.writeFloat64(_value);
  w.writeVarUInt((unsigned int)_type);
  w.writeFloat64(_minimum);
  w.writeFloat64(_maximum);
  }

  void Parameter::decodeBinary(BinaryReader & r, const String & parameterName, unsigned int /*version*/)
  {
  _name    = parameterName;
  _value   = r.readFloat64();
  uint64_t type = r.readVarUInt();
  if (type > (uint64_t)High) throw Exception("Invalid parameter type","Parameter::decodeBinary()");
  _type    = ParameterType(type);
  _minimum = r.readFloat64();
  _maximum = r.readFloat64();
  }

  void Parameter::writeBinary(std::ostream & out) const                     { writeBinaryObject(out,*this); }
  bool Parameter::readBinary(std::istream & in)                             { return readBinaryObject(in,*this); }
  void Parameter::saveBinary(const String & fileName, bool append) const    { saveBinaryObject(fileName,*this,append); }
  void Parameter::loadBinary(const String & fileName, const String & name)   { loadBinaryObject(fileName,*this,name); }
  void Parameter::saveBinary(const String & fileName, const std::vector<Parameter> & objects) { saveBinaryObjects(fileName,objects); }
  std::vector<Parameter> Parameter::loadAllBinary(const String & fileName)       { return loadAllBinaryObjects<Parameter>(fileName); }

  } // namespace MATH
} // namespace CAP
