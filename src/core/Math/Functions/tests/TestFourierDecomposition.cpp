/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestFourierDecomposition.cpp
//! @brief Test program of FourierDecomposition and FourierHarmonic (run by ctest).
//
// TestFourierDecomposition -- both forms against the formulas; parameters named and ordered k, c1, s1, ... and k, v1,
// psi1, ...; the coefficients of one form read in the other, the setters of the other form refused; derivative and
// integral against numbers; copies; the registry; records written and read back as a FourierDecomposition of its own
// form; a fit of form 2 to a histogram-like set of points through the parameters. Prints each failed check and exits
// with the number of failures.
//
#include <cmath>
#include <cstdio>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "BinaryIO.hpp"
#include "Exceptions.hpp"
#include "FourierDecomposition.hpp"
#include "FunctionRegistry.hpp"

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

bool close(double a, double b, double tolerance=1.0E-10) { return std::fabs(a - b) <= tolerance*(1.0 + std::fabs(b)); }

std::unique_ptr<ParameterizedFunction> roundTrip(const ParameterizedFunction & f)
{
  std::stringstream stream(std::ios::in | std::ios::out | std::ios::binary);
  writeBinaryObject(stream, f);
  stream.seekg(0);
  return std::unique_ptr<ParameterizedFunction>(ParameterizedFunction::readBinary(stream));
}

template <typename F>
bool throws(F f) { try { f(); } catch (const std::exception &) { return true; } return false; }
} // namespace

