/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file HerwigEventGenerator.cpp
//! @brief Implementation of HerwigEventGenerator (through Herwig::API and ThePEG).
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "HerwigEventGenerator.hpp"
#include "Event.hpp"
#include "Exceptions.hpp"
#include "FileSystem.hpp"
#include "Particle.hpp"
#include "ParticleDb.hpp"
#include "ParticleFilter.hpp"
#include "ParticleType.hpp"
#include "PrintHelpers.hpp"
// Herwig and ThePEG last (their names stay in their namespaces)
#include "Herwig/API/HerwigAPI.h"
#include "Herwig/API/HerwigUI.h"
#include "ThePEG/EventRecord/Event.h"
#include "ThePEG/EventRecord/Particle.h"
#include "ThePEG/Repository/EventGenerator.h"

namespace CAP
{

//! Number of free-form command keys (<name>:Option0 ... <name>:Option19).
static const int kNHerwigOptions = 20;

namespace
{
//!
//! The settings Herwig::API reads, as its command line gives them (Herwig::HerwigCLI): a run mode, the repository,
//! the input or run file, a setup file, a seed and a tag. quit() and quitWithHelp() throw instead of ending the job.
//!
class HerwigSettings : public Herwig::HerwigUI
{
public:
  HerwigSettings(Herwig::RunMode::Mode mode, const std::string & repository, const std::string & input,
                 const std::string & setup="", int seed=0, const std::string & tag="")
  : _mode(mode), _repository(repository), _input(input), _setup(setup), _seed(seed), _tag(tag) { }
  Herwig::RunMode::Mode runMode() const override { return _mode; }
  std::string repository() const override { return _repository; }
  std::string inputfile() const override { return _input; }
  std::string setupfile() const override { return _setup; }
  bool resume() const override { return false; }
  bool tics() const override { return false; }
  std::string tag() const override { return _tag; }
  std::string integrationList() const override { return ""; }
  const std::vector<std::string> & prependReadDirectories() const override { return _none; }
  const std::vector<std::string> & appendReadDirectories() const override { return _none; }
  long N() const override { return 0; }
  int seed() const override { return _seed; }
  int jobs() const override { return 1; }
  unsigned int jobSize() const override { return 1; }
  unsigned int maxJobs() const override { return 1; }
  void quitWithHelp() const override { quit(); }
  void quit() const override
  {
    throw Exception("Herwig stopped (see its messages above, and its .log file)", "HerwigSettings::quit");
  }
  std::ostream & outStream() const override { return std::cout; }
  std::ostream & errStream() const override { return std::cerr; }
  std::istream & inStream() const override { return std::cin; }
private:
  Herwig::RunMode::Mode _mode;
  std::string _repository, _input, _setup;
  int _seed;
  std::string _tag;
  std::vector<std::string> _none;
};

//! Back to the current directory when it goes out of scope.
class DirectoryGuard
{
public:
  explicit DirectoryGuard(const std::filesystem::path & directory) : _previous(std::filesystem::current_path())
  {
    if (!directory.empty()) std::filesystem::current_path(directory);
  }
  ~DirectoryGuard() { std::error_code e; std::filesystem::current_path(_previous, e); }
private:
  std::filesystem::path _previous;
};

//! Name of the run file an input file saves: <name>.run of its last command "saverun <name> <generator>" ("" if none).
std::string savedRunName(const std::filesystem::path & input)
{
  std::ifstream in(input);
  std::string line, name;
  while (std::getline(in, line))
    {
    std::istringstream words(line.substr(0, line.find('#')));
    std::string command, run;
    if (words >> command >> run && command == "saverun") name = run + ".run";
    }
  return name;
}
} // namespace

//! The event generator of Herwig (a ThePEG::EventGenerator, reference counted).
class HerwigRun
{
public:
  ThePEG::EGPtr generator;
};

HerwigEventGenerator::HerwigEventGenerator()
:
EventProcessor(),
herwig(nullptr),
savePhotons(true),
saveNeutrinos(true),
nWeighted(0),
nFailed(0)
{
  appendClassName("HerwigEventGenerator");
  setMinimumReportLevel(Object::kInfo);
  setName("HerwigEventGenerator");
  setTitle("HerwigEventGenerator");
}

HerwigEventGenerator::HerwigEventGenerator(const HerwigEventGenerator & source)
:
EventProcessor(source),
herwig(nullptr),
savePhotons(source.savePhotons),
saveNeutrinos(source.saveNeutrinos),
nWeighted(0),
nFailed(0)
{ }

HerwigEventGenerator::~HerwigEventGenerator()
{
  delete herwig;
}

HerwigEventGenerator & HerwigEventGenerator::operator=(const HerwigEventGenerator & rhs)
{
  if (this!=&rhs)
    {
    EventProcessor::operator=(rhs);
    savePhotons   = rhs.savePhotons;
    saveNeutrinos = rhs.saveNeutrinos;
    }
  return *this;
}

void HerwigEventGenerator::setDefaultConfiguration()
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  EventProcessor::setDefaultConfiguration();
  String taskName = name();
  _configuration.addProperty(createKey(taskName,"InputFile"),        String(""));
  _configuration.addProperty(createKey(taskName,"RunFile"),          String("LHC.run"));
  _configuration.addProperty(createKey(taskName,"Repository"),       String("HerwigDefaults.rpo"));
  _configuration.addProperty(createKey(taskName,"SetupFile"),        String(""));
  _configuration.addProperty(createKey(taskName,"Tag"),              String(""));
  _configuration.addProperty(createKey(taskName,"SetSeed"),          true);
  _configuration.addProperty(createKey(taskName,"SeedValue"),        121211);
  _configuration.addProperty(createKey(taskName,"SavePhotons"),      true);
  _configuration.addProperty(createKey(taskName,"SaveNeutrinos"),    true);
  _configuration.addProperty(createKey(taskName,"Print:Statistics"), true);
  for (int k=0; k<kNHerwigOptions; k++)
    {
    String option = "Option"; option += k;
    _configuration.addProperty(createKey(taskName,option), String("none"));
    }
}

