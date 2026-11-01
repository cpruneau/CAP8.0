/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestFunctionStorage.cpp
//! @brief Test program of the binary records of parameterized functions ("CAPU") (run by ctest).
//
// TestFunctionStorage -- every registered class of ParameterizedFunction written to a CAP file in a Folder and read
// back (eagerly and lazily) as an object of its own class, with its parameters (values, types, bounds), name, title
// and fits; composites (sums and products, registered or not) rebuilt from their components; copies of folders
// holding functions read back; streams with writeBinaryObject() and ParameterizedFunction::readBinary(); classes of a
// user, registered or not; invalid records refused. Prints each failed check and exits with the number of failures.
//
#include <cmath>
#include <cstdio>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "BinaryIO.hpp"
#include "CapFile.hpp"
#include "DataPointsVector.hpp"
#include "Exceptions.hpp"
#include "Exponential.hpp"
#include "FitViews.hpp"
#include "Fitter.hpp"
#include "Folder.hpp"
#include "FunctionRegistry.hpp"
#include "Histogram.hpp"
#include "Linear.hpp"
#include "MathExceptions.hpp"
#include "Normal.hpp"
#include "Normal2D.hpp"
#include "ParameterizedComposite.hpp"
#include "Polynomial.hpp"

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

//! A function of a user (a line), registered in the tests below.
class UserLine : public ParameterizedFunction
{
public:
  UserLine() : ParameterizedFunction({ Parameter("a", 1.0), Parameter("b", 0.0) }, 1) { }
  double evaluateAt(const std::vector<double> & x) const override { checkRank(x.size(), __FUNCTION__); return parameterValue(0)*x[0] + parameterValue(1); }
  using ParameterizedFunction::evaluateAt;
  UserLine * clone() const override { return new UserLine(*this); }
};

//! A function of a user with state beyond its parameters: a power of x, the power being part of its state.
class UserPower : public ParameterizedFunction
{
public:
  explicit UserPower(int power=1) : ParameterizedFunction({ Parameter("a", 1.0) }, 1), _power(power) { }
  double evaluateAt(const std::vector<double> & x) const override { checkRank(x.size(), __FUNCTION__); return parameterValue(0)*std::pow(x[0], _power); }
  using ParameterizedFunction::evaluateAt;
  UserPower * clone() const override { return new UserPower(*this); }
  void encodeState(BinaryWriter & w) const override { w.writeVarInt(_power); }
  void decodeState(BinaryReader & r, unsigned int) override { if (r.remaining() > 0) _power = int(r.readVarInt()); }
  int _power;
};

//! A function of a user that is never registered.
class Unregistered : public UserLine
{
public:
  Unregistered * clone() const override { return new Unregistered(*this); }
};

//! Same class, name, title, parameters (names, values, types, bounds) and values at a few points.
bool same(const ParameterizedFunction & a, const ParameterizedFunction & b)
{
  if (typeid(a) != typeid(b) || a.rank() != b.rank() || a.nParameters() != b.nParameters()) return false;
  if (!(String(a.name()) == String(b.name())) || !(String(a.title()) == String(b.title()))) return false;
  for (unsigned int k=0; k<a.nParameters(); k++)
    {
    const Parameter & p = a.parameter(k), & q = b.parameter(k);
    if (!(p.name() == q.name()) || p.value() != q.value() || p.type() != q.type() || p.minimum() != q.minimum() || p.maximum() != q.maximum())
      return false;
    }
  for (double t : { 0.3, 1.1, 2.7 })
    {
    std::vector<double> x(a.rank(), t);
    if (a.rank() > 1) x[1] = 0.5*t;
    double u = a.evaluateAt(x), v = b.evaluateAt(x);
    if (!(u == v || (std::isnan(u) && std::isnan(v)))) return false;
    }
  return true;
}

//! A function written as one record to a stream and read back.
std::unique_ptr<ParameterizedFunction> roundTrip(const ParameterizedFunction & f)
{
  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  writeBinaryObject(stream, f);
  stream.seekg(0);
  return std::unique_ptr<ParameterizedFunction>(ParameterizedFunction::readBinary(stream));
}

} // namespace

#define CHECK(condition) check((condition), #condition, __LINE__)

