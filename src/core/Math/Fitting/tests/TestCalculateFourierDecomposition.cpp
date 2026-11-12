/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestCalculateFourierDecomposition.cpp
//! @brief Test program of CalculateFourierDecomposition (run by ctest).
//
// TestCalculateFourierDecomposition -- a Fourier series recovered exactly in both forms (chi square 0, higher
// harmonics 0); the chi square of a truncated series against its formula; for a series with Gaussian noise of known
// sigma, chi square per degree of freedom near 1, uniform p-values, and pulls of the coefficients (both forms) of mean
// 0 and rms 1; a function that is not a series (x); the period moved; the errors refused. Prints each failed check
// and exits with the number of failures.
//
#include <cmath>
#include <cstdio>
#include <functional>
#include <random>
#include <string>
#include <vector>
#include "CalculateFourierDecomposition.hpp"
#include "DataPoint2D.hpp"
#include "DataPointsVector.hpp"
#include "FunctionRegistry.hpp"
#include "Fitter.hpp"
#include "Function.hpp"
#include "Histogram.hpp"

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

bool close(double a, double b, double tolerance) { return std::fabs(a - b) <= tolerance; }

template <typename F>
bool throws(F f) { try { f(); } catch (const std::exception &) { return true; } return false; }

//! A function of one variable given by a lambda.
class Lambda : public Function
{
public:
  explicit Lambda(std::function<double(double)> g, unsigned int rank=1) : Function(rank), _g(g) { }
  double evaluateAt(double x) const override { return _g(x); }
  double evaluateAt(const std::vector<double> & x) const override { return _g(x[0]); }
  using Function::evaluateAt;
private:
  std::function<double(double)> _g;
};

double series(double x)
{
  return 3.0 + 0.4*std::cos(x) - 0.2*std::sin(x) + 0.25*std::cos(2*x) + 0.1*std::sin(2*x) - 0.05*std::sin(3*x);
}
} // namespace

