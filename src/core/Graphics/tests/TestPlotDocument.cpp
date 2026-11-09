/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
//! @file TestPlotDocument.cpp
//! @brief Tests of PlotDocument: data dropped where they go, moves, copy and paste, undo and redo, inputs, files
//! written elsewhere, and the document read by PlotSpec and built.
#include <cstdio>
#include <set>
#include <string>
#include <unistd.h>
#include "CapFile.hpp"
#include "FileSystem.hpp"
#include "Histogram.hpp"
#include "PlotDocument.hpp"
#include "PlotSpec.hpp"

using namespace CAP;
using namespace CAP::GRAPHICS;
using Path = PlotDocument::Path;

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

int count(const XmlElement & e, const char * name)
{
  int n = e.isNamed(name) ? 1 : 0;
  for (const XmlElement * c : e.children()) n += count(*c, name);
  return n;
}
} // namespace

int main()
{
  char buffer[] = "/tmp/CapTestPlotDocument.XXXXXX";
  String top(mkdtemp(buffer));
  String data = FileSystem::join(top, "data.cap");
  {
    CapFile out(data, File::kWriteOnly);
    Folder * f = out.folder().addFolder("Pair")->addFolder("MB_PiP");
    f->add(new MATH::Histogram("n1_pt", "pt", 10, 0.0, 2.0), "n1_pt");
    f->add(new MATH::Histogram("n1_eta", "eta", 10, -1.0, 1.0), "n1_eta");
    f->add(new MATH::Histogram("n2_ptpt", "ptpt", 8, 0.0, 2.0, 8, 0.0, 2.0), "n2_ptpt");
    out.save();
  }

  PlotDocument d;
  CHECK(d.root().isNamed("PlotHistos"));
  CHECK(d.displayName() == "Untitled");
  CHECK(!d.canUndo());

  // dropped on the top: a window, a canvas, a graph, the histogram, and the input
  Path canvas = d.addData(Path(), data, {"Pair/MB_PiP/n1_pt"});
  CHECK(d.kind(canvas) == PlotDocument::kCanvas);
  CHECK(count(d.root(), "Input") == 1 && count(d.root(), "Window") == 1 && count(d.root(), "Graph") == 1);
  CHECK(d.root().child(0).isNamed("Input") && d.root().child(0).attribute("id") == "data");
  CHECK(d.isDirty());
  CHECK(d.canUndo());
  // one step of undo for all of it
  CHECK(d.undo());
  CHECK(count(d.root(), "Input") == 0 && count(d.root(), "Window") == 0);
  CHECK(d.redo());
  CHECK(count(d.root(), "Histogram") == 1);

  // dropped on the graph: overlaid; on the histogram: after it; on the canvas: a new graph
  std::vector<Path> graphs = d.children(canvas, PlotDocument::kGraph);
  CHECK(graphs.size() == 1);
  Path h2 = d.addData(graphs[0], data, {"Pair/MB_PiP/n1_eta"});
  CHECK(d.kind(h2) == PlotDocument::kHistogram);
  CHECK(d.element(graphs[0])->children("Histogram").size() == 2);
  Path first = d.children(graphs[0], PlotDocument::kHistogram)[0];
  Path after = d.addData(first, data, {"Pair/MB_PiP/n2_ptpt"});
  CHECK(after.back() == first.back() + 1);
  CHECK(d.element(after)->attribute("path") == "Pair/MB_PiP/n2_ptpt");
  Path g2 = d.addData(canvas, data, {"Pair/MB_PiP/n1_eta"});
  CHECK(d.kind(g2) == PlotDocument::kGraph);
  CHECK(d.element(canvas)->children("Graph").size() == 2);
  CHECK(count(d.root(), "Input") == 1);   // the same input
  // a folder dropped on the window: one canvas, a graph per object
  Path window = d.children(Path(), PlotDocument::kWindow)[0];
  Path c2 = d.addData(window, data, {"Pair/MB_PiP/n1_pt", "Pair/MB_PiP/n1_eta"});
  CHECK(d.kind(c2) == PlotDocument::kCanvas && d.element(c2)->children("Graph").size() == 2);
  CHECK(d.element(c2)->attribute("name") == "MB_PiP");

  // locating for the preview
  int w = -1, c = -1;
  CHECK(d.locate(h2, w, c) && w == 0 && c == 0);
  CHECK(d.locate(c2, w, c) && w == 0 && c == 1);
  CHECK(!d.locate(Path{0}, w, c));

  // style of an item: a Marker child made, then removed when empty
  CHECK(d.setStyle(h2, "Marker", "color", "kRed+1"));
  CHECK(d.style(h2, "Marker", "color") == "kRed+1");
  CHECK(d.element(h2)->child("Marker") != nullptr);
  CHECK(d.setStyle(h2, "Marker", "color", ""));
  CHECK(d.element(h2)->child("Marker") == nullptr);
  CHECK(d.setStyle(graphs[0], "Axis", "log", "true", "y"));
  CHECK(d.style(graphs[0], "Axis", "log", "y") == "true" && d.style(graphs[0], "Axis", "log", "x").isNull());
  CHECK(d.element(graphs[0])->child(0).isNamed("Axis"));    // style before the items
  CHECK(!d.setAttribute(canvas, "title", d.element(canvas)->attribute("title")));   // nothing changed
  CHECK(d.setAttribute(canvas, "columns", "2"));

  // moves: a histogram to the other graph; not into itself
  graphs = d.children(canvas, PlotDocument::kGraph);
  Path h = d.children(graphs[0], PlotDocument::kHistogram)[0];
  bool ok = false;
  Path moved = d.move(h, graphs[1], ok);
  CHECK(ok && d.kind(moved) == PlotDocument::kHistogram && moved.size() == graphs[1].size() + 1);
  CHECK(d.element(graphs[0])->children("Histogram").size() == 2 && d.element(graphs[1])->children("Histogram").size() == 2);
  d.move(canvas, h, ok);
  CHECK(!ok);
  // shift
  Path second = d.children(window, PlotDocument::kCanvas)[1];
  Path up = d.shift(second, -1);
  CHECK(up.back() == second.back() - 1 && d.element(up)->attribute("name") == "MB_PiP");
  d.undo();

  // copy and paste: a canvas pasted on a graph goes into the window, renamed
  String text = d.copy(canvas);
  Path pasted = d.paste(text, graphs[0], ok);
  CHECK(ok && d.kind(pasted) == PlotDocument::kCanvas);
  CHECK(!(d.element(pasted)->attribute("name") == d.element(canvas)->attribute("name")));
  Path hp = d.paste(d.copy(moved), graphs[0], ok);
  CHECK(ok && d.kind(hp) == PlotDocument::kHistogram && PlotDocument::parent(hp) == graphs[0]);
  d.paste("<Window title=\"pasted\"/>", hp, ok);
  CHECK(ok && count(d.root(), "Window") == 2);
  d.paste("not xml", hp, ok);
  CHECK(!ok);
  Path dup = d.duplicate(canvas);
  CHECK(dup.back() == canvas.back() + 1 && !(d.element(dup)->attribute("name") == d.element(canvas)->attribute("name")));
  CHECK(d.remove(dup));
  // a window duplicated: its canvases under names of their own
  Path canvasWindow = PlotDocument::parent(canvas);
  Path windowCopy = d.duplicate(canvasWindow);
  CHECK(d.kind(windowCopy) == PlotDocument::kWindow && windowCopy.back() == canvasWindow.back() + 1);
  {
  std::vector<Path> before = d.children(canvasWindow, PlotDocument::kCanvas), after = d.children(windowCopy, PlotDocument::kCanvas);
  CHECK(!before.empty() && before.size() == after.size());
  std::set<std::string> names;
  for (const Path & c : before) names.insert(d.element(c)->attribute("name").str());
  for (const Path & c : after)  names.insert(d.element(c)->attribute("name").str());
  CHECK(names.size() == before.size() + after.size());
  }
  CHECK(d.remove(windowCopy));

  // labels
  CHECK(PlotDocument::label(*d.element(canvas)).str().find("(2 x 1)") != std::string::npos);
  Path anyItem = d.children(d.children(canvas, PlotDocument::kGraph)[1], PlotDocument::kHistogram)[0];
  CHECK(PlotDocument::label(*d.element(anyItem)).str().find("data:Pair/MB_PiP/") != std::string::npos);

  // written: relative input path; written elsewhere: relocated; read back by PlotSpec and built
  String error;
  FileSystem::createDirectory(FileSystem::join(top, "plots"));
  CHECK(d.write(FileSystem::join(top, "plots/one.xml"), error));
  CHECK(!d.isDirty() && d.displayName() == "one.xml");
  CHECK(d.root().child(0).attribute("file") == "../data.cap");
  CHECK(d.write(FileSystem::join(top, "two.xml"), error));
  CHECK(d.root().child(0).attribute("file") == "data.cap");
  CHECK(d.root().child(1).attribute("directory") == "plots/figures");
  PlotDocument again;
  CHECK(again.read(FileSystem::join(top, "two.xml"), error));
  CHECK(again.root().toString() == d.root().toString());
  CHECK(!again.read(data, error));
  // the comments before the root are kept
  {
    String commented = FileSystem::join(top, "commented.xml");
    std::FILE * f = std::fopen(commented.data(), "w");
    std::fputs("<?xml version=\"1.0\"?>\n<!-- notes of the author -->\n<PlotHistos version=\"1\"><Window title=\"w\"/></PlotHistos>\n", f);
    std::fclose(f);
    PlotDocument c;
    CHECK(c.read(commented, error));
    c.setAttribute(Path{0}, "title", "v");
    CHECK(c.write(commented, error));
    PlotDocument c2;
    CHECK(c2.read(commented, error) && c2.root().child(0).attribute("title") == "v");
    std::FILE * g = std::fopen(commented.data(), "r");
    char line[256] = {0};
    std::fgets(line, sizeof(line), g);
    std::fgets(line, sizeof(line), g);
    std::fclose(g);
    CHECK(std::string(line).find("notes of the author") != std::string::npos);
  }

  PlotSpec spec;
  CHECK(spec.read(FileSystem::join(top, "two.xml"), error));
  CHECK(spec.openInputs(error));
  String problems;
  std::vector<PlotSpec::WindowState> windows = spec.buildAll(problems);
  CHECK(problems.isNull());
  if (!problems.isNull()) std::printf("%s", problems.data());
  CHECK(windows.size() == 2 && windows[0].canvases.size() == 3);

  // a graph edited as an object: only what differs from the defaults is written, the items are kept
  {
    XmlElement element("Graph");
    element.setAttribute("title", "pt");
    element.addChild("Axis").setAttribute("which", "y").setAttribute("log", "true");
    element.addChild("Histogram").setAttribute("path", "a/b");
    Graph graph;
    GraphXml::readGraph(element, graph);
    graph.setShowLegend(!graph.showLegend());
    graph.axis(0).setTitle("p_{T}");
    XmlElement full("Graph"), defaults("Graph");
    GraphXml::writeGraph(full, graph);
    GraphXml::writeGraph(defaults, Graph());
    XmlElement merged = element;
    PlotDocument::mergeDifferences(merged, full, defaults);
    CHECK(merged.attribute("title") == "pt");
    CHECK(merged.hasAttribute("legend"));
    CHECK(merged.child("Histogram") != nullptr);
    CHECK(merged.findFirst("Axis", "which", "x") && merged.findFirst("Axis", "which", "x")->attribute("title") == "p_{T}");
    CHECK(merged.findFirst("Axis", "which", "y") && merged.findFirst("Axis", "which", "y")->attribute("log") == "true");
    int nAttributes = int(merged.attributes().size());
    CHECK(nAttributes <= 3);                                   // title, legend (and nothing at its default)
    Graph again;
    GraphXml::readGraph(merged, again);
    CHECK(again.showLegend() == graph.showLegend() && again.axis(0).title() == "p_{T}" && again.isLogY());
    // back to the defaults: the attributes go away
    again.setShowLegend(Graph().showLegend());
    XmlElement full2("Graph");
    GraphXml::writeGraph(full2, again);
    PlotDocument::mergeDifferences(merged, full2, defaults);
    CHECK(!merged.hasAttribute("legend"));
  }

  // changes made on screen recorded in the file: only what changed
  {
    PlotDocument doc;
    CHECK(doc.read(FileSystem::join(top, "two.xml"), error));
    PlotSpec drawn;
    CHECK(drawn.read(doc.document(), doc.directory(), error) && drawn.openInputs(error));
    Canvas canvas;
    String problems2;
    drawn.build(drawn.windows()[0].canvases[0], canvas, problems2);
    XmlElement before = drawn.canvasXml(canvas);
    CHECK(before.isNamed("Canvas") && before.children("Graph").size() == 2);
    // on screen: log y on graph 0, the color of its first item, the second item of graph 1 removed
    canvas.graph(0).setLogY(true);
    canvas.graph(0).items()[0].markerAttributes().setColor(632, 1.0f);
    int itemsBefore = int(canvas.graph(1).items().size());
    canvas.graph(1).items().erase(canvas.graph(1).items().begin() + 1);
    XmlElement after = drawn.canvasXml(canvas);
    Path cp = doc.children(doc.children(Path(), PlotDocument::kWindow)[0], PlotDocument::kCanvas)[0];
    XmlElement element = *doc.element(cp);
    String original = element.toString();
    CHECK(PlotDocument::syncCanvas(element, before, after));
    const XmlElement * g0 = element.children("Graph")[0];
    const XmlElement * g1 = element.children("Graph")[1];
    CHECK(g0->attribute("logY", "") == "true" || (g0->findFirst("Axis", "which", "y") && g0->findFirst("Axis", "which", "y")->attribute("log", "") == "true"));
    CHECK(g0->children("Histogram")[0]->child("Marker") && g0->children("Histogram")[0]->child("Marker")->attribute("color") == GraphXml::colorName(632));
    CHECK(int(g1->children("Histogram").size()) == itemsBefore - 1);
    // compact: no attribute that did not change
    CHECK(!g0->hasAttribute("legend") && !g0->children("Histogram")[0]->hasAttribute("showLine"));
    CHECK(element.toString().str().size() < original.str().size() + 200);
    // nothing changed: nothing written
    XmlElement same = *doc.element(cp);
    CHECK(!PlotDocument::syncCanvas(same, before, before));
    // the element changed draws the canvas as it was on screen
    doc.replace(cp, element);
    PlotSpec again2;
    CHECK(again2.read(doc.document(), doc.directory(), error) && again2.openInputs(error));
    Canvas redrawn;
    again2.build(again2.windows()[0].canvases[0], redrawn, problems2);
    CHECK(redrawn.graph(0).isLogY());
    CHECK(redrawn.graph(0).items()[0].markerAttributes().color() == 632);
    CHECK(int(redrawn.graph(1).items().size()) == itemsBefore - 1);
    CHECK(again2.canvasXml(redrawn).toString() == after.toString());
  }

  // relative paths
  CHECK(PlotDocument::relativePath("/a/b/c.cap", "/a/b") == "c.cap");
  CHECK(PlotDocument::relativePath("/a/x/c.cap", "/a/b") == "../x/c.cap");
  CHECK(PlotDocument::relativePath("/q/c.cap", "/a/b/c/d") == "/q/c.cap");

  std::printf("TestPlotDocument: %d checks, %d failures\n", nChecks, nFailures);
  return nFailures == 0 ? 0 : 1;
}
