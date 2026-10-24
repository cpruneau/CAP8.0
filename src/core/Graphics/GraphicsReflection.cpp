/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file GraphicsReflection.cpp
//! @brief Canvases and graphs described for the run-time reflection (see Reflection.hpp): usable from CapShell.
#include "Reflection.hpp"
#include "CanvasRegistry.hpp"
#include "DataPoints.hpp"
#include "FileSystem.hpp"
#include "PdfPainter.hpp"
#include "SvgPainter.hpp"

namespace
{
  using namespace CAP;
  using namespace CAP::GRAPHICS;

  //! Write a canvas (or one graph of it) to an SVG or PDF file, the size of the canvas (or of the cell of the graph).
  bool saveCanvas(const Canvas & canvas, const String & fileName, int graph)
  {
  float w = float(canvas.width()), h = float(canvas.height());
  bool pdf = FileSystem::hasExtension(fileName, ".pdf");
  if (graph < 0)
    {
    if (pdf) { PdfPainter p(w, h); canvas.draw(p); return p.write(fileName); }
    SvgPainter p(w, h); canvas.draw(p); return p.write(fileName);
    }
  const Graph & g = canvas.graph(graph);
  float gw = (g.cellXHigh() - g.cellXLow())*w, gh = (g.cellYHigh() - g.cellYLow())*h;
  if (pdf) { PdfPainter p(gw, gh); CanvasRegistry::drawGraph(p, canvas, graph, w, h); return p.write(fileName); }
  SvgPainter p(gw, gh); CanvasRegistry::drawGraph(p, canvas, graph, w, h); return p.write(fileName);
  }
} // namespace

CAP_REFLECTION(Graphics)
{
  using namespace CAP;
  using namespace CAP::GRAPHICS;
  using namespace CAP::REFLECTION;

  ClassBuilder<Graph>("CAP::GRAPHICS::Graph")
    .property("title", static_cast<const String & (Graph::*)() const>(&Graph::title), &Graph::setTitle)
    .property("logX", &Graph::isLogX, &Graph::setLogX)
    .property("logY", &Graph::isLogY, &Graph::setLogY)
    .property("legend", &Graph::showLegend, &Graph::setShowLegend)
    .property("colorPalette", &Graph::colorPaletteName, &Graph::setColorPaletteName)
    .property("markerPalette", &Graph::markerPaletteName, &Graph::setMarkerPaletteName)
    .function("add", +[](Graph & g, MATH::DataPoints * data) { if (data) g.show(GraphItem(data, data->name()), false); })
    .function("nItems", +[](const Graph & g) { return long(g.items().size()); })
    .function("nShapes", +[](const Graph & g) { return long(g.shapes().size()); })
    .function("clear", +[](Graph & g) { g.clearItems(); g.clearShapes(); })
    .method("restyleItems", &Graph::restyleItems)
    .method("setAutoRange", &Graph::setAutoRange);

  ClassBuilder<Canvas>("CAP::GRAPHICS::Canvas")
    .constructor<const String &, const String &, int, int>()
    .constructor<const String &, const String &>()
    .property("name", static_cast<const String & (Canvas::*)() const>(&Canvas::name), &Canvas::setName)
    .property("title", static_cast<const String & (Canvas::*)() const>(&Canvas::title), &Canvas::setTitle)
    .property("width", &Canvas::width)
    .property("height", &Canvas::height)
    .method("setSize", &Canvas::setSize)
    .method("divide", &Canvas::divide)
    .method("nGraphs", &Canvas::nGraphs)
    .method("nColumns", &Canvas::nColumns)
    .method("nRows", &Canvas::nRows)
    .method("graph", static_cast<Graph & (Canvas::*)(int)>(&Canvas::graph))
    .function("save", +[](const Canvas & c, const String & fileName) { return saveCanvas(c, fileName, -1); })
    .function("saveGraph", +[](const Canvas & c, int graph, const String & fileName) { return saveCanvas(c, fileName, graph); });
}
