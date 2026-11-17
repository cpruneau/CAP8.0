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
#include "MersenneTwister.hpp"
#include "Configuration.hpp"
#include "PrintHelpers.hpp"
#include "Task.hpp"
#include "EventProcessor.hpp"
#include "DatumFile.hpp"
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

void usage()
{
  std::cout << "Datum (formerly CAP)" << std::endl;
  std::cout << "Version 1.0" << std::endl << std::endl << std::endl;
  std::cout << "Usage:" << std::endl;
  std::cout << "Datum  configurationFile" << std::endl;
  std::cout << "Datum  configurationPath configurationFile" << std::endl;
  std::cout << "Datum  configurationPath configurationFile histogramPath" << std::endl<< std::endl;
  std::cout << "A configuration file ending with .xml is read as XML: the task named taskName (or the first TASK" << std::endl;
  std::cout << "element if taskName is -) is created from its TASK element." << std::endl << std::endl;
  std::cout << "Datum  --xml-templates outputPath [className ...]" << std::endl;
  std::cout << "Write the XML template (all parameters, defaults and types) of the given task classes, or of all" << std::endl;
  std::cout << "task classes, to outputPath/<ClassName>.xml." << std::endl << std::endl;
  std::cout << "Datum  --function-catalog outputFile" << std::endl;
  std::cout << "Write the catalog of the functions of MATH::FunctionRegistry (names, ranks, parameters) to outputFile." << std::endl << std::endl << std::endl;
}

//!
//! Name of a file of the job: a Datum file (".datum", or ".cap" for those of CAP; a name ending with ".root" is read and written as ".datum") in the
//! histogram directory, unless absolute.
//!
Datum::String jobFile(const Datum::String & name, const Datum::String & histogramPath, Datum::String & path)
{
  Datum::String fileName = name;
  if (fileName.endsWith(".root")) fileName = fileName.subString(0,fileName.length()-5)+Datum::String(Datum::DatumFile::extension());
  path = fileName.beginsWith("/") ? Datum::String("") : histogramPath;
  return fileName;
}

void parse(int argc,
           char* argv[],
           Datum::String &  taskName,
           Datum::String &  configurationPath,
           Datum::String &  configurationFile,
           Datum::String &  histogramPath)
{
  Datum::String configurationPathDefault  = "Test/";
  Datum::String configurationFileDefault  = "test.xml";
  Datum::String histogramPathDefault      = "test/";

  Datum::String hPath;
  Datum::String cPath;
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

  // absolute, "./..." or relative to $DATUM_PROJECTS_PATH, $DATUM_HISTOS_IMPORT_PATH
  configurationPath = Datum::FileSystem::configuredPath(cPath,"DATUM_PROJECTS_PATH");
  histogramPath     = Datum::FileSystem::configuredPath(hPath,"DATUM_HISTOS_IMPORT_PATH");
  if (!histogramPath.endsWith("/")) histogramPath += "/";
  setenv("DATUM_HISTOS_IMPORT_PATH",histogramPath,1);
  setenv("DATUM_HISTOS_EXPORT_PATH",histogramPath,1);
  Datum::printValue("Configuration path",configurationPath);
  Datum::printValue("Configuration file",configurationFile);
  Datum::printValue("DATUM_HISTOS_IMPORT_PATH:",histogramPath);
  Datum::printValue("DATUM_HISTOS_EXPORT_PATH:",histogramPath);

}