int main()
{
  // form 1: k + sum c_n cos(n x) + s_n sin(n x)
  FourierDecomposition f(FourierDecomposition::kCosineSine, 2.0, {0.3, -0.2, 0.05}, {0.1, 0.0, -0.4});
  CHECK(f.form() == FourierDecomposition::kCosineSine && f.nMax() == 3 && f.nParameters() == 7 && f.rank() == 1);
  CHECK(f.nComponents() == 4);
  CHECK(f.parameter(0).name() == "k" && f.parameter(1).name() == "c1" && f.parameter(2).name() == "s1"
        && f.parameter(6).name() == "s3");
  CHECK(f.harmonicIndex(2) == 3 && throws([&] { f.harmonicIndex(4); }) && throws([&] { f.harmonicIndex(0); }));
  auto formula1 = [](double x)
    { return 2.0 + 0.3*std::cos(x) + 0.1*std::sin(x) - 0.2*std::cos(2*x) + 0.05*std::cos(3*x) - 0.4*std::sin(3*x); };
  for (double x : {-3.0, -0.7, 0.0, 0.4, 1.9, 3.1, 7.5}) CHECK(close(f(x), formula1(x)));
  CHECK(close(f.evaluateAt(std::vector<double>{0.4}), formula1(0.4)));

  // form 2: k (1 + 2 sum v_n cos(n (x - psi_n)))
  FourierDecomposition g(4, FourierDecomposition::kFlow);
  CHECK(g.form() == FourierDecomposition::kFlow && g.nMax() == 4 && g.nParameters() == 9);
  CHECK(g.parameter(0).name() == "k" && g.parameter(1).name() == "v1" && g.parameter(2).name() == "psi1"
        && g.parameter(8).name() == "psi4");
  g.setConstant(100.0);
  g.setFlow(2, 0.06);
  g.setFlowPhase(2, 0.3);
  g.setFlow(3, 0.02);
  g.setFlowPhase(3, -0.5);
  auto formula2 = [](double x) { return 100.0*(1.0 + 2.0*(0.06*std::cos(2*(x - 0.3)) + 0.02*std::cos(3*(x + 0.5)))); };
  for (double x : {-3.0, -0.7, 0.0, 0.4, 1.9, 3.1}) CHECK(close(g(x), formula2(x)));
  CHECK(close(g(0.0), 100.0*(1.0 + 2.0*(0.06*std::cos(0.6) + 0.02*std::cos(1.5)))));

  // the coefficients of one form read in the other; the setters of the other form refused
  CHECK(close(g.cosine(2), 2.0*100.0*0.06*std::cos(0.6)) && close(g.sine(2), 2.0*100.0*0.06*std::sin(0.6)));
  CHECK(g.flow(2) == 0.06 && g.flowPhase(2) == 0.3 && g.cosine(1) == 0.0);
  FourierDecomposition h(FourierDecomposition::kCosineSine, 100.0, {0.0, g.cosine(2), g.cosine(3)},
                         {0.0, g.sine(2), g.sine(3)});
  for (double x : {-2.0, 0.1, 1.3}) CHECK(close(h(x), g(x)));
  CHECK(close(h.flow(2), 0.06) && close(h.flowPhase(2), 0.3) && close(h.flow(3), 0.02) && close(h.flowPhase(3), -0.5));
  CHECK(h.flowPhase(1) == 0.0 && h.flow(1) == 0.0);
  CHECK(throws([&] { g.setCosine(1, 1.0); }) && throws([&] { h.setFlow(1, 1.0); }));
  FourierDecomposition zero(2);
  CHECK(throws([&] { zero.flow(1); }));
  CHECK(throws([] { FourierDecomposition bad(0); }) && throws([] { FourierHarmonic bad(0); }));

  // derivative and integral against numbers, both forms
  for (const FourierDecomposition * d : {&f, &g})
    {
    for (double x : {-1.2, 0.3, 2.2})
      CHECK(close(d->derivativeAt(x), ((*d)(x + 1.0E-6) - (*d)(x - 1.0E-6))/2.0E-6, 1.0E-6));
    double low = -0.8, high = 2.6, sum = 0.0;
    int nSteps = 100000;
    for (int s=0; s<nSteps; s++) sum += (*d)(low + (high - low)*(s + 0.5)/nSteps)*(high - low)/nSteps;
    CHECK(close(d->integralInRange(low, high), sum, 1.0E-8));
    CHECK(close(d->integralInRange(std::vector<double>{low}, std::vector<double>{high}), sum, 1.0E-8));
    CHECK(close(d->integralInRange(0.0, 2.0*M_PI), 2.0*M_PI*d->constant()));   // one period: the constant only
    }

  // the parameters are those that count (e.g. set by a fit)
  FourierDecomposition fitted = g;
  std::vector<double> values = fitted.parameterValues();
  values[0] = 50.0;
  values[3] = 0.1;
  fitted.setParameterValues(values);
  CHECK(close(fitted(0.7), 50.0*(1.0 + 2.0*(0.1*std::cos(2*(0.7 - 0.3)) + 0.02*std::cos(3*(0.7 + 0.5))))));
  CHECK(close(g(0.7), formula2(0.7)));                     // the copy is independent
  std::unique_ptr<ParameterizedFunction> cloned(g.clone());
  CHECK(dynamic_cast<FourierDecomposition *>(cloned.get()) && close((*cloned)(1.1), g(1.1)));

  // the harmonic alone
  FourierHarmonic flowTerm(2, FourierHarmonic::kFlow, 0.5, 0.25);
  CHECK(flowTerm.parameter(0).name() == "v" && flowTerm.parameter(1).name() == "psi");
  CHECK(close(flowTerm(1.0), 0.5*std::cos(2.0*0.75)));
  CHECK(close(flowTerm.integralInRange(0.0, M_PI), 0.0));

  // registry and records: read back as a FourierDecomposition of its own form
  CHECK(FunctionRegistry::typeName(f) == "FourierDecomposition" && FunctionRegistry::typeName(flowTerm) == "FourierHarmonic");
  std::unique_ptr<ParameterizedFunction> made(FunctionRegistry::create("FourierDecomposition", 7));
  const FourierDecomposition * three = dynamic_cast<const FourierDecomposition *>(made.get());
  CHECK(three && three->nMax() == 3 && three->form() == FourierDecomposition::kCosineSine);
  CHECK(FunctionRegistry::create("FourierDecomposition", 6) == nullptr);
  std::unique_ptr<ParameterizedFunction> standard(FunctionRegistry::create("FourierDecomposition"));
  CHECK(standard && standard->nParameters() == 9);
  for (const FourierDecomposition * d : {&f, &g})
    {
    std::unique_ptr<ParameterizedFunction> read = roundTrip(*d);
    const FourierDecomposition * back = dynamic_cast<const FourierDecomposition *>(read.get());
    CHECK(back && back->form() == d->form() && back->nMax() == d->nMax());
    if (!back) continue;
    for (unsigned int k=0; k<d->nParameters(); k++) CHECK(back->parameter(k).name() == d->parameter(k).name());
    for (double x : {-2.5, 0.2, 1.7}) CHECK(close((*back)(x), (*d)(x)));
    }
  FourierFlow flowForm(2);
  flowForm.setConstant(3.0);
  flowForm.setFlow(2, 0.2);
  std::unique_ptr<ParameterizedFunction> flowBack = roundTrip(flowForm);
  CHECK(dynamic_cast<FourierFlow *>(flowBack.get()) && close((*flowBack)(0.4), flowForm(0.4)));
  std::unique_ptr<ParameterizedFunction> flowMade(FunctionRegistry::create("FourierFlow", 5));
  CHECK(dynamic_cast<FourierFlow *>(flowMade.get()) && dynamic_cast<FourierFlow *>(flowMade.get())->form() == FourierDecomposition::kFlow);
  std::unique_ptr<ParameterizedFunction> termBack = roundTrip(flowTerm);
  const FourierHarmonic * term = dynamic_cast<const FourierHarmonic *>(termBack.get());
  CHECK(term && term->order() == 2 && term->form() == FourierHarmonic::kFlow && close((*term)(1.0), flowTerm(1.0)));

  std::printf("TestFourierDecomposition: %d checks, %d failures\n", nChecks, nFailures);
  return nFailures;
}
