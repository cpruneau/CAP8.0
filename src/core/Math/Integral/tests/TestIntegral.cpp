/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestIntegral.cpp
//! @brief Test program of the integrators and of the integration of sampled values (run by ctest).
//
// TestIntegral -- each integrator (trapezoid, Simpson, Romberg, Gauss-Legendre, Monte Carlo, Integration) on known
// integrals of one variable, reversed and equal bounds, functions of rank 1, 2 and n, boxes of 2 to 6 variables, error
// estimates and convergence orders, exactness for polynomials (Simpson: cubics, Gauss-Legendre with m points: degree
// 2m-1, Romberg), the Gauss-Legendre nodes and weights, Monte Carlo errors that contain the exact value, the method
// chosen by Integration; the rules for sampled values (trapezoid, Simpson, cubic; uneven spacing; 2D and nD grids;
// cumulative); the numerical defaults of Function. Prints each failed check and exits with the number of failures.
//
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include "TrapezoidIntegrator.hpp"
#include "SimpsonIntegrator.hpp"
#include "RombergIntegrator.hpp"
#include "GaussLegendreIntegrator.hpp"
#include "MonteCarloIntegrator.hpp"
#include "Integration.hpp"
#include "MathBasicIntegration.hpp"
#include "Normal.hpp"
#include "Polynomial.hpp"
#include "Normal2D.hpp"

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

// a plain Function of any rank: exp(-|x|^2)
class Gaussian : public Function
{
public:
  explicit Gaussian(unsigned int rank) : Function(rank) {}
  double evaluateAt(const std::vector<double> & x) const override
  { double s = 0; for (double v : x) s += v*v; return std::exp(-s); }
  using Function::evaluateAt;
};
} // namespace

