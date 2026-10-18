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
//! @file TestFitter.cpp
//! @brief Test program of Fitter, Matrix::inverse()/solve(), ChiSquare::probability() and the parameter setters (run by ctest).
//
// TestFitter -- tests of CAP::MATH::Fitter against exact solutions: weighted straight line (closed form), exact
// polynomial, Normal and exponential data, a plane (normal equations), a noisy histogram, fixed and bounded
// parameters, empty bins, scaled errors, and fits by a search engine. Also Matrix::inverse() and solve(), the
// p-value of a chi square, and the public parameter setters of ParameterizedFunction. Prints each failed check and
// exits with the number of failures (0: all passed).
//
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include "DataPoint3D.hpp"
#include "DataPointsVector.hpp"
#include "Exponential.hpp"
#include "Fitter.hpp"
#include "Histogram.hpp"
#include "Linear.hpp"
#include "MathExceptions.hpp"
#include "Normal.hpp"
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

bool close(double a, double b, double relative) { return std::fabs(a - b) <= relative*std::max(1.0, std::max(std::fabs(a), std::fabs(b))); }

std::string show(const FitResult & r)
{
  std::string s = " [" + std::string(r.method.data()) + ": " + r.status.data() + ";";
  for (std::size_t k=0; k<r.values.size(); k++) { char t[64]; std::snprintf(t, sizeof(t), " %.8g+-%.3g", r.values[k], r.errors[k]); s += t; }
  char t[96];
  std::snprintf(t, sizeof(t), "; chi2 %.6g ndf %d p %.3g; %ld evaluations]", r.chiSquare, r.nDegreesOfFreedom, r.pValue, r.nEvaluations);
  return s + t;
}

//! a + b x + c y: a function of rank 2 with three parameters.
class Plane : public ParameterizedFunction
{
public:
  Plane() : ParameterizedFunction({ Parameter("a", 0.0), Parameter("b", 0.0), Parameter("c", 0.0) }, 2) { }
  double evaluateAt(const std::vector<double> & x) const override
  {
    checkRank(x.size(), __FUNCTION__);
    return parameterValue(0) + parameterValue(1)*x[0] + parameterValue(2)*x[1];
  }
  using ParameterizedFunction::evaluateAt;
};
} // namespace

//! @brief Count a check, and report it if the condition is false.
#define CHECK(condition) check((condition), #condition, __LINE__)
//! @brief Count a check about a fit, and report the fit if the condition is false.
#define CHECK_FIT(condition, result) check((condition), std::string(#condition) + show(result), __LINE__)

