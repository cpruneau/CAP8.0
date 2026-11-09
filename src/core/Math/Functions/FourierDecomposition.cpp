/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FourierDecomposition.cpp
//! @brief Implementation of FourierHarmonic and FourierDecomposition.
#include <algorithm>
#include <cmath>
#include <string>
#include "FourierDecomposition.hpp"
#include "BinaryIO.hpp"
#include "Constant.hpp"
#include "Exceptions.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  namespace
  {
  String indexed(const char * name, unsigned int n) { return String((std::string(name) + std::to_string(n)).c_str()); }

  bool validForm(unsigned int form)
  {
  return form == FourierHarmonic::kCosineSine || form == FourierHarmonic::kFlow;
  }
  } // namespace

  // ---- FourierHarmonic

  FourierHarmonic::FourierHarmonic(unsigned int n, Form form, double a, double b)
  :
  ParameterizedFunction({Parameter("c", a), Parameter("s", b)}),
  _n(n),
  _form(form)
  {
  if (n == 0) throw MathException("The order of a Fourier harmonic is 1 or more", __FUNCTION__);
  if (!validForm(form)) throw MathException("Unknown form of a Fourier harmonic", __FUNCTION__);
  nameParameters();
  }

  void FourierHarmonic::nameParameters()
  {
  _parameters[0].setName(_form == kFlow ? "v" : "c");
  _parameters[1].setName(_form == kFlow ? "psi" : "s");
  }

  double FourierHarmonic::evaluateAt(double x) const
  {
  double a = parameterValue(0), b = parameterValue(1), n = _n;
  if (_form == kFlow) return a*std::cos(n*(x - b));
  return a*std::cos(n*x) + b*std::sin(n*x);
  }

  double FourierHarmonic::derivativeAt(double x) const
  {
  double a = parameterValue(0), b = parameterValue(1), n = _n;
  if (_form == kFlow) return -n*a*std::sin(n*(x - b));
  return n*(b*std::cos(n*x) - a*std::sin(n*x));
  }

  double FourierHarmonic::integralInRange(double low, double high) const
  {
  double a = parameterValue(0), b = parameterValue(1), n = _n;
  if (_form == kFlow) return a*(std::sin(n*(high - b)) - std::sin(n*(low - b)))/n;
  return (a*(std::sin(n*high) - std::sin(n*low)) - b*(std::cos(n*high) - std::cos(n*low)))/n;
  }

  void FourierHarmonic::encodeState(BinaryWriter & w) const
  {
  w.writeVarUInt(_n);
  w.writeVarUInt(unsigned(_form));
  }

  void FourierHarmonic::decodeState(BinaryReader & r, unsigned int version)
  {
  (void) version;
  uint64_t n = r.readVarUInt();
  uint64_t form = r.readVarUInt();
  if (n == 0 || n > 1000000) throw Exception("Invalid order of a Fourier harmonic", __FUNCTION__);
  if (!validForm(unsigned(form))) throw Exception("Invalid form of a Fourier harmonic", __FUNCTION__);
  _n = (unsigned int)n;
  _form = Form(form);
  nameParameters();
  }

  // ---- FourierDecomposition

  FourierDecomposition::FourierDecomposition(unsigned int nMax, Form form)
  :
  ParameterizedComposite(kSum),
  _form(form)
  {
  build(nMax, form);
  }

  FourierDecomposition::FourierDecomposition(Form form, double k, const std::vector<double> & a,
                                             const std::vector<double> & b)
  :
  ParameterizedComposite(kSum),
  _form(form)
  {
  build((unsigned int)std::max(a.size(), b.size()), form);
  setConstant(k);
  for (unsigned int n=1; n<=a.size(); n++) setParameterValue(harmonicIndex(n), a[n-1]);
  for (unsigned int n=1; n<=b.size(); n++) setParameterValue(harmonicIndex(n) + 1, b[n-1]);
  }

  void FourierDecomposition::build(unsigned int nMax, Form form)
  {
  if (nMax == 0) throw MathException("A Fourier decomposition has at least one harmonic", __FUNCTION__);
  if (!validForm(form)) throw MathException("Unknown form of a Fourier decomposition", __FUNCTION__);
  _components.clear();
  _labels.clear();
  _first.clear();
  _offsets.clear();
  _parameters.clear();
  _synchronized.clear();
  _form = form;
  add("k", Constant(0.0));
  for (unsigned int n=1; n<=nMax; n++) add(indexed("h", n), FourierHarmonic(n, form));
  // the names of the formula rather than "label.name"
  _parameters[0].setName("k");
  for (unsigned int n=1; n<=nMax; n++)
    {
    _parameters[2*n-1].setName(indexed(form == kFlow ? "v" : "c", n));
    _parameters[2*n].setName(indexed(form == kFlow ? "psi" : "s", n));
    }
  }

  // f = k + sum h_n (form 1) or k (1 + 2 sum h_n) (form 2); component 0 is the constant, component n harmonic n

  double FourierDecomposition::evaluateAt(double x) const
  {
  synchronize();
  double sum = 0.0;
  for (std::size_t n=1; n<_components.size(); n++) sum += _components[n]->evaluateAt(x);
  return _form == kFlow ? constant()*(1.0 + 2.0*sum) : constant() + sum;
  }

  double FourierDecomposition::evaluateAt(const std::vector<double> & x) const
  {
  checkRank((unsigned int)(x.size()), __FUNCTION__);
  return evaluateAt(x[0]);
  }

  double FourierDecomposition::derivativeAt(double x) const
  {
  synchronize();
  double sum = 0.0;
  for (std::size_t n=1; n<_components.size(); n++) sum += _components[n]->derivativeAt(x);
  return _form == kFlow ? 2.0*constant()*sum : sum;
  }

  double FourierDecomposition::integralInRange(double low, double high) const
  {
  synchronize();
  double sum = 0.0;
  for (std::size_t n=1; n<_components.size(); n++) sum += _components[n]->integralInRange(low, high);
  return _form == kFlow ? constant()*((high - low) + 2.0*sum) : constant()*(high - low) + sum;
  }

  double FourierDecomposition::integralInRange(const std::vector<double> & low, const std::vector<double> & high) const
  {
  checkRank((unsigned int)(low.size()), __FUNCTION__);
  checkRank((unsigned int)(high.size()), __FUNCTION__);
  return integralInRange(low[0], high[0]);
  }

  unsigned int FourierDecomposition::harmonicIndex(unsigned int n) const
  {
  if (n == 0 || n > nMax())
    throw MathException(String("Harmonic ") + String(std::to_string(n).c_str()) + " of a Fourier decomposition of order "
                        + String(std::to_string(nMax()).c_str()), __FUNCTION__);
  return 2*n - 1;
  }

  void FourierDecomposition::checkForm(Form wanted, const char * what) const
  {
  if (_form != wanted)
    throw MathException(wanted == kFlow ? "Flow coefficients are set in a decomposition of form 2 (kFlow)"
                                        : "Cosine and sine coefficients are set in a decomposition of form 1 (kCosineSine)",
                        what);
  }

  double FourierDecomposition::cosine(unsigned int n) const
  {
  unsigned int i = harmonicIndex(n);
  if (_form == kCosineSine) return parameterValue(i);
  return 2.0*constant()*parameterValue(i)*std::cos(n*parameterValue(i+1));
  }

  double FourierDecomposition::sine(unsigned int n) const
  {
  unsigned int i = harmonicIndex(n);
  if (_form == kCosineSine) return parameterValue(i+1);
  return 2.0*constant()*parameterValue(i)*std::sin(n*parameterValue(i+1));
  }

  double FourierDecomposition::flow(unsigned int n) const
  {
  unsigned int i = harmonicIndex(n);
  if (_form == kFlow) return parameterValue(i);
  if (constant() == 0.0) throw MathException("Flow coefficient of a decomposition whose constant is 0", __FUNCTION__);
  return std::hypot(parameterValue(i), parameterValue(i+1))/(2.0*constant());
  }

  double FourierDecomposition::flowPhase(unsigned int n) const
  {
  unsigned int i = harmonicIndex(n);
  if (_form == kFlow) return parameterValue(i+1);
  double c = parameterValue(i), s = parameterValue(i+1);
  if (c == 0.0 && s == 0.0) return 0.0;
  return std::atan2(s, c)/double(n);
  }

  void FourierDecomposition::setCosine(unsigned int n, double c)
  {
  checkForm(kCosineSine, __FUNCTION__);
  setParameterValue(harmonicIndex(n), c);
  }

  void FourierDecomposition::setSine(unsigned int n, double s)
  {
  checkForm(kCosineSine, __FUNCTION__);
  setParameterValue(harmonicIndex(n) + 1, s);
  }

  void FourierDecomposition::setFlow(unsigned int n, double v)
  {
  checkForm(kFlow, __FUNCTION__);
  setParameterValue(harmonicIndex(n), v);
  }

  void FourierDecomposition::setFlowPhase(unsigned int n, double psi)
  {
  checkForm(kFlow, __FUNCTION__);
  setParameterValue(harmonicIndex(n) + 1, psi);
  }

  void FourierDecomposition::encodeState(BinaryWriter & w) const
  {
  w.writeVarUInt(unsigned(_form));
  w.writeVarUInt(nMax());
  }

  void FourierDecomposition::decodeState(BinaryReader & r, unsigned int version)
  {
  (void) version;
  uint64_t form = r.readVarUInt();
  uint64_t n = r.readVarUInt();
  if (!validForm(unsigned(form))) throw Exception("Invalid form of a Fourier decomposition", __FUNCTION__);
  if (n != nMax())
    throw Exception("Fourier decomposition: the order recorded differs from the number of parameters", __FUNCTION__);
  if (Form(form) != _form) build((unsigned int)n, Form(form));
  }

  } // namespace MATH
} // namespace CAP
