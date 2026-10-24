/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
//!
//! Write the XML template of every CAP task class (see CAP::Task::exportXmlTemplates()): one file
//! <ClassName>.xml per class, listing the parameters the class accepts with their default values and types. These templates are the input of the task-configuration editor.
//!
//! Usage (after sourcing SetupCAP.sh):
//!   root -l -b -q 'ExportXmlTemplates.C'                          // all task classes -> $CAP_DATABASE_PATH/Templates/
//!   root -l -b -q 'ExportXmlTemplates.C("/some/dir/")'            // all task classes -> /some/dir/
//!   root -l -b -q 'ExportXmlTemplates.C("", "CAP::PtPtAnalyzer")' // one class
//!
#include <iostream>
#include <vector>
#include "TSystem.h"
#include "TSystemDirectory.h"
#include "TList.h"
#include "TString.h"

//! Load every CAP library found in $CAP_LIB_PATH, so that all task classes are known to ROOT.
int loadCapLibraries()
{
  const char * libPath = gSystem->Getenv("CAP_LIB_PATH");
  if (!libPath) { std::cout << "<ERROR> CAP_LIB_PATH is not set: source SetupCAP.sh first" << std::endl; return -1; }
  TSystemDirectory directory("lib",libPath);
  TList * files = directory.GetListOfFiles();
  if (!files) { std::cout << "<ERROR> cannot list " << libPath << std::endl; return -1; }
  int nLoaded = 0;
  for (TObject * object : *files)
    {
    TString fileName = object->GetName();
    if (!fileName.BeginsWith("lib") || !(fileName.EndsWith(".dylib") || fileName.EndsWith(".so"))) continue;
    if (gSystem->Load(TString(libPath)+"/"+fileName)>=0) nLoaded++;
    else std::cout << "<WARNING> could not load " << fileName << std::endl;
    }
  delete files;
  return nLoaded;
}

int ExportXmlTemplates(const char * outputPath="", const char * className="")
{
  if (loadCapLibraries()<=0) return 1;

  TString path = outputPath;
  if (path.IsNull())
    {
    const char * db = gSystem->Getenv("CAP_DATABASE_PATH");
    if (!db) { std::cout << "<ERROR> CAP_DATABASE_PATH is not set and no output path given" << std::endl; return 1; }
    path = TString(db) + "/Templates/";
    }
  if (!path.EndsWith("/")) path += "/";
  gSystem->mkdir(path,kTRUE);

  std::vector<CAP::String> classNames;
  if (TString(className).Length()>0) classNames.push_back(CAP::String(className));

  std::vector<CAP::String> written = CAP::Task::exportXmlTemplates(CAP::String(path.Data()),classNames);
  for (const CAP::String & fileName : written) std::cout << "  wrote " << fileName << std::endl;
  std::cout << written.size() << " template(s) written to " << path << std::endl;
  return written.empty() ? 1 : 0;
}
