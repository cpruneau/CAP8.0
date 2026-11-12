/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file JetTask.hpp
//! @brief JetTask: base of the jet analyzers: jet finding for a list of radii, jet filters, and one folder of histograms
//! per event filter, particle filter, radius, jet filter (and a further selection of the analyzer, e.g. pair filters).
#ifndef CAP__JetTask
#define CAP__JetTask
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>
#include "EventProcessor.hpp"
#include "Event.hpp"
#include "Exceptions.hpp"
#include "NameHelpers.hpp"
#include "JetClustering.hpp"
#include "JetFilter.hpp"

namespace CAP
{

//!
//! @brief Base of the jet analyzers, for a folder class HISTOS (a HistogramFolder with addDefaultConfiguration(),
//! configure(), create(), normalize(eventFactor)).
//!
//! The particles accepted by each particle filter are clustered into jets (JetClustering, once per event, particle
//! filter and radius, shared with the other jet tasks) for each radius of the list JET:R; the jets accepted by each jet
//! filter (JET_FILTER) are passed to fill() with the folders of their combination.
//!
//! Folders: one per event filter e, particle filter p, radius r, jet filter j and selection x of the derived class
//! (nSelections(), 1 by default), at index (((e*nP + p)*nR + r)*nJ + j)*nX + x, named
//! \<event filter\>_\<particle filter\>_\<R label\>_\<jet filter\>[_\<selection\>].
//!
//! Settings (keys \<task\>:JET:\<name\>): R (list of radii, e.g. "0.2, 0.4, 0.6"), ALGORITHM (ANTIKT, KT or CA),
//! RECOMBINATION (E or PT), PT_MIN (minimum jet pT, GeV/c), PARTICLE_ETA_MAX (pseudorapidity range of the particles:
//! the jets kept have |eta| < PARTICLE_ETA_MAX - R, their rank is counted among those). Jet filters: the keys of a list
//! of managed objects (\<task\>:JET_FILTER:N, JET_FILTER_\<k\>:NAME, ...), made by JetFilterCreator. Binning: HISTOS.
//!
//! At the end of the run, each folder is normalized (HISTOS::normalize()) with the factor of its event filter
//! (1/number of events accepted).
//!
//! With HISTOGRAM:IMPORT true (a post-processing job, e.g. a FileIterator after the sub-samples are merged), the folders
//! are read from the input file of the job (loadHistograms(), HISTOS::loadFrom()) and not normalized again; the
//! derived histograms of a derived class are then computed by calculateDerived() (none here).
//!
template <class HISTOS>
class JetTask : public EventProcessor
{
public:

  //! @brief Default constructor.
  JetTask() : EventProcessor(), _managedJetFilters(), _radii(), _settings(), _folders(), _folderNames() { }
  //! @brief Copy constructor. @param source task
  JetTask(const JetTask & source)
  :
  EventProcessor(source), _managedJetFilters(source._managedJetFilters), _radii(source._radii),
  _settings(source._settings), _folders(), _import(source._import), _folderNames(source._folderNames)
  {
  for (HISTOS * folder : source._folders) _folders.push_back(new HISTOS(*folder));
  }
  //! @brief Copy assignment. @param rhs task @return this task
  JetTask & operator=(const JetTask & rhs)
  {
  if (this!=&rhs)
    {
    EventProcessor::operator=(rhs);
    _managedJetFilters = rhs._managedJetFilters;
    _radii = rhs._radii;
    _settings = rhs._settings;
    _import = rhs._import;
    _folderNames = rhs._folderNames;
    deleteFolders();
    for (HISTOS * folder : rhs._folders) _folders.push_back(new HISTOS(*folder));
    }
  return *this;
  }
  //! @brief Destructor (deletes the folders).
  virtual ~JetTask() { deleteFolders(); }

  //! @brief Add the default configuration: jet finder (JET), jet filters (JET_FILTER), binning (HISTOGRAM).
  virtual void setDefaultConfiguration() override
  {
  EventProcessor::setDefaultConfiguration();
  const String & taskName = name();
  _configuration.addProperty(createKey(taskName,"JET","R"),                String("0.4"));
  _configuration.addProperty(createKey(taskName,"JET","ALGORITHM"),        String("ANTIKT"));
  _configuration.addProperty(createKey(taskName,"JET","RECOMBINATION"),    String("E"));
  _configuration.addProperty(createKey(taskName,"JET","PT_MIN"),           5.0);
  _configuration.addProperty(createKey(taskName,"JET","PARTICLE_ETA_MAX"), 0.9);
  _configuration.addProperty(createKey(taskName,"HISTOGRAM","IMPORT"),     false);
  _managedJetFilters.setDefaultObjectConfiguration(taskName,"JET_FILTER",_configuration);
  HISTOS::addDefaultConfiguration(_configuration,taskName);
  }

