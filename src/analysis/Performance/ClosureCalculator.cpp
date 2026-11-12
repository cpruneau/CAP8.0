/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
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
#include "FileSystem.hpp"
#include "Exceptions.hpp"
#include "MathExceptions.hpp"
#include "StringParameter.hpp"


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
  _configuration.addProperty(createKey(taskName,objectType,"TEST"),int(kNoTest));
  _configuration.addProperty(createKey(taskName,objectType,"IMPORT:PATH"),"DEFAULT");
  _configuration.addProperty(createKey(taskName,objectType,"IMPORT:GEN_FILE_NAME"),"NONE");
  _configuration.addProperty(createKey(taskName,objectType,"IMPORT:GEN_FOLDER"),"");
  _configuration.addProperty(createKey(taskName,objectType,"IMPORT:REC_FILE_NAME"),"NONE");
  _configuration.addProperty(createKey(taskName,objectType,"IMPORT:REC_FOLDER"),"");
  _configuration.addProperty(createKey(taskName,objectType,"EXPORT:PATH"),"DEFAULT");
  _configuration.addProperty(createKey(taskName,objectType,"EXPORT:CLOSURE_FILE_NAME"),"NONE");
  _configuration.addProperty(createKey(taskName,objectType,"EXPORT:FORCE_REWRITE"),true);
}

void ClosureCalculator::execute()
{
  if (reportInfo(__FUNCTION__)) { printCR(); }
  if (_taskExecuted.size()==0) _taskExecuted.initialize("taskExecuted",1);
  _taskExecuted.increment();
  const String & taskName = name();
  String objectType = "HISTOGRAM";
  int    method = _configuration.valueInt(createKey(taskName,objectType,"METHOD"),1,0);
  int    test   = _configuration.valueInt(createKey(taskName,objectType,"TEST"),1,kNoTest);
  String importPath         = _configuration.valueString(createKey(taskName,objectType,"IMPORT:PATH"));
  String importGenFileName  = _configuration.valueString(createKey(taskName,objectType,"IMPORT:GEN_FILE_NAME"));
  String importGenFolder    = _configuration.valueString(createKey(taskName,objectType,"IMPORT:GEN_FOLDER"));
  String importRecoFileName = _configuration.valueString(createKey(taskName,objectType,"IMPORT:REC_FILE_NAME"));
  String importRecoFolder   = _configuration.valueString(createKey(taskName,objectType,"IMPORT:REC_FOLDER"));
  String exportPath         = _configuration.valueString(createKey(taskName,objectType,"EXPORT:PATH"));
  String exportFileName     = _configuration.valueString(createKey(taskName,objectType,"EXPORT:CLOSURE_FILE_NAME"));
  bool   forceRewrite       = _configuration.valueBool(createKey(taskName,objectType,"EXPORT:FORCE_REWRITE"),1,true);
  for (const String * fileName : { &importGenFileName, &importRecoFileName, &exportFileName })
    if (fileName->isNull() || fileName->equalTo("NONE"))
      throw Exception("Set the file names: HISTOGRAM:IMPORT:GEN_FILE_NAME, IMPORT:REC_FILE_NAME and "
                      "EXPORT:CLOSURE_FILE_NAME",__FUNCTION__);
  calculate(method,test,
            FileSystem::configuredPath(importPath,"CAP_HISTOS_IMPORT_PATH"),importGenFileName,importGenFolder,
            importRecoFileName,importRecoFolder,
            FileSystem::configuredPath(exportPath,"CAP_HISTOS_EXPORT_PATH"),exportFileName,forceRewrite);
}

namespace
{
  ClosureCalculator::UserTest & theUserTest()
  {
  static ClosureCalculator::UserTest test;
  return test;
  }
}

void ClosureCalculator::setUserTest(const UserTest & test) { theUserTest() = test; }

const ClosureCalculator::UserTest & ClosureCalculator::userTest() { return theUserTest(); }

MATH::TestResult ClosureCalculator::runTest(int test, const MATH::Histogram & gen, const MATH::Histogram & reco)
{
  switch (test)
    {
    case kChi2Test:
      return MATH::histogramCompatibilityTest(reco,gen,MATH::HistogramComparison::kValues);
    case kKolmogorovTest:
      {
      // the sample sizes are the numbers of entries: each histogram scaled to its entries (D is unchanged)
      MATH::Histogram g(gen), r(reco);
      for (MATH::Histogram * h : { &g, &r })
        {
        long entries = 0;
        for (long e : h->binEntries()) entries += e;
        double sum = 0.0;
        for (unsigned int k=0; k<h->size(); k++) sum += h->valueAt(k);
        if (entries>0 && sum>0.0) h->scale(double(entries)/sum);
        }
      return MATH::kolmogorovSmirnovTest(r,g);
      }
    case kUserTest:
      if (!theUserTest())
        throw Exception("No user test: call ClosureCalculator::setUserTest() first","ClosureCalculator::runTest");
      return theUserTest()(gen,reco);
    default:
      throw Exception("Unknown statistical test (1: chi2, 2: Kolmogorov, 3: user defined)",
                      "ClosureCalculator::runTest");
    }
}

