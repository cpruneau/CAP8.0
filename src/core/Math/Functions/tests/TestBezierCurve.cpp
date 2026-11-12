/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestBezierCurve.cpp
//! @brief Test program of BezierCurve (run by ctest).
//
// TestBezierCurve -- the curve through its knots; no overshoot (bounded by its knots, hence by [0, 1] for an
// efficiency) for random knots; continuous value and derivative; derivative and integral against numbers; knots
// inserted on the curve, moved within their neighbors and the bounds, removed; the bounds kept through a copy, the
// registry and a record written and read back; a product eff(pT) f(eta) g(phi). Prints each failed check and exits
// with the number of failures.
//
#include <cmath>
#include <cstdio>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include "BezierCurve.hpp"
#include "BinaryIO.hpp"
#include "Exceptions.hpp"
#include "FunctionRegistry.hpp"
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

#define CHECK(condition) check((condition), #condition, __LINE__)

bool close(double a, double b, double tolerance=1.0E-9) { return std::fabs(a - b) <= tolerance*(1.0 + std::fabs(b)); }

std::unique_ptr<ParameterizedFunction> roundTrip(const ParameterizedFunction & f)
{
  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  writeBinaryObject(stream, f);
  stream.seekg(0);
  return std::unique_ptr<ParameterizedFunction>(ParameterizedFunction::readBinary(stream));
}
} // namespace

