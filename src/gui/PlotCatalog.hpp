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
//! @brief catalogPlots(): the histograms of Datum files or RunDatum workflows, for a new plot file (PlotTemplate). Part of
//! the programs (DatumGui, PlotHistos), which link the analysis libraries: a workflow is configured to know them.
#ifndef DATUM__GUI__PlotCatalog
#define DATUM__GUI__PlotCatalog
#include <vector>
#include "PlotTemplate.hpp"
#include "String.hpp"

namespace Datum
{
  namespace GUI
  {
  //!
  //! @brief Catalog the histograms of sources: Datum files (their contents), or workflow files of RunDatum.py (the
  //! histograms their stages write, made empty without running them; one input per stage, its output file, in
  //! dataDirectory). @param sources files (.datum or .cap, or .xml workflows) @param dataDirectory directory of the output files
  //! of the workflows ("": as written in them) @param catalog catalog filled @param problems what could not be read
  //! @return false if nothing could be catalogued
  //!
  bool catalogPlots(const std::vector<String> & sources, const String & dataDirectory, GRAPHICS::PlotTemplate & catalog,
                    String & problems);
  } // namespace GUI
} // namespace Datum

#endif // !DATUM__GUI__PlotCatalog
