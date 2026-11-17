/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file DatumGuiMain.cpp
//! @brief main() of bin/DatumGui (DatumGui.app on macOS) and of bin/PlotEditor, its other name.
//
// DatumGui -- the Datum graphical interface: canvases, plot files (the XML files of PlotHistos) and the file browser.
//
//   DatumGui [--basic | --edit] [plots.xml ...] [file.datum | folder ...] [--config gui.xml]
//
// Every session is a plot file: the canvases are its windows (see DatumGuiApp). Modes: --basic (the default without
// plot files) starts with a new plot file and the current folder in the browser; --edit (the default with plot
// files) opens the plot files given, or asks for one. Datum files and folders given are shown in the browser.
//
#include <cstdio>
#include <string>
#include <vector>
#include "DatumGuiApp.hpp"
#include "FileSystem.hpp"
#include "PlotCatalog.hpp"

namespace
{
void usage()
{
  std::printf(
    "Usage: DatumGui [--basic | --edit] [plots.xml ...] [file.datum | folder ...] [--config gui.xml] [--maps maps.datum]\n"
    "  The canvases are the windows of a plot file (the XML files of PlotHistos): drag histograms from the\n"
    "  browser onto them or onto the Plot Tree, change them, save the plot file; Plots > Export writes the figures.\n"
    "  --basic   start from a new plot file (the default without plot files)\n"
    "  --edit    start from the plot files given, or ask for one (the default with plot files)\n"
    "  Datum files and folders given are shown in the file browser.\n"
    "  --maps    open a maps file (particle efficiencies and smearers) in the Detector Maps window\n");
}
} // namespace

//! @brief Start the graphical interface. @param argc number of arguments @param argv arguments
//! @return exit status
int main(int argc, char ** argv)
{
  using namespace Datum;
  GUI::DatumGuiApp::Mode mode = GUI::DatumGuiApp::kBasic;
  bool modeGiven = false;
  std::vector<String> plotFiles;
  std::vector<char *> guiArguments{argv[0]};
  for (int k=1; k<argc; k++)
    {
    std::string a = argv[k];
    if (a == "--help" || a == "-h") { usage(); return 0; }
    else if (a == "--basic") { mode = GUI::DatumGuiApp::kBasic; modeGiven = true; }
    else if (a == "--edit")  { mode = GUI::DatumGuiApp::kEdit;  modeGiven = true; }
    else if ((a == "--config" || a == "-c") && k + 1 < argc) { guiArguments.push_back(argv[k]); guiArguments.push_back(argv[++k]); }
    else if (a == "--maps" && k + 1 < argc) { guiArguments.push_back(argv[k]); guiArguments.push_back(argv[++k]); }
    else if (a.compare(0, 4, "-psn") == 0) continue;   // macOS: a process serial number given by the Finder
    else if (a.size() > 1 && a[0] == '-') { std::fprintf(stderr, "DatumGui: unknown option %s\n", a.c_str()); usage(); return 2; }
    else if (FileSystem::hasExtension(String(a), ".xml")) plotFiles.push_back(String(a));
    else guiArguments.push_back(argv[k]);   // Datum files and folders: for the browser
    }
  if (!modeGiven && !plotFiles.empty()) mode = GUI::DatumGuiApp::kEdit;
  guiArguments.push_back(nullptr);
  GUI::DatumGuiApp application(mode, plotFiles, int(guiArguments.size()) - 1, guiArguments.data());
  application.setCatalog(GUI::catalogPlots);
  return application.run();
}
