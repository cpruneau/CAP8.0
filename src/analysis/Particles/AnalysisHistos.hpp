/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
//! @file AnalysisHistos.hpp
//! @brief AnalysisHistos: the histogram folders of an analysis task, by combination of filters; the aliases
//! EventProcessorHistos (analyzers) and CalculatorHistos (calculators of the later phases).
#ifndef CAP__AnalysisHistos
#define CAP__AnalysisHistos
#include <type_traits>
#include "AnalysisTask.hpp"
#include "EventProcessor.hpp"
#include "HistogramFolder.hpp"
#include "PrintHelpers.hpp"

namespace CAP
{
  //!
  //! @brief Base class of the tasks of an analysis that work on its histogram folders: the analyzer (event processing),
  //! and the calculators of the later phases (derived histograms, balance functions).
  //!
  //! The template is used through EventProcessorHistos, for the analyzers (BASE = EventProcessor: events, filters,
  //! efficiencies, smearers, particle databases), and CalculatorHistos, for the calculators (BASE = AnalysisTask: the
  //! filters only, no events; plus SOURCE_TASK, the analyzer whose folders they complete).
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
  //! @tparam BASE base task: AnalysisTask, or a class derived from it (EventProcessor)
  //!
  template <class HISTOS, unsigned int RANK, class SINGLES, class BASE>
  class AnalysisHistos
  :
  public BASE
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
  AnalysisHistos()
  :
  BASE(),
  _managedHistograms(),
  _singleHistograms()
  {
  this->appendClassName("AnalysisHistos");
  }

  //! @brief Copy constructor. @param source task to copy
  AnalysisHistos(const AnalysisHistos & source)
  :
  BASE(source),
  _managedHistograms(source._managedHistograms),
  _singleHistograms()
  {
  copySingles(source);
  }

  //! @brief Copy assignment. @param rhs task to copy @return this task
  AnalysisHistos & operator=(const AnalysisHistos & rhs)
  {
  if (this!=&rhs)
    {
    BASE::operator=(rhs);
    _managedHistograms = rhs._managedHistograms;
    copySingles(rhs);
    }
  return *this;
  }

  //! @brief Destructor: deletes the single-particle folders.
  virtual ~AnalysisHistos() { deleteSingles(); }

  //! @brief Add the default configuration of the task and of its list of histogram folders (HISTOGRAM).
  virtual void setDefaultConfiguration() override
  {
  BASE::setDefaultConfiguration();
  _managedHistograms.setDefaultObjectConfiguration(this->name(),"HISTOGRAM",this->configuration());
  // for RunCAP (one folder per combination of an event filter and FOLDER_RANK particle filters); not read here
  this->configuration().addProperty(createKey(this->name(),"HISTOGRAM","FOLDER_RANK"),int(RANK));
  }

  //! @brief Configure the task, its histogram folders and its single-particle folders (if any).
  //! @param configuration configuration
  virtual void configure(const Configuration & configuration) override
  {
  BASE::configure(configuration);
  _managedHistograms.configureObjects(configuration);
  deleteSingles();
  if constexpr (!std::is_void_v<SINGLES>)
    {
    unsigned int n = this->nEventFilters()*this->nParticleFilters();
    for (unsigned int k=0; k<n; k++)
      {
      SINGLES * single = new SINGLES();
      single->configure(this->name(),"HISTOGRAM",configuration,k);
      _singleHistograms.push_back(single);
      }
    }
  }

  //! @brief Initialize the task and its folders (created here if CREATE; read later if IMPORT).
  virtual void initialize() override
  {
  BASE::initialize();
  _managedHistograms.initialize();
  const ManagedObjects<HISTOS> & folderList = _managedHistograms;   // create() const: the option
  if (folderList.create()) for (SingleFolder * single : _singleHistograms) single->create();
  }

  //! @brief Finalize the task and its histogram folders.
  virtual void finalize() override
  {
  BASE::finalize();
  _managedHistograms.finalize();
  }

  //! @brief Reset the task and its histogram folders.
  virtual void reset() override
  {
  BASE::reset();
  _managedHistograms.reset();
  for (SingleFolder * single : _singleHistograms) single->reset();
  }

  //! @brief Clear the task and its histogram folders.
  virtual void clear() override
  {
  BASE::clear();
  _managedHistograms.clear();
  deleteSingles();
  }

  //! @brief Print the task and its histogram folders.
  virtual void print() const override
  {
  BASE::print();
  _managedHistograms.print();
  }