int main()
{
  const double pi = M_PI;
  Integrand1D sine = [](double x) { return std::sin(x); };
  Integrand1D bump = [](double x) { return std::exp(-x*x); };

  // ---------------------------------------------------------------- one variable, each integrator
  TrapezoidIntegrator trapezoid(1.0E-8, 24);
  SimpsonIntegrator simpson;
  RombergIntegrator romberg;
  GaussLegendreIntegrator gauss;
  MonteCarloIntegrator mc(2.0E-3);
  std::vector<const Integrator *> all = {&trapezoid, &simpson, &romberg, &gauss};
  for (const Integrator * integrator : all)
    {
    IntegralResult r = integrator->integrate(sine, 0.0, pi);
    check(close(r.value, 2.0, 1.0E-7) && r.converged && r.nEvaluations > 0, integrator->name().str() + " sin", __LINE__);
    check(close(integrator->integral(bump, -3.0, 3.0), std::sqrt(pi)*std::erf(3.0), 1.0E-7), integrator->name().str() + " gauss", __LINE__);
    check(close(integrator->integral(sine, pi, 0.0), -2.0, 1.0E-7), integrator->name().str() + " reversed", __LINE__);
    check(integrator->integral(sine, 1.0, 1.0) == 0.0, integrator->name().str() + " equal", __LINE__);
    check(throws([&] { integrator->integral(sine, 0.0, INFINITY); }), integrator->name().str() + " infinite", __LINE__);
    check(r.error < 1.0E-6, integrator->name().str() + " error", __LINE__);
    }
  IntegralResult m = mc.integrate(sine, 0.0, pi);
  CHECK(m.converged && std::fabs(m.value - 2.0) < 5.0*m.error && m.error < 5.0E-3);
  // exactness: Simpson for cubics on any panels, Gauss-Legendre with m points up to degree 2m-1, Romberg on polynomials
  Integrand1D cubic = [](double x) { return 1.0 - x + 2.0*x*x*x; };
  SimpsonIntegrator fixed;
  fixed.setPanels(1);
  CHECK(close(fixed.integral(cubic, -1.0, 2.0), 3.0 - 1.5 + 7.5));
  GaussLegendreIntegrator three(3);
  three.setPanels(1);
  Integrand1D quintic = [](double x) { return std::pow(x, 5) - x*x; };
  CHECK(close(three.integral(quintic, 0.0, 2.0), 64.0/6.0 - 8.0/3.0));
  CHECK(close(GaussLegendreIntegrator::quadrature(quintic, 0.0, 2.0, 3), 64.0/6.0 - 8.0/3.0));
  CHECK(!close(GaussLegendreIntegrator::quadrature([](double x) { return std::pow(x, 6); }, 0.0, 1.0, 3), 1.0/7.0, 1.0E-6));
  CHECK(close(romberg.integral([](double x) { return std::pow(x, 7); }, 0.0, 1.0), 0.125, 1.0E-13));
  std::vector<double> nodes, weights;
  GaussLegendreIntegrator::rule(4, nodes, weights);
  CHECK(nodes.size() == 4 && close(nodes[3], std::sqrt(3.0/7.0 + 2.0/7.0*std::sqrt(1.2))) && close(nodes[0], -nodes[3]));
  CHECK(close(weights[0] + weights[1] + weights[2] + weights[3], 2.0) && close(weights[3], (18.0 - std::sqrt(30.0))/36.0));
  CHECK(throws([] { std::vector<double> a, b; GaussLegendreIntegrator::rule(0, a, b); }));
  // convergence orders: trapezoid h^2, Simpson h^4 (fixed panels)
  TrapezoidIntegrator t8, t16;
  t8.setPanels(8); t16.setPanels(16);
  double e8 = std::fabs(t8.integral(bump, 0.0, 2.0) - std::sqrt(pi)/2*std::erf(2.0));
  double e16 = std::fabs(t16.integral(bump, 0.0, 2.0) - std::sqrt(pi)/2*std::erf(2.0));
  CHECK(e8/e16 > 3.5 && e8/e16 < 4.5);
  SimpsonIntegrator s4, s8;
  s4.setPanels(4); s8.setPanels(8);
  double f4 = std::fabs(s4.integral(bump, 0.0, 2.0) - std::sqrt(pi)/2*std::erf(2.0));
  double f8 = std::fabs(s8.integral(bump, 0.0, 2.0) - std::sqrt(pi)/2*std::erf(2.0));
  CHECK(f4/f8 > 12.0 && f4/f8 < 20.0);
  // the accuracy requested is reached; a tight request costs more evaluations
  SimpsonIntegrator loose(1.0E-4), tight(1.0E-12);
  IntegralResult rl = loose.integrate(bump, -2.0, 2.0), rt = tight.integrate(bump, -2.0, 2.0);
  CHECK(rt.nEvaluations > rl.nEvaluations && close(rt.value, std::sqrt(pi)*std::erf(2.0), 1.0E-12));
  TrapezoidIntegrator limited(1.0E-15, 3);
  CHECK(!limited.integrate(bump, -2.0, 2.0).converged);

  // ---------------------------------------------------------------- functions of rank 1, 2 and n
  Normal normal(1.0, 0.5, 2.0);
  double exact = std::sqrt(2.0*pi)*2.0*0.5*(std::erf((3.0 - 0.5)/(2.0*std::sqrt(2.0))) - std::erf((-1.0 - 0.5)/(2.0*std::sqrt(2.0))));
  for (const Integrator * integrator : all)
    check(close(integrator->integral(normal, -1.0, 3.0), exact, 1.0E-7), integrator->name().str() + " Normal", __LINE__);
  CHECK(throws([&] { simpson.integral(normal, std::vector<double>{0.0, 0.0}, std::vector<double>{1.0, 1.0}); }));
  Gaussian g2(2), g3(3), g5(5);
  double one = std::sqrt(pi)/2.0*std::erf(1.0);                         // integral of exp(-x^2) over [0, 1]
  std::vector<const Integrator *> boxes = {&trapezoid, &simpson, &romberg, &gauss};
  TrapezoidIntegrator trapezoidBox(1.0E-6);
  boxes[0] = &trapezoidBox;
  for (const Integrator * integrator : boxes)
    {
    check(close(integrator->integral(g2, 0.0, 1.0, 0.0, 1.0), one*one, 1.0E-6), integrator->name().str() + " 2D", __LINE__);
    check(close(integrator->integral(g3, std::vector<double>(3, 0.0), std::vector<double>(3, 1.0)), one*one*one, 1.0E-5),
          integrator->name().str() + " 3D", __LINE__);
    }
  MonteCarloIntegrator mc5(3.0E-3);
  mc5.setSeed(2026);
  IntegralResult r5 = mc5.integrate(g5, std::vector<double>(5, 0.0), std::vector<double>(5, 1.0));
  CHECK(r5.converged && std::fabs(r5.value - std::pow(one, 5)) < 5.0*r5.error && r5.error < 4.0E-3*std::pow(one, 5));
  // the volume of the unit ball in 6 dimensions, pi^3/6: an integrand with a step
  MonteCarloIntegrator ball(1.0E-2);
  ball.setSeed(7);
  IntegralResult rb = ball.integrate([](const std::vector<double> & x) { double s = 0; for (double v : x) s += v*v; return s < 1.0 ? 1.0 : 0.0; },
                                     std::vector<double>(6, -1.0), std::vector<double>(6, 1.0));
  CHECK(std::fabs(rb.value - pi*pi*pi/6.0) < 5.0*rb.error);
  MonteCarloIntegrator fixedSamples;
  fixedSamples.setSamples(1000);
  CHECK(fixedSamples.integrate(sine, 0.0, pi).nEvaluations == 1000);
  MonteCarloIntegrator same1(1.0E-3, 100000000, 11), same2(1.0E-3, 100000000, 11);
  CHECK(same1.integral(bump, 0.0, 1.0) == same2.integral(bump, 0.0, 1.0));   // reproducible
  Normal2D n2;
  double n2Analytical = n2.integralInRange(-1.0, 1.0, -0.5, 2.0);
  CHECK(close(gauss.integral(n2, -1.0, 1.0, -0.5, 2.0), n2Analytical, 1.0E-8));

  // ---------------------------------------------------------------- Integration (steering)
  Integration automatic;
  CHECK(automatic.methodFor(1) == Integration::Simpson && automatic.methodFor(3) == Integration::Simpson
        && automatic.methodFor(4) == Integration::MonteCarlo);
  CHECK(close(automatic.integral(sine, 0.0, pi), 2.0, 1.0E-8) && automatic.name() == "Automatic");
  Integration chosen(Integration::Romberg, 1.0E-12);
  CHECK(close(chosen.integral(normal, -1.0, 3.0), exact, 1.0E-11) && chosen.name() == "Romberg");
  chosen.setMethod(Integration::GaussLegendre);
  dynamic_cast<GaussLegendreIntegrator &>(chosen.integrator(Integration::GaussLegendre)).setPoints(8);
  CHECK(close(chosen.integral(g2, 0.0, 1.0, 0.0, 1.0), one*one, 1.0E-10));
  chosen.setMethod(Integration::MonteCarlo);
  chosen.setRelativeTolerance(5.0E-3);
  IntegralResult rc = chosen.integrate(g5, std::vector<double>(5, 0.0), std::vector<double>(5, 1.0));
  CHECK(std::fabs(rc.value - std::pow(one, 5)) < 5.0*rc.error);
  std::unique_ptr<Integrator> copy(chosen.clone());
  CHECK(copy->name() == "MonteCarlo");
  CHECK(throws([&] { chosen.integrator(Integration::Automatic); }));
  CHECK(Integration::methodName(Integration::Trapezoid) == "Trapezoid");

  // ---------------------------------------------------------------- sampled values
  std::vector<double> squares = {0, 1, 4, 9, 16};
  CHECK(close(integrationTrapezoidRule(squares, 1.0), 22.0) && close(integrationSimpsonRule(squares, 1.0), 64.0/3.0));
  double raw[5] = {0, 1, 4, 9, 16};
  CHECK(close(integrationSimpsonRule(raw, 5, 1.0), 64.0/3.0));
  std::vector<double> c = cumulativeTrapezoidRule(squares, 1.0);
  CHECK(c.size() == 5 && close(c[1], 0.5) && close(c[4], 22.0));
  std::vector<double> cs = cumulativeSimpsonRule(squares, 1.0);
  CHECK(cs.size() == 3 && close(cs[1], 8.0/3.0) && close(cs[2], 64.0/3.0));
  CHECK(throws([&] { cumulativeSimpsonRule(std::vector<double>{1, 2, 3, 4}, 1.0); }));
  std::vector<double> xu = {0.0, 0.4, 1.0, 1.3, 2.0, 2.2}, yu(xu.size()), yc(xu.size());   // uneven spacing
  for (std::size_t i=0; i<xu.size(); i++) { yu[i] = xu[i]*xu[i]; yc[i] = xu[i]*xu[i]*xu[i] - xu[i]; }
  CHECK(close(integrationSimpsonRule(xu, yu), 2.2*2.2*2.2/3.0));       // exact for parabolas, odd last interval too
  CHECK(close(integrationCubicRule(xu, yc), std::pow(2.2, 4)/4.0 - 2.2*2.2/2.0));   // exact for cubics
  CHECK(close(integrationCubicRule(squares, 1.0), 64.0/3.0));
  CHECK(close(integrationTrapezoidRule(xu, std::vector<double>(xu.size(), 3.0)), 6.6));
  // grids: f = x y^2 over [0, 2] x [0, 1], 5 x 3 nodes; Simpson exact
  std::vector<double> grid;
  for (unsigned int j=0; j<3; j++) for (unsigned int i=0; i<5; i++) grid.push_back(0.5*i*(0.5*j)*(0.5*j));
  CHECK(close(integrationSimpsonRule2D(grid, 5, 3, 0.5, 0.5), 2.0/3.0));
  CHECK(close(integrationTrapezoidRule2D(std::vector<double>(15, 2.0), 5, 3, 0.5, 0.5), 4.0));
  std::vector<double> g3d(27, 1.0);
  CHECK(close(integrationSimpsonRuleND(g3d, {3, 3, 3}, {0.5, 1.0, 2.0}), 8.0) && close(integrationTrapezoidRuleND(g3d, {3, 3, 3}, {0.5, 1.0, 2.0}), 8.0));
  CHECK(throws([&] { integrationSimpsonRuleND(g3d, {3, 3}, {1.0, 1.0}); }));
  CHECK(throws([&] { integrationSimpsonRuleND(std::vector<double>(12, 1.0), {3, 4}, {1.0, 1.0}); }));

  // ---------------------------------------------------------------- the numerical defaults of Function
  CHECK(close(g2.integralInRange(0.0, 1.0, 0.0, 1.0), one*one, 1.0E-7));
  Polynomial p{1.0, 2.0};
  CHECK(close(p.expectationValue(0.0, 1.0), (0.5 + 2.0/3.0)/2.0, 1.0E-12));

  std::printf("TestIntegral: %d checks, %d failures\n", nChecks, nFailures);
  return nFailures;
}
