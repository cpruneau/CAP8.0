/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestFitRegions.cpp
//! @brief Test program of the regions of fits: Region, FitSelection, regions in Fitter, likelihoods and FitRecord (run by ctest).
//
// TestFitRegions -- tests of the points chosen for a fit: a box and regions included and excluded (FitSelection), in
// one and two variables, for chi square, Poisson and unbinned fits; the normalization of an unbinned fit over a
// selection (FitSelection::cells()); the starting values from the selection; the regions kept by FitRecord, written to
// and read from the binary encoding of the fits; the residuals and projections of fits with regions. Prints each
// failed check and exits with the number of failures (0: all passed).
//
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include "BinaryIO.hpp"
#include "DataPoint2D.hpp"
#include "DataPoint3D.hpp"
#include "DataPointsVector.hpp"
#include "Exponential.hpp"
#include "FitSelection.hpp"
#include "FitViews.hpp"
#include "Fitter.hpp"
#include "Histogram.hpp"
#include "Linear.hpp"
#include "MathExceptions.hpp"
#include "Normal.hpp"
#include "Normal2D.hpp"
#include "Region.hpp"

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

bool near(double a, double b, double tolerance) { return std::fabs(a - b) <= tolerance; }

//! Straight line 1 + 2x at x = 0, 0.5, ..., 20 (uncertainty 0.1), with a peak of 50 added for 8 <= x <= 12.
DataPointsVector lineWithPeak()
{
  DataPointsVector points;
  for (int k=0; k<=40; k++)
    {
    double x = 0.5*k;
    double y = 1.0 + 2.0*x + (x >= 8.0 && x <= 12.0 ? 50.0*std::exp(-0.5*(x - 10.0)*(x - 10.0)) : 0.0);
    points.addPoint(DataPoint2D(x, y, 0.1));
    }
  return points;
}

//! Uniform deviates in (0, 1) of a fixed sequence.
double uniform(unsigned long long & seed)
{
  seed = seed*6364136223846793005ULL + 1442695040888963407ULL;
  return ((seed >> 11) + 0.5)/9007199254740992.0;
}

} // namespace

#define CHECK(condition) check((condition), #condition, __LINE__)

