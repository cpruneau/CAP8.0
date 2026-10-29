/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
#ifndef CAP__ParticlePairEventFilterHistos
#define CAP__ParticlePairEventFilterHistos
#include "HistogramFolder.hpp"
#include "ParticleSingleHistos.hpp"
#include "ParticlePairHistos.hpp"

namespace CAP
{

//!
//! @brief All the histograms of a pair analysis for one event filter: those of its two particle filters p1 and p2
//! (ParticleSingleHistos) and those of the pairs 11, 12, 21 and 22 (ParticlePairHistos), base and derived, in one
//! folder of the files of the jobs.
//!
//! The histogram names begin with the names of the particle filters: \<p1\>_n1_y, \<p2\>_n1_y for the particles,
//! \<p1\>_\<p1\>_n2_yY, \<p1\>_\<p2\>_n2_yY, \<p2\>_\<p1\>_n2_yY, \<p2\>_\<p2\>_n2_yY for the pairs. The particle filters are
//! those of the task (keys \<taskName\>:PARTICLE_FILTER_0:NAME and PARTICLE_FILTER_1:NAME), the binning and fill
//! options those of its HISTOGRAM group, for the particles and the pairs.
//!
class ParticlePairEventFilterHistos : public HistogramFolder
{
public:

  //! @brief Default constructor.
  ParticlePairEventFilterHistos();
  //! @brief Copy constructor. @param source folder to copy
  ParticlePairEventFilterHistos(const ParticlePairEventFilterHistos & source);
  //! @brief Copy assignment. @param rhs folder to copy @return this folder
  ParticlePairEventFilterHistos & operator=(const ParticlePairEventFilterHistos & rhs);
  //! @brief Destructor.
  virtual ~ParticlePairEventFilterHistos() {}
  //! @brief Polymorphic copy. @return copy, owned by the caller
  virtual HistogramFolder * clone() const { return new ParticlePairEventFilterHistos(*this); }

  //!
  //! @brief Read the names of the two particle filters and configure the groups of the particles and of the pairs.
  //! @param taskName name of the task @param objectType object type @param configuration configuration @param index index
  //! @throws Exception if the two particle filters have the same name
  //!
  virtual void configure(const String & taskName, const String & objectType,
                         const Configuration & configuration, unsigned int index);
  //! @brief Create the base histograms of the particles and of the pairs.
  virtual void create();
  //! @brief Create the derived histograms of the particles and of the pairs (empty).
  virtual void createDerived();
  //! @brief Whether the derived histograms exist (created or loaded). @return true if they do
  bool hasDerived() const { return single1.h_n1_phi!=nullptr; }
  //! @brief Compute the derived histograms: those of the particles, then those of the four pairs.
  virtual void calculateDerived();
  //! @brief Load the histograms (base, and derived if present) from a folder of a file. @param file folder
  virtual void loadFrom(const Folder & file);
  //! @brief Copy all the histograms into a folder of a file. @param file folder
  virtual void saveTo(Folder & file) const;
  //! @brief Multiply all the histograms by a factor (e.g. 1/number of events). @param factor factor
  void scaleHistograms(double factor);
  //! @brief Reset all the histograms (zero content).
  virtual void reset();

  //! @brief Name of particle filter 1. @return name
  const String & particleName1() const { return _name1; }
  //! @brief Name of particle filter 2. @return name
  const String & particleName2() const { return _name2; }

  ParticleSingleHistos single1;  ///< histograms of particle filter 1
  ParticleSingleHistos single2;  ///< histograms of particle filter 2
  ParticlePairHistos   pair11;   ///< histograms of the pairs (1, 1)
  ParticlePairHistos   pair12;   ///< histograms of the pairs (1, 2)
  ParticlePairHistos   pair21;   ///< histograms of the pairs (2, 1)
  ParticlePairHistos   pair22;   ///< histograms of the pairs (2, 2)

protected:

  String _name1;  ///< name of particle filter 1
  String _name2;  ///< name of particle filter 2

  //! @brief The six groups, in a fixed order (single1, single2, pair11, pair12, pair21, pair22). @return groups
  std::vector<HistogramFolder*> groups();
  //! @brief The six groups (const). @return groups
  std::vector<const HistogramFolder*> groups() const;

};

} // namespace CAP

#endif /* CAP__ParticlePairEventFilterHistos */