  //! @brief Configure the task, the jet filters, the selections of the derived class and the folders.
  //! @param configuration configuration @throws ConfigurationException if a setting is invalid
  virtual void configure(const Configuration & configuration) override
  {
  EventProcessor::configure(configuration);
  const String & taskName = name();
  auto key = [&](const char * k) { return createKey(taskName,"JET",k); };
  _settings.algorithm      = JetSettings::algorithmOf(configuration.valueString(key("ALGORITHM")));
  _settings.recombination  = JetSettings::recombinationOf(configuration.valueString(key("RECOMBINATION")));
  _settings.ptMin          = configuration.valueDouble(key("PT_MIN"));
  _settings.particleEtaMax = configuration.valueDouble(key("PARTICLE_ETA_MAX"));
  _import = configuration.valueBool(createKey(taskName,"HISTOGRAM","IMPORT"));
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
  configureSelections(configuration);
  deleteFolders();
  _folderNames.clear();
  const unsigned int n = nEventFilters()*nParticleFilters()*_radii.size()*nJetFilters()*nSelections();
  for (unsigned int k=0; k<n; k++) _folderNames.push_back(makeFolderName(k));
  for (unsigned int k=0; k<n; k++)
    {
    HISTOS * folder = new HISTOS();
    folder->configure(taskName,"HISTOGRAM",configuration,k);
    folder->setName(_folderNames[k]);
    _folders.push_back(folder);
    }
  }

  //! @brief Initialize the task, the jet filters and the selections, and create the histograms.
  virtual void initialize() override
  {
  EventProcessor::initialize();
  _managedJetFilters.initialize();
  initializeSelections();
  for (HISTOS * folder : _folders) folder->create();
  }

  //! @brief Find the jets of the current event and fill the folders of the jets accepted by each jet filter.
  virtual void execute() override
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
          if (_acceptedEventFilters[e]) fill(folderIndex(e,p,r,j,0),jets,accepted,particles,jetSettings);
        }
      }
    }
  }

  //! @brief Normalize the folders (factor of their event filter), then finalize.
  virtual void finalize() override
  {
  if (!_import) EventProcessor::scaleHistograms();   // folders read from a file: already normalized
  _managedJetFilters.finalize();
  finalizeSelections();
  EventProcessor::finalize();
  }

  //! @brief Normalize the folders of each event filter. @param factors factors (1/number of events)
  virtual void scaleHistograms(const std::vector<double> & factors) override
  {
  const unsigned int perEventFilter = nEventFilters()>0 ? _folders.size()/nEventFilters() : 0;
  for (unsigned int k=0; k<_folders.size() && perEventFilter>0; k++) _folders[k]->normalize(factors.at(k/perEventFilter));
  }

  //! @brief Write the folders to the folder of the task in the output folder of the job. @param top folder of the job
  virtual void saveHistograms(Folder & top) override
  {
  saveFolders(top);
  EventProcessor::saveHistograms(top);
  }

  //! @brief Add the folders, empty, to a folder. @param top folder
  virtual void loadHistograms(const Folder & top) override
  {
  if (_import)
    {
    Folder * folder = top.folder(name());
    if (!folder) throw Exception(String("No folder ")+name()+" in the input file",__FUNCTION__);
    for (unsigned int k=0; k<_folders.size(); k++)
      {
      Folder * sub = folder->folder(folderName(k));
      if (!sub) throw Exception(String("No folder ")+folderName(k)+" of "+name()+" in the input file",__FUNCTION__);
      _folders[k]->loadFrom(*sub);
      }
    }
  EventProcessor::loadHistograms(top);
  }

  virtual void calculateDerived() override { }

  virtual void makeEmptyHistograms(Folder & top) override
  {
  for (HISTOS * folder : _folders) folder->create();
  saveFolders(top);
  }

  //! @name Settings and folders
  //! @{
  //! @brief Radii. @return radii
  const std::vector<double> & radii() const { return _radii; }
  //! @brief Settings of the jet finder of a radius. @param r index of the radius @return settings
  JetSettings settings(unsigned int r) const { JetSettings s = _settings; s.radius = _radii.at(r); return s; }
  //! @brief Number of jet filters. @return number
  unsigned int nJetFilters() const { return _managedJetFilters.size(); }
  //! @brief Jet filters. @return filters
  const std::vector<JetFilter*> & jetFilters() const { return _managedJetFilters.getObjects(); }
  //! @brief Number of selections of the derived class per jet filter (1 by default). @return number
  virtual unsigned int nSelections() const { return 1; }
  //! @brief Name of a selection of the derived class ("" for none). @param x selection @return name
  virtual String selectionName(unsigned int x __attribute__((unused))) const { return String(""); }
  //! @brief Index of the folder of a combination. @param e event filter @param p particle filter @param r radius
  //! @param j jet filter @param x selection @return index
  unsigned int folderIndex(unsigned int e, unsigned int p, unsigned int r, unsigned int j, unsigned int x) const
  { return (((e*nParticleFilters()+p)*_radii.size()+r)*nJetFilters()+j)*nSelections()+x; }
  //! @brief Name of a folder (recorded at configuration). @param k index @return name
  String folderName(unsigned int k) const { return k<_folderNames.size() ? _folderNames[k] : makeFolderName(k); }
  //! @brief Folders. @return folders
  const std::vector<HISTOS*> & folders() const { return _folders; }
  //! @}

