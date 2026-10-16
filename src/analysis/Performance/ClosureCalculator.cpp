/* **********************************************************************
 * Copyright (C) 2019-2024, Claude Pruneau, Victor Gonzalez   
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "ClosureCalculator.hpp"
#include "ManagedObjects.hpp"
#include <memory>
#include "PrintHelpers.hpp"
#include "NameHelpers.hpp"
#include "CapFile.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "HistogramFolder.hpp"

ClassImp(CAP::ClosureCalculator);

namespace CAP
{

ClosureCalculator::ClosureCalculator()
:
Task()
{
  appendClassName("ClosureCalculator");
  setMinimumReportLevel(Object::kInfo);
}

void ClosureCalculator::setDefaultConfiguration()
{
  Task::setDefaultConfiguration();
  const String & taskName = name();
  String objectType = "HISTOGRAM";
  _configuration.addProperty(createKey(taskName,objectType,"METHOD"),int(0));
  _configuration.addProperty(createKey(taskName,objectType,"IMPORT:PATH"),"DEFAULT");
  _configuration.addProperty(createKey(taskName,objectType,"IMPORT:GEN_FILE_NAME"),"NONE");
  _configuration.addProperty(createKey(taskName,objectType,"IMPORT:REC_FILE_NAME"),"NONE");
  _configuration.addProperty(createKey(taskName,objectType,"EXPORT:PATH"),"DEFAULT");
  _configuration.addProperty(createKey(taskName,objectType,"EXPORT:CLOSURE_FILE_NAME"),"NONE");
  _configuration.addProperty(createKey(taskName,objectType,"EXPORT:FORCE_REWRITE"),true);
}

void ClosureCalculator::execute()
{
  if (reportInfo(__FUNCTION__)) { printCR(); }
  const String & taskName = name();
  String objectType = "HISTOGRAM";
  int    method = _configuration.valueInt(createKey(taskName,objectType,"METHOD"),1,0);
  String importPath         = _configuration.valueString(createKey(taskName,objectType,"IMPORT:PATH"));
  String importGenFileName  = _configuration.valueString(createKey(taskName,objectType,"IMPORT:GEN_FILE_NAME"));
  String importRecoFileName = _configuration.valueString(createKey(taskName,objectType,"IMPORT:REC_FILE_NAME"));
  String exportPath         = _configuration.valueString(createKey(taskName,objectType,"EXPORT:PATH"));
  String exportFileName     = _configuration.valueString(createKey(taskName,objectType,"EXPORT:CLOSURE_FILE_NAME"));

  if (reportInfo(__FUNCTION__))
    {
    printCR();
    printValue(createKey(taskName,objectType,"METHOD"),method);
    printValue(createKey(taskName,objectType,"IMPORT:PATH"),importPath);
    printValue(createKey(taskName,objectType,"IMPORT:GEN_FILE_NAME"),importGenFileName);
    printValue(createKey(taskName,objectType,"IMPORT:REC_FILE_NAME"),importRecoFileName);
    printValue(createKey(taskName,objectType,"EXPORT:PATH"),exportPath);
    printValue(createKey(taskName,objectType,"EXPORT:CLOSURE_FILE_NAME"),exportFileName);
    }
  std::unique_ptr<CapFile> generatorFile(CapFile::openOldCapFile(importPath,ManagedObjects<HistogramFolder>::capFileName(importGenFileName)));
  std::unique_ptr<CapFile> detectorFile (CapFile::openOldCapFile(importPath,ManagedObjects<HistogramFolder>::capFileName(importRecoFileName)));
  HistogramFolder generator("GeneratorLevel");
  HistogramFolder detector("DetectorLevel");
  generator.setParentTask(this);
  detector.setParentTask(this);
  generator.importAllHistograms(*generatorFile);
  detector.importAllHistograms(*detectorFile);
  // closure = detector - generator (method 0) or detector/generator (method 1), histogram by histogram
  HistogramFolder closure(detector);
  closure.setName("Closure");
  closure.setParentTask(this);
  switch (method)
    {
      case 0: closure -= generator; break;
      case 1: HistogramFolder::divideHistogramsLists(1.0,detector,1.0,generator,closure); break;
      default: throw Exception("Unknown closure method (0: difference, 1: ratio)",__FUNCTION__);
    }
  bool forceRewrite = _configuration.valueBool(createKey(taskName,objectType,"EXPORT:FORCE_REWRITE"),1,true);
  std::unique_ptr<CapFile> closureFile(forceRewrite
                                       ? CapFile::openRecreateCapFile(exportPath,ManagedObjects<HistogramFolder>::capFileName(exportFileName))
                                       : CapFile::openNewCapFile(exportPath,ManagedObjects<HistogramFolder>::capFileName(exportFileName)));
  closure.saveTo(*closureFile);
  closureFile->write();
  if (reportInfo(__FUNCTION__))
    {
    printCR();
    printString("Closure Test Completed.");
    }
}

} // namespace CAP