void HerwigEventGenerator::initialize()
{
  if (reportStart(__FUNCTION__)) { printCR(); }
  EventProcessor::initialize();
  namespace fs = std::filesystem;
  String taskName = name();
  delete herwig;
  herwig = nullptr;
  typeOfCode.clear();
  skippedCodes.clear();
  nWeighted = 0;
  nFailed   = 0;
  savePhotons   = _configuration.valueBool(createKey(taskName,"SavePhotons"));
  saveNeutrinos = _configuration.valueBool(createKey(taskName,"SaveNeutrinos"));
  std::string repository = FileSystem::expand(_configuration.valueString(createKey(taskName,"Repository"))).str();
  std::string input      = FileSystem::expand(_configuration.valueString(createKey(taskName,"InputFile"))).str();
  std::string runFile    = FileSystem::expand(_configuration.valueString(createKey(taskName,"RunFile"))).str();
  std::string tag        = _configuration.valueString(createKey(taskName,"Tag")).str();
  int seed = _configuration.valueBool(createKey(taskName,"SetSeed"))
             ? int(_configuration.valueLong(createKey(taskName,"SeedValue"))) : 0;

  // an input file (.in) given as the run file is taken as the input file
  bool noInput = input.empty() || input=="none" || input=="NONE";
  if (noInput && fs::path(runFile).extension()==".in")
    {
    input   = runFile;
    noInput = false;
    printValue("Herwig input file (given as RunFile)",String(input.c_str()));
    }
  // the run file: read from the input file (Herwig read) when it is missing or older
  if (!noInput)
    {
    fs::path in = fs::absolute(input);
    if (!fs::is_regular_file(in))
      throw ConfigurationException(createKey(taskName,"InputFile"), String("No Herwig input file ") + String(in.c_str()),
                                   __FUNCTION__);
    std::string saved = savedRunName(in);
    if (saved.empty())
      throw ConfigurationException(createKey(taskName,"InputFile"),
                                   String("The Herwig input file has no saverun command: ") + String(in.c_str()),
                                   __FUNCTION__);
    fs::path run = in.parent_path() / saved;
    if (!fs::is_regular_file(run) || fs::last_write_time(run) < fs::last_write_time(in))
      {
      printValue("Herwig read",String(in.c_str()));
      DirectoryGuard here(in.parent_path());          // Herwig saves the run file in the current directory
      Herwig::API::read(HerwigSettings(Herwig::RunMode::READ, repository, in.filename().string()));
      if (!fs::is_regular_file(run))
        throw Exception(String("Herwig read did not make the run file ") + String(run.c_str()),__FUNCTION__);
      }
    runFile = run.string();
    }
  if (!fs::is_regular_file(runFile))
    throw ConfigurationException(createKey(taskName,"RunFile"),
                                 String("No Herwig run file ") + String(runFile.c_str())
                                 + " (make it with: Herwig read <input file>, or give InputFile)",__FUNCTION__);

  // the setup: the setup file and the commands of the options, in one file of the current directory
  std::string setup = FileSystem::expand(_configuration.valueString(createKey(taskName,"SetupFile"))).str();
  std::vector<std::string> commands;
  for (int k=0; k<kNHerwigOptions; k++)
    {
    String option = "Option"; option += k;
    String value  = _configuration.valueString(createKey(taskName,option));
    if (!value.isNull() && !value.contains("none"))
      {
      String label = "Herwig:"; label += option;
      printValue(label.data(),value);
      commands.push_back(value.str());
      }
    }
  if (!commands.empty())
    {
    std::string combined = std::string(taskName.str()) + "_setup.in";
    std::ofstream out(combined);
    if (!setup.empty() && setup!="none" && setup!="NONE")
      {
      std::ifstream in(setup);
      if (!in) throw ConfigurationException(createKey(taskName,"SetupFile"),
                                            String("No Herwig setup file ") + String(setup.c_str()),__FUNCTION__);
      out << in.rdbuf() << "\n";
      }
    for (const std::string & command : commands) out << command << "\n";
    if (!out) throw Exception(String("Cannot write ") + String(combined.c_str()),__FUNCTION__);
    setup = combined;
    }
  else if (setup=="none" || setup=="NONE")
    setup = "";

  printValue("Herwig run file",String(runFile.c_str()));
  if (!setup.empty()) printValue("Herwig setup file",String(setup.c_str()));
  if (seed>0) printValue("Herwig seed",long(seed));
  herwig = new HerwigRun();
  herwig->generator = Herwig::API::prepareRun(HerwigSettings(Herwig::RunMode::RUN, repository, runFile, setup, seed,
                                                             tag));
  if (!herwig->generator)
    throw Exception(String("Herwig could not prepare the event generator of ") + String(runFile.c_str()),__FUNCTION__);
}

