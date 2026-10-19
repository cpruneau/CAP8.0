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
//! @file TestGraph2D.cpp
//! @brief Test program of the graphs of rank-2 functions and data: Palette, Grid2D, ranges, color maps (run by ctest).
//
// TestGraph2D -- tests of the color maps of CAP::GRAPHICS::Graph: palettes, grids made from histograms, functions
// and points, the x, y and z ranges, and the SVG drawn (cells, color scale, frame). Prints each failed check and
// exits with the number of failures (0: all passed). With an argument, also writes an example page to that SVG file.
//
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include "Canvas.hpp"
#include "DataPoint3D.hpp"
#include "DataPointsVector.hpp"
#include "Fitter.hpp"
#include "Linear.hpp"
#include "Parabolic.hpp"
#include "Function.hpp"
#include "Graph.hpp"
#include "Grid2D.hpp"
#include "Histogram.hpp"
#include "Palette.hpp"
#include "SvgPainter.hpp"

using namespace CAP;
using namespace CAP::GRAPHICS;

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

bool near(double a, double b, double tolerance=1E-6) { return std::fabs(a - b) <= tolerance; }

int count(const std::string & text, const std::string & part)
{
  int n = 0;
  for (std::size_t at = text.find(part); at != std::string::npos; at = text.find(part, at+1)) n++;
  return n;
}

//! x^2 + y^2, counting its evaluations.
class Paraboloid : public MATH::Function
{
public:
  Paraboloid() : Function(2) { setName("paraboloid"); }
  double evaluateAt(const std::vector<double> & x) const override
  {
    checkRank(x.size(), __FUNCTION__);
    nEvaluations++;
    return x[0]*x[0] + x[1]*x[1];
  }
  using Function::evaluateAt;
  mutable long nEvaluations = 0;
};

//! x y on [-1,1]^2: values of both signs.
class Saddle : public MATH::Function
{
public:
  Saddle() : Function(2) { setName("saddle"); }
  double evaluateAt(const std::vector<double> & x) const override { checkRank(x.size(), __FUNCTION__); return x[0]*x[1]; }
  using Function::evaluateAt;
};

//! SVG of a graph alone on a 600 x 400 page.
std::string svgOf(const Graph & graph)
{
  SvgPainter svg(600, 400);
  graph.draw(svg);
  return svg.document().str();
}
} // namespace

//! @brief Count a check, and report it if the condition is false.
#define CHECK(condition) check((condition), #condition, __LINE__)

