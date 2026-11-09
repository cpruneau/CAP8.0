/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "FunctionRegistry.hpp"
#include <iostream>
#include <fstream>
#include <TStyle.h>
#include <TROOT.h>
#include <TRandom.h>
#include "Configuration.hpp"
#include "PrintHelpers.hpp"
#include "Task.hpp"
#include "CapFile.hpp"
#include "NameHelpers.hpp"
#include <memory>
#include "XmlDocument.hpp"
#include "Exceptions.hpp"
#include "FileSystem.hpp"
#include "MathExceptions.hpp"
//#include "Histogram.hpp"
#include "Timer.hpp"
#include "ParticleEfficiency.hpp"
#include "ParticleSmear.hpp"
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
  std::cout << "task classes, to outputPath/<ClassName>.xml." << std::endl << std::endl;
  std::cout << "CAP  --function-catalog outputFile" << std::endl;
  std::cout << "Write the catalog of the functions of MATH::FunctionRegistry (names, ranks, parameters) to outputFile." << std::endl << std::endl << std::endl;
}

//!
//! Name of a file of the job: a CAP file (".cap"; a name ending with ".root" is read and written as ".cap") in the
//! histogram directory, unless absolute.
//!
CAP::String jobFile(const CAP::String & name, const CAP::String & histogramPath, CAP::String & path)
{
  CAP::String fileName = name;
  if (fileName.endsWith(".root")) fileName = fileName.subString(0,fileName.length()-5)+CAP::String(".cap");
  path = fileName.beginsWith("/") ? CAP::String("") : histogramPath;
  return fileName;
}

void parse(int argc,
           char* argv[],
           CAP::String &  taskName,
           CAP::String &  configurationPath,
           CAP::String &  configurationFile,
           CAP::String &  histogramPath)
{
  CAP::String configurationPathDefault  = "Test/";
  CAP::String configurationFileDefault  = "test.xml";
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

  // absolute, "./..." or relative to $CAP_PROJECTS_PATH, $CAP_HISTOS_IMPORT_PATH
  configurationPath = CAP::FileSystem::configuredPath(cPath,"CAP_PROJECTS_PATH");
  histogramPath     = CAP::FileSystem::configuredPath(hPath,"CAP_HISTOS_IMPORT_PATH");
  if (!histogramPath.endsWith("/")) histogramPath += "/";
  setenv("CAP_HISTOS_IMPORT_PATH",histogramPath,1);
  setenv("CAP_HISTOS_EXPORT_PATH",histogramPath,1);
  CAP::printValue("Configuration path",configurationPath);
  CAP::printValue("Configuration file",configurationFile);
  CAP::printValue("CAP_HISTOS_IMPORT_PATH:",histogramPath);
  CAP::printValue("CAP_HISTOS_EXPORT_PATH:",histogramPath);

}

int main(int argc, char* argv[])
{
  if (argc>=3 && CAP::String(argv[1])=="--function-catalog")
    {
    CAP::String composites = CAP::MATH::FunctionRegistry::defaultCompositesFile();
    std::ifstream registered(composites.data());
    if (registered) CAP::MATH::FunctionRegistry::readComposites(composites);   // those registered by CapGui
    if (!CAP::MATH::FunctionRegistry::writeCatalog(argv[2])) { std::cout << "Cannot write " << argv[2] << std::endl; return 1; }
    CAP::printValue("Function catalog written",CAP::String(argv[2]));
    return 0;
    }
  if (argc>=3 && CAP::String(argv[1])=="--maps-list")
    {
    // the particle efficiencies and smearers of a maps file (CapGui, Detector Maps), for RunCAP.py: one line each,
    // MAP <tab> kind <tab> name <tab> species (PDG codes, comma separated) <tab> maps (key=summary, ...)
    try
      {
      std::unique_ptr<CAP::CapFile> file(CAP::CapFile::openOldCapFile(argv[2]));
      const CAP::Folder & top = file->folder();
      auto list = [&](CAP::KinematicMapFolder & object, const std::vector<int> & species)
        {
        std::cout << "MAP\t" << object.kindName() << "\t" << object.name() << "\t";
        for (std::size_t k=0; k<species.size(); k++) std::cout << (k ? "," : "") << species[k];
        std::cout << "\t";
        for (unsigned int k=0; k<object.nMaps(); k++)
          {
          const CAP::KinematicMap & map = object.map(k);
          const char * model[] = { "none", "constant", "function", "histogram" };
          std::cout << (k ? ", " : "") << object.mapKey(k) << "=" << model[int(map.model())];
          }
        std::cout << std::endl;
        };
      for (const CAP::String & name : CAP::KinematicMapFolder::objectNames(top,"PARTICLE_EFFICIENCY"))
        {
        CAP::ParticleEfficiency o;
        o.setName(name);
        o.loadFrom(top);
        list(o,o.species());
        }
      for (const CAP::String & name : CAP::KinematicMapFolder::objectNames(top,"PARTICLE_SMEARER"))
        {
        CAP::ParticleSmear o;
        o.setName(name);
        o.loadFrom(top);
        list(o,std::vector<int>());
        }
      return 0;
      }
    catch (CAP::Exception & exception)
      {
      exception.print();
      return 1;
      }
    }
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
  // Files of the job: the histograms of all its tasks are written to one output file (<main>:JOB:OUTPUT_FILE, by
  // default <main>.cap), a folder per task holding a folder per combination of filters; those imported are read from
  // one input file (<main>:JOB:INPUT_FILE) organized in the same way. Both are in the histogram directory.
  const CAP::Configuration & jobConfiguration = mainTask->configuration();
  CAP::String outputName = jobConfiguration.valueString(CAP::createKey(mainTask->name(),"JOB:OUTPUT_FILE"),
                                                        true,mainTask->name()+CAP::String(".cap"));
  CAP::String inputName  = jobConfiguration.valueString(CAP::createKey(mainTask->name(),"JOB:INPUT_FILE"),true,"NONE");
  // Seed of gRandom (<main>:JOB:SEED, default 112133): the sub-samples of RunCAP.py are given different seeds.
  CAP::String seedText = jobConfiguration.valueString(CAP::createKey(mainTask->name(),"JOB:SEED"),true,"112133");
  long seed = 112133;
  try { seed = std::stol(seedText.str()); }
  catch (const std::exception &) { throw CAP::ConfigurationException(mainTask->name(),"Invalid JOB:SEED "+seedText,"main()"); }
  if (seed!=0) gRandom->SetSeed(seed);
  CAP::printValue("Random seed (gRandom)",seed);
  mainTask->initialize();
  if (!inputName.isEmpty() && inputName!="NONE")
    {
    CAP::String path;
    CAP::String name = jobFile(inputName,histogramPath,path);
    CAP::printValue("Input file of the job",CAP::CapFile::makeFileName(path,name));
    std::unique_ptr<CAP::CapFile> input(CAP::CapFile::openOldCapFile(path,name));
    mainTask->loadHistograms(*input);
    }
  mainTask->execute();
  mainTask->finalize();
  if (!outputName.isEmpty() && outputName!="NONE")
    {
    CAP::String path;
    CAP::String name = jobFile(outputName,histogramPath,path);
    std::unique_ptr<CAP::CapFile> output(CAP::CapFile::openRecreateCapFile(path,name));
    mainTask->saveHistograms(*output);
    if (output->names().empty())
      CAP::printString("No histograms: no output file");
    else
      {
      output->write();
      CAP::printValue("Output file of the job",CAP::CapFile::makeFileName(path,name));
      }
    }
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

