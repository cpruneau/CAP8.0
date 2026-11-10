/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestDerivative.cpp
//! @brief Test program of the derivatives: finite differences, Differentiation and Derivative (run by ctest).
//
// TestDerivative -- Fornberg weights against the classical formulas; offsets and orders of accuracy; numerical
// derivatives exact for polynomials of their degree, errors falling as h^p; each method (Linear, Quadratic, Cubic,
// Polynomial) and stencil; Richardson; partial, mixed and gradient; derivatives of samples and on grids; analytical
// derivatives of the functions (derivativeFunction(): polynomials, exponentials, orthogonal series, Fourier,
// Polynomial2D, splines; derivativeAt(): Normal, Bessel); Derivative as a function (analytical, numerical, of any order,
// mixed, derivatives of derivatives); the numerical defaults of Function. Prints each failed check and exits with the
// number of failures.
//
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include "FiniteDifference.hpp"
#include "Differentiation.hpp"
#include "Derivative.hpp"
#include "Polynomial.hpp"
#include "Exponential.hpp"
#include "Normal.hpp"
#include "Polynomial2D.hpp"
#include "OrthogonalSeries.hpp"
#include "FourierDecomposition.hpp"
#include "CubicSpline.hpp"
#include "Bessel.hpp"
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

#define CHECK(condition) check((condition), #condition, __LINE__)

bool close(double a, double b, double tolerance=1.0E-10) { return std::fabs(a - b) <= tolerance*(1.0 + std::fabs(b)); }

template <typename F>
bool throws(F f) { try { f(); } catch (const std::exception &) { return true; } return false; }

bool sameVector(const std::vector<double> & a, const std::vector<double> & b, double tolerance=1.0E-12)
{
  if (a.size() != b.size()) return false;
  for (std::size_t i=0; i<a.size(); i++) if (!close(a[i], b[i], tolerance)) return false;
  return true;
}

// a plain Function of rank 2 (no analytical derivative): sin(x) e^y
class Plain : public Function
{
public:
  Plain() : Function(2) {}
  double evaluateAt(const std::vector<double> & x) const override { return std::sin(x[0])*std::exp(x[1]); }
  using Function::evaluateAt;
};

// a plain Function of rank 1: x^3 sin(x)
class Plain1 : public Function
{
public:
  Plain1() : Function(1) {}
  double evaluateAt(const std::vector<double> & x) const override { return x[0]*x[0]*x[0]*std::sin(x[0]); }
  using Function::evaluateAt;
};
} // namespace

