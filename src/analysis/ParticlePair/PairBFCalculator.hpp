/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file PairBFCalculator.hpp
//! @brief PairBFCalculator: the balance-function phase of a pair analysis, for any kind of pair histograms.
#ifndef CAP__PairBFCalculator
#define CAP__PairBFCalculator
#include <sstream>
#include <utility>
#include <vector>
#include "AnalysisHistos.hpp"
#include "PairDerivedCalculator.hpp"
#include "ParticleSingleHistos.hpp"
#include "Exceptions.hpp"
#include "NameHelpers.hpp"
#include "PrintHelpers.hpp"

namespace CAP
{

//!
//! @brief Balance functions of a pair analysis (third phase, no event processing), common to the pair analyses: the
//! analyses differ only by their histograms (PAIR and BF), not by how they are called.
//!
//! The task reads the histograms of the analyzer named by \<name\>:SOURCE_TASK in the input file of the job (its single
//! folders (e, p) and pair folders (e, p1, p2)), with the same filters and histogram settings as the analyzer. It
//! computes the derived histograms of every folder, as the derived calculator does (the input file can be the output of
//! the analyzer or of the derived calculator), then, for each event filter e and each ordered pair of species (a, b),
//! the balance functions of a BF folder from the pair folders (e, a, b), (e, a, b̄), (e, ā, b), (e, ā, b̄) and the single
//! folders (e, a), (e, ā), (e, b), (e, b̄).
//!
//! The species are pairs of particle filters, a particle and its antiparticle, given by \<name\>:SPECIES, a list of
//! particle/antiparticle filter names separated by commas or spaces:
//! @code
//!   BF:SPECIES = PiP/PiM, KP/KM, PP/PM
//! @endcode
//! If SPECIES is empty (the default), the first half of the particle filters are the particles and the second half
//! their antiparticles, in the same order (filter k and filter k + n/2).
//!
//! The output file of the job holds the folders of the analyzer, completed with their derived histograms, and one BF
//! folder per (e, a, b), in the folder of the analyzer, named after the event filter and the two particles with the
//! suffix _BF (e.g. ALL_PiP_KP_BF).
//!
//! There is no class per analysis: the task of an analysis is an instance of this template, registered under its own
//! name (e.g. ParticlePairBFCalculator = PairBFCalculator<ParticlePairHistos,ParticlePairBFHistos>, registered as
//! CAP::ParticlePairBFCalculator in ParticlePairTasks.cpp).
//!
//! @tparam PAIR pair folder: static addDefaultConfiguration(configuration, taskName), hasDerived(), createDerived(),
//! calculateDerived(single1, single2, same)
//! @tparam BF balance-function folder: constructor BF(name), createFrom(const PAIR &), calculate(ab, abB, aBb, aBbB,
//! a, aB, b, bB), setParentTask(), and the static functions calculatorName() (name of the task class, without
//! namespace) and defaultSourceTask() (default name of the analyzer task)
//!
template <class PAIR, class BF>
class PairBFCalculator : public CalculatorHistos<PAIR,2,ParticleSingleHistos>
{
public:

  //! Base class.
  typedef CalculatorHistos<PAIR,2,ParticleSingleHistos> Base;

  //! @brief Default constructor: the name of the task and the default SOURCE_TASK come from BF (BF::calculatorName(),
  //! BF::defaultSourceTask()).
  PairBFCalculator()
  :
  Base(BF::defaultSourceTask()),
  _species(),
  _bfHistograms()
  {
  this->appendClassName(BF::calculatorName());
  this->setMinimumReportLevel(Object::kInfo);
  this->setName(BF::calculatorName());
  this->setTitle(BF::calculatorName());
  }

  //! @brief Copy constructor (copies the BF folders). @param source task to copy
  PairBFCalculator(const PairBFCalculator & source)
  :
  Base(source),
  _species(source._species),
  _bfHistograms()
  {
  copyBfHistograms(source);
  }

  //! @brief Copy assignment. @param rhs task to copy @return this task
  PairBFCalculator & operator=(const PairBFCalculator & rhs)
  {
  if (this!=&rhs)
    {
    Base::operator=(rhs);
    _species = rhs._species;
    copyBfHistograms(rhs);
    }
  return *this;
  }

  //! @brief Destructor: deletes the BF folders.
  virtual ~PairBFCalculator() { deleteBfHistograms(); }

  //! @brief Copy of this task. @return new task, owned by the caller
  virtual Task * clone() override { return new PairBFCalculator(*this); }