  //!
  //! @brief Name of the folder of the job files holding the histogram folders of this task: its own name. The
  //! calculators of the later phases return the name of the analyzer whose histograms they complete.
  //! @return folder name
  //!
  virtual String histogramTaskName() const { return this->name(); }

  //!
  //! @brief Add the histogram folders to the output folder of the job: a folder named histogramTaskName() holding a
  //! folder per combination of filters (combinationName()); then those of the subtasks. @param top folder of the job
  //!
  void saveHistograms(Folder & top) override
  {
  if (_managedHistograms.size()>0)
    {
    String name = this->histogramTaskName();
    Folder * folder = top.folder(name);
    if (!folder) folder = top.addFolder(name);
    auto namer = [this](unsigned int k) { return this->combinationName(k,RANK); };
    _managedHistograms.saveToFolders(*folder,namer);
    for (unsigned int k=0; k<_singleHistograms.size(); k++)
      _singleHistograms[k]->saveTo(*folder->addFolder(this->combinationName(k,1)));
    }
  BASE::saveHistograms(top);
  }

  //!
  //! @brief Add to a folder the empty histogram folders of the task, as saveHistograms() would write them (see
  //! AnalysisTask::makeEmptyHistograms()): each folder's histograms created and, for the calculators (BASE is
  //! AnalysisTask), its derived histograms too. @param top folder of the job
  //!
  void makeEmptyHistograms(Folder & top) override
  {
  constexpr bool calculator = std::is_same_v<BASE, AnalysisTask>;
  for (HISTOS * h : _managedHistograms.getObjects())
    {
    h->create();
    if constexpr (calculator && hasCreateDerived<HISTOS>::value) h->createDerived();
    }
  for (SingleFolder * single : _singleHistograms)
    {
    single->create();
    if constexpr (calculator && hasCreateDerived<SingleFolder>::value) single->createDerived();
    }
  if (_managedHistograms.size()>0)
    {
    String name = this->histogramTaskName();
    Folder * folder = top.folder(name);
    if (!folder) folder = top.addFolder(name);
    auto namer = [this](unsigned int k) { return this->combinationName(k,RANK); };
    _managedHistograms.saveToFolders(*folder,namer);
    for (unsigned int k=0; k<_singleHistograms.size(); k++)
      _singleHistograms[k]->saveTo(*folder->addFolder(this->combinationName(k,1)));
    }
  }

  //! Whether a folder class has createDerived() (the folders of the analyses with derived histograms).
  template <class T, class = void> struct hasCreateDerived : std::false_type { };
  template <class T> struct hasCreateDerived<T, std::void_t<decltype(std::declval<T&>().createDerived())>> : std::true_type { };

  //!
  //! @brief Read the histogram folders imported (IMPORT) from the input folder of the job (folder histogramTaskName(),
  //! a folder per combination of filters); then those of the subtasks. @param top folder of the job
  //! @throws Exception if a folder is missing
  //!
  void loadHistograms(const Folder & top) override
  {
  if (_managedHistograms.imports() && _managedHistograms.size()>0)
    {
    Folder * folder = top.folder(this->histogramTaskName());
    if (!folder) throw Exception(String("No folder ")+this->histogramTaskName()+" in the input file",__FUNCTION__);
    auto namer = [this](unsigned int k) { return this->combinationName(k,RANK); };
    _managedHistograms.loadFromFolders(*folder,namer);
    for (unsigned int k=0; k<_singleHistograms.size(); k++)
      {
      const String name = this->combinationName(k,1);
      Folder * singleFolder = folder->folder(name);
      if (!singleFolder) throw Exception(String("No folder ")+name+" of "+this->histogramTaskName()+" in the input file",__FUNCTION__);
      _singleHistograms[k]->loadFrom(*singleFolder);
      }
    }
  BASE::loadHistograms(top);
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
  unsigned int n = this->nEventFilters();
  for (unsigned int k=0; k<RANK; k++) n *= this->nParticleFilters();
  return n;
  }

