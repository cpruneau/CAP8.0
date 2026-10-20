#include "ScalarField.hpp"
#include <cmath>
#include <string>
#include "MathExceptions.hpp"

ClassImp(CAP::MATH::ScalarField);

namespace CAP
{
  namespace MATH
  {

  ScalarField::ScalarField()
  :
  Function("Null","Null","ScalarField",1),
  _constValue(0.0),
  _minima(), _maxima(), _nPoints(), _values()
  { }

  ScalarField::ScalarField(const String & name, double value, unsigned int rank)
  :
  Function(name,name,"ScalarField",rank),
  _constValue(value),
  _minima(), _maxima(), _nPoints(), _values()
  { }

  ScalarField::ScalarField(const String & name,
                           const std::vector<double> & minima,
                           const std::vector<double> & maxima,
                           const std::vector<unsigned int> & nPoints,
                           double initialValue)
  :
  Function(name,name,"ScalarField",1),
  _constValue(0.0),
  _minima(), _maxima(), _nPoints(), _values()
  {
  setMesh(minima,maxima,nPoints,initialValue);
  }

  ScalarField::ScalarField(const String & name,
                           double xMin, double xMax, unsigned int xNPts,
                           double yMin, double yMax, unsigned int yNPts,
                           double zMin, double zMax, unsigned int zNPts,
                           double initialValue)
  :
  Function(name,name,"ScalarField",3),
  _constValue(0.0),
  _minima(), _maxima(), _nPoints(), _values()
  {
  setValue(name,xMin,xMax,xNPts,yMin,yMax,yNPts,zMin,zMax,zNPts,initialValue);
  }

  ScalarField::ScalarField(const ScalarField & source)
  :
  Function(source),
  _constValue(source._constValue),
  _minima(source._minima),
  _maxima(source._maxima),
  _nPoints(source._nPoints),
  _values(source._values)
  { }

  ScalarField & ScalarField::operator=(const ScalarField & rhs)
  {
  if (this!=&rhs)
    {
    Function::operator=(rhs);
    _constValue = rhs._constValue;
    _minima     = rhs._minima;
    _maxima     = rhs._maxima;
    _nPoints    = rhs._nPoints;
    _values     = rhs._values;
    }
  return *this;
  }

  // ---------------------------------------------------------------- setting the field

  void ScalarField::setValue(double value)
  {
  _minima.clear();
  _maxima.clear();
  _nPoints.clear();
  _values.clear();
  _constValue = value;
  }

  void ScalarField::setValue(const ScalarField & source)
  {
  if (this==&source) return;
  Function::operator=(source);
  _constValue = source._constValue;
  _minima     = source._minima;
  _maxima     = source._maxima;
  _nPoints    = source._nPoints;
  _values     = source._values;
  }

  void ScalarField::setValue(const String & name,
                             double xMin, double xMax, unsigned int xNPts,
                             double initialValue)
  {
  if (xMax<=xMin) throw InvalidBoundsException(xMin,xMax,"Invalid input: xMin>=xMax",__FUNCTION__);
  setMesh({xMin},{xMax},{xNPts<1 ? 1u : xNPts},initialValue);
  setName(name);
  setTitle(name);
  }

  void ScalarField::setValue(const String & name,
                             double xMin, double xMax, unsigned int xNPts,
                             double yMin, double yMax, unsigned int yNPts,
                             double initialValue)
  {
  if (xMax<=xMin) throw InvalidBoundsException(xMin,xMax,"Invalid input: xMin>=xMax",__FUNCTION__);
  if (yMax<=yMin) throw InvalidBoundsException(yMin,yMax,"Invalid input: yMin>=yMax",__FUNCTION__);
  setMesh({xMin,yMin},{xMax,yMax},{xNPts<1 ? 1u : xNPts, yNPts<1 ? 1u : yNPts},initialValue);
  setName(name);
  setTitle(name);
  }

