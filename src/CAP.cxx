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
#include <iostream>
#include <fstream>
#include <TStyle.h>
#include <TROOT.h>
#include <TRandom.h>
#include "Configuration.hpp"
#include "PrintHelpers.hpp"
#include "Task.hpp"
#include "XmlDocument.hpp"
#include "Exceptions.hpp"
#include "MathExceptions.hpp"
//#include "Histogram.hpp"
#include "Timer.hpp"
#include "TH1D.h"

void usage()
{
  std::cout << "CAP" << std::endl;
  std::cout << "Version 1.0" << std::endl << std::endl << std::endl;
  std::cout << "Usage:" << std::endl;
  std::cout << "CAP  configurationFile" << std::endl;
  std::cout << "CAP  configurationPath configurationFile" << std::endl;
  std::cout << "CAP  configurationPath configurationFile histogramPath" << std::endl<< std::endl;
  std::cout << "A configuration file ending with .xml is read as XML: the task named taskName (or the first TASK" << std::endl;
  std::cout << "element if taskName is -) is created from its TASK element." << std::endl << std::endl;
  std::cout << "CAP  --xml-templates outputPath [className ...]" << std::endl;
  std::cout << "Write the XML template (all parameters, defaults and types) of the given task classes, or of all" << std::endl;
  std::cout << "task classes, to outputPath/<ClassName>.xml." << std::endl << std::endl << std::endl;
}

void parse(int argc,
           char* argv[],
           CAP::String &  taskName,
           CAP::String &  configurationPath,
           CAP::String &  configurationFile,
           CAP::String &  histogramPath)
{
  CAP::String configurationPathDefault  = "Test/";
  CAP::String configurationFileDefault  = "test.ini";
  CAP::String histogramPathDefault      = "test/";

  CAP::String hPath;
  CAP::String cPath;
  switch (argc)
    {
      default:
      case 1: usage(); exit(0);

      case 2:
      taskName          = argv[1];
      cPath             = configurationPathDefault;
      configurationFile = configurationFileDefault;
      hPath             = histogramPathDefault;
      break;

      case 3:
      taskName          = argv[1];
      cPath             = "";
      configurationFile = argv[2];
      hPath             = histogramPathDefault;
      break;

      case 4:
      taskName          = argv[1];
      cPath             = argv[2];
      configurationFile = argv[3];
      hPath             = histogramPathDefault;
      break;

      case 5:
      taskName          = argv[1];
      cPath             = argv[2];
      configurationFile = argv[3];
      hPath             = argv[4];
      break;
    }

  if (cPath.beginsWith("/"))
    {
    configurationPath = cPath;
    }
  else
    {
    CAP::String base = getenv("CAP_PROJECTS_PATH");
    configurationPath = base;
    configurationPath += "/";
    configurationPath += cPath;
    configurationPath += "/";
    }

  if (hPath.beginsWith("/"))
    {
    histogramPath = hPath;
    }
  else
    {
    histogramPath = getenv("CAP_HISTOS_IMPORT_PATH");
    histogramPath += "/";
    histogramPath += hPath;
    histogramPath += "/";
    }
  setenv("CAP_HISTOS_IMPORT_PATH",histogramPath,1);
  setenv("CAP_HISTOS_EXPORT_PATH",histogramPath,1);
  CAP::printValue("Configuration path",configurationPath);
  CAP::printValue("Configuration file",configurationFile);
  CAP::printValue("CAP_HISTOS_IMPORT_PATH:",histogramPath);
  CAP::printValue("CAP_HISTOS_EXPORT_PATH:",histogramPath);

}

int main(int argc, char* argv[])
{
  if (argc>=3 && CAP::String(argv[1])=="--xml-templates")
    {
    try
      {
      std::vector<CAP::String> classNames;
      for (int k=3; k<argc; k++) classNames.push_back(argv[k]);
      std::vector<CAP::String> files = CAP::Task::exportXmlTemplates(argv[2],classNames);
      for (auto & f : files) CAP::printValue("Template written",f);
      return 0;
      }
    catch (CAP::Exception & exception)
      {
      exception.print();
      return 1;
      }
    }
  CAP::String mainTaskName;
  CAP::String configurationPath;
  CAP::String configurationFile;
  CAP::String histogramPath;
  parse(argc,argv,mainTaskName,configurationPath,configurationFile,histogramPath);
  long seed=112133;
  if (seed!=0)  gRandom->SetSeed(seed);
 try
  {
  CAP::Task * creatorTask = new CAP::Task();
  CAP::Task * mainTask = nullptr;
  if (configurationFile.endsWith(".xml"))
    {
    CAP::XmlDocument document = CAP::XmlDocument::read(configurationPath,configurationFile);
    const CAP::XmlElement * taskElement = (mainTaskName=="-") ? document.findFirst("TASK")
                                                                : document.findFirst("TASK","name",mainTaskName);
    if (!taskElement) throw CAP::ConfigurationException(mainTaskName,"No such TASK element in "+configurationFile,"main()");
    mainTask = creatorTask->createTask(nullptr,*taskElement);
    }
  else
    {
    CAP::Configuration configuration;
    configuration.loadFrom(configurationPath,configurationFile);
    mainTask = creatorTask->createTask(nullptr,mainTaskName,configuration);
    }
  mainTask->initialize();
  mainTask->execute();
  mainTask->finalize();
  delete mainTask;
  delete creatorTask;
  CAP::printLine();
  CAP::printLine();

//  CAP::Timer timer;
//
//  timer.start();
//  CAP::Histogram * h = new CAP::Histogram(CAP::String("test"),CAP::String("test"),100,0.0, 100.0);
//  long n = 400000000;
//  CAP::printLine();
//  CAP::printLine();
//  CAP::printLine();
//  CAP::printLine();
//  cout << n << endl;
//  for (long k=0; k<n; k++)
//    {
//   // if (k%1000) cout << k << endl;
//    h->fill(50.0,1.0);
//    }
//  timer.stop();
//  timer.print();
//  CAP::printString("Part I  completed successfully");
//  timer.start();
//  TH1 * hh = new TH1D(CAP::String("test"),CAP::String("test"),100,0.0, 100.0);
//  for (long k=0; k<n; k++)
//    {
//    //if (k%1000) cout << k << endl;
//    hh->Fill(50.0,1.0);
//    }
//  timer.stop();
//  timer.print();



  CAP::printString("CAP completed successfully");
  CAP::printLine();
  CAP::printLine();
  

  return 0;
  }

  catch (CAP::ConfigurationException & exception)
  {
  exception.print();
  }
  catch (CAP::MATH::MathException & exception)
  {
  exception.print();
  }
  catch (CAP::FileException & exception)
  {
  exception.print();
  }
  catch (CAP::MemoryException & exception)
  {
  exception.print();
  }
  catch (CAP::HistogramException & exception)
  {
  exception.print();
  }
  catch (CAP::FactoryException & exception)
  {
  exception.print();
  }
  catch (CAP::Exception & exception)
  {
  exception.print();
  }
  CAP::printLine();
  CAP::printString("CAP FAILED");
  CAP::printLine();
  return 1;
}

