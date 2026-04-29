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
#include "PythiaEventGenerator.hpp"
#include "TVectorLorentz.h"
ClassImp(CAP::PythiaEventGenerator);

namespace CAP
{

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

PythiaEventGenerator::PythiaEventGenerator(const PythiaEventGenerator & source)
:
EventProcessor(source),
pythia(source.pythia),
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
    pythia = rhs.pythia;
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
  ProcessorSingle::setDefaultConfiguration();
  const String & taskName = name();
  _configuration.addProperty(createKey(taskName,"Print:Banner"),     false);
  _configuration.addProperty(createKey(taskName,"Print:Statistics"), false);
  _configuration.addProperty(createKey(taskName,"Print:NEvents"),    0);
  _configuration.addProperty(createKey(taskName,"Beams:idA"),        2212);
  _configuration.addProperty(createKey(taskName,"Beams:idB"),        2212);
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

  for (int k=0; k<30; k++)
    {
    String & key = taskName; key += ":Option"; key += k;
    _configuration.addProperty(key, String("none"));
    }
}

//!
//! Initialize generator
//! pythia->Initialize(2212 /* p */, 2212 /* p */, 14000. /* GeV */);
//!
void PythiaEventGenerator::initialize()
{
  if (reportStart(__FUNCTION__)) { printCR(); }
  EventProcessor::initialize();
  const String & taskName = name();
  pythia = new Pythia8::Pythia(valueString("xmlInputPath").Data(), valueBool("Print:Banner"));
  pythia->settings.mode("Beams:idA",       _configuration.valueInt(createKey(taskName,"Beams:idA")));
  pythia->settings.mode("Beams:idB",       _configuration.valueInt(createKey(taskName,"Beams:idB")));
  pythia->settings.mode("Beams:frameType", _configuration.valueInt(createKey(taskName,"Beams:frameType"));
  switch (_configuration.valueInt(createKey(taskName,"Beams:frameType"))
    {
      default:
      case 1:
      pythia->settings.parm("Beams:eCM",_configuration.valueDouble(createKey(taskName,"Beams:eCM")));
      break;
      case 2:
      pythia->settings.parm("Beams:eA",_configuration.valueDouble(createKey(taskName,"Beams:eA"));
      pythia->settings.parm("Beams:eB",_configuration.valueDouble(createKey(taskName,"Beams:eB"));
      break;
    }
  saveFinalOnly   = _configuration.valueBool(createKey(taskName,"SaveFinalOnly"));
  savePhotons     = _configuration.valueBool(createKey(taskName,"SavePhotons"));
  saveNeutrinos   = _configuration.valueBool(createKey(taskName,"SaveNeutrinos"));
  saveQuarks      = _configuration.valueBool(createKey(taskName,"SaveQuarks"));
  saveGaugeBosons = _configuration.valueBool(createKey(taskName,"SaveGaugeBosons"));

  if (valueDouble("SetSeed"))
    {
    String  seedValueString = "Random:seed = ";
    seedValueString += _configuration.valueLong(createKey(taskName,"SeedValue"));
    pythia->readString("Random:setSeed = on");
    pythia->readString(seedValueString.Data());
    printValue("Pythia:Random:setSeed","ON");
    printValue("Pythia:Random:SeedValue",seedValueString);
    }
  for (int k=0; k<30; k++)
    {
  String & key = taskName; key += ":Option"; key += k;
    String  value = _configuration.valueString(key);
    if (!value.Contains("none") )
      {
      String s = "Pythia:"; s+=key;
      printValue(s,value);
      pythia->readString(value.Data());
      }
    }
  if(valueBool(  "UseQCDCR"))
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
    if(!valueBool("UseRopes")) pythia->readString("Ropewalk:RopeHadronization = off");
    }
  if(valueBool("UseQCDCR")  &&  valueBool(  "UseRopes"))
    {
    printValue("Pythia:UseQCDCR","ON");
    printValue("Pythia:UseRopes","ON");
    pythia->readString("Ropewalk:RopeHadronization = on");
    pythia->readString("Ropewalk:doShoving = on");
    pythia->readString("Ropewalk:doFlavour = on");
    pythia->readString("Ropewalk:tInit = 1.5");
    pythia->readString("Ropewalk:deltat = 0.05");
    pythia->readString("Ropewalk:tShove = 0.1");
    pythia->readString("Ropewalk:gAmplitude = 0.");// # Set shoving strength to 0 explicitly
    pythia->readString("Ropewalk:r0 = 0.5");
    pythia->readString("Ropewalk:m0 = 0.2");
    pythia->readString("Ropewalk:beta = 0.1");
    pythia->readString("PartonVertex:setVertex = on");
    pythia->readString("PartonVertex:protonRadius = 0.7");
    pythia->readString("PartonVertex:emissionWidth = 0.1");
  }
  if(!valueBool("UseQCDCR")  &&  valueBool(  "UseRopes"))
    throw Exception("ropes w/o the necessary junctions! Flip kQCDCR=kTRUE",__FUNCTION__);
  if(valueBool("UseShoving"))
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
    pythia->readString("Ropewalk:deltat = 0.1");
    pythia->readString("Ropewalk:gAmplitude = 10.0");
    pythia->readString("Ropewalk:gExponent = 1.0");
    pythia->readString("Ropewalk:r0 = 0.41");
    pythia->readString("Ropewalk:m0 = 0.2");
    pythia->readString("PartonVertex:setVertex = on");
    pythia->readString("PartonVertex:protonRadius = 0.7");
    pythia->readString("PartonVertex:emissionWidth = 0.1");
    }
  pythia->init();
//  if (reportDebug(__FUNCTION__))
//    {
//    pythia->settings.listAll();
//    pythia->settings.listChanged();
//    }
}

void PythiaEventGenerator::execute()
{
  ParticleTypeList & particleTypeList = ParticleTypeList::list();
  Event & theEvent = event();
  event.reset();
  if (!pythia->next()) return;
  int nParticleToCopy   = pythia->event.size();
  //printValue("PythiaEventGenerator::execute() nParticleToCopy",nParticleToCopy);
  //return;
  if (pythia->event[0].id() == 90)
    {
    nParticleToCopy--;
    }

  for (int i = 1; i <= nParticleToCopy; i++)
    {
    int pdg = pythia->event[i].id();
    if (saveFinalOnly    &&  !pythia->event[i].isFinal()) continue;
    if (!saveQuarks      &&  abs(pdg)<10) continue; // skip quarks, gluons, etc
    if (!saveNeutrinos   &&  (abs(pdg)==12 || abs(pdg)==14  || abs(pdg)==16 || abs(pdg)==18)) continue;
    if (!savePhotons     &&  pdg==22) continue;
    if (!saveGaugeBosons  &&  (abs(pdg)>22  &&  abs(pdg)<40)) continue;
    ParticleType * particleType = particleTypeList.findPdgCode(pdg);
    Particle & particle = Particle::factory().nextObject();
    particle.setType(particleType);
    particle.setLive(1);
    particle.setPxPyPzE(pythia->event[i].px(),pythia->event[i].py(),pythia->event[i].pz(),pythia->event[i].e());
    particle.setXYZT(pythia->event[i].xProd(),pythia->event[i].yProd(),pythia->event[i].zProd(),pythia->event[i].tProd());
    event.addParticle(&particle);
    }
  eventAccepted.increment();
  _taskExecuted.increment();
  //printValue("PythiaEventGenerator::execute() event.size()",event->size());
}

void PythiaEventGenerator::finalize()
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  printCR();
  printLine();
  pythia->stat();
  printLine();
  printCR();
  delete pythia;
}

} // namespace CAP
