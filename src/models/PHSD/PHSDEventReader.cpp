/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include <algorithm>
#include <cstdlib>
#include "PHSDEventReader.hpp"
#include "Exceptions.hpp"
#include "FileSystem.hpp"
#include "PrintHelpers.hpp"

namespace CAP
{

PHSDEventReader::PHSDEventReader()
:
EventProcessor(),
_inputFileNames(),
_nextFileIndex(0),
_inputFile(),
_inputFileName(),
_header(),
_typeOfCode(),
_skippedCodes()
{
  appendClassName("PHSDEventReader");
  setMinimumReportLevel(Object::kInfo);
  setName("PHSDEventReader");
  setTitle("PHSDEventReader");
}

PHSDEventReader::PHSDEventReader(const PHSDEventReader & source)
:
EventProcessor(source),
_inputFileNames(source._inputFileNames),
_nextFileIndex(0),
_inputFile(),
_inputFileName(),
_header(),
_typeOfCode(),
_skippedCodes()
{ }

PHSDEventReader & PHSDEventReader::operator=(const PHSDEventReader & source)
{
  if (this!=&source)
    {
    EventProcessor::operator=(source);
    _inputFileNames = source._inputFileNames;
    _nextFileIndex  = 0;
    _inputFile.reset();
    _inputFileName  = "";
    _header         = Header();
    _typeOfCode.clear();
    _skippedCodes.clear();
    }
  return *this;
}

void PHSDEventReader::setDefaultConfiguration()
{
  EventProcessor::setDefaultConfiguration();
  String taskName = name();
  const char * importPath = std::getenv("CAP_DATA_IMPORT_PATH");
  _configuration.addProperty(createKey(taskName,"DataImportPath"),  String(importPath ? importPath : ""));
  _configuration.addProperty(createKey(taskName,"DataImportFile"),  String("*.dat"));
  _configuration.addProperty(createKey(taskName,"DataImportDepth"), 2);
}

void PHSDEventReader::initialize()
{
  if (reportStart(__FUNCTION__)) { printCR(); }
  EventProcessor::initialize();
  const String taskName = name();
  String path    = FileSystem::expand(_configuration.valueString(createKey(taskName,"DataImportPath")));
  String pattern = _configuration.valueString(createKey(taskName,"DataImportFile"),true,"*.dat");
  int    depth   = _configuration.valueInt(createKey(taskName,"DataImportDepth"),true,2);
  if (path.isNull())
    throw ConfigurationException(createKey(taskName,"DataImportPath"),"File or directory of the PHSD events not set",__FUNCTION__);
  _inputFileNames.clear();
  if (FileSystem::isFile(path))
    _inputFileNames.push_back(path);
  else if (FileSystem::isDirectory(path))
    {
    FileSystem::ListOptions options;
    options.depth       = depth;
    options.directories = false;
    options.order       = FileSystem::ListOptions::kName;
    if (!pattern.isNull()) options.include.push_back(pattern);
    for (const FileSystem::EntryInfo & entry : FileSystem::list(path,options))
      if (entry.isFile()) _inputFileNames.push_back(entry.path);
    std::sort(_inputFileNames.begin(),_inputFileNames.end(),
              [](const String & a, const String & b) { return a.str() < b.str(); });
    }
  else
    throw ConfigurationException(createKey(taskName,"DataImportPath"),String("no file or directory ")+path,__FUNCTION__);
  if (_inputFileNames.empty())
    throw ConfigurationException(createKey(taskName,"DataImportFile"),String("no PHSD event file ")+pattern+" in "+path,__FUNCTION__);
  _nextFileIndex = 0;
  _inputFile.reset();
  _typeOfCode.clear();
  _skippedCodes.clear();
  if (reportInfo(__FUNCTION__))
    {
    printValue("PHSD:Files",long(_inputFileNames.size()));
    for (const String & file : _inputFileNames) printValue("PHSD:File",file);
    }
}

bool PHSDEventReader::openNextFile()
{
  _inputFile.reset();
  if (_nextFileIndex>=_inputFileNames.size()) return false;
  _inputFileName = _inputFileNames[_nextFileIndex++];
  _inputFile.reset(new std::ifstream(_inputFileName.c_str()));
  if (!_inputFile->is_open()) throw FileException(_inputFileName,"Cannot open the PHSD event file",__FUNCTION__);
  if (reportDebug(__FUNCTION__)) printValue("PHSD:Reading",_inputFileName);
  return true;
}

ParticleType * PHSDEventReader::particleType(int pdg)
{
  auto found = _typeOfCode.find(pdg);
  if (found!=_typeOfCode.end())
    {
    if (!found->second) _skippedCodes[pdg]++;
    return found->second;
    }
  ParticleDb & particleDb = db();
  ParticleType * type = (particleDb.findIndexForPdgCode(pdg)>=0) ? particleDb.findPdgCode(pdg) : nullptr;
  _typeOfCode[pdg] = type;
  if (!type) _skippedCodes[pdg]++;
  return type;
}

void PHSDEventReader::execute()
{
  // the header of the next event, in the current or a following file
  Header & h = _header;
  while (true)
    {
    if (!_inputFile && !openNextFile())
      throw EndOfDataException(_inputFileName,"End of the PHSD input",__FUNCTION__);
    std::ifstream & in = *_inputFile;
    if (in >> h.nParticles)
      {
      in >> h.runIndex >> h.eventIndex >> h.impactParameter >> h.impactSamplingMethod
         >> h.pdgTarget >> h.pdgProjectile >> h.sqrtS
         >> h.nParticipants;
      for (int k=0; k<4; k++) in >> h.psi[k] >> h.epsilon[k];
      if (!in || h.nParticles<0) throw FileException(_inputFileName,"Incomplete PHSD event header",__FUNCTION__);
      break;
      }
    if (!in.eof()) throw FileException(_inputFileName,"Unreadable PHSD event header",__FUNCTION__);
    _inputFile.reset();                          // end of this file: go to the next one
    }

  Event & theEvent = event();
  ParticleFilter * particleFilter = particleFilters().empty() ? nullptr : particleFilters()[0];
  std::ifstream & in = *_inputFile;
  int    pdgCode, charge, phsdCode, parentCode;
  double px, py, pz, e;
  for (long iParticle=0; iParticle<h.nParticles; iParticle++)
    {
    if (!(in >> pdgCode >> charge >> px >> py >> pz >> e >> phsdCode >> parentCode))
      throw FileException(_inputFileName,"Incomplete PHSD event (fewer particles than announced)",__FUNCTION__);
    ParticleType * type = particleType(pdgCode);
    if (!type) continue;                         // not in the particle database (reported by finalize())
    Particle & particle = Particle::factory().nextObject();
    particle.reset();
    particle.setType(type);
    particle.setLive(true);
    particle.setEPxPyPz(e,px,py,pz);
    particle.setXYZT(0.0,0.0,0.0,0.0);
    if (!particleFilter || particleFilter->accept(particle)) theEvent.addParticle(&particle);
    }
  _eventAccepted.increment();
  _taskExecuted.increment();
}

void PHSDEventReader::finalize()
{
  if (!_skippedCodes.empty() && reportWarning(__FUNCTION__))
    {
    printValue("Particles skipped: PDG codes not in the particle database",long(_skippedCodes.size()));
    for (auto & entry : _skippedCodes)
      {
      String label = "PDG code "; label += entry.first;
      printValue(label.data(),entry.second);
      }
    }
  _inputFile.reset();
  EventProcessor::finalize();
}

} // namespace CAP
