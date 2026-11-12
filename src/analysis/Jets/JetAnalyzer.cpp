/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
#include "JetAnalyzer.hpp"
#include <cstdlib>
#include <sstream>
#include "Exceptions.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"
#include "JetClustering.hpp"
#include "Event.hpp"

namespace CAP
{

JetAnalyzer::JetAnalyzer()
:
EventProcessor(),
_managedJetFilters(),
_radii(),
_settings(),
_folders(),
_folderNames()
{
  appendClassName("JetAnalyzer");
  setMinimumReportLevel(Object::kInfo);
  setName("JetAnalyzer");
  setTitle("JetAnalyzer");
}

JetAnalyzer::JetAnalyzer(const JetAnalyzer & source)
:
EventProcessor(source),
_managedJetFilters(source._managedJetFilters),
_radii(source._radii),
_settings(source._settings),
_folders(),
_folderNames(source._folderNames)
{
  for (JetHistos * folder : source._folders) _folders.push_back(new JetHistos(*folder));
}

JetAnalyzer & JetAnalyzer::operator=(const JetAnalyzer & rhs)
{
  if (this!=&rhs)
    {
    EventProcessor::operator=(rhs);
    _managedJetFilters = rhs._managedJetFilters;
    _radii = rhs._radii;
    _settings = rhs._settings;
    _folderNames = rhs._folderNames;
    deleteFolders();
    for (JetHistos * folder : rhs._folders) _folders.push_back(new JetHistos(*folder));
    }
  return *this;
}

JetAnalyzer::~JetAnalyzer()
{
  deleteFolders();
}

void JetAnalyzer::deleteFolders()
{
  for (JetHistos * folder : _folders) delete folder;
  _folders.clear();
}

void JetAnalyzer::setDefaultConfiguration()
{
  EventProcessor::setDefaultConfiguration();
  const String & taskName = name();
  _configuration.addProperty(createKey(taskName,"JET","R"),                String("0.4"));
  _configuration.addProperty(createKey(taskName,"JET","ALGORITHM"),        String("ANTIKT"));
  _configuration.addProperty(createKey(taskName,"JET","RECOMBINATION"),    String("E"));
  _configuration.addProperty(createKey(taskName,"JET","PT_MIN"),           5.0);
  _configuration.addProperty(createKey(taskName,"JET","PARTICLE_ETA_MAX"), 0.9);
  _managedJetFilters.setDefaultObjectConfiguration(taskName,"JET_FILTER",_configuration);
  JetHistos::addDefaultConfiguration(_configuration,taskName);
}

void JetAnalyzer::configure(const Configuration & configuration)
{
  EventProcessor::configure(configuration);
  const String & taskName = name();
  auto key = [&](const char * k) { return createKey(taskName,"JET",k); };
  _settings.algorithm      = JetSettings::algorithmOf(configuration.valueString(key("ALGORITHM")));
  _settings.recombination  = JetSettings::recombinationOf(configuration.valueString(key("RECOMBINATION")));
  _settings.ptMin          = configuration.valueDouble(key("PT_MIN"));
  _settings.particleEtaMax = configuration.valueDouble(key("PARTICLE_ETA_MAX"));
  _radii.clear();
  std::string list = configuration.valueString(key("R")).c_str();
  for (char & c : list) if (c==',' || c==';') c = ' ';
  std::istringstream stream(list);
  std::string item;
  while (stream >> item)
    {
    char * end = nullptr;
    double r = std::strtod(item.c_str(),&end);
    if (end==item.c_str() || *end!='\0' || !(r>0.0))
      throw ConfigurationException(key("R"),String("invalid radius ")+String(item.c_str()),__FUNCTION__);
    if (!(r<_settings.particleEtaMax))
      throw ConfigurationException(key("R"),String("radius ")+String(item.c_str())+" not below PARTICLE_ETA_MAX",__FUNCTION__);
    _radii.push_back(r);
    }
  if (_radii.empty()) throw ConfigurationException(key("R"),"no radius given",__FUNCTION__);
  _managedJetFilters.configureObjects(configuration);
  deleteFolders();
  _folderNames.clear();
  unsigned int n = nEventFilters()*nParticleFilters()*_radii.size()*nJetFilters();
  for (unsigned int k=0; k<n; k++) _folderNames.push_back(folderName(k));
  for (unsigned int k=0; k<n; k++)
    {
    JetHistos * folder = new JetHistos();
    folder->configure(taskName,"HISTOGRAM",configuration,k);
    folder->setName(folderName(k));
    _folders.push_back(folder);
    }
}

void JetAnalyzer::initialize()
{
  EventProcessor::initialize();
  _managedJetFilters.initialize();
  for (JetHistos * folder : _folders) folder->create();
}

JetSettings JetAnalyzer::settings(unsigned int r) const
{
  JetSettings s = _settings;
  s.radius = _radii.at(r);
  return s;
}

String JetAnalyzer::folderName(unsigned int k) const
{
  if (k<_folderNames.size()) return _folderNames[k];
  unsigned int nJ = nJetFilters(), nR = _radii.size(), nP = nParticleFilters();
  if (nJ==0 || nR==0 || nP==0) return String("Folder_")+String(std::to_string(k).c_str());
  unsigned int j = k%nJ;  k /= nJ;
  unsigned int r = k%nR;  k /= nR;
  unsigned int p = k%nP;  k /= nP;
  unsigned int e = k;
  return eventFilters()[e]->name()+"_"+particleFilters()[p]->name()+"_"+JetSettings::radiusLabel(_radii[r])+"_"
         +jetFilters()[j]->name();
}

void JetAnalyzer::execute()
{
  _taskExecuted.increment();
  Event & theEvent = event();
  if (!filterEvent(theEvent)) return;
  filterParticles(theEvent);                  // the events without accepted particles count (no jets)
  const unsigned int nE = nEventFilters(), nP = nParticleFilters(), nR = _radii.size(), nJ = nJetFilters();
  std::vector<const Jet*> accepted;
  for (unsigned int p=0; p<nP; p++)
    {
    const std::vector<Particle*> & particles = acceptedParticles(p);
    for (unsigned int r=0; r<nR; r++)
      {
      const JetSettings jetSettings = settings(r);
      const std::vector<Jet> & jets = JetClustering::jets(theEvent,particleFilters()[p],particles,jetSettings);
      for (unsigned int j=0; j<nJ; j++)
        {
        accepted.clear();
        for (const Jet & jet : jets) if (jetFilters()[j]->accept(jet)) accepted.push_back(&jet);
        for (unsigned int e=0; e<nE; e++)
          if (_acceptedEventFilters[e]) _folders[folderIndex(e,p,r,j)]->fill(accepted);
        }
      }
    }
}

void JetAnalyzer::scaleHistograms(const std::vector<double> & factors)
{
  const unsigned int perEventFilter = nEventFilters()>0 ? _folders.size()/nEventFilters() : 0;
  for (unsigned int k=0; k<_folders.size(); k++) _folders[k]->normalize(factors.at(k/perEventFilter));
}

void JetAnalyzer::finalize()
{
  EventProcessor::scaleHistograms();
  _managedJetFilters.finalize();
  EventProcessor::finalize();
}

void JetAnalyzer::saveHistograms(Folder & top)
{
  if (!_folders.empty())
    {
    Folder * folder = top.folder(name());
    if (!folder) folder = top.addFolder(name());
    for (unsigned int k=0; k<_folders.size(); k++) _folders[k]->saveTo(*folder->addFolder(folderName(k)));
    }
  EventProcessor::saveHistograms(top);
}

void JetAnalyzer::makeEmptyHistograms(Folder & top)
{
  for (JetHistos * h : _folders) h->create();
  if (!_folders.empty())
    {
    Folder * folder = top.folder(name());
    if (!folder) folder = top.addFolder(name());
    for (unsigned int k=0; k<_folders.size(); k++) _folders[k]->saveTo(*folder->addFolder(folderName(k)));
    }
}

} // namespace CAP