//! @brief Run the checks. @return number of failed checks (0: all passed)
int main()
{
  // ---- Region and FitSelection
  {
    Region band = Region::interval(0, 3.0, 2.0);                             // ends in any order
    double a = 0, b = 0;
    CHECK(band.intervalOf(0, a, b) && a == 2.0 && b == 3.0 && !band.restricts(1) && band.contains({2.5, 99.0}));
    CHECK(band.toString() == "x in [2, 3]" && Region().toString() == "everywhere" && Region().unrestricted());
    Region box = Region().add(1, 0.0, 1.0).add(0, 2.0, 3.0);
    CHECK(box.toString() == "x in [2, 3] and y in [0, 1]" && box.toString({"#Delta#eta", "#Delta#phi"}) == "#Delta#eta in [2, 3] and #Delta#phi in [0, 1]");
    CHECK(box == Region().add(0, 2.0, 3.0).add(1, 0.0, 1.0) && box != band);
    CHECK(box.isValid(2) && !box.isValid(1) && !Region::interval(0, 0.0, INFINITY).isValid(1));
    CHECK(Region(box).remove(1) == band && Region::dimensionName(3) == "x3");

    FitSelection all;
    CHECK(all.everything() && all.contains({123.0}) && all.toString() == "all points");
    FitSelection s({0.0}, {10.0});
    s.exclude(0, 4.0, 6.0);
    CHECK(s.contains(3.9) && !s.contains(4.0) && !s.contains(5.0) && !s.contains(6.0) && s.contains(6.1) && !s.contains(10.5));
    CHECK(s.toString() == "[0, 10]; excluding x in [4, 6]");
    FitSelection sidebands;
    sidebands.include(0, 1.0, 2.0).include(0, 8.0, 9.0);
    CHECK(sidebands.contains(1.5) && sidebands.contains(8.0) && !sidebands.contains(5.0) && !sidebands.hasBox());
    sidebands.exclude(0, 1.4, 1.6);                                          // an exclusion inside an included region
    CHECK(!sidebands.contains(1.5) && sidebands.contains(1.3));
    bool thrown = false;
    try { FitSelection({1.0}, {0.0}); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);
    thrown = false;
    try { FitSelection one({0.0}, {1.0}); one.exclude(1, 0.0, 1.0); one.validate(1); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);                                                           // a region along a variable the function does not have
  }

  // ---- cells of a selection: the integral over the selection
  {
    FitSelection s({0.0}, {10.0});
    s.exclude(0, 2.0, 3.0);
    auto cells = s.cells();
    CHECK(cells.size() == 2 && cells[0].first[0] == 0.0 && cells[0].second[0] == 2.0 && cells[1].first[0] == 3.0 && cells[1].second[0] == 10.0);
    Exponential f(1.0, -1.0);                                                // exp(-x)
    double integral = 0;
    for (const auto & cell : cells) integral += f.integralInRange(cell.first, cell.second);
    CHECK(near(integral, (1 - std::exp(-2.0)) + (std::exp(-3.0) - std::exp(-10.0)), 1E-9));

    FitSelection plane({0.0, 0.0}, {4.0, 4.0});                              // area 16
    plane.exclude(Region().add(0, 1.0, 2.0).add(1, 1.0, 2.0));               // - 1
    plane.exclude(Region::interval(1, 3.0, 5.0));                            // - 4 (beyond the box: clipped)
    double area = 0;
    for (const auto & cell : plane.cells()) area += (cell.second[0] - cell.first[0])*(cell.second[1] - cell.first[1]);
    CHECK(near(area, 11.0, 1E-12));
    FitSelection windows({0.0, 0.0}, {4.0, 4.0});                           // two overlapping windows: union, area 4 + 4 - 1
    windows.include(Region().add(0, 0.0, 2.0).add(1, 0.0, 2.0)).include(Region().add(0, 1.0, 3.0).add(1, 1.0, 3.0));
    area = 0;
    for (const auto & cell : windows.cells()) area += (cell.second[0] - cell.first[0])*(cell.second[1] - cell.first[1]);
    CHECK(near(area, 7.0, 1E-12));
    bool thrown = false;
    try { FitSelection().exclude(0, 0.0, 1.0).cells(); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);                                                           // cells need a box
  }

  // ---- chi square: a peak excluded, sidebands included
  {
    DataPointsVector points = lineWithPeak();
    Linear line(0.0, 1.0);
    Fitter fitter(line, points);
    FitResult plain = fitter.fit();
    CHECK(plain.chiSquare > 1E4);                                            // the peak spoils the line

    line = Linear(0.0, 1.0);
    fitter.exclude(0, 7.9, 12.1);                                            // x = 8, ..., 12 (9 points) left out
    FitResult r = fitter.fit();
    CHECK(r.converged && r.nPoints == 32 && r.nDegreesOfFreedom == 30 && near(r.values[0], 1.0, 1E-6) && near(r.values[1], 2.0, 1E-6) && r.chiSquare < 1E-6);
    FitRecord record = fitter.record("background");
    CHECK(record.excluded.size() == 1 && record.included.empty() && record.inRange(5.0) && !record.inRange(10.0) && record.inRange(20.0));
    CHECK(!record.inRange(-1.0) && record.inBox(std::vector<double>{10.0}) && record.selection().contains(5.0));
    bool hasRow = false;
    for (const auto & row : record.quality()) hasRow = hasRow || (row.first == "excluding" && row.second == "x in [7.9, 12.1]");
    CHECK(hasRow);
    DataPointsVector pulls = residuals(points, record);
    CHECK(pulls.size() == 32);                                               // only the points fitted

    line = Linear(0.0, 1.0);
    fitter.clearRegions();
    fitter.include(0, 0.0, 3.0);                                             // two sidebands: 7 + 7 points
    fitter.include(0, 17.0, 20.0);
    r = fitter.fit();
    CHECK(r.nPoints == 14 && near(r.values[1], 2.0, 1E-6) && fitter.record().included.size() == 2 && !fitter.record().inRange(10.0));

    line = Linear(0.0, 1.0);
    fitter.clearRegions();
    fitter.setRange(0.0, 15.0);                                              // a box and an exclusion
    fitter.exclude(0, 7.9, 12.1);
    r = fitter.fit();
    CHECK(r.nPoints == 22 && near(r.values[1], 2.0, 1E-6));
    fitter.clearRange();                                                     // the regions are kept
    CHECK(fitter.selection().excluded().size() == 1 && !fitter.selection().hasBox());
    CHECK(fitter.fit().nPoints == 32);

    bool thrown = false;
    try { fitter.exclude(Region::interval(1, 0.0, 1.0)); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);                                                           // y does not exist for a function of x
    thrown = false;
    fitter.clearRegions();
    fitter.include(0, 50.0, 60.0);                                           // no point
    try { fitter.fit(); } catch (const MathException &) { thrown = true; }
    CHECK(thrown);
  }

  // ---- fitAndAttach() with regions; starting values from the selection
  {
    DataPointsVector points = lineWithPeak();
    Linear line(0.0, 0.0);
    Fitter::FitOptions options;
    options.excluded = { Region::interval(0, 7.9, 12.1) };
    options.label = "background";
    FitRecord record = Fitter::fitAndAttach(line, points, options);
    CHECK(points.nFits() == 1 && points.fit(0).excluded.size() == 1 && near(record.result.values[1], 2.0, 1E-6));

    DataPointsVector peak;                                                   // a Normal peak with a spike far from it
    for (int k=0; k<=80; k++)
      {
      double x = 0.25*k;
      peak.addPoint(DataPoint2D(x, 100*std::exp(-0.5*(x - 6.0)*(x - 6.0)) + (x >= 15.0 && x <= 16.0 ? 500.0 : 0.0), 1.0));
      }
    Normal model;
    FitSelection selection;
    CHECK(Fitter::estimate(model, peak, selection) && model.parameter(1).value() > 7.0);   // the spike pulls the mean
    selection.exclude(0, 14.9, 16.1);
    CHECK(Fitter::estimate(model, peak, selection) && near(model.parameter(1).value(), 6.0, 0.05) && near(model.parameter(0).value(), 100.0, 1E-6));
  }

  // ---- two variables: a box and a band excluded
  {
    DataPointsVector points;
    Normal2D truth(50.0, 1.0, -0.5, 1.2, 0.8, 0.3);
    for (int i=0; i<=24; i++)
      for (int j=0; j<=24; j++)
        {
        double x = -3.0 + 0.25*i, y = -3.0 + 0.25*j;
        double z = truth.evaluateAt(x, y);
        if (x >= 2.0 && x <= 2.5 && y >= 1.0 && y <= 1.5) z += 300.0;      // a box of bad points
        if (y >= 2.6) z -= 40.0;                                             // a band of bad points
        points.addPoint(DataPoint3D(x, y, z, 0.5));
        }
    Normal2D model(40.0, 0.8, -0.3, 1.0, 1.0, 0.0);
    Fitter fitter(model, points);
    fitter.exclude(Region().add(0, 1.9, 2.6).add(1, 0.9, 1.6));              // 3 x 3 points
    fitter.exclude(1, 2.55, 3.1);                                            // y = 2.75, 3: 2 x 25 points
    FitResult r = fitter.fit();
    CHECK(r.converged && r.nPoints == 625 - 9 - 50 && r.chiSquare < 1E-6);
    CHECK(near(r.values[0], 50.0, 1E-4) && near(r.values[1], 1.0, 1E-5) && near(r.values[2], -0.5, 1E-5) && near(r.values[5], 0.3, 1E-5));
    FitRecord record = fitter.record();
    CHECK(!record.inRange(std::vector<double>{2.25, 1.25}) && !record.inRange(std::vector<double>{0.0, 3.0}) && record.inRange(std::vector<double>{0.0, 0.0}));
  }

  // ---- Poisson fit of a histogram with a region excluded; projection of a fit of two variables with regions
  {
    Histogram h("h", "h", 40, 0.0, 20.0);
    for (unsigned int i=0; i<40; i++)
      {
      double x = h.binCenter(i, 0);
      double v = std::round(200.0*std::exp(-0.2*x));
      if (x > 9.0 && x < 11.0) v += 500.0;                                   // a spike
      h.setBinValueAt(i, v);
      h.setBinUncertaintyAt(i, std::sqrt(v));
      }
    Exponential model(150.0, -0.1);
    Fitter fitter(model, h, Fitter::kPoisson);
    fitter.exclude(0, 9.0, 11.0);                                            // bins centered at 9.25, ..., 10.75
    FitResult r = fitter.fit();
    CHECK(r.converged && r.nPoints == 36 && near(r.values[1], -0.2, 0.005) && r.chiSquare/r.nDegreesOfFreedom < 0.5);

    Histogram h2("h2", "h2", 20, -2.0, 2.0, 20, -2.0, 2.0);
    for (unsigned int j=0; j<20; j++)
      for (unsigned int i=0; i<20; i++)
        {
        double x = h2.binCenter(i, 0), y = h2.binCenter(j, 1);
        double v = 100*std::exp(-0.5*(x*x + y*y));
        h2.setBinValueAt(i, j, v);
        h2.setBinUncertaintyAt(i, j, 1.0);
        }
    Normal2D model2(80.0, 0.1, 0.1, 1.2, 1.2, 0.0);
    Fitter fitter2(model2, h2);
    fitter2.exclude(0, 0.5, 1.0);                                            // along x
    fitter2.exclude(Region().add(0, -1.0, -0.5).add(1, -1.0, -0.5));        // a box
    fitter2.fit();
    h2.addFit(fitter2.record("peak"));
    Histogram px = projection(h2, 0);
    CHECK(px.nFits() == 1 && px.fit(0).excluded.size() == 1 && !px.fit(0).inRange(0.7) && px.fit(0).inRange(0.1));
    CHECK(px.fit(0).result.status.contains("other axis"));                   // the box cannot apply to the projection
    Histogram py = projection(h2, 1);
    CHECK(py.nFits() == 1 && py.fit(0).excluded.empty() && py.fit(0).inRange(0.7));
  }

  // ---- unbinned fit: normalization over the selection
  {
    std::vector<double> events;                                              // exp(-x/2), x in [0, 10]
    unsigned long long seed = 11;
    for (int k=0; k<40000; k++)
      {
      double x = -2.0*std::log(1 - uniform(seed)*(1 - std::exp(-5.0)));
      events.push_back(x);
      }
    Exponential model(1.0, -1.0);
    model.fixParameter(0);
    UnbinnedLikelihood likelihood(model, events, 0.0, 10.0);
    FitSelection selection;
    selection.exclude(0, 1.0, 3.0).exclude(0, 6.0, 7.0);                    // no box: the range of the events is kept
    likelihood.setSelection(selection);
    CHECK(likelihood.selection().hasBox() && likelihood.selection().high()[0] == 10.0);
    Fitter fitter(likelihood);
    FitResult r = fitter.fit();
    CHECK(r.converged && near(r.values[1], -0.5, 4*r.errors[1]) && r.errors[1] > 0 && r.errors[1] < 0.02);
    int inside = 0;
    for (double x : events) inside += (x < 1.0 || x > 3.0) && (x < 6.0 || x > 7.0) ? 1 : 0;
    CHECK(r.nPoints == inside);
    // without the normalization over the selection, the slope would come out wrong: check against the box alone
    Exponential plain(1.0, -1.0);
    plain.fixParameter(0);
    std::vector<double> kept;
    for (double x : events) if ((x < 1.0 || x > 3.0) && (x < 6.0 || x > 7.0)) kept.push_back(x);
    UnbinnedLikelihood boxOnly(plain, kept, 0.0, 10.0);
    Fitter wrong(boxOnly);
    double wrongSlope = wrong.fit().values[1];
    CHECK(std::fabs(wrongSlope + 0.5) > 5*r.errors[1]);                     // about -0.53: biased by 10 sigma
  }

  // ---- regions written and read back with the fits; fits without regions unchanged
  {
    DataPointsVector points = lineWithPeak();
    Linear line(0.0, 1.0);
    Fitter fitter(line, points);
    fitter.setRange(0.0, 18.0);
    fitter.exclude(0, 7.9, 12.1);
    fitter.include(Region::interval(0, 0.0, 16.0));
    fitter.fit();
    FitList list;
    list.add(fitter.record("with regions"));
    fitter.clearRegions();
    fitter.fit();
    list.add(fitter.record("without"));
    BinaryWriter w;
    list.encodeBinary(w);
    BinaryReader reader(w.buffer().data(), w.buffer().size());
    FitList read;
    CHECK(read.decodeBinary(reader) == 0 && read.size() == 2 && reader.remaining() == 0);
    CHECK(read.at(0).excluded.size() == 1 && read.at(0).excluded[0] == Region::interval(0, 7.9, 12.1));
    CHECK(read.at(0).included.size() == 1 && read.at(0).included[0] == Region::interval(0, 0.0, 16.0));
    CHECK(!read.at(0).inRange(10.0) && !read.at(0).inRange(17.0) && read.at(0).inRange(15.0));
    CHECK(read.at(1).included.empty() && read.at(1).excluded.empty() && read.at(1).inRange(10.0));

    FitList plain;                                                           // a fit without regions: the same bytes as before regions existed
    plain.add(fitter.record("without"));
    BinaryWriter a;
    plain.encodeBinary(a);
    BinaryWriter b;
    list.encodeBinary(b);
    CHECK(a.buffer().size() < b.buffer().size());
  }

  std::printf("TestFitRegions: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
