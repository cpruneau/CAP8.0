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
#include "PythiaEventReader.hpp"
#include "FileSystem.hpp"
#include "Exceptions.hpp"


namespace CAP
{

PythiaEventReader::PythiaEventReader()
:
EventProcessor(),
RootChainManager(),
saveFinalOnly(true),
savePhotons(false),
saveNeutrinos(false),
saveQuarks(false),
saveGaugeBosons(false),
nParticles(0)
{
  appendClassName("PythiaEventReader");
  setMinimumReportLevel(Object::kInfo);
  setName("PythiaEventReader");
  setTitle("PythiaEventReader");
}

PythiaEventReader::PythiaEventReader(const PythiaEventReader & source)
:
EventProcessor(source),
RootChainManager(source),
saveFinalOnly(source.saveFinalOnly),
savePhotons(source.savePhotons),
saveNeutrinos(source.saveNeutrinos),
saveQuarks(source.saveQuarks),
saveGaugeBosons(source.saveGaugeBosons),
nParticles(0)
{  }

PythiaEventReader & PythiaEventReader::operator=(const PythiaEventReader & rhs)
{
  if (this!=&rhs)
    {
    EventProcessor::operator=(rhs);
    RootChainManager::operator=(rhs);
    saveFinalOnly    = rhs.saveFinalOnly;
    savePhotons      = rhs.savePhotons;
    saveNeutrinos    = rhs.saveNeutrinos;
    saveQuarks       = rhs.saveQuarks;
    saveGaugeBosons  = rhs.saveGaugeBosons;
    }
  return *this;
}

void PythiaEventReader::setDefaultConfiguration()
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  EventProcessor::setDefaultConfiguration();
  String taskName = name();
  const char * importPath = std::getenv("CAP_DATA_IMPORT_PATH");
  _configuration.addProperty(createKey(taskName,"DataImportPath"),   String(importPath ? importPath : ""));
  _configuration.addProperty(createKey(taskName,"DataImportFile"),   String(""));
  _configuration.addProperty(createKey(taskName,"TreeName"),         String(""));
  _configuration.addProperty(createKey(taskName,"SaveFinalOnly"),    true);
  _configuration.addProperty(createKey(taskName,"SavePhotons"),      false);
  _configuration.addProperty(createKey(taskName,"SaveNeutrinos"),    false);
  _configuration.addProperty(createKey(taskName,"SaveQuarks"),       false);
  _configuration.addProperty(createKey(taskName,"SaveGaugeBosons"),  false);
}