int main()
{
  Lambda f(series);

  // a series recovered exactly; the harmonics above its order are 0; chi square 0
  CalculateFourierDecomposition calculator(5);
  FourierDecomposition d = calculator.calculate(f);
  CHECK(d.nMax() == 5 && d.form() == FourierDecomposition::kCosineSine);
  CHECK(close(d.constant(), 3.0, 1.0E-12) && close(d.cosine(1), 0.4, 1.0E-12) && close(d.sine(1), -0.2, 1.0E-12));
  CHECK(close(d.cosine(2), 0.25, 1.0E-12) && close(d.sine(2), 0.1, 1.0E-12) && close(d.sine(3), -0.05, 1.0E-12));
  CHECK(close(d.cosine(3), 0.0, 1.0E-12) && close(d.cosine(4), 0.0, 1.0E-12) && close(d.sine(5), 0.0, 1.0E-12));
  const FitResult & r = calculator.result();
  CHECK(r.nPoints == 360 && r.nDegreesOfFreedom == 360 - 11 && r.values.size() == 11 && r.errors.size() == 11);
  CHECK(r.chiSquare < 1.0E-9 && calculator.maxDeviation() < 1.0E-6 && calculator.rmsDeviation() < 1.0E-6);
  CHECK(close(r.errors[0], std::sqrt(1.0/360), 1.0E-14) && close(r.errors[3], std::sqrt(2.0/360), 1.0E-14));
  CHECK(r.converged && r.validErrors && calculator.samples().size() == 360);

  // form 2: k (1 + 2 sum v_n cos(n (x - psi_n)))
  Lambda flow([](double x) { return 50.0*(1.0 + 2.0*(0.08*std::cos(2*(x - 0.4)) + 0.03*std::cos(3*(x + 0.2)))); });
  FourierDecomposition g = CalculateFourierDecomposition::decompose(flow, 4, FourierDecomposition::kFlow);
  CHECK(g.form() == FourierDecomposition::kFlow && g.nMax() == 4);
  CHECK(close(g.constant(), 50.0, 1.0E-10) && close(g.flow(2), 0.08, 1.0E-12) && close(g.flowPhase(2), 0.4, 1.0E-10));
  CHECK(close(g.flow(3), 0.03, 1.0E-12) && close(g.flowPhase(3), -0.2, 1.0E-10) && close(g.flow(1), 0.0, 1.0E-12));
  for (double x : {-2.0, 0.3, 2.9}) CHECK(close(g(x), flow.evaluateAt(x), 1.0E-10));

  // a truncated series: chi square = sum of the squares of the harmonics left out at the samples, N (c^2 + s^2)/2
  CalculateFourierDecomposition truncated(1);
  truncated.setNSamples(200);
  truncated.setError(0.05);
  truncated.calculate(f);
  double expected = 200.0*(0.25*0.25 + 0.1*0.1 + 0.05*0.05)/2.0/(0.05*0.05);
  CHECK(close(truncated.chiSquare(), expected, 1.0E-4*expected));
  CHECK(truncated.nDegreesOfFreedom() == 197 && close(truncated.chiSquarePerDegreeOfFreedom(), expected/197, 1.0E-4*expected/197));
  CHECK(truncated.pValue() < 1.0E-10);
  CHECK(close(truncated.rmsDeviation(), std::sqrt((0.25*0.25 + 0.1*0.1 + 0.05*0.05)/2.0), 1.0E-6));

  // Gaussian noise of known sigma: chi square / ndf near 1, p-values uniform, pulls of mean 0 and rms 1
  const unsigned int n = 120, nTrials = 400;
  const double sigma = 0.05, h = 2.0*M_PI/n;
  std::mt19937 random(2026);
  std::normal_distribution<double> gauss(0.0, sigma);
  std::vector<double> noise(n);
  Lambda noisy([&](double x) { int i = int(std::lround((x + M_PI)/h - 0.5)); return series(x) + noise[i]; });
  Lambda noisyFlow([&](double x)
    { int i = int(std::lround((x + M_PI)/h - 0.5)); return 2.0*(1.0 + 2.0*0.1*std::cos(2*(x - 0.3))) + noise[i]; });
  CalculateFourierDecomposition first(3), second(2, FourierDecomposition::kFlow);
  for (CalculateFourierDecomposition * c : {&first, &second}) { c->setNSamples(n); c->setError(sigma); }
  double sumChi2 = 0.0, sumP = 0.0, pull[4] = {0, 0, 0, 0}, pull2[4] = {0, 0, 0, 0};
  int nLowP = 0;
  for (unsigned int t=0; t<nTrials; t++)
    {
    for (double & e : noise) e = gauss(random);
    FourierDecomposition a = first.calculate(noisy);
    sumChi2 += first.chiSquarePerDegreeOfFreedom();
    sumP += first.pValue();
    if (first.pValue() < 0.1) nLowP++;
    double p0 = (a.cosine(2) - 0.25)/first.result().errors[3], p1 = (a.constant() - 3.0)/first.result().errors[0];
    FourierDecomposition b = second.calculate(noisyFlow);
    double p2 = (b.flow(2) - 0.1)/second.result().errors[3], p3 = (b.flowPhase(2) - 0.3)/second.result().errors[4];
    double p[4] = {p0, p1, p2, p3};
    for (int q=0; q<4; q++) { pull[q] += p[q]; pull2[q] += p[q]*p[q]; }
    }
  CHECK(close(sumChi2/nTrials, 1.0, 0.02));
  CHECK(close(sumP/nTrials, 0.5, 0.05) && nLowP > 20 && nLowP < 65);
  for (int q=0; q<4; q++)
    {
    double mean = pull[q]/nTrials, rms = std::sqrt(pull2[q]/nTrials - mean*mean);
    CHECK(close(mean, 0.0, 0.2) && close(rms, 1.0, 0.12));
    }

  // not a series: f(x) = x over [-pi, pi], s_n close to 2 (-1)^(n+1)/n
  Lambda line([](double x) { return x; });
  CalculateFourierDecomposition sawtooth(3);
  sawtooth.setNSamples(4000);
  FourierDecomposition s = sawtooth.calculate(line);
  CHECK(close(s.constant(), 0.0, 1.0E-12) && close(s.cosine(1), 0.0, 1.0E-12));
  CHECK(close(s.sine(1), 2.0, 1.0E-5) && close(s.sine(2), -1.0, 1.0E-5) && close(s.sine(3), 2.0/3.0, 1.0E-5));
  CHECK(sawtooth.chiSquare() > 100.0);

  // the period moved: same coefficients for a periodic function
  CalculateFourierDecomposition moved(3);
  moved.setLow(0.0);
  FourierDecomposition m = moved.calculate(f);
  CHECK(close(m.cosine(2), 0.25, 1.0E-12) && close(m.sine(3), -0.05, 1.0E-12));

  // the fit of a function: the same decomposition as the averages
  CalculateFourierDecomposition fitting(3, FourierDecomposition::kFlow, CalculateFourierDecomposition::kFit);
  fitting.setError(0.01);
  FourierDecomposition fitted = fitting.calculate(f);
  FourierDecomposition averaged = CalculateFourierDecomposition::decompose(f, 3, FourierDecomposition::kFlow,
                                                                           CalculateFourierDecomposition::kAverage);
  for (unsigned int k=0; k<7; k++) CHECK(close(fitted.parameter(k).value(), averaged.parameter(k).value(), 1.0E-6));
  CHECK(fitting.result().converged && fitting.chiSquare() < 1.0E-3 && fitting.nDegreesOfFreedom() == 360 - 7);
  CHECK(close(fitting.result().errors[3], std::sqrt(2.0/360)*0.01/(2*3.0), 1.0E-5));   // v2: sigma(c)/(2k), roughly

  // data points of unequal uncertainties: the fit weighs them, the averages do not
  DataPointsVector unequal;
  for (unsigned int i=0; i<40; i++)
    {
    double x = -M_PI + (i + 0.5)*2.0*M_PI/40, e = i < 20 ? 0.01 : 1.0;
    unequal.addPoint(DataPoint2D(float(x), float(series(x) + (i%2 ? e : -e)), float(e)));
    }
  CalculateFourierDecomposition byAverage(3), byFit(3, FourierDecomposition::kCosineSine, CalculateFourierDecomposition::kFit);
  FourierDecomposition da = byAverage.calculate(unequal), df = byFit.calculate(unequal);
  CHECK(byFit.chiSquare() < byAverage.chiSquare());
  CHECK(byAverage.result().nPoints == 40 && byFit.result().nPoints == 40);
  CHECK(byFit.result().errors[0] < byAverage.result().errors[0]);   // the precise points dominate the fit
  std::vector<double> xs, ys, ws;
  for (unsigned int i=0; i<unequal.size(); i++)
    { DataPoint2D p = unequal.point(i); xs.push_back(p.x()); ys.push_back(p.y()); ws.push_back(1.0/(p.yStatError()*p.yStatError())); }
  std::vector<double> ls;
  CHECK(CalculateFourierDecomposition::leastSquares(xs, ys, ws, 3, ls));
  for (unsigned int k=0; k<7; k++) CHECK(close(df.parameter(k).value(), ls[k], 1.0E-5));
  (void) da;

  // points over part of the period: the fit only
  DataPointsVector part;
  for (unsigned int i=0; i<30; i++) { double x = -1.0 + 2.5*i/29.0; part.addPoint(DataPoint2D(float(x), float(series(x)), 0.001f)); }
  CHECK(throws([&] { byAverage.calculate(part); }));
  FourierDecomposition dp = byFit.calculate(part);
  CHECK(close(dp.cosine(2), 0.25, 1.0E-3) && close(dp.sine(3), -0.05, 1.0E-3) && byFit.chiSquare() < 1.0);

  // a histogram of averages over its bins: bin averaging recovers the coefficients of the function
  Histogram histogram("h", "h", 12, -M_PI, M_PI);
  double width = 2.0*M_PI/12;
  for (unsigned int i=0; i<12; i++)
    {
    double a = -M_PI + i*width, b = a + width;
    double average = (2.0*width + 0.5*(std::sin(2*b) - std::sin(2*a)) - 0.2/3.0*(std::cos(3*b) - std::cos(3*a)))/width;
    histogram.setBinAt(i, 100, average, 0.001);   // 2 + cos 2x + 0.2 sin 3x averaged over the bin
    }
  CHECK(close(histogram.point(0).x(), -M_PI + width/2, 1.0E-6));
  for (CalculateFourierDecomposition::Method method : {CalculateFourierDecomposition::kAverage, CalculateFourierDecomposition::kFit})
    {
    CalculateFourierDecomposition binned(3, FourierDecomposition::kCosineSine, method);
    FourierDecomposition plain = binned.calculate(histogram);
    CHECK(plain.cosine(2) < 0.98);                    // the averages over the bins reduce the harmonics
    binned.setBinAveraging(true);
    FourierDecomposition corrected = binned.calculate(histogram);
    CHECK(close(corrected.constant(), 2.0, 1.0E-5) && close(corrected.cosine(2), 1.0, 1.0E-5));
    CHECK(close(corrected.sine(3), 0.2, 1.0E-5) && close(corrected.cosine(1), 0.0, 1.0E-5));
    CHECK(binned.chiSquare() < 1.0);
    }

  // form 1 changed into form 2, and the estimates of the Fit window
  std::vector<double> values = {2.0, 0.0, 0.0, 0.3, 0.4};
  Matrix covariance(5, 5, 0.0);
  for (unsigned int k=0; k<5; k++) covariance(k, k) = 1.0E-4;
  CHECK(CalculateFourierDecomposition::toFlow(values, &covariance));
  CHECK(close(values[3], 0.5/4.0, 1.0E-12) && close(values[4], std::atan2(0.4, 0.3)/2.0, 1.0E-12) && values[1] == 0.0);
  CHECK(std::isinf(covariance(2, 2)) && close(covariance(1, 1), 1.0E-4/16.0, 1.0E-12));
  std::vector<double> zeroK = {0.0, 1.0, 1.0};
  CHECK(!CalculateFourierDecomposition::toFlow(zeroK));
  FourierFlow estimated(3);
  CHECK(FunctionRegistry::typeName(estimated) == "FourierFlow");
  CHECK(Fitter::estimate(estimated, fitting.samples(), -M_PI, M_PI));
  for (unsigned int k=0; k<7; k++) CHECK(close(estimated.parameter(k).value(), fitted.parameter(k).value(), 1.0E-5));
  Fitter windowFit(estimated, fitting.samples());
  FitResult windowResult = windowFit.fit();
  CHECK(windowResult.converged && windowResult.chiSquare < 1.0E-3 && close(estimated.flow(2), fitted.flow(2), 1.0E-6));

  // errors
  CHECK(throws([] { CalculateFourierDecomposition c(0); }));
  CHECK(throws([&] { calculator.setError(0.0); }));
  CalculateFourierDecomposition few(3);
  few.setNSamples(7);
  CHECK(throws([&] { few.calculate(f); }));
  few.setNSamples(8);
  CHECK(!throws([&] { few.calculate(f); }) && few.nDegreesOfFreedom() == 1);
  Lambda plane([](double x) { return x; }, 2);
  CHECK(throws([&] { calculator.calculate(plane); }));
  Lambda zero([](double x) { return std::cos(x); });
  CHECK(throws([&] { CalculateFourierDecomposition::decompose(zero, 2, FourierDecomposition::kFlow); }));

  std::printf("TestCalculateFourierDecomposition: %d checks, %d failures\n", nChecks, nFailures);
  return nFailures;
}
