/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
#ifndef CAP__ParticleSelection
#define CAP__ParticleSelection
#include <vector>

namespace CAP
{
class Particle;
class ParticleFilter;
class EventFilter;

//!
//! @brief The particles of an event accepted by one particle filter, with the quantities computed from them: their pT
//! moments and the global quantities of the selection (multiplicity, energy, net charge, net strangeness, net baryon
//! number, pT sum, spherocity).
//!
//! The selections of an event are made once per event and shared by all the tasks using the same filter (see
//! Event::selection()): a filter is filtered once, however many analyzers use it. Each filter has a slot, the index of
//! its selection in the events, given once by slot() (at the initialization of the tasks, see
//! EventProcessor::initialize()); a task maps its own filters to their slots and then reaches a selection by two array
//! lookups. The filters are identified by their object: the filters shared by tasks (created by FilterCreator, taken
//! from the store of the ManagedObjects) share a slot; a filter made by a task for itself has a slot of its own.
//!
//! The spherocity is computed on demand (spherocity()): S0 = min over the axes n of (sum |pT x n| / sum pT)^2, the axes
//! every degree in azimuth, as SpherocityAnalyzer with nSteps = 360 and no normalization factor.
//!
class ParticleSelection
{
public:

  //! @brief Empty selection of a filter. @param filter filter @param slot slot
  ParticleSelection(const ParticleFilter * filter=nullptr, unsigned int slot=0);

  //! @brief Make the selection: the particles accepted by the filter and their quantities.
  //! @param particles particles of the event @param ptOrder highest order of the pT moments
  void fill(const std::vector<Particle*> & particles, unsigned int ptOrder);
  //! @brief Forget the particles (the capacity is kept).
  void clear();

  //! @brief Filter of the selection. @return filter
  const ParticleFilter * filter() const { return _filter; }
  //! @brief Slot of the selection. @return slot
  unsigned int slot() const { return _slot; }
  //! @brief Accepted particles. @return particles
  const std::vector<Particle*> & particles() const { return _particles; }
  //! @brief Accepted particles (for the tasks reading them as before; not to be changed). @return particles
  std::vector<Particle*> & particles() { return _particles; }
  //! @brief Whether no particle is accepted. @return true if empty
  bool empty() const { return _particles.empty(); }
  //! @brief pT moments: sum of pT^k of the accepted particles, k = 0..order. @return moments
  std::vector<double> & ptMoments() { return _moments; }
  //! @brief pT moments. @return moments
  const std::vector<double> & ptMoments() const { return _moments; }

  //! @brief Number of accepted particles. @return multiplicity
  double multiplicity() const { return double(_particles.size()); }
  //! @brief Sum of the energies. @return energy
  double energy() const { return _energy; }
  //! @brief Net charge. @return charge
  double charge() const { return _charge; }
  //! @brief Net strangeness. @return strangeness
  double strangeness() const { return _strangeness; }
  //! @brief Net baryon number. @return baryon number
  double baryon() const { return _baryon; }
  //! @brief Sum of the pT. @return pT sum
  double ptSum() const { return _ptSum; }
  //! @brief Spherocity S0 (computed at the first call for the event). @return spherocity (0 without particles)
  double spherocity() const;

  //! @name Slots (shared by all the events)
  //! @{
  //! @brief Slot of a particle filter (given at the first call). @param filter filter
  //! @param ptOrder highest order of the pT moments the caller needs (the selections hold those of all callers)
  //! @return slot
  static unsigned int slot(const ParticleFilter * filter, unsigned int ptOrder=0);
  //! @brief Slot of an event filter, for the results of the event filters kept by the events. @param filter filter
  //! @return slot
  static unsigned int slot(const EventFilter * filter);
  //! @brief Number of particle-filter slots. @return number
  static unsigned int nSlots();
  //! @brief Number of event-filter slots. @return number
  static unsigned int nEventSlots();
  //! @brief Filter of a slot. @param slot slot @return filter
  static const ParticleFilter * filterAt(unsigned int slot);
  //! @brief Highest order of the pT moments asked for. @return order
  static unsigned int ptOrder();
  //! @brief Forget all the slots (tests; the events must be cleared too).
  static void resetSlots();
  //! @}

private:

  const ParticleFilter * _filter;       //!< filter
  unsigned int _slot;                   //!< slot
  std::vector<Particle*> _particles;    //!< accepted particles
  std::vector<double> _moments;         //!< pT moments
  double _energy;                       //!< energy
  double _charge;                       //!< net charge
  double _strangeness;                  //!< net strangeness
  double _baryon;                       //!< net baryon number
  double _ptSum;                        //!< pT sum
  mutable double _spherocity;           //!< spherocity (once computed)
  mutable bool _hasSpherocity;          //!< whether computed for this event
};

} // namespace CAP

#endif /* CAP__ParticleSelection */
