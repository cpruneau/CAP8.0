/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestFits2D.cpp
//! @brief Test program of the fits of functions of two variables (run by ctest).
//
// TestFits2D -- Normal2D, Polynomial2D and products f(x) g(y) (ParameterizedComposite), fits of histograms and data
// points of rank 2 (estimates, ranges over boxes, averages over the bins), fits of a function to another one on a
// grid, storage of the fits, maps of residuals and projections with their fits. Prints each failed check and exits
// with the number of failures (0: all passed).
//
#include <cmath>
#include <cstdio>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "BinaryIO.hpp"
#include "DataPoint3D.hpp"
#include "DataPointsVector.hpp"
#include "Exponential.hpp"
#include "FitViews.hpp"
#include "Fitter.hpp"
#include "FunctionRegistry.hpp"
#include "Histogram.hpp"
#include "Linear.hpp"
#include "MathExceptions.hpp"
#include "ModelValues.hpp"
#include "Normal.hpp"
#include "Normal2D.hpp"
#include "ParameterizedComposite.hpp"
#include "Polynomial2D.hpp"

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

//! Pseudo-random numbers with a fixed seed.
struct Random
{
  unsigned long long seed;
  explicit Random(unsigned long long s) : seed(s) { }
  double uniform() { seed = seed*6364136223846793005ULL + 1442695040888963407ULL; return ((seed >> 11) + 0.5)/9007199254740992.0; }
  double gaussian() { double u1 = uniform(), u2 = uniform(); return std::sqrt(-2*std::log(u1))*std::cos(2*M_PI*u2); }
};

//! A histogram of n events of a correlated Gaussian.
Histogram gaussian2D(int n, double mx, double my, double sx, double sy, double rho, int bins, double half, unsigned long long seed)
{
  Histogram h("h2", "correlated Gaussian", bins, -half, half, bins, -half, half);
  Random random(seed);
  for (int k=0; k<n; k++)
    {
    double z1 = random.gaussian(), z2 = random.gaussian();
    h.fill(mx + sx*z1, my + sy*(rho*z1 + std::sqrt(1 - rho*rho)*z2), 1.0);
    }
  return h;
}
} // namespace

//! @brief Count a check, and report it if the condition is false.
#define CHECK(condition) check((condition), #condition, __LINE__)

