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
//! @file TestGraph3D.cpp
//! @brief Test program of the 3D views of graphs: projection, surfaces, meshes, lego boxes, axes (run by ctest).
//
// TestGraph3D -- tests of the 3D views of CAP::GRAPHICS::Graph (Projection3D, GraphRenderer::draw3D()), through the
// SVG they produce. Prints each failed check and exits with the number of failures (0: all passed). With an
// argument, also writes an example page to that SVG file.
//
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include "Canvas.hpp"
#include "DataPoint3D.hpp"
#include "DataPointsVector.hpp"
#include "Function.hpp"
#include "Graph.hpp"
#include "Histogram.hpp"
#include "FitRecord.hpp"
#include "Normal2D.hpp"
#include "Projection3D.hpp"
#include "SvgPainter.hpp"
#include "Painter.hpp"

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

int count(const std::string & text, const std::string & part)
{
  int n = 0;
  for (std::size_t at = text.find(part); at != std::string::npos; at = text.find(part, at+1)) n++;
  return n;
}

//! A function of two variables given by a lambda.
class Surface : public MATH::Function
{
public:
  Surface(const char * name, double (*f)(double, double)) : Function(2), _f(f) { setName(name); }
  double evaluateAt(const std::vector<double> & x) const override { checkRank(x.size(), __FUNCTION__); return _f(x[0], x[1]); }
  using Function::evaluateAt;
private:
  double (*_f)(double, double);
};

std::string svgOf(const Graph & graph)
{
  SvgPainter svg(600, 500);
  graph.draw(svg);
  return svg.document().str();
}
//! The color of a ROOT color index as written in SVG files ("#rrggbb").
std::string hexOf(int color)
{
  float r, g, b;
  Painter::rgb(color, r, g, b);
  char text[16];
  std::snprintf(text, sizeof(text), "#%02x%02x%02x", int(255*r + 0.5), int(255*g + 0.5), int(255*b + 0.5));
  return text;
}

} // namespace

//! @brief Count a check, and report it if the condition is false.
#define CHECK(condition) check((condition), #condition, __LINE__)