int main()
{
  // ---------------------------------------------------------------- weights, offsets, accuracy
  CHECK(sameVector(finiteDifferenceWeights(1, {-1, 0, 1}), {-0.5, 0.0, 0.5}));
  CHECK(sameVector(finiteDifferenceWeights(2, {-1, 0, 1}), {1.0, -2.0, 1.0}));
  CHECK(sameVector(finiteDifferenceWeights(1, {-2, -1, 0, 1, 2}), {1.0/12, -8.0/12, 0.0, 8.0/12, -1.0/12}));
  CHECK(sameVector(finiteDifferenceWeights(1, {0, 1, 2}), {-1.5, 2.0, -0.5}));
  CHECK(sameVector(finiteDifferenceWeights(0, {0, 1}), {1.0, 0.0}));
  CHECK(throws([] { finiteDifferenceWeights(3, {0, 1, 2}); }) && throws([] { finiteDifferenceWeights(1, {0, 0}); }));
  CHECK(sameVector(finiteDifferenceOffsets(3, Stencil::Central), {-1.5, -0.5, 0.5, 1.5}));
  CHECK(sameVector(finiteDifferenceOffsets(2, Stencil::Backward), {-2, -1, 0}));
  CHECK(finiteDifferenceAccuracy(1, 4, Stencil::Central) == 4 && finiteDifferenceAccuracy(2, 2, Stencil::Central) == 2
        && finiteDifferenceAccuracy(1, 1, Stencil::Forward) == 1 && finiteDifferenceAccuracy(1, 3, Stencil::Central) == 4);
  CHECK(close(finiteDifferenceStep(1, 4, Stencil::Central, 0.5), 1.0E-3) && close(finiteDifferenceStep(1, 4, Stencil::Central, -20.0), 2.0E-2));

  // ---------------------------------------------------------------- numerical derivatives of callables
  auto sine = [](double x) { return std::sin(x); };
  Differentiation automatic;
  CHECK(close(automatic.derivative(sine, 0.7), std::cos(0.7), 1.0E-10));
  CHECK(close(automatic.derivative(sine, 0.7, 2), -std::sin(0.7), 1.0E-6));
  CHECK(close(automatic.derivative(sine, 0.7, 3), -std::cos(0.7), 1.0E-4));
  // each method is exact for polynomials of its degree (up to rounding)
  auto quartic = [](double x) { return 2.0 - x + 3.0*x*x - 0.5*x*x*x + 0.25*x*x*x*x; };
  auto quartic1 = [](double x) { return -1.0 + 6.0*x - 1.5*x*x + x*x*x; };
  Differentiation linear(Differentiation::Linear), quadratic(Differentiation::Quadratic), cubicD(Differentiation::Cubic);
  Differentiation degree6(Differentiation::Polynomial, 6);
  CHECK(close(linear.derivative([](double x) { return 3.0*x - 1.0; }, 2.0), 3.0, 1.0E-9));
  CHECK(close(quadratic.derivative([](double x) { return x*x; }, 2.0), 4.0, 1.0E-9));
  CHECK(close(cubicD.derivative([](double x) { return x*x*x; }, 2.0), 12.0, 1.0E-8));
  CHECK(close(degree6.derivative(quartic, 1.3), quartic1(1.3), 1.0E-10));
  CHECK(degree6.degree() == 6 && cubicD.degree() == 3 && Differentiation::methodName(Differentiation::Cubic) == "Cubic");
  // errors fall as h^p
  auto errorWith = [&](Differentiation d, double step) { d.setStep(step); return std::fabs(d.derivative(sine, 0.7) - std::cos(0.7)); };
  Differentiation forward(Differentiation::Linear, 1, Stencil::Forward);
  double rf = errorWith(forward, 1.0E-3)/errorWith(forward, 0.5E-3);
  CHECK(rf > 1.8 && rf < 2.2);                                                    // h^1
  double rq = errorWith(quadratic, 1.0E-2)/errorWith(quadratic, 0.5E-2);
  CHECK(rq > 3.6 && rq < 4.4);                                                    // h^2
  double rc = errorWith(cubicD, 4.0E-2)/errorWith(cubicD, 2.0E-2);
  CHECK(rc > 13.0 && rc < 19.0);                                                  // h^4 (symmetric)
  Differentiation richardson(Differentiation::Quadratic);
  richardson.setRichardson();
  richardson.setStep(1.0E-2);
  CHECK(errorWith(richardson, 1.0E-2) < 0.05*errorWith(quadratic, 1.0E-2));
  Differentiation backward(Differentiation::Quadratic, 2, Stencil::Backward);
  CHECK(close(backward.derivative([](double x) { return x*x; }, 1.0), 2.0, 1.0E-8));
  // partial, mixed, gradient
  auto field = [](const std::vector<double> & x) { return std::sin(x[0])*std::exp(x[1]) + x[0]*x[1]*x[1]; };
  std::vector<double> at = {0.4, 0.3};
  CHECK(close(automatic.partialDerivative(field, at, 0), std::cos(0.4)*std::exp(0.3) + 0.09, 1.0E-9));
  CHECK(close(automatic.partialDerivative(field, at, 1, 2), std::sin(0.4)*std::exp(0.3) + 0.8, 1.0E-6));
  CHECK(close(automatic.mixedDerivative(field, at, {0, 1}), std::cos(0.4)*std::exp(0.3) + 0.6, 1.0E-6));
  VectorND grad = automatic.gradient(field, at);
  CHECK(grad.dimension() == 2 && close(grad[1], std::sin(0.4)*std::exp(0.3) + 0.24, 1.0E-9));
  CHECK(throws([&] { automatic.partialDerivative(field, at, 2); }));

  // ---------------------------------------------------------------- sampled values
  std::vector<double> xs = {0, 0.5, 1, 1.5, 2}, ys = {0, 0.25, 1, 2.25, 4};
  CHECK(sameVector(derivativesOfSamples(xs, ys), {0, 1, 2, 3, 4}, 1.0E-12));
  std::vector<double> xu = {0.0, 0.3, 1.0, 1.2, 2.5}, yu(5);
  for (int i=0; i<5; i++) yu[i] = xu[i]*xu[i]*xu[i];
  std::vector<double> du = cubicD.derivatives(xu, yu);
  bool exactCubic = true;
  for (int i=0; i<5; i++) exactCubic = exactCubic && close(du[i], 3.0*xu[i]*xu[i], 1.0E-10);
  CHECK(exactCubic);
  CHECK(sameVector(derivativesOfSamples(xs, ys, 2), {2, 2, 2, 2, 2}, 1.0E-10));
  // a grid of rank 2, f = x^2 + 3 x y, 4 x 3 nodes: d/dx = 2x + 3y exactly with parabolas
  std::vector<double> grid;
  for (unsigned int j=0; j<3; j++) for (unsigned int i=0; i<4; i++) grid.push_back(0.5*i*0.5*i + 3.0*0.5*i*j);
  std::vector<double> dx = derivativeOnGrid(grid, {4, 3}, 0, 0.5);
  bool exactGrid = true;
  for (unsigned int j=0; j<3; j++) for (unsigned int i=0; i<4; i++) exactGrid = exactGrid && close(dx[i + 4*j], 2.0*0.5*i + 3.0*j, 1.0E-12);
  CHECK(exactGrid);
  ScalarField sf("f", std::vector<double>{0.0, 0.0}, std::vector<double>{1.5, 2.0}, std::vector<unsigned int>{4, 3});
  for (unsigned int j=0; j<3; j++) for (unsigned int i=0; i<4; i++) sf(i, j) = grid[i + 4*j];
  ScalarField sfx = sf.derivativeX("dfdx");
  CHECK(close(sfx(2, 1), 2.0*1.0 + 3.0));

  // ---------------------------------------------------------------- analytical derivatives of the functions
  Polynomial p{1.0, -2.0, 0.0, 4.0};                                             // 1 - 2x + 4x^3
  std::unique_ptr<Function> dp(p.derivativeFunction());
  CHECK(dp && close(dp->evaluateAt(2.0), -2.0 + 48.0) && p.hasAnalyticalDerivative());
  Exponential e(2.0, -0.5);
  std::unique_ptr<Function> de(e.derivativeFunction());
  CHECK(de && close(de->evaluateAt(1.0), -std::exp(-0.5)));
  LegendreSeries ls(4);
  ChebyshevSeries cs(5);
  HermiteSeries hs(4);
  LaguerreSeries las(4, 0.5);
  std::vector<OrthogonalSeries *> series = {&ls, &cs, &hs, &las};
  for (OrthogonalSeries * s : series)
    {
    for (unsigned int k=0; k<s->nCoefficients(); k++) s->setParameterValue(k, 0.3 + 0.2*k - 0.05*k*k);
    s->adaptToRange(-1.0, 3.0);
    Differentiation numeric(Differentiation::Polynomial, 6);
    std::unique_ptr<Function> ds(s->derivativeFunction());
    bool ok = ds != nullptr;
    for (double x=-0.9; ok && x<2.9; x+=0.31)
      ok = close(ds->evaluateAt(x), numeric.derivative([&](double t) { return s->evaluateAt(t); }, x), 1.0E-8)
           && close(s->derivativeAt(x), ds->evaluateAt(x), 1.0E-12);
    check(ok, "series derivative", __LINE__);
    }
  FourierDecomposition fd(FourierDecomposition::kCosineSine, 1.0, {0.5, -0.2, 0.1}, {0.3, 0.0, -0.4});
  std::unique_ptr<Function> dfd(fd.derivativeFunction());
  CHECK(dfd && close(dfd->evaluateAt(0.8), fd.derivativeAt(0.8), 1.0E-12));
  FourierDecomposition flow(FourierDecomposition::kFlow, 2.0, {0.1, 0.05}, {0.3, 0.2});
  std::unique_ptr<Function> dflow(flow.derivativeFunction());
  CHECK(dflow && close(dflow->evaluateAt(1.1), flow.derivativeAt(1.1), 1.0E-12));
  Polynomial2D p2(3);
  for (unsigned int k=0; k<p2.nCoefficients(); k++) p2.setParameterValue(k, 0.1*k - 0.3);
  p2.adaptToRange({0.0, -1.0}, {2.0, 3.0});
  std::unique_ptr<Function> p2x(p2.derivativeFunction(0)), p2y(p2.derivativeFunction(1));
  Differentiation numeric6(Differentiation::Polynomial, 6);
  auto p2call = [&](const std::vector<double> & x) { return p2.evaluateAt(x); };
  CHECK(p2x && close(p2x->evaluateAt(0.7, 0.4), numeric6.partialDerivative(p2call, {0.7, 0.4}, 0), 1.0E-8));
  CHECK(p2y && close(p2y->evaluateAt(0.7, 0.4), numeric6.partialDerivative(p2call, {0.7, 0.4}, 1), 1.0E-8));
  CHECK(close(p2.gradientAt(0.7, 0.4)[1], p2y->evaluateAt(0.7, 0.4), 1.0E-12) && !p2.derivativeFunction(2));
  Normal normal(3.0, 0.5, 1.5);
  CHECK(normal.hasAnalyticalDerivative() && !normal.derivativeFunction());

  // ---------------------------------------------------------------- Differentiation of functions
  CHECK(close(automatic.derivative(p, 2.0), 46.0, 1.0E-14));                     // analytical
  CHECK(close(automatic.derivative(p, 2.0, 3), 24.0, 1.0E-14));
  CHECK(close(automatic.derivative(p, 2.0, 5), 0.0, 1.0E-14));
  Differentiation analytical(Differentiation::Analytical);
  CHECK(close(analytical.derivative(normal, 1.0), normal.derivativeAt(1.0), 1.0E-15));
  CHECK(throws([&] { analytical.derivative(normal, 1.0, 2); }));
  double z = (1.0 - 0.5)/1.5;
  CHECK(close(automatic.derivative(normal, 1.0, 2), 3.0*(z*z - 1.0)/(1.5*1.5)*std::exp(-0.5*z*z), 1.0E-7));   // analytical, then numerical
  Plain plain;
  CHECK(close(automatic.partialDerivative(plain, {0.4, 0.3}, 0), std::cos(0.4)*std::exp(0.3), 1.0E-9));
  CHECK(close(automatic.mixedDerivative(plain, {0.4, 0.3}, {0, 1}), std::cos(0.4)*std::exp(0.3), 1.0E-6));
  CHECK(close(automatic.gradient(plain, {0.4, 0.3})[1], std::sin(0.4)*std::exp(0.3), 1.0E-9));
  Bessel j0(Bessel::BesselJ, 0);
  CHECK(j0.hasAnalyticalDerivative() && close(automatic.derivative(j0, 1.5), j0.derivativeAt(1.5), 1.0E-15));

  // ---------------------------------------------------------------- Derivative
  Derivative d1(p);
  CHECK(d1.isAnalytical() && close(d1(2.0), 46.0) && d1.order() == 1 && d1.rank() == 1);
  Derivative d3(p, 3);
  CHECK(d3.isAnalytical() && close(d3(-1.0), 24.0));
  Derivative dn(normal);
  CHECK(dn.isAnalytical() && close(dn(1.0), normal.derivativeAt(1.0), 1.0E-15));
  Derivative dn2(normal, 2);
  CHECK(!dn2.isAnalytical() && close(dn2(1.0), 3.0*(z*z - 1.0)/(1.5*1.5)*std::exp(-0.5*z*z), 1.0E-7));
  Derivative cubicDerivative(normal, 1, 0, Differentiation(Differentiation::Cubic));
  CHECK(!cubicDerivative.isAnalytical() && close(cubicDerivative(1.0), normal.derivativeAt(1.0), 1.0E-9));
  CHECK(throws([&] { Derivative bad(normal, 2, 0, Differentiation(Differentiation::Analytical)); }));
  CHECK(throws([&] { Derivative bad(p, 1, 1); }));
  Derivative mixed(plain, std::vector<unsigned int>{0, 1});
  CHECK(close(mixed.evaluateAt(0.4, 0.3), std::cos(0.4)*std::exp(0.3), 1.0E-6) && mixed.rank() == 2);
  Derivative partial(p2, 1, 1);
  CHECK(partial.isAnalytical() && close(partial.evaluateAt(0.7, 0.4), p2y->evaluateAt(0.7, 0.4), 1.0E-12));
  // a derivative of a derivative, and the derivative follows changes of f
  std::unique_ptr<Function> second(d1.derivativeFunction());
  CHECK(second && close(second->evaluateAt(2.0), 48.0));
  Polynomial q{0.0, 1.0};
  Derivative dq(q);
  q.setCoefficient(1, 5.0);
  CHECK(close(dq(3.0), 1.0));                                                    // made once
  dq.update();
  CHECK(close(dq(3.0), 5.0));                                                    // made again
  Plain1 plain1;
  Derivative numericalDerivative(plain1);
  CHECK(!numericalDerivative.isAnalytical() && close(numericalDerivative(1.2), 3*1.44*std::sin(1.2) + 1.728*std::cos(1.2), 1.0E-9));
  std::shared_ptr<const Function> owned(new Exponential(1.0, 2.0));
  Derivative shared(owned, 2);
  owned.reset();
  CHECK(shared.isAnalytical() && close(shared(0.5), 4.0*std::exp(1.0)));
  CubicSpline spline({0.0, 1.0, 2.0, 3.0}, {0.0, 1.0, 8.0, 27.0});
  Derivative ds2(spline, 2);
  CHECK(ds2.isAnalytical() && close(ds2(1.7), 6.0*1.7, 1.0E-12));

  // ---------------------------------------------------------------- numerical defaults of Function
  CHECK(close(plain1.derivativeAt(1.2), 3*1.44*std::sin(1.2) + 1.728*std::cos(1.2), 1.0E-9));
  CHECK(close(plain.gradientAt(0.4, 0.3)[0], std::cos(0.4)*std::exp(0.3), 1.0E-9));

  std::printf("TestDerivative: %d checks, %d failures\n", nChecks, nFailures);
  return nFailures;
}
