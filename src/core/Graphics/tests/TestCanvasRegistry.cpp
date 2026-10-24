/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestCanvasRegistry.cpp
//! @brief Tests of the registry of canvases: listing (unique names, groups, copies not listed, canvases removing
//! themselves), access by index, name and group, iteration, and the output of all the canvases (files, a book).
#include <cstdio>
#include <memory>
#include <string>
#include <unistd.h>
#include "CanvasRegistry.hpp"
#include "Directory.hpp"
#include "Histogram.hpp"
#include "PdfPainter.hpp"

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

int count(const std::string & text, const std::string & what)
{
  int n = 0;
  for (std::size_t at = text.find(what); at != std::string::npos; at = text.find(what, at + 1)) n++;
  return n;
}
} // namespace

#define CHECK(condition) check((condition), #condition, __LINE__)

int main()
{
  MATH::Histogram h("h", "h", 20, 0.0, 1.0);
  for (int k=0; k<200; k++) h.fill(0.005*k, 1.0);
  CanvasRegistry registry;
  Canvas a("spectra", "Spectra", 600, 400), b("spectra", "Again", 500, 500), c("maps", "Maps", 800, 600);
  a.divide(2, 1); b.divide(1, 1); c.divide(1, 1);
  a.graph(0).show(GraphItem(&h, "h"), false);
  b.graph(0).show(GraphItem(&h, "h"), false);
  c.graph(0).show(GraphItem(&h, "h"), false);

  // ---- listing: unique names, groups
  CHECK(registry.add(a, "", "Window 1") == "spectra");
  CHECK(registry.add(b, "", "Window 1") == "spectra_2");          // name taken
  CHECK(registry.add(c, "map of pt", "Window 2") == "map of pt");
  CHECK(registry.size() == 3 && a.registry() == &registry && registry.contains(b));
  CHECK(registry.find("spectra_2") == &b && registry.find("nothing") == nullptr);
  CHECK(&registry.canvas(2) == &c && registry.name(0) == "spectra" && registry.group(2) == "Window 2");
  CHECK(registry.groups().size() == 2 && registry.canvases("Window 1").size() == 2 && registry.canvases().size() == 3);
  CHECK(registry.rename(b, "spectra") == "spectra_2" && registry.rename(b, "other") == "other");
  registry.setGroup(c, "Window 1");
  CHECK(registry.groups().size() == 1);
  int n = 0;
  for (Canvas & canvas : registry) n += canvas.nGraphs();
  CHECK(n == 4);

  // ---- copies are not listed; a canvas destroyed is removed
  {
    Canvas copy(a);
    CHECK(copy.registry() == nullptr && registry.size() == 3);
    auto temporary = std::make_unique<Canvas>("temporary", "t");
    registry.add(*temporary);
    CHECK(registry.size() == 4);
    temporary.reset();
    CHECK(registry.size() == 3 && registry.find("temporary") == nullptr);
  }
  // a canvas moved to another registry leaves the first one
  {
    CanvasRegistry other;
    other.add(c, "c");
    CHECK(!registry.contains(c) && other.contains(c) && c.registry() == &other);
    registry.add(c, "map of pt", "Window 2");
    CHECK(!other.contains(c) && registry.contains(c));
  }

  // ---- output: files per canvas or per graph, one book of pages
  char buffer[] = "/tmp/CapTestCanvasRegistry.XXXXXX";
  Directory top{String(mkdtemp(buffer))};
  std::vector<String> files = registry.saveAll(top.join("figures"), ".pdf");
  CHECK(files.size() == 3 && top.contains("figures/spectra.pdf") && top.contains("figures/map_of_pt.pdf"));
  files = registry.saveAll(top.join("graphs"), ".svg", true, "Window 1");
  CHECK(files.size() == 3 && top.contains("graphs/spectra_1.svg") && top.contains("graphs/spectra_2.svg") && top.contains("graphs/other_1.svg"));
  CHECK(registry.writeBook(top.join("book.pdf")));
  {
    PdfPainter pdf(100, 100);
    pdf.newPage(200, 50);
    CHECK(pdf.nPages() == 2);
    std::string doc = pdf.document();
    CHECK(count(doc, "/Type /Page ") == 2 && doc.find("/Count 2") != std::string::npos && doc.find("/MediaBox [0 0 200 50]") != std::string::npos);
  }
  std::FILE * f = std::fopen(top.join("book.pdf").data(), "rb");
  std::string book;
  for (int ch; f && (ch = std::fgetc(f)) != EOF;) book += char(ch);
  if (f) std::fclose(f);
  CHECK(count(book, "/Type /Page ") == 3 && book.find("/Count 3") != std::string::npos);
  top.remove(true);

  registry.remove(a);
  CHECK(registry.size() == 2 && a.registry() == nullptr);

  std::printf("TestCanvasRegistry: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