//! @brief Run the checks. @return number of failed checks (0: all passed)
int main(int argc, char ** argv)
{
  Surface paraboloid("paraboloid", [](double x, double y) { return x*x + y*y; });
  Surface bump("bump", [](double x, double y) { return std::exp(-x*x - y*y)*std::cos(2*x); });

  // ---- angles and the 3D mode
  {
    Graph graph;
    graph.setDimension(Graph::k3D);
    CHECK(!graph.is3D());                                                   // nothing to show in 3D yet
    graph.add(GraphItem(&paraboloid, -1, 1, -1, 1));
    CHECK(graph.is3D() && graph.azimuth() == 30 && graph.elevation() == 30);
    graph.setViewAngles(-30, 120);
    CHECK(graph.azimuth() == 330 && graph.elevation() == 90);
    graph.setViewAngles(400, -100);
    CHECK(graph.azimuth() == 40 && graph.elevation() == -90);
    graph.resetViewAngles();
    CHECK(graph.azimuth() == 30 && graph.elevation() == 30);
    CHECK(graph.items()[0].style3D() == GraphItem::kSurface);
    MATH::Histogram h("h", "h", 2, 0.0, 2.0, 2, 0.0, 2.0);
    CHECK(GraphItem(&h).style3D() == GraphItem::kLego);                     // histograms: lego boxes
  }

  // ---- projection
  {
    Graph graph;
    graph.add(GraphItem(&paraboloid, -1, 1, -1, 1));
    graph.setDimension(Graph::k3D);
    Graph::View view = graph.view();
    Projection3D p(graph, view, 600, 500);
    CHECK(p.toCanvas(0, 0, 0).x() < p.toCanvas(1, 0, 0).x());                // x runs to the right
    CHECK(p.toCanvas(0.5, 0.5, 1).y() > p.toCanvas(0.5, 0.5, 0).y());        // z upward
    double front = p.depth(1, 0, 0);
    CHECK(front < p.depth(0, 0, 0) && front < p.depth(1, 1, 0) && front < p.depth(0, 1, 0));   // (x max, y min) in front
    CHECK(p.depth(0.5, 0.5, 1) < p.depth(0.5, 0.5, 0));                     // seen from above: the top is nearer
    bool inside = true;
    for (int c=0; c<8; c++)
      {
      Point2D q = p.toCanvas(c & 1, (c >> 1) & 1, (c >> 2) & 1);
      inside = inside && graph.frameContains(q);
      }
    CHECK(inside);                                                          // the box fits in the frame
    CHECK(std::fabs(p.fraction(0, 0.0) - 0.5) < 1E-9 && std::fabs(p.fraction(2, view.zMaximum) - 1) < 1E-9);
    graph.setViewAngles(0, 90);                                             // from above: z has no effect
    Projection3D top(graph, graph.view(), 600, 500);
    Point2D a = top.toCanvas(0.3, 0.4, 0), b = top.toCanvas(0.3, 0.4, 1);
    CHECK(std::fabs(a.x() - b.x()) < 1E-5 && std::fabs(a.y() - b.y()) < 1E-5);
  }

  // ---- surfaces, meshes, lego boxes
  {
    Graph graph;
    graph.add(GraphItem(&paraboloid, -1, 1, -1, 1));
    graph.items()[0].setSurfaceSamples(10);
    graph.setDimension(Graph::k3D);
    std::string svg = svgOf(graph);
    CHECK(count(svg, "<polygon") == 3 + 9*9);                               // floor, 2 walls, the quads between 10 x 10 centers
    CHECK(count(svg, "shape-rendering=\"crispEdges\"") == 128);           // the color scale is kept
    for (int k=0; k<3; k++) check(!graph.axis(k).bodyRect().empty, "axis drawn", __LINE__);
    Rect2D xBody = graph.xAxis().bodyRect();
    Graph::Pick pick = graph.pick(Point2D(0.5f*(xBody.x0 + xBody.x1), 0.5f*(xBody.y0 + xBody.y1)), 0.005f, 0.005f);
    CHECK(pick.part == Graph::kAxis);
    graph.items()[0].setStyle3D(GraphItem::kMesh);
    svg = svgOf(graph);
    CHECK(count(svg, "<polygon") == 3 + 81 && count(svg, "fill=\"#ffffff\"") >= 81);   // white quads, hidden lines removed
    graph.items()[0].setShowSurfaceLines(false);
    graph.items()[0].setStyle3D(GraphItem::kSurface);
    int lines = count(svgOf(graph), "<polyline");
    graph.items()[0].setShowSurfaceLines(true);
    CHECK(count(svgOf(graph), "<polyline") >= lines + 81);                  // the lines of the cells
    graph.setDimension(Graph::k2D);
    CHECK(count(svgOf(graph), "shape-rendering=\"crispEdges\"") == 128 + 100*100);   // back to the color map
  }
  {
    MATH::Histogram h("h", "h", 3, 0.0, 3.0, 2, 0.0, 2.0);
    h.fill(0.5, 0.5, 2.0);
    h.fill(2.5, 1.5, 5.0);                                                  // 2 bins with a value, 4 empty
    Graph graph;
    graph.add(GraphItem(&h));
    graph.setDimension(Graph::k3D);
    std::string svg = svgOf(graph);
    CHECK(count(svg, "<polygon") == 3 + 2*3);                               // each box: top and 2 sides turned to the eye
    graph.setViewAngles(30, -30);                                           // from below: the bottoms instead of the tops
    CHECK(count(svgOf(graph), "<polygon") == 3 + 2*3);
    graph.setViewAngles(210, 30);                                           // from the other side: still 2 sides each
    CHECK(count(svgOf(graph), "<polygon") == 3 + 2*3);
    graph.resetViewAngles();
    graph.setZRange(0, 3);                                                  // the 5 is cut at the top of the box
    CHECK(count(svgOf(graph), "<polygon") == 3 + 2*3);
    graph.setZRange(3, 10);                                                 // the 2 is below the range: from the base 3, no box
    CHECK(count(svgOf(graph), "<polygon") == 3 + 1*3);
  }
  {
    MATH::DataPointsVector points;
    points.addPoint(MATH::DataPoint3D(0.0, 0.0, 1.0));
    points.addPoint(MATH::DataPoint3D(1.0, 2.0, 5.0));
    points.addPoint(MATH::DataPoint3D(3.0, 1.0, 3.0));
    MATH::DataPointsVector curve({0.0, 1.0}, {0.0, 1.0});
    Graph graph;
    graph.add(GraphItem(&points));
    graph.add(GraphItem(&curve));                                           // rank 1: not drawn in 3D
    graph.setDimension(Graph::k3D);
    std::string svg = svgOf(graph);
    CHECK(count(svg, "<polygon") == 3 + 3);                                 // the box panes, three disks
  }

  // ---- an example page
  if (argc > 1)
    {
    Canvas canvas("c", "c", 1000, 1000);
    canvas.divide(2, 2);
    MATH::Histogram gauss("gauss", "2D Gaussian", 20, -3.0, 3.0, 20, -3.0, 3.0);
    unsigned long seed = 12345;
    auto uniform = [&seed]() { seed = seed*6364136223846793005ULL + 1442695040888963407ULL; return ((seed >> 11) + 0.5)/9007199254740992.0; };
    for (int k=0; k<20000; k++)
      {
      double r = std::sqrt(-2*std::log(uniform())), phi = 2*3.14159265358979*uniform();
      double x = r*std::cos(phi), y = 0.6*x + 0.8*r*std::sin(phi);
      gauss.fill(x, y, 1.0);
      }
    Graph & a = canvas.graph(0);
    a.add(GraphItem(&gauss));
    a.setDimension(Graph::k3D);
    a.setTitle("Lego plot of a histogram");
    a.xAxis().setTitle("x"); a.yAxis().setTitle("y"); a.zAxis().setTitle("counts");
    Graph & b = canvas.graph(1);
    b.add(GraphItem(&bump, -2, 2, -2, 2));
    b.setDimension(Graph::k3D);
    b.setPalette(Palette(Palette::kDiverging));
    b.setSymmetricZ(true);
    b.setTitle("Surface: e^{-x^{2}-y^{2}} cos 2x");
    b.xAxis().setTitle("x"); b.yAxis().setTitle("y"); b.zAxis().setTitle("z");
    Graph & c = canvas.graph(2);
    c.add(GraphItem(&paraboloid, -1.5, 1.5, -1.5, 1.5));
    c.items()[0].setStyle3D(GraphItem::kMesh);
    c.items()[0].setSurfaceSamples(25);
    c.setDimension(Graph::k3D);
    c.setViewAngles(60, 25);
    c.setTitle("Mesh, azimuth 60, elevation 25");
    c.xAxis().setTitle("x"); c.yAxis().setTitle("y");
    Graph & d = canvas.graph(3);
    MATH::DataPointsVector points;
    for (int k=0; k<80; k++) { double x = 4*uniform(), y = 4*uniform(); points.addPoint(MATH::DataPoint3D(x, y, x*y)); }
    d.add(GraphItem(&points, "x y"));
    d.setDimension(Graph::k3D);
    d.setTitle("Scattered points in 3D");
    d.xAxis().setTitle("x"); d.yAxis().setTitle("y"); d.zAxis().setTitle("x y");
    SvgPainter svg(1000, 1000);
    canvas.draw(svg);
    svg.write(argv[1]);
    }

  // ---- a fit of rank 2 as a wire mesh in a 3D view
  {
    MATH::Normal2D shape(50.0, 0.0, 0.0, 1.0, 1.5, 0.3);
    MATH::Histogram h("h", "h", 20, -4.0, 4.0, 20, -4.0, 4.0);
    for (unsigned int i=0; i<20; i++)
      for (unsigned int j=0; j<20; j++) h.setBinAtGlobal(h.binIndex(i, j), 1, shape.evaluateAt(h.binCenter(i, 0), h.binCenter(j, 1)), 1.0);
    MATH::FitRecord fit;
    fit.label = "mesh";
    fit.function = std::shared_ptr<const MATH::ParameterizedFunction>(shape.clone());
    fit.low = { -3.0, -3.0 };
    fit.high = { 3.0, 3.0 };
    h.addFit(fit);
    Graph graph;
    graph.add(GraphItem(&h, "data"));
    graph.setDimension(Graph::k3D);
    std::string red = hexOf(graph.items()[0].fitLineAttributes(0).color());
    std::string svg = svgOf(graph);
    int lines = count(svg, "stroke=\"" + red + "\"");
    CHECK(graph.is3D() && lines > 100);                                     // cells of the mesh (some hidden by the surface)
    graph.items()[0].setShowFits(false);
    CHECK(count(svgOf(graph), "stroke=\"" + red + "\"") == 0);
    graph.items()[0].setShowFits(true);
    CHECK(lines < 29*29);                                                    // cells on the floor (fit near 0) are left out
    graph.items()[0].setExtendFits(true);                                   // over the whole view: another mesh
    int extended = count(svgOf(graph), "stroke=\"" + red + "\"");
    CHECK(extended > 50 && extended != lines);
  }

  std::printf("TestGraph3D: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
