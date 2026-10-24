/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file TestCanvasPresentation.cpp
//! @brief Tests of the presentations of canvases: frames per canvas or per graph, text frames, the .tex file and its
//! figures, and the PDF made by pdflatex (when pdflatex is there). With a directory argument, the presentation is
//! written there and kept.
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include "CanvasPresentation.hpp"
#include "Directory.hpp"
#include "FileSystem.hpp"
#include "Histogram.hpp"

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

std::string read(const String & path)
{
  std::ifstream in(path.data());
  std::stringstream s;
  s << in.rdbuf();
  return s.str();
}

int count(const std::string & text, const std::string & what)
{
  int n = 0;
  for (std::size_t at = text.find(what); at != std::string::npos; at = text.find(what, at + 1)) n++;
  return n;
}
} // namespace

#define CHECK(condition) check((condition), #condition, __LINE__)

int main(int argc, char ** argv)
{
  MATH::Histogram gauss("gauss", "Gaussian", 40, -4.0, 4.0), flat("flat", "Flat", 20, 0.0, 1.0);
  for (int k=0; k<4000; k++) { gauss.fill(2.0*std::sin(0.37*k)*std::cos(0.11*k), 1.0); flat.fill((k % 100)/100.0, 1.0); }
  Canvas spectra("spectra", "Spectra", 800, 600), pair("pair", "Pair", 900, 450);
  spectra.divide(1, 1);
  spectra.graph(0).show(GraphItem(&gauss, "gauss"), false);
  pair.divide(2, 1);
  pair.graph(0).show(GraphItem(&gauss, "gauss"), false);
  pair.graph(1).show(GraphItem(&flat, "flat"), false);
  CanvasRegistry registry;
  registry.add(spectra, "Gaussian spectrum", "Window 1");
  registry.add(pair, "Side by side", "Window 2");

  char buffer[] = "/tmp/CapTestCanvasPresentation.XXXXXX";
  String directory = argc > 1 ? String(argv[1]) : String(mkdtemp(buffer));
  CanvasPresentation talk("Results of the week");
  talk.setAuthor("C. Pruneau", "", "Wayne State University");
  talk.addText("Outline", "Spectra\nCorrelations");
  talk.addCanvases(registry);                                          // a frame per canvas
  talk.setOnePerGraph(true);
  talk.addCanvas(pair, "Pair");                                         // a frame per graph
  CHECK(talk.nFrames() == 1 + 2 + 2);
  String tex = talk.write(directory, "talk");
  CHECK(!tex.str().empty() && FileSystem::isFile(tex));
  std::string text = read(tex);
  CHECK(text.find("\\documentclass{beamer}") != std::string::npos && text.find("\\title{Results of the week}") != std::string::npos);
  CHECK(count(text, "\\begin{frame}") == 6 && count(text, "\\end{frame}") == 6);       // the title page, and 5 frames
  CHECK(text.find("\\frametitle{Gaussian spectrum}") != std::string::npos && text.find("\\frametitle{Side by side}") != std::string::npos);
  CHECK(count(text, "\\includegraphics") == 4 && text.find("Correlations") != std::string::npos);
  for (int k=1; k<=4; k++) CHECK(FileSystem::isFile(FileSystem::join(directory, String("figures/figure") + String(std::to_string(k)) + String(".pdf"))));
  if (std::system("command -v pdflatex > /dev/null 2>&1") == 0 || FileSystem::isFile("/Library/TeX/texbin/pdflatex"))
    {
    String log;
    bool ok = CanvasPresentation::compile(tex, &log);
    CHECK(ok);
    if (!ok) std::printf("%s\n", log.data());
    }
  else std::printf("pdflatex not found: the presentation is not compiled\n");
  if (argc <= 1) Directory(directory).remove(true);

  std::printf("TestCanvasPresentation: %d checks, %d failed\n", nChecks, nFailures);
  return nFailures;
}
