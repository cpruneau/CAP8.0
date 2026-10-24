/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestFitFunctions.cpp
//! @brief Test program of the functions for fits: orthogonal series, Bessel functions, composites (run by ctest).
//
// TestFitFunctions -- tests of CAP::MATH::OrthogonalSeries (Legendre, Chebyshev, Hermite, Laguerre), BesselFunction
// (J, Y, I, K of any order), ParameterizedComposite, and their registration in FunctionRegistry (composites written
// to and read from files, fits of composites stored in records). Prints each failed check and exits with the number
// of failures (0: all passed).
//
#include <cmath>
#include <cstdio>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "BesselFunction.hpp"
#include "BesselFunctions.hpp"
#include "BinaryIO.hpp"
#include "CompositeFunction.hpp"
#include "Constant.hpp"
#include "Exponential.hpp"
#include "Normal2D.hpp"
#include "DataPointsVector.hpp"
#include "Fitter.hpp"
#include "FunctionRegistry.hpp"
#include "Laguerre.hpp"
#include "Legendre.hpp"
#include "Linear.hpp"
#include "MathExceptions.hpp"
#include "Normal.hpp"
#include "OrthogonalSeries.hpp"
#include "ParameterizedComposite.hpp"

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

bool near(double a, double b, double tolerance) { return std::fabs(a - b) <= tolerance*std::max(1.0, std::fabs(b)); }

//! Points y = f(x) at n points of [low, high], each with an uncertainty.
DataPointsVector pointsOf(const ParameterizedFunction & f, double low, double high, int n, double error)
{
  DataPointsVector points;
  for (int k=0; k<n; k++)
    {
    double x = low + (high - low)*k/(n - 1);
    points.addPoint(DataPoint2D(float(x), float(f.evaluateAt(x)), float(error)));
    }
  return points;
}
} // namespace

//! @brief Count a check, and report it if the condition is false.
#define CHECK(condition) check((condition), #condition, __LINE__)

