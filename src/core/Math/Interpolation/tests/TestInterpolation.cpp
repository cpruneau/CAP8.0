/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestInterpolation.cpp
//! @brief Test program of the interpolation tools (run by ctest).
//
// TestInterpolation -- Neville and local polynomials exact for polynomials of their degree; intervals and windows;
// inverse linear interpolation; multilinear interpolation exact for multilinear functions; piecewise polynomials
// (values, derivatives, integrals, derivative functions); cubic splines: not-a-knot exact for cubics at any spacing,
// natural with zero second derivatives at the ends, clamped with the slopes given, monotone without overshoot,
// convergence as h^4; Interpolation with each method. Prints each failed check and exits with the number of failures.
//
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include "InterpolationTools.hpp"
#include "PiecewisePolynomial.hpp"
#include "CubicSpline.hpp"
#include "Interpolation.hpp"

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

double cubic(double x) { return 1.0 - 2.0*x + 0.5*x*x + 0.25*x*x*x; }
double cubic1(double x) { return -2.0 + x + 0.75*x*x; }
double cubic2(double x) { return 1.0 + 1.5*x; }
} // namespace

int main()
{
  // ---------------------------------------------------------------- Neville, local polynomials
  std::vector<double> x = {0.0, 0.3, 1.0, 1.7, 2.1, 3.0, 4.4};     // uneven spacing
  std::vector<double> y(x.size());
  for (std::size_t i=0; i<x.size(); i++) y[i] = cubic(x[i]);
  double error = -1.0;
  CHECK(close(interpolatePolynomial(x, y, 2.5, &error), cubic(2.5)));
  CHECK(error >= 0.0 && error < 1.0E-8);
  CHECK(close(interpolatePolynomial(x.data(), y.data(), 4, 0.7), cubic(0.7)));      // 4 points: a cubic, exact
  CHECK(close(interpolateLocalPolynomial(x, y, 3.7, 3), cubic(3.7)));
  CHECK(close(interpolateLocalPolynomial(x, y, -0.5, 3), cubic(-0.5)));              // extrapolated
  CHECK(!close(interpolateLocalPolynomial(x, y, 2.5, 1), cubic(2.5), 1.0E-3));       // linear: not exact
  CHECK(throws([] { std::vector<double> a = {1.0, 1.0}, b = {0.0, 1.0}; interpolatePolynomial(a, b, 0.5); }));
  CHECK(findInterval(x, -1.0) == 0 && findInterval(x, 0.5) == 1 && findInterval(x, 1.0) == 2 && findInterval(x, 9.0) == 5);
  CHECK(localWindow(x, 0.0, 2) == 0 && localWindow(x, 4.4, 2) == 4 && localWindow(x, 1.6, 2) == 2);
  // inverse of a monotonic table
  std::vector<double> t = {0.0, 1.0, 2.0, 3.0}, c = {0.0, 0.1, 0.5, 1.0};
  CHECK(close(inverseLinearInterpolation(t, c, 0.3), 1.5));
  CHECK(close(inverseLinearInterpolation(t, c, 1.0), 3.0) && close(inverseLinearInterpolation(t, c, -1.0), 0.0));
  // multilinear on a grid of rank 3: f = 1 + x + 2y - z + x y z is multilinear, hence exact
  unsigned int n[3] = {3, 4, 2};
  double minima[3] = {0.0, -1.0, 2.0}, spacings[3] = {0.5, 1.0, 0.25};
  std::vector<double> grid;
  for (unsigned int k=0; k<n[2]; k++)
    for (unsigned int j=0; j<n[1]; j++)
      for (unsigned int i=0; i<n[0]; i++)
        {
        double gx = minima[0] + i*spacings[0], gy = minima[1] + j*spacings[1], gz = minima[2] + k*spacings[2];
        grid.push_back(1.0 + gx + 2.0*gy - gz + gx*gy*gz);
        }
  double p[3] = {0.7, 0.3, 2.1};
  CHECK(close(interpolateMultilinear(grid.data(), 3, n, minima, spacings, p), 1.0 + 0.7 + 0.6 - 2.1 + 0.7*0.3*2.1));
  double q[3] = {1.5, 2.5, 2.4};                                                    // beyond the edges: extrapolated
  CHECK(close(interpolateMultilinear(grid.data(), 3, n, minima, spacings, q), 1.0 + 1.5 + 5.0 - 2.4 + 1.5*2.5*2.4));
  double bad[3] = {NAN, 0.0, 2.0};
  CHECK(throws([&] { interpolateMultilinear(grid.data(), 3, n, minima, spacings, bad); }));

  // ---------------------------------------------------------------- piecewise polynomials
  PiecewisePolynomial tent({0.0, 1.0, 2.0}, {{0.0, 1.0}, {1.0, -1.0}});
  CHECK(close(tent(0.5), 0.5) && close(tent(1.5), 0.5) && close(tent(1.0), 1.0));
  CHECK(close(tent.integralInRange(0.0, 2.0), 1.0) && close(tent.integralInRange(2.0, 0.5), -0.875));
  CHECK(close(tent.derivativeAt(0.5), 1.0) && close(tent.derivativeAt(1.5), -1.0));
  CHECK(tent.hasAnalyticalDerivative());
  std::unique_ptr<Function> slope(tent.derivativeFunction());
  CHECK(slope && close(slope->evaluateAt(0.2), 1.0) && close(slope->evaluateAt(1.8), -1.0));
  CHECK(throws([] { PiecewisePolynomial bad({0.0, 0.0}, {{1.0}}); }));
  CHECK(throws([] { PiecewisePolynomial bad({0.0, 1.0}, {{1.0}, {2.0}}); }));

  // ---------------------------------------------------------------- cubic splines
  CubicSpline notAKnot(x, y);                                     // exact for a cubic, at any spacing
  bool exact = true;
  for (double v=-0.5; v<=4.9; v+=0.137)
    exact = exact && close(notAKnot(v), cubic(v), 1.0E-9) && close(notAKnot.derivativeAt(v), cubic1(v), 1.0E-8)
                  && close(notAKnot.derivativeAt(v, 2), cubic2(v), 1.0E-7);
  CHECK(exact);
  CHECK(close(notAKnot.integralInRange(0.0, 4.4), 4.4 - 4.4*4.4 + 4.4*4.4*4.4/6.0 + std::pow(4.4, 4)/16.0, 1.0E-10));
  std::vector<double> x3 = {0.0, 1.0, 3.0}, y3 = {1.0, 2.0, 10.0};             // 3 points: the parabola 1 + x^2... no:
  CubicSpline parabola(x3, y3);                                   // the parabola through them
  CHECK(close(parabola(2.0), interpolatePolynomial(x3, y3, 2.0)));
  CubicSpline natural(x, y, CubicSpline::Natural);
  CHECK(close(natural.derivativeAt(0.0, 2), 0.0, 1.0E-9) && close(natural.derivativeAt(4.4, 2), 0.0, 1.0E-9));
  CHECK(close(natural(1.7), cubic(1.7)));                         // through the points
  CubicSpline clamped(x, y, CubicSpline::Clamped, cubic1(0.0), cubic1(4.4));
  CHECK(close(clamped.derivativeAt(0.0), cubic1(0.0)) && close(clamped(3.3), cubic(3.3), 1.0E-9));   // a cubic: exact
  // monotone: a step stays within the data, a monotonic table gives a monotonic function
  std::vector<double> xs = {0, 1, 2, 3, 4, 5}, ys = {0, 0, 0, 1, 1, 1};
  CubicSpline monotone(xs, ys, CubicSpline::Monotone), overshoot(xs, ys, CubicSpline::Natural);
  bool inside = true, increasing = true;
  double previous = -1.0;
  for (double v=0.0; v<=5.0; v+=0.01) { double m = monotone(v); inside = inside && m >= -1e-12 && m <= 1.0 + 1e-12;
                                        increasing = increasing && m >= previous - 1e-12; previous = m; }
  CHECK(inside && increasing);
  bool over = false;
  for (double v=0.0; v<=5.0; v+=0.01) over = over || overshoot(v) > 1.0 + 1e-6 || overshoot(v) < -1e-6;
  CHECK(over);                                                    // the natural spline overshoots
  // convergence on sin: error falls as h^4
  auto splineError = [](int nPoints)
    {
    std::vector<double> u(nPoints), w(nPoints);
    for (int i=0; i<nPoints; i++) { u[i] = 3.0*i/(nPoints - 1); w[i] = std::sin(u[i]); }
    CubicSpline s(u, w);
    double worst = 0.0;
    for (double v=0.0; v<=3.0; v+=0.001) worst = std::max(worst, std::fabs(s(v) - std::sin(v)));
    return worst;
    };
  double ratio = splineError(21)/splineError(41);
  CHECK(ratio > 12.0 && ratio < 26.0);                            // 2^4 = 16 asymptotically (from above)
  CHECK(throws([] { CubicSpline s({0.0, 2.0, 1.0}, {0.0, 1.0, 2.0}); }));

  // ---------------------------------------------------------------- Interpolation
  std::vector<double> xl = {0.0, 1.0, 2.0, 4.0}, yl = {1.0, 3.0, 2.0, 6.0};
  Interpolation linear(xl, yl, Interpolation::Linear);
  CHECK(close(linear(0.5), 2.0) && close(linear(3.0), 4.0) && close(linear(5.0), 8.0));
  CHECK(close(linear.integralInRange(0.0, 4.0), 2.0 + 2.5 + 8.0) && close(linear.derivativeAt(3.0), 2.0));
  CHECK(linear.hasAnalyticalDerivative() && linear.piecewise());
  Interpolation nearest(xl, yl, Interpolation::Nearest);
  CHECK(close(nearest(0.4), 1.0) && close(nearest(2.9), 2.0) && close(nearest(3.1), 6.0) && !nearest.hasAnalyticalDerivative());
  Interpolation local(x, y, Interpolation::LocalPolynomial, 3), global(x, y, Interpolation::Polynomial);
  CHECK(close(local(2.5), cubic(2.5)) && close(global(2.5), cubic(2.5)));
  CHECK(close(local.integralInRange(0.0, 4.4), notAKnot.integralInRange(0.0, 4.4), 1.0E-8));   // numerical
  Interpolation spline(x, y);
  CHECK(spline.method() == Interpolation::Spline && close(spline(2.5), cubic(2.5)));
  std::unique_ptr<Function> ds(spline.derivativeFunction());
  CHECK(ds && close(ds->evaluateAt(2.5), cubic1(2.5), 1.0E-8));
  CHECK(close(Interpolation::value(xl, yl, 0.5), 2.0) && close(Interpolation::value(x, y, 2.5, Interpolation::Spline), cubic(2.5)));
  Interpolation changed(xl, yl);
  changed.setMethod(Interpolation::MonotoneSpline);
  CHECK(changed.method() == Interpolation::MonotoneSpline && close(changed(1.0), 3.0));
  CHECK(Interpolation::methodName(Interpolation::ClampedSpline) == "ClampedSpline");
  std::vector<double> v = linear.values({0.5, 3.0});
  CHECK(v.size() == 2 && close(v[1], 4.0));

  std::printf("TestInterpolation: %d checks, %d failures\n", nChecks, nFailures);
  return nFailures;
}
