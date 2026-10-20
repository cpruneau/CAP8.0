/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file FitRecord.cpp
//! @brief Implementation of FitRecord.
#include <cstdio>
#include <limits>
#include <string>
#include <memory>
#include "BinaryIO.hpp"
#include "FitRecord.hpp"
#include "FunctionRegistry.hpp"
#include "ParameterizedFunction.hpp"
#include "MathExceptions.hpp"

namespace CAP
{
  namespace MATH
  {

  double FitRecord::valueAt(double x) const
  {
  if (!function) return std::numeric_limits<double>::quiet_NaN();
  try { return function->evaluateAt(x); }
  catch (...) { return std::numeric_limits<double>::quiet_NaN(); }
  }

  double FitRecord::valueAt(const std::vector<double> & x) const
  {
  if (!function) return std::numeric_limits<double>::quiet_NaN();
  try { return function->evaluateAt(x); }
  catch (...) { return std::numeric_limits<double>::quiet_NaN(); }
  }

  namespace
  {
  std::string number(double v, const char * format="%.6g")
  {
  char text[48];
  std::snprintf(text, sizeof(text), format, v);
  return text;
  }
  } // namespace

  bool FitRecord::inRange(const std::vector<double> & x) const
  {
  if (low.empty() || low.size() != x.size()) return false;
  for (std::size_t k=0; k<x.size(); k++) if (x[k] < low[k] || x[k] > high[k]) return false;
  return true;
  }

  bool FitRecord::chiSquareLike() const { return !result.statistic.contains("unbinned"); }

  String FitRecord::parameterName(unsigned int k) const
  {
  if (function && k < function->nParameters() && !function->parameter(k).name().isNull()) return function->parameter(k).name();
  return String(("p" + std::to_string(k)).c_str());
  }

  void FitRecord::parameterAt(unsigned int k, double & value, double & error) const
  {
  value = k < result.values.size() ? result.values[k] : (function && k < function->nParameters() ? function->parameter(k).value() : 0.0);
  error = k < result.errors.size() ? result.errors[k] : 0.0;
  }

  std::vector<std::pair<String,String>> FitRecord::quality() const
  {
  const FitResult & r = result;
  std::vector<std::pair<String,String>> rows;
  auto add = [&rows](const std::string & name, const std::string & value) { rows.push_back({ String(name.c_str()), String(value.c_str()) }); };
  bool chi2 = chiSquareLike();
  add(chi2 ? "\xCF\x87\xC2\xB2" : "-2 ln L", number(r.chiSquare));
  add("degrees of freedom", std::to_string(r.nDegreesOfFreedom));
  if (chi2)
    {
    add("\xCF\x87\xC2\xB2/ndf", r.nDegreesOfFreedom > 0 ? number(r.chiSquarePerDegreeOfFreedom, "%.4g") : "-");
    add("p-value", r.nDegreesOfFreedom > 0 ? number(r.pValue, "%.4g") : "-");
    }
  add("points", std::to_string(r.nPoints));
  add("converged", r.converged ? "yes" : "no");
  add("uncertainties", r.validErrors ? "valid" : "not available");
  if (!r.status.isNull()) add("status", r.status.data());
  add("statistic", r.statistic.isNull() ? "-" : r.statistic.data());
  add("method", r.method.isNull() ? "-" : r.method.data());
  add("evaluations", std::to_string(r.nEvaluations));
  add("iterations", std::to_string(r.nIterations));
  if (!low.empty())
    {
    std::string range;
    for (std::size_t i=0; i<low.size(); i++) range += (i ? " x " : "") + std::string("[") + number(low[i]) + ", " + number(high[i]) + "]";
    add("range", range);
    }
  return rows;
  }