  using BASE::scaleHistograms;
  //!
  //! @brief Scale all the histograms of the folders of each event filter by its factor.
  //! @param factors factor of each event filter
  //! @throws Exception if there are fewer factors than event filters or fewer folders than combinations
  //!
  void scaleHistograms(const std::vector<double> & factors) override
  {
  std::vector<HISTOS*> & folders = this->histograms();
  unsigned int nEF = this->nEventFilters();
  unsigned int perEventFilter = nEF>0 ? this->nCombinations()/nEF : 0;
  if (factors.size()<nEF)                 throw Exception("factors.size()<nEventFilters()",__FUNCTION__);
  if (folders.size()<nEF*perEventFilter)  throw Exception("Fewer histogram folders than combinations of filters",__FUNCTION__);
  for (unsigned int iEventFilter=0; iEventFilter<nEF; iEventFilter++)
    for (unsigned int k=0; k<perEventFilter; k++)
      folders[iEventFilter*perEventFilter+k]->scaleHistograms(factors[iEventFilter]);
  unsigned int nPF = this->nParticleFilters();
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
  void copySingles(const AnalysisHistos & source)
  {
  deleteSingles();
  for (const SingleFolder * single : source._singleHistograms) _singleHistograms.push_back(new SingleFolder(*single));
  }

  };



  //! @brief Histogram folders of an analyzer (a task processing events). @tparam HISTOS folder @tparam RANK particle
  //! filters per combination @tparam SINGLES single-particle folder, or void
  template <class HISTOS, unsigned int RANK, class SINGLES = void>
  using EventProcessorHistos = AnalysisHistos<HISTOS,RANK,SINGLES,EventProcessor>;

  //!
  //! @brief Histogram folders of a calculator of the later phases of an analysis (no events: the filters only).
  //!
  //! A calculator completes the folders of the analyzer of its analysis: it reads (by default, HISTOGRAM:USE and IMPORT
  //! true, CREATE false) and writes the folders of the folder named by the key \<name\>:SOURCE_TASK (the name of the
  //! analyzer task) in the files of the jobs.
  //!
  //! @tparam HISTOS folder @tparam RANK particle filters per combination @tparam SINGLES single-particle folder, or void
  //!
  template <class HISTOS, unsigned int RANK, class SINGLES = void>
  class CalculatorHistos
  :
  public AnalysisHistos<HISTOS,RANK,SINGLES,AnalysisTask>
  {
  public:

  //! Base class.
  typedef AnalysisHistos<HISTOS,RANK,SINGLES,AnalysisTask> Base;

  //! @brief Constructor. @param defaultSourceTask default name of the analyzer task (SOURCE_TASK)
  CalculatorHistos(const String & defaultSourceTask = String(""))
  :
  Base(),
  _defaultSourceTask(defaultSourceTask),
  _sourceTask(defaultSourceTask)
  {
  this->appendClassName("CalculatorHistos");
  }

  //! @brief Copy constructor. @param source task to copy
  CalculatorHistos(const CalculatorHistos & source) = default;
  //! @brief Copy assignment. @param rhs task to copy @return this task
  CalculatorHistos & operator=(const CalculatorHistos & rhs) = default;
  //! @brief Destructor.
  virtual ~CalculatorHistos() {}

  //! @brief Add the default configuration: that of the histogram folders, read (USE, IMPORT) rather than created
  //! (CREATE), and SOURCE_TASK.
  virtual void setDefaultConfiguration() override
  {
  Base::setDefaultConfiguration();
  Configuration & configuration = this->configuration();
  const String & taskName = this->name();
  configuration.addProperty(createKey(taskName,"HISTOGRAM","USE"),    true);
  configuration.addProperty(createKey(taskName,"HISTOGRAM","IMPORT"), true);
  configuration.addProperty(createKey(taskName,"HISTOGRAM","CREATE"), false);
  configuration.addProperty(createKey(taskName,"SOURCE_TASK"),_defaultSourceTask);
  }

  //! @brief Configure the task, its histogram folders, and read SOURCE_TASK. @param configuration configuration
  virtual void configure(const Configuration & configuration) override
  {
  Base::configure(configuration);
  _sourceTask = configuration.valueString(createKey(this->name(),"SOURCE_TASK"));
  if (this->reportInfo(__FUNCTION__)) printValue(createKey(this->name(),"SOURCE_TASK"),_sourceTask);
  }

  //! @brief Name of the folder of the job files holding the histograms: that of the analyzer (SOURCE_TASK). @return name
  virtual String histogramTaskName() const override { return _sourceTask; }

  //! @brief Name of the analyzer task whose folders are completed (SOURCE_TASK). @return name
  const String & sourceTask() const { return _sourceTask; }

  protected:

  //! Default name of the analyzer task (SOURCE_TASK).
  String _defaultSourceTask;
  //! Name of the analyzer task whose folders are completed.
  String _sourceTask;

  };

} // namespace CAP

#endif /* CAP__AnalysisHistos */
