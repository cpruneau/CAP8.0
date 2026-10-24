/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include <cmath>
#include <cstdlib>
#include "EposEventReader.hpp"
#include "FileSystem.hpp"
#include "Exceptions.hpp"


namespace CAP
{

EposEventReader::EposEventReader()
:
EventProcessor(),
RootChainManager(),
events(0),
nParticles(0),
impact(0)
{
  appendClassName("EposEventReader");
  setMinimumReportLevel(Object::kInfo);
  setName("EposEventReader");
  setTitle("EposEventReader");
}

void EposEventReader::setDefaultConfiguration()
{
  EventProcessor::setDefaultConfiguration();
  String taskName = name();
  const char * importPath = std::getenv("CAP_DATA_IMPORT_PATH");
  _configuration.addProperty(createKey(taskName,"DataImportPath"), String(importPath ? importPath : ""));
  _configuration.addProperty(createKey(taskName,"DataImportFile"), String(""));
  _configuration.addProperty(createKey(taskName,"TreeName"),       String(""));
}

void EposEventReader::initialize()
{
  if (reportStart(__FUNCTION__)) { printCR(); }
  EventProcessor::initialize();
  String taskName = name();
  String treeName = _configuration.valueString(createKey(taskName,"TreeName"));
  String path     = _configuration.valueString(createKey(taskName,"DataImportPath"));
  String files    = _configuration.valueString(createKey(taskName,"DataImportFile"));
  if (treeName.isNull())
    throw ConfigurationException(createKey(taskName,"TreeName"),"Name of the tree of EPOS events not set",__FUNCTION__);
  if (files.isNull())
    throw ConfigurationException(createKey(taskName,"DataImportFile"),"EPOS event file(s) not set",__FUNCTION__);
  _inputFiles = FileSystem::join(path,files);   // an absolute DataImportFile is used as it is
  initializeImportTree(treeName);
  if (chain()->Add(_inputFiles.data())==0)
    throw FileException(_inputFiles,"No EPOS event file found",__FUNCTION__);
  importEvent();
  printValue("Epos:Input",_inputFiles);
  printValue("Epos:Tree",treeName);

  TTree * tree = chain();
  tree->SetMakeClass(1);
  tree->SetBranchAddress("Events", & events,  & b_Events);
  tree->SetBranchAddress("Mult", & nParticles, & b_Mult);
  tree->SetBranchAddress("Impact", & impact,  & b_Impact);
  tree->SetBranchAddress("PID", pid, & b_PID);
  tree->SetBranchAddress("Px", px, & b_Px);
  tree->SetBranchAddress("Py", py, & b_Py);
  tree->SetBranchAddress("Pz", pz, & b_Pz);
  tree->SetBranchAddress("E", e, & b_E);
}

//!
//! Read the next EPOS event and add its particles to the current event (the event is reset by the event iterator).
//! The energy of each particle is computed from its momentum and the mass of its type. Throws EndOfDataException at
//! the end of the input, which ends the event loop. Particles whose PDG code is not in the particle database are
//! skipped; a particle is added if it passes the first particle filter of the task (or always if the task has no
//! particle filter).
//!
void EposEventReader::execute()
{
  if (loadTree(_entryIndex) < 0)
    throw EndOfDataException(_inputFiles,"End of the EPOS input",__FUNCTION__);
  _nb = chain()->GetEntry(_entryIndex++);
  _nBytes += _nb;
  if (nParticles > arraySize)
    throw Exception("More particles in the event than the reader can hold (arraySize)",__FUNCTION__);

  Event & theEvent = event();
  ParticleDb & particleDb = db();
  ParticleFilter * particleFilter = particleFilters().empty() ? nullptr : particleFilters()[0];
  for (int iParticle=0; iParticle<nParticles; iParticle++)
    {
    int pdgCode = pid[iParticle];
    ParticleType * type = particleDb.findPdgCode(pdgCode);
    if (type==nullptr)
      {
      if (reportWarning(__FUNCTION__)) printValue("PDG code not in the particle database (skipped)",pdgCode);
      continue;
      }
    double p_x  = px[iParticle];
    double p_y  = py[iParticle];
    double p_z  = pz[iParticle];
    double mass = type->mass();
    double p_e  = std::sqrt(p_x*p_x + p_y*p_y + p_z*p_z + mass*mass);
    Particle & particle = Particle::factory().nextObject();
    particle.reset();
    particle.setType(type);
    particle.setLive(true);
    particle.setEPxPyPz(p_e,p_x,p_y,p_z);
    particle.setXYZT(0.0,0.0,0.0,0.0);
    if (!particleFilter || particleFilter->accept(particle)) theEvent.addParticle(&particle);
    }
  _eventAccepted.increment();
  _taskExecuted.increment();
}

} // namespace CAP
