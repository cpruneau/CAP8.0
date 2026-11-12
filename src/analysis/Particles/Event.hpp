/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__Event
#define CAP__Event
#include <memory>
#include <vector>
#include "String.hpp"
#include "Exceptions.hpp"
#include "Particle.hpp"
#include "ManagedObject.hpp"
#include "Configuration.hpp"
#include "ParticleSelection.hpp"

namespace CAP
{

class Event;
class EventFilter;

//template <class Event> class Manager;

//!
//! @brief Event (real data or Monte Carlo): its particles, the two colliding projectiles, and global quantities.
//!
//! The particles are created by the particle factory (Particle::factory()) and referenced here.
//!
//! Selections: the event keeps, for each particle filter used by the tasks, the particles it accepts and their global
//! quantities (ParticleSelection), made at the first request in the event and shared by all the tasks (selection()); and
//! the results of the event filters (accepted()). A filter is reached by its slot (ParticleSelection::slot()). They are
//! made again after any change of the particles: reset(), clear() and addParticle() invalidate them, and the tasks
//! changing the particles in place (decays, rotations, shifts) call invalidateSelections().
//!
//! The global quantities indexed by a number k (mult(k), energy(k), ...; conditions MULT_k, ... of EventFilter) are
//! those of earlier versions, set by GlobalAnalyzer for its particle filter k: the selections replace them (conditions
//! on the quantities of a named particle filter, see EventFilter).
//!
class Event
:
public ManagedObject
{
  public:
  //! @brief Default constructor: no particles, index 0.
  Event();
  //! @brief Copy constructor (the selections are not copied: made again). @param source event
  Event(const Event & source);
  //! @brief Copy assignment (the selections are not copied: made again). @param source event @return this event
  Event & operator=(const Event & source);
  //! @brief Destructor (the particles are owned by the particle factory).
  virtual ~Event();
  //! @brief Remove the particles and set the event index and number to 0.
  virtual void clear();
  //! @brief Remove the particles, increment the event index and set the event number to 0 (next event).
  virtual void reset();
  //! @brief Add a particle. @param particle particle (owned by the particle factory)
  void addParticle(Particle * particle);

  //! @name Selections (shared by the tasks, see the class description)
  //! @{
  //! @brief Selection of a particle filter in this event (made if needed). @param slot slot of the filter
  //! (ParticleSelection::slot()) @return selection
  ParticleSelection & selection(unsigned int slot) const;
  //! @brief Whether an event filter accepts this event (evaluated once per event). @param filter filter
  //! @param slot its slot (ParticleSelection::slot(filter)) @return true if accepted
  bool accepted(const EventFilter & filter, unsigned int slot) const;
  //! @brief Forget the selections and the results of the event filters: to call after changing the particles in place.
  void invalidateSelections() { _epoch++; }
  //! @brief Number of changes of the particles (the selections are made again after each). @return number
  unsigned long epoch() const { return _epoch; }
  //! @}
  //! @brief Particles of the event. @return particles
  std::vector<Particle*> & particles() {  return _particles;}
  //! @brief Particles of the event. @return particles
  const std::vector<Particle*> & particles() const {  return _particles;}

  //!
  //! Return to the particle at position "index" in this event. A null pointer is returned is index exceeds the
  //!  number of particles in this event.
  //!
  Particle * particleAt(unsigned long index);

  //! @brief Print the particles.
  void printParticles() const;
  //! @brief Print the event.
  void print() const;


  //!
  //! Return the index of this event. The event index might correspond to the position of the event
  //! in the production or input stream.
  //!
  unsigned long eventIndex() const      { return _eventIndex;  }

   //!
  //! Return the event number of this event. The event number might correspond to a tag or index given
  //! to this event externally by some other  production or input stream.
  //!
  unsigned long eventNumber() const     { return _eventNumber; }

  //!
  //! Return the number of particles stored in this event.
  //!
  unsigned long  nParticles() const { return _particles.size();}
  //! @brief Number of particles (same as nParticles()). @return number of particles
  unsigned long  size() const { return _particles.size();}

  //! @brief First projectile (beam particle or nucleus). @return projectile
  Particle & projectileA() { return _projectileA; }
  //! @brief Second projectile (beam particle or nucleus). @return projectile
  Particle & projectileB() { return _projectileB; }


  //!
  //! Increment the index of this event.
  //!
  void incrementEventIndex()      { _eventIndex++;  }

  //!
  //! Set the index of this event.
  //!
  void setEventIndex(unsigned long index)  { _eventIndex  = index;}

  //! @brief Set the event number (tag given by the production or input stream). @param number event number
  void setEventNumber(unsigned long number)  { _eventNumber = number;}

  //! @name Global quantities, per particle selection (index)
  //! @{

  //! @brief Size the multiplicities and set them to 0. @param n number of selections
  void initializeMult(unsigned int n=5)    {  _multiplicity.assign(n,0.0); }
  //! @brief Size the energies and set them to 0. @param n number of selections
  void initializeEnergy(unsigned int n=5)  {  _energy.assign(n,0.0); }
  //! @brief Size the net charges and set them to 0. @param n number of selections
  void initializeCharge(unsigned int n=5)  {  _charge.assign(n,0.0); }
  //! @brief Size the net strangeness values and set them to 0. @param n number of selections
  void initializeStrangeness(unsigned int n=5)  {  _strangeness.assign(n,0.0); }
  //! @brief Size the net baryon numbers and set them to 0. @param n number of selections
  void initializeBaryon(unsigned int n=5)       {  _baryon.assign(n,0.0); }
  //! @brief Size the pT sums and set them to 0. @param n number of selections
  void initializePtSum(unsigned int n=5)        {  _ptSum.assign(n,0.0); }
  //! @brief Size the spherocity values and set them to 0. @param n number of selections
  void initializeSpherocity(unsigned int n=5)   {  _spherocity.assign(n,0.0); }

  //! @brief Multiplicity of a selection. @param index selection @return multiplicity
  double mult(unsigned int index)   const { return _multiplicity[index]; }
  //! @brief Energy of a selection. @param index selection @return energy
  double energy(unsigned int index) const { return _energy[index]; }
  //! @brief Net charge of a selection. @param index selection @return charge
  double charge(unsigned int index) const { return _charge[index]; }
  //! @brief Net strangeness of a selection. @param index selection @return strangeness
  double strangeness(unsigned int index) const { return _strangeness[index]; }
  //! @brief Net baryon number of a selection. @param index selection @return baryon number
  double baryon(unsigned int index)      const { return _baryon[index]; }
  //! @brief Sum of the pT of a selection. @param index selection @return pT sum
  double ptSum(unsigned int index)       const { return _ptSum[index]; }
  //! @brief Spherocity of a selection. @param index selection @return spherocity
  double spherocity(unsigned int index)  const { return _spherocity[index]; }

  //! @brief Set the multiplicity of a selection. @param index selection @param value value
  void setMult(unsigned int index, double value)        { _globalsVersion++; _multiplicity[index] = value; }
  //! @brief Set the energy of a selection. @param index selection @param value value
  void setEnergy(unsigned int index, double value)      { _globalsVersion++; _energy[index] = value; }
  //! @brief Set the net charge of a selection. @param index selection @param value value
  void setCharge(unsigned int index, double value)      { _globalsVersion++; _charge[index] = value; }
  //! @brief Set the net strangeness of a selection. @param index selection @param value value
  void setStrangeness(unsigned int index, double value) { _globalsVersion++; _strangeness[index] = value; }
  //! @brief Set the net baryon number of a selection. @param index selection @param value value
  void setBaryon(unsigned int index, double value)      { _globalsVersion++; _baryon[index] = value; }
  //! @brief Set the pT sum of a selection. @param index selection @param value value
  void setPtSum(unsigned int index, double value)       { _globalsVersion++; _ptSum[index] = value; }
  //! @brief Set the spherocity of a selection. @param index selection @param value value
  void setSpherocity(unsigned int index, double value)  { _globalsVersion++; _spherocity[index] = value; }

  //! @brief Set the multiplicities to 0.
  void resetMult()         {  for (auto& v :_multiplicity) v=0.0; }
  //! @brief Set the energies to 0.
  void resetEnergy()       {  for (auto& v :_energy) v=0.0; }
  //! @brief Set the net charges to 0.
  void resetCharge()       {  for (auto& v :_charge) v=0.0; }
  //! @brief Set the net strangeness values to 0.
  void resetStrangeness()  {  for (auto& v :_strangeness) v=0.0; }
  //! @brief Set the net baryon numbers to 0.
  void resetBaryon()       {  for (auto& v :_baryon) v=0.0; }
  //! @brief Set the pT sums to 0.
  void resetPtSum()        {  for (auto& v :_ptSum)  v=0.0; }
  //! @brief Set the spherocity values to 0.
  void resetpherocity()    {  for (auto& v :_spherocity) v=0.0; }

//! @}
protected:

  //! Index of the event (e.g., position in the production or input stream).
  unsigned long _eventIndex;
  //! Number given to the event by an external producer.
  unsigned long _eventNumber;
  //! First projectile.
  Particle _projectileA;
  //! Second projectile.
  Particle _projectileB;
  //! Particles (owned by the particle factory).
  std::vector<Particle*> _particles;

  //! Multiplicity of each particle selection.
  std::vector<double> _multiplicity;
  //! Energy of each particle selection.
  std::vector<double> _energy;
  //! Net charge of each particle selection.
  std::vector<double> _charge;
  //! Net strangeness of each particle selection.
  std::vector<double> _strangeness;
  //! Net baryon number of each particle selection.
  std::vector<double> _baryon;
  //! pT sum of each particle selection.
  std::vector<double> _ptSum;
  //! Spherocity of each particle selection.
  std::vector<double> _spherocity;

  //! Number of changes of the particles: the selections and filter results of earlier values are made again.
  unsigned long _epoch;
  //! Number of changes of the global quantities indexed by number (the event filters reading them are evaluated again).
  unsigned long _globalsVersion;
  //! Selections of the particle filters, by slot (pointers: a selection stays in place when more slots are added).
  mutable std::vector<std::unique_ptr<ParticleSelection>> _selections;
  //! Epoch of each selection.
  mutable std::vector<unsigned long> _selectionEpochs;
  //! Results of the event filters, by slot, with the epoch and version of the global quantities they were made with.
  mutable std::vector<char> _filterResults;
  mutable std::vector<unsigned long> _filterEpochs;      //!< epoch of each result
  mutable std::vector<unsigned long> _filterVersions;    //!< version of the global quantities of each result

};

} //namespace CAP

#endif /* Event */