  void ScalarField::setValue(const String & name,
                             double xMin, double xMax, unsigned int xNPts,
                             double yMin, double yMax, unsigned int yNPts,
                             double zMin, double zMax, unsigned int zNPts,
                             double initialValue)
  {
  if (xMax<=xMin) throw InvalidBoundsException(xMin,xMax,"Invalid input: xMin>=xMax",__FUNCTION__);
  if (yMax<=yMin) throw InvalidBoundsException(yMin,yMax,"Invalid input: yMin>=yMax",__FUNCTION__);
  if (zMax<=zMin) throw InvalidBoundsException(zMin,zMax,"Invalid input: zMin>=zMax",__FUNCTION__);
  setMesh({xMin,yMin,zMin},{xMax,yMax,zMax},
          {xNPts<1 ? 1u : xNPts, yNPts<1 ? 1u : yNPts, zNPts<1 ? 1u : zNPts},
          initialValue);
  setName(name);
  setTitle(name);
  }

  void ScalarField::setMesh(const std::vector<double> & minima,
                            const std::vector<double> & maxima,
                            const std::vector<unsigned int> & nPoints,
                            double initialValue)
  {
  if (minima.empty()) throw MathException("The mesh must have at least one axis",__FUNCTION__);
  if (maxima.size()!=minima.size() || nPoints.size()!=minima.size())
    throw MathException("minima, maxima and nPoints must have the same size",__FUNCTION__);
  unsigned long long total = 1;
  for (unsigned int n : nPoints)
    {
    if (n==0) throw MathException("Number of points along an axis must be at least 1",__FUNCTION__);
    total *= n;
    if (total > 0xFFFFFFFFull) throw MathException("Too many mesh nodes",__FUNCTION__);
    }
  // check with the new numbers of points
  for (unsigned int k=0; k<minima.size(); k++)
    {
    if (!std::isfinite(minima[k]) || !std::isfinite(maxima[k]))
      throw InvalidBoundsException(minima[k],maxima[k],"Mesh boundaries must be finite",__FUNCTION__);
    if (nPoints[k]>1 && maxima[k]<=minima[k])
      throw InvalidBoundsException(minima[k],maxima[k],"Maximum must be larger than minimum",__FUNCTION__);
    }
  Function::setRank(minima.size());
  _minima  = minima;
  _maxima  = maxima;
  _nPoints = nPoints;
  _values.assign(total,initialValue);
  _constValue = 0.0;
  }

  void ScalarField::setRank(unsigned int rank)
  {
  if (!isConstant() && rank!=_minima.size())
    throw MathException("Cannot change the rank of a mesh field: use setMesh()",__FUNCTION__);
  Function::setRank(rank);
  }

  void ScalarField::reset()
  {
  _constValue = 0.0;
  for (auto & v : _values) v = 0.0;
  }

  // ---------------------------------------------------------------- mesh boundaries and nodes

  void ScalarField::checkAxis(unsigned int axis, const char * source) const
  {
  if (axis>=_minima.size())
    {
    String message = isConstant() ? "A constant field has no mesh axis " : "No mesh axis ";
    message += std::to_string(axis).c_str();
    throw MathException(message,source);
    }
  }

  void ScalarField::checkBounds(const std::vector<double> & minima, const std::vector<double> & maxima, const char * source) const
  {
  for (unsigned int k=0; k<minima.size(); k++)
    {
    if (!std::isfinite(minima[k]) || !std::isfinite(maxima[k]))
      throw InvalidBoundsException(minima[k],maxima[k],"Mesh boundaries must be finite",source);
    if (_nPoints[k]>1 && maxima[k]<=minima[k])
      throw InvalidBoundsException(minima[k],maxima[k],"Maximum must be larger than minimum",source);
    }
  }

  double ScalarField::minimum(unsigned int axis) const       { checkAxis(axis,__FUNCTION__); return _minima[axis]; }
  double ScalarField::maximum(unsigned int axis) const       { checkAxis(axis,__FUNCTION__); return _maxima[axis]; }
  unsigned int ScalarField::nPoints(unsigned int axis) const { checkAxis(axis,__FUNCTION__); return _nPoints[axis]; }

  double ScalarField::spacing(unsigned int axis) const
  {
  checkAxis(axis,__FUNCTION__);
  return _nPoints[axis]>1 ? (_maxima[axis]-_minima[axis])/double(_nPoints[axis]-1) : 0.0;
  }

  double ScalarField::coordinate(unsigned int axis, unsigned int index) const
  {
  checkAxis(axis,__FUNCTION__);
  if (index>=_nPoints[axis]) throw IndexOutOfBoundException(index,_nPoints[axis],"Node index out of range",__FUNCTION__);
  return _minima[axis] + index*spacing(axis);
  }

