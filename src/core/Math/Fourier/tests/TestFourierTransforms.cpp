/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestFourierTransforms.cpp
//! @brief Tests of the Fourier transforms of the functions: each analytical transform against a numerical one
//! (composite Gauss-Legendre quadrature of f(x) e^{-ikx}), the special functions, the approximations, the series.
#include <cmath>
#include <complex>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "Bessel.hpp"
#include "BesselFunction.hpp"
#include "BezierCurve.hpp"
#include "CompositeFunction.hpp"
#include "ContinuousDistributions.hpp"
#include "DiscreteDistributions.hpp"
#include "Exponential.hpp"
#include "FourierDecomposition.hpp"
#include "FourierTools.hpp"
#include "FourierTransform.hpp"
#include "GaussLegendreIntegrator.hpp"
#include "Legendre.hpp"
#include "LogNormal.hpp"
#include "Normal.hpp"
#include "Normal2D.hpp"
#include "OrthogonalSeries.hpp"
#include "ParameterizedComposite.hpp"
#include "Polynomial.hpp"
#include "SignalFunctions.hpp"

using namespace Datum::MATH;
using Complex = std::complex<double>;

namespace
{
int failures = 0;
int checks = 0;
const double kPi = 3.14159265358979323846;

void expect(bool condition, const std::string & what)
{
  checks++;
  if (!condition) { failures++; std::cout << "FAILED: " << what << std::endl; }
}

//! |a - b| <= tolerance (scale + |b|)
bool close(Complex a, Complex b, double tolerance, double scale=1.0)
{
  return std::abs(a - b) <= tolerance*(scale + std::abs(b));
}

std::string text(Complex z)
{
  return "(" + std::to_string(z.real()) + ", " + std::to_string(z.imag()) + ")";
}

//! Numerical transform on [a, b]: 20-point Gauss-Legendre panels, at most one oscillation per panel.
Complex numerical(const std::function<double(double)> & f, double k, double a, double b, int minimumPanels=400)
{
  std::vector<double> nodes, weights;
  GaussLegendreIntegrator::rule(20, nodes, weights);
  int panels = std::max(minimumPanels, int(std::ceil(std::fabs(k)*(b - a)/(2.0*kPi))) + 1);
  double h = (b - a)/panels;
  Complex sum = 0.0;
  for (int p=0; p<panels; p++)
    {
    double m = a + (p + 0.5)*h;
    for (std::size_t q=0; q<nodes.size(); q++)
      {
      double x = m + 0.5*h*nodes[q];
      sum += weights[q]*f(x)*std::exp(Complex(0.0, -k*x));
      }
    }
  return 0.5*h*sum;
}

void compareLine(const Function & f, double a, double b, const std::vector<double> & ks, double tolerance,
                 const std::string & name, int minimumPanels=400)
{
  for (double k : ks)
    {
    Complex exact = f.fourierTransformAt(k);
    Complex reference = numerical([&f](double x) { return f.evaluateAt(x); }, k, a, b, minimumPanels);
    expect(close(exact, reference, tolerance), name + " at k = " + std::to_string(k) + ": " + text(exact) + " vs " + text(reference));
    }
}

void compareDomain(const Function & f, double a, double b, const std::vector<double> & ks, double tolerance,
                   const std::string & name)
{
  for (double k : ks)
    {
    Complex exact = f.fourierTransformAt(k, a, b);
    Complex reference = numerical([&f](double x) { return f.evaluateAt(x); }, k, a, b);
    expect(close(exact, reference, tolerance), name + " on [" + std::to_string(a) + ", " + std::to_string(b) + "] at k = "
           + std::to_string(k) + ": " + text(exact) + " vs " + text(reference));
    }
}

void testSpecialFunctions()
{
  expect(close(FourierTools::faddeeva(0.0), 1.0, 1e-13), "w(0) = 1");
  expect(close(FourierTools::faddeeva(1.0), Complex(0.36787944117144233, 0.60715770584139372), 1e-12), "w(1)");
  expect(close(FourierTools::faddeeva(Complex(0.0, 1.0)), 0.42758357615580700, 1e-12), "w(i) = e erfc(1)");
  // w(z) = (i/pi) integral e^{-t^2}/(z - t) dt, Im z > 0
  for (Complex z : { Complex(2.0, 0.5), Complex(-1.5, 2.0), Complex(6.0, 0.1), Complex(0.3, 0.05) })
    {
    std::vector<double> nodes, weights;
    GaussLegendreIntegrator::rule(20, nodes, weights);
    Complex sum = 0.0;
    int panels = 40000;
    double a = -12.0, b = 12.0, h = (b - a)/panels;
    for (int p=0; p<panels; p++)
      for (std::size_t q=0; q<nodes.size(); q++)
        {
        double t = a + (p + 0.5)*h + 0.5*h*nodes[q];
        sum += weights[q]*std::exp(-t*t)/(z - t);
        }
    Complex reference = Complex(0.0, 1.0/kPi)*0.5*h*sum;
    expect(close(FourierTools::faddeeva(z), reference, 1e-10), "w(" + text(z) + ") " + text(FourierTools::faddeeva(z)) + " vs " + text(reference));
    }
  // spherical Bessel functions: sum (2n + 1) j_n^2 = 1, and j_2
  for (double x : { 0.5, 5.0, 50.0, 300.0 })
    {
    std::vector<double> j;
    unsigned int n = (unsigned int)(x) + 80;
    FourierTools::sphericalBesselJ(n, x, j);
    double sum = 0.0;
    for (unsigned int m=0; m<=n; m++) sum += (2.0*m + 1.0)*j[m]*j[m];
    expect(std::fabs(sum - 1.0) < 1e-12, "sum (2n+1) j_n(" + std::to_string(x) + ")^2 = " + std::to_string(sum));
    double j2 = (3.0/(x*x) - 1.0)*std::sin(x)/x - 3.0*std::cos(x)/(x*x);
    expect(std::fabs(j[2] - j2) < 1e-13, "j_2(" + std::to_string(x) + ")");
    }
  for (double z : { 0.01, 0.7, 3.0, 25.0 })
    {
    expect(std::fabs(FourierTools::besselKNormalized(0.5, z) - std::exp(-z)) < 1e-12, "z^0.5 K_0.5(z) normalized");
    expect(std::fabs(FourierTools::besselKNormalized(1.5, z) - (1.0 + z)*std::exp(-z)) < 1e-12, "z^1.5 K_1.5(z) normalized");
    expect(std::fabs(FourierTools::besselKNormalized(2.5, z) - (1.0 + z + z*z/3.0)*std::exp(-z)) < 1e-12, "z^2.5 K_2.5 normalized");
    }
  // half-line Gaussian: the Gaussian on [0, infinity) through erf, and through w
  for (double k : { 0.0, 0.8, 3.0 })
    expect(close(FourierTools::gaussianDomain(k, 0.0, 1.3, 0.0, INFINITY), FourierTools::halfGaussianMoment(0, k, 1.3), 1e-12),
           "half Gaussian, two ways");
}

void testLineTransforms()
{
  std::vector<double> ks{ 0.0, 0.4, 1.7, 3.2 };
  compareLine(Normal(2.0, 0.7, 1.3), 0.7 - 20.0, 0.7 + 20.0, ks, 1e-11, "Normal");
  compareLine(StudentDistribution(1.5, 5.0, 0.3, 0.8), -3000.0, 3000.0, { 0.0, 0.5, 2.0 }, 1e-7, "Student 5", 4000);
  StudentDistribution s1(1.0, 1.0, 0.4, 0.9);
  CauchyDistribution c1(1.0, 0.4, 0.9);
  for (double k : ks) expect(close(s1.fourierTransformAt(k), c1.fourierTransformAt(k), 1e-12), "Student 1 = Cauchy");
  compareLine(ChiSquareDistribution(1.0, 4.0), 0.0, 300.0, ks, 1e-11, "ChiSquare 4");
  compareLine(GammaDistribution(2.0, 3.5, 0.7), 0.0, 120.0, ks, 1e-11, "Gamma");
  compareLine(MaxwellBoltzmannDistribution(1.0, 1.2), 0.0, 40.0, ks, 1e-11, "MaxwellBoltzmann");
  compareLine(WeibullDistribution(1.0, 2.0, 1.5), 0.0, 40.0, ks, 1e-11, "Weibull 2");
  compareLine(WeibullDistribution(1.0, 1.0, 1.5), 0.0, 120.0, ks, 1e-11, "Weibull 1");
  // approximations (no closed form): piecewise Legendre on the quantiles
  WeibullDistribution weibull(1.0, 1.5, 1.2);   // density ~ sqrt(x) at 0: reference integrated in u = sqrt(x)
  for (double k : ks)
    {
    std::vector<double> nodes, weights;
    GaussLegendreIntegrator::rule(20, nodes, weights);
    Complex reference = 0.0;
    int panels = 4000;
    double b = std::sqrt(60.0), h = b/panels;
    for (int p=0; p<panels; p++)
      for (std::size_t q=0; q<nodes.size(); q++)
        {
        double u = (p + 0.5)*h + 0.5*h*nodes[q], x = u*u;
        reference += weights[q]*weibull.evaluateAt(x)*2.0*u*std::exp(Complex(0.0, -k*x));
        }
    reference *= 0.5*h;
    expect(close(weibull.fourierTransformAt(k), reference, 1e-9), "Weibull 1.5 (approximation) at " + std::to_string(k)
           + ": " + text(weibull.fourierTransformAt(k)) + " vs " + text(reference));
    }
  compareLine(LogNormal(1.0, 0.2, 0.6), 0.0, 400.0, ks, 1e-9, "LogNormal (approximation)", 4000);
  compareLine(FDistribution(1.0, 5.0, 12.0), 0.0, 20000.0, { 0.0, 0.3, 1.0 }, 1e-6, "F (approximation)", 40000);
  std::unique_ptr<FourierTransform> lognormal(LogNormal(1.0, 0.2, 0.6).fourierTransform());
  expect(lognormal->method() == FourierTransform::kApproximation, "LogNormal: approximation");
  // signal functions
  compareLine(Boxcar(1.5, 0.3, 2.0), -0.7, 1.3, ks, 1e-12, "Boxcar");
  Triangle triangle(1.2, -0.5, 1.5);
  for (double k : ks)
    {
    Complex reference = numerical([&](double x) { return triangle.evaluateAt(x); }, k, -2.0, -0.5)
                      + numerical([&](double x) { return triangle.evaluateAt(x); }, k, -0.5, 1.0);
    expect(close(triangle.fourierTransformAt(k), reference, 1e-12), "Triangle at " + std::to_string(k));
    }
  TwoSidedExponential laplace(1.0, 0.5, 0.8);
  for (double k : ks)
    {
    Complex reference = numerical([&](double x) { return laplace.evaluateAt(x); }, k, 0.5 - 40.0, 0.5)
                      + numerical([&](double x) { return laplace.evaluateAt(x); }, k, 0.5, 0.5 + 40.0);
    expect(close(laplace.fourierTransformAt(k), reference, 1e-12), "TwoSidedExponential at " + std::to_string(k));
    }
  compareLine(OneSidedExponential(2.0, 1.0, 0.6), 1.0, 1.0 + 40.0, ks, 1e-12, "OneSidedExponential");
  compareLine(DampedOscillation(1.3, 0.2, 1.1, 4.0, 0.7), 0.2, 0.2 + 60.0, ks, 1e-11, "DampedOscillation");
  compareLine(GaussianWavePacket(1.0, -0.3, 0.9, 3.0, 0.4), -20.0, 20.0, ks, 1e-11, "GaussianWavePacket");
  compareLine(HyperbolicSecant(1.0, 0.4, 0.7), -60.0, 60.0, ks, 1e-11, "HyperbolicSecant");
  compareLine(HermiteFunction(3, 1.0, 0.2, 1.4), -40.0, 40.0, ks, 1e-11, "HermiteFunction 3");
  compareLine(HermiteFunction(8, 1.0, 0.0, 0.9), -40.0, 40.0, ks, 1e-11, "HermiteFunction 8");
  compareLine(Voigt(1.0, 0.3, 0.8, 0.4), -20000.0, 20000.0, { 0.0, 0.6, 2.0 }, 5e-5, "Voigt (tails cut)", 200000);
  compareLine(Sinc(1.0, 0.0, 2.0), -20000.0, 20000.0, { 0.5, 1.0, 3.0 }, 5e-5, "Sinc (tails cut)", 200000);
  // Bessel: values of the closed forms
  expect(close(BesselJFunction(0).fourierTransformAt(0.5), 2.0/std::sqrt(0.75), 1e-13), "J0 at k = 0.5");
  expect(std::abs(BesselJFunction(0).fourierTransformAt(1.5)) == 0.0, "J0 beyond k = 1");
  expect(close(Bessel(Bessel::BesselK, 0).fourierTransformAt(0.0), kPi/2.0, 1e-13), "K0 at 0");
  // Normal2D: k = 0 is the volume; rho = 0 is the product of two Gaussians
  Normal2D n2(1.5, 0.2, -0.4, 0.8, 1.3, 0.0);
  Normal gx(1.0, 0.2, 0.8), gy(1.5, -0.4, 1.3);
  expect(close(n2.fourierTransformAt({ 0.7, -1.1 }), gx.fourierTransformAt(0.7)*gy.fourierTransformAt(-1.1), 1e-12), "Normal2D product");
  Normal2D n2r(1.0, 0.0, 0.0, 1.0, 2.0, 0.5);
  expect(close(n2r.fourierTransformAt({ 0.0, 0.0 }), n2r.volume(), 1e-12), "Normal2D volume");
}

void testDomainTransforms()
{
  std::vector<double> ks{ 0.0, 0.3, 7.0, 40.0 };
  compareDomain(Polynomial({ 1.0, -2.0, 0.5, 0.3, -0.1, 0.02 }), -1.3, 2.1, ks, 1e-12, "Polynomial");
  compareDomain(Legendre(7), -0.5, 1.0, ks, 1e-12, "Legendre 7");
  LegendreSeries series(4);
  series.setParameterValues({ 0.5, -0.2, 0.3, 0.1, -0.4, -1.0, 2.0 });
  compareDomain(series, -0.7, 1.9, ks, 1e-12, "LegendreSeries");
  compareDomain(Exponential(1.5, -0.4), 0.5, 3.0, ks, 1e-12, "Exponential");
  compareDomain(Normal(1.0, 0.6, 0.5), -0.4, 2.0, { 0.0, 0.3, 7.0, 20.0 }, 1e-11, "Normal");
  compareDomain(GaussianWavePacket(1.0, 0.0, 0.7, 4.0, 0.3), -1.0, 0.6, ks, 1e-11, "GaussianWavePacket");
  compareDomain(TwoSidedExponential(1.0, 0.5, 0.8), -1.0, 2.0, { 0.0, 0.3, 7.0 }, 1e-6, "TwoSidedExponential (kink inside)");
  FourierHarmonic h(3, FourierHarmonic::kCosineSine, 0.7, -0.4);
  compareDomain(h, 0.3, 2.5, ks, 1e-12, "FourierHarmonic");
  FourierDecomposition d(3);
  d.setConstant(0.4); d.setCosine(1, 0.2); d.setSine(2, -0.3); d.setCosine(3, 0.1); d.setSine(3, 0.05);
  expect(close(d.fourierCoefficient(0, 0.0, 2.0*kPi), 0.4, 1e-13), "FourierDecomposition c0");
  expect(close(d.fourierCoefficient(2, 0.0, 2.0*kPi), Complex(0.0, 0.15), 1e-13), "FourierDecomposition c2 = (c - is)/2");
  expect(close(d.fourierCoefficient(-3, 1.0, 2.0*kPi), Complex(0.05, 0.025), 1e-13), "FourierDecomposition c-3, shifted period");
  FourierFlow flow(2);
  flow.setConstant(10.0); flow.setFlow(2, 0.1); flow.setFlowPhase(2, 0.3);
  expect(close(flow.fourierCoefficient(2, 0.0, 2.0*kPi), 10.0*0.1*std::exp(Complex(0.0, -0.6)), 1e-13), "FourierFlow c2 = k v e^{-2i psi}");
  BezierCurve bezier({ 0.0, 0.5, 1.2, 2.0 }, { 0.0, 1.0, 0.3, 0.8 });
  for (double k : ks)
    {
    Complex reference = numerical([&](double x) { return bezier.evaluateAt(x); }, k, -0.5, 0.0)
                      + numerical([&](double x) { return bezier.evaluateAt(x); }, k, 0.0, 0.5)
                      + numerical([&](double x) { return bezier.evaluateAt(x); }, k, 0.5, 1.2)
                      + numerical([&](double x) { return bezier.evaluateAt(x); }, k, 1.2, 2.0)
                      + numerical([&](double x) { return bezier.evaluateAt(x); }, k, 2.0, 2.6);
    expect(close(bezier.fourierTransformAt(k, -0.5, 2.6), reference, 1e-12), "BezierCurve at " + std::to_string(k));
    }
  // approximation: no exact transform of the Cauchy density on a domain
  CauchyDistribution cauchy(1.0, 0.5, 0.7);
  compareDomain(cauchy, -3.0, 5.0, ks, 1e-11, "Cauchy on a domain (approximation)");
  std::unique_ptr<FourierTransform> t(cauchy.fourierTransform(-3.0, 5.0));
  expect(t->method() == FourierTransform::kApproximation, "Cauchy on a domain: approximation");
  std::unique_ptr<FourierTransform> p(Polynomial({ 1.0, 2.0 }).fourierTransform(0.0, 1.0));
  expect(p->method() == FourierTransform::kExactOnDomain, "Polynomial on a domain: exact");
  expect(close(p->coefficient(0), 2.0, 1e-13), "Polynomial: c0 = mean on the period");
  bool thrown = false;
  try { std::unique_ptr<FourierTransform> none(Polynomial({ 1.0 }).fourierTransform()); } catch (const std::exception &) { thrown = true; }
  expect(thrown, "Polynomial: no transform on the line");
  // step and comb
  HeavisideStep step(2.0, 0.5);
  expect(close(step.fourierTransformAt(0.0, -1.0, 3.0), 5.0, 1e-13), "step: integral");
  DiracComb comb(1.5, 0.3, 0.8);
  expect(std::fabs(comb.integralInRange(0.0, 4.0) - 1.5*5.0) < 1e-12, "comb: 5 teeth in [0, 4]");
  expect(close(comb.fourierCoefficient(3, 0.3 - 0.4, 0.8), 1.5/0.8*std::exp(Complex(0.0, -2.0*kPi*3*0.3/0.8)), 1e-12), "comb: c3");
  expect(close(comb.fourierCoefficient(2, 0.3, 0.8), 1.5/0.8*std::exp(Complex(0.0, -2.0*kPi*2*0.3/0.8)), 1e-12), "comb: c2, teeth on the bounds");
}

void testDiscrete()
{
  PoissonDistribution poisson(1.0, 3.3);
  BinomialDistribution binomial(2.0, 12.0, 0.3);
  NegativeBinomialDistribution negative(1.0, 4.0, 2.5);
  HypergeometricDistribution hyper(1.0, 50.0, 10.0, 10.0);
  for (double k : { 0.0, 0.7, 2.5 })
    {
    for (const DiscreteDistribution * d : std::vector<const DiscreteDistribution *>{ &poisson, &binomial, &negative, &hyper })
      {
      Complex sum = 0.0;
      for (unsigned long m=0; m<400; m++) sum += d->probability(m)*std::exp(Complex(0.0, -k*m));
      expect(close(d->fourierTransformAt(k), d->amplitude()*sum, 1e-12), std::string("counts at ") + std::to_string(k));
      }
    Complex part = 0.5*poisson.probability(2)*std::exp(Complex(0.0, -2.0*k)) + poisson.probability(3)*std::exp(Complex(0.0, -3.0*k))
                 + poisson.probability(4)*std::exp(Complex(0.0, -4.0*k)) + 0.5*poisson.probability(5)*std::exp(Complex(0.0, -5.0*k));
    expect(close(poisson.fourierTransformAt(k, 2.0, 5.0), part, 1e-13), "Poisson on [2, 5]");
    }
}

void testComposites()
{
  Normal g(1.0, 0.3, 0.6);
  CauchyDistribution c(2.0, -0.2, 0.4);
  CompositeFunction sum({ &g, &c });
  for (double k : { 0.0, 1.3 })
    expect(close(sum.fourierTransformAt(k), g.fourierTransformAt(k) + c.fourierTransformAt(k), 1e-13), "composite sum");
  std::unique_ptr<FourierTransform> st(sum.fourierTransform());
  expect(close(st->valueAt(1.3), g.fourierTransformAt(1.3) + c.fourierTransformAt(1.3), 1e-13), "composite sum, as a function");
  CompositeFunction product = CompositeFunction::product(&g, &c);
  expect(close(product.fourierTransformAt({ 0.5, -0.8 }), g.fourierTransformAt(0.5)*c.fourierTransformAt(-0.8), 1e-13), "composite product");
  ParameterizedComposite p;
  p.add("g", g);
  p.add("c", c);
  std::unique_ptr<FourierTransform> pt(p.fourierTransform());
  expect(close(pt->valueAt(0.9), g.fourierTransformAt(0.9) + c.fourierTransformAt(0.9), 1e-13), "parameterized composite");
  std::unique_ptr<FourierTransform> gt(g.fourierTransform());
  gt->setPart(FourierTransform::kModulus);
  expect(std::fabs(gt->evaluateAt(0.8) - std::abs(g.fourierTransformAt(0.8))) < 1e-14 && gt->isAnalytical(), "part: modulus");
}
} // namespace

int main()
{
  try
    {
    testSpecialFunctions();
    testLineTransforms();
    testDomainTransforms();
    testDiscrete();
    testComposites();
    }
  catch (const std::exception & e)
    {
    std::cout << "Exception: " << e.what() << std::endl;
    failures++;
    }
  std::cout << checks - failures << "/" << checks << " checks passed" << std::endl;
  return failures == 0 ? 0 : 1;
}
