/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "PlotCatalog.hpp"
#include "CapFile.hpp"
#include "FileSystem.hpp"
#include "WorkflowHistograms.hpp"
#include "XmlDocument.hpp"

namespace CAP
{
  namespace GUI
  {

  bool catalogPlots(const std::vector<String> & sources, const String & dataDirectory, GRAPHICS::PlotTemplate & catalog,
                            String & problems)
  {
  int n = 0;
  for (const String & source : sources)
    {
    if (FileSystem::hasExtension(source, ".xml"))
      {
      // a workflow: the histograms of its stages, made empty
      XmlDocument workflow;
      try { workflow = XmlDocument::read(source); }
      catch (const std::exception & e) { problems += source + String(": ") + String(e.what()) + String("\n"); continue; }
      String error;
      std::vector<WorkflowHistograms::Stage> stages = WorkflowHistograms::make(workflow, error);
      if (!error.isNull()) { problems += source + String(": ") + error + String("\n"); continue; }
      for (auto & stage : stages)
        {
        problems += stage.problems;
        if (!stage.folder || stage.folder->paths().empty()) continue;
        String file = dataDirectory.isNull() ? stage.outputFile : FileSystem::join(dataDirectory, stage.outputFile);
        int input = -1;
        for (std::size_t k=0; k<catalog.inputs().size(); k++) if (catalog.inputs()[k].file == file) input = int(k);
        if (input < 0) input = catalog.addInput(stage.name, file);
        n += catalog.addFolder(input, *stage.folder);
        }
      }
    else
      {
      // a CAP file: its contents
      try
        {
        CapFile file(source, File::kReadOnly);
        file.load(File::Loading::kLazy);
        String id = FileSystem::stem(source);
        int input = catalog.addInput(id, source);
        n += catalog.addFolder(input, file.folder());
        }
      catch (const std::exception & e) { problems += source + String(": ") + String(e.what()) + String("\n"); }
      }
    }
  return n > 0;
  }

  } // namespace GUI
} // namespace CAP