  //! @brief Add the default configuration: that of CalculatorHistos (SOURCE_TASK, folders imported), the histogram
  //! settings of the pair analysis, and SPECIES.
  virtual void setDefaultConfiguration() override
  {
  Base::setDefaultConfiguration();
  const String & taskName = this->name();
  Configuration & configuration = this->_configuration;
  PAIR::addDefaultConfiguration(configuration,taskName);
  configuration.addProperty(createKey(taskName,"SPECIES"),String(""));
  }

  //!
  //! @brief Configure the task (and SOURCE_TASK, see CalculatorHistos), and the species from SPECIES and the particle
  //! filters.
  //! @param configuration configuration
  //! @throws ConfigurationException if a species names an unknown filter, or SPECIES is empty and the number of
  //! particle filters is odd
  //!
  virtual void configure(const Configuration & configuration) override
  {
  Base::configure(configuration);
  const String & taskName = this->name();
  String speciesList = configuration.valueString(createKey(taskName,"SPECIES"),true,"");
  if (speciesList=="NONE") speciesList = "";
  if (this->reportInfo(__FUNCTION__)) printValue(createKey(taskName,"SPECIES"),speciesList);
  _species.clear();
  const std::vector<String> & filters = this->_particleFilterNames;
  const unsigned int nPF = filters.size();
  std::vector<std::pair<String,String>> names = parseSpecies(speciesList);
  if (names.empty())
    {
    if (nPF%2!=0)
      throw ConfigurationException(createKey(taskName,"SPECIES"),"empty, and the number of particle filters is odd: name the species",__FUNCTION__);
    for (unsigned int k=0; k<nPF/2; k++) _species.push_back(std::make_pair(k,k+nPF/2));
    }
  else
    {
    auto indexOf = [&](const String & filterName) -> unsigned int
      {
      for (unsigned int k=0; k<nPF; k++) if (filters[k]==filterName) return k;
      throw ConfigurationException(createKey(taskName,"SPECIES"),String("no particle filter named ")+filterName,__FUNCTION__);
      };
    for (const auto & pair : names) _species.push_back(std::make_pair(indexOf(pair.first),indexOf(pair.second)));
    }
  }

  //!
  //! @brief Compute the derived histograms of every folder, then the balance functions of every event filter and
  //! ordered pair of species. @throws Exception if there are fewer folders than combinations of filters
  //!
  virtual void execute() override
  {
  if (this->reportDebug(__FUNCTION__)) { printCR(); }
  this->_taskExecuted.increment();
  const unsigned int nEF   = this->nEventFilters();
  const unsigned int nPF   = this->nParticleFilters();
  const unsigned int nPFsq = nPF*nPF;
  const unsigned int nSp   = _species.size();
  std::vector<PAIR*>                 & pairFolders   = this->histograms();
  std::vector<ParticleSingleHistos*> & singleFolders = this->singleHistograms();
  if (pairFolders.size()<this->nCombinations())
    throw Exception("Fewer pair folders than combinations of filters",__FUNCTION__);
  if (singleFolders.size()<nEF*nPF)
    throw Exception("Fewer single folders than nEventFilters()*nParticleFilters()",__FUNCTION__);

  // derived histograms of all the folders, as the derived calculator computes them
  calculatePairDerived(pairFolders,singleFolders,nPF);

  // balance functions of every event filter and ordered pair of species
  deleteBfHistograms();
  auto pairOf   = [&](unsigned int e, unsigned int i, unsigned int j) -> const PAIR & { return *pairFolders[e*nPFsq+i*nPF+j]; };
  auto singleOf = [&](unsigned int e, unsigned int i) -> const ParticleSingleHistos & { return *singleFolders[e*nPF+i]; };
  for (unsigned int e=0; e<nEF; e++)
    for (unsigned int s1=0; s1<nSp; s1++)
      for (unsigned int s2=0; s2<nSp; s2++)
        {
        unsigned int a = _species[s1].first, aB = _species[s1].second;
        unsigned int b = _species[s2].first, bB = _species[s2].second;
        BF * folder = new BF(bfFolderName(e,s1,s2));
        folder->setParentTask(this);
        _bfHistograms.push_back(folder);
        folder->createFrom(pairOf(e,a,b));
        folder->calculate(pairOf(e,a,b), pairOf(e,a,bB), pairOf(e,aB,b), pairOf(e,aB,bB),
                          singleOf(e,a), singleOf(e,aB), singleOf(e,b), singleOf(e,bB));
        }
  }

