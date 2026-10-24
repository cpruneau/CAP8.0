/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestMathReflection.cpp
//! @brief Tests of the Math classes used through the run-time reflection only (by name): histograms filled from a
//! Mersenne Twister, as an interpreter would.
#include <cmath>
#include <cstdio>
#include <string>
#include "Reflection.hpp"

using namespace CAP;
using namespace CAP::REFLECTION;

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
} // namespace

#define CHECK(condition) check((condition), #condition, __LINE__)

int main()
{
  Registry & registry = Registry::instance();
  const ClassInfo * histogramClass = registry.find("Histogram");
  const ClassInfo * twisterClass = registry.find("MersenneTwister");
  CHECK(histogramClass && twisterClass && registry.find("CAP::Object"));
  if (!histogramClass || !twisterClass) return 1;
  CHECK(histogramClass->inheritsFrom(*registry.find("CAP::Object")) && histogramClass->inheritsFrom(*registry.find("Partition")));

  Value generator = twisterClass->create({ 12345 });
  Value h = histogramClass->create({ "h", "Gaussian numbers", 40, -4.0, 4.0 });
  for (int k=0; k<10000; k++) h.call("fill", { generator.call("gauss", { 0.5, 1.0 }) });
  CHECK(h.get("name").toText() == "h" && h.get("title").toText() == "Gaussian numbers");   // members of Object
  CHECK(h.call("nBins", { 0 }).toInteger() == 40 && h.get("rank").toInteger() == 1);       // of Partition
  CHECK(std::fabs(h.call("mean", { 0 }).toReal() - 0.5) < 0.05);
  CHECK(std::fabs(h.call("integral").toReal() - 10000.0) < 50.0);                          // a few beyond the edges
  h.call("scale", { 0.5 });
  CHECK(std::fabs(h.call("integral").toReal() - 5000.0) < 25.0);
  h.set("name", "renamed");
  CHECK(h.get("name").toText() == "renamed");
  // a copy made by the copy constructor
  Value copy = histogramClass->create({ h });
  CHECK(copy.call("binValueAt", { 20 }).toReal() == h.call("binValueAt", { 20 }).toReal());
  // lists as std::vector arguments
  double x = generator.call("polynomial", { std::vector<Value>{ 1.0, 0.0, 3.0 }, 0.0, 1.0 }).toReal();
  CHECK(x >= 0.0 && x <= 1.0);
  // a 2D histogram, filled with weights
  Value h2 = histogramClass->create({ "h2", "map", 10, 0.0, 1.0, 10, 0.0, 1.0 });
  h2.call("fill", { 0.55, 0.15, 2.0 });
  CHECK(h2.call("binValueAt", { 5, 1 }).toReal() == 2.0);
  std::string d = histogramClass->describe();
  CHECK(d.find("fill void (double, double, double)") != std::string::npos);

  std::printf("TestMathReflection: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