  void ScalarField::setMinima(const std::vector<double> & minima)
  {
  if (isConstant()) throw MathException("A constant field has no mesh",__FUNCTION__);
  if (minima.size()!=_minima.size()) throw MathException("Size of minima differs from the mesh rank",__FUNCTION__);
  checkBounds(minima,_maxima,__FUNCTION__);
  _minima = minima;
  }

  void ScalarField::setMaxima(const std::vector<double> & maxima)
  {
  if (isConstant()) throw MathException("A constant field has no mesh",__FUNCTION__);
  if (maxima.size()!=_maxima.size()) throw MathException("Size of maxima differs from the mesh rank",__FUNCTION__);
  checkBounds(_minima,maxima,__FUNCTION__);
  _maxima = maxima;
  }

  void ScalarField::setMinimum(unsigned int axis, double minimum)
  {
  checkAxis(axis,__FUNCTION__);
  std::vector<double> minima(_minima);
  minima[axis] = minimum;
  checkBounds(minima,_maxima,__FUNCTION__);
  _minima = minima;
  }

  void ScalarField::setMaximum(unsigned int axis, double maximum)
  {
  checkAxis(axis,__FUNCTION__);
  std::vector<double> maxima(_maxima);
  maxima[axis] = maximum;
  checkBounds(_minima,maxima,__FUNCTION__);
  _maxima = maxima;
  }

  double ScalarField::volume() const
  {
  double v = 1.0;
  for (unsigned int k=0; k<_minima.size(); k++) v *= _maxima[k]-_minima[k];
  return v;
  }

  double ScalarField::random(unsigned int axis, double r) const
  {
  checkAxis(axis,__FUNCTION__);
  return _minima[axis] + (_maxima[axis]-_minima[axis])*r;
  }

  // ---------------------------------------------------------------- node values

  unsigned int ScalarField::nodeIndex(const std::vector<unsigned int> & indices, const char * source) const
  {
  if (indices.size()!=_nPoints.size())
    {
    String message = "Mesh of rank ";
    message += std::to_string(_nPoints.size()).c_str();
    message += " addressed with ";
    message += std::to_string(indices.size()).c_str();
    message += " index(es)";
    throw MathException(message,source);
    }
  unsigned int index  = 0;
  unsigned int stride = 1;
  for (unsigned int k=0; k<indices.size(); k++)
    {
    if (indices[k]>=_nPoints[k]) throw IndexOutOfBoundException(indices[k],_nPoints[k],"Node index out of range",source);
    index  += indices[k]*stride;
    stride *= _nPoints[k];
    }
  return index;
  }

  double & ScalarField::operator()(unsigned int i)
  {
  if (isConstant()) return _constValue;
  return _values[nodeIndex({i},__FUNCTION__)];
  }

  double & ScalarField::operator()(unsigned int i, unsigned int j)
  {
  if (isConstant()) return _constValue;
  return _values[nodeIndex({i,j},__FUNCTION__)];
  }

  double & ScalarField::operator()(unsigned int i, unsigned int j, unsigned int k)
  {
  if (isConstant()) return _constValue;
  return _values[nodeIndex({i,j,k},__FUNCTION__)];
  }

  double & ScalarField::operator()(const std::vector<unsigned int> & indices)
  {
  if (isConstant()) return _constValue;
  return _values[nodeIndex(indices,__FUNCTION__)];
  }

  double ScalarField::valueAt(const std::vector<unsigned int> & indices) const
  {
  if (isConstant()) return _constValue;
  return _values[nodeIndex(indices,__FUNCTION__)];
  }

  double ScalarField::valueAt(unsigned int i, unsigned int j, unsigned int k) const
  {
  return valueAt(std::vector<unsigned int>{i,j,k});
  }

  double ScalarField::valueAt(double x, double y, double z) const
  {
  return interpolate(x,y,z);
  }

  double ScalarField::value() const
  {
  if (!isConstant()) throw MathException("Not a constant field: specify a position or node indices",__FUNCTION__);
  return _constValue;
  }

  // ---------------------------------------------------------------- evaluation and interpolation

