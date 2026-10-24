/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include <cstdlib>
#include "PythiaEventGenerator.hpp"
#include "Exceptions.hpp"

namespace CAP
{

//! Number of free-form PYTHIA option keys (<name>:Option0 ... <name>:Option29).
static const int kNPythiaOptions = 30;

PythiaEventGenerator::PythiaEventGenerator()
:
EventProcessor(),
pythia(nullptr),
saveFinalOnly(true),
savePhotons(false),
saveNeutrinos(false),
saveQuarks(false),
saveGaugeBosons(false)
{
  appendClassName("PythiaEventGenerator");
  setMinimumReportLevel(Object::kInfo);
  setName("PythiaEventGenerator");
  setTitle("PythiaEventGenerator");
}

// The generator itself is not copied: a copy creates its own in initialize().
PythiaEventGenerator::PythiaEventGenerator(const PythiaEventGenerator & source)
:
EventProcessor(source),
pythia(nullptr),
saveFinalOnly(source.saveFinalOnly),
savePhotons(source.savePhotons),
saveNeutrinos(source.saveNeutrinos),
saveQuarks(source.saveQuarks),
saveGaugeBosons(source.saveGaugeBosons)
{ }

PythiaEventGenerator & PythiaEventGenerator::operator=(const PythiaEventGenerator & rhs)
{
  if (this!=&rhs)
    {
    EventProcessor::operator=(rhs);
    saveFinalOnly    = rhs.saveFinalOnly;
    savePhotons      = rhs.savePhotons;
    saveNeutrinos    = rhs.saveNeutrinos;
    saveQuarks       = rhs.saveQuarks;
    saveGaugeBosons  = rhs.saveGaugeBosons;
    }
  return *this;
}

void PythiaEventGenerator::setDefaultConfiguration()
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  EventProcessor::setDefaultConfiguration();
  String taskName = name();
  _configuration.addProperty(createKey(taskName,"Print:Banner"),     false);
  _configuration.addProperty(createKey(taskName,"Print:Statistics"), true);
  _configuration.addProperty(createKey(taskName,"Beams:idA"),        2212);
  _configuration.addProperty(createKey(taskName,"Beams:idB"),        2212);
  _configuration.addProperty(createKey(taskName,"Print:NEvents"),    0);
  _configuration.addProperty(createKey(taskName,"Beams:frameType"),  2);
  _configuration.addProperty(createKey(taskName,"Beams:eCM"),        2700.0);
  _configuration.addProperty(createKey(taskName,"Beams:eA"),         1350.0);
  _configuration.addProperty(createKey(taskName,"Beams:eB"),         1350.0);
  _configuration.addProperty(createKey(taskName,"SetSeed"),          true);
  _configuration.addProperty(createKey(taskName,"SeedValue"),        121211);
  _configuration.addProperty(createKey(taskName,"UseQCDCR"),         true);
  _configuration.addProperty(createKey(taskName,"UseRopes"),         false);
  _configuration.addProperty(createKey(taskName,"UseShoving"),       false);
  _configuration.addProperty(createKey(taskName,"xmlInputPath"),     String(""));
  _configuration.addProperty(createKey(taskName,"SaveFinalOnly"),    true);
  _configuration.addProperty(createKey(taskName,"SavePhotons"),      false);
  _configuration.addProperty(createKey(taskName,"SaveNeutrinos"),    false);
  _configuration.addProperty(createKey(taskName,"SaveQuarks"),       false);
  _configuration.addProperty(createKey(taskName,"SaveGaugeBosons"),  false);
  for (int k=0; k<kNPythiaOptions; k++)
    {
    String option = "Option"; option += k;
    _configuration.addProperty(createKey(taskName,option), String("none"));
    }
}

//!
//! Directory of the PYTHIA xml files: the xmlInputPath parameter, else $PYTHIA8DATA, else
//! $CAP_PYTHIA8_PATH/share/Pythia8/xmldoc, else PYTHIA's own default.
//!
static String pythiaXmlPath(const String & configured)
{
  if (!configured.isNull()) return configured;
  const char * data = std::getenv("PYTHIA8DATA");
  if (data && *data) return String(data);
  const char * prefix = std::getenv("CAP_PYTHIA8_PATH");
  if (prefix && *prefix)
    {
    String path = prefix;
    path += "/share/Pythia8/xmldoc";
    return path;
    }
  return String("../share/Pythia8/xmldoc");
}