ParticleType * HerwigEventGenerator::particleType(int pdg)
{
  auto found = typeOfCode.find(pdg);
  if (found!=typeOfCode.end())
    {
    if (!found->second) skippedCodes[pdg]++;
    return found->second;
    }
  ParticleDb & particleDb = db();
  ParticleType * type = (particleDb.findIndexForPdgCode(pdg)>=0) ? particleDb.findPdgCode(pdg) : nullptr;
  typeOfCode[pdg] = type;
  if (!type) skippedCodes[pdg]++;
  return type;
}

void HerwigEventGenerator::execute()
{
  if (!herwig || !herwig->generator) throw Exception("Herwig is not initialized",__FUNCTION__);
  ThePEG::EventPtr source;
  try
    {
    source = herwig->generator->shoot();
    }
  catch (std::exception & error)
    {
    nFailed++;
    if (reportWarning(__FUNCTION__)) printValue("Herwig event failed (skipped)",String(error.what()));
    return;
    }
  if (!source)
    {
    nFailed++;
    return;
    }
  if (source->weight()!=1.0) nWeighted++;
  Event & theEvent = event();
  ParticleFilter * particleFilter = particleFilters().empty() ? nullptr : particleFilters()[0];
  for (const ThePEG::tPPtr & p : source->getFinalState())
    {
    int pdg = int(p->id());
    printValue("HERWIG::execute() pdg",pdg);
    int absPdg = std::abs(pdg);
    if (!savePhotons   && pdg==22) continue;
    if (!saveNeutrinos && (absPdg==12 || absPdg==14 || absPdg==16 || absPdg==18)) continue;
    ParticleType * type = particleType(pdg);
    if (!type) continue;                         // not in the particle database (reported by finalize())
    printValue("HERWIG::execute() ","Got a type");
    const ThePEG::Lorentz5Momentum & momentum = p->momentum();
    const ThePEG::LorentzPoint vertex = p->labVertex();
    Particle & particle = Particle::factory().nextObject();
    particle.reset();
    particle.setType(type);
    particle.setLive(true);
//    printValue("HERWIG::execute() p.E=",momentum.e()/ThePEG::GeV);
//    printValue("HERWIG::execute() p.x=",momentum.x()/ThePEG::GeV);
//    printValue("HERWIG::execute() p.y=",momentum.y()/ThePEG::GeV);
//    printValue("HERWIG::execute() p.z=",momentum.z()/ThePEG::GeV);
    particle.setEPxPyPz(momentum.e()/ThePEG::GeV, momentum.x()/ThePEG::GeV, momentum.y()/ThePEG::GeV,
                        momentum.z()/ThePEG::GeV);
    particle.setXYZT(vertex.x()/ThePEG::mm, vertex.y()/ThePEG::mm, vertex.z()/ThePEG::mm, vertex.t()/ThePEG::mm);
    if (!particleFilter || particleFilter->accept(particle)) theEvent.addParticle(&particle);
    }
  _eventAccepted.increment();
  _taskExecuted.increment();
}

void HerwigEventGenerator::finalize()
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  if (!skippedCodes.empty() && reportWarning(__FUNCTION__))
    {
    printValue("Particles skipped: PDG codes not in the particle database",long(skippedCodes.size()));
    for (auto & entry : skippedCodes)
      {
      String label = "PDG code "; label += entry.first;
      printValue(label.data(),entry.second);
      }
    }
  if (nFailed>0 && reportWarning(__FUNCTION__)) printValue("Herwig events that failed (skipped)",nFailed);
  if (nWeighted>0 && reportWarning(__FUNCTION__))
    printValue("Weighted Herwig events (CAP ignores the weights)",nWeighted);
  if (herwig && herwig->generator)
    {
    if (_configuration.valueBool(createKey(name(),"Print:Statistics")))
      {
      printValue("Herwig events",long(herwig->generator->currentEventNumber()));
      printValue("Herwig cross section (pb)",double(herwig->generator->integratedXSec()/ThePEG::picobarn));
      printValue("Herwig cross section error (pb)",double(herwig->generator->integratedXSecErr()/ThePEG::picobarn));
      }
    herwig->generator->finalize();                  // Herwig writes its .out file
    }
  delete herwig;
  herwig = nullptr;
  EventProcessor::finalize();
}

} // namespace CAP
