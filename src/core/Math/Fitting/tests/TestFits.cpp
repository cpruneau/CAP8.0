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
//! @file TestFits.cpp
//! @brief Test program of the fits attached to data points and functions (FitRecord, clone(), Fitter::record()) (run by ctest).
//
// TestFits -- tests of CAP::MATH::FitRecord and FitList, the fits attached to DataPoints (and histograms) and to
// Functions, ParameterizedFunction::clone() for every function of CAP, Fitter::record() and Fitter::samples(). Prints
// each failed check and exits with the number of failures (0: all passed).
//
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include "Constant.hpp"
#include "Cubic.hpp"
#include "DataPoint2D.hpp"
#include "DataPointsVector.hpp"
#include "Exponential.hpp"
#include "Fitter.hpp"
#include "Histogram.hpp"
#include "Linear.hpp"
#include "LogNormal.hpp"
#include "MathExceptions.hpp"
#include "Normal.hpp"
#include "Parabolic.hpp"
#include "Polynomial.hpp"
#include "Quartic.hpp"

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

//! A function of a user, without clone().
class Unclonable : public ParameterizedFunction
{
public:
  Unclonable() : ParameterizedFunction({ Parameter("a", 1.0) }, 1) { }
  double evaluateAt(const std::vector<double> & x) const override { checkRank(x.size(), __FUNCTION__); return parameterValue(0)*x[0]; }
  using ParameterizedFunction::evaluateAt;
};

//! The clone has the same class and values, and its own parameters.
template <typename F>
void checkClone(F & f, const char * name)
{
  std::unique_ptr<ParameterizedFunction> copy(f.clone());
  bool sameClass = dynamic_cast<F*>(copy.get()) != nullptr;
  bool sameValues = copy->parameterValues() == f.parameterValues() && copy->evaluateAt(0.7) == f.evaluateAt(0.7);
  std::vector<double> before = copy->parameterValues();
  f.setParameterValue(0, f.parameterValues()[0] + 1.0);
  bool independent = copy->parameterValues() == before;
  check(sameClass && sameValues && independent, std::string("clone of ") + name, __LINE__);
}
} // namespace

//! @brief Count a check, and report it if the condition is false.
#define CHECK(condition) check((condition), #condition, __LINE__)

