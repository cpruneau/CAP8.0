/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
#ifndef CAP__EventProcessorHistos
#define CAP__EventProcessorHistos
#include <type_traits>
#include "EventProcessor.hpp"
#include "HistogramFolder.hpp"

namespace CAP
{
  //!
  //! @brief Base class of the tasks of an analysis that work on its histogram folders: the analyzer (event processing),
  //! and the calculators of the later phases (derived histograms, balance functions).
  //!
  //! The task manages one list of histogram folders (managed objects of type HISTOGRAM, configuration keys
  //! \<name\>:HISTOGRAM:USE, N, CREATE, IMPORT and \<name\>:HISTOGRAM_\<k\>:NAME, ...), one folder per combination of an
  //! event filter and RANK particle filters: index e*nPF^RANK + p1*nPF^(RANK-1) + ... (see combinationName()). Each
  //! folder holds all the histograms of the analysis for its combination: those filled event by event and those
  //! computed from them in the later phases.
  //!
  //! In the files of the jobs, the folders are those of a folder named after the analysis task (histogramTaskName()):
  //! the calculators of the later phases read and write the folders of the analyzer whose histograms they complete.
  //!
  //! Optionally (SINGLES not void, e.g. a pair analysis), the task also has a single-particle folder (SINGLES) per
  //! combination of an event filter and a particle filter, at index e*nPF + p, named combinationName(k,1) in the folder
  //! of the task. They are configured with the HISTOGRAM group of the task, and created, imported, saved, scaled and
  //! reset with the folders of the list.
  //!
  //! @tparam HISTOS histogram folder (a HistogramFolder) @tparam RANK number of particle filters per combination
  //! @tparam SINGLES single-particle histogram folder, or void (none)
  //!
  template <class HISTOS, unsigned int RANK, class SINGLES = void>
  class EventProcessorHistos
  :
  public EventProcessor
  {
  protected:
  //! Histogram folders, one per combination of filters.
  ManagedObjects<HISTOS> _managedHistograms;
  //! Type of the single-particle folders (HistogramFolder if there are none).
  using SingleFolder = std::conditional_t<std::is_void_v<SINGLES>, HistogramFolder, SINGLES>;
  //! Single-particle folders (SINGLES not void), one per (event filter, particle filter), at index e*nPF + p, owned.
  std::vector<SingleFolder*> _singleHistograms;

  public:

  //! @brief Default constructor.
  EventProcessorHistos()
  :
  EventProcessor(),
  _managedHistograms(),
  _singleHistograms()
  {
  appendClassName("EventProcessorHistos");
  }

  //! @brief Copy constructor. @param source task to copy
  EventProcessorHistos(const EventProcessorHistos & source)
  :
  EventProcessor(source),
  _managedHistograms(source._managedHistograms),
  _singleHistograms()
  {
  copySingles(source);
  }

  //! @brief Copy assignment. @param rhs task to copy @return this task
  EventProcessorHistos & operator=(const EventProcessorHistos & rhs)
  {
  if (this!=&rhs)
    {
    EventProcessor::operator=(rhs);
    _managedHistograms = rhs._managedHistograms;
    copySingles(rhs);
    }
  return *this;
  }

  //! @brief Destructor: deletes the single-particle folders.
  virtual ~EventProcessorHistos() { deleteSingles(); }

  //! @brief Add the default configuration of the task and of its list of histogram folders (HISTOGRAM).
  virtual void setDefaultConfiguration() override
  {
  EventProcessor::setDefaultConfiguration();
  _managedHistograms.setDefaultObjectConfiguration(Task::name(),"HISTOGRAM",configuration());
  // for RunCAP (one folder per combination of an event filter and FOLDER_RANK particle filters); not read here
  configuration().addProperty(createKey(Task::name(),"HISTOGRAM","FOLDER_RANK"),int(RANK));
  }

  //! @brief Configure the task, its histogram folders and its single-particle folders (if any).
  //! @param configuration configuration
  virtual void configure(const Configuration & configuration) override
  {
  EventProcessor::configure(configuration);
  _managedHistograms.configureObjects(configuration);
  deleteSingles();
  if constexpr (!std::is_void_v<SINGLES>)
    {
    unsigned int n = nEventFilters()*nParticleFilters();
    for (unsigned int k=0; k<n; k++)
      {
      SINGLES * single = new SINGLES();
      single->configure(Task::name(),"HISTOGRAM",configuration,k);
      _singleHistograms.push_back(single);
      }
    }
  }

  //! @brief Initialize the task and its folders (created here if CREATE; read later if IMPORT).
  virtual void initialize() override
  {
  EventProcessor::initialize();
  _managedHistograms.initialize();
  const ManagedObjects<HISTOS> & folderList = _managedHistograms;   // create() const: the option
  if (folderList.create()) for (SingleFolder * single : _singleHistograms) single->create();
  }

  //! @brief Finalize the task and its histogram folders.
  virtual void finalize() override
  {
  EventProcessor::finalize();
  _managedHistograms.finalize();
  }

  //! @brief Reset the task and its histogram folders.
  virtual void reset() override
  {
  EventProcessor::reset();
  _managedHistograms.reset();
  for (SingleFolder * single : _singleHistograms) single->reset();
  }

  //! @brief Clear the task and its histogram folders.
  virtual void clear() override
  {
  EventProcessor::clear();
  _managedHistograms.clear();
  deleteSingles();
  }

  //! @brief Print the task and its histogram folders.
  virtual void print() const override
  {
  EventProcessor::print();
  _managedHistograms.print();
  }

  //!
  //! @brief Name of the folder of the job files holding the histogram folders of this task: its own name. The
  //! calculators of the later phases return the name of the analyzer whose histograms they complete.
  //! @return folder name
  //!
  virtual String histogramTaskName() const { return Task::name(); }

  //!
  //! @brief Add the histogram folders to the output folder of the job: a folder named histogramTaskName() holding a
  //! folder per combination of filters (combinationName()); then those of the subtasks. @param top folder of the job
  //!
  void saveHistograms(Folder & top) override
  {
  if (_managedHistograms.size()>0)
    {
    String name = histogramTaskName();
    Folder * folder = top.folder(name);
    if (!folder) folder = top.addFolder(name);
    auto namer = [this](unsigned int k) { return this->combinationName(k,RANK); };
    _managedHistograms.saveToFolders(*folder,namer);
    for (unsigned int k=0; k<_singleHistograms.size(); k++)
      _singleHistograms[k]->saveTo(*folder->addFolder(combinationName(k,1)));
    }
  EventProcessor::saveHistograms(top);
  }

  //!
  //! @brief Read the histogram folders imported (IMPORT) from the input folder of the job (folder histogramTaskName(),
  //! a folder per combination of filters); then those of the subtasks. @param top folder of the job
  //! @throws Exception if a folder is missing
  //!
  void loadHistograms(const Folder & top) override
  {
  if (_managedHistograms.imports() && _managedHistograms.size()>0)
    {
    Folder * folder = top.folder(histogramTaskName());
    if (!folder) throw Exception(String("No folder ")+histogramTaskName()+" in the input file",__FUNCTION__);
    auto namer = [this](unsigned int k) { return this->combinationName(k,RANK); };
    _managedHistograms.loadFromFolders(*folder,namer);
    for (unsigned int k=0; k<_singleHistograms.size(); k++)
      {
      const String name = combinationName(k,1);
      Folder * singleFolder = folder->folder(name);
      if (!singleFolder) throw Exception(String("No folder ")+name+" of "+histogramTaskName()+" in the input file",__FUNCTION__);
      _singleHistograms[k]->loadFrom(*singleFolder);
      }
    }
  EventProcessor::loadHistograms(top);
  }

  //! @brief Number of histogram folders. @return number
  unsigned int nHistograms() const { return _managedHistograms.size(); }
  //! @brief Histogram folders. @return folders
  const std::vector<HISTOS*> & histograms() const { return _managedHistograms.getObjects(); }
  //! @brief Histogram folders. @return folders
  std::vector<HISTOS*> & histograms() { return _managedHistograms.getObjects(); }

  //! @brief Single-particle folders (none if SINGLES is void), at index e*nPF + p. @return folders
  std::vector<SingleFolder*> & singleHistograms() { return _singleHistograms; }
  //! @brief Single-particle folders (none if SINGLES is void), at index e*nPF + p. @return folders
  const std::vector<SingleFolder*> & singleHistograms() const { return _singleHistograms; }

  //! @brief Number of combinations of filters (folders expected): nEventFilters()*nParticleFilters()^RANK. @return number
  unsigned int nCombinations() const
  {
  unsigned int n = nEventFilters();
  for (unsigned int k=0; k<RANK; k++) n *= nParticleFilters();
  return n;
  }

  using EventProcessor::scaleHistograms;
  //!
  //! @brief Scale all the histograms of the folders of each event filter by its factor.
  //! @param factors factor of each event filter
  //! @throws Exception if there are fewer factors than event filters or fewer folders than combinations
  //!
  void scaleHistograms(const std::vector<double> & factors) override
  {
  std::vector<HISTOS*> & folders = histograms();
  unsigned int nEF = nEventFilters();
  unsigned int perEventFilter = nEF>0 ? nCombinations()/nEF : 0;
  if (factors.size()<nEF)                 throw Exception("factors.size()<nEventFilters()",__FUNCTION__);
  if (folders.size()<nEF*perEventFilter)  throw Exception("Fewer histogram folders than combinations of filters",__FUNCTION__);
  for (unsigned int iEventFilter=0; iEventFilter<nEF; iEventFilter++)
    for (unsigned int k=0; k<perEventFilter; k++)
      folders[iEventFilter*perEventFilter+k]->scaleHistograms(factors[iEventFilter]);
  unsigned int nPF = nParticleFilters();
  for (unsigned int k=0; k<_singleHistograms.size(); k++) _singleHistograms[k]->scaleHistograms(factors[k/nPF]);
  }

  protected:

  //! @brief Delete the single-particle folders.
  void deleteSingles()
  {
  for (SingleFolder * single : _singleHistograms) delete single;
  _singleHistograms.clear();
  }

  //! @brief Replace the single-particle folders by copies of those of another task. @param source task
  void copySingles(const EventProcessorHistos & source)
  {
  deleteSingles();
  for (const SingleFolder * single : source._singleHistograms) _singleHistograms.push_back(new SingleFolder(*single));
  }

  };

} // namespace CAP

#endif /* CAP__EventProcessorHistos */