  String FitRecord::text() const
  {
  std::string text = std::string(label.data()) + "\t" + (function ? FunctionRegistry::typeName(*function).data() : "(no function)") + "\n";
  for (const auto & row : quality()) text += std::string(row.first.data()) + "\t" + row.second.data() + "\n";
  if (!function) return String(text.c_str());
  text += "parameter\tvalue\tuncertainty\tbounds\n";
  for (unsigned int k=0; k<function->nParameters(); k++)
    {
    double value, error;
    parameterAt(k, value, error);
    text += std::string(parameterName(k).data()) + "\t" + number(value) + "\t" + number(error) + "\t" + function->parameter(k).boundsText().data() + "\n";
    }
  const Matrix & c = result.correlation;
  const std::vector<unsigned int> & free = result.freeParameters;
  if (!free.empty() && c.nRows() == free.size() && c.nColumns() == free.size())
    {
    text += "correlations";
    for (unsigned int j : free) text += std::string("\t") + parameterName(j).data();
    text += "\n";
    for (std::size_t i=0; i<free.size(); i++)
      {
      text += parameterName(free[i]).data();
      for (std::size_t j=0; j<free.size(); j++) text += "\t" + number(c.values()[i*free.size() + j], "%.4f");
      text += "\n";
      }
    }
  return String(text.c_str());
  }

  const FitRecord & FitList::at(unsigned int k) const
  {
  if (k >= _records.size()) throw MathException("Fit index out of range", __FUNCTION__);
  return *_records[k];
  }

  std::shared_ptr<const FitRecord> FitList::pointer(unsigned int k) const
  {
  if (k >= _records.size()) throw MathException("Fit index out of range", __FUNCTION__);
  return _records[k];
  }

  void FitList::remove(unsigned int k)
  {
  if (k >= _records.size()) throw MathException("Fit index out of range", __FUNCTION__);
  _records.erase(_records.begin() + k);
  }


  // ---------------------------------------------------------------- binary encoding

  namespace
  {

  const uint8_t kFitsVersion = 1;   // format of the fits section

  void writeMatrix(BinaryWriter & w, const Matrix & m) { m.encodeBinary(w); }

  Matrix readMatrix(BinaryReader & r)
  {
  Matrix m;
  m.decodeBinary(r, "", Matrix::binaryVersion());
  return m;
  }

  void writeResult(BinaryWriter & w, const FitResult & result)
  {
  w.writeUInt8(uint8_t((result.converged ? 1 : 0) | (result.validErrors ? 2 : 0)));
  w.writeString(result.status);
  w.writeString(result.method);
  w.writeString(result.statistic);
  w.writeFloat64(result.chiSquare);
  w.writeVarInt(result.nPoints);
  w.writeVarInt(result.nDegreesOfFreedom);
  w.writeFloat64(result.chiSquarePerDegreeOfFreedom);
  w.writeFloat64(result.pValue);
  writeDoubles(w, result.values);
  writeDoubles(w, result.errors);
  w.writeVarUInt(result.atLimit.size());
  for (bool b : result.atLimit) w.writeUInt8(b ? 1 : 0);
  w.writeVarUInt(result.freeParameters.size());
  for (unsigned int k : result.freeParameters) w.writeVarUInt(k);
  writeMatrix(w, result.covariance);
  writeMatrix(w, result.correlation);
  w.writeVarInt(result.nEvaluations);
  w.writeVarInt(result.nIterations);
  }

  FitResult readResult(BinaryReader & r)
  {
  FitResult result;
  uint8_t flags = r.readUInt8();
  if (flags > 3) throw Exception("Invalid fit flags", "FitList::decodeBinary()");
  result.converged   = (flags & 1) != 0;
  result.validErrors = (flags & 2) != 0;
  result.status    = r.readString();
  result.method    = r.readString();
  result.statistic = r.readString();
  result.chiSquare = r.readFloat64();
  result.nPoints           = int(r.readVarInt());
  result.nDegreesOfFreedom = int(r.readVarInt());
  result.chiSquarePerDegreeOfFreedom = r.readFloat64();
  result.pValue = r.readFloat64();
  result.values = readDoubles(r);
  result.errors = readDoubles(r);
  uint64_t n = r.readVarUInt();
  if (n > r.remaining()) throw Exception("More flags than data", "FitList::decodeBinary()");
  for (uint64_t k=0; k<n; k++) result.atLimit.push_back(r.readUInt8() != 0);
  n = r.readVarUInt();
  if (n > r.remaining()) throw Exception("More parameter indices than data", "FitList::decodeBinary()");
  for (uint64_t k=0; k<n; k++) result.freeParameters.push_back((unsigned int)(r.readVarUInt()));
  result.covariance  = readMatrix(r);
  result.correlation = readMatrix(r);
  result.nEvaluations = long(r.readVarInt());
  result.nIterations  = long(r.readVarInt());
  return result;
  }

