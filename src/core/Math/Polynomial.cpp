#include "Polynomial.hpp"
#include <iostream>
#include <string>
#include <algorithm>
#include <cmath>
#include "MathExceptions.hpp"

ClassImp(CAP::MATH::Polynomial);

namespace CAP
{
  namespace MATH
  {

  Polynomial::Polynomial()
  :
  ParameterizedFunction(),
  _fixedDegree(false)
  { }

  Polynomial::Polynomial(unsigned int degree)
  :
  ParameterizedFunction(coefficientParameters(std::vector<double>(degree+1,0.0))),
  _fixedDegree(false)
  { }

  Polynomial::Polynomial(const std::vector<double> & coefficients)
  :
  ParameterizedFunction(coefficientParameters(coefficients)),
  _fixedDegree(false)
  { }

  Polynomial::Polynomial(FixedDegree, unsigned int degree)
  :
  ParameterizedFunction(coefficientParameters(std::vector<double>(degree+1,0.0))),
  _fixedDegree(true)
  { }

  Polynomial::Polynomial(std::initializer_list<double> coefficients)
  :
  ParameterizedFunction(coefficientParameters(std::vector<double>(coefficients))),
  _fixedDegree(false)
  { }

  Polynomial::Polynomial(const Polynomial & source)
  :
  ParameterizedFunction(source),
  _fixedDegree(source._fixedDegree)
  { }

  Polynomial & Polynomial::operator=(const Polynomial & rhs)
  {
  if (this!=&rhs)
    {
    checkDegreeChange(rhs._parameters.size(),__FUNCTION__);
    ParameterizedFunction::operator=(rhs);
    }
  return *this;
  }

  void Polynomial::checkDegreeChange(unsigned int newNCoefficients, const char * source) const
  {
  if (_fixedDegree && newNCoefficients!=_parameters.size())
    throw MathException("Cannot change the degree of a polynomial of fixed degree",source);
  }

  String Polynomial::coefficientName(unsigned int k)
  {
  String name = "c";
  name += std::to_string(k).c_str();
  return name;
  }

  std::vector<Parameter> Polynomial::coefficientParameters(const std::vector<double> & coefficients)
  {
  std::vector<Parameter> parameters;
  parameters.reserve(coefficients.size());
  for (unsigned int k=0; k<coefficients.size(); k++) parameters.push_back(Parameter(coefficientName(k),coefficients[k]));
  return parameters;
  }

  void Polynomial::resizeCoefficients(unsigned int n)
  {
  if (n<_parameters.size())
    _parameters.erase(_parameters.begin()+n,_parameters.end());
  else
    while (_parameters.size()<n) _parameters.push_back(Parameter(coefficientName(_parameters.size()),0.0));
  }

  std::vector<double> Polynomial::coefficients() const
  {
  std::vector<double> values;
  values.reserve(_parameters.size());
  for (const auto & parameter : _parameters) values.push_back(parameter.value());
  return values;
  }

  // Horner's method: p(x) = c0 + x*(c1 + x*(c2 + ... + x*cn))
  double Polynomial::evaluateAt(double x) const
  {
  double value = 0.0;
  for (auto c = _parameters.rbegin(); c != _parameters.rend(); ++c) value = value*x + c->value();
  return value;
  }

  // Horner's method on the derivative coefficients k*c_k
  double Polynomial::derivativeAt(double x) const
  {
  double value = 0.0;
  for (unsigned int k=_parameters.size(); k-- > 1; ) value = value*x + double(k)*parameterValue(k);
  return value;
  }

  // Horner's method on c_k/(k+1), then one more factor x
  double Polynomial::antiderivativeAt(double x) const
  {
  double value = 0.0;
  for (unsigned int k=_parameters.size(); k-- > 0; ) value = value*x + parameterValue(k)/double(k+1);
  return value*x;
  }

  double Polynomial::integralInRange(double low, double high) const
  {
  return antiderivativeAt(high) - antiderivativeAt(low);
  }

  double Polynomial::expectationValue(double low, double high) const
  {
  double norm = integralInRange(low,high);
  if (norm==0.0) throw MathException("Integral of the polynomial over the range is zero",__FUNCTION__);
  // first moment: Q(x) = sum c_k x^(k+2)/(k+2), by Horner's method
  auto Q = [this](double x)
    {
    double value = 0.0;
    for (unsigned int k=_parameters.size(); k-- > 0; ) value = value*x + parameterValue(k)/double(k+2);
    return value*x*x;
    };
  return (Q(high) - Q(low))/norm;
  }

  double Polynomial::expectationValue(double low, double high, int power) const
  {
  if (power==1) return expectationValue(low,high);
  checkPowerRange(low,high,power,__FUNCTION__);
  double norm = integralInRange(low,high);
  if (norm==0.0) throw MathException("Integral of the polynomial over the range is zero",__FUNCTION__);
  if (power==0) return 1.0;
  double moment = 0.0;
  for (unsigned int k=0; k<_parameters.size(); k++)
    {
    double c = parameterValue(k);
    if (c==0.0) continue;
    int e = int(k) + power;   // integrate c_k x^e
    if (e==-1)
      moment += c*std::log(std::fabs(high)/std::fabs(low));   // same sign: checked above
    else
      moment += c*(std::pow(high,e+1) - std::pow(low,e+1))/double(e+1);
    }
  return moment/norm;
  }

  void Polynomial::setCoefficient(unsigned int k, double value)
  {
  if (k>=_parameters.size())
    {
    checkDegreeChange(k+1,__FUNCTION__);
    resizeCoefficients(k+1);
    }
  setParameterValue(k,value);
  }

  void Polynomial::setCoefficients(const std::vector<double> & coefficients)
  {
  checkDegreeChange(coefficients.size(),__FUNCTION__);
  resizeCoefficients(coefficients.size());
  for (unsigned int k=0; k<coefficients.size(); k++) setParameterValue(k,coefficients[k]);
  }

  void Polynomial::setDegree(unsigned int degree)
  {
  checkDegreeChange(degree+1,__FUNCTION__);
  resizeCoefficients(degree+1);
  }

  void Polynomial::clear()
  {
  if (_fixedDegree)
    for (auto & parameter : _parameters) parameter.setValue(0.0);
  else
    _parameters.clear();
  }

  void Polynomial::print() const
  {
  std::cout << *this << std::endl;
  }

  std::ostream& operator<<(std::ostream& out, const Polynomial & p)
  {
  if (p.nCoefficients()==0) { out << "0"; return out; }
  for (unsigned int k=0; k<p.nCoefficients(); k++)
    {
    if (k>0) out << " + ";
    out << p.coefficient(k);
    if (k==1) out << "*x";
    else if (k>1) out << "*x^" << k;
    }
  return out;
  }

  } // namespace MATH
} // namespace CAP