  double ScalarField::interpolateMesh(const double * x, const char * source) const
  {
  const unsigned int n = _nPoints.size();
  std::vector<unsigned int> base(n);
  std::vector<double> frac(n);
  std::vector<unsigned int> stride(n);
  unsigned int s = 1;
  for (unsigned int k=0; k<n; k++)
    {
    if (!std::isfinite(x[k])) throw MathException("Coordinate is not finite",source);
    stride[k] = s;
    s *= _nPoints[k];
    if (_nPoints[k]==1) { base[k] = 0; frac[k] = 0.0; continue; }
    double t = (x[k]-_minima[k])/spacing(k);
    double cell = std::floor(t);
    if (cell<0.0) cell = 0.0;
    if (cell>double(_nPoints[k]-2)) cell = double(_nPoints[k]-2);  // last cell; linear extrapolation beyond the edges
    base[k] = (unsigned int) cell;
    frac[k] = t - cell;
    }
  // sum over the 2^n corners of the cell; axes with a single node contribute one corner only
  double sum = 0.0;
  const unsigned long long nCorners = 1ull << n;
  for (unsigned long long corner=0; corner<nCorners; corner++)
    {
    double weight = 1.0;
    unsigned int index = 0;
    bool valid = true;
    for (unsigned int k=0; k<n; k++)
      {
      bool upper = (corner>>k) & 1ull;
      if (upper && _nPoints[k]==1) { valid = false; break; }
      weight *= upper ? frac[k] : 1.0-frac[k];
      index  += (base[k] + (upper ? 1 : 0))*stride[k];
      }
    if (valid) sum += weight*_values[index];
    }
  return sum;
  }

  double ScalarField::interpolate(double x) const
  {
  checkRank(1,__FUNCTION__);
  return isConstant() ? _constValue : interpolateMesh(&x,__FUNCTION__);
  }

  double ScalarField::interpolate(double x1, double x2) const
  {
  checkRank(2,__FUNCTION__);
  double x[2] = {x1,x2};
  return isConstant() ? _constValue : interpolateMesh(x,__FUNCTION__);
  }

  double ScalarField::interpolate(double x1, double x2, double x3) const
  {
  checkRank(3,__FUNCTION__);
  double x[3] = {x1,x2,x3};
  return isConstant() ? _constValue : interpolateMesh(x,__FUNCTION__);
  }

  double ScalarField::interpolate(const std::vector<double> & x) const
  {
  checkRank(x.size(),__FUNCTION__);
  return isConstant() ? _constValue : interpolateMesh(x.data(),__FUNCTION__);
  }

  double ScalarField::evaluateAt(double x) const                          { return interpolate(x); }
  double ScalarField::evaluateAt(double x1, double x2) const              { return interpolate(x1,x2); }
  double ScalarField::evaluateAt(double x1, double x2, double x3) const   { return interpolate(x1,x2,x3); }
  double ScalarField::evaluateAt(const std::vector<double> & x) const     { return interpolate(x); }

  // ---------------------------------------------------------------- derivatives

  ScalarField ScalarField::derivative(unsigned int axis, const String & name) const
  {
  checkAxis(axis,__FUNCTION__);
  const unsigned int n = _nPoints[axis];
  if (n<=1) throw DivByZeroException("nPoints<=1","Too few points along the axis",__FUNCTION__);
  ScalarField result(*this);
  result.setName(name);
  result.setTitle(name);
  const double h = spacing(axis);
  unsigned int stride = 1;
  for (unsigned int k=0; k<axis; k++) stride *= _nPoints[k];
  for (unsigned int g=0; g<_values.size(); g++)
    {
    unsigned int i = (g/stride)%n;   // node index along the axis
    const double * f = &_values[g - i*stride];   // f[m*stride] is node m along the axis
    double d;
    if (n==2)
      d = (f[stride] - f[0])/h;
    else if (i==0)
      d = (-3.0*f[0] + 4.0*f[stride] - f[2*stride])/(2.0*h);
    else if (i==n-1)
      d = (3.0*f[i*stride] - 4.0*f[(i-1)*stride] + f[(i-2)*stride])/(2.0*h);
    else
      d = (f[(i+1)*stride] - f[(i-1)*stride])/(2.0*h);
    result._values[g] = d;
    }
  return result;
  }
  void ScalarField::encodeBinary(BinaryWriter & w) const
  {
  w.writeString(title());
  w.writeVarUInt(rank());
  w.writeFloat64(_constValue);
  w.writeVarUInt(_minima.size());          // 0 for a constant field
  for (unsigned int k=0; k<_minima.size(); k++)
    {
    w.writeFloat64(_minima[k]);
    w.writeFloat64(_maxima[k]);
    w.writeVarUInt(_nPoints[k]);
    }
  if (!isConstant()) writeDoubles(w,_values);
  _fits.encodeBinary(w);   // format version 2: fits attached
  }

