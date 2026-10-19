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
#include <limits>
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