  //!
  //! @brief Add to a folder the empty folders the task writes (see AnalysisTask::makeEmptyHistograms()): those of the
  //! analyzer with their derived histograms, and the BF folders. @param top folder of the job
  //!
  void makeEmptyHistograms(Folder & top) override
  {
  Base::makeEmptyHistograms(top);
  const unsigned int nEF = this->nEventFilters(), nPF = this->nParticleFilters(), nSp = _species.size();
  std::vector<PAIR*> & pairFolders = this->histograms();
  if (pairFolders.size()<this->nCombinations()) return;
  Folder * folder = top.folder(this->histogramTaskName());
  if (!folder) folder = top.addFolder(this->histogramTaskName());
  for (unsigned int e=0; e<nEF; e++)
    for (unsigned int s1=0; s1<nSp; s1++)
      for (unsigned int s2=0; s2<nSp; s2++)
        {
        BF bf(bfFolderName(e,s1,s2));
        bf.setParentTask(this);
        bf.createFrom(*pairFolders[e*nPF*nPF + _species[s1].first*nPF + _species[s2].first]);
        Folder * target = folder->folder(bf.name());
        if (!target) target = folder->addFolder(bf.name());
        bf.saveTo(*target);
        }
  }

  //! @brief Delete the BF folders, and clear the task.
  virtual void clear() override
  {
  deleteBfHistograms();
  Base::clear();
  }

  //! @brief Write the folders of the analyzer and the BF folders. @param top folder of the job
  virtual void saveHistograms(Folder & top) override
  {
  Base::saveHistograms(top);
  if (_bfHistograms.empty()) return;
  Folder * folder = top.folder(this->histogramTaskName());
  if (!folder) folder = top.addFolder(this->histogramTaskName());
  for (const BF * bf : _bfHistograms)
    {
    Folder * target = folder->folder(bf->name());
    if (!target) target = folder->addFolder(bf->name());
    bf->saveTo(*target);
    }
  }

  //! @brief Species: (particle, antiparticle) indices of particle filters. @return species
  const std::vector<std::pair<unsigned int,unsigned int>> & species() const { return _species; }
  //! @brief BF folders, at index (e*nSpecies + s1)*nSpecies + s2. @return folders
  const std::vector<BF*> & bfHistograms() const { return _bfHistograms; }

  //! @brief Name of the BF folder of an event filter and two species (e.g. ALL_PiP_KP_BF).
  //! @param e event filter @param s1 first species @param s2 second species @return name
  String bfFolderName(unsigned int e, unsigned int s1, unsigned int s2) const
  {
  const std::vector<String> & events    = this->_eventFilterNames;
  const std::vector<String> & particles = this->_particleFilterNames;
  String folderName = e<events.size() ? events[e] : String("Event")+String(std::to_string(e).c_str());
  folderName += "_";
  folderName += particles[_species[s1].first];
  folderName += "_";
  folderName += particles[_species[s2].first];
  folderName += "_BF";
  return folderName;
  }

  //!
  //! @brief Parse a list of species: "PiP/PiM, KP/KM" (commas or spaces between species, '/' between particle and
  //! antiparticle). @param text list @return pairs of names @throws ConfigurationException if an entry has no '/'
  //!
  static std::vector<std::pair<String,String>> parseSpecies(const String & text)
  {
  std::vector<std::pair<String,String>> result;
  std::string list(text.data());
  for (char & c : list) if (c==',' || c==';' || c=='\t' || c=='\n') c = ' ';
  std::istringstream in(list);
  std::string entry;
  while (in >> entry)
    {
    size_t slash = entry.find('/');
    if (slash==std::string::npos || slash==0 || slash+1==entry.size())
      throw ConfigurationException("SPECIES",String("species '")+entry.c_str()+"' is not of the form particle/antiparticle",__FUNCTION__);
    result.push_back(std::make_pair(String(entry.substr(0,slash).c_str()),String(entry.substr(slash+1).c_str())));
    }
  return result;
  }

protected:

  //! @brief Delete the BF folders.
  void deleteBfHistograms()
  {
  for (BF * folder : _bfHistograms) delete folder;
  _bfHistograms.clear();
  }

  //! @brief Replace the BF folders by copies of those of another task. @param source task
  void copyBfHistograms(const PairBFCalculator & source)
  {
  deleteBfHistograms();
  for (const BF * folder : source._bfHistograms) _bfHistograms.push_back(new BF(*folder));
  }

  //! Species: (particle, antiparticle) indices of particle filters.
  std::vector<std::pair<unsigned int,unsigned int>> _species;
  //! BF folders (owned), at index (e*nSpecies + s1)*nSpecies + s2.
  std::vector<BF*> _bfHistograms;

};

} // namespace CAP

#endif /* CAP__PairBFCalculator */