void PythiaEventReader::initialize()
{
  if (reportStart(__FUNCTION__)) { printCR(); }
  EventProcessor::initialize();
  String taskName = name();
  saveFinalOnly   = _configuration.valueBool(createKey(taskName,"SaveFinalOnly"));
  savePhotons     = _configuration.valueBool(createKey(taskName,"SavePhotons"));
  saveNeutrinos   = _configuration.valueBool(createKey(taskName,"SaveNeutrinos"));
  saveQuarks      = _configuration.valueBool(createKey(taskName,"SaveQuarks"));
  saveGaugeBosons = _configuration.valueBool(createKey(taskName,"SaveGaugeBosons"));
  typeOfCode.clear();
  skippedCodes.clear();

  String treeName = _configuration.valueString(createKey(taskName,"TreeName"));
  String path     = _configuration.valueString(createKey(taskName,"DataImportPath"));
  String files    = _configuration.valueString(createKey(taskName,"DataImportFile"));
  if (treeName.isNull())
    throw ConfigurationException(createKey(taskName,"TreeName"),"Name of the tree of PYTHIA events not set",__FUNCTION__);
  if (files.isNull())
    throw ConfigurationException(createKey(taskName,"DataImportFile"),"PYTHIA event file(s) not set",__FUNCTION__);
  _inputFiles = FileSystem::join(path,files);   // an absolute DataImportFile is used as it is
  initializeImportTree(treeName);
  if (chain()->Add(_inputFiles.data())==0)
    throw FileException(_inputFiles,"No PYTHIA event file found",__FUNCTION__);
  importEvent();
  printValue("Pythia:Input",_inputFiles);
  printValue("Pythia:Tree",treeName);

  TTree * tree = chain();
  tree->SetMakeClass(1);
  tree->SetBranchAddress("tracks", & nParticles, & b_tracks_);
  tree->SetBranchAddress("tracks.fUniqueID", tracks_fUniqueID, & b_tracks_fUniqueID);
  tree->SetBranchAddress("tracks.fBits", tracks_fBits, & b_tracks_fBits);
  tree->SetBranchAddress("tracks.fLineColor", tracks_fLineColor, & b_tracks_fLineColor);
  tree->SetBranchAddress("tracks.fLineStyle", tracks_fLineStyle, & b_tracks_fLineStyle);
  tree->SetBranchAddress("tracks.fLineWidth", tracks_fLineWidth, & b_tracks_fLineWidth);
  tree->SetBranchAddress("tracks.fPdgCode", tracks_fPdgCode, & b_tracks_fPdgCode);
  tree->SetBranchAddress("tracks.fStatusCode", tracks_fStatusCode, & b_tracks_fStatusCode);
  tree->SetBranchAddress("tracks.fMother[2]", tracks_fMother, & b_tracks_fMother);
  tree->SetBranchAddress("tracks.fDaughter[2]", tracks_fDaughter, & b_tracks_fDaughter);
  tree->SetBranchAddress("tracks.fWeight", tracks_fWeight, & b_tracks_fWeight);
  tree->SetBranchAddress("tracks.fCalcMass", tracks_fCalcMass, & b_tracks_fCalcMass);
  tree->SetBranchAddress("tracks.fPx", tracks_fPx, & b_tracks_fPx);
  tree->SetBranchAddress("tracks.fPy", tracks_fPy, & b_tracks_fPy);
  tree->SetBranchAddress("tracks.fPz", tracks_fPz, & b_tracks_fPz);
  tree->SetBranchAddress("tracks.fE", tracks_fE, & b_tracks_fE);
  tree->SetBranchAddress("tracks.fVx", tracks_fVx, & b_tracks_fVx);
  tree->SetBranchAddress("tracks.fVy", tracks_fVy, & b_tracks_fVy);
  tree->SetBranchAddress("tracks.fVz", tracks_fVz, & b_tracks_fVz);
  tree->SetBranchAddress("tracks.fVt", tracks_fVt, & b_tracks_fVt);
  tree->SetBranchAddress("tracks.fPolarTheta", tracks_fPolarTheta, & b_tracks_fPolarTheta);
  tree->SetBranchAddress("tracks.fPolarPhi", tracks_fPolarPhi, & b_tracks_fPolarPhi);
  tree->SetBranchAddress("HP_Inc1", & HP_Inc1, & b_HP_Inc1);
  tree->SetBranchAddress("HP_Inc2", & HP_Inc2, & b_HP_Inc2);
  tree->SetBranchAddress("HP_Inc3", & HP_Inc3, & b_HP_Inc3);
  tree->SetBranchAddress("HP_Inc4", & HP_Inc4, & b_HP_Inc4);
  tree->SetBranchAddress("HP_Out1", & HP_Out1, & b_HP_Out1);
  tree->SetBranchAddress("HP_Out2", & HP_Out2, & b_HP_Out2);
  tree->SetBranchAddress("HP_Out3", & HP_Out3, & b_HP_Out3);
  tree->SetBranchAddress("HP_Out4", & HP_Out4, & b_HP_Out4);
  tree->SetBranchAddress("HP_Inter1", & HP_Inter1, & b_HP_Inter1);
  tree->SetBranchAddress("HP_Inter2", & HP_Inter2, & b_HP_Inter2);
  tree->SetBranchAddress("HP_Inter3", & HP_Inter3, & b_HP_Inter3);
  tree->SetBranchAddress("HP_Inter4", & HP_Inter4, & b_HP_Inter4);
  tree->SetBranchAddress("HP_Kick1", & HP_Kick1, & b_HP_Kick1);
  tree->SetBranchAddress("HP_Kick2", & HP_Kick2, & b_HP_Kick2);
  tree->SetBranchAddress("HP_Kick3", & HP_Kick3, & b_HP_Kick3);
  tree->SetBranchAddress("HP_Kick4", & HP_Kick4, & b_HP_Kick4);
}

