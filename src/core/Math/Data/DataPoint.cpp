/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "DataPoint.hpp"
#include "MathExceptions.hpp"
#include <algorithm>

ClassImp(CAP::MATH::DataPoint);

namespace CAP
{
  namespace MATH
  {

  namespace
  {
  // Relative tolerance used for compatibility checks.
  constexpr double kRelTolerance = 1.0E-6;
  }

  DataPoint::DataPoint()
  :
  _value(0.0), _stat(0.0), _syst(0.0)
  { }

  DataPoint::DataPoint(double value, double stat, double syst)
  :
  _value(value), _stat(stat), _syst(syst)
  {
  if (stat<0) throw MathException("stat<0",__FUNCTION__);
  if (syst<0) throw MathException("syst<0",__FUNCTION__);
  }

  DataPoint::DataPoint(const DataPoint & source)
  :
  _value(source._value), _stat(source._stat), _syst(source._syst)
  { }

  DataPoint & DataPoint::operator=(const DataPoint & rhs)
  {
  if (this!=&rhs)
    {
    _value = rhs._value;
    _stat  = rhs._stat;
    _syst  = rhs._syst;
    }
  return *this;
  }

  std::vector<double> DataPoint::xValues() const
  {
  std::vector<double> values(rank());
  for (unsigned int i=0; i<values.size(); i++) values[i] = x(i);
  return values;
  }

  std::vector<double> DataPoint::xErrors() const
  {
  std::vector<double> errors(rank());
  for (unsigned int i=0; i<errors.size(); i++) errors[i] = xError(i);
  return errors;
  }

  void DataPoint::setValue(double value, double stat, double syst)
  {
  if (stat<0) throw MathException("stat<0",__FUNCTION__);
  if (syst<0) throw MathException("syst<0",__FUNCTION__);
  _value = value;
  _stat  = stat;
  _syst  = syst;
  }

  void DataPoint::setStat(double stat)
  {
  if (stat<0) throw MathException("stat<0",__FUNCTION__);
  _stat = stat;
  }

  void DataPoint::setSyst(double syst)
  {
  if (syst<0) throw MathException("syst<0",__FUNCTION__);
  _syst = syst;
  }

  bool DataPoint::compatibleX(const DataPoint & other, double nSigma) const
  {
  unsigned int n = rank();
  if (other.rank()!=n) return false;
  for (unsigned int i=0; i<n; i++)
    {
    double x1 = x(i), x2 = other.x(i);
    double e1 = xError(i), e2 = other.xError(i);
    double dx    = std::fabs(x1-x2);
    double error = std::sqrt(e1*e1 + e2*e2);
    double tol   = kRelTolerance*std::max(std::fabs(x1),std::fabs(x2));
    if (dx > nSigma*error + tol) return false;
    }
  return true;
  }

  bool DataPoint::operator==(const DataPoint & rhs) const
  {
  if (!compatibleX(rhs)) return false;
  double e1  = error();
  double e2  = rhs.error();
  double dv  = std::fabs(double(_value) - rhs._value);
  double tol = kRelTolerance*std::max(std::fabs(double(_value)),std::fabs(double(rhs._value)));
  return dv <= 2.0*std::sqrt(e1*e1+e2*e2) + tol;
  }

  void DataPoint::scale(double coefficient)
  {
  double a = std::fabs(coefficient);
  _value *= coefficient;
  _stat  *= a;
  _syst  *= a;
  }

  void DataPoint::inverse()
  {
  if (_value==0) throw MathException("Cannot invert a point with value==0",__FUNCTION__);
  double v   = _value;
  double vSq = v*v;
  _value = 1.0/v;
  _stat  = _stat/vSq;
  _syst  = _syst/vSq;
  }

  void DataPoint::clear()
  {
  _value = 0.0;
  _stat  = 0.0;
  _syst  = 0.0;
  for (unsigned int i=0; i<rank(); i++)
    {
    setX(i,0.0);
    setXError(i,0.0);
    }
  }

  void DataPoint::print(std::ostream & out) const
  {
  for (unsigned int i=0; i<rank(); i++)
    out << "x[" << i << "]: " << x(i) << " +- " << xError(i) << "  ";
  out << "value: " << _value << " +- " << _stat << " (stat) +- " << _syst << " (syst)";
  }

  void DataPoint::print() const
  {
  print(std::cout);
  std::cout << std::endl;
  }

  void DataPoint::requireCompatible(const DataPoint & rhs, const char * source) const
  {
  if (rhs.rank()!=rank()) throw MathException("Operation on data points of different ranks",source);
  if (!compatibleX(rhs))  throw MathException("Operation on data points with incompatible independent variables",source);
  }

  void DataPoint::checkIndex(unsigned int index, const char * source) const
  {
  if (index>=rank()) throw IndexOutOfBoundException(int(index),int(rank()),"index>=rank()",source);
  }

  void DataPoint::combine(const DataPoint & p1, const DataPoint & p2, double value, double d1, double d2)
  {
  // Read everything first: p1 or p2 may be this point.
  unsigned int n = rank();
  std::vector<double> xMean(n), xErrorMean(n);
  for (unsigned int i=0; i<n; i++)
    {
    xMean[i]      = 0.5*(double(p1.x(i))      + p2.x(i));
    xErrorMean[i] = 0.5*(double(p1.xError(i)) + p2.xError(i));
    }
  double s1 = d1*p1._stat, s2 = d2*p2._stat;
  double t1 = d1*p1._syst, t2 = d2*p2._syst;
  for (unsigned int i=0; i<n; i++)
    {
    setX(i,xMean[i]);
    setXError(i,xErrorMean[i]);
    }
  _value = value;
  _stat  = std::sqrt(s1*s1 + s2*s2);
  _syst  = std::sqrt(t1*t1 + t2*t2);
  }

  void DataPoint::add(const DataPoint & rhs)
  {
  requireCompatible(rhs,__FUNCTION__);
  combine(*this,rhs,double(_value)+rhs._value,1.0,1.0);
  }

  void DataPoint::subtract(const DataPoint & rhs)
  {
  requireCompatible(rhs,__FUNCTION__);
  combine(*this,rhs,double(_value)-rhs._value,1.0,-1.0);
  }

  void DataPoint::multiply(const DataPoint & rhs)
  {
  requireCompatible(rhs,__FUNCTION__);
  double v1 = _value, v2 = rhs._value;
  combine(*this,rhs,v1*v2,v2,v1);
  }

  void DataPoint::divide(const DataPoint & rhs)
  {
  requireCompatible(rhs,__FUNCTION__);
  if (rhs._value==0) throw MathException("Attempting division by a point with value==0",__FUNCTION__);
  double v1 = _value, v2 = rhs._value;
  combine(*this,rhs,v1/v2,1.0/v2,-v1/(v2*v2));
  }

  std::ostream & operator<<(std::ostream & out, const DataPoint & p)
  {
  p.print(out);
  return out;
  }

  } // namespace MATH
} // namespace CAP