void PythiaEventGenerator::initialize()
{
  if (reportStart(__FUNCTION__)) { printCR(); }
  EventProcessor::initialize();
  String taskName = name();
  delete pythia;
  String xmlPath = pythiaXmlPath(_configuration.valueString(createKey(taskName,"xmlInputPath")));
  pythia = new Pythia8::Pythia(xmlPath.str(), _configuration.valueBool(createKey(taskName,"Print:Banner")));
  int frameType = _configuration.valueInt(createKey(taskName,"Beams:frameType"));
  pythia->settings.mode("Beams:idA",       _configuration.valueInt(createKey(taskName,"Beams:idA")));
  pythia->settings.mode("Beams:idB",       _configuration.valueInt(createKey(taskName,"Beams:idB")));
  pythia->settings.mode("Beams:frameType", frameType);
  switch (frameType)
    {
      default:
      case 1:
      pythia->settings.parm("Beams:eCM",_configuration.valueDouble(createKey(taskName,"Beams:eCM")));
      break;
      case 2:
      pythia->settings.parm("Beams:eA",_configuration.valueDouble(createKey(taskName,"Beams:eA")));
      pythia->settings.parm("Beams:eB",_configuration.valueDouble(createKey(taskName,"Beams:eB")));
      break;
    }
  saveFinalOnly   = _configuration.valueBool(createKey(taskName,"SaveFinalOnly"));
  savePhotons     = _configuration.valueBool(createKey(taskName,"SavePhotons"));
  saveNeutrinos   = _configuration.valueBool(createKey(taskName,"SaveNeutrinos"));
  saveQuarks      = _configuration.valueBool(createKey(taskName,"SaveQuarks"));
  saveGaugeBosons = _configuration.valueBool(createKey(taskName,"SaveGaugeBosons"));

  if (_configuration.valueBool(createKey(taskName,"SetSeed")))
    {
    String seedValueString = "Random:seed = ";
    seedValueString += _configuration.valueLong(createKey(taskName,"SeedValue"));
    pythia->readString("Random:setSeed = on");
    pythia->readString(seedValueString.str());
    printValue("Pythia:Random:setSeed","ON");
    printValue("Pythia:Random:SeedValue",seedValueString);
    }
  for (int k=0; k<kNPythiaOptions; k++)
    {
    String option = "Option"; option += k;
    String value  = _configuration.valueString(createKey(taskName,option));
    if (!value.isNull() && !value.contains("none"))
      {
      String label = "Pythia:"; label += option;
      printValue(label.data(),value);
      pythia->readString(value.str());
      }
    }
  bool useQCDCR    = _configuration.valueBool(createKey(taskName,"UseQCDCR"));
  bool useRopes    = _configuration.valueBool(createKey(taskName,"UseRopes"));
  bool useShoving  = _configuration.valueBool(createKey(taskName,"UseShoving"));
  if (useRopes && !useQCDCR)
    throw Exception("UseRopes requires UseQCDCR (ropes without the necessary junctions)",__FUNCTION__);
  if (useQCDCR)
    {
    printValue("Pythia:UseQCDCR","ON");
    pythia->readString("MultiPartonInteractions:pT0Ref = 2.15");
    pythia->readString("BeamRemnants:remnantMode = 1");
    pythia->readString("BeamRemnants:saturation = 5");
    pythia->readString("ColourReconnection:mode = 1");
    pythia->readString("ColourReconnection:allowDoubleJunRem = off");
    pythia->readString("ColourReconnection:m0 = 0.3");
    pythia->readString("ColourReconnection:allowJunctions = on");
    pythia->readString("ColourReconnection:junctionCorrection = 1.2");
    pythia->readString("ColourReconnection:timeDilationMode = 2");
    pythia->readString("ColourReconnection:timeDilationPar = 0.18");
    if (!useRopes) pythia->readString("Ropewalk:RopeHadronization = off");
    }
  if (useQCDCR && useRopes)
    {
    printValue("Pythia:UseRopes","ON");
    pythia->readString("Ropewalk:RopeHadronization = on");
    pythia->readString("Ropewalk:doShoving = on");
    pythia->readString("Ropewalk:doFlavour = on");
    pythia->readString("Ropewalk:tInit = 1.5");
    pythia->readString("Ropewalk:deltat = 0.05");
    pythia->readString("Ropewalk:tShove = 0.1");
    pythia->readString("Ropewalk:gAmplitude = 0.");   // shoving strength set to 0 explicitly
    pythia->readString("Ropewalk:r0 = 0.5");
    pythia->readString("Ropewalk:m0 = 0.2");
    pythia->readString("Ropewalk:beta = 0.1");
    pythia->readString("PartonVertex:setVertex = on");
    pythia->readString("PartonVertex:protonRadius = 0.7");
    pythia->readString("PartonVertex:emissionWidth = 0.1");
    }
  if (useShoving)
    {
    printValue("Pythia:UseShoving","ON");
    pythia->readString("Ropewalk:RopeHadronization = on");
    pythia->readString("Ropewalk:doShoving = on");
    pythia->readString("Ropewalk:doFlavour = off");
    pythia->readString("Ropewalk:tInit = 1.5");
    pythia->readString("Ropewalk:rCutOff = 10.0");
    pythia->readString("Ropewalk:limitMom =  on");
    pythia->readString("Ropewalk:pTcut = 2.0");
    pythia->readString("Ropewalk:deltat = 0.1");
    pythia->readString("Ropewalk:deltay = 0.1");
    pythia->readString("Ropewalk:tShove = 1.");
    pythia->readString("Ropewalk:gAmplitude = 10.0");
    pythia->readString("Ropewalk:gExponent = 1.0");
    pythia->readString("Ropewalk:r0 = 0.41");
    pythia->readString("Ropewalk:m0 = 0.2");
    pythia->readString("PartonVertex:setVertex = on");
    pythia->readString("PartonVertex:protonRadius = 0.7");
    pythia->readString("PartonVertex:emissionWidth = 0.1");
    }
  if (!pythia->init())
    throw Exception("Pythia8::Pythia::init() failed (check the beam settings, the options and xmlInputPath)",__FUNCTION__);
}