  void writeRecord(BinaryWriter & w, const FitRecord & record)
  {
  const ParameterizedFunction & f = *record.function;
  w.writeString(FunctionRegistry::typeName(f));
  w.writeVarUInt(f.nParameters());
  w.writeVarUInt(f.rank());
  w.writeString(f.name());
  w.writeString(f.title());
  for (const Parameter & p : f.parameters()) { w.writeString(p.name()); p.encodeBinary(w); }
  w.writeString(record.label);
  writeResult(w, record.result);
  writeDoubles(w, record.low);
  writeDoubles(w, record.high);
  }

  //! The record of a block, or null if the class of its function is not registered.
  std::shared_ptr<const FitRecord> readRecord(BinaryReader & r)
  {
  String type = r.readString();
  uint64_t nParameters = r.readVarUInt();
  uint64_t rank = r.readVarUInt();
  if (nParameters > r.remaining() || rank == 0 || rank > 64) throw Exception("Invalid fit function", "FitList::decodeBinary()");
  std::unique_ptr<ParameterizedFunction> f(FunctionRegistry::create(type, (unsigned int)nParameters));
  if (!f || f->rank() != rank) return nullptr;
  f->setName(r.readString());
  f->setTitle(r.readString());
  for (unsigned int k=0; k<nParameters; k++)
    {
    Parameter p;
    String name = r.readString();
    p.decodeBinary(r, name, Parameter::binaryVersion());
    f->setParameter(k, p);
    }
  auto record = std::make_shared<FitRecord>();
  record->label  = r.readString();
  record->result = readResult(r);
  record->low  = readDoubles(r);
  record->high = readDoubles(r);
  if (record->low.size() != record->high.size()) throw Exception("Invalid fit range", "FitList::decodeBinary()");
  record->function.reset(f.release());
  return record;
  }

  } // namespace

  void FitList::encodeBinary(BinaryWriter & w) const
  {
  w.writeUInt8(kFitsVersion);
  uint64_t n = 0;
  for (const auto & record : _records) if (record && record->function) n++;
  w.writeVarUInt(n);
  for (const auto & record : _records)
    {
    if (!record || !record->function) continue;
    BinaryWriter block;
    writeRecord(block, *record);
    w.writeVarUInt(block.buffer().size());
    w.writeBytes(block.buffer().data(), block.buffer().size());
    }
  }

  unsigned int FitList::decodeBinary(BinaryReader & r)
  {
  uint8_t version = r.readUInt8();
  if (version == 0 || version > kFitsVersion)
    throw Exception("Fits of an unsupported format version (written by a newer version?)", "FitList::decodeBinary()");
  uint64_t n = r.readVarUInt();
  if (n > r.remaining()) throw Exception("More fits than data", "FitList::decodeBinary()");
  std::vector<std::shared_ptr<const FitRecord>> records;
  unsigned int skipped = 0;
  for (uint64_t k=0; k<n; k++)
    {
    uint64_t length = r.readVarUInt();
    if (length > r.remaining()) throw Exception("Fit larger than the data", "FitList::decodeBinary()");
    BinaryReader block(r.readSpan(std::size_t(length)), std::size_t(length));
    std::shared_ptr<const FitRecord> record = readRecord(block);
    if (record) records.push_back(record);
    else skipped++;
    }
  _records.swap(records);
  return skipped;
  }

  } // namespace MATH
} // namespace CAP