//! @brief Run the checks. @return number of failed checks (0: all passed)
int main()
{
  // ---- clone() of every function of CAP
  {
    Constant constant(2.0);          checkClone(constant, "Constant");
    Linear linear(1.0, 2.0);         checkClone(linear, "Linear");
    Parabolic parabolic(1, 2, 3);    checkClone(parabolic, "Parabolic");
    Cubic cubic(1, 2, 3, 4);         checkClone(cubic, "Cubic");
    Quartic quartic(1, 2, 3, 4, 5);  checkClone(quartic, "Quartic");
    Polynomial polynomial(5);        polynomial.setParameterValues({1, 2, 3, 4, 5, 6}); checkClone(polynomial, "Polynomial");
    Normal normal(3.0, 0.5, 1.2);    checkClone(normal, "Normal");
    LogNormal logNormal(2.0, 0.1, 0.5); checkClone(logNormal, "LogNormal");
    Exponential exponential(2.0, -0.3); checkClone(exponential, "Exponential");
    Unclonable user;
    bool thrown = false;
    try { std::unique_ptr<ParameterizedFunction> c(user.clone()); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);                                                          // a class that does not implement it
  }

  // ---- a record of a fit, attached to the data
  DataPointsVector data;
  data.addPoints({0.0, 1.0, 2.0, 3.0, 4.0, 5.0}, {1.1, 2.9, 5.2, 6.8, 9.1, 11.3}, {0.1, 0.2, 0.1, 0.3, 0.2, 0.4});
  {
    Linear line(0.0, 1.0);
    Fitter fitter(line, data);
    bool thrown = false;
    try { fitter.record(); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);                                                          // no fit yet
    FitResult result = fitter.fit();
    FitRecord record = fitter.record();
    CHECK(record.label == "Linear fit" && record.function && record.function->parameterValues() == result.values);
    CHECK(record.result.chiSquare == result.chiSquare && record.result.nDegreesOfFreedom == 4 && record.result.errors == result.errors);
    CHECK(record.low.size() == 1 && record.low[0] == 0.0 && record.high[0] == 5.0 && record.inRange(2.5) && !record.inRange(6.0));
    CHECK(std::fabs(record.valueAt(2.0) - line.evaluateAt(2.0)) < 1E-12 && std::fabs(record.valueAt(std::vector<double>{3.0}) - line.evaluateAt(3.0)) < 1E-12);
    line.setParameterValues({100.0, 100.0});                                // the model changes: the record does not
    CHECK(std::fabs(record.valueAt(2.0) - (result.values[0] + 2*result.values[1])) < 1E-9);
    CHECK(data.nFits() == 0);
    data.addFit(record);
    Parabolic parabola(0.0, 1.0, 0.0);
    Fitter second(parabola, data);
    second.fit();
    data.addFit(second.record("quadratic"));
    CHECK(data.nFits() == 2 && data.fit(0).label == "Linear fit" && data.fit(1).label == "quadratic");
    CHECK(data.fit(1).result.nDegreesOfFreedom == 3 && data.fit(1).result.chiSquare <= data.fit(0).result.chiSquare + 1E-9);
    DataPointsVector copy(data);                                            // copies share the fits
    CHECK(copy.nFits() == 2 && copy.fitPointer(0) == data.fitPointer(0));
    copy.removeFit(0);
    CHECK(copy.nFits() == 1 && copy.fit(0).label == "quadratic" && data.nFits() == 2);
    copy.clearFits();
    CHECK(copy.nFits() == 0 && data.nFits() == 2);
    thrown = false;
    try { data.fit(5); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);
    std::shared_ptr<const FitRecord> shared = data.fitPointer(1);
    data.clearFits();
    CHECK(shared->label == "quadratic" && shared->function);               // a record outlives its detachment
  }

  // ---- a histogram, fitted with the Poisson likelihood
  {
    Histogram h("h", "h", 20, -4.0, 4.0);
    unsigned long long seed = 7;
    for (int k=0; k<500; k++)
      {
      seed = seed*6364136223846793005ULL + 1442695040888963407ULL;
      double u1 = ((seed >> 11) + 0.5)/9007199254740992.0;
      seed = seed*6364136223846793005ULL + 1442695040888963407ULL;
      double u2 = ((seed >> 11) + 0.5)/9007199254740992.0;
      h.fill(0.3 + std::sqrt(-2*std::log(u1))*std::cos(2*M_PI*u2));
      }
    Normal model(30.0, 0.0, 1.0);
    Fitter fitter(model, h, Fitter::kPoisson);
    fitter.fit();
    h.addFit(fitter.record("gaussian"));
    CHECK(h.nFits() == 1 && h.fit(0).result.statistic == "Poisson likelihood" && h.fit(0).result.nPoints == 20);
    CHECK(std::fabs(h.fit(0).low[0] - h.binCenter(0, 0)) < 1E-6 && std::fabs(h.fit(0).high[0] - h.binCenter(19, 0)) < 1E-6);
    Histogram copy(h);
    CHECK(copy.nFits() == 1 && copy.fit(0).label == "gaussian");
    Histogram assigned;
    assigned = h;                                                           // an assignment takes the fits too
    CHECK(assigned.nFits() == 1 && assigned.fitPointer(0) == h.fitPointer(0));
    DataPointsVector points;
    points = data;
    CHECK(points.nFits() == 0 && data.nFits() == 0);                        // (cleared above)
  }

  // ---- a function fitted by another one, through samples of it
  {
    Normal target(1.0, 0.0, 1.0);
    DataPointsVector samples = Fitter::samples(target, -1.0, 1.0, 41);
    CHECK(samples.size() == 41 && samples.point(0).x() == -1.0f && samples.point(40).x() == 1.0f && samples.point(20).yStatError() == 1.0f);
    Parabolic approximation(1.0, 0.0, 0.0);
    Fitter fitter(approximation, samples);
    FitResult r = fitter.fit();
    target.addFit(fitter.record("parabola"));
    CHECK(target.nFits() == 1 && target.fit(0).label == "parabola" && r.converged);
    CHECK(r.values[2] < -0.3 && r.values[2] > -0.6 && std::fabs(r.values[1]) < 1E-9);   // exp(-x^2/2) ~ 1 - x^2/2: curvature near -0.5, no slope
    Normal copy(target);                                                    // a copy of the function shares its fits
    CHECK(copy.nFits() == 1);
    copy = Normal(2.0, 0.0, 1.0);
    CHECK(copy.nFits() == 0);                                               // an assignment takes the fits of the other
    bool thrown = false;
    try { Fitter::samples(target, 0.0, 1.0, 1); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);
  }

  // ---- an unbinned fit: the range of the events
  {
    std::vector<double> events = {0.1, 0.5, 0.7, 1.1, 1.6, 2.2, 0.3, 0.9};
    Exponential model(1.0, -1.0);
    model.fixParameter(0);
    UnbinnedLikelihood likelihood(model, events, 0.0, 4.0);
    Fitter fitter(likelihood);
    fitter.fit();
    FitRecord record = fitter.record("decay");
    CHECK(record.low[0] == 0.0 && record.high[0] == 4.0 && record.result.statistic == "unbinned likelihood");
  }

  // ---- a range of the fit: the points outside are left out
  {
    DataPointsVector points;
    for (int k=0; k<10; k++) points.addPoint(DataPoint2D(k, k < 9 ? 2.0*k + 1.0 : 100.0, 0.1));
    Linear line(0.0, 1.0);
    Fitter fitter(line, points);
    fitter.setRange(0.0, 8.0);
    FitResult r = fitter.fit();
    CHECK(r.nPoints == 9 && std::fabs(r.values[1] - 2.0) < 1E-6 && std::fabs(r.values[0] - 1.0) < 1E-6);
    FitRecord record = fitter.record();
    CHECK(record.low[0] == 0.0 && record.high[0] == 8.0);
    fitter.clearRange();
    CHECK(fitter.fit().nPoints == 10);
    bool thrown = false;
    try { fitter.setRange(3.0, 1.0); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);
    thrown = false;
    fitter.setRange(20.0, 30.0);                                            // no point in the range
    try { fitter.fit(); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);
  }

  // ---- starting values estimated from the data
  {
    Histogram h("g", "g", 40, -4.0, 4.0);
    unsigned long long seed = 5;
    for (int k=0; k<4000; k++)
      {
      seed = seed*6364136223846793005ULL + 1442695040888963407ULL;
      double u1 = ((seed >> 11) + 0.5)/9007199254740992.0;
      seed = seed*6364136223846793005ULL + 1442695040888963407ULL;
      double u2 = ((seed >> 11) + 0.5)/9007199254740992.0;
      h.fill(0.3 + 0.9*std::sqrt(-2*std::log(u1))*std::cos(2*M_PI*u2));
      }
    Normal normal(1.0, 0.0, 1.0);
    CHECK(Fitter::estimate(normal, h, -4.0, 4.0));
    CHECK(std::fabs(normal.parameterValues()[1] - 0.3) < 0.1 && std::fabs(normal.parameterValues()[2] - 0.9) < 0.15
          && normal.parameterValues()[0] > 300);
    Fitter poisson(normal, h, Fitter::kPoisson);
    poisson.setRange(-1.0, 1.5);                                            // a Poisson fit over part of the bins
    FitResult r = poisson.fit();
    CHECK(r.converged && r.nPoints == 12 && std::fabs(r.values[1] - 0.3) < 0.1);
    DataPointsVector parabola, decay;
    for (int k=0; k<8; k++)
      {
      double x = 0.5*k;
      parabola.addPoint(DataPoint2D(x, 3.0 - 2.0*x + 0.5*x*x, 0.1));   // (stored as floats)
      decay.addPoint(DataPoint2D(x, 7.0*std::exp(-0.8*x), 0.1));
      }
    Parabolic p(0.0, 0.0, 0.0);
    CHECK(Fitter::estimate(p, parabola, 0.0, 10.0));
    CHECK(std::fabs(p.parameterValues()[0] - 3.0) < 1E-5 && std::fabs(p.parameterValues()[1] + 2.0) < 1E-5 && std::fabs(p.parameterValues()[2] - 0.5) < 1E-5);
    Polynomial cubic(3);
    cubic.setParameterValue(3, 0.25);
    cubic.fixParameter(3);                                                  // a fixed parameter keeps its value
    CHECK(Fitter::estimate(cubic, parabola, 0.0, 10.0) && cubic.parameterValues()[3] == 0.25);
    Exponential e(1.0, 0.0);
    CHECK(Fitter::estimate(e, decay, 0.0, 10.0) && std::fabs(e.parameterValues()[0] - 7.0) < 1E-5 && std::fabs(e.parameterValues()[1] + 0.8) < 1E-5);
    Unclonable unknown;
    CHECK(!Fitter::estimate(unknown, parabola, 0.0, 10.0));
    CHECK(!Fitter::estimate(p, parabola, 20.0, 30.0));                     // no point in the range
  }

  // ---- what the GUI does, in one call: fit and attach, reports, tables
  {
    DataPointsVector points;
    for (int k=0; k<10; k++) points.addPoint(DataPoint2D(k, k < 9 ? 2.0*k + 1.0 : 100.0, 0.1));
    Linear line(0.0, 1.0);
    Fitter::FitOptions options;
    options.hasRange = true;
    options.low = 0.0;
    options.high = 8.0;
    options.label = "straight";
    FitRecord record = Fitter::fitAndAttach(line, points, options);
    CHECK(points.nFits() == 1 && points.fit(0).label == "straight" && record.result.nPoints == 9
          && std::fabs(record.result.values[1] - 2.0) < 1E-6);
    Normal target(1.0, 0.0, 1.0);
    Parabolic parabola(1.0, 0.0, -0.5);
    Fitter::FitOptions few;
    few.samples = 21;
    FitRecord ofFunction = Fitter::fitAndAttach(parabola, target, -1.0, 1.0, few);
    CHECK(target.nFits() == 1 && ofFunction.result.nPoints == 21 && ofFunction.low[0] == -1.0 && ofFunction.high[0] == 1.0);
    bool thrown = false;
    Linear other(0.0, 1.0);
    options.low = 50.0; options.high = 60.0;                                // nothing in the range: nothing attached
    try { Fitter::fitAndAttach(other, points, options); } catch (const MathException &) { thrown = true; }
    CHECK(thrown && points.nFits() == 1);

    // report of a fit
    const FitRecord & fit = points.fit(0);
    CHECK(fit.chiSquareLike() && fit.parameterName(1) == "c1" || fit.parameterName(1).length() > 0);
    double value, error;
    fit.parameterAt(1, value, error);
    CHECK(value == fit.result.values[1] && error == fit.result.errors[1]);
    std::vector<std::pair<String,String>> rows = fit.quality();
    CHECK(rows.size() >= 10 && rows[1].first == "degrees of freedom" && rows[1].second == "7");
    String text = fit.text();
    CHECK(text.contains("straight\tLinear") && text.contains("parameter\tvalue\tuncertainty\tbounds") && text.contains("correlations"));
    CHECK(fit.inRange(std::vector<double>{4.0}) && !fit.inRange(std::vector<double>{9.0}) && !fit.inRange(std::vector<double>{1.0, 2.0}));
    Parameter bounded("b", 1.0, Parameter::LowHigh, 0.0, 2.0);
    CHECK(Parameter("a", 1.0).boundsText() == "free" && bounded.boundsText() == "[0, 2]"
          && Parameter("f", 1.0, Parameter::Fixed).boundsText() == "fixed" && Parameter("l", 1.0, Parameter::Low, 0.5).boundsText() == ">= 0.5");

    // the table of the data, with the columns of the fit
    CHECK(points.description() == "Data points (1D), 10 points");
    std::vector<String> names = points.columnNames();
    std::vector<bool> shown = points.columnsShown();
    CHECK(names.size() == 6 && names[0] == "#" && names[1] == "x" && names[3] == "y" && shown.size() == 6
          && shown[0] && !shown[2] && shown[4] && !shown[5]);
    CHECK(points.rowValues(2).size() == 5 && points.rowValues(2)[0] == "2" && points.rowValues(2)[2] == "5");
    CHECK(points.fitColumnNames() == std::vector<String>({ "f(x) straight", "pull straight" }) && points.fitColumnNames(false).size() == 1);
    std::vector<bool> outside;
    std::vector<String> cells = points.fitValues(9, true, &outside);
    CHECK(cells.size() == 2 && outside.size() == 2 && outside[0] && !points.fitValues(3, true)[1].isNull());
    CHECK(std::fabs(points.pullAt(3, fit) - (7.0 - fit.valueAt(3.0))/0.1) < 1E-3);
    String table = points.table();
    CHECK(table.contains("#\tx\ty\t\xC2\xB1 stat.\tf(x) straight\tpull straight\n") && table.contains("\n9\t9\t100\t0.1\t"));
    CHECK(!points.table(false).contains("f(x)"));
  }

  std::printf("TestFits: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
