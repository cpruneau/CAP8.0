/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file PlotHistos.C
//! @brief bin/PlotHistos: plots of the histograms and functions described in an XML file, on screen or in batch.
//
// PlotHistos -- the histograms and functions of CAP files, plotted as an XML file describes them.
//
//   PlotHistos [options] plots.xml [file.cap ...]
//
// The XML file (see GRAPHICS::PlotSpec) describes windows of canvases (n x m per window), canvases of graphs (q x p
// per canvas), and the histograms and functions of each graph, with every setting of margins, axes, canvases, graphs
// and items. The CAP files given after it replace its inputs, in order.
//
// On screen (the default), CapGui opens the plot file with every window shown (the first one in the main window):
// everything changed on the canvases is written to the plot file (File > Save), and File > Export prints the canvases
// and graphs that have file names, and writes the PDF book (one canvas per page), the PowerPoint presentation (one
// canvas per slide) and the LaTeX document.
//
// Options:
//   --print              print the canvases and graphs that have file names
//   --pdf                write the PDF book
//   --pptx               write the PowerPoint presentation
//   --latex              write the LaTeX document
//   --all                all of the above
//   --save file.xml      write the settings (every one of them) to a file
//   --output folder      folder of the outputs (instead of the one of the XML file)
//   --batch              no window: write what is asked and exit (the default when an output is asked)
//   --show               show the windows even when outputs are asked (they are then written with File > Export)
//   --config file.xml    configuration of CapGui
//   --help               this text
//
// Template mode: PlotHistos --template plots.xml SOURCE... writes a plot file listing the histograms of the sources:
// CAP files (their contents), or workflow files of RunCAP.py (the histograms their stages create, found by
// configuring the tasks without running them: the event and particle filters decide which are made). The file can
// then be edited by hand, or, with --choose, in the Plot Template window, which selects the histograms and lays them
// out before writing the file and showing it.
//
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include "FileSystem.hpp"
#include "CapGuiApp.hpp"
#include "PlotCatalog.hpp"
#include "PlotOutputs.hpp"

namespace
{
void usage()
{
  std::printf(
    "Usage: PlotHistos [options] plots.xml [file.cap ...]\n"
    "  Plots the histograms and functions of CAP files as the XML file describes them: windows of canvases,\n"
    "  canvases of graphs, graphs of histograms and functions, with their settings. The CAP files replace the\n"
    "  inputs of the XML file, in order.\n"
    "Options:\n"
    "  --print            print the canvases and graphs that have file names\n"
    "  --pdf              write the PDF book (one canvas per page)\n"
    "  --pptx             write the PowerPoint presentation (one canvas per slide)\n"
    "  --latex            write the LaTeX document\n"
    "  --all              all of the above\n"
    "  --save file.xml    write the settings to a file\n"
    "  --output folder    folder of the outputs\n"
    "  --batch            no window: write the outputs asked and exit (the default when one is asked)\n"
    "  --show             show the windows even when outputs are asked\n"
    "  --config file.xml  configuration of CapGui\n"
    "\n"
    "Usage: PlotHistos --template plots.xml [options] SOURCE ...\n"
    "  Writes a plot file of the histograms of the sources: CAP files (what they hold), or workflow XML files of\n"
    "  RunCAP.py (what their stages will write, from their tasks and filters, without running them).\n"
    "Options:\n"
    "  --choose              choose the histograms and the layout in the Plot Template window\n"
    "  --data-dir folder     folder of the output files of the workflows (default: as written in them)\n"
    "  --select pattern      plot the histograms matching a pattern (* and ?; without '/', the name only);\n"
    "                        repeatable, default all\n"
    "  --exclude pattern     do not plot the histograms matching a pattern; repeatable\n"
    "  --group histogram|combination  a canvas per histogram name (a graph per filter combination, the\n"
    "                        default) or a canvas per combination (a graph per name)\n"
    "  --overlay             overlay the combinations of 1D histograms in one graph\n"
    "  --max-graphs N        graphs per canvas at most (12)\n"
    "  --max-canvases N      canvases per window at most (6)\n"
    "  --figures folder      output folder written in the plot file (figures)\n"
    "  --title text          title of the outputs (Plots)\n"
    "  --list                print the catalog of the histograms found\n");
}

//! An absolute path (relative to the current folder).
CAP::String absolute(const CAP::String & path)
{
  CAP::String p = CAP::FileSystem::expand(path);
  if (p.isNull() || p.str()[0] == '/') return p;
  return CAP::FileSystem::normalize(CAP::FileSystem::join(CAP::FileSystem::current(), p));
}
} // namespace