//! @brief Run the checks. @return number of failed checks (0: all passed)
int main()
{
  // ---- Normal2D
  {
    Normal2D g(10.0, 1.0, -1.0, 0.5, 2.0, 0.6);
    CHECK(g.rank() == 2 && g.nParameters() == 6 && g.parameter(5).name() == "correlation" && g.parameter(5).type() == Parameter::LowHigh);
    double u = (1.4 - 1.0)/0.5, v = (0.0 + 1.0)/2.0;
    CHECK(near(g.evaluateAt(1.0, -1.0), 10.0, 1E-14) && near(g.evaluateAt(1.4, 0.0), 10.0*std::exp(-(u*u - 1.2*u*v + v*v)/(2*0.64)), 1E-13));
    CHECK(near(g.volume(), 2*M_PI*10.0*0.5*2.0*0.8, 1E-13));
    CHECK(near(g.integralInRange(std::vector<double>{ -5.0, -17.0 }, std::vector<double>{ 7.0, 15.0 }), g.volume(), 1E-6));   // numerical, rho != 0
    Normal2D r(3.0, 0.5, 0.0, 1.0, 0.7, 0.0);
    double analytic = r.integralInRange(-0.5, 1.0, -1.0, 0.3);
    r.setParameterValue(5, 1E-12);                                                   // numerical
    CHECK(near(analytic, r.integralInRange(-0.5, 1.0, -1.0, 0.3), 1E-6));
    bool thrown = false;
    try { Normal2D bad(1.0, 0.0, 0.0, 1.0, 1.0, 1.0); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);
  }

  // ---- Polynomial2D
  {
    Polynomial2D p(2);
    CHECK(p.nParameters() == 10 && p.nCoefficients() == 6 && p.parameter(0).name() == "c00" && p.parameter(1).name() == "c10"
          && p.parameter(2).name() == "c01" && p.parameter(4).name() == "c11" && p.parameter(6).name() == "x0" && p.parameter(9).type() == Parameter::Fixed);
    p.setParameterValues({ 1.0, 2.0, -1.0, 0.5, 0.25, -0.75, 0.0, 1.0, 0.0, 1.0 });
    double x = 1.5, y = -0.5;
    CHECK(near(p.evaluateAt(x, y), 1 + 2*x - y + 0.5*x*x + 0.25*x*y - 0.75*y*y, 1E-14));
    p.setDegree(3);
    CHECK(p.nParameters() == 14 && p.parameter(Polynomial2D::index(1, 1)).value() == 0.25 && p.parameter(10).name() == "x0"
          && near(p.evaluateAt(x, y), 1 + 2*x - y + 0.5*x*x + 0.25*x*y - 0.75*y*y, 1E-14));
    p.setDegree(1);
    CHECK(p.nParameters() == 7 && near(p.evaluateAt(x, y), 1 + 2*x - y, 1E-14));
    p.adaptToRange(std::vector<double>{ 0.0, -2.0 }, std::vector<double>{ 10.0, 2.0 });
    CHECK(p.parameter(3).value() == 5.0 && p.parameter(4).value() == 5.0 && p.parameter(5).value() == 0.0 && p.parameter(6).value() == 2.0);
    DataPointsVector points;                                                        // exact quadratic surface
    for (int i=0; i<=10; i++)
      for (int j=0; j<=8; j++)
        {
        double a = i, b = -2.0 + 0.5*j;
        points.addPoint(DataPoint3D(a, b, 3.0 - 0.2*a + 0.5*b + 0.01*a*a - 0.1*a*b + 0.3*b*b, 0.1));
        }
    Polynomial2D q(2);
    CHECK(Fitter::estimate(q, points, std::vector<double>{ 0.0, -2.0 }, std::vector<double>{ 10.0, 2.0 }));
    CHECK(near(q.evaluateAt(4.0, 1.0), 3.0 - 0.8 + 0.5 + 0.16 - 0.4 + 0.3, 1E-5) && q.parameter(6).value() == 5.0);
    FitResult r = Fitter(q, points).fit();
    CHECK(r.converged && r.chiSquare < 1E-3 && r.nDegreesOfFreedom == 99 - 6);
    bool thrown = false;
    try { Polynomial2D big(10); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);
  }

  // ---- products f(x) g(y), sums of functions of two variables, registration
  {
    ParameterizedComposite product = ParameterizedComposite::product("x", Normal(2.0, 0.0, 1.0), "y", Exponential(3.0, -0.5));
    CHECK(product.rank() == 2 && product.operation() == ParameterizedComposite::kProduct && product.nParameters() == 5
          && product.parameter(3).name() == "y.amplitude");
    CHECK(near(product.evaluateAt(0.5, 2.0), 2.0*std::exp(-0.125)*3.0*std::exp(-1.0), 1E-13));
    CHECK(near(product.integralInRange(std::vector<double>{ -1.0, 0.0 }, std::vector<double>{ 1.0, 2.0 }),
               Normal(2.0, 0.0, 1.0).integralInRange(-1.0, 1.0)*Exponential(3.0, -0.5).integralInRange(0.0, 2.0), 1E-10));
    ParameterizedComposite sum;
    sum.add("peak", Normal2D(5.0, 0.0, 0.0, 1.0, 1.0, 0.0));
    sum.add("flat", Polynomial2D(0));
    CHECK(sum.rank() == 2 && near(sum.evaluateAt(0.0, 0.0), 6.0, 1E-14));
    bool thrown = false;
    try { sum.add("line", Linear()); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);                                                                  // a sum keeps one rank

    CHECK(FunctionRegistry::isRegistered("Normal2D") && FunctionRegistry::isRegistered("Polynomial2D"));
    std::unique_ptr<ParameterizedFunction> made(FunctionRegistry::create("Polynomial2D", 14));
    CHECK(made && dynamic_cast<Polynomial2D*>(made.get())->degree() == 3 && FunctionRegistry::create("Polynomial2D", 11) == nullptr);
    FunctionRegistry::registerComposite("GaussXExpY", product);
    const char * file = "TestFits2D.xml";
    CHECK(FunctionRegistry::writeComposites(file));
    FunctionRegistry::removeComposite("GaussXExpY");
    CHECK(FunctionRegistry::readComposites(file) >= 1);
    std::unique_ptr<ParameterizedFunction> back(FunctionRegistry::create("GaussXExpY"));
    const ParameterizedComposite * c = dynamic_cast<const ParameterizedComposite*>(back.get());
    CHECK(c && c->operation() == ParameterizedComposite::kProduct && back->rank() == 2 && near(back->evaluateAt(0.5, 2.0), product.evaluateAt(0.5, 2.0), 1E-13));
    std::remove(file);
  }

  // ---- a correlated Gaussian in a histogram of rank 2
  Histogram h = gaussian2D(40000, 0.3, -0.2, 0.8, 1.2, 0.5, 40, 5.0, 11);
  {
    Normal2D model;
    CHECK(Fitter::estimate(model, h, std::vector<double>{ -5.0, -5.0 }, std::vector<double>{ 5.0, 5.0 }));
    CHECK(near(model.parameter(1).value(), 0.3, 0.05) && near(model.parameter(4).value(), 1.2, 0.1) && near(model.parameter(5).value(), 0.5, 0.1));
    Fitter fitter(model, h, Fitter::kPoisson);
    FitResult r = fitter.fit();
    bool recovered = true;
    double truth[] = { 0.0, 0.3, -0.2, 0.8, 1.2, 0.5 };
    for (int k=1; k<6; k++) recovered = recovered && std::fabs(r.values[k] - truth[k]) < 4*r.errors[k];
    CHECK(r.converged && recovered && r.chiSquarePerDegreeOfFreedom < 1.5 && r.nPoints == 1600);
    h.addFit(fitter.record("gaussian"));
    CHECK(h.fit(0).low.size() == 2 && near(h.fit(0).low[0], h.binCenter(0, 0), 1E-9) && near(h.fit(0).high[1], h.binCenter(39, 1), 1E-9));

    Normal2D boxed(model);                                                          // a range over a box
    Fitter inBox(boxed, h, Fitter::kPoisson);
    inBox.setRange({ -2.0, -3.0 }, { 2.0, 3.0 });
    FitResult b = inBox.fit();
    CHECK(b.converged && b.nPoints == 16*24 && std::fabs(b.values[1] - 0.3) < 4*b.errors[1]);
    bool thrown = false;
    try { inBox.setRange({ -2.0 }, { 2.0 }); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);
  }

  // ---- averages over the bins: a narrow peak in wide bins
  {
    Histogram narrow = gaussian2D(60000, 0.1, 0.0, 0.3, 0.3, 0.0, 25, 5.0, 23);     // bins of 0.4 for widths of 0.3
    Normal2D centers, averages;
    Fitter::estimate(centers, narrow, std::vector<double>{ -5.0, -5.0 }, std::vector<double>{ 5.0, 5.0 });
    averages = centers;
    FitResult c = Fitter(centers, narrow, Fitter::kPoisson).fit();
    Fitter averaged(averages, narrow, Fitter::kPoisson);
    averaged.setBinAveraging(true);
    FitResult a = averaged.fit();
    CHECK(c.converged && a.converged && std::fabs(c.values[3] - 0.3) > 5*c.errors[3]);              // biased at the centers
    CHECK(std::fabs(a.values[3] - 0.3) < 4*a.errors[3] && std::fabs(a.values[4] - 0.3) < 4*a.errors[4]);
    CHECK(a.method.contains("averaged over the bins") && a.chiSquare < c.chiSquare);
    Histogram line("line", "line", 10, 0.0, 10.0);
    Linear l(2.0, 3.0);
    CHECK(near(binAverage(l, line, 4), l.evaluateAt(4.5), 1E-12));                  // exact for a straight line
    CHECK(near(modelValue(l, line, 4, false), l.evaluateAt(4.5), 1E-12));
  }

  // ---- fits of functions to functions, options of fitAndAttach
  {
    Normal2D target(4.0, 0.5, -0.5, 1.0, 0.6, -0.3);
    DataPointsVector grid = Fitter::samples(target, -3.0, 3.0, 21, -2.0, 2.0, 11);
    CHECK(grid.size() == 231 && grid.rank() == 2 && near(grid.valueAt(0), target.evaluateAt(-3.0, -2.0), 1E-6));
    Normal2D start(3.0, 0.2, -0.2, 1.3, 0.9, 0.0);
    Fitter::FitOptions options;
    options.samples = 25;
    options.label = "on a grid";
    FitRecord record = Fitter::fitAndAttach(start, target, { -3.0, -2.0 }, { 3.0, 2.0 }, options);
    CHECK(target.nFits() == 1 && record.result.nPoints == 625 && near(record.result.values[5], -0.3, 1E-4) && near(record.result.values[3], 1.0, 1E-4));
    bool thrown = false;
    try { Fitter::samples(Normal(), 0.0, 1.0, 5, 0.0, 1.0, 5); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);
    Normal2D model(1000.0, 0.0, 0.0, 1.0, 1.0, 0.0);
    Fitter::FitOptions box;
    box.statistic = Fitter::kPoisson;
    box.rangeLow = { -1.5, -2.0 };
    box.rangeHigh = { 1.5, 2.0 };
    box.binAveraging = true;
    Histogram copy(h);
    copy.clearFits();
    FitRecord inBox = Fitter::fitAndAttach(model, copy, box);
    CHECK(copy.nFits() == 1 && inBox.result.nPoints == 12*16 && inBox.result.method.contains("averaged") && near(inBox.low[0], -1.375, 1E-9));
  }

  // ---- fits of rank 2 stored with the data
  {
    ParameterizedComposite product = ParameterizedComposite::product("x", Normal(1.0, 0.0, 1.0), "y", Normal(1.0, 0.0, 1.0));
    FunctionRegistry::registerComposite("NormalXNormalY", product);
    std::unique_ptr<ParameterizedFunction> separable(FunctionRegistry::create("NormalXNormalY"));
    Histogram uncorrelated = gaussian2D(20000, 0.0, 0.0, 1.0, 1.0, 0.0, 20, 4.0, 5);
    Fitter::estimate(*separable, uncorrelated, std::vector<double>{ -4.0, -4.0 }, std::vector<double>{ 4.0, 4.0 });   // not known: false
    separable->setParameterValues({ 400.0, 0.1, 0.9, 1.0, -0.1, 1.1 });
    separable->fixParameter(3);                                                     // the two amplitudes multiply
    Fitter::fitAndAttach(*separable, uncorrelated);
    Polynomial2D flat(0);
    Fitter::fitAndAttach(flat, uncorrelated);
    std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
    writeBinaryObject(stream, uncorrelated);
    stream.seekg(0);
    Histogram read;
    readBinaryObject(stream, read);
    CHECK(read.nFits() == 2 && read.fit(0).function->rank() == 2 && FunctionRegistry::typeName(*read.fit(0).function) == "NormalXNormalY"
          && dynamic_cast<const Polynomial2D*>(read.fit(1).function.get()) && read.fit(0).low.size() == 2
          && near(read.fit(0).valueAt(std::vector<double>{ 0.3, -0.4 }), uncorrelated.fit(0).valueAt(std::vector<double>{ 0.3, -0.4 }), 1E-12));
    Histogram withGaussian = h;
    std::stringstream second(std::ios::in | std::ios::out | std::ios::binary);
    writeBinaryObject(second, withGaussian);
    second.seekg(0);
    Histogram back;
    readBinaryObject(second, back);
    CHECK(back.nFits() == 1 && dynamic_cast<const Normal2D*>(back.fit(0).function.get()) && back.fit(0).result.values == h.fit(0).result.values);
  }

  // ---- residual maps and projections
  {
    const FitRecord & fit = h.fit(0);
    Histogram pulls = residualMap(h, fit);
    double sum = 0, sum2 = 0;
    int n = 0;
    for (unsigned int k=0; k<pulls.nBinsTotal(); k++)
      if (h.statErrorAt(k) > 0) { sum += pulls.valueAt(k); sum2 += pulls.valueAt(k)*pulls.valueAt(k); n++; }
    CHECK(pulls.nFits() == 0 && pulls.rank() == 2 && n > 300 && std::fabs(sum/n) < 0.15 && std::fabs(std::sqrt(sum2/n) - 1.0) < 0.2);
    unsigned int peak = h.binIndex(20u, 18u);
    CHECK(near(pulls.valueAt(peak), (h.valueAt(peak) - fit.valueAt(h.coordinatesAt(peak)))/h.statErrorAt(peak), 1E-6) && pulls.statErrorAt(peak) == 1.0);
    Histogram differences = residualMap(h, fit, Residual::kDifference);
    CHECK(near(differences.valueAt(peak), h.valueAt(peak) - fit.valueAt(h.coordinatesAt(peak)), 1E-6) && near(differences.statErrorAt(peak), h.statErrorAt(peak), 1E-6));
    DataPointsVector points = residuals(h, fit);
    CHECK(int(points.size()) == n && points.rank() == 2);

    Histogram px = projection(h, 0), py = projection(h, 1, 10, 29);
    double total = 0, projected = 0;
    for (unsigned int k=0; k<h.nBinsTotal(); k++) total += h.valueAt(k);
    for (unsigned int k=0; k<px.nBinsTotal(); k++) projected += px.valueAt(k);
    CHECK(near(projected, total, 1E-9) && px.rank() == 1 && px.nFits() == 1 && py.nFits() == 1);
    const FitRecord & fx = px.fit(0);
    CHECK(fx.label == "gaussian (x projection)" && py.fit(0).label == "gaussian (y projection)" && fx.function->rank() == 1
          && fx.result.values == fit.result.values && fx.result.nDegreesOfFreedom == fx.result.nPoints);
    double expected = 0, x = px.binCenter(17);
    for (unsigned int j=0; j<40; j++)                                                 // the fit summed over the y bins
      expected += binAverage(Normal2D(fit.result.values[0], fit.result.values[1], fit.result.values[2], fit.result.values[3],
                                      fit.result.values[4], fit.result.values[5]), h, h.binIndex(17u, j));
    CHECK(near(fx.valueAt(x), expected, 2E-3) && fx.result.chiSquarePerDegreeOfFreedom < 2.0 && fx.result.pValue > 1E-4);
    std::unique_ptr<ParameterizedFunction> copy(fx.function->clone());
    CHECK(copy->evaluateAt(x) == fx.function->evaluateAt(x) && FunctionRegistry::typeName(*fx.function) == "Normal2D (projection)");
    std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
    writeBinaryObject(stream, px);
    stream.seekg(0);
    Histogram readBack;
    readBinaryObject(stream, readBack);
    CHECK(readBack.nBinsTotal() == px.nBinsTotal() && readBack.nFits() == 0);       // projected fits are views
    CHECK(h.fitColumnNames(false) == std::vector<String>({ "f(x,y) gaussian" }) && px.fitColumnNames(false)[0] == "f(x) gaussian (x projection)");
    bool thrown = false;
    try { projection(px, 0); } catch (const std::exception &) { thrown = true; }
    CHECK(thrown);
  }

  std::printf("TestFits2D: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
