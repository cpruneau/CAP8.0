/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__EventProcessorSingle
#define CAP__EventProcessorSingle
#include "EventProcessor.hpp"

namespace CAP
{
  //!
  //! @brief Base class of the single-particle analyzers: an EventProcessor that manages one group of base histograms
  //! (HISTOGRAM_1, filled event by event) and one group of derived histograms (HISTOGRAM_2, computed at the end) per
  //! (event filter, particle filter) combination.
  //!
  //! The groups are managed objects of types HISTOGRAM_1 and HISTOGRAM_2 (configuration keys \<name\>:HISTOGRAM_1_\<k\>:...,
  //! \<name\>:HISTOGRAM_2_\<k\>:...). The group of event filter e and particle filter p has index e*nParticleFilters()+p.
  //!
  //! @tparam HISTOGRAM_1 base histograms (a BaseHistos) @tparam HISTOGRAM_2 derived histograms (a BaseDerivedHistos)
  //!
  template <class HISTOGRAM_1,class HISTOGRAM_2>
  class EventProcessorSingle
  :
  public EventProcessor
  {
  protected:
  //! Base histogram groups.
  ManagedObjects<HISTOGRAM_1>        _managedHistograms_1s;
  //! Derived histogram groups.
  ManagedObjects<HISTOGRAM_2>        _managedHistograms_2s;

  public:

  //! @brief Default constructor.
  EventProcessorSingle()
  :
  EventProcessor(),
  _managedHistograms_1s(),
  _managedHistograms_2s()
  {
  appendClassName("EventProcessorSingle");
  setMinimumReportLevel(Object::kInfo);
  setName("EventProcessorSingle");
  setTitle("EventProcessorSingle");
  }

  //! @brief Copy constructor. @param source task to copy
  EventProcessorSingle(const EventProcessorSingle & source)
  :
  EventProcessor(source),
  _managedHistograms_1s(source._managedHistograms_1s),
  _managedHistograms_2s(source._managedHistograms_2s)
  {   }

  //! @brief Copy assignment. @param rhs task to copy @return this task
  EventProcessorSingle & operator=(const EventProcessorSingle & rhs)
  {
  if (this!=&rhs)
    {
    EventProcessor::operator=(rhs);
    _managedHistograms_1s        = rhs._managedHistograms_1s;
    _managedHistograms_2s        = rhs._managedHistograms_2s;
    }
  return *this;
  }

  //! @brief Destructor.
  virtual ~EventProcessorSingle() {}

  //! @brief Add the default configuration of the task and of the histogram groups (HISTOGRAM_1, HISTOGRAM_2).
  virtual void setDefaultConfiguration() override
  {
  EventProcessor::setDefaultConfiguration();
  const String & taskName = Task::name();
  _managedHistograms_1s.setDefaultObjectConfiguration(taskName,"HISTOGRAM_1",configuration());
  _managedHistograms_2s.setDefaultObjectConfiguration(taskName,"HISTOGRAM_2",configuration());
  }

  //! @brief Configure the task and the histogram groups. @param configuration configuration
  virtual void configure(const Configuration & configuration) override
  {
  if (reportDebug(__FUNCTION__)) { printCR();  }
  EventProcessor::configure(configuration);
  _managedHistograms_1s.configureObjects(configuration);
  _managedHistograms_2s.configureObjects(configuration);
  }


  //! @brief Initialize the task and the histogram groups.
  virtual void initialize() override
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  EventProcessor::initialize();
  _managedHistograms_1s.initialize();
  _managedHistograms_2s.initialize();
  }

