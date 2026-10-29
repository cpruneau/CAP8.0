/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
#ifndef CAP__ParticlePairHistosTask
#define CAP__ParticlePairHistosTask
#include "EventProcessorHistos.hpp"
#include "ParticleSingleHistos.hpp"
#include "ParticlePairHistos.hpp"

namespace CAP
{

//!
//! @brief Base class of the tasks of a pair analysis (ParticlePairAnalyzer, ParticlePairDerivedCalculator): they work
//! on the folders of the analysis, for n event filters and m particle filters:
//!
//! - n*m*m pair folders (ParticlePairHistos), one per (event filter e, particle filter i, particle filter j), at
//!   index e*m*m + i*m + j: the list of histogram folders of the task (HISTOGRAM, see EventProcessorHistos);
//! - n*m single-particle folders (ParticleSingleHistos), one per (event filter e, particle filter i), at index e*m + i,
//!   with the binning and fill options of the HISTOGRAM group of the task. They are created and imported with the pair
//!   folders.
//!
//! In the files of the jobs, all are in the folder named after the analyzer (histogramTaskName()): the single folders
//! are named \<event\>_\<particle\> (e.g. ALL_PiP), the pair folders \<event\>_\<particle i\>_\<particle j\> (e.g.
//! ALL_PiP_PiM). The histograms have short names (n1_y, n2_yY, R2_yY, ...).
//!
class ParticlePairHistosTask
:
public EventProcessorHistos<ParticlePairHistos,2>
{
public:

  //! @brief Default constructor.
  ParticlePairHistosTask()
  :
  EventProcessorHistos<ParticlePairHistos,2>(),
  _singleHistograms()
  {
  appendClassName("ParticlePairHistosTask");
  }

  //! @brief Copy constructor (copies the single folders). @param source task to copy
  ParticlePairHistosTask(const ParticlePairHistosTask & source)
  :
  EventProcessorHistos<ParticlePairHistos,2>(source),
  _singleHistograms()
  {
  copySingles(source);
  }

  //! @brief Copy assignment (copies the single folders). @param rhs task to copy @return this task
  ParticlePairHistosTask & operator=(const ParticlePairHistosTask & rhs)
  {
  if (this!=&rhs)
    {
    EventProcessorHistos<ParticlePairHistos,2>::operator=(rhs);
    copySingles(rhs);
    }
  return *this;
  }

  //! @brief Destructor: deletes the single folders.
  virtual ~ParticlePairHistosTask() { deleteSingles(); }

  //! @brief Add the default configuration: that of EventProcessorHistos and the binning and fill options of the
  //! particles and of the pairs (ParticlePairHistos::addDefaultConfiguration()).
  virtual void setDefaultConfiguration() override
  {
  EventProcessorHistos<ParticlePairHistos,2>::setDefaultConfiguration();
  ParticlePairHistos::addDefaultConfiguration(_configuration,name());
  }

  //! @brief Configure the task, its pair folders and its n*m single folders. @param configuration configuration
  virtual void configure(const Configuration & configuration) override
  {
  EventProcessorHistos<ParticlePairHistos,2>::configure(configuration);
  deleteSingles();
  unsigned int n = nEventFilters()*nParticleFilters();
  for (unsigned int k=0; k<n; k++)
    {
    ParticleSingleHistos * single = new ParticleSingleHistos();
    single->setName(combinationName(k,1));
    single->configure(name(),"HISTOGRAM",configuration,k);
    _singleHistograms.push_back(single);
    }
  }

  //! @brief Initialize the task; the single folders are created with the pair folders (CREATE).
  virtual void initialize() override
  {
  EventProcessorHistos<ParticlePairHistos,2>::initialize();
  const ManagedObjects<ParticlePairHistos> & folderList = _managedHistograms;   // create() const: the option
  if (folderList.create()) for (ParticleSingleHistos * single : _singleHistograms) single->create();
  }

  //! @brief Reset the task and its folders.
  virtual void reset() override
  {
  EventProcessorHistos<ParticlePairHistos,2>::reset();
  for (ParticleSingleHistos * single : _singleHistograms) single->reset();
  }

  //! @brief Clear the task and delete its single folders.
  virtual void clear() override
  {
  EventProcessorHistos<ParticlePairHistos,2>::clear();
  deleteSingles();
  }

  //! @brief Add the pair folders and the single folders to the output folder of the job (folder histogramTaskName()).
  //! @param top folder of the job
  void saveHistograms(Folder & top) override
  {
  EventProcessorHistos<ParticlePairHistos,2>::saveHistograms(top);
  if (_singleHistograms.empty()) return;
  Folder * folder = top.folder(histogramTaskName());
  if (!folder) folder = top.addFolder(histogramTaskName());
  for (unsigned int k=0; k<_singleHistograms.size(); k++)
    _singleHistograms[k]->saveTo(*folder->addFolder(combinationName(k,1)));
  }

  //! @brief Read the pair folders and the single folders imported (IMPORT) from the input folder of the job.
  //! @param top folder of the job @throws Exception if a folder is missing
  void loadHistograms(const Folder & top) override
  {
  EventProcessorHistos<ParticlePairHistos,2>::loadHistograms(top);
  if (!_managedHistograms.imports() || _singleHistograms.empty()) return;
  Folder * folder = top.folder(histogramTaskName());
  if (!folder) throw Exception(String("No folder ")+histogramTaskName()+" in the input file",__FUNCTION__);
  for (unsigned int k=0; k<_singleHistograms.size(); k++)
    {
    const String folderName = combinationName(k,1);
    Folder * singleFolder = folder->folder(folderName);
    if (!singleFolder)
      throw Exception(String("Folder ")+folderName+" of "+histogramTaskName()+" not found in the input file",__FUNCTION__);
    _singleHistograms[k]->loadFrom(*singleFolder);
    }
  }

  using EventProcessorHistos<ParticlePairHistos,2>::scaleHistograms;
  //!
  //! @brief Scale the pair and single folders of each event filter by its factor.
  //! @param factors factor of each event filter
  //!
  void scaleHistograms(const std::vector<double> & factors) override
  {
  EventProcessorHistos<ParticlePairHistos,2>::scaleHistograms(factors);
  unsigned int nPF = nParticleFilters();
  for (unsigned int k=0; k<_singleHistograms.size(); k++) _singleHistograms[k]->scaleHistograms(factors[k/nPF]);
  }

  //! @brief Single folders, at index e*m + i. @return folders
  std::vector<ParticleSingleHistos*> & singleHistograms() { return _singleHistograms; }
  //! @brief Single folders, at index e*m + i. @return folders
  const std::vector<ParticleSingleHistos*> & singleHistograms() const { return _singleHistograms; }

protected:

  //! Single folders, one per (event filter, particle filter), owned.
  std::vector<ParticleSingleHistos*> _singleHistograms;

  //! @brief Delete the single folders.
  void deleteSingles()
  {
  for (ParticleSingleHistos * single : _singleHistograms) delete single;
  _singleHistograms.clear();
  }

  //! @brief Replace the single folders by copies of those of another task. @param source task
  void copySingles(const ParticlePairHistosTask & source)
  {
  deleteSingles();
  for (const ParticleSingleHistos * single : source._singleHistograms)
    _singleHistograms.push_back(new ParticleSingleHistos(*single));
  }

};

} // namespace CAP

#endif /* CAP__ParticlePairHistosTask */