int main()
{
  // through the knots, held outside them
  BezierCurve e({0.2, 0.5, 1.0, 3.0, 10.0}, {0.0, 0.6, 0.85, 0.95, 0.9}, 0.0, 1.0);
  CHECK(e.nKnots() == 5 && e.nParameters() == 10);
  for (unsigned int k=0; k<e.nKnots(); k++) CHECK(close(e(e.knotX(k)), e.knotY(k)));
  CHECK(close(e(0.0), 0.0) && close(e(50.0), 0.9));
  CHECK(e.lower() == 0.0 && e.upper() == 1.0);
  CHECK(e.parameter(1).name() == "y0" && e.parameter(2).name() == "x1");

  // no overshoot: between two knots, the curve stays between their values (random knots, many points)
  std::mt19937 random(12345);
  std::uniform_real_distribution<double> uniform(0.0, 1.0);
  bool bounded = true, betweenKnots = true;
  for (int trial=0; trial<200; trial++)
    {
    unsigned int n = 2 + unsigned(trial%9);
    std::vector<double> x, y;
    double xk = 0.0;
    for (unsigned int k=0; k<n; k++) { xk += 0.01 + uniform(random)*(trial%2 ? 1.0 : 0.05); x.push_back(xk); y.push_back(uniform(random)); }
    BezierCurve c(x, y, 0.0, 1.0);
    for (unsigned int k=0; k+1<n; k++)
      {
      double lo = std::min(y[k], y[k+1]), hi = std::max(y[k], y[k+1]);
      for (int s=0; s<=50; s++)
        {
        double v = c(x[k] + (x[k+1] - x[k])*s/50.0);
        if (v < -1.0E-12 || v > 1.0 + 1.0E-12) bounded = false;
        if (v < lo - 1.0E-12 || v > hi + 1.0E-12) betweenKnots = false;
        }
      std::vector<double> p = c.controlPoints(k);
      if (p[1] < lo - 1.0E-12 || p[1] > hi + 1.0E-12 || p[2] < lo - 1.0E-12 || p[2] > hi + 1.0E-12) betweenKnots = false;
      }
    }
  CHECK(bounded);
  CHECK(betweenKnots);

  // value and derivative continuous at the knots; derivative and integral against numbers
  for (unsigned int k=1; k+1<e.nKnots(); k++)
    {
    double x = e.knotX(k), dx = 1.0E-7;
    CHECK(close(e(x - dx), e(x + dx), 1.0E-5));
    CHECK(close(e.derivativeAt(x - dx), e.derivativeAt(x + dx), 1.0E-4));
    }
  for (double x : {0.3, 0.77, 2.0, 6.5})
    CHECK(close(e.derivativeAt(x), (e(x + 1.0E-6) - e(x - 1.0E-6))/2.0E-6, 1.0E-5));
  double sum = 0.0;
  int nSteps = 200000;
  for (int s=0; s<nSteps; s++) sum += e(-1.0 + 13.0*(s + 0.5)/nSteps)*13.0/nSteps;
  CHECK(close(e.integralInRange(-1.0, 12.0), sum, 1.0E-7));
  CHECK(close(e.integralInRange(12.0, -1.0), -sum, 1.0E-7));
  CHECK(close(e.integralInRange(0.6, 0.8), (e(0.6) + 4.0*e(0.7) + e(0.8))*0.2/6.0, 1.0E-4));

  // a monotone set of knots gives a monotone curve
  BezierCurve rising({0.0, 1.0, 2.0, 5.0}, {0.0, 0.1, 0.9, 1.0}, 0.0, 1.0);
  bool monotone = true;
  for (int s=1; s<=500; s++) if (rising(5.0*s/500.0) < rising(5.0*(s - 1)/500.0) - 1.0E-14) monotone = false;
  CHECK(monotone);

  // knots: inserted on the curve, moved within the neighbors and the bounds, removed
  BezierCurve edited = e;
  double before = edited(2.0);
  int k = edited.insertKnot(2.0);
  CHECK(k == 3 && edited.nKnots() == 6 && close(edited.knotY(3), before));
  CHECK(edited.insertKnot(2.0) == -1);
  CHECK(edited.lower() == 0.0 && edited.upper() == 1.0);
  edited.moveKnot(3, 100.0, 7.0);
  CHECK(edited.knotX(3) < edited.knotX(4) && edited.knotX(3) > edited.knotX(2) && edited.knotY(3) == 1.0);
  edited.moveKnot(3, -100.0, -7.0);
  CHECK(edited.knotX(3) > edited.knotX(2) && edited.knotY(3) == 0.0);
  edited.moveKnot(0, -5.0, 0.5);
  CHECK(edited.knotX(0) == -5.0 && edited.knotY(0) == 0.5);
  CHECK(edited.removeKnot(3) && edited.nKnots() == 5);
  BezierCurve two;
  CHECK(!two.removeKnot(0) && two.nKnots() == 2);
  CHECK(close(two(0.25), 0.25));                   // two knots: a straight line

  // knots given out of order are sorted; two at the same x refused; bounds clamp the values
  BezierCurve unsorted({3.0, 1.0, 2.0}, {0.3, 0.1, 5.0}, 0.0, 1.0);
  CHECK(unsorted.knotX(0) == 1.0 && unsorted.knotX(2) == 3.0 && unsorted.knotY(1) == 1.0);
  bool thrown = false;
  try { BezierCurve bad({1.0, 1.0}, {0.0, 1.0}); } catch (const std::exception &) { thrown = true; }
  CHECK(thrown);
  BezierCurve resolution({0.0, 1.0}, {0.02, 0.01}, 0.0);
  CHECK(resolution.lower() == 0.0 && resolution.upper() == Parameter::defaultMaximum);
  resolution.setBounds(0.0, 0.015);
  CHECK(resolution.knotY(0) == 0.015 && resolution.upper() == 0.015);

  // registry and records: the knots and the bounds kept
  CHECK(FunctionRegistry::typeName(e) == "BezierCurve");
  std::unique_ptr<ParameterizedFunction> made(FunctionRegistry::create("BezierCurve", 10));
  CHECK(made && dynamic_cast<BezierCurve *>(made.get()) && dynamic_cast<BezierCurve *>(made.get())->nKnots() == 5);
  CHECK(FunctionRegistry::create("BezierCurve", 5) == nullptr);
  std::unique_ptr<ParameterizedFunction> read = roundTrip(e);
  const BezierCurve * back = dynamic_cast<const BezierCurve *>(read.get());
  CHECK(back && back->nKnots() == e.nKnots() && back->lower() == 0.0 && back->upper() == 1.0);
  if (back) for (double x : {0.1, 0.4, 0.9, 2.5, 7.0}) CHECK(close((*back)(x), e(x)));

  // a product eff(pT) f(eta) g(phi): bounded by 1, written and read back
  ParameterizedComposite product(ParameterizedComposite::kProduct);
  product.add("pt", e);
  product.add("eta", BezierCurve({-1.0, 0.0, 1.0}, {0.8, 1.0, 0.8}, 0.0, 1.0));
  product.add("phi", BezierCurve({0.0, 6.2832}, {1.0, 1.0}, 0.0, 1.0));
  CHECK(product.rank() == 3);
  CHECK(close(product.evaluateAt(std::vector<double>{3.0, 0.0, 1.0}), 0.95));
  CHECK(close(product.evaluateAt(std::vector<double>{3.0, 1.0, 1.0}), 0.95*0.8));
  std::unique_ptr<ParameterizedFunction> productBack = roundTrip(product);
  CHECK(productBack && productBack->rank() == 3
        && close(productBack->evaluateAt(std::vector<double>{1.0, 0.5, 2.0}), product.evaluateAt(std::vector<double>{1.0, 0.5, 2.0})));
  const ParameterizedComposite * composite = dynamic_cast<const ParameterizedComposite *>(productBack.get());
  CHECK(composite && composite->nComponents() == 3 && dynamic_cast<const BezierCurve *>(&composite->component(1)));

  std::printf("TestBezierCurve: %d checks, %d failures\n", nChecks, nFailures);
  return nFailures;
}