//! @brief Run the checks. @return number of failed checks (0: all passed)
int main()
{
  const std::string path = "TestFunctionStorage.cap";

  // ---- every registered class: write to a file in a folder, read back eagerly and lazily
  {
    std::vector<String> names = FunctionRegistry::names();
    CHECK(names.size() >= 15);
    CapFile out(path, File::kWriteOnly);
    out.setFolderName("functions");
    Folder * all = out.addFolder("all");
    std::vector<std::unique_ptr<ParameterizedFunction>> originals;
    for (const String & name : names)
      {
      if (FunctionRegistry::isComposite(name)) continue;
      std::unique_ptr<ParameterizedFunction> f(FunctionRegistry::create(name));
      if (!f) { CHECK(f != nullptr); continue; }
      for (unsigned int k=0; k<f->nParameters(); k++)                   // values that differ from the defaults
        if (f->parameter(k).type() == Parameter::Free) f->setParameterValue(k, f->parameter(k).value() + 0.25*(k + 1));
      if (f->nParameters() > 1) f->fixParameter(1);
      if (f->nParameters() > 2 && f->parameter(2).type() == Parameter::Free) f->setParameterLimits(2, -50.0, 50.0);
      f->setName(name);
      f->setTitle(String("title of ") + name);
      originals.push_back(std::unique_ptr<ParameterizedFunction>(f->clone()));
      all->add(f.release(), name);
      }
    out.write();

    CapFile in(path, File::kReadOnly);
    in.read();
    int good = 0;
    for (const auto & f : originals)
      {
      ParameterizedFunction * g = in.get<ParameterizedFunction>(String("all/") + f->name());
      if (g && same(*f, *g)) good++;
      else std::printf("  not read back: %s\n", f->name().data());
      }
    CHECK(good == int(originals.size()));
    CHECK(in.get<Normal>("all/Normal") != nullptr && in.get<Function>("all/Normal") != nullptr);   // through its own class and its bases

    CapFile lazy(path, File::kReadOnly);
    lazy.load(File::Loading::kLazy);
    CHECK(!lazy.folder().isLoaded("all/Exponential"));
    Exponential * e = lazy.get<Exponential>("all/Exponential");
    CHECK(e && same(*e, *in.get<Exponential>("all/Exponential")));

    Folder copy(in.folder());                                           // copies of functions held by their base class
    CHECK(copy.get<Normal>("all/Normal") && copy.get<Normal>("all/Normal") != in.get<Normal>("all/Normal")
          && same(*copy.get<Normal>("all/Normal"), *in.get<Normal>("all/Normal")));
    std::remove(path.c_str());
  }

  // ---- composites: sums and products, not registered or registered, nested
  {
    ParameterizedComposite sum;
    sum.add("signal", Normal(100.0, 1.0, 0.5));
    sum.add("background", Linear(10.0, -1.0));
    sum.setName("gauss+line");
    sum.fixParameter(2);
    std::unique_ptr<ParameterizedFunction> r = roundTrip(sum);
    CHECK(r && same(sum, *r));
    const ParameterizedComposite * c = dynamic_cast<const ParameterizedComposite*>(r.get());
    CHECK(c && c->nComponents() == 2 && c->label(1) == "background" && c->parameter(1).name() == "signal.mean");

    ParameterizedComposite product(ParameterizedComposite::kProduct);   // f(x) g(y), rank 2
    product.add("x", Normal(2.0, 0.0, 1.0));
    product.add("y", Exponential(1.0, -0.5));
    r = roundTrip(product);
    c = dynamic_cast<const ParameterizedComposite*>(r.get());
    CHECK(c && c->operation() == ParameterizedComposite::kProduct && c->rank() == 2 && same(product, *r));

    ParameterizedComposite outer;                                       // a composite of a composite
    outer.add("peak", sum);
    outer.add("offset", Polynomial(std::vector<double>{ 3.0 }));
    r = roundTrip(outer);
    CHECK(r && same(outer, *r) && dynamic_cast<const ParameterizedComposite*>(r.get())->nComponents() == 2);

    FunctionRegistry::registerComposite("TestGaussLine", sum);          // a registered composite keeps its name
    std::unique_ptr<ParameterizedFunction> registered(FunctionRegistry::create("TestGaussLine"));
    r = roundTrip(*registered);
    CHECK(r && r->registryName() == "TestGaussLine" && same(*registered, *r));
    FunctionRegistry::removeComposite("TestGaussLine");
    r = roundTrip(*registered);                                         // and is still read once it is gone
    CHECK(r && same(*registered, *r));
  }

  // ---- fits attached to a function
  {
    Normal target(1.0, 0.0, 1.0);
    target.setName("target");
    DataPointsVector samples = Fitter::samples(target, -1.0, 1.0, 41);
    Linear approximation(1.0, 0.0);
    Fitter fitter(approximation, samples);
    fitter.exclude(0, -0.2, 0.2);
    fitter.fit();
    target.addFit(fitter.record("line"));
    std::unique_ptr<ParameterizedFunction> r = roundTrip(target);
    CHECK(r && r->nFits() == 1 && r->fit(0).label == "line" && r->fit(0).excluded.size() == 1
          && r->fit(0).valueAt(0.5) == target.fit(0).valueAt(0.5));
  }

  // ---- a fit made with a composite that was never registered: read back from the fit itself
  {
    Histogram h("h", "peak over a line", 40, 0.0, 10.0);
    for (unsigned int i=0; i<40; i++)
      {
      double x = h.binCenter(i, 0);
      h.setBinValueAt(i, 50.0*std::exp(-0.5*(x - 5.0)*(x - 5.0)/0.36) + 20.0 - 1.5*x);
      h.setBinUncertaintyAt(i, 1.0);
      }
    ParameterizedComposite model;                                       // used for this fit only: not registered
    model.add("signal", Normal(40.0, 4.8, 0.5));
    model.add("background", Linear(15.0, -1.0));
    CHECK(!FunctionRegistry::isComposite("model") && model.registryName().isNull());
    Fitter fitter(model, h);
    FitResult result = fitter.fit();
    CHECK(result.converged && std::fabs(result.values[1] - 5.0) < 1E-6);
    h.addFit(fitter.record("peak"));
    std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
    writeBinaryObject(stream, h);
    stream.seekg(0);
    Histogram read;
    CHECK(readBinaryObject(stream, read) && read.nFits() == 1);
    const ParameterizedComposite * c = read.nFits() ? dynamic_cast<const ParameterizedComposite*>(read.fit(0).function.get()) : nullptr;
    CHECK(c && c->nComponents() == 2 && c->label(0) == "signal" && read.fit(0).label == "peak"
          && read.fit(0).valueAt(4.2) == h.fit(0).valueAt(4.2) && read.fit(0).result.values == result.values);
  }

  // ---- classes of a user: registered (with state beyond the parameters) or not
  {
    FunctionRegistry::registerType<UserLine>("UserLine");
    FunctionRegistry::registerType<UserPower>("UserPower");
    UserLine line;
    line.setParameterValue(0, 3.0);
    std::unique_ptr<ParameterizedFunction> r = roundTrip(line);
    CHECK(r && dynamic_cast<UserLine*>(r.get()) && same(line, *r));
    UserPower cube(3);
    cube.setParameterValue(0, 2.0);
    r = roundTrip(cube);
    CHECK(r && dynamic_cast<UserPower*>(r.get()) && dynamic_cast<UserPower*>(r.get())->_power == 3 && r->evaluateAt(2.0) == 16.0);

    Unregistered unregistered;
    bool thrown = false;
    try { roundTrip(unregistered); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);
    Folder folder("f");
    folder.add(new Unregistered(), "u");
    thrown = false;
    try { folder.saveBinary("TestFunctionStorage2.cap"); } catch (const std::exception &) { thrown = true; }
    CHECK(thrown);
    std::remove("TestFunctionStorage2.cap");
  }

  // ---- invalid records
  {
    BinaryWriter w;                                                     // a class that does not exist
    w.writeString(String("f"));
    w.writeString(String("NoSuchFunction"));
    w.writeVarUInt(2);
    w.writeVarUInt(1);
    std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
    writeBinaryRecord(stream, "CAPU", 1, w.buffer(), "f");
    stream.seekg(0);
    bool thrown = false;
    try { std::unique_ptr<ParameterizedFunction> f(ParameterizedFunction::readBinary(stream)); } catch (const std::exception &) { thrown = true; }
    CHECK(thrown);

    BinaryWriter v;                                                     // a known class with the wrong number of parameters
    v.writeString(String("n"));
    v.writeString(String("Normal"));
    v.writeVarUInt(5);
    v.writeVarUInt(1);
    std::stringstream second(std::ios::in | std::ios::out | std::ios::binary);
    writeBinaryRecord(second, "CAPU", 1, v.buffer(), "n");
    second.seekg(0);
    thrown = false;
    try { std::unique_ptr<ParameterizedFunction> f(ParameterizedFunction::readBinary(second)); } catch (const std::exception &) { thrown = true; }
    CHECK(thrown);

    std::stringstream empty(std::ios::in | std::ios::out | std::ios::binary);
    CHECK(ParameterizedFunction::readBinary(empty) == nullptr);
  }

  std::printf("TestFunctionStorage: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
