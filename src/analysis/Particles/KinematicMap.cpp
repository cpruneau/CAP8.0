/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,04/01/2024
 *
 * *********************************************************************/
#include "KinematicMap.hpp"
#include "Exceptions.hpp"
#include "MathConstants.hpp"
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <string>

namespace CAP
{

  namespace
  {
    //! Items of a text separated by commas or white space.
    std::vector<std::string> items(const String & text)
    {
    std::string s = text.data();
    for (char & c : s) if (c==',' || c==';') c = ' ';
    std::istringstream in(s);
    std::vector<std::string> result;
    std::string item;
    while (in >> item) result.push_back(item);
    return result;
    }

    std::string lower(std::string s) { for (char & c : s) c = char(std::tolower(static_cast<unsigned char>(c))); return s; }
  }

  KinematicMap::KinematicMap()
  :
  _model(kNone),
  _constant(0.0),
  _function(),
  _histogram(nullptr),
  _variables(),
  _phiMinimum(0.0),
  _outsideValue(0.0)
  { }

  KinematicMap::KinematicMap(double value)
  :
  KinematicMap()
  {
  setConstant(value);
  }

  void KinematicMap::setNone()
  {
  _model = kNone;
  _function.reset();
  _histogram = nullptr;
  _variables.clear();
  }

  void KinematicMap::setConstant(double value)
  {
  setNone();
  _model    = kConstant;
  _constant = value;
  }

  void KinematicMap::setFunction(std::shared_ptr<const MATH::Function> function, const std::vector<Variable> & variables)
  {
  if (!function) throw Exception("No function","KinematicMap::setFunction()");
  std::vector<Variable> checked = checkedVariables(function->rank(),variables);
  setNone();
  _model     = kFunction;
  _function  = function;
  _variables = checked;
  }

  void KinematicMap::setHistogram(const MATH::Histogram * histogram, const std::vector<Variable> & variables)
  {
  if (!histogram) throw Exception("No histogram","KinematicMap::setHistogram()");
  std::vector<Variable> checked = checkedVariables(histogram->rank(),variables);
  setNone();
  _model     = kHistogram;
  _histogram = histogram;
  _variables = checked;
  for (unsigned int k=0; k<_variables.size(); k++)
    if (_variables[k]==kPhi) _phiMinimum = histogram->minimum(k);
  }

  void KinematicMap::expectHistogram(const std::vector<Variable> & variables)
  {
  std::vector<Variable> checked = checkedVariables(variables.size(),variables);
  setNone();
  _model     = kHistogram;
  _variables = checked;
  }

  double KinematicMap::value(double pt, double eta, double phi) const
  {
  switch (_model)
    {
    case kNone:     return 0.0;
    case kConstant: return _constant;
    default: break;
    }
  if (_model==kHistogram && !_histogram)
    throw Exception("The map has no histogram: create it (CREATE) or import it (IMPORT)","KinematicMap::value()");
  while (phi <  _phiMinimum)                  phi += MATH::twoPi();
  while (phi >= _phiMinimum + MATH::twoPi())  phi -= MATH::twoPi();
  const double values[3] = { pt, eta, phi };
  std::vector<double> x(_variables.size());
  for (unsigned int k=0; k<_variables.size(); k++) x[k] = values[_variables[k]];
  if (_model==kFunction) return _function->evaluateAt(x);
  int bin = _histogram->binIndex(x);
  return bin<0 ? _outsideValue : _histogram->binValueAtGlobal(bin);
  }

  std::vector<KinematicMap::Variable> KinematicMap::checkedVariables(unsigned int rank,
                                                                     const std::vector<Variable> & variables)
  {
  if (rank<1 || rank>3)
    throw Exception(String("A kinematic map depends on 1 to 3 variables (pT, eta, phi): rank ")
                    +String(std::to_string(rank).c_str()),"KinematicMap::checkedVariables()");
  if (variables.empty())
    {
    std::vector<Variable> defaults = { kPt, kEta, kPhi };
    return std::vector<Variable>(defaults.begin(),defaults.begin()+rank);
    }
  if (variables.size()!=rank)
    throw Exception(String("Number of variables (")+String(std::to_string(variables.size()).c_str())
                    +") different from the rank ("+String(std::to_string(rank).c_str())+")",
                    "KinematicMap::checkedVariables()");
  for (unsigned int k=0; k<variables.size(); k++)
    for (unsigned int j=0; j<k; j++)
      if (variables[j]==variables[k]) throw Exception("A variable is given twice","KinematicMap::checkedVariables()");
  return variables;
  }

  const char * KinematicMap::variableName(Variable v)
  {
  static const char * const names[3] = { "pt", "eta", "phi" };
  return names[v];
  }

  std::vector<KinematicMap::Variable> KinematicMap::parseVariables(const String & text)
  {
  std::vector<Variable> result;
  for (const std::string & item : items(text))
    {
    std::string name = lower(item);
    if      (name=="pt")  result.push_back(kPt);
    else if (name=="eta") result.push_back(kEta);
    else if (name=="phi") result.push_back(kPhi);
    else throw Exception(String("Unknown variable ")+item.c_str()+" (pt, eta or phi)","KinematicMap::parseVariables()");
    }
  return result;
  }

  String KinematicMap::variablesText(const std::vector<Variable> & variables)
  {
  std::string text;
  for (unsigned int k=0; k<variables.size(); k++) { if (k) text += ","; text += variableName(variables[k]); }
  return String(text.c_str());
  }

  KinematicMap::Model KinematicMap::parseModel(const String & text)
  {
  std::string name = lower(text.data());
  if (name=="none" || name.empty()) return kNone;
  if (name=="constant")             return kConstant;
  if (name=="function")             return kFunction;
  if (name=="histogram")            return kHistogram;
  throw Exception(String("Unknown model ")+text+" (NONE, CONSTANT, FUNCTION or HISTOGRAM)","KinematicMap::parseModel()");
  }

  const char * KinematicMap::modelName(Model model)
  {
  static const char * const names[4] = { "NONE", "CONSTANT", "FUNCTION", "HISTOGRAM" };
  return names[model];
  }

  std::vector<double> KinematicMap::parseNumbers(const String & text)
  {
  std::vector<double> result;
  for (const std::string & item : items(text))
    {
    char * end = nullptr;
    double v = std::strtod(item.c_str(),&end);
    if (!end || *end!='\0') throw Exception(String("Not a number: ")+item.c_str(),"KinematicMap::parseNumbers()");
    result.push_back(v);
    }
  return result;
  }

} // namespace CAP