//! @brief Run the checks. @return number of failed checks (0: all passed)
int main(int argc, char ** argv)
{
  // ---- palettes
  {
    Palette viridis;
    Color low = viridis.color(0.0), high = viridis.color(1.0);
    CHECK(viridis.type() == Palette::kViridis && viridis.nSteps() == 0);
    CHECK(near(low.red, 68/255.0) && near(low.green, 1/255.0) && near(low.blue, 84/255.0));
    CHECK(near(high.red, 253/255.0) && near(high.green, 231/255.0) && near(high.blue, 37/255.0));
    CHECK(viridis.color(-1.0) == low && viridis.color(2.0) == high && viridis.color(std::nan("")) == low);
    Palette diverging(Palette::kDiverging);
    Color middle = diverging.color(0.5);
    CHECK(near(middle.red, 247/255.0) && near(middle.green, 247/255.0) && near(middle.blue, 247/255.0));
    CHECK(diverging.color(0.0).blue > diverging.color(0.0).red && diverging.color(1.0).red > diverging.color(1.0).blue);
    Palette steps(Palette::kGrayscale, 4);
    CHECK(steps.nSteps() == 4 && steps.color(0.01) == steps.color(0.24) && !(steps.color(0.24) == steps.color(0.26)));
    CHECK(near(steps.color(0.0).red, 1.0 - 0.125));          // the middle of the first band
    CHECK(Palette::type("Bird") == Palette::kBird && Palette::type("nonsense") == Palette::kViridis);
    CHECK(std::string(Palette::name(Palette::kDiverging)) == "diverging");
  }

  // ---- grids from histograms, functions and points
  MATH::Histogram histogram("h", "h", 4, 0.0, 4.0, 2, -1.0, 1.0);
  histogram.fill(0.5, -0.5, 3.0);
  histogram.fill(3.5,  0.5, 1.0);                     // twice in the same bin: 2
  histogram.fill(3.5,  0.5, 1.0);
  {
    Grid2D grid = Grid2D::fromHistogram(histogram);
    CHECK(grid.nX() == 4 && grid.nY() == 2);
    CHECK(grid.xEdges().front() == 0.0 && grid.xEdges().back() == 4.0 && grid.yEdges().front() == -1.0 && grid.yEdges().back() == 1.0);
    CHECK(grid.value(0, 0) == 3.0 && grid.value(3, 1) == 2.0);
    CHECK(std::isnan(grid.value(1, 0)) && std::isnan(grid.value(0, 1)));    // empty bins: no value
    double low, high;
    CHECK(grid.range(0, 4, -1, 1, false, low, high) && low == 2.0 && high == 3.0);
    CHECK(grid.range(3, 4, 0, 1, false, low, high) && low == 2.0 && high == 2.0);   // only the cells in the rectangle
    int i, j;
    CHECK(grid.cell(3.2, 0.7, i, j) && i == 3 && j == 1 && !grid.cell(5, 0, i, j));
    Grid2D fromData;
    CHECK(Grid2D::fromData(histogram, fromData) && fromData.nX() == 4);
    MATH::Histogram line("line", "line", 10, 0.0, 1.0);
    CHECK(Grid2D::fromHistogram(line).isEmpty());                            // rank 1: no grid
  }
  Paraboloid paraboloid;
  {
    Grid2D grid = Grid2D::fromFunction(paraboloid, -1, 1, 4, 0, 2, 2);
    CHECK(grid.nX() == 4 && grid.nY() == 2 && paraboloid.nEvaluations == 8);
    CHECK(near(grid.value(0, 0), 0.75*0.75 + 0.5*0.5) && near(grid.value(2, 1), 0.25*0.25 + 1.5*1.5));   // at the centers
    Grid2D logGrid = Grid2D::fromFunction(paraboloid, 1, 100, 2, 1, 10, 1, true, false);
    CHECK(near(logGrid.xEdges()[1], 10.0) && near(logGrid.value(0, 0), 10.0 + 5.5*5.5, 1E-9));   // geometric centers in x
    CHECK(Grid2D::fromFunction(paraboloid, 1, 0, 4, 0, 1, 4).isEmpty());   // empty range
  }
  MATH::DataPointsVector regular, scattered;
  for (int jy=0; jy<3; jy++)
    for (int ix=0; ix<2; ix++) regular.addPoint(MATH::DataPoint3D(10.0*ix, 1.0*jy, ix + 10*jy));
  scattered.addPoint(MATH::DataPoint3D(0.0, 0.0, 1.0));
  scattered.addPoint(MATH::DataPoint3D(1.0, 2.0, 5.0));
  scattered.addPoint(MATH::DataPoint3D(3.0, 1.0, 3.0));
  {
    Grid2D grid;
    CHECK(regular.rank() == 2 && Grid2D::fromPoints(regular, grid));
    CHECK(grid.nX() == 2 && grid.nY() == 3 && grid.value(1, 2) == 21.0);
    CHECK(near(grid.xEdges()[0], -5.0) && near(grid.xEdges()[1], 5.0) && near(grid.xEdges()[2], 15.0));   // halfway between the nodes
    CHECK(near(grid.yEdges()[0], -0.5) && near(grid.yEdges()[3], 2.5));
    CHECK(!Grid2D::fromPoints(scattered, grid));                            // no grid
  }

  // ---- items and ranges
  {
    Graph graph;
    CHECK(!graph.hasMaps() && graph.nAxes() == 2 && graph.colorScaleWidth() == 0);
    float frameRight = graph.frameXHigh();
    graph.add(GraphItem(&histogram));
    CHECK(graph.items()[0].rank() == 2 && graph.items()[0].isMap());
    CHECK(graph.hasMaps() && graph.nAxes() == 3 && &graph.axis(2) == &graph.zAxis());
    CHECK(graph.frameXHigh() < frameRight);                                 // room for the color scale
    Graph::View v = graph.view();
    CHECK(v.hasZ && v.xMinimum == 0 && v.xMaximum == 4 && v.yMinimum == -1 && v.yMaximum == 1);   // exact extent of the map
    CHECK(v.zMinimum == 2 && v.zMaximum == 3);                              // the values of the bins (2 and 3)
    graph.setLogZ(true);
    v = graph.view();
    CHECK(v.logZ && v.zMinimum == 1 && v.zMaximum == 10);                   // whole decades
    graph.setLogZ(false);
    graph.setZRange(1.0, 2.5);
    v = graph.view();
    CHECK(!graph.isAutoZ() && graph.isAuto(0) && v.zMinimum == 1.0f && v.zMaximum == 2.5f);
    CHECK(graph.colorOf(v, 0.0) == graph.palette().color(0.0) && graph.colorOf(v, 9.0) == graph.palette().color(1.0));
    graph.setAutoRange();
    CHECK(graph.isAutoZ());
    graph.setXRange(3.0, 4.0);                                              // zoom: only the cells in view
    v = graph.view();
    CHECK(near(v.zMinimum, 1.8, 1E-5) && near(v.zMaximum, 2.2, 1E-5));     // the bin of 2 only, widened
    graph.setRange(2, 0.5, 5.0);
    graph.setLog(2, true);
    CHECK(graph.isLogZ() && !graph.isAuto(2) && graph.view().minimum(2) == 0.5f && graph.view().maximum(2) == 5.0f && graph.view().log(2));
  }
  {
    Graph graph;
    Saddle saddle;
    graph.add(GraphItem(&saddle, -1, 1, -1, 1));
    graph.setSymmetricZ(true);
    Graph::View v = graph.view();
    CHECK(v.xMinimum == -1 && v.xMaximum == 1 && v.zMinimum == -v.zMaximum && v.zMaximum >= 0.98f && v.zMaximum <= 1.0f);
    graph.setSymmetricZ(false);
    v = graph.view();
    CHECK(v.zMinimum < 0 && v.zMaximum > 0);
  }
  {
    // a function is evaluated again only when the visible range or the number of samples changes
    Graph graph;
    graph.add(GraphItem(&paraboloid, -2, 2, -2, 2));
    graph.items()[0].setMapSamples(10);
    paraboloid.nEvaluations = 0;
    graph.view();
    svgOf(graph);
    svgOf(graph);
    CHECK(paraboloid.nEvaluations == 100);
    graph.setXRange(0, 1);
    svgOf(graph);
    CHECK(paraboloid.nEvaluations == 200);                                  // resampled over the zoomed range
    Graph::View v = graph.view();
    CHECK(v.xMinimum == 0 && v.xMaximum == 1 && v.yMinimum == -2 && v.yMaximum == 2);
  }

  // ---- drawing
  {
    Graph graph;
    graph.add(GraphItem(&histogram));
    std::string svg = svgOf(graph);
    int cells = count(svg, "shape-rendering=\"crispEdges\"");
    CHECK(cells == 2 + 128);                                                // 2 bins with a value, 128 slices of the scale (no legend for a lone map)
    graph.setPalette(Palette(Palette::kViridis, 5));
    svg = svgOf(graph);
    CHECK(count(svg, "shape-rendering=\"crispEdges\"") == 2 + 5);         // 5 bands
    graph.setShowLegend(false);
    graph.setShowColorScale(false);
    svg = svgOf(graph);
    CHECK(count(svg, "shape-rendering=\"crispEdges\"") == 2);
    CHECK(graph.colorScaleWidth() == 0 && graph.zAxis().bodyRect().empty);
    graph.setShowColorScale(true);
    svg = svgOf(graph);
    CHECK(!graph.zAxis().bodyRect().empty && graph.zAxis().bodyRect().x0 > graph.frameXHigh());
    Graph::Pick pick = graph.pick(Point2D(0.5f*(graph.zAxis().bodyRect().x0 + graph.zAxis().bodyRect().x1), 0.5f), 0.01f, 0.01f);
    CHECK(pick.part == Graph::kAxis && pick.axis == 2);                     // the color scale is the z axis
    graph.setLogZ(true);
    graph.setZRange(2.5, 10.0);                                             // the bin of 2 is below the range: lowest color
    svg = svgOf(graph);
    Color lowest = graph.palette().color(0.0);
    char hex[16];
    std::snprintf(hex, sizeof(hex), "#%02x%02x%02x", int(255*lowest.red + 0.5f), int(255*lowest.green + 0.5f), int(255*lowest.blue + 0.5f));
    CHECK(count(svg, std::string("fill=\"") + hex + "\"") >= 2);           // the bin of 2 and the bottom of the scale
  }
  {
    Graph graph;
    graph.add(GraphItem(&scattered));
    std::string svg = svgOf(graph);
    CHECK(count(svg, "<polygon") >= 3);                                     // three colored disks
    CHECK(graph.pointPositions().size() == 3);
    Graph::View v = graph.view();
    CHECK(v.xMinimum == 0 && v.xMaximum == 3 && v.zMinimum == 0 && v.zMaximum == 5);
  }
  {
    // a curve over a map: the ranges cover both, rounded as for curves
    Graph graph;
    MATH::DataPointsVector curve({0.5, 1.5, 2.5}, {-0.5, 0.0, 0.5});
    graph.add(GraphItem(&histogram));
    graph.add(GraphItem(&curve));
    Graph::View v = graph.view();
    CHECK(v.hasZ && v.xMinimum <= 0 && v.xMaximum >= 4 && v.yMinimum <= -1 && v.yMaximum >= 1);
    std::string svg = svgOf(graph);
    CHECK(count(svg, "shape-rendering=\"crispEdges\"") == 2 + 128 + 3);   // with a curve: a legend, with 3 swatches for the map
    CHECK(graph.legendRect().empty == false);
  }

  // ---- contours
  {
    Grid2D grid = Grid2D::fromFunction(paraboloid, -2, 2, 40, -2, 2, 40);
    std::vector<std::vector<Grid2D::Point>> lines = grid.contour(1.0);
    CHECK(lines.size() == 1);                                               // one closed line: the unit circle
    if (lines.size() == 1)
      {
      const auto & line = lines[0];
      CHECK(line.size() > 20 && line.front().x == line.back().x && line.front().y == line.back().y);
      double worst = 0;
      for (const auto & q : line) worst = std::max(worst, std::fabs(std::sqrt(q.x*q.x + q.y*q.y) - 1.0));
      CHECK(worst < 0.02);
      }
    CHECK(grid.contour(100.0).empty() && grid.contour(-1.0).empty());       // outside the values
    CHECK(grid.contour(4.0).size() == 4);                                   // radius 2: beyond the centers but in the 4 corners
    Saddle saddle;
    Grid2D s = Grid2D::fromFunction(saddle, -1, 1, 21, -1, 1, 21);
    std::vector<std::vector<Grid2D::Point>> zero = s.contour(0.25);
    CHECK(zero.size() == 2);                                                // the two branches of x y = 0.25
    for (const auto & line : zero) for (const auto & q : line) check(std::fabs(q.x*q.y - 0.25) < 0.02, "x y = 0.25 on the contour", __LINE__);
    Grid2D holes = Grid2D::fromFunction(paraboloid, -2, 2, 20, -2, 2, 20);
    for (int j=8; j<12; j++) for (int i=0; i<20; i++) holes.setValue(i, j, std::nan(""));   // a band without values
    CHECK(holes.contour(2.5).size() == 2);                                  // the circle cut in two open arcs
    Grid2D logGrid = Grid2D::fromFunction(paraboloid, 1, 10, 20, 0, 0.001, 2);
    std::vector<std::vector<Grid2D::Point>> ten = logGrid.contour(10.0, true);
    CHECK(ten.size() == 1 && !ten[0].empty() && std::fabs(ten[0][0].x - std::sqrt(10.0)) < 0.1);   // interpolated in log10
  }
  {
    Graph graph;
    graph.add(GraphItem(&paraboloid, -2, 2, -2, 2));
    GraphItem & item = graph.items()[0];
    graph.setZRange(0, 8);
    item.setContourIntervals(4);
    std::vector<double> levels = graph.contourLevels(graph.view(), item);
    CHECK(levels.size() == 3 && levels[0] == 2 && levels[1] == 4 && levels[2] == 6);
    item.setContourLevels({5.0, 1.0});
    levels = graph.contourLevels(graph.view(), item);
    CHECK(levels.size() == 2 && levels[0] == 1 && levels[1] == 5);         // sorted
    item.setContourLevels({});
    graph.setLogZ(true);
    graph.setZRange(1, 100);
    item.setContourIntervals(2);
    levels = graph.contourLevels(graph.view(), item);
    CHECK(levels.size() == 1 && near(levels[0], 10.0, 1E-9));
    graph.setLogZ(false);
    graph.setAutoZ();
    item.setContourIntervals(4);
    item.setShowContours(true);
    item.setShowColorMap(false);
    std::string svg = svgOf(graph);
    CHECK(count(svg, "shape-rendering=\"crispEdges\"") == 128);           // no cells, only the color scale
    CHECK(count(svg, "<polyline") >= 3);                                    // the three circles
    item.setContourUsesPalette(true);
    svg = svgOf(graph);
    Color c = graph.colorOf(graph.view(), graph.contourLevels(graph.view(), item)[0]);
    char hex[16];
    std::snprintf(hex, sizeof(hex), "#%02x%02x%02x", int(255*c.red + 0.5f), int(255*c.green + 0.5f), int(255*c.blue + 0.5f));
    CHECK(svg.find(std::string("stroke=\"") + hex) != std::string::npos);   // the first circle in the color of its level
  }

  // ---- values in the cells, points of a map
  {
    Graph graph;
    graph.add(GraphItem(&histogram));
    GraphItem & item = graph.items()[0];
    item.setShowValues(true);
    std::string svg = svgOf(graph);
    CHECK(svg.find(">3</text>") != std::string::npos && svg.find(">2</text>") != std::string::npos);
    CHECK(svg.find("fill=\"#ffffff\"") != std::string::npos);           // white on the darkest cell (the 2)
    item.setValueFormat("%.1f");
    svg = svgOf(graph);
    CHECK(svg.find(">3.0</text>") != std::string::npos);
    item.setValueFormat("%s");                                              // not a number format: the default
    svg = svgOf(graph);
    CHECK(svg.find(">3</text>") != std::string::npos);
    item.setShowValues(false);
    item.setShowMapPoints(true);
    item.setPointOutline(false);
    svg = svgOf(graph);
    CHECK(count(svg, "<polygon") == 1 + 2);                                 // the frame, then a disk per bin with a value
    CHECK(graph.pointPositions().size() == 2);
    Graph::Pick pick = graph.pick(graph.pointPositions()[0].position, 0.01f, 0.01f);
    CHECK(pick.part == Graph::kDataPoint && histogram.valueAt(pick.index) > 0);   // the index of the bin
    item.setPointOutline(true);
    svg = svgOf(graph);
    CHECK(count(svg, "<polyline") >= 2);                                    // outlines
    item.setPointSizeByValue(true);
    svg = svgOf(graph);
    CHECK(count(svg, "<polygon") == 1 + 2);
  }

  // ---- an example page
  if (argc > 1)
    {
    Canvas canvas("c", "c", 1000, 1000);
    canvas.divide(2, 2);
    Graph & a = canvas.graph(0);
    MATH::Histogram gauss("gauss", "2D Gaussian", 40, -3.0, 3.0, 40, -3.0, 3.0);
    unsigned long seed = 12345;
    auto uniform = [&seed]() { seed = seed*6364136223846793005ULL + 1442695040888963407ULL; return ((seed >> 11) + 0.5)/9007199254740992.0; };
    for (int k=0; k<20000; k++)
      {
      double r = std::sqrt(-2*std::log(uniform())), phi = 2*3.14159265358979*uniform();
      double x = r*std::cos(phi), y = 0.6*x + 0.8*r*std::sin(phi);
      gauss.fill(x, y, 1.0);
      }
    a.add(GraphItem(&gauss));
    a.items()[0].setShowContours(true);
    a.items()[0].setContourIntervals(6);
    a.items()[0].contourLineAttributes().set(0, 0.8, 1, 1.0);   // white lines
    a.setTitle("Histogram of rank 2, contours");
    a.xAxis().setTitle("x"); a.yAxis().setTitle("y"); a.zAxis().setTitle("counts");
    Graph & b = canvas.graph(1);
    b.add(GraphItem(&paraboloid, -2, 2, -2, 2));
    b.items()[0].setShowContours(true);
    b.items()[0].setContourIntervals(8);
    b.setPalette(Palette(Palette::kBird, 8));
    b.setTitle("x^{2} + y^{2}, filled contours");
    b.xAxis().setTitle("x"); b.yAxis().setTitle("y");
    Graph & c = canvas.graph(2);
    MATH::Histogram small("small", "small", 6, 0.0, 6.0, 5, 0.0, 5.0);
    for (int jy=0; jy<5; jy++) for (int ix=0; ix<6; ix++) if ((ix + jy) % 4 != 3) small.fill(ix + 0.5, jy + 0.5, 0.1*(ix + 1)*(jy + 2));
    c.add(GraphItem(&small));
    c.items()[0].setShowValues(true);
    c.items()[0].setValueFormat("%.1f");
    c.setTitle("Values in the cells");
    c.xAxis().setTitle("column"); c.yAxis().setTitle("row");
    Graph & d = canvas.graph(3);
    MATH::DataPointsVector points;
    for (int k=0; k<60; k++) { double x = 4*uniform(), y = 4*uniform(); points.addPoint(MATH::DataPoint3D(x, y, x*y)); }
    MATH::DataPointsVector curve({0.0, 1.0, 2.0, 3.0, 4.0}, {0.5, 1.5, 2.0, 2.5, 3.5});
    d.add(GraphItem(&points, "scattered x y"));
    d.items()[0].setPointSizeByValue(true);
    d.items()[0].markerAttributes().setSize(2.0);
    d.add(GraphItem(&curve, "a curve"));
    d.setTitle("Scattered points, area by value");
    d.xAxis().setTitle("x"); d.yAxis().setTitle("y");
    SvgPainter svg(1000, 1000);
    canvas.draw(svg);
    svg.write(argv[1]);
    }

  // ---- fits attached to data points: curves over their ranges, and legend entries with their quality
  {
    MATH::DataPointsVector data;
    data.addPoints({0.0, 1.0, 2.0, 3.0, 4.0, 5.0}, {1.1, 2.9, 5.2, 6.8, 9.1, 11.3}, {0.1, 0.2, 0.1, 0.3, 0.2, 0.4});
    data.setName("data");
    MATH::Linear line(0.0, 1.0);
    MATH::Fitter fitter(line, data);
    fitter.fit();
    data.addFit(fitter.record("straight"));
    MATH::Parabolic parabola(0.0, 1.0, 0.0);
    MATH::Fitter second(parabola, data);
    second.fit();
    data.addFit(second.record("curved"));
    Graph graph;
    graph.add(GraphItem(&data));
    GraphItem & item = graph.items()[0];
    CHECK(item.nFits() == 2 && item.fit(1) && item.fit(1)->label == "curved" && item.fit(2) == nullptr);
    CHECK(graph.hasFits() && item.isFitShown(0) && item.isFitShown(7));
    CHECK(item.fitLineAttributes(0).color() == 2 && item.fitLineAttributes(1).color() == 4 && item.fitLineAttributes(8).color() == 2);
    std::string chi2 = "\xCF\x87\xC2\xB2/ndf";
    CHECK(GraphItem::fitSummary(*item.fit(0)).contains("/4") && std::string(GraphItem::fitSummary(*item.fit(0)).data()).find(chi2) == 0);
    std::string svg = svgOf(graph);
    int lines = count(svg, "<polyline");
    CHECK(count(svg, ">straight<") == 1 && count(svg, ">curved<") == 1 && count(svg, chi2) == 2);
    item.setFitShown(1, false);                                             // one fit hidden: its curve and entry go
    svg = svgOf(graph);
    CHECK(count(svg, ">curved<") == 0 && count(svg, chi2) == 1 && count(svg, "<polyline") < lines);
    item.setFitShown(1, true);
    graph.setShowFitStatistics(false);                                      // labels only
    svg = svgOf(graph);
    CHECK(count(svg, ">curved<") == 1 && count(svg, chi2) == 0);
    graph.setShowFits(false);                                               // no fit at all
    svg = svgOf(graph);
    CHECK(count(svg, ">straight<") == 0 && count(svg, "<polyline") < lines && !graph.hasFits());
    graph.setShowFits(true);
    graph.setShowFitStatistics(true);
    GRAPHICS::LineAttributes custom;
    custom.set(8, 1.0, 2, 3.0);
    item.setFitLineAttributes(1, custom);
    item.setExtendFits(true);
    Graph copy(graph);                                                      // styles and toggles are copied
    CHECK(copy.items()[0].fitLineAttributes(1).color() == 8 && copy.items()[0].fitLineAttributes(1).width() == 3.0f
          && copy.items()[0].fitLineAttributes(0).color() == 2 && copy.items()[0].extendFits() && copy.showFitStatistics());
    item.setShowFits(false);
    CHECK(!graph.hasFits() && count(svgOf(graph), chi2) == 0);
  }

  std::printf("TestGraph2D: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
