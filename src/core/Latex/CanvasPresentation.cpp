/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include <cstdio>
#include <cstdlib>
#include "CanvasPresentation.hpp"
#include "FileSystem.hpp"
#include "LatexDocument.hpp"
#include "LatexFrame.hpp"
#include "PdfPainter.hpp"

namespace CAP
{

  using namespace GRAPHICS;

  CanvasPresentation::CanvasPresentation(const String & title)
  :
  _title(title),
  _subtitle(),
  _author(),
  _email(),
  _affiliation(),
  _theme("Madrid"),
  _onePerGraph(false),
  _frames()
  { }

  void CanvasPresentation::addCanvas(const Canvas & canvas, const String & title)
  {
  String name = title.str().empty() ? canvas.title() : title;
  if (!_onePerGraph || canvas.nGraphs() <= 1) { _frames.push_back(Frame{ name, &canvas, -1, String() }); return; }
  for (int k=0; k<canvas.nGraphs(); k++)
    {
    const Graph & g = canvas.graph(k);
    if (g.items().empty() && g.shapes().empty()) continue;                    // empty cells: no frame
    String graphTitle = g.title().str().empty() ? name + String(" (") + String(std::to_string(k + 1)) + String(")") : g.title();
    _frames.push_back(Frame{ graphTitle, &canvas, k, String() });
    }
  }

  void CanvasPresentation::addCanvases(const CanvasRegistry & registry, const String & group)
  {
  for (int k=0; k<registry.size(); k++)
    if (group.str().empty() || registry.group(k) == group) addCanvas(registry.canvas(k), registry.name(k));
  }

  void CanvasPresentation::addText(const String & title, const String & text)
  {
  _frames.push_back(Frame{ title, nullptr, -1, text });
  }

  String CanvasPresentation::write(const String & directory, const String & name)
  {
  String figures = FileSystem::join(directory, "figures");
  try
    {
    if (!FileSystem::isDirectory(figures)) FileSystem::createDirectory(figures);
    }
  catch (const std::exception &) { return String(); }
  LatexDocument document;
  document.setDocumentClassName("beamer");
  document.setThemeName(_theme);
  document.setTitle(_title);
  document.setSubtitle(_subtitle);
  if (!_author.str().empty()) document.addAuthor(_author, _email, _affiliation);
  document.setUseToday(true);
  document.setOutputPath(directory);
  document.setOutFileName(name);
  int number = 0;
  for (const Frame & f : _frames)
    {
    LatexFrame & frame = document.addFrame(f.title);
    if (f.canvas)
      {
      // the figure: the canvas (or one graph) as a PDF the size it has on screen, scaled to fill the frame
      const Canvas & c = *f.canvas;
      float w = float(c.width()), h = float(c.height());
      String file = String("figure") + String(std::to_string(++number)) + String(".pdf");
      bool ok;
      if (f.graph < 0) { PdfPainter p(w, h); c.draw(p); ok = p.write(FileSystem::join(figures, file)); }
      else
        {
        const Graph & g = c.graph(f.graph);
        PdfPainter p((g.cellXHigh() - g.cellXLow())*w, (g.cellYHigh() - g.cellYLow())*h);
        CanvasRegistry::drawGraph(p, c, f.graph, w, h);
        ok = p.write(FileSystem::join(figures, file));
        }
      if (!ok) return String();
      frame.addText(String("\\centering\\includegraphics[width=\\textwidth,height=0.82\\textheight,keepaspectratio]{figures/")
                    + file + String("}"));
      }
    else
      {
      std::vector<String> items;
      std::string text = f.text.str();
      for (std::size_t start = 0; start <= text.size();)
        {
        std::size_t end = text.find('\n', start);
        if (end == std::string::npos) end = text.size();
        if (end > start) items.push_back(String(text.substr(start, end - start)));
        start = end + 1;
        }
      frame.addList(items);
      }
    document.setScopeToParent();                                              // the next frame: in the document
    }
  document.create();
  return FileSystem::join(directory, name + String(".tex"));
  }

  bool CanvasPresentation::compile(const String & texPath, String * log)
  {
  // pdflatex: on the path, or in the MacTeX folder
  String latex = "pdflatex";
  if (std::system("command -v pdflatex > /dev/null 2>&1") != 0 && FileSystem::isFile("/Library/TeX/texbin/pdflatex"))
    latex = "/Library/TeX/texbin/pdflatex";
  String directory = FileSystem::parent(texPath), file = FileSystem::name(texPath);
  String command = String("cd '") + (directory.str().empty() ? String(".") : directory) + String("' && ") + latex
                   + String(" -interaction=nonstopmode -halt-on-error '") + file + String("' 2>&1");
  std::string output;
  for (int pass=0; pass<2; pass++)                                            // twice: references, frame numbers
    {
    output.clear();
    FILE * pipe = popen(command.data(), "r");
    if (!pipe) return false;
    char buffer[4096];
    while (size_t n = std::fread(buffer, 1, sizeof(buffer), pipe)) output.append(buffer, n);
    if (pclose(pipe) != 0) break;
    }
  if (log) *log = String(output);
  return FileSystem::isFile(FileSystem::withExtension(texPath, ".pdf"));
  }

} // namespace CAP
