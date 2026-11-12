/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file PlotCatalog.hpp
//! @brief catalogPlots(): the histograms of CAP files or RunCAP workflows, for a new plot file (PlotTemplate). Part of
//! the programs (CapGui, PlotHistos), which link the analysis libraries: a workflow is configured to know them.
#ifndef CAP__GUI__PlotCatalog
#define CAP__GUI__PlotCatalog
#include <vector>
#include "PlotTemplate.hpp"
#include "String.hpp"

namespace CAP
{
  namespace GUI
  {
  //!
  //! @brief Catalog the histograms of sources: CAP files (their contents), or workflow files of RunCAP.py (the
  //! histograms their stages write, made empty without running them; one input per stage, its output file, in
  //! dataDirectory). @param sources files (.cap, or .xml workflows) @param dataDirectory directory of the output files
  //! of the workflows ("": as written in them) @param catalog catalog filled @param problems what could not be read
  //! @return false if nothing could be catalogued
  //!
  bool catalogPlots(const std::vector<String> & sources, const String & dataDirectory, GRAPHICS::PlotTemplate & catalog,
                    String & problems);
  } // namespace GUI
} // namespace CAP

#endif // !CAP__GUI__PlotCatalog