int main(int argc, char* argv[])
{
  if (argc>=3 && Datum::String(argv[1])=="--function-catalog")
    {
    Datum::String composites = Datum::MATH::FunctionRegistry::defaultCompositesFile();
    std::ifstream registered(composites.data());
    if (registered) Datum::MATH::FunctionRegistry::readComposites(composites);   // those registered by DatumGui
    if (!Datum::MATH::FunctionRegistry::writeCatalog(argv[2])) { std::cout << "Cannot write " << argv[2] << std::endl; return 1; }
    Datum::printValue("Function catalog written",Datum::String(argv[2]));
    return 0;
    }
  if (argc>=3 && Datum::String(argv[1])=="--maps-list")
    {
    // the particle efficiencies and smearers of a maps file (DatumGui, Detector Maps), for RunDatum.py: one line each,
    // MAP <tab> kind <tab> name <tab> species (PDG codes, comma separated) <tab> maps (key=summary, ...)
    try
      {
      std::unique_ptr<Datum::DatumFile> file(Datum::DatumFile::openOldDatumFile(argv[2]));
      const Datum::Folder & top = file->folder();
      auto list = [&](Datum::KinematicMapFolder & object, const std::vector<int> & species)
        {
        std::cout << "MAP\t" << object.kindName() << "\t" << object.name() << "\t";
        for (std::size_t k=0; k<species.size(); k++) std::cout << (k ? "," : "") << species[k];
        std::cout << "\t";
        for (unsigned int k=0; k<object.nMaps(); k++)
          {
          const Datum::KinematicMap & map = object.map(k);
          const char * model[] = { "none", "constant", "function", "histogram" };
          std::cout << (k ? ", " : "") << object.mapKey(k) << "=" << model[int(map.model())];
          }
        std::cout << std::endl;
        };
      for (const Datum::String & name : Datum::KinematicMapFolder::objectNames(top,"PARTICLE_EFFICIENCY"))
        {
        Datum::ParticleEfficiency o;
        o.setName(name);
        o.loadFrom(top);
        list(o,o.species());
        }
      for (const Datum::String & name : Datum::KinematicMapFolder::objectNames(top,"PARTICLE_SMEARER"))
        {
        Datum::ParticleSmear o;
        o.setName(name);
        o.loadFrom(top);
        list(o,std::vector<int>());
        }
      return 0;
      }
    catch (Datum::Exception & exception)
      {
      exception.print();
      return 1;
      }
    }
  if (argc>=3 && Datum::String(argv[1])=="--xml-templates")
    {
    try
      {
      std::vector<Datum::String> classNames;
      for (int k=3; k<argc; k++) classNames.push_back(argv[k]);
      std::vector<Datum::String> files = Datum::Task::exportXmlTemplates(argv[2],classNames);
      for (auto & f : files) Datum::printValue("Template written",f);
      return 0;
      }
    catch (Datum::Exception & exception)
      {
      exception.print();
      return 1;
      }
    }
  Datum::String mainTaskName;
  Datum::String configurationPath;
  Datum::String configurationFile;
  Datum::String histogramPath;
  parse(argc,argv,mainTaskName,configurationPath,configurationFile,histogramPath);
 try
  {
  Datum::Task * creatorTask = new Datum::Task();
  Datum::Task * mainTask = nullptr;
  if (configurationFile.endsWith(".xml"))
    {
    Datum::XmlDocument document = Datum::XmlDocument::read(configurationPath,configurationFile);
    const Datum::XmlElement * taskElement = (mainTaskName=="-") ? document.findFirst("TASK")
                                                                : document.findFirst("TASK","name",mainTaskName);
    if (!taskElement) throw Datum::ConfigurationException(mainTaskName,"No such TASK element in "+configurationFile,"main()");
    mainTask = creatorTask->createTask(nullptr,*taskElement);
    }
  else
    {
    Datum::Configuration configuration;
    configuration.loadFrom(configurationPath,configurationFile);
    mainTask = creatorTask->createTask(nullptr,mainTaskName,configuration);
    }
  // Files of the job: the histograms of all its tasks are written to one output file (<main>:JOB:OUTPUT_FILE, by
  // default <main>.datum), a folder per task holding a folder per combination of filters; those imported are read from
  // one input file (<main>:JOB:INPUT_FILE) organized in the same way. Both are in the histogram directory, the input
  // file in <main>:JOB:INPUT_PATH when set; an absolute name is used as is.
  const Datum::Configuration & jobConfiguration = mainTask->configuration();
  Datum::String outputName = jobConfiguration.valueString(Datum::createKey(mainTask->name(),"JOB:OUTPUT_FILE"),
                                                        true,mainTask->name()+Datum::String(Datum::DatumFile::extension()));
  Datum::String inputName  = jobConfiguration.valueString(Datum::createKey(mainTask->name(),"JOB:INPUT_FILE"),true,"NONE");
  // Directory of the input file (<main>:JOB:INPUT_PATH; empty: the histogram directory, that of the output file).
  Datum::String inputPath  = jobConfiguration.valueString(Datum::createKey(mainTask->name(),"JOB:INPUT_PATH"),true,"");
  // Seed of Datum::MATH::randomGenerator() (<main>:JOB:SEED, default 112133): the sub-samples of RunDatum.py are given different seeds.
  Datum::String seedText = jobConfiguration.valueString(Datum::createKey(mainTask->name(),"JOB:SEED"),true,"112133");
  long seed = 112133;
  try { seed = std::stol(seedText.str()); }
  catch (const std::exception &) { throw Datum::ConfigurationException(mainTask->name(),"Invalid JOB:SEED "+seedText,"main()"); }
  if (seed!=0) Datum::MATH::randomGenerator().setSeed(seed);
  Datum::printValue("Random seed",seed);
  // Normalization of the histograms (<main>:JOB:NORMALIZE, default true): a job of a production in pT-hat bins keeps
  // the raw histograms, added with the weights sigma/N of the bins by SubSampleStatCalculator.
  bool normalize = jobConfiguration.valueBool(Datum::createKey(mainTask->name(),"JOB:NORMALIZE"),true,true);
  Datum::EventProcessor::setNormalizeHistograms(normalize);
  if (!normalize) Datum::printValue("Histograms normalized","no (raw counts kept)");
  mainTask->initialize();
  if (!inputName.isEmpty() && inputName!="NONE")
    {
    Datum::String path;
    Datum::String name = jobFile(inputName,inputPath.isEmpty() ? histogramPath : inputPath,path);
    Datum::printValue("Input file of the job",Datum::DatumFile::makeFileName(path,name));
    std::unique_ptr<Datum::DatumFile> input(Datum::DatumFile::openOldDatumFile(path,name));
    mainTask->loadHistograms(*input);
    }
  mainTask->execute();
  mainTask->finalize();
  if (!outputName.isEmpty() && outputName!="NONE")
    {
    Datum::String path;
    Datum::String name = jobFile(outputName,histogramPath,path);
    std::unique_ptr<Datum::DatumFile> output(Datum::DatumFile::openRecreateDatumFile(path,name));
    mainTask->saveHistograms(*output);
    if (output->names().empty())
      Datum::printString("No histograms: no output file");
    else
      {
      output->write();
      Datum::printValue("Output file of the job",Datum::DatumFile::makeFileName(path,name));
      }
    }
  delete mainTask;
  delete creatorTask;
  Datum::printLine();
  Datum::printLine();

//  Datum::Timer timer;
//
//  timer.start();
//  Datum::Histogram * h = new Datum::Histogram(Datum::String("test"),Datum::String("test"),100,0.0, 100.0);
//  long n = 400000000;
//  Datum::printLine();
//  Datum::printLine();
//  Datum::printLine();
//  Datum::printLine();
//  cout << n << endl;
//  for (long k=0; k<n; k++)
//    {
//   // if (k%1000) cout << k << endl;
//    h->fill(50.0,1.0);
//    }
//  timer.stop();
//  timer.print();
//  Datum::printString("Part I  completed successfully");
//  timer.start();
//  TH1 * hh = new TH1D(Datum::String("test"),Datum::String("test"),100,0.0, 100.0);
//  for (long k=0; k<n; k++)
//    {
//    //if (k%1000) cout << k << endl;
//    hh->Fill(50.0,1.0);
//    }
//  timer.stop();
//  timer.print();



  Datum::printString("Datum completed successfully");
  Datum::printLine();
  Datum::printLine();
  

  return 0;
  }

  catch (Datum::ConfigurationException & exception)
  {
  exception.print();
  }
  catch (Datum::MATH::MathException & exception)
  {
  exception.print();
  }
  catch (Datum::FileException & exception)
  {
  exception.print();
  }
  catch (Datum::MemoryException & exception)
  {
  exception.print();
  }
  catch (Datum::HistogramException & exception)
  {
  exception.print();
  }
  catch (Datum::FactoryException & exception)
  {
  exception.print();
  }
  catch (Datum::Exception & exception)
  {
  exception.print();
  }
  Datum::printLine();
  Datum::printString("Datum FAILED");
  Datum::printLine();
  return 1;
}