//! @brief Run the checks. @return number of failed checks (0: all passed)
int main()
{
  // ---- the polynomials of the series
  {
    std::vector<double> p;
    OrthogonalSeries::polynomials(OrthogonalSeries::kLegendre, 5, 0.3, 0.0, p);
    bool same = true;
    for (unsigned int n=0; n<=5; n++) same = same && near(p[n], Legendre::value(n, 0.3), 1E-12);
    CHECK(same);
    OrthogonalSeries::polynomials(OrthogonalSeries::kLaguerre, 4, 1.7, 0.5, p);
    same = true;
    for (unsigned int n=0; n<=4; n++) same = same && near(p[n], Laguerre::value(n, 0.5, 1.7), 1E-12);
    CHECK(same);
    OrthogonalSeries::polynomials(OrthogonalSeries::kChebyshev, 3, 0.5, 0.0, p);
    CHECK(near(p[2], -0.5, 1E-12) && near(p[3], -1.0, 1E-12));                 // T2 = 2t^2 - 1, T3 = 4t^3 - 3t
    OrthogonalSeries::polynomials(OrthogonalSeries::kHermite, 3, 1.0, 0.0, p);
    CHECK(near(p[1], 2.0, 1E-12) && near(p[2], 2.0, 1E-12) && near(p[3], -4.0, 1E-12));   // H2 = 4t^2 - 2, H3 = 8t^3 - 12t
  }

  // ---- series: mapping, degree, estimate and fit
  {
    LegendreSeries series(2);
    CHECK(series.nParameters() == 5 && series.parameter(3).name() == "low" && series.parameter(3).type() == Parameter::Fixed);
    series.adaptToRange(0.0, 10.0);
    CHECK(series.parameter(3).value() == 0.0 && series.parameter(4).value() == 10.0 && near(series.variable(10.0), 1.0, 1E-12));
    Normal unused;
    DataPointsVector parabola;
    for (int k=0; k<=20; k++) { double x = 0.5*k; parabola.addPoint(DataPoint2D(float(x), float(3.0 + 2.0*x - 0.5*x*x), 0.1f)); }
    LegendreSeries fitted(2);
    CHECK(Fitter::estimate(fitted, parabola, 0.0, 10.0));
    bool exact = true;
    for (double x : { 0.0, 2.5, 7.0, 10.0 }) exact = exact && near(fitted.evaluateAt(x), 3.0 + 2.0*x - 0.5*x*x, 1E-4);
    CHECK(exact && fitted.parameter(3).value() == 0.0 && fitted.parameter(4).value() == 10.0);
    FitResult r = Fitter(fitted, parabola).fit();
    CHECK(r.converged && r.chiSquare < 1E-4 && r.nDegreesOfFreedom == 18);
    fitted.setDegree(4);
    CHECK(fitted.nParameters() == 7 && fitted.degree() == 4 && fitted.parameter(5).name() == "low" && fitted.parameter(4).value() == 0.0
          && near(fitted.evaluateAt(5.0), 3.0 + 10.0 - 12.5, 1E-4));
    fitted.setDegree(1);
    CHECK(fitted.nParameters() == 4 && fitted.parameter(2).name() == "low");
    ChebyshevSeries chebyshev(3);
    chebyshev.fixParameter(4, false);                                       // a free mapping parameter is not changed
    chebyshev.adaptToRange(2.0, 4.0);
    CHECK(chebyshev.parameter(4).value() == -1.0 && chebyshev.parameter(5).value() == 4.0);   // only the fixed one follows
    HermiteSeries hermite(3);
    CHECK(Fitter::estimate(hermite, parabola, 0.0, 10.0) && near(hermite.evaluateAt(4.0), 3.0 + 8.0 - 8.0, 1E-4)
          && hermite.parameter(4).value() == 5.0 && hermite.parameter(5).value() == 2.5);
    LaguerreSeries laguerre(2, 0.5);
    CHECK(Fitter::estimate(laguerre, parabola, 0.0, 10.0) && near(laguerre.evaluateAt(6.0), 3.0 + 12.0 - 18.0, 1E-4)
          && laguerre.parameter(5).value() == 0.5);
    bool thrown = false;
    try { LaguerreSeries bad(2, -1.0); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);
    std::unique_ptr<ParameterizedFunction> copy(laguerre.clone());
    CHECK(dynamic_cast<LaguerreSeries*>(copy.get()) && copy->evaluateAt(6.0) == laguerre.evaluateAt(6.0));
  }

  // ---- Bessel functions of any order
  {
    CHECK(near(BesselFunction::value(Bessel::BesselJ, 0, 2.0), besselJ0(2.0), 1E-12) && near(BesselFunction::value(Bessel::BesselJ, 1, 2.0), besselJ1(2.0), 1E-12));
    CHECK(near(BesselFunction::value(Bessel::BesselJ, 2, 1.0), 0.1149034849, 1E-7));    // Miller's recurrence
    CHECK(near(BesselFunction::value(Bessel::BesselJ, 5, 1.0), 2.497577302E-4, 1E-6));
    CHECK(near(BesselFunction::value(Bessel::BesselJ, 3, 10.0), 0.05837937931, 1E-6));   // upward recurrence
    CHECK(near(BesselFunction::value(Bessel::BesselJ, 3, -10.0), -0.05837937931, 1E-6));
    CHECK(near(BesselFunction::value(Bessel::BesselY, 2, 1.0), -1.650682607, 1E-6));
    CHECK(near(BesselFunction::value(Bessel::BesselI, 2, 1.0), 0.1357476698, 1E-6));
    CHECK(near(BesselFunction::value(Bessel::BesselK, 2, 1.0), 1.624838899, 1E-6));
    CHECK(std::isnan(BesselFunction::value(Bessel::BesselK, 0, -1.0)) && std::isnan(BesselFunction::value(Bessel::BesselY, 1, 0.0)));
    BesselJFunction j(1, 3.0, 1.5, 0.0);
    CHECK(j.nParameters() == 4 && j.parameter(3).type() == Parameter::Fixed && j.order() == 1
          && near(j.evaluateAt(2.0), 3.0*besselJ1(3.0), 1E-12));
    DataPointsVector points = pointsOf(j, 0.0, 6.0, 40, 0.01);
    BesselJFunction model(1, 2.5, 1.4, 0.0);
    model.fixParameter(2);
    FitResult r = Fitter(model, points).fit();
    CHECK(r.converged && near(r.values[0], 3.0, 1E-3) && near(r.values[1], 1.5, 1E-3));
    bool thrown = false;
    try { BesselFunction struve(Bessel::StruveH); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);
  }

  // ---- composites
  ParameterizedComposite model;
  model.add("signal", Normal(100.0, 2.0, 0.5));
  model.add("background", Linear(10.0, 1.0));
  {
    CHECK(model.nComponents() == 2 && model.nParameters() == 5 && model.parameter(1).name() == "signal.mean"
          && model.parameter(4).name() == "background.c1" && model.firstParameter(1) == 3 && model.label(1) == "background");
    CHECK(near(model.evaluateAt(2.0), 100.0 + 12.0, 1E-12));
    model.setParameterValue(4, 2.0);                                        // the parameters of the composite count
    CHECK(near(model.evaluateAt(2.0), 100.0 + 14.0, 1E-12) && near(model.component(1).parameter(1).value(), 2.0, 1E-15));
    model.setParameterValue(4, 1.0);
    CHECK(near(model.integralInRange(0.0, 4.0), Normal(100.0, 2.0, 0.5).integralInRange(0.0, 4.0) + 48.0, 1E-9)
          && near(model.derivativeAt(2.0), 1.0, 1E-9));
    ParameterizedComposite copy(model);
    copy.setParameterValue(0, 50.0);
    CHECK(near(model.evaluateAt(2.0), 112.0, 1E-12) && near(copy.evaluateAt(2.0), 62.0, 1E-12));   // components copied
    bool thrown = false;
    try { copy.add("signal", Linear()); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);

    DataPointsVector points = pointsOf(model, 0.0, 4.0, 41, 0.5);
    ParameterizedComposite start(model);
    start.setParameterValues({ 80.0, 1.8, 0.6, 8.0, 1.2 });
    FitResult r = Fitter(start, points).fit();
    CHECK(r.converged && near(r.values[0], 100.0, 1E-3) && near(r.values[1], 2.0, 1E-4) && near(r.values[2], 0.5, 1E-4)
          && near(r.values[3], 10.0, 1E-3));

    LegendreSeries background(1);
    ParameterizedComposite mapped;
    mapped.add("peak", Normal(1.0, 0.0, 1.0));
    mapped.add("background", background);
    mapped.adaptToRange(-3.0, 5.0);                                         // passed on to the components
    CHECK(mapped.parameter(5).value() == -3.0 && mapped.parameter(6).value() == 5.0 && near(mapped.component(1).parameter(3).value(), 5.0, 0));
  }

  // ---- registry: the new classes, composites, files
  {
    for (const char * name : { "LegendreSeries", "ChebyshevSeries", "HermiteSeries", "LaguerreSeries", "BesselJ", "BesselY", "BesselI", "BesselK" })
      CHECK(FunctionRegistry::isRegistered(String(name)));
    std::unique_ptr<ParameterizedFunction> s(FunctionRegistry::create("LegendreSeries", 6));
    CHECK(s && dynamic_cast<LegendreSeries*>(s.get())->degree() == 3);
    CHECK(FunctionRegistry::create("LaguerreSeries", 3) == nullptr);
    std::unique_ptr<ParameterizedFunction> d(FunctionRegistry::create("ChebyshevSeries"));
    std::unique_ptr<ParameterizedFunction> k(FunctionRegistry::create("BesselK"));
    CHECK(d && d->nParameters() == 6 && k && k->nParameters() == 4 && FunctionRegistry::typeName(*k) == "BesselK");

    FunctionRegistry::registerComposite("Gauss+Line", model);
    CHECK(FunctionRegistry::isComposite("Gauss+Line") && FunctionRegistry::isRegistered("Gauss+Line"));
    std::unique_ptr<ParameterizedFunction> made(FunctionRegistry::create("Gauss+Line"));
    CHECK(made && made->nParameters() == 5 && made->registryName() == "Gauss+Line" && FunctionRegistry::typeName(*made) == "Gauss+Line"
          && near(made->evaluateAt(2.0), 112.0, 1E-12));
    std::unique_ptr<ParameterizedFunction> clone(made->clone());
    CHECK(FunctionRegistry::typeName(*clone) == "Gauss+Line");
    bool thrown = false;
    try { FunctionRegistry::registerComposite("Normal", model); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);
    thrown = false;
    try { FunctionRegistry::registerComposite("empty", ParameterizedComposite()); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);

    ParameterizedComposite nested;                                          // a composite of a composite
    nested.add("model", *made);
    nested.add("offset", Linear(1.0, 0.0));
    FunctionRegistry::registerComposite("Nested", nested);
    ParameterizedComposite bounded(model);
    bounded.setParameterLimits(1, 1.0, 3.0);
    bounded.fixParameter(2);
    FunctionRegistry::registerComposite("Bounded", bounded);
    std::vector<String> names = FunctionRegistry::compositeNames();
    CHECK(names.size() >= 3 && names[names.size()-3] == "Gauss+Line" && names.back() == "Bounded");

    const char * file = "TestFitFunctions.xml";
    CHECK(FunctionRegistry::writeComposites(file));
    for (const char * name : { "Gauss+Line", "Nested", "Bounded" }) FunctionRegistry::removeComposite(name);
    CHECK(!FunctionRegistry::isComposite("Bounded") && !FunctionRegistry::isRegistered("Bounded"));
    String problems;
    CHECK(FunctionRegistry::readComposites(file, &problems) == 3 && problems.isNull());
    std::unique_ptr<ParameterizedFunction> back(FunctionRegistry::create("Bounded"));
    CHECK(back && back->parameter(1).type() == Parameter::LowHigh && back->parameter(1).minimum() == 1.0 && back->parameter(1).maximum() == 3.0
          && back->parameter(2).type() == Parameter::Fixed && back->parameter(1).name() == "signal.mean" && near(back->evaluateAt(2.0), 112.0, 1E-12));
    std::unique_ptr<ParameterizedFunction> backNested(FunctionRegistry::create("Nested"));
    CHECK(backNested && backNested->nParameters() == 7 && near(backNested->evaluateAt(2.0), 113.0, 1E-12));
    std::remove(file);
    {
      std::ofstream out(file);
      out << "<Functions version=\"1\"><Composite name=\"Unknown\"><Component label=\"a\" type=\"NotRegistered\" parameters=\"2\"/>"
             "</Composite><Composite name=\"Known\"><Component label=\"a\" type=\"Linear\" parameters=\"2\"/></Composite></Functions>\n";
    }
    problems = "";
    CHECK(FunctionRegistry::readComposites(file, &problems) == 1 && problems.contains("Unknown") && !FunctionRegistry::isComposite("Unknown")
          && FunctionRegistry::isComposite("Known"));
    std::remove(file);
    CHECK(FunctionRegistry::readComposites("no-such-file.xml") == -1);

    // a fit of a registered composite, stored with the data and read back
    DataPointsVector points = pointsOf(model, 0.0, 4.0, 21, 0.5);
    std::unique_ptr<ParameterizedFunction> fitModel(FunctionRegistry::create("Gauss+Line"));
    Fitter::fitAndAttach(*fitModel, points);
    std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
    writeBinaryObject(stream, points);
    stream.seekg(0);
    DataPointsVector read;
    readBinaryObject(stream, read);
    CHECK(read.nFits() == 1 && dynamic_cast<const ParameterizedComposite*>(read.fit(0).function.get())
          && FunctionRegistry::typeName(*read.fit(0).function) == "Gauss+Line"
          && near(read.fit(0).valueAt(2.0), points.fit(0).valueAt(2.0), 1E-12));
  }

  // ---- CompositeFunction: sums, products f(x) g(y), pointwise products
  {
    Linear line(1.0, 2.0);
    Constant offset(3.0);
    CompositeFunction sum({ &line, &offset });
    CHECK(sum.operation() == CompositeFunction::kSum && near(sum.evaluateAt(2.0), 8.0, 1E-14) && near(sum.derivativeAt(2.0), 2.0, 1E-8)
          && near(sum.integralInRange(0.0, 1.0), 5.0, 1E-10));

    Normal gx(1.0, 0.0, 1.0);
    Exponential ey(2.0, -0.5);
    CompositeFunction product = CompositeFunction::product(&gx, &ey);
    CHECK(product.operation() == CompositeFunction::kProduct && product.rank() == 2 && product.nFunctions() == 2);
    CHECK(near(product.evaluateAt(1.0, 2.0), std::exp(-0.5)*2.0*std::exp(-1.0), 1E-14)
          && product.evaluateAt(std::vector<double>{ 1.0, 2.0 }) == product.evaluateAt(1.0, 2.0));
    CHECK(near(product.integralInRange(-1.0, 1.0, 0.0, 4.0), gx.integralInRange(-1.0, 1.0)*ey.integralInRange(0.0, 4.0), 1E-12));
    bool thrown = false;
    try { product.evaluateAt(1.0); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);
    Normal2D peak(1.0, 0.0, 0.0, 1.0, 2.0, 0.0);
    CompositeFunction three(CompositeFunction::kProduct);                   // a function of x and y times one of z
    three.addFunction(&peak);
    three.addFunction(&ey);
    CHECK(three.rank() == 3 && near(three.evaluateAt(0.5, 1.0, 2.0), peak.evaluateAt(0.5, 1.0)*ey.evaluateAt(2.0), 1E-14));
    Legendre p2(2), p1(1);                                                   // components without parameters
    CompositeFunction legendre = CompositeFunction::product(&p2, &p1);
    CHECK(near(legendre.evaluateAt(0.5, 0.3), Legendre::value(2, 0.5)*0.3, 1E-14)
          && std::fabs(legendre.integralInRange(-1.0, 1.0, 0.0, 1.0)) < 1E-8);  // the integral of P2 over [-1, 1] is 0
    thrown = false;
    try { product.setFunction(1, &peak); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);                                                           // a function of as many variables
    product.setFunction(1, &offset);
    CHECK(near(product.evaluateAt(0.0, 9.0), 3.0, 1E-14));
    product.setFunction(1, &ey);
    CompositeFunction copy(product);
    CHECK(copy.operation() == CompositeFunction::kProduct && copy.rank() == 2);

    Linear rising(1.0, 1.0);
    Exponential decay(1.0, -1.0);
    CompositeFunction pointwise({ &rising, &decay }, CompositeFunction::kPointwiseProduct);
    double x = 0.7;
    CHECK(pointwise.rank() == 1 && near(pointwise.evaluateAt(x), (1 + x)*std::exp(-x), 1E-14)
          && near(pointwise.derivativeAt(x), -x*std::exp(-x), 1E-8)                       // product rule
          && near(pointwise.integralInRange(0.0, 2.0), 2.0 - 4.0*std::exp(-2.0), 1E-8));   // numerical
    thrown = false;
    try { pointwise.addFunction(&peak); } catch (const MathException &) { thrown = true; }
    CHECK(thrown && pointwise.nFunctions() == 2);

    CompositeFunction changed({ &rising, &decay });                         // a sum becomes a product of x and y
    changed.setOperation(CompositeFunction::kProduct);
    CHECK(changed.rank() == 2 && near(changed.evaluateAt(1.0, 2.0), 2.0*std::exp(-2.0), 1E-14));
    changed.setOperation(CompositeFunction::kSum);
    CHECK(changed.rank() == 1 && near(changed.evaluateAt(1.0), 2.0 + std::exp(-1.0), 1E-14));
    thrown = false;
    try { three.setOperation(CompositeFunction::kSum); } catch (const MathException &) { thrown = true; }
    CHECK(thrown && three.operation() == CompositeFunction::kProduct && three.rank() == 3);   // unchanged
    CompositeFunction single = CompositeFunction(std::vector<Function*>{ &decay }, CompositeFunction::kProduct);
    CHECK(near(single.derivativeAt(x), -std::exp(-x), 1E-8));
    three.clearFunctions();
    CHECK(three.rank() == 1 && three.evaluateAt(4.0) == 1.0);               // an empty product
  }

  std::printf("TestFitFunctions: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
