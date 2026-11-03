/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
//! @file TestPlotSpec.cpp
//! @brief Tests of PlotSpec (canvases of histograms and functions from XML, written back) and of PptxPainter.
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <unistd.h>
#include "CapFile.hpp"
#include "ColorTable.hpp"
#include "FileSystem.hpp"
#include "Histogram.hpp"
#include "PlotSpec.hpp"
#include "PptxPainter.hpp"

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

const char * kSpec = R"XML(<?xml version="1.0"?>
<PlotHistos version="1">
  <Input id="data" file="input.cap"/>
  <Output directory="out" pdf="book.pdf" pptx="slides.pptx" title="Test"/>
  <Window title="First" width="1000" height="700" columns="2" rows="1">
    <Canvas name="spectra" title="Spectra" width="800" height="400" columns="2" rows="1" file="spectra.pdf">
      <Background color="#fafafa"/>
      <Graph title="Gaussian" file="gauss.svg" legend="false">
        <Margins left="0.2" bottom="0.18" right="0.04" top="0.1"/>
        <Axis which="x" title="x (cm)" min="-3" max="3"/>
        <Axis which="y" title="counts" log="true"/>
        <Histogram input="data" path="singles/gauss" label="data">
          <Marker color="kRed+1" style="21" size="1.5"/>
        </Histogram>
        <Function class="Normal" parameters="1000, 0, 1" xMin="-3" xMax="3" label="model" nSamples="200">
          <Line color="blue" style="2" width="2"/>
        </Function>
      </Graph>
      <Graph>
        <Histogram path="singles/flat" showMarkers="false" showLine="true"/>
      </Graph>
    </Canvas>
    <Canvas name="map" title="Map">
      <Graph><Histogram path="maps/xy" showContours="true"/></Graph>
    </Canvas>
  </Window>
  <Window title="Second">
    <Canvas name="missing">
      <Graph><Histogram path="singles/none"/><Function class="NoSuchFunction"/></Graph>
    </Canvas>
  </Window>
</PlotHistos>
)XML";
} // namespace

