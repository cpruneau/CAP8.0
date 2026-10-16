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
#include "HistogramFolder.hpp"
#include "SubSampleStatCalculator.hpp"
#include "PrintHelpers.hpp"
#include "Histogram.hpp"
#include "Profile.hpp"
#include "NameHelpers.hpp"
#include "Exceptions.hpp"
#include "CapFile.hpp"
#include "RootFileHelpers.hpp"
#include "ManagedObjects.hpp"
#include <memory>

ClassImp(CAP::SubSampleStatCalculator);

namespace CAP
{

  SubSampleStatCalculator::SubSampleStatCalculator()
  :
  Task(),
  _totalTaskExecuted(),
  _eventAccepted(),
  _totalEventAccepted(),
  _importPath(),
  _importFile(),
  _exportPath(),
  _exportFile()
  {
  appendClassName("SubSampleStatCalculator");
  setMinimumReportLevel(Object::kInfo);
  setName("SubSampleStatCalculator");
  }

  SubSampleStatCalculator::SubSampleStatCalculator(const SubSampleStatCalculator & src)
  :
  Task(src),
  _totalTaskExecuted(src._totalTaskExecuted),
  _eventAccepted(src._eventAccepted),
  _totalEventAccepted(src._totalEventAccepted),
  _importPath(src._importPath),
  _importFile(src._importFile),
  _exportPath(src._exportPath),
  _exportFile(src._exportFile)
  { }

  SubSampleStatCalculator & SubSampleStatCalculator::operator=(const SubSampleStatCalculator & rhs)
  {
  if (this!=&rhs)
    {
    Task::operator=(rhs),
    _totalTaskExecuted  = rhs._totalTaskExecuted;
    _eventAccepted      = rhs._eventAccepted;
    _totalEventAccepted = rhs._totalEventAccepted;
    _importPath = rhs._importPath;
    _importFile = rhs._importFile;
    _exportPath = rhs._exportPath;
    _exportFile = rhs._exportFile;
    }
  return *this;
  }

  void SubSampleStatCalculator::setDefaultConfiguration()
  {
  Task::setDefaultConfiguration();
  const String & taskName = name();
  String objectType = "HISTOGRAM";
  // define the parameters (they were read here, before any value could be set)
  _configuration.addProperty(createKey(taskName,objectType,"IMPORT:PATH"),      String("NONE"));
  _configuration.addProperty(createKey(taskName,objectType,"IMPORT:FILE_NAME"), String("NONE"));
  _configuration.addProperty(createKey(taskName,objectType,"EXPORT:PATH"),      String("NONE"));
  _configuration.addProperty(createKey(taskName,objectType,"EXPORT:FILE_NAME"), String("NONE"));
  }

  void SubSampleStatCalculator::execute()
  {
  const String & taskName = name();
  String objectType = "HISTOGRAM";
  _importPath  = _configuration.valueString(createKey(taskName,objectType,"IMPORT:PATH"));
  _importFile  = _configuration.valueString(createKey(taskName,objectType,"IMPORT:FILE_NAME"));
  _exportPath  = _configuration.valueString(createKey(taskName,objectType,"EXPORT:PATH"));
  _exportFile  = _configuration.valueString(createKey(taskName,objectType,"EXPORT:FILE_NAME"));
  if (reportDebug(__FUNCTION__))
    {
    printCR();
    printValue(createKey(taskName,objectType,"IMPORT:PATH"),_importPath);
    printValue(createKey(taskName,objectType,"IMPORT:FILE_NAME"),_importFile);
    printValue(createKey(taskName,objectType,"EXPORT:PATH"),_exportPath);
    printValue(createKey(taskName,objectType,"EXPORT:FILE_NAME"),_exportFile);
    }
  bool verbose = false;
  std::vector<String> directories = listDirsIn(_importPath,verbose);
  int nDirectories = directories.size();
  if (directories.size()<2) throw Exception("directories.size()<2",__FUNCTION__);
  if (reportDebug(__FUNCTION__))
    {
    printCR();
    printValue("directories.size()",nDirectories);
    }
  // Running weighted average of the histograms of all sub-samples (see HistogramFolder::squareDifferenceLists()),
  // each sub-sample weighted by its number of executed tasks.
  HistogramFolder groupAverage("Average");
  int iFile = 0;
  double sumWeights = 0.0;
  _totalTaskExecuted.initialize("Executed",1);
  _totalEventAccepted.initialize("EventAccepted",0);
  for (auto & name : directories)
    {
    String path = _importPath; path += "/"; path += name;
    if (reportDebug(__FUNCTION__))
      {
      printCR();
      printValue(createKey(taskName,objectType,"IMPORT:PATH"),_importPath);
      printValue(createKey(taskName,objectType,"IMPORT:FILE_NAME"),_importFile);
      printValue("File index",iFile);
      }
    std::unique_ptr<CapFile> file(CapFile::openOldCapFile(path,ManagedObjects<HistogramFolder>::capFileName(_importFile)));
    double weight = 1.0;
    try
      {
      _taskExecuted.initialize("Executed",0);
      file->importAccountant(_taskExecuted);
      _totalTaskExecuted.increment(_taskExecuted);
      weight = double(_taskExecuted.valueAt(0));
      _eventAccepted.initialize("EventAccepted",0);
      file->importAccountant(_eventAccepted);
      if (_totalEventAccepted.size()==0) _totalEventAccepted = _eventAccepted;
      else                               _totalEventAccepted.increment(_eventAccepted);
      }
    catch (Exception & e)
      {
      if (reportWarning(__FUNCTION__)) std::cout << "No task/event accountants in " << path << ": sub-sample weight set to 1" << std::endl;
      }
    if (iFile==0)
      {
      groupAverage.importAllHistograms(*file);
      }
    else
      {
      HistogramFolder group;
      group.importAllHistograms(*file);
      int n = (iFile==(nDirectories-1)) ? nDirectories-1 : -iFile;
      groupAverage.squareDifferenceLists(group,sumWeights,weight,n);
      }
    sumWeights += weight;
    iFile++;
    }

  std::unique_ptr<CapFile> outputFile(CapFile::openRecreateCapFile(_exportPath,ManagedObjects<HistogramFolder>::capFileName(_exportFile)));
  outputFile->exportAccountant(_totalTaskExecuted);
  if (_totalEventAccepted.size()>0) outputFile->exportAccountant(_totalEventAccepted);
  groupAverage.saveTo(*outputFile);
  outputFile->write();
  }

} // namespace CAP

