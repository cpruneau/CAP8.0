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
//! @file TestPdfPainter.cpp
//! @brief Tests of PdfPainter: structure of the file (objects, cross-reference table, fonts, graphics states), text
//! encoding and widths, drawing of a canvas. With a file name argument, also writes an example page there (and the
//! same page as SVG, with ".svg" appended), to look at.
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include "Canvas.hpp"
#include "DataPoint3D.hpp"
#include "DataPointsVector.hpp"
#include "Histogram.hpp"
#include "PdfPainter.hpp"
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

//! Whether each entry of the cross-reference table points at "k 0 obj".
bool crossReferencesValid(const std::string & pdf)
{
  size_t xref = pdf.rfind("startxref\n");
  if (xref == std::string::npos) return false;
  size_t table = std::strtoul(pdf.c_str() + xref + 10, nullptr, 10);
  if (pdf.compare(table, 5, "xref\n") != 0) return false;
  size_t at = table + 5;
  unsigned first = 0, count = 0;
  if (std::sscanf(pdf.c_str() + at, "%u %u", &first, &count) != 2 || count < 2) return false;
  at = pdf.find('\n', at) + 1 + 20;                       // after the entry of object 0
  for (unsigned k=1; k<count; k++, at+=20)
    {
    size_t offset = std::strtoul(pdf.c_str() + at, nullptr, 10);
    std::string expected = std::to_string(k) + " 0 obj";
    if (pdf.compare(offset, expected.size(), expected) != 0) return false;
    }
  return true;
}
} // namespace

#define CHECK(condition) check((condition), #condition, __LINE__)

int main(int argc, char ** argv)
{
  // ---- encoding and widths of the standard fonts
  {
    CHECK(PdfPainter::winAnsi("abc") == "abc");
    CHECK(PdfPainter::winAnsi("\xC3\xA9t\xC3\xA9") == "\xE9t\xE9");               // UTF-8 -> Latin-1
    CHECK(PdfPainter::winAnsi("\xCE\xB1") == "?");                                // alpha: not in WinAnsi
    CHECK(std::fabs(PdfPainter::standardTextWidth("A", 0, 10) - 6.67f) < 1e-3);   // Helvetica A: 667
    CHECK(std::fabs(PdfPainter::standardTextWidth("ii", 8, 10) - 12.0f) < 1e-3);  // Courier: 600
    CHECK(PdfPainter::fontIndex(42) == 0 && PdfPainter::fontIndex(62) == 1 && PdfPainter::fontIndex(132) == 4);
  }

  // ---- a canvas drawn to PDF: a valid file, its fonts and graphics states
  {
    MATH::Histogram h("h", "a histogram", 20, 0.0, 10.0);
    for (int k=0; k<200; k++) h.fill(5.0 + 2.0*std::sin(0.37*k), 1.0);
    Canvas canvas("c", "c", 600, 400);
    canvas.divide(1, 1);
    Graph & graph = canvas.graph(0);
    graph.add(GraphItem(&h, "histogram (plain text)"));
    graph.setTitle("Plain title");
    graph.xAxis().setTitle("x");
    PdfPainter pdf(600, 400);
    canvas.draw(pdf);
    std::string doc = pdf.document();
    CHECK(doc.compare(0, 8, "%PDF-1.4") == 0);
    CHECK(doc.size() > 1000 && doc.compare(doc.size() - 6, 6, "%%EOF\n") == 0);
    CHECK(crossReferencesValid(doc));
    CHECK(doc.find("/MediaBox [0 0 600 400]") != std::string::npos);
    CHECK(doc.find("/BaseFont /Helvetica") != std::string::npos);
    CHECK(doc.find(" Tf ") != std::string::npos && doc.find("(Plain title) Tj") != std::string::npos);
    CHECK(doc.find("/Type /ExtGState") != std::string::npos);
    size_t length = doc.find("/Length ");
    size_t stream = doc.find("stream\n", length) + 7, end = doc.find("endstream", stream);
    CHECK(std::strtoul(doc.c_str() + length + 8, nullptr, 10) == end - stream);
  }

  // ---- an example page, as PDF and SVG
  if (argc > 1)
    {
    Canvas canvas("c", "c", 1000, 800);
    canvas.divide(2, 2);
    unsigned long seed = 12345;
    auto uniform = [&seed]() { seed = seed*6364136223846793005ULL + 1442695040888963407ULL; return ((seed >> 11) + 0.5)/9007199254740992.0; };
    MATH::Histogram gauss("gauss", "2D Gaussian", 40, -3.0, 3.0, 40, -3.0, 3.0);
    for (int k=0; k<20000; k++)
      {
      double r = std::sqrt(-2*std::log(uniform())), phi = 2*3.14159265358979*uniform();
      double x = r*std::cos(phi), y = 0.6*x + 0.8*r*std::sin(phi);
      gauss.fill(x, y, 1.0);
      }
    Graph & a = canvas.graph(0);
    a.add(GraphItem(&gauss));
    a.items()[0].setShowContours(true);
    a.items()[0].setContourIntervals(6);
    a.setTitle("Map with contours");
    a.xAxis().setTitle("#Delta#eta"); a.yAxis().setTitle("#Delta#varphi");
    MATH::Histogram pt("pt", "pT", 30, 0.0, 3.0);
    for (int k=0; k<5000; k++) pt.fill(-0.5*std::log(uniform()), 1.0);
    Graph & b = canvas.graph(1);
    b.add(GraphItem(&pt, "p_{T} spectrum"));
    b.setTitle("Spectrum");
    b.xAxis().setTitle("p_{T} (GeV/c)"); b.yAxis().setTitle("counts");
    MATH::Histogram small("small", "small", 6, 0.0, 6.0, 5, 0.0, 5.0);
    for (int jy=0; jy<5; jy++) for (int ix=0; ix<6; ix++) small.fill(ix + 0.5, jy + 0.5, 0.1*(ix + 1)*(jy + 2));
    Graph & c = canvas.graph(2);
    c.add(GraphItem(&small));
    c.items()[0].setShowValues(true);
    c.items()[0].setValueFormat("%.1f");
    c.setTitle("Values in the cells (\xC3\xA9t\xC3\xA9)");
    MATH::DataPointsVector points;
    for (int k=0; k<60; k++) { double x = 4*uniform(), y = 4*uniform(); points.addPoint(MATH::DataPoint3D(x, y, x*y)); }
    MATH::DataPointsVector curve({0.0, 1.0, 2.0, 3.0, 4.0}, {0.5, 1.5, 2.0, 2.5, 3.5});
    Graph & d = canvas.graph(3);
    d.add(GraphItem(&points, "scattered x y"));
    d.items()[0].setPointSizeByValue(true);
    d.items()[0].markerAttributes().setSize(2.0);
    d.add(GraphItem(&curve, "a curve"));
    d.setTitle("Markers and a curve");
    PdfPainter pdf(1000, 800);
    canvas.draw(pdf);
    CHECK(pdf.write(argv[1]));
    SvgPainter svg(1000, 800);
    canvas.draw(svg);
    CHECK(svg.write(String(argv[1]) + String(".svg")));
    }

  std::printf("TestPdfPainter: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