  void ScalarField::decodeBinary(BinaryReader & r, const String & fieldName, unsigned int version)
  {
  setName(fieldName);
  setTitle(r.readString());
  uint64_t rank = r.readVarUInt();
  double constValue = r.readFloat64();
  uint64_t meshRank = r.readVarUInt();
  if (rank==0 || rank > 64 || meshRank > 64) throw Exception("Invalid rank","ScalarField::decodeBinary()");
  if (meshRank==0)
    {
    setValue(constValue);
    setRank((unsigned int)rank);
    if (version>=2) _fits.decodeBinary(r);
    return;
    }
  if (meshRank!=rank) throw Exception("Mesh rank differs from the rank","ScalarField::decodeBinary()");
  std::vector<double> minima(meshRank), maxima(meshRank);
  std::vector<unsigned int> nPoints(meshRank);
  for (uint64_t k=0; k<meshRank; k++)
    {
    minima[k] = r.readFloat64();
    maxima[k] = r.readFloat64();
    uint64_t n = r.readVarUInt();
    if (n==0 || n > 0xFFFFFFFFull) throw Exception("Invalid number of points","ScalarField::decodeBinary()");
    nPoints[k] = (unsigned int) n;
    }
  std::vector<double> values = readDoubles(r);
  setMesh(minima,maxima,nPoints);                    // validates the mesh
  if (values.size()!=_values.size()) throw Exception("Number of values differs from the number of nodes","ScalarField::decodeBinary()");
  _values = values;
  _constValue = constValue;
  if (version>=2) _fits.decodeBinary(r);
  }

  void ScalarField::writeBinary(std::ostream & out) const                     { writeBinaryObject(out,*this); }
  bool ScalarField::readBinary(std::istream & in)                             { return readBinaryObject(in,*this); }
  void ScalarField::saveBinary(const String & fileName, bool append) const    { saveBinaryObject(fileName,*this,append); }
  void ScalarField::loadBinary(const String & fileName, const String & name)   { loadBinaryObject(fileName,*this,name); }
  void ScalarField::saveBinary(const String & fileName, const std::vector<ScalarField> & objects) { saveBinaryObjects(fileName,objects); }
  std::vector<ScalarField> ScalarField::loadAllBinary(const String & fileName)       { return loadAllBinaryObjects<ScalarField>(fileName); }


  void ScalarField::combineWith(const ScalarField & rhs, double sign, const char * source)
  {
  if (rank()!=rhs.rank()) throw MathException("Fields have different ranks",source);
  if (rhs.isConstant())
    {
    if (isConstant()) _constValue += sign*rhs._constValue;
    else for (auto & v : _values) v += sign*rhs._constValue;
    return;
    }
  if (isConstant())
    {
    // constant +- mesh: the result is defined on the mesh of rhs
    std::vector<double> values(rhs._values.size());
    for (std::size_t k=0; k<values.size(); k++) values[k] = _constValue + sign*rhs._values[k];
    _minima = rhs._minima; _maxima = rhs._maxima; _nPoints = rhs._nPoints; _values.swap(values); _constValue = 0.0;
    return;
    }
  if (_minima!=rhs._minima || _maxima!=rhs._maxima || _nPoints!=rhs._nPoints)
    throw MathException("Fields are defined on different meshes",source);
  for (std::size_t k=0; k<_values.size(); k++) _values[k] += sign*rhs._values[k];
  }

  ScalarField & ScalarField::operator+=(const ScalarField & rhs) { combineWith(rhs,+1.0,__FUNCTION__); return *this; }
  ScalarField & ScalarField::operator-=(const ScalarField & rhs) { combineWith(rhs,-1.0,__FUNCTION__); return *this; }

  ScalarField operator+(const ScalarField & left, const ScalarField & right) { ScalarField r(left); r += right; return r; }
  ScalarField operator-(const ScalarField & left, const ScalarField & right) { ScalarField r(left); r -= right; return r; }

  } // namespace MATH
} // namespace CAP