int main()
{
  char buffer[] = "/tmp/CapTestPlotSpec.XXXXXX";
  String top(mkdtemp(buffer));
  {
    CapFile out(FileSystem::join(top, "input.cap"), File::kWriteOnly);
    out.setFolderName("run");
    MATH::Histogram * gauss = new MATH::Histogram("gauss", "Gaussian", 60, -3.0, 3.0);
    MATH::Histogram * flat  = new MATH::Histogram("flat", "Flat", 10, 0.0, 1.0);
    MATH::Histogram * xy    = new MATH::Histogram("xy", "Map", 10, 0.0, 1.0, 10, 0.0, 1.0);
    for (int k=0; k<5000; k++)
      {
      double u1 = (k + 0.5)/5000.0, u2 = std::fmod(k*0.618034, 1.0);
      gauss->fill(std::sqrt(-2*std::log(u1))*std::cos(6.2831853*u2));
      flat->fill(u2);
      xy->fill(u1, u2, 1.0);
      }
    out.addFolder("singles")->add(gauss, "gauss");
    out.cd("singles"); out.add(flat, "flat"); out.cd("/");
    out.addFolder("maps")->add(xy, "xy");
    out.save();
  }
  { std::ofstream spec(FileSystem::join(top, "plots.xml").data()); spec << kSpec; }

  // reading and building
  PlotSpec spec;
  String error;
  CHECK(spec.read(FileSystem::join(top, "plots.xml"), error));
  CHECK(spec.windows().size() == 2);
  CHECK(spec.windows()[0].columns == 2 && spec.windows()[0].canvases.size() == 2);
  CHECK(spec.inputs().size() == 1 && spec.inputs()[0].id == "data");
  CHECK(spec.openInputs(error));
  String problems;
  std::vector<PlotSpec::WindowState> windows = spec.buildAll(problems);
  CHECK(windows.size() == 2 && windows[0].canvases.size() == 2);
  const Canvas & spectra = *windows[0].canvases[0];
  CHECK(spectra.width() == 800 && spectra.height() == 400 && spectra.nGraphs() == 2);
  const Graph & g = spectra.graph(0);
  CHECK(g.items().size() == 2);
  CHECK(g.title() == "Gaussian" && !g.showLegend());
  CHECK(std::fabs(g.margins().left() - 0.2f) < 1e-6);
  CHECK(!g.isAutoX() && g.isAutoY() && g.isLogY() && !g.isLogX());
  CHECK(g.xAxis().title() == "x (cm)");
  CHECK(g.items()[0].markerAttributes().style() == 21 && std::fabs(g.items()[0].markerAttributes().size() - 1.5f) < 1e-6);
  CHECK(g.items()[0].markerAttributes().color() == 633);   // kRed+1
  CHECK(g.items()[1].kind() == GraphItem::kFunction && g.items()[1].lineAttributes().style() == 2);
  CHECK(g.items()[1].nSamples() == 200 && g.items()[1].label() == "model");
  CHECK(spectra.graph(1).items().size() == 1 && !spectra.graph(1).items()[0].showMarkers());
  CHECK(windows[0].canvases[1]->graph(0).items().size() == 1 && windows[0].canvases[1]->graph(0).items()[0].isMap());
  CHECK(windows[0].canvases[1]->graph(0).items()[0].showContours());
  // what cannot be shown is reported, the rest is made
  CHECK(problems.str().find("singles/none") != std::string::npos);
  CHECK(problems.str().find("NoSuchFunction") != std::string::npos);
  CHECK(windows[1].canvases.size() == 1 && windows[1].canvases[0]->graph(0).items().empty());
  // colors
  CHECK(GraphXml::parseColor("2", 0) == 2 && GraphXml::parseColor("kRed+1", 0) == 633);
  CHECK(GraphXml::parseColor(GraphXml::colorName(GraphXml::parseColor("#336699", 0)), 0) == GraphXml::parseColor("#336699", 0));

  // written back, read again: the same document
  String rewritten = FileSystem::join(top, "rewritten.xml");
  CHECK(spec.write(rewritten, windows, error));
  PlotSpec again;
  CHECK(again.read(rewritten, error));
  CHECK(again.openInputs(error));
  String problems2;
  std::vector<PlotSpec::WindowState> windows2 = again.buildAll(problems2);
  String first = spec.toXml(windows).toString(), second = again.toXml(windows2).toString();
  CHECK(first == second);
  CHECK(windows2[0].canvases[0]->graph(0).items().size() == 2);

  // printing: the canvas and the graph with file names
  std::vector<const Canvas *> all;
  for (const auto & w : windows) for (const Canvas * c : w.canvases) all.push_back(c);
  std::vector<String> printed = spec.print(all);
  CHECK(printed.size() == 2);
  CHECK(FileSystem::isFile(spec.outputPath("spectra.pdf")) && FileSystem::isFile(spec.outputPath("gauss.svg")));
  // PDF book: one page per canvas
  CHECK(PlotSpec::writePdf(spec.outputPath("book.pdf"), all));
  {
  std::ifstream in(spec.outputPath("book.pdf").data());
  std::string pdf((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  CHECK(pdf.find("/Count 3") != std::string::npos);
  }
  // PowerPoint: one slide per canvas, a ZIP archive with the parts of a presentation
  PptxPainter pptx;
  for (const Canvas * c : all) pptx.addCanvas(*c);
  CHECK(pptx.nSlides() == 3);
  std::string bytes = pptx.package();
  CHECK(bytes.size() > 4 && bytes.compare(0, 4, "PK\x03\x04") == 0);
  CHECK(bytes.find("ppt/slides/slide3.xml") != std::string::npos);
  CHECK(bytes.find("<a:t>Gaussian</a:t>") != std::string::npos);
  CHECK(pptx.write(spec.outputPath("slides.pptx")));

  // texts of the slides: ROOT syntax and LaTeX as Unicode, with superscripts and subscripts
  {
  auto runs = PptxPainter::textRuns("#pi^{+}");
  CHECK(runs.size() == 2 && runs[0].text == "\xCF\x80" && runs[1].text == "+" && runs[1].script == 1);
  runs = PptxPainter::textRuns("p_{T} (GeV/c)");
  CHECK(runs.size() == 3 && runs[0].text == "p" && runs[1].text == "T" && runs[1].script == -1 && runs[2].text == " (GeV/c)");
  runs = PptxPainter::textRuns("$\\Delta\\varphi$");
  CHECK(runs.size() == 1 && runs[0].text == "\xCE\x94\xCF\x86");
  runs = PptxPainter::textRuns("plain text");
  CHECK(runs.size() == 1 && runs[0].text == "plain text");
  }

  std::printf("TestPlotSpec: %d checks, %d failures (files in %s)\n", nChecks, nFailures, top.data());
  return nFailures == 0 ? 0 : 1;
}
