/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "FileListing.hpp"
#include "ClosureIterator.hpp"
#include "ClosureCalculator.hpp"
#include "PrintHelpers.hpp"
#include "NameHelpers.hpp"
#include "Exceptions.hpp"
#include "FileSystem.hpp"


namespace CAP
{

ClosureIterator::ClosureIterator()
:
Task()
{
  appendClassName("ClosureIterator");
  setMinimumReportLevel(Object::kInfo);
}

void ClosureIterator::setDefaultConfiguration()
{
  Task::setDefaultConfiguration();
  const String & taskName = name();
  String objectType = "HISTOGRAM";
  _configuration.addProperty(createKey(taskName,objectType,"METHOD"),int(0));
  _configuration.addProperty(createKey(taskName,objectType,"TEST"),int(ClosureCalculator::kNoTest));
  _configuration.addProperty(createKey(taskName,objectType,"IMPORT:PATH"),"DEFAULT");
  _configuration.addProperty(createKey(taskName,objectType,"IMPORT:GEN_FOLDER"),"");
  _configuration.addProperty(createKey(taskName,objectType,"IMPORT:REC_FOLDER"),"");
  _configuration.addProperty(createKey(taskName,objectType,"EXPORT:PATH"),"DEFAULT");
  _configuration.addProperty(createKey(taskName,objectType,"EXPORT:FORCE_REWRITE"),true);
  String path = createKey(taskName,objectType);
  _configuration.generateKeyValuePairs(path.data(),"IncludedPattern","NONE",5);
  _configuration.generateKeyValuePairs(path.data(),"ExcludedPattern","NONE",5);
}

void ClosureIterator::execute()
{
  if (reportStart(__FUNCTION__)) { /* no ops*/ };
  if (_taskExecuted.size()==0) _taskExecuted.initialize("taskExecuted",1);
  _taskExecuted.increment();
  const String & taskName = name();
  String objectType = "HISTOGRAM";
  int    method       = _configuration.valueInt(createKey(taskName,objectType,"METHOD"),1,0);
  int    test         = _configuration.valueInt(createKey(taskName,objectType,"TEST"),1,ClosureCalculator::kNoTest);
  String importPath   = FileSystem::configuredPath(_configuration.valueString(createKey(taskName,objectType,"IMPORT:PATH")),
                                                   "CAP_HISTOS_IMPORT_PATH");
  String genFolder    = _configuration.valueString(createKey(taskName,objectType,"IMPORT:GEN_FOLDER"));
  String recoFolder   = _configuration.valueString(createKey(taskName,objectType,"IMPORT:REC_FOLDER"));
  String exportPath   = FileSystem::configuredPath(_configuration.valueString(createKey(taskName,objectType,"EXPORT:PATH")),
                                                   "CAP_HISTOS_EXPORT_PATH");
  bool   forceRewrite = _configuration.valueBool(createKey(taskName,objectType,"EXPORT:FORCE_REWRITE"),1,true);
  String path = createKey(taskName,objectType);
  std::vector<String> included = _configuration.selectedValues(path.data(),"IncludedPattern","NONE");
  std::vector<String> excluded = _configuration.selectedValues(path.data(),"ExcludedPattern","NONE");
  run(method,test,importPath,genFolder,recoFolder,exportPath,forceRewrite,included,excluded);
  if (reportEnd(__FUNCTION__)) { /* no ops*/ };
}

std::vector<std::size_t> ClosureIterator::tagPositions(const String & fileName, const String & tag)
{
  std::vector<std::size_t> positions;
  const std::string name = fileName.data(), t = tag.data();
  if (t.empty()) return positions;
  for (std::size_t p = name.find(t); p!=std::string::npos; p = name.find(t,p+1))
    {
    std::size_t end = p+t.size();
    if (end==name.size() || name[end]=='_' || name[end]=='.') positions.push_back(p);
    }
  return positions;
}

String ClosureIterator::counterpart(const String & genFileName, const String & tag)
{
  std::vector<std::size_t> positions = tagPositions(genFileName,"_Gen");
  if (positions.size()!=1) return String("");
  std::string name = genFileName.data();
  name.replace(positions[0],4,tag.data());
  return String(name.c_str());
}

ClosureIterator::Summary ClosureIterator::run(int method, int test, const String & importDirectory,
                                              const String & genFolder, const String & recoFolder,
                                              const String & exportDirectory, bool forceRewrite,
                                              const std::vector<String> & included,
                                              const std::vector<String> & excluded)
{
  // the CAP files of the directory (by name; names relative to it, no sub-directory) with the patterns asked for,
  // holding the tag _Gen once, and neither _Reco nor _Closure
  std::vector<String> files;
  for (const String & name : listFilesInDir(importDirectory,included,excluded,false,false,0))
    if (tagPositions(name,"_Gen").size()==1 && tagPositions(name,"_Reco").empty()
        && tagPositions(name,"_Closure").empty())
      files.push_back(name);
  if (files.empty())
    throw Exception(String("No generator-level file (a .cap file with the tag _Gen) selected in ")+importDirectory,
                    __FUNCTION__);
  if (reportInfo(__FUNCTION__))
    {
    printCR();
    printValue("METHOD",method);
    printValue("TEST",test);
    printValue("IMPORT:PATH",importDirectory);
    printValue("EXPORT:PATH",exportDirectory);
    printValue("Files",long(files.size()));
    }
  Summary summary;
  ClosureCalculator calculator;
  calculator.setName(name()+"_Closure");
  calculator.setMinimumReportLevel(minimumReportLevel());
  for (const String & genFile : files)
    {
    String recoFile    = counterpart(genFile,"_Reco");
    String closureFile = counterpart(genFile,"_Closure");
    String reason;
    if (!FileSystem::isFile(FileSystem::join(importDirectory,recoFile)))
      reason = String("no detector-level file ")+recoFile;
    else
      {
      try
        {
        summary.nHistograms += calculator.calculate(method,test,importDirectory,genFile,genFolder,recoFile,recoFolder,
                                                    exportDirectory,closureFile,forceRewrite);
        summary.compared.push_back(genFile);
        for (const ClosureCalculator::TestRecord & r : calculator.testResults())
          {
          if (r.second.test.isNull()) continue;                  // not tested
          summary.nTested++;
          if (r.second.rejected(0.05)) summary.nRejected++;
          }
        continue;
        }
      catch (const Exception & e)
        {
        reason = String(e.what());
        }
      }
    summary.skipped.push_back(genFile+": "+reason);
    if (reportWarning(__FUNCTION__))
      {
      printCR();
      printString(String("Skipped ")+genFile+": "+reason);
      }
    }
  if (reportInfo(__FUNCTION__))
    {
    printCR();
    printValue("Files compared",long(summary.compared.size()));
    printValue("Files skipped",long(summary.skipped.size()));
    for (const String & s : summary.skipped) printString(String("  ")+s);
    printValue("Histograms compared",summary.nHistograms);
    if (test!=ClosureCalculator::kNoTest)
      {
      printValue("Histograms tested",summary.nTested);
      printValue("Closure rejected at the 5% level",summary.nRejected);
      }
    }
  if (summary.compared.empty())
    throw Exception(String("No file could be compared (")+String(std::to_string(summary.skipped.size()).c_str())
                    +" skipped, see the warnings)",__FUNCTION__);
  return summary;
}

} // namespace CAP