//! @brief Run the checks. @return number of failed checks (0: all passed)
int main()
{
  // ---- p-value of a chi square: closed forms for 1, 2 and 4 degrees of freedom
  for (double x : { 0.1, 1.0, 3.84146, 10.0, 50.0 })
    {
    check(close(ChiSquare::probability(x, 1), std::erfc(std::sqrt(x/2)), 1E-12), "Q(1/2, x/2) = erfc(sqrt(x/2))", __LINE__);
    check(close(ChiSquare::probability(x, 2), std::exp(-x/2), 1E-12), "Q(1, x/2) = exp(-x/2)", __LINE__);
    check(close(ChiSquare::probability(x, 4), std::exp(-x/2)*(1 + x/2), 1E-12), "Q(2, x/2) = exp(-x/2)(1 + x/2)", __LINE__);
    }
  CHECK(std::fabs(ChiSquare::probability(3.84146, 1) - 0.05) < 1E-5);
  CHECK(ChiSquare::probability(0.0, 5) == 1.0 && ChiSquare::probability(1E4, 5) < 1E-100);
  {
    bool thrown = false;
    try { ChiSquare::probability(1.0, 0); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);
  }

  // ---- Matrix: inverse, solve, transpose
  {
    Matrix a(2, 2, {4.0, 7.0, 2.0, 6.0});
    Matrix inverse = a.inverse();
    CHECK(close(inverse(0, 0), 0.6, 1E-14) && close(inverse(0, 1), -0.7, 1E-14) && close(inverse(1, 0), -0.2, 1E-14) && close(inverse(1, 1), 0.4, 1E-14));
    Matrix b(3, 3, {2.0, -1.0, 0.0, -1.0, 2.0, -1.0, 0.0, -1.0, 2.0});
    Matrix product = b*b.inverse();
    bool identity = true;
    for (unsigned int r=0; r<3; r++) for (unsigned int c=0; c<3; c++) identity = identity && std::fabs(product(r, c) - (r == c ? 1.0 : 0.0)) < 1E-14;
    CHECK(identity);
    std::vector<double> x = b.solve({1.0, 0.0, 1.0});
    CHECK(close(x[0], 1.0, 1E-14) && close(x[1], 1.0, 1E-14) && close(x[2], 1.0, 1E-14));
    Matrix c(2, 3, {1, 2, 3, 4, 5, 6});
    Matrix t = c.transpose();
    CHECK(t.nRows() == 3 && t.nColumns() == 2 && t(2, 0) == 3 && t(0, 1) == 4);
    Matrix permuted(2, 2, {0.0, 1.0, 1.0, 0.0});                           // needs a row exchange
    CHECK(close(permuted.inverse()(0, 1), 1.0, 1E-15));
    bool thrown = false;
    try { Matrix(2, 2, {1.0, 2.0, 2.0, 4.0}).inverse(); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);                                                          // singular
    thrown = false;
    try { c.inverse(); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);                                                          // not square
  }

  // ---- public parameter setters
  {
    Normal f(1.0, 0.0, 1.0);
    f.setParameterValues({2.0, 0.5, 0.3});
    CHECK(f.parameterValues() == std::vector<double>({2.0, 0.5, 0.3}) && close(f.evaluateAt(0.5), 2.0, 1E-15));
    f.fixParameter(2);
    CHECK(f.freeParameters() == std::vector<unsigned int>({0, 1}) && f.parameter(2).type() == Parameter::Fixed);
    f.fixParameter(2, false);
    CHECK(f.parameter(2).type() == Parameter::Free && f.freeParameters().size() == 3);
    f.setParameterLimits(1, -1.0, 0.2);                                     // the value 0.5 moves to 0.2
    CHECK(f.parameter(1).type() == Parameter::LowHigh && f.parameter(1).value() == 0.2);
    f.fixParameter(1); f.fixParameter(1, false);
    CHECK(f.parameter(1).type() == Parameter::LowHigh);
    bool thrown = false;
    try { f.setParameterValue(1, 3.0); } catch (const CAP::Exception &) { thrown = true; }   // a MathDomainException
    CHECK(thrown);                                                          // outside its bounds
  }

  // ---- weighted straight line: the closed form of least squares
  {
    DataPointsVector data;
    data.addPoints({0.0, 1.0, 2.0, 3.0, 4.0, 5.0}, {1.1, 2.9, 5.2, 6.8, 9.1, 11.3}, {0.1, 0.2, 0.1, 0.3, 0.2, 0.4});
    double S = 0, Sx = 0, Sy = 0, Sxx = 0, Sxy = 0;
    for (unsigned int k=0; k<data.size(); k++)
      {
      DataPoint2D p = data.point(k);                                        // the stored (float) values
      double w = 1.0/(double(p.yStatError())*p.yStatError());
      S += w; Sx += w*p.x(); Sy += w*p.y(); Sxx += w*p.x()*p.x(); Sxy += w*p.x()*p.y();
      }
    double delta = S*Sxx - Sx*Sx;
    double a = (Sxx*Sy - Sx*Sxy)/delta, b = (S*Sxy - Sx*Sy)/delta;
    Linear line(0.0, 1.0);
    Fitter fitter(line, data);
    FitResult r = fitter.fit();
    CHECK_FIT(r.converged && r.validErrors && close(r.values[0], a, 1E-8) && close(r.values[1], b, 1E-8), r);
    CHECK_FIT(close(r.errors[0], std::sqrt(Sxx/delta), 1E-6) && close(r.errors[1], std::sqrt(S/delta), 1E-6), r);
    CHECK_FIT(close(r.covariance(0, 1), -Sx/delta, 1E-6) && r.correlation(0, 0) == 1.0, r);
    double chi2 = 0;
    for (unsigned int k=0; k<data.size(); k++)
      { DataPoint2D p = data.point(k); double t = (p.y() - a - b*p.x())/p.yStatError(); chi2 += t*t; }
    CHECK_FIT(close(r.chiSquare, chi2, 1E-8) && r.nPoints == 6 && r.nDegreesOfFreedom == 4 && close(r.pValue, ChiSquare::probability(chi2, 4), 1E-8), r);
    CHECK(line.parameterValues() == r.values);                              // the function is left at the best values
    fitter.setScaleErrors(true);
    FitResult scaled = fitter.fit();
    CHECK_FIT(close(scaled.errors[1], r.errors[1]*std::sqrt(chi2/4), 1E-6), scaled);
  }

  // ---- exact data: the parameters are recovered from poor starting values
  {
    Polynomial truth(2), model(2);
    truth.setParameterValues({1.5, -2.0, 0.75});
    DataPointsVector data;
    for (int k=0; k<10; k++) data.addPoint(DataPoint2D(0.5*k, truth.evaluateAt(0.5*k), 0.1));
    FitResult r = Fitter(model, data).fit();
    CHECK_FIT(close(r.values[0], 1.5, 1E-5) && close(r.values[1], -2.0, 1E-5) && close(r.values[2], 0.75, 1E-5) && r.chiSquare < 1E-6, r);
    Normal gaussian(25.0, 1.2, 0.8);
    DataPointsVector peak;
    for (int k=0; k<40; k++) { double x = -2 + 0.15*k, y = gaussian.evaluateAt(x); peak.addPoint(DataPoint2D(x, y, std::sqrt(y) + 0.1)); }
    Normal start(10.0, 0.0, 2.0);
    r = Fitter(start, peak).fit();
    CHECK_FIT(r.converged && close(r.values[0], 25.0, 1E-5) && close(r.values[1], 1.2, 1E-5) && close(std::fabs(r.values[2]), 0.8, 1E-5), r);
    Exponential decay(3.0, -0.4);
    DataPointsVector counts;
    for (int k=0; k<20; k++) { double x = 0.5*k, y = decay.evaluateAt(x); counts.addPoint(DataPoint2D(x, y, 0.05*y + 0.01)); }
    Exponential guess(1.0, -1.0);
    r = Fitter(guess, counts).fit();
    CHECK_FIT(close(r.values[0], 3.0, 1E-5) && close(r.values[1], -0.4, 1E-5), r);
  }

  // ---- rank 2: a plane, against the normal equations
  {
    DataPointsVector data;
    unsigned long seed = 3;
    auto uniform = [&seed]() { seed = seed*6364136223846793005ULL + 1442695040888963407ULL; return ((seed >> 11) + 0.5)/9007199254740992.0; };
    for (int k=0; k<30; k++)
      {
      double x = 4*uniform(), y = 4*uniform();
      data.addPoint(DataPoint3D(x, y, 1.0 + 0.5*x - 0.25*y + 0.1*(uniform() - 0.5), 0.05 + 0.05*uniform()));
      }
    Matrix A(3, 3, 0.0);
    std::vector<double> rhs(3, 0.0);
    for (unsigned int k=0; k<data.size(); k++)
      {
      std::vector<double> c = data.coordinatesAt(k);
      double basis[3] = { 1.0, c[0], c[1] }, w = 1.0/(data.statErrorAt(k)*data.statErrorAt(k));
      for (int i=0; i<3; i++) { rhs[i] += w*basis[i]*data.valueAt(k); for (int j=0; j<3; j++) A(i, j) += w*basis[i]*basis[j]; }
      }
    std::vector<double> exact = A.solve(rhs);
    Matrix covariance = A.inverse();
    Plane plane;
    FitResult r = Fitter(plane, data).fit();
    CHECK_FIT(r.converged && close(r.values[0], exact[0], 1E-7) && close(r.values[1], exact[1], 1E-7) && close(r.values[2], exact[2], 1E-7), r);
    CHECK_FIT(close(r.errors[2], std::sqrt(covariance(2, 2)), 1E-5) && close(r.covariance(0, 2), covariance(0, 2), 1E-5) && r.nDegreesOfFreedom == 27, r);
  }

  // ---- a noisy histogram: estimates within their uncertainties, a sensible chi square; empty bins left out
  {
    Histogram h("h", "h", 60, -6.0, 6.0);
    unsigned long seed = 11;
    auto uniform = [&seed]() { seed = seed*6364136223846793005ULL + 1442695040888963407ULL; return ((seed >> 11) + 0.5)/9007199254740992.0; };
    for (int k=0; k<5000; k++) h.fill(0.3 + 1.1*std::sqrt(-2*std::log(uniform()))*std::cos(2*M_PI*uniform()));
    unsigned int empty = 0;
    for (unsigned int b=0; b<60; b++) if (h.valueAt(b) == 0) empty++;
    Normal model(100.0, 0.0, 1.0);
    Fitter fitter(model, h);
    FitResult r = fitter.fit();
    double expectedAmplitude = 5000*0.2/(std::sqrt(2*M_PI)*1.1);
    CHECK_FIT(r.converged && r.validErrors && empty > 0 && r.nPoints == int(60 - empty), r);
    CHECK_FIT(std::fabs(r.values[1] - 0.3) < 4*r.errors[1] && std::fabs(std::fabs(r.values[2]) - 1.1) < 4*r.errors[2]
              && std::fabs(r.values[0] - expectedAmplitude) < 0.1*expectedAmplitude, r);
    CHECK_FIT(r.chiSquarePerDegreeOfFreedom > 0.3 && r.chiSquarePerDegreeOfFreedom < 3 && r.pValue > 1E-4, r);
    CHECK_FIT(std::fabs(r.correlation(0, 2)) > 0.1 && std::fabs(r.correlation(0, 2)) < 1, r);   // amplitude and width are correlated

    // a fixed width: one free parameter less
    model.setParameterValues({100.0, 0.0, 1.1});
    model.fixParameter(2);
    FitResult fixed = fitter.fit();
    CHECK_FIT(fixed.values[2] == 1.1 && fixed.errors[2] == 0 && fixed.nDegreesOfFreedom == r.nDegreesOfFreedom + 1
              && fixed.freeParameters.size() == 2 && fixed.covariance.nRows() == 2, fixed);
    model.fixParameter(2, false);
    // a bounded mean, below the true one: the fit stops at the bound
    model.setParameterValues({100.0, 0.0, 1.0});
    model.setParameterLimits(1, -1.0, 0.1);
    FitResult bounded = fitter.fit();
    CHECK_FIT(bounded.values[1] == 0.1 && bounded.atLimit[1] && !bounded.atLimit[0], bounded);

    // a search engine (differential evolution over bounded parameters) from poor starting values
    model.setParameterLimits(0, 1.0, 1000.0);
    model.setParameterLimits(1, -5.0, 5.0);
    model.setParameterLimits(2, 0.1, 5.0);
    model.setParameterValues({10.0, -4.0, 4.0});
    fitter.setMethod(Fitter::kSearch);
    fitter.search().setMethod(ExtremumSearch::kDifferentialEvolution);
    FitResult searched = fitter.fit();
    CHECK_FIT(searched.method == "differential evolution + Levenberg-Marquardt" && close(searched.values[1], r.values[1], 1E-5)
              && close(searched.values[2], std::fabs(r.values[2]), 1E-5) && close(searched.chiSquare, r.chiSquare, 1E-7), searched);
    fitter.setPolish(false);
    fitter.search().setMethod(ExtremumSearch::kAnnealing);
    model.setParameterValues({10.0, -4.0, 4.0});
    FitResult annealed = fitter.fit();
    CHECK_FIT(annealed.method == "simulated annealing" && std::fabs(annealed.values[1] - r.values[1]) < 2*r.errors[1], annealed);
  }

  // ---- errors
  {
    DataPointsVector data;
    data.addPoints({0.0, 1.0}, {1.0, 2.0}, {0.1, 0.1});
    Linear line(0.0, 1.0);
    line.fixParameter(0); line.fixParameter(1);
    bool thrown = false;
    try { Fitter(line, data).fit(); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);                                                          // nothing to fit
    Plane plane;
    thrown = false;
    try { Fitter fitter(plane, data); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);                                                          // ranks differ
    DataPointsVector noErrors;
    noErrors.addPoints({0.0, 1.0, 2.0}, {1.0, 2.0, 3.0});
    Linear free(0.0, 1.0);
    thrown = false;
    try { Fitter(free, noErrors).fit(); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);                                                          // no point with an uncertainty
  }

  std::printf("TestFitter: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
