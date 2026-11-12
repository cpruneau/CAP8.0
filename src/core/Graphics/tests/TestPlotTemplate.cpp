/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
//! @file TestPlotTemplate.cpp
//! @brief Tests of PlotTemplate: the catalog of a folder tree, the selection, and the PlotHistos file made of it
//! (read back by PlotSpec and built).
#include <cstdio>
#include <string>
#include <unistd.h>
#include "CapFile.hpp"
#include "FileSystem.hpp"
#include "Folder.hpp"
#include "Histogram.hpp"
#include "PlotSpec.hpp"
#include "PlotTemplate.hpp"

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
#define CHECK(condition) check((condition), #condition, __LINE__)

//! Folders as a pair analysis writes them: Pair/<combination>/<name>, singles and pairs
void fill(Folder & top, bool withPairs)
{
  Folder * pair = top.addFolder("Pair");
  for (const char * s : {"MB_PiP", "MB_PiM"})
    {
    Folder * f = pair->addFolder(s);
    f->add(new MATH::Histogram("n1_pt", "pt", 10, 0.0, 2.0), "n1_pt");
    f->add(new MATH::Histogram("n1_eta", "eta", 10, -1.0, 1.0), "n1_eta");
    }
  if (withPairs)
    for (const char * c : {"MB_PiP_PiP", "MB_PiP_PiM", "MB_PiM_PiM"})
      {
      Folder * f = pair->addFolder(c);
      f->add(new MATH::Histogram("R2_DyDphi", "R2", 8, -2.0, 2.0, 8, 0.0, 6.3), "R2_DyDphi");
      f->add(new MATH::Histogram("R2_ptpt", "R2", 8, 0.0, 2.0, 8, 0.0, 2.0), "R2_ptpt");
      }
  top.addFolder("Event")->add(new MATH::Histogram("mult", "mult", 10, 0.0, 100.0), "mult");
}

int count(const XmlElement & e, const char * name)
{
  int n = e.isNamed(name) ? 1 : 0;
  for (const XmlElement * c : e.children()) n += count(*c, name);
  return n;
}
} // namespace

int main()
{
  // matching
  CHECK(PlotTemplate::matches("R2_DyDphi", "R2*"));
  CHECK(PlotTemplate::matches("R2_DyDphi", "*Dy?phi"));
  CHECK(!PlotTemplate::matches("R2_DyDphi", "n1*"));
  CHECK(PlotTemplate::matches("Pair/MB_PiP/n1_pt", "Pair/*/n1_pt"));

  // catalog: a later input of the same paths takes them over (a later stage writes them again)
  Folder first("first"), second("second");
  fill(first, false);
  fill(second, true);
  PlotTemplate t;
  int a = t.addInput("fill", "fill.cap"), b = t.addInput("derived", "derived.cap");
  CHECK(t.addFolder(a, first) == 5);
  CHECK(t.addFolder(b, second) == 11);
  CHECK(t.entries().size() == 11);
  CHECK(t.tasks().size() == 2);
  CHECK(t.tasks()[0] == "Pair");
  CHECK(t.names("Pair").size() == 4);
  CHECK(t.combinations("Pair").size() == 5);
  CHECK(t.rankOf("Pair", "R2_DyDphi") == 2);
  CHECK(t.rankOf("Pair", "n1_pt") == 1);
  bool allSecond = true;
  for (const auto & e : t.entries()) allSecond = allSecond && e.input == b;
  CHECK(allSecond);
  const auto & e0 = t.entries()[0];
  CHECK(e0.task == "Pair" && e0.combination == "MB_PiP" && e0.name == "n1_pt");
  CHECK(t.entries()[4].task == "Event" && t.entries()[4].combination.isNull() && t.entries()[4].name == "mult");

  // selection
  CHECK(t.nSelected() == 11);
  t.select({"R2_*"}, {"*_ptpt"});
  CHECK(t.nSelected() == 3);
  CHECK(t.isNameSelected("Pair", "R2_DyDphi") && !t.isNameSelected("Pair", "R2_ptpt"));
  t.setCombinationSelected("Pair", "MB_PiM_PiM", false);
  CHECK(t.nSelected() == 2);
  t.selectAll("", true);
  CHECK(t.nSelected() == 11);
  t.selectAll("Event", false);
  CHECK(t.nSelected() == 10);

  // layout: a canvas per name, a graph per combination
  PlotTemplate::Options options;
  XmlDocument byName = t.makeSpec(options);
  CHECK(count(byName.root(), "Input") == 1);          // the first input plots nothing
  CHECK(count(byName.root(), "Canvas") == 4);
  CHECK(count(byName.root(), "Graph") == 10);
  CHECK(count(byName.root(), "Histogram") == 10);
  // a canvas per combination, a graph per name
  options.grouping = PlotTemplate::Options::kByCombination;
  XmlDocument byCombination = t.makeSpec(options);
  CHECK(count(byCombination.root(), "Canvas") == 5);
  CHECK(count(byCombination.root(), "Graph") == 10);
  // overlays of the 1D combinations; graphs split across canvases, canvases across windows
  options.grouping = PlotTemplate::Options::kByHistogram;
  options.overlay = true;
  XmlDocument overlaid = t.makeSpec(options);
  CHECK(count(overlaid.root(), "Graph") == 2 + 3 + 3);
  options.overlay = false;
  options.maxGraphs = 2;
  options.maxCanvases = 3;
  XmlDocument split = t.makeSpec(options);
  CHECK(count(split.root(), "Canvas") == 6);
  CHECK(count(split.root(), "Window") == 2);

  // the file written is read by PlotSpec and its canvases are built from a CAP file
  char buffer[] = "/tmp/CapTestPlotTemplate.XXXXXX";
  String top(mkdtemp(buffer));
  {
    CapFile out(FileSystem::join(top, "derived.cap"), File::kWriteOnly);
    fill(out.folder(), true);
    out.save();
  }
  t.setInputFile(b, FileSystem::join(top, "derived.cap"));
  options = PlotTemplate::Options();
  String path = FileSystem::join(top, "plots.xml");
  t.makeSpec(options).write(path);
  PlotSpec spec;
  String error, problems;
  CHECK(spec.read(path, error));
  CHECK(spec.openInputs(error));
  std::vector<PlotSpec::WindowState> windows = spec.buildAll(problems);
  CHECK(problems.isNull());
  int nCanvases = 0;
  for (const auto & w : windows) nCanvases += int(w.canvases.size());
  CHECK(nCanvases == 4);

  // catalog of a CAP file read lazily
  {
    CapFile in(FileSystem::join(top, "derived.cap"), File::kReadOnly);
    in.load(File::Loading::kLazy);
    PlotTemplate u;
    CHECK(u.addFolder(u.addInput("d", "derived.cap"), in.folder()) == 11);
    CHECK(u.rankOf("Pair", "R2_ptpt") == 2);
  }

  std::printf("TestPlotTemplate: %d checks, %d failures\n", nChecks, nFailures);
  return nFailures == 0 ? 0 : 1;
}