  //! @brief Finalize the task and the histogram groups.
  virtual void finalize() override
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  EventProcessor::finalize();
  _managedHistograms_1s.finalize();
  _managedHistograms_2s.finalize();
  }

  //! @brief Reset the task and the histogram groups.
  virtual void reset() override
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  EventProcessor::reset();
  _managedHistograms_1s.reset();
  _managedHistograms_2s.reset();
  }

  //!
  //! @brief Add the histograms to the folder of the job: a folder named after the task, holding a folder per
  //! combination of filters (combinationName()) with its histograms and derived histograms; then those of the
  //! subtasks. @param top folder of the job
  //!
  void saveHistograms(Folder & top) override
  {
  if (_managedHistograms_1s.size()+_managedHistograms_2s.size()>0)
    {
    Folder * folder = top.addFolder(Task::name());
    auto namer = [this](unsigned int k) { return this->combinationName(k,1); };
    _managedHistograms_1s.saveToFolders(*folder,namer);
    _managedHistograms_2s.saveToFolders(*folder,namer);
    }
  EventProcessor::saveHistograms(top);
  }

  //!
  //! @brief Read the histograms imported (IMPORT) from the folder of the job (the folder named after the task, a folder
  //! per combination of filters); then those of the subtasks. @param top folder of the job
  //! @throws Exception if a folder is missing
  //!
  void loadHistograms(const Folder & top) override
  {
  bool import1 = _managedHistograms_1s.imports() && _managedHistograms_1s.size()>0;
  bool import2 = _managedHistograms_2s.imports() && _managedHistograms_2s.size()>0;
  if (import1 || import2)
    {
    Folder * folder = top.folder(Task::name());
    if (!folder) throw Exception(String("No folder ")+Task::name()+" in the input file",__FUNCTION__);
    auto namer = [this](unsigned int k) { return this->combinationName(k,1); };
    if (import1) _managedHistograms_1s.loadFromFolders(*folder,namer);
    if (import2) _managedHistograms_2s.loadFromFolders(*folder,namer);
    }
  EventProcessor::loadHistograms(top);
  }

  //! @brief Clear the task and the histogram groups.
  virtual void clear() override
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  EventProcessor::clear();
  _managedHistograms_1s.clear();
  _managedHistograms_2s.clear();
  }

  //! @brief Print the task and the histogram groups.
  virtual void print() const override
  {
  if (reportDebug(__FUNCTION__)) { printCR();}
  EventProcessor::print();
  _managedHistograms_1s.print();
  _managedHistograms_2s.print();
  }

  //! @brief Number of base histogram groups. @return number
  unsigned int nHistograms_1() const  { return _managedHistograms_1s.size();  }
  //! @brief Number of derived histogram groups. @return number
  unsigned int nHistograms_2() const  { return _managedHistograms_2s.size();  }

  //! @brief Base histogram groups. @return groups
  const std::vector<HISTOGRAM_1*> & histograms_1() const         { return _managedHistograms_1s.getObjects(); }
  //! @brief Derived histogram groups. @return groups
  const std::vector<HISTOGRAM_2*> & histograms_2() const         { return _managedHistograms_2s.getObjects(); }
  //! @brief Base histogram groups. @return groups
  std::vector<HISTOGRAM_1*> & histograms_1()                     { return _managedHistograms_1s.getObjects(); }
  //! @brief Derived histogram groups. @return groups
  std::vector<HISTOGRAM_2*> & histograms_2()                     { return _managedHistograms_2s.getObjects(); }

  //! @brief Scale the base histograms of each event filter by its factor. @param factors factor of each event filter
  //! @throws Exception if there are fewer factors than event filters or fewer groups than combinations
  void scaleHistograms(const std::vector<double> & factors) override
  {
  if (reportDebug(__FUNCTION__))
    {
    printCR();
    printValue("factors.size()", factors.size());
    printValue("nEventFilters()",    _managedEventFilters.size());
    printValue("nParticleFilters()", _managedParticleFilters.size());
    }
  std::vector<HISTOGRAM_1*> & histos_1 = histograms_1();
  unsigned int nEF = nEventFilters();
  unsigned int nPF = nParticleFilters();
  if (factors.size()<nEF)        throw Exception("factors.size()<nEventFilters()",__FUNCTION__);
  if (histos_1.size()<nEF*nPF)   throw Exception("histograms_1().size()<nEventFilters()*nParticleFilters()",__FUNCTION__);
  for (unsigned int iEventFilter=0; iEventFilter<nEF; iEventFilter++)
    {
    unsigned int baseSingleIndex = iEventFilter*nPF;
    for (unsigned int iParticleFilter=0; iParticleFilter<nPF; iParticleFilter++)
      histos_1[baseSingleIndex + iParticleFilter]->scaleHistograms(factors[iEventFilter]);   // once (was done twice)
    }
  if (reportDebug(__FUNCTION__))
    {
    printCR();
    printString("DONE!!!!! ");
    }
  }

  //! @brief Compute the derived histograms of each (event filter, particle filter) combination from its base histograms.
  virtual void calculateDerived() override
  {
  if (EventProcessor::reportDebug(__FUNCTION__))  { printCR(); };
  std::vector<HISTOGRAM_1*> & baseSingleHistos      = histograms_1();
  std::vector<HISTOGRAM_2*> & derivedSingleHistos   = histograms_2();

  unsigned int nEF = nEventFilters();
  unsigned int nPF = nParticleFilters();
  unsigned int baseSingle;
  unsigned int indexSingle;

  printValue("baseSingleHistos.size()",baseSingleHistos.size());
  printValue("derivedSingleHistos.size()",derivedSingleHistos.size());
  printValue("nEventFilters",nEF);
  printValue("nParticleFilters",nPF);

  for (unsigned int iEventFilter=0; iEventFilter<nEF; iEventFilter++)
    {
    baseSingle = iEventFilter*nPF;
    printValue("baseSingle",baseSingle);

    for (unsigned int iParticleFilter=0; iParticleFilter<nPF; iParticleFilter++)
      {
      indexSingle = baseSingle + iParticleFilter;
      printValue("indexSingle",indexSingle);
      derivedSingleHistos[indexSingle]->calculateDerivedHistograms(*(baseSingleHistos[indexSingle]));
      }
    }
  }

  };

} // namespace CAP

#endif /* CAP__EventProcessorSingle */
