/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "RunSubSampleSum.hpp"
#include "PrintHelpers.hpp"
#include "CapFile.hpp"
#include "RootFileHelpers.hpp"
#include "SubSampleStatCalculator.hpp"
#include "Exceptions.hpp"


namespace CAP
{

RunSubSampleSum::RunSubSampleSum()
{
  
}

RunSubSampleSum::~RunSubSampleSum()
{   }


void RunSubSampleSum::run(String & histogramImportPathName,
                          String & histogramImportFileName,
                          String & histogramExportPathName,
                          String & histogramExportFileName,
                          bool verbose)
{
  printLine();
  printString("Executing RunSubSampleSum.run()");
  printValue("importPathName",histogramImportPathName);
  printValue("importFileName",histogramImportFileName);
  printValue("exportPathName",histogramExportPathName);
  printValue("exportPathName",histogramExportFileName);
  std::vector<String> directories = listDirsIn(histogramImportPathName,verbose);
  printValue("Number of folders found",directories.size());
  Configuration configuration;
  try
  {
    // The task is named SubSample and reads SubSample:HISTOGRAM:IMPORT:PATH, ... (the keys set here were
    // SubSample:HistogramsImportPath, ..., which the task does not read, and its default name SubSampleStatCalculator
    // did not select the SubSample: keys).
    Task * task = new CAP::SubSampleStatCalculator();
    task->setName("SubSample");
    task->setDefaultConfiguration();
    configuration.addProperty("SubSample:TASK:SEVERITY",              String(verbose ? "DEBUG" : "INFO"));
    configuration.addProperty("SubSample:HISTOGRAM:IMPORT:PATH",      histogramImportPathName);
    configuration.addProperty("SubSample:HISTOGRAM:IMPORT:FILE_NAME", histogramImportFileName);
    configuration.addProperty("SubSample:HISTOGRAM:EXPORT:PATH",      histogramExportPathName);
    configuration.addProperty("SubSample:HISTOGRAM:EXPORT:FILE_NAME", histogramExportFileName);
    task->configure(configuration);
    task->execute();
    delete task;
  }
  catch (PropertyException & pe)
  {
  pe.print();
  }
  catch (Exception & te)
  {
  te.print();
  }
}

} // namespace CAP
