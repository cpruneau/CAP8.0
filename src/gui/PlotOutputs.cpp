/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include "PlotOutputs.hpp"
#include "CapFile.hpp"
#include "DataPoints.hpp"
#include "FileSystem.hpp"
#include "LatexRenderer.hpp"
#include "CanvasRegistry.hpp"
#include "PdfPainter.hpp"
#include "PptxPainter.hpp"
#include "XmlDocument.hpp"
#ifdef CAP_GUI_PRESENTATIONS
#include "CanvasPresentation.hpp"
#include "LatexAuthor.hpp"
#include "LatexDocument.hpp"
#endif

namespace CAP
{
  namespace GUI
  {

  using GRAPHICS::Canvas;
  using GRAPHICS::PlotSpec;

  namespace
  {
#ifdef CAP_GUI_PRESENTATIONS
  //! A LaTeX article (the document classes of the Latex folder write Beamer presentations): its header, then the
  //! elements added (sections and figures, as texts), then its trailer.
  class LatexArticle : public LatexDocument
  {
  public:
  void writeHeader(std::ofstream & out) override
  {
  out << "\\documentclass[11pt]{article}\n"
      << "\\usepackage[utf8]{inputenc}\n\\usepackage{amsmath,amssymb}\n\\usepackage{graphicx}\n\\usepackage[margin=2cm]{geometry}\n\n"
      << "\\title{" << title() << "}\n";
  for (const LatexAuthor * a : authors())
    {
    out << "\\author{" << a->author();
    if (!a->affiliation().isNull()) out << "\\\\ " << a->affiliation();
    out << "}\n";
    }
  out << "\\date{\\today}\n\n\\begin{document}\n\\maketitle\n\n";
  }
  };
#endif
  } // namespace

  // ------------------------------------------------------------------------------------------------ outputs

  std::vector<const Canvas *> PlotOutputs::canvases(const std::vector<PlotSpec::WindowState> & windows)
  {
  std::vector<const Canvas *> all;
  for (const auto & w : windows) for (const Canvas * c : w.canvases) all.push_back(c);
  return all;
  }

  namespace
  {
  //! The canvases of windows at their output sizes (see PlotSpec::atOutputSize()).
  struct Sized
  {
  std::vector<std::unique_ptr<Canvas>> copies;
  std::vector<const Canvas *> canvases;
  Sized(const PlotSpec & spec, const std::vector<PlotSpec::WindowState> & windows)
  : copies(), canvases(spec.atOutputSize(PlotOutputs::canvases(windows), copies)) { }
  };
  } // namespace

  std::vector<String> PlotOutputs::print(const PlotSpec & spec, const std::vector<PlotSpec::WindowState> & windows)
  {
  return spec.print(canvases(windows));
  }

  namespace
  {
  //! Path of an output: a file chosen, or the name of the specification in its output directory, with an extension.
  String outputFile(const PlotSpec & spec, const String & chosen, const String & name, const char * extension)
  {
  String path = chosen.isNull() ? spec.outputPath(name) : chosen;
  if (!FileSystem::hasExtension(path, extension)) path += String(extension);
  String parent = FileSystem::parent(path);
  if (!parent.isNull() && !FileSystem::isDirectory(parent)) FileSystem::createDirectory(parent);
  return path;
  }
  } // namespace

  String PlotOutputs::pdf(const PlotSpec & spec, const std::vector<PlotSpec::WindowState> & windows, const String & fileName)
  {
  String path = outputFile(spec, fileName, spec.output().pdf, ".pdf");
  Sized sized(spec, windows);
  return PlotSpec::writePdf(path, sized.canvases) ? path : String();
  }

  String PlotOutputs::pptx(const PlotSpec & spec, const std::vector<PlotSpec::WindowState> & windows, const String & fileName)
  {
  String path = outputFile(spec, fileName, spec.output().pptx, ".pptx");
  GRAPHICS::PptxPainter painter;
  Sized sized(spec, windows);
  for (const Canvas * c : sized.canvases) painter.addCanvas(*c);
  if (painter.nSlides() == 0) return String();
  return painter.write(path) ? path : String();
  }