//! @brief Plot as an XML file describes. @param argc number of arguments @param argv arguments @return exit status
int main(int argc, char ** argv)
{
  using namespace CAP;
  using GRAPHICS::PlotSpec;
  bool print = false, pdf = false, pptx = false, latex = false, batch = false, show = false;
  String save, output, specFile;
  std::vector<String> dataFiles;
  // template mode
  String templateFile, dataDirectory;
  bool choose = false, list = false;
  std::vector<String> include, exclude;
  GRAPHICS::PlotTemplate::Options options;
  std::vector<char *> guiArguments{argv[0]};
  for (int k=1; k<argc; k++)
    {
    std::string a = argv[k];
    if (a == "--help" || a == "-h") { usage(); return 0; }
    else if (a == "--print") print = true;
    else if (a == "--pdf") pdf = true;
    else if (a == "--pptx") pptx = true;
    else if (a == "--latex") latex = true;
    else if (a == "--all") print = pdf = pptx = latex = true;
    else if (a == "--batch") batch = true;
    else if (a == "--show") show = true;
    else if (a == "--save" && k + 1 < argc) save = absolute(String(argv[++k]));
    else if (a == "--output" && k + 1 < argc) output = absolute(String(argv[++k]));
    else if (a == "--template" && k + 1 < argc) templateFile = absolute(String(argv[++k]));
    else if (a == "--choose") choose = true;
    else if (a == "--list") list = true;
    else if (a == "--overlay") options.overlay = true;
    else if (a == "--data-dir" && k + 1 < argc) dataDirectory = absolute(String(argv[++k]));
    else if (a == "--select" && k + 1 < argc) include.push_back(String(argv[++k]));
    else if (a == "--exclude" && k + 1 < argc) exclude.push_back(String(argv[++k]));
    else if (a == "--figures" && k + 1 < argc) options.outputDirectory = String(argv[++k]);
    else if (a == "--title" && k + 1 < argc) options.title = String(argv[++k]);
    else if (a == "--max-graphs" && k + 1 < argc) options.maxGraphs = std::max(1, std::atoi(argv[++k]));
    else if (a == "--max-canvases" && k + 1 < argc) options.maxCanvases = std::max(1, std::atoi(argv[++k]));
    else if (a == "--group" && k + 1 < argc)
      {
      std::string g = argv[++k];
      if (g == "histogram" || g == "name") options.grouping = GRAPHICS::PlotTemplate::Options::kByHistogram;
      else if (g == "combination") options.grouping = GRAPHICS::PlotTemplate::Options::kByCombination;
      else { std::fprintf(stderr, "PlotHistos: --group histogram or combination\n"); return 2; }
      }
    else if ((a == "--config" || a == "-c") && k + 1 < argc) { guiArguments.push_back(argv[k]); guiArguments.push_back(argv[++k]); }
    else if (a.size() > 1 && a[0] == '-') { std::fprintf(stderr, "PlotHistos: unknown option %s\n", a.c_str()); usage(); return 2; }
    else if (specFile.isNull()) specFile = absolute(String(argv[k]));
    else dataFiles.push_back(absolute(String(argv[k])));
    }
  if (!templateFile.isNull())
    {
    // template mode: every file given is a source
    std::vector<String> sources;
    if (!specFile.isNull()) sources.push_back(specFile);
    sources.insert(sources.end(), dataFiles.begin(), dataFiles.end());
    if (sources.empty()) { usage(); return 2; }
    GRAPHICS::PlotTemplate catalog;
    String problems;
    bool found = GUI::catalogPlots(sources, dataDirectory, catalog, problems);
    if (!problems.isNull()) std::fprintf(stderr, "PlotHistos: %s", problems.data());
    if (!found) { std::fprintf(stderr, "PlotHistos: no histogram found\n"); return 1; }
    if (!include.empty() || !exclude.empty()) catalog.select(include, exclude);
    if (list)
      for (const auto & e : catalog.entries())
        std::printf("%s %-12s %s%s\n", catalog.isSelected(e) ? "*" : " ", catalog.inputs()[std::size_t(e.input)].id.data(),
                    e.path.data(), e.rank == 2 ? "  (2D)" : "");
    if (choose)
      {
      guiArguments.push_back(nullptr);
      GUI::CapGuiApp application(GUI::CapGuiApp::kBasic, {}, int(guiArguments.size()) - 1, guiArguments.data());
      application.setCatalog(GUI::catalogPlots);
      application.startTemplate(catalog, options, templateFile);
      return application.run();
      }
    if (catalog.nSelected() == 0) { std::fprintf(stderr, "PlotHistos: no histogram selected\n"); return 1; }
    try { catalog.makeSpec(options).write(templateFile); }
    catch (const std::exception & e) { std::fprintf(stderr, "PlotHistos: %s: %s\n", templateFile.data(), e.what()); return 1; }
    std::printf("PlotHistos: %d of %d histograms of %d inputs in %s\n", catalog.nSelected(), int(catalog.entries().size()),
                int(catalog.inputs().size()), templateFile.data());
    return 0;
    }
  if (specFile.isNull()) { usage(); return 2; }
  bool outputs = print || pdf || pptx || latex || !save.isNull();
  if (outputs && !show) batch = true;

  if (!batch)
    {
    guiArguments.push_back(nullptr);
    // on screen: CapGui on the plot file, every window shown; the files given replace its inputs
    GUI::CapGuiApp application(GUI::CapGuiApp::kEdit, { specFile }, int(guiArguments.size()) - 1, guiArguments.data());
    application.setCatalog(GUI::catalogPlots);
    application.setAllWindows(true);
    application.setInputFiles(dataFiles);
    return application.run();
    }

  // batch: the canvases made without a window, the outputs written
  PlotSpec spec;
  String error, problems;
  if (!spec.read(specFile, error)) { std::fprintf(stderr, "PlotHistos: %s: %s\n", specFile.data(), error.data()); return 1; }
  for (std::size_t k=0; k<dataFiles.size(); k++) spec.setInputPath(int(k), dataFiles[k]);
  if (!output.isNull()) spec.output().directory = output;
  if (!spec.openInputs(error)) { std::fprintf(stderr, "PlotHistos: %s\n", error.data()); return 1; }
  std::vector<PlotSpec::WindowState> windows = spec.buildAll(problems);
  if (!problems.isNull()) std::fprintf(stderr, "PlotHistos: not shown:\n%s", problems.data());
  int status = 0;
  if (print)
    {
    std::vector<String> files = GUI::PlotOutputs::print(spec, windows);
    std::printf("PlotHistos: %d plots printed in %s\n", int(files.size()), spec.outputPath("").data());
    }
  if (pdf)
    {
    String path = GUI::PlotOutputs::pdf(spec, windows);
    if (path.isNull()) { std::fprintf(stderr, "PlotHistos: cannot write the PDF book\n"); status = 1; }
    else std::printf("PlotHistos: PDF book %s\n", path.data());
    }
  if (pptx)
    {
    String path = GUI::PlotOutputs::pptx(spec, windows);
    if (path.isNull()) { std::fprintf(stderr, "PlotHistos: cannot write the PowerPoint presentation\n"); status = 1; }
    else std::printf("PlotHistos: PowerPoint %s\n", path.data());
    }
  if (latex)
    {
    String path = GUI::PlotOutputs::latex(spec, windows, error);
    if (!error.isNull()) { std::fprintf(stderr, "PlotHistos: %s\n", error.data()); status = 1; }
    if (!path.isNull()) std::printf("PlotHistos: LaTeX %s\n", path.data());
    }
  if (!save.isNull())
    {
    if (!spec.write(save, windows, error)) { std::fprintf(stderr, "PlotHistos: %s: %s\n", save.data(), error.data()); status = 1; }
    else std::printf("PlotHistos: settings written to %s\n", save.data());
    }
  return status;
}
