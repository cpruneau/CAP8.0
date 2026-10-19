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
//! @file TestFitStorage.cpp
//! @brief Test program of the storage of fits in CAP files (FunctionRegistry, FitList::encodeBinary()) (run by ctest).
//
// TestFitStorage -- tests of CAP::MATH::FunctionRegistry and of the fits saved with the objects they are attached to:
// histograms (in a CapFile, read eagerly and lazily), profiles, DataPointsVector, DataPointColumns and ScalarField
// records, fits of unregistered classes, records written before the fits were stored, and invalid data. Prints each
// failed check and exits with the number of failures (0: all passed).
//
#include <cmath>
#include <cstdio>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "BinaryIO.hpp"
#include "CapFile.hpp"
#include "DataPointColumns.hpp"
#include "DataPointsVector.hpp"
#include "Fitter.hpp"
#include "FunctionRegistry.hpp"
#include "Histogram.hpp"
#include "Linear.hpp"
#include "MathExceptions.hpp"
#include "Normal.hpp"
#include "Parabolic.hpp"
#include "Polynomial.hpp"
#include "Profile.hpp"
#include "ScalarField.hpp"

using namespace CAP;
using namespace CAP::MATH;

namespace
{
int nChecks = 0, nFailures = 0;

void check(bool ok, const std::string & what, int line)
{
  nChecks++;
  if (ok) return;
  nFailures++;
  std::printf("FAILED (line %d): %s\n", line, what.c_str());
}

//! A function of a user: a x + b, registered during the test.
class UserLine : public ParameterizedFunction
{
public:
  UserLine() : ParameterizedFunction({ Parameter("a", 1.0), Parameter("b", 0.0) }, 1) { }
  double evaluateAt(const std::vector<double> & x) const override { checkRank(x.size(), __FUNCTION__); return parameterValue(0)*x[0] + parameterValue(1); }
  using ParameterizedFunction::evaluateAt;
  UserLine * clone() const override { return new UserLine(*this); }
};

//! A function of a user that is never registered.
class Unregistered : public UserLine
{
public:
  Unregistered * clone() const override { return new Unregistered(*this); }
};

bool sameMatrix(const Matrix & a, const Matrix & b)
{
  return a.nRows() == b.nRows() && a.nColumns() == b.nColumns() && a.values() == b.values();
}

bool sameResult(const FitResult & a, const FitResult & b)
{
  return a.converged == b.converged && a.validErrors == b.validErrors && a.status == b.status && a.method == b.method
      && a.statistic == b.statistic && a.chiSquare == b.chiSquare && a.nPoints == b.nPoints
      && a.nDegreesOfFreedom == b.nDegreesOfFreedom && a.chiSquarePerDegreeOfFreedom == b.chiSquarePerDegreeOfFreedom
      && a.pValue == b.pValue && a.values == b.values && a.errors == b.errors && a.atLimit == b.atLimit
      && a.freeParameters == b.freeParameters && sameMatrix(a.covariance, b.covariance)
      && sameMatrix(a.correlation, b.correlation) && a.nEvaluations == b.nEvaluations && a.nIterations == b.nIterations;
}

//! Same label, class, parameters (names, values, types, bounds), result and range.
bool sameRecord(const FitRecord & a, const FitRecord & b)
{
  if (!a.function || !b.function) return false;
  const ParameterizedFunction & fa = *a.function;
  const ParameterizedFunction & fb = *b.function;
  if (typeid(fa) != typeid(fb) || fa.rank() != fb.rank()) return false;
  if (a.function->nParameters() != b.function->nParameters()) return false;
  for (unsigned int k=0; k<a.function->nParameters(); k++)
    {
    const Parameter & p = a.function->parameter(k);
    const Parameter & q = b.function->parameter(k);
    if (!(p.name() == q.name()) || p.value() != q.value() || p.type() != q.type() || p.minimum() != q.minimum() || p.maximum() != q.maximum())
      return false;
    }
  return a.label == b.label && sameResult(a.result, b.result) && a.low == b.low && a.high == b.high
      && a.valueAt(0.37) == b.valueAt(0.37);
}

bool sameFits(const DataPoints & a, const DataPoints & b)
{
  if (a.nFits() != b.nFits()) return false;
  for (unsigned int k=0; k<a.nFits(); k++) if (!sameRecord(a.fit(k), b.fit(k))) return false;
  return true;
}

//! A record of class T (its own tag and encoding), written and read back through a stream.
template <typename T>
T roundTrip(const T & object)
{
  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  writeBinaryObject(stream, object);
  stream.seekg(0);
  T copy;
  readBinaryObject(stream, copy);
  return copy;
}

//! Read a record of class T. @return true if one was read
template <typename T>
bool readRecord(const std::string & record, T & object)
{
  std::istringstream in(record, std::ios::binary);
  return readBinaryObject(in, object);
}

//! The record of an object, written with the format version given and without the last bytes of its payload.
template <typename T>
std::string olderRecord(const T & object, unsigned int version, std::size_t dropped)
{
  BinaryWriter w;
  w.writeString(object.binaryName());
  object.encodeBinary(w);
  std::string payload = w.buffer().substr(0, w.buffer().size() - dropped);
  std::ostringstream out(std::ios::binary);
  writeBinaryRecord(out, T::binaryTag(), version, payload, object.binaryName());
  return out.str();
}

//! Values of a Gaussian, sampled with a fixed seed.
std::vector<double> gaussianEvents(int n, double mean)
{
  std::vector<double> events;
  unsigned long long seed = 11;
  for (int k=0; k<n; k++)
    {
    seed = seed*6364136223846793005ULL + 1442695040888963407ULL;
    double u1 = ((seed >> 11) + 0.5)/9007199254740992.0;
    seed = seed*6364136223846793005ULL + 1442695040888963407ULL;
    double u2 = ((seed >> 11) + 0.5)/9007199254740992.0;
    events.push_back(mean + std::sqrt(-2*std::log(u1))*std::cos(2*M_PI*u2));
    }
  return events;
}
} // namespace