  String PlotOutputs::latexText(const String & text)
  {
  const std::string & s = text.str();
  if (s.find('$') != std::string::npos || s.find('\\') != std::string::npos) return text;
  if (s.find('#') != std::string::npos || s.find("_{") != std::string::npos || s.find("^{") != std::string::npos)
    return GRAPHICS::LatexRenderer::fromRoot(text);
  std::string out;
  for (char c : s)
    {
    if (c == '_' || c == '%' || c == '&' || c == '#' || c == '{' || c == '}') out += '\\';
    out += c;
    }
  return String(out);
  }

  String PlotOutputs::latex(const PlotSpec & spec, const std::vector<PlotSpec::WindowState> & windows, String & error,
                            const String & fileName)
  {
#ifdef CAP_GUI_PRESENTATIONS
  const PlotSpec::OutputSpec & o = spec.output();
  String path = outputFile(spec, fileName, o.latex, ".tex");
  String directory = FileSystem::parent(path), stem = FileSystem::stem(path);
  String tex;
  if (o.latexStyle == "article")
    {
    String figures = FileSystem::join(directory, "figures");
    if (!FileSystem::isDirectory(figures)) FileSystem::createDirectory(figures);
    LatexArticle document;
    document.setTitle(latexText(o.title));
    if (!o.author.isNull()) document.addAuthor(latexText(o.author), "", latexText(o.affiliation));
    document.setOutputPath(directory);
    document.setOutFileName(stem);
    int number = 0;
    for (const auto & window : windows)
      {
      document.addText(String("\\section{") + latexText(window.title) + String("}"));
      std::vector<std::unique_ptr<Canvas>> copies;
      for (const Canvas * c : spec.atOutputSize(window.canvases, copies))
        {
        String name = GRAPHICS::CanvasRegistry::fileName(c->name().isNull() ? String("canvas") : c->name());
        String file = String(std::to_string(++number)) + String("_") + name + String(".pdf");
        GRAPHICS::PdfPainter p(float(c->width()), float(c->height()));
        c->draw(p);
        if (!p.write(FileSystem::join(figures, file))) { error = String("cannot write ") + FileSystem::join(figures, file); return String(); }
        String caption = latexText(c->title().isNull() ? c->name() : c->title());
        document.addText(String("\\begin{figure}[htbp]\n\\centering\n\\includegraphics[width=\\linewidth,height=0.8\\textheight,keepaspectratio]{figures/")
                         + file + String("}\n\\caption{") + caption + String("}\n\\label{fig:") + name + String("}\n\\end{figure}"));
        }
      }
    try { document.create(); }
    catch (const std::exception & e) { error = String(e.what()); return String(); }
    tex = FileSystem::join(directory, stem + String(".tex"));
    }
  else
    {
    CanvasPresentation presentation(latexText(o.title));
    if (!o.author.isNull()) presentation.setAuthor(latexText(o.author), "", latexText(o.affiliation));
    Sized sized(spec, windows);
    for (const Canvas * c : sized.canvases) presentation.addCanvas(*c, latexText(c->title().isNull() ? c->name() : c->title()));
    tex = presentation.write(directory, stem);
    if (tex.isNull()) { error = String("cannot write ") + path; return String(); }
    }
  if (o.latexCompile)
    {
    String log;
    if (!CanvasPresentation::compile(tex, &log))
      {
      const std::string & l = log.str();
      error = String("pdflatex failed:\n") + String(l.substr(l.size() > 1500 ? l.size() - 1500 : 0));
      }
    }
  return tex;
#else
  (void)spec; (void)windows; (void)fileName;
  error = "CAP is built without the Latex module (BuildCAP.py: Latex)";
  return String();
#endif
  }

  } // namespace GUI
} // namespace CAP
