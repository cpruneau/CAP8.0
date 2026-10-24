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
//! @file TestShapes.cpp
//! @brief Tests of the shapes drawn over graphs (outlines, picking, copies with the graph) and of the hatch patterns of
//! the painters. With a file name argument, also writes an example page there as SVG (and as PDF, with ".pdf"
//! appended), to look at.
#include <cmath>
#include <cstdio>
#include <string>
#include "Canvas.hpp"
#include "Histogram.hpp"
#include "PdfPainter.hpp"
#include "Shape.hpp"
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

Shape make(Shape::Kind kind, std::vector<Point2D> points, int fillColor, int fillStyle, float alpha=1.0f)
{
  Shape s(kind);
  s.setPoints(points);
  s.fillAttributes().set(fillColor, alpha, fillStyle);
  return s;
}
} // namespace

#define CHECK(condition) check((condition), #condition, __LINE__)

int main(int argc, char ** argv)
{
  // ---- outlines and picking
  {
    Shape r = make(Shape::kRectangle, { Point2D(0.2f, 0.2f), Point2D(0.4f, 0.6f) }, 2, FillAttributes::kSolid);
    CHECK(r.outline().size() == 4 && r.isValid());
    CHECK(r.contains(Point2D(0.3f, 0.4f), 0.01f, 0.01f));                 // filled: inside
    CHECK(!r.contains(Point2D(0.6f, 0.4f), 0.01f, 0.01f));
    r.fillAttributes().setStyle(FillAttributes::kHollow);
    CHECK(!r.contains(Point2D(0.3f, 0.4f), 0.01f, 0.01f));                // hollow: only near the outline
    CHECK(r.contains(Point2D(0.205f, 0.4f), 0.01f, 0.01f));
    Shape e = make(Shape::kEllipse, { Point2D(0.0f, 0.0f), Point2D(1.0f, 0.5f) }, 3, FillAttributes::kCrossHatch);
    CHECK(e.outline().size() == 72 && e.contains(Point2D(0.5f, 0.25f), 0.01f, 0.01f));
    Shape t = make(Shape::kTriangle, { Point2D(0.0f, 0.0f), Point2D(1.0f, 1.0f) }, 4, FillAttributes::kSolid);
    CHECK(t.outline().size() == 3 && std::fabs(t.outline()[2].x() - 0.5f) < 1e-6f);
    CHECK(!t.contains(Point2D(0.05f, 0.9f), 0.01f, 0.01f));               // outside, beside the apex
    Shape l(Shape::kLine);
    l.setPoints({ Point2D(0.1f, 0.1f), Point2D(0.9f, 0.1f) });
    CHECK(l.contains(Point2D(0.5f, 0.105f), 0.01f, 0.01f) && !l.contains(Point2D(0.5f, 0.2f), 0.01f, 0.01f));
    Shape p(Shape::kPolygon);
    p.setPoints({ Point2D(0.1f, 0.1f), Point2D(0.2f, 0.1f) });
    CHECK(!p.isValid());
    l.move(0.1f, 0.2f);
    CHECK(std::fabs(l.points()[0].x() - 0.2f) < 1e-6f && std::fabs(l.points()[1].y() - 0.3f) < 1e-6f);
  }

  // ---- shapes of a graph: in its cell, copied with it, picked from the top
  {
    Canvas canvas("c", "c", 600, 400);
    canvas.divide(2, 1);
    Graph & g = canvas.graph(1);                                          // right half of the canvas
    g.addShape(make(Shape::kRectangle, { Point2D(0.0f, 0.0f), Point2D(0.5f, 0.5f) }, 2, FillAttributes::kSolid));
    g.addShape(make(Shape::kEllipse, { Point2D(0.25f, 0.25f), Point2D(0.75f, 0.75f) }, 3, FillAttributes::kSolid));
    Point2D c = Shape::toCanvas(g, Point2D(0.5f, 0.5f));
    CHECK(std::fabs(c.x() - (g.cellXLow() + 0.5f*(g.cellXHigh() - g.cellXLow()))) < 1e-6f);
    Point2D back = Shape::fromCanvas(g, c);
    CHECK(std::fabs(back.x() - 0.5f) < 1e-6f && std::fabs(back.y() - 0.5f) < 1e-6f);
    CHECK(g.shapeAt(Shape::toCanvas(g, Point2D(0.4f, 0.4f)), 0.005f, 0.005f) == 1);   // both: the top one
    CHECK(g.shapeAt(Shape::toCanvas(g, Point2D(0.1f, 0.1f)), 0.005f, 0.005f) == 0);
    CHECK(g.shapeAt(Shape::toCanvas(g, Point2D(0.9f, 0.1f)), 0.005f, 0.005f) == -1);
    Graph copy(g);
    CHECK(copy.shapes().size() == 2);
    Canvas canvasCopy(canvas);
    CHECK(canvasCopy.graph(1).shapes().size() == 2);
  }

  // ---- hatch lines: inside the polygon, about 8 device units apart; drawn in SVG and PDF
  {
    SvgPainter svg(400, 400);
    std::vector<Point2D> square = { Point2D(0.25f, 0.25f), Point2D(0.75f, 0.25f), Point2D(0.75f, 0.75f), Point2D(0.25f, 0.75f) };
    std::vector<Point2D> lines = svg.hatchSegments(square, FillAttributes::kHatchHorizontal);
    CHECK(lines.size() == 2*25);                                          // y 100 to 300: lines at 104, 112, ..., 296
    bool within = true;
    for (const auto & q : lines) within = within && q.x() >= 0.25f - 1e-4f && q.x() <= 0.75f + 1e-4f && q.y() >= 0.25f && q.y() <= 0.75f;
    CHECK(within);
    std::vector<Point2D> diagonal = svg.hatchSegments(square, FillAttributes::kHatchForward);
    CHECK(diagonal.size() > 40 && diagonal.size() % 2 == 0);
    CHECK(svg.hatchSegments(square, FillAttributes::kSolid).empty());
    // a concave polygon (a U): the lines across the gap are cut in two
    std::vector<Point2D> u = { Point2D(0.1f, 0.1f), Point2D(0.9f, 0.1f), Point2D(0.9f, 0.9f), Point2D(0.7f, 0.9f),
                               Point2D(0.7f, 0.3f), Point2D(0.3f, 0.3f), Point2D(0.3f, 0.9f), Point2D(0.1f, 0.9f) };
    std::vector<Point2D> cut = svg.hatchSegments(u, FillAttributes::kHatchHorizontal);
    int across = 0;
    for (size_t k=0; k+1<cut.size(); k+=2) if (cut[k].y() > 0.5f && std::fabs(cut[k].x() - 0.1f) < 1e-3f) across += (cut[k+1].x() < 0.31f);
    CHECK(across > 10);
    Shape box = make(Shape::kRectangle, { Point2D(0.2f, 0.2f), Point2D(0.8f, 0.8f) }, 4, FillAttributes::kGrid);
    Canvas canvas("c", "c", 400, 400);
    canvas.divide(1, 1);
    canvas.graph(0).addShape(box);
    canvas.draw(svg);
    CHECK(svg.document().str().find("<polyline") != std::string::npos || svg.document().str().find("<path") != std::string::npos);
    PdfPainter pdf(400, 400);
    canvas.draw(pdf);
    CHECK(pdf.document().find(" l\n") != std::string::npos || pdf.document().find(" l ") != std::string::npos);
  }

  // ---- an example page
  if (argc > 1)
    {
    Canvas canvas("c", "c", 900, 600);
    canvas.divide(1, 1);
    MATH::Histogram h("h", "Shapes over a histogram", 30, 0.0, 3.0);
    for (int k=0; k<3000; k++) h.fill(1.5 + 0.5*std::sin(0.7*k)*std::cos(0.13*k), 1.0);
    Graph & g = canvas.graph(0);
    g.add(GraphItem(&h, "data"));
    g.addShape(make(Shape::kRectangle, { Point2D(0.15f, 0.55f), Point2D(0.35f, 0.8f) }, 2, FillAttributes::kHatchForward));
    g.addShape(make(Shape::kEllipse, { Point2D(0.4f, 0.55f), Point2D(0.55f, 0.8f) }, 4, FillAttributes::kSolid, 0.35f));
    g.addShape(make(Shape::kTriangle, { Point2D(0.6f, 0.55f), Point2D(0.75f, 0.8f) }, 8, FillAttributes::kCrossHatch));
    Shape poly = make(Shape::kPolygon, { Point2D(0.78f, 0.55f), Point2D(0.9f, 0.6f), Point2D(0.86f, 0.8f), Point2D(0.8f, 0.7f) },
                      6, FillAttributes::kGrid);
    poly.lineAttributes().set(6, 1.0f, 2, 1.5f);
    g.addShape(poly);
    Shape arrow(Shape::kLine);
    arrow.setPoints({ Point2D(0.2f, 0.4f), Point2D(0.45f, 0.25f) });
    arrow.setArrows(Shape::kSingleArrow);
    arrow.lineAttributes().set(2, 1.0f, 1, 3.0f);
    g.addShape(arrow);
    Shape both(Shape::kLine);
    both.setPoints({ Point2D(0.6f, 0.3f), Point2D(0.85f, 0.3f) });
    both.setArrows(Shape::kDoubleArrow);
    g.addShape(both);
    SvgPainter svg(900, 600);
    canvas.draw(svg);
    CHECK(svg.write(String(argv[1])));
    PdfPainter pdf(900, 600);
    canvas.draw(pdf);
    CHECK(pdf.write(String(argv[1]) + String(".pdf")));
    }

  std::printf("TestShapes: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