protected:

  //! @brief Fill the folders of one event filter, particle filter, radius and jet filter.
  //! @param first index of the folder of the first selection (the others follow) @param jets all the jets of the
  //! event (this particle filter and radius), by rank @param accepted the jets accepted by the jet filter
  //! @param particles the particles accepted by the particle filter @param settings settings of the jet finder
  virtual void fill(unsigned int first, const std::vector<Jet> & jets, const std::vector<const Jet*> & accepted,
                    const std::vector<Particle*> & particles, const JetSettings & settings) = 0;
  //! @brief Read the selections of the derived class (before the folders are made). @param configuration configuration
  virtual void configureSelections(const Configuration & configuration __attribute__((unused))) { }
  //! @brief Initialize the selections of the derived class.
  virtual void initializeSelections() { }
  //! @brief Finalize the selections of the derived class.
  virtual void finalizeSelections() { }

  //! @brief Name of the folder of a combination from the filters. @param k index @return name
  String makeFolderName(unsigned int k) const
  {
  unsigned int nX = nSelections(), nJ = nJetFilters(), nR = _radii.size(), nP = nParticleFilters();
  if (nX==0 || nJ==0 || nR==0 || nP==0) return String("Folder_")+String(std::to_string(k).c_str());
  unsigned int x = k%nX;  k /= nX;
  unsigned int j = k%nJ;  k /= nJ;
  unsigned int r = k%nR;  k /= nR;
  unsigned int p = k%nP;  k /= nP;
  unsigned int e = k;
  String name = eventFilters()[e]->name()+"_"+particleFilters()[p]->name()+"_"+JetSettings::radiusLabel(_radii[r])+"_"
                +jetFilters()[j]->name();
  String selection = selectionName(x);
  if (!selection.isEmpty()) name += String("_")+selection;
  return name;
  }

  //! @brief Write the folders to the folder of the task. @param top folder of the job
  void saveFolders(Folder & top)
  {
  if (_folders.empty()) return;
  Folder * folder = top.folder(name());
  if (!folder) folder = top.addFolder(name());
  for (unsigned int k=0; k<_folders.size(); k++) _folders[k]->saveTo(*folder->addFolder(folderName(k)));
  }

  //! @brief Delete the folders.
  void deleteFolders()
  {
  for (HISTOS * folder : _folders) delete folder;
  _folders.clear();
  }

  ManagedObjects<JetFilter> _managedJetFilters;   //!< jet filters
  std::vector<double> _radii;                     //!< radii
  JetSettings _settings;                          //!< settings (the radius set per radius)
  std::vector<HISTOS*> _folders;                  //!< folders
  bool _import = false;                           //!< folders read from the input file of the job (HISTOGRAM:IMPORT)
  std::vector<String> _folderNames;               //!< names of the folders (the filters are released by finalize())
};

} // namespace CAP

#endif /* CAP__JetTask */