//!
//! Type of a PDG code in the particle database of the task, or nullptr if the database does not have it (e.g. K0L and
//! K0S, 130 and 310, kept undecayed by PYTHIA options but absent from DefaultDb): such particles are skipped.
//!
ParticleType * PythiaEventReader::particleType(int pdg)
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

void PythiaEventReader::reportSkippedCodes()
{
  if (skippedCodes.empty() || !reportWarning(__FUNCTION__)) return;
  printValue("Particles skipped: PDG codes not in the particle database",long(skippedCodes.size()));
  for (auto & entry : skippedCodes)
    {
    String label = "PDG code "; label += entry.first;
    printValue(label.data(),entry.second);
    }
}

//!
//! Read the next PYTHIA event with more than two particles and add its particles to the current event (the event is
//! reset by the event iterator). Throws EndOfDataException at the end of the input, which ends the event loop.
//! Particles whose PDG code is not in the particle database are skipped; a particle is added if it passes the first
//! particle filter of the task (or always if the task has no particle filter).
//!
void PythiaEventReader::execute()
{
  do
    {
    if (loadTree(_entryIndex) < 0)
      throw EndOfDataException(_inputFiles,"End of the PYTHIA input",__FUNCTION__);
    _nb = chain()->GetEntry(_entryIndex++);
    _nBytes += _nb;
    if (reportDebug(__FUNCTION__)) { printValue("Bytes read",long(_nb)); printValue("nParticles",nParticles); }
    }
  while (nParticles<=2);
  if (nParticles>nMaxTracks)
    throw Exception("More particles in the event than the reader can hold (nMaxTracks)",__FUNCTION__);

  Event & theEvent = event();
  ParticleFilter * particleFilter = particleFilters().empty() ? nullptr : particleFilters()[0];
  for (int iParticle = 0; iParticle < nParticles; iParticle++)
    {
    if (saveFinalOnly && tracks_fStatusCode[iParticle] <= 0) continue;
    int pdg = tracks_fPdgCode[iParticle];
    int absPdg = std::abs(pdg);
    if (!saveQuarks      &&  absPdg<10) continue;    // quarks
    if (!saveQuarks      &&  absPdg==21) continue;   // gluons
    if (!saveNeutrinos   &&  (absPdg==12 || absPdg==14 || absPdg==16 || absPdg==18)) continue;
    if (!savePhotons     &&  pdg==22) continue;
    if (!saveGaugeBosons &&  absPdg>22 && absPdg<40) continue;
    ParticleType * type = particleType(pdg);
    if (type==nullptr) continue;                 // not in the particle database (reported by finalize())
    Particle & particle = Particle::factory().nextObject();
    particle.reset();
    particle.setType(type);
    particle.setLive(true);
    particle.setEPxPyPz(tracks_fE[iParticle],tracks_fPx[iParticle],tracks_fPy[iParticle],tracks_fPz[iParticle]);
    particle.setXYZT(0.0,0.0,0.0,0.0);
    if (!particleFilter || particleFilter->accept(particle)) theEvent.addParticle(&particle);
    }
  _eventAccepted.increment();
  _taskExecuted.increment();
}

void PythiaEventReader::finalize()
{
  if (reportDebug(__FUNCTION__)) { printCR(); }
  reportSkippedCodes();
  EventProcessor::finalize();
}

} // namespace CAP