//!
//! Generate one PYTHIA event and add its particles to the current event (the event is reset by the event iterator).
//! Entry 0 of the PYTHIA event record is the system as a whole and is skipped. Particles whose PDG code is not in the
//! particle database are skipped; a particle is added if it passes the first particle filter of the task (or always
//! if the task has no particle filter).
//!
void PythiaEventGenerator::execute()
{
  if (!pythia) throw Exception("PYTHIA is not initialized",__FUNCTION__);
  if (!pythia->next())
    {
    if (reportWarning(__FUNCTION__)) printValue("Pythia8::Pythia::next() failed: event skipped",true);
    return;
    }
  Event & theEvent = event();
  ParticleDb & particleDb = db();
  ParticleFilter * particleFilter = particleFilters().empty() ? nullptr : particleFilters()[0];
  int nEntries = pythia->event.size();
  for (int i = 1; i < nEntries; i++)
    {
    const Pythia8::Particle & source = pythia->event[i];
    int pdg = source.id();
    int absPdg = std::abs(pdg);
    if (saveFinalOnly    &&  !source.isFinal()) continue;
    if (!saveQuarks      &&  absPdg<10) continue;    // quarks
    if (!saveQuarks      &&  absPdg==21) continue;   // gluons
    if (!saveNeutrinos   &&  (absPdg==12 || absPdg==14 || absPdg==16 || absPdg==18)) continue;
    if (!savePhotons     &&  pdg==22) continue;
    if (!saveGaugeBosons &&  absPdg>22 && absPdg<40) continue;
    ParticleType * particleType = particleDb.findPdgCode(pdg);
    if (!particleType)
      {
      if (reportDebug(__FUNCTION__)) printValue("PDG code not in the particle database (skipped)",pdg);
      continue;
      }
    Particle & particle = Particle::factory().nextObject();
    particle.reset();
    particle.setType(particleType);
    particle.setLive(true);
    particle.setEPxPyPz(source.e(),source.px(),source.py(),source.pz());
    particle.setXYZT(source.xProd(),source.yProd(),source.zProd(),source.tProd());
    if (!particleFilter || particleFilter->accept(particle)) theEvent.addParticle(&particle);
    }
  _eventAccepted.increment();
  _taskExecuted.increment();
}

void PythiaEventGenerator::finalize()
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  if (pythia)
    {
    if (_configuration.valueBool(createKey(name(),"Print:Statistics")))
      {
      printCR();
      printLine();
      pythia->stat();
      printLine();
      printCR();
      }
    delete pythia;
    pythia = nullptr;
    }
  EventProcessor::finalize();
}

} // namespace CAP