unsigned int ClosureCalculator::calculateClosureFor(const Folder & gen, const Folder & reco, Folder & out, int method,
                                                    int test, std::vector<String> & skipped, const String & path,
                                                    std::vector<TestRecord> * results)
{
  unsigned int n = 0;
  for (const String & name : gen.names())
    {
    String entryPath = path.isNull() ? name : path+"/"+name;
    if (Folder * genSub = gen.folder(name))
      {
      Folder * recoSub = reco.folder(name);
      if (!recoSub) { skipped.push_back(entryPath+"/"); continue; }
      n += calculateClosureFor(*genSub,*recoSub,*out.addFolder(name),method,test,skipped,entryPath,results);
      continue;
      }
    const MATH::Histogram * g = gen.get<MATH::Histogram>(name);
    if (!g) continue;                                        // not a histogram: parameters, counters, ...
    const MATH::Histogram * r = reco.get<MATH::Histogram>(name);
    if (!r) { skipped.push_back(entryPath); continue; }
    // plain histograms (a profile is compared by its averages)
    MATH::Histogram genValues(*g);
    MATH::Histogram recoValues(*r);
    MATH::Histogram * closure = new MATH::Histogram(recoValues);
    if (method==0)
      closure->add(genValues,-1.0);
    else
      closure->setAsRatioOf(recoValues,genValues);
    closure->setName(name);
    out.add(closure,name);
    if (test!=kNoTest)
      {
      MATH::TestResult result;
      String text;
      try
        {
        result = runTest(test,genValues,recoValues);
        text = result.summary();
        }
      catch (const MATH::MathException & e)    // this histogram cannot be tested (rank, no bin compared, ...)
        {
        result = MATH::TestResult();
        result.nullHypothesis = String(e.what());
        text = String("not tested: ")+String(e.what());
        }
      String testName = name+"_TEST";
      out.add(new StringParameter(testName,text),testName);
      if (results) results->push_back(TestRecord(entryPath,result));
      }
    n++;
    }
  return n;
}

unsigned int ClosureCalculator::calculate(int method,
                                          int test,
                                          const String & importDirectory,
                                          const String & genFileName,
                                          const String & genFolder,
                                          const String & recoFileName,
                                          const String & recoFolder,
                                          const String & exportDirectory,
                                          const String & closureFileName,
                                          bool forceRewrite)
{
  if (method!=0 && method!=1) throw Exception("Unknown closure method (0: difference, 1: ratio)",__FUNCTION__);
  if (test<kNoTest || test>kUserTest)
    throw Exception("Unknown statistical test (0: none, 1: chi2, 2: Kolmogorov, 3: user defined)",__FUNCTION__);
  if (reportInfo(__FUNCTION__))
    {
    printCR();
    printValue("METHOD",method);
    printValue("TEST",test);
    printValue("Generator-level file",FileSystem::join(importDirectory,genFileName));
    printValue("Generator-level folder",genFolder);
    printValue("Detector-level file",FileSystem::join(importDirectory,recoFileName));
    printValue("Detector-level folder",recoFolder);
    printValue("Closure file",FileSystem::join(exportDirectory,closureFileName));
    }
  std::unique_ptr<CapFile> generatorFile(CapFile::openOldCapFile(importDirectory,ManagedObjects<HistogramFolder>::capFileName(genFileName)));
  std::unique_ptr<CapFile> detectorFile (CapFile::openOldCapFile(importDirectory,ManagedObjects<HistogramFolder>::capFileName(recoFileName)));
  const Folder & genTop  = generatorFile->folder();
  const Folder & recoTop = detectorFile->folder();
  const Folder * gen  = genFolder.isNull()  ? &genTop  : genTop.folder(genFolder);
  const Folder * reco = recoFolder.isNull() ? &recoTop : recoTop.folder(recoFolder);
  if (!gen)  throw Exception(String("No folder ")+genFolder+" in "+genFileName,__FUNCTION__);
  if (!reco) throw Exception(String("No folder ")+recoFolder+" in "+recoFileName,__FUNCTION__);

  String outputName = ManagedObjects<HistogramFolder>::capFileName(closureFileName);
  std::unique_ptr<CapFile> closureFile(forceRewrite
                                       ? CapFile::openRecreateCapFile(exportDirectory,outputName)
                                       : CapFile::openNewCapFile(exportDirectory,outputName));
  if (test==kUserTest && !theUserTest())
    throw Exception("TEST 3 (user defined): no user test, call ClosureCalculator::setUserTest() first",__FUNCTION__);
  std::vector<String> skipped;
  _testResults.clear();
  std::vector<TestRecord> & results = _testResults;
  unsigned int n = calculateClosureFor(*gen,*reco,closureFile->folder(),method,test,skipped,"",&results);
  if (n==0) throw Exception("No histogram of the generator-level tree has a counterpart in the detector-level tree",__FUNCTION__);
  closureFile->write();
  if (reportInfo(__FUNCTION__))
    {
    printCR();
    printValue("Histograms compared",long(n));
    printValue("Histograms without counterpart (skipped)",long(skipped.size()));
    for (const String & s : skipped) printString(String("  ")+s);
    if (test!=kNoTest)
      {
      long tested = 0, failed = 0;
      for (const TestRecord & r : results)
        {
        if (r.second.test.isNull())
          {
          printString(String("  ")+r.first+": not tested: "+r.second.nullHypothesis);
          continue;
          }
        tested++;
        if (r.second.rejected(0.05)) failed++;
        printString(String("  ")+r.first+": "+r.second.summary());
        }
      printValue("Histograms tested",tested);
      printValue("Closure rejected at the 5% level",failed);
      }
    printString("Closure Test Completed.");
    }
  return n;
}

} // namespace CAP
