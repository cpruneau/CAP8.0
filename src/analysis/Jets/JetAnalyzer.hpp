/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file JetAnalyzer.hpp
//! @brief JetAnalyzer: jets found with FastJet for several radii in one run, selected by jet filters, and their
//! per-event, single-jet and constituent histograms.
#ifndef CAP__JetAnalyzer
#define CAP__JetAnalyzer
#include <vector>
#include "EventProcessor.hpp"
#include "JetFilter.hpp"
#include "JetHistos.hpp"

namespace CAP
{

//!
//! @brief Analysis of jets: the particles accepted by each particle filter are clustered into jets (JetClustering, once
//! per event, particle filter and radius, shared with the other jet tasks) for each radius of the list JET:R; the jets
//! accepted by each jet filter (JET_FILTER: kinematic ranges and rank in the event) fill a folder of JetHistos.
//!
//! Folders: one per combination of an event filter e, a particle filter p, a radius r and a jet filter j, at index
//! ((e*nP + p)*nR + r)*nJ + j, named \<event filter\>_\<particle filter\>_\<R label\>_\<jet filter\> (R label: R0p4 for
//! 0.4, see JetSettings::radiusLabel()), in the folder of the task in the output file of the job.
//!
//! Settings (keys \<task\>:JET:\<name\>): R (list of radii, e.g. "0.2, 0.4, 0.6"), ALGORITHM (ANTIKT, KT or CA),
//! RECOMBINATION (E or PT), PT_MIN (minimum jet pT, GeV/c), PARTICLE_ETA_MAX (pseudorapidity range of the particles:
//! the jets kept have |eta| < PARTICLE_ETA_MAX - R; the rank of a jet is counted among those). The jet filters: the keys
//! of a list of managed objects (\<task\>:JET_FILTER:N, JET_FILTER_\<k\>:NAME, ...), made by JetFilterCreator. The
//! binning: see JetHistos.
//!
//! At the end of the run (JetHistos::normalize()), the per-event histograms (number of jets, pT sum) are divided by the
//! number of events accepted by the event filter, and the per-jet and per-constituent histograms by the number of jets
//! of their folder (event filter, particle filter, radius and jet filter); the profiles are averages per jet.
//!
class JetAnalyzer : public EventProcessor
{
public:

  //! @brief Default constructor.
  JetAnalyzer();
  //! @brief Copy constructor. @param source task
  JetAnalyzer(const JetAnalyzer & source);
  //! @brief Copy assignment. @param rhs task @return this task
  JetAnalyzer & operator=(const JetAnalyzer & rhs);
  //! @brief Destructor (deletes the folders).
  virtual ~JetAnalyzer();
  //! @brief Copy of the task. @return new task
  virtual Task * clone() override { return new JetAnalyzer(*this); }
  //! @brief Add the default configuration: jet finder (JET), jet filters (JET_FILTER), binning (HISTOGRAM).
  virtual void setDefaultConfiguration() override;
  //! @brief Configure the task, the jet filters and the folders. @param configuration configuration
  //! @throws ConfigurationException if a setting is invalid
  virtual void configure(const Configuration & configuration) override;
  //! @brief Initialize the task and create the histograms.
  virtual void initialize() override;
  //! @brief Fill the histograms with the jets of the current event.
  virtual void execute() override;
  //! @brief Scale the histograms per event, then finalize.
  virtual void finalize() override;
  //! @brief Normalize the folders: per event (factor of their event filter) and per jet. @param factors factors
  virtual void scaleHistograms(const std::vector<double> & factors) override;
  //! @brief Write the folders to the output folder of the job. @param top folder of the job
  virtual void saveHistograms(Folder & top) override;
  //! @brief Add the empty folders to a folder (see AnalysisTask::makeEmptyHistograms()). @param top folder
  virtual void makeEmptyHistograms(Folder & top) override;

  //! @name Settings and folders
  //! @{
  //! @brief Radii. @return radii
  const std::vector<double> & radii() const { return _radii; }
  //! @brief Settings of the jet finder of a radius. @param r index of the radius @return settings
  JetSettings settings(unsigned int r) const;
  //! @brief Number of jet filters. @return number
  unsigned int nJetFilters() const { return _managedJetFilters.size(); }
  //! @brief Jet filters. @return filters
  const std::vector<JetFilter*> & jetFilters() const { return _managedJetFilters.getObjects(); }
  //! @brief Index of the folder of a combination. @param e event filter @param p particle filter @param r radius
  //! @param j jet filter @return index
  unsigned int folderIndex(unsigned int e, unsigned int p, unsigned int r, unsigned int j) const
  { return ((e*nParticleFilters()+p)*_radii.size()+r)*nJetFilters()+j; }
  //! @brief Name of the folder of a combination (recorded at configuration). @param k index @return name
  String folderName(unsigned int k) const;
  //! @brief Folders. @return folders
  const std::vector<JetHistos*> & folders() const { return _folders; }
  //! @}

protected:

  //! @brief Delete the folders.
  void deleteFolders();

  ManagedObjects<JetFilter> _managedJetFilters;   //!< jet filters
  std::vector<double> _radii;                     //!< radii
  JetSettings _settings;                          //!< settings (the radius set per radius)
  std::vector<JetHistos*> _folders;               //!< folders
  std::vector<String> _folderNames;               //!< names of the folders (the filters are released by finalize())
};

} // namespace CAP

#endif /* CAP__JetAnalyzer */