//! @brief Count a check, and report it if the condition is false.
#define CHECK(condition) check((condition), #condition, __LINE__)

//! @brief Run the checks. @return number of failed checks (0: all passed)
int main()
{
  // ---- the registry
  {
    std::vector<String> names = FunctionRegistry::names();
    CHECK(names.size() >= 9);
    for (const char * name : { "Constant", "Linear", "Parabolic", "Cubic", "Quartic", "Polynomial", "Normal", "LogNormal", "Exponential" })
      CHECK(FunctionRegistry::isRegistered(String(name)));
    std::unique_ptr<ParameterizedFunction> p(FunctionRegistry::create("Polynomial", 4));
    CHECK(p && dynamic_cast<Polynomial*>(p.get()) && p->nParameters() == 4);
    std::unique_ptr<ParameterizedFunction> n(FunctionRegistry::create("Normal", 3));
    CHECK(n && dynamic_cast<Normal*>(n.get()));
    CHECK(FunctionRegistry::create("Normal", 2) == nullptr);                // wrong number of parameters
    CHECK(FunctionRegistry::create("Polynomial", 0) == nullptr);
    CHECK(FunctionRegistry::create("NoSuchFunction", 1) == nullptr);
    CHECK(FunctionRegistry::typeName(Normal()) == "Normal" && FunctionRegistry::typeName(Linear()) == "Linear");
    UserLine user;
    CHECK(!FunctionRegistry::isRegistered(user) && FunctionRegistry::typeName(user).contains("UserLine"));
    FunctionRegistry::registerType<UserLine>("UserLine");
    CHECK(FunctionRegistry::isRegistered(user) && FunctionRegistry::typeName(user) == "UserLine");
    std::unique_ptr<ParameterizedFunction> u(FunctionRegistry::create("UserLine", 2));
    CHECK(u && dynamic_cast<UserLine*>(u.get()));
    bool thrown = false;
    try { FunctionRegistry::registerType("", typeid(UserLine), [](unsigned int) -> ParameterizedFunction * { return new UserLine(); }); }
    catch (const MathException &) { thrown = true; }
    CHECK(thrown);
  }

  // ---- a histogram with two fits, in a CAP file read eagerly and lazily
  Histogram h("h", "a gaussian", 24, -4.0, 4.0);
  for (double x : gaussianEvents(800, 0.3)) h.fill(x);
  {
    Normal model(40.0, 0.0, 1.0);
    Fitter fitter(model, h, Fitter::kPoisson);
    fitter.fit();
    h.addFit(fitter.record("gaussian"));
    Polynomial cubic(3);
    cubic.setParameterValues({ 30.0, 0.0, -5.0, 0.0 });
    cubic.fixParameter(3);                                                  // a fixed parameter
    cubic.setParameterLimits(1, -2.0, 2.0);                                 // a bounded one
    Fitter second(cubic, h);
    second.fit();
    h.addFit(second.record());
    CHECK(h.nFits() == 2 && h.fit(1).function->parameter(3).type() == Parameter::Fixed);
    CHECK(h.fit(0).result.validErrors && h.fit(0).result.covariance.nRows() == 3);
  }
  const char * path = "TestFitStorage.cap";
  {
    CapFile out(path, File::kWriteOnly);
    out.setFolderName("fits");
    out.addFolder("histograms")->add(new Histogram(h));
    out.write();
  }
  {
    CapFile in(path, File::kReadOnly);
    in.read();
    const Histogram * read = in.get<Histogram>("histograms/h");
    CHECK(read && sameFits(h, *read));
    CHECK(read && read->fit(0).label == "gaussian" && read->fit(1).label == h.fit(1).label);
    CHECK(read && dynamic_cast<const Normal*>(read->fit(0).function.get()) && dynamic_cast<const Polynomial*>(read->fit(1).function.get()));
    CHECK(read && read->fit(1).function->parameter(1).minimum() == -2.0 && read->fit(1).function->parameter(1).maximum() == 2.0);
  }
  {
    CapFile in(path, File::kReadOnly);
    in.load(File::Loading::kLazy);
    CHECK(!in.isLoaded("histograms/h"));
    const Histogram * read = in.get<Histogram>("histograms/h");
    CHECK(read && sameFits(h, *read));
  }
  std::remove(path);

  // ---- a profile
  {
    Profile p("p", "profile", 10, 0.0, 10.0);
    for (int k=0; k<100; k++) p.fill(0.1*k, 2.0 + 0.5*0.1*k + 0.1*std::sin(k), 1.0);
    Linear line(1.0, 1.0);
    Fitter fitter(line, p);
    fitter.fit();
    p.addFit(fitter.record("trend"));
    Profile read = roundTrip(p);
    CHECK(sameFits(p, read) && read.nBinsTotal() == p.nBinsTotal() && read.binValueAtGlobal(3) == p.binValueAtGlobal(3));
  }

  // ---- data points, in a vector and in columns
  DataPointsVector data;
  data.addPoints({0.0, 1.0, 2.0, 3.0, 4.0, 5.0}, {1.1, 2.9, 5.2, 6.8, 9.1, 11.3}, {0.1, 0.2, 0.1, 0.3, 0.2, 0.4});
  {
    Linear line(0.0, 1.0);
    Fitter fitter(line, data);
    fitter.fit();
    data.addFit(fitter.record());
    UserLine user;
    Fitter second(user, data);
    second.fit();
    data.addFit(second.record("user line"));
    DataPointsVector read = roundTrip(data);
    CHECK(sameFits(data, read) && read.size() == 6);
    CHECK(dynamic_cast<const UserLine*>(read.fit(1).function.get()) != nullptr);   // a registered class of a user

    DataPointsVector empty;
    empty.addFit(data.fitPointer(0));
    DataPointsVector readEmpty = roundTrip(empty);                         // fits without points
    CHECK(readEmpty.size() == 0 && sameFits(empty, readEmpty));
  }
  {
    DataPointColumns columns({0.0, 1.0, 2.0, 3.0}, {1.0, 3.1, 4.9, 7.2});
    columns.addFit(data.fitPointer(0));
    DataPointColumns read = roundTrip(columns);
    CHECK(sameFits(columns, read) && read.size() == 4);
  }

  // ---- a field, a Function, with a fit made through samples of it
  {
    ScalarField field("field", std::vector<double>{-1.0}, std::vector<double>{1.0}, std::vector<unsigned int>{21});
    Normal shape(1.0, 0.0, 1.0);
    for (unsigned int k=0; k<21; k++) field(k) = shape.evaluateAt(-1.0 + 0.1*k);
    Parabolic parabola(1.0, 0.0, -0.5);
    DataPointsVector samples = Fitter::samples(field, -1.0, 1.0, 21);   // the fitter keeps a reference to the data
    Fitter fitter(parabola, samples);
    fitter.fit();
    field.addFit(fitter.record("parabola"));
    ScalarField read = roundTrip(field);
    CHECK(read.nFits() == 1 && sameRecord(field.fit(0), read.fit(0)));
    ScalarField constant("c", 2.5);
    constant.addFit(field.fitPointer(0));
    ScalarField readConstant = roundTrip(constant);                        // a constant field
    CHECK(readConstant.isConstant() && readConstant.nFits() == 1 && sameRecord(constant.fit(0), readConstant.fit(0)));
  }

  // ---- a fit of a class that is not registered is skipped; the others are read
  {
    DataPointsVector mixed(data);
    mixed.clearFits();
    Unregistered unregistered;
    Fitter fitter(unregistered, data);
    fitter.fit();
    mixed.addFit(fitter.record("unregistered"));
    mixed.addFit(data.fitPointer(0));
    DataPointsVector read = roundTrip(mixed);
    CHECK(read.nFits() == 1 && sameRecord(read.fit(0), data.fit(0)));
    FitList list;
    list.add(mixed.fitPointer(0));
    list.add(mixed.fitPointer(1));
    BinaryWriter w;
    list.encodeBinary(w);
    FitList decoded;
    BinaryReader r(w.buffer().data(), w.buffer().size());
    CHECK(decoded.decodeBinary(r) == 1 && decoded.size() == 1 && r.remaining() == 0);
  }

  // ---- records written before the fits were stored are still read
  {
    Histogram plain("plain", "no fit", 5, 0.0, 1.0);
    plain.fill(0.5);
    Histogram read;                                                         // the fits section is 2 bytes when empty
    CHECK(readRecord(olderRecord(plain, 3, 2), read) && read.nFits() == 0 && read.binValueAtGlobal(2) == plain.binValueAtGlobal(2));
    DataPointsVector points;
    points.addPoints({0.0, 1.0}, {2.0, 3.0}, {0.1, 0.1});
    DataPointsVector readPoints;
    CHECK(readRecord(olderRecord(points, 2, 2), readPoints) && readPoints.size() == 2 && readPoints.nFits() == 0);
    Profile profile("profile", "no fit", 4, 0.0, 1.0);
    profile.fill(0.3, 2.0, 1.0);
    BinaryWriter w;                                                         // version 1: histogram part of version 3
    w.writeString(profile.binaryName());
    profile.encodeBinary(w);
    Histogram histogramPart(profile);
    histogramPart.clearFits();
    BinaryWriter hw;
    hw.writeString(profile.binaryName());
    histogramPart.Histogram::encodeBinary(hw);
    std::size_t histogramLength = hw.buffer().size();
    std::string payload = w.buffer().substr(0, histogramLength - 2) + w.buffer().substr(histogramLength);
    std::ostringstream out(std::ios::binary);
    writeBinaryRecord(out, Profile::binaryTag(), 1, payload, "profile");
    Profile readProfile;
    CHECK(w.buffer().compare(0, histogramLength, hw.buffer()) == 0);
    CHECK(readRecord(out.str(), readProfile) && readProfile.nBinsTotal() == 4 && readProfile.binValueAtGlobal(1) == profile.binValueAtGlobal(1));
  }

  // ---- invalid data
  {
    BinaryWriter w;
    w.writeUInt8(9);                                                        // a format of the fits not known yet
    w.writeVarUInt(0);
    FitList list;
    list.add(data.fitPointer(0));
    BinaryReader r(w.buffer().data(), w.buffer().size());
    bool thrown = false;
    try { list.decodeBinary(r); } catch (const Exception &) { thrown = true; }
    CHECK(thrown && list.size() == 1);                                      // unchanged
    BinaryWriter full;
    list.encodeBinary(full);
    std::string truncated = full.buffer().substr(0, full.buffer().size() - 5);
    BinaryReader rt(truncated.data(), truncated.size());
    thrown = false;
    try { FitList other; other.decodeBinary(rt); } catch (const Exception &) { thrown = true; }
    CHECK(thrown);
  }

  std::printf("TestFitStorage: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
