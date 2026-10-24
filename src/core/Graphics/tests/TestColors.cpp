/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestColors.cpp
//! @brief Tests of the colors without ROOT (ColorTable: ROOT's numbers and values, new colors) and of the palettes
//! (presets, the current palettes of a session, the palettes of a graph).
#include <cmath>
#include <cstdio>
#include <string>
#include "ColorTable.hpp"
#include "Graph.hpp"
#include "Histogram.hpp"
#include "Palettes.hpp"

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

bool near(const Color & c, float r, float g, float b) { return std::fabs(c.red - r) < 1e-4f && std::fabs(c.green - g) < 1e-4f && std::fabs(c.blue - b) < 1e-4f; }
} // namespace

#define CHECK(condition) check((condition), #condition, __LINE__)

int main()
{
  // ---- the table: ROOT's numbers and values
  ColorTable & table = ColorTable::instance();
  CHECK(table.size() >= 446);
  CHECK(near(table.color(2), 1, 0, 0) && near(table.color(8), 0.35f, 0.83f, 0.33f));
  CHECK(near(table.color(634), 0.6f, 0, 0) && table.name(634) == "kRed+2");          // kRed+2
  CHECK(near(table.color(920), 0.8f, 0.8f, 0.8f) && table.name(920) == "kGray");
  CHECK(near(table.color(860), 0, 0.2f, 1) && near(table.color(12345), 0, 0, 0));     // unknown: black
  CHECK(table.index(Color(1, 0, 0)) == 2);                                              // an existing color: its number
  int k = table.index(Color(0.123f, 0.456f, 0.789f));                                    // a new one: from 1000
  CHECK(k >= 1000 && near(table.color(k), 0.123f, 0.456f, 0.789f) && table.index(Color(0.123f, 0.456f, 0.789f)) == k);
  CHECK(table.name(k) == "#1f74c9");
  table.set(5000, Color(1, 0.5f, 0), "my orange");
  CHECK(table.name(5000) == "my orange" && table.index(Color(1, 0.5f, 0)) != 0);

  // ---- palettes: presets, the current ones of the session
  Palettes & palettes = Palettes::session();
  CHECK(palettes.colorPalette("default") && palettes.colorPalette("okabe-ito") && palettes.colorPalette("fits"));
  CHECK(palettes.markerPalette("open") && palettes.shadingPalette("viridis") && palettes.shadingPalette("diverging"));
  CHECK(palettes.colorPalette().name() == "default" && palettes.colorPalette().color(1) == 2 && palettes.colorPalette().color(9) == 2);
  const ColorPalette * okabe = palettes.colorPalette("okabe-ito");
  CHECK(okabe->size() == 8 && near(table.color(okabe->color(1)), 230/255.0f, 159/255.0f, 0));   // orange E69F00
  CHECK(!palettes.setColorPalette("nothing") && palettes.colorPalette().name() == "default");
  palettes.add(ColorPalette("mine", { 616, 418 }));
  palettes.add(MarkerPalette("squares", { 21, 25 }));

  // ---- the palettes of graphs: the session's (changing it), or their own
  MATH::Histogram a("a", "a", 10, 0.0, 1.0), b("b", "b", 10, 0.0, 1.0);
  Graph g;
  g.show(GraphItem(&a, "a"), false);                                                    // show(): styled by the palettes
  g.show(GraphItem(&b, "b"), false);
  CHECK(g.items()[1].lineAttributes().color() == 2 && g.items()[1].markerAttributes().style() == 21);
  palettes.setColorPalette("okabe-ito");
  g.restyleItems();
  CHECK(g.items()[1].lineAttributes().color() == okabe->color(1));
  Graph own;
  own.setColorPaletteName("mine");
  own.setMarkerPaletteName("squares");
  own.show(GraphItem(&a, "a"), false);
  own.show(GraphItem(&b, "b"), false);
  CHECK(own.items()[1].lineAttributes().color() == 418 && own.items()[1].markerAttributes().style() == 25);
  Graph copy(own);
  CHECK(copy.colorPaletteName() == "mine");
  own.setColorPaletteName("unknown");                                                    // unknown: the session's
  CHECK(own.colorPalette().name() == "okabe-ito");
  CHECK(palettes.setShadingPalette("grayscale"));
  Graph shaded;
  CHECK(shaded.palette().type() == Palette::kGrayscale);                                 // new graphs: the session's
  palettes.setShadingPalette("viridis");
  palettes.setColorPalette("default");

  std::printf("TestColors: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
