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
#include "Aliases.hpp"
#include "Exceptions.hpp"
#include "Particle.hpp"
#include "ManagedObject.hpp"
#include "Configuration.hpp"
#include "ClassDef.hpp"

namespace CAP
{

class Event;

//template <class Event> class Manager;

//!
//! @brief Event (real data or Monte Carlo): its particles, the two colliding projectiles, and global quantities.
//!
//! The particles are created by the particle factory (Particle::factory()) and referenced here. The global quantities
//! (multiplicity, energy, charge, strangeness, baryon number, pT sum, spherocity) are computed by the analysis tasks
//! for several particle selections (index k): they must be sized with initializeMult(n), ... before use, and are
//! used, e.g., by EventFilter.
//!
class Event
:
public ManagedObject
{
  public:
  //! @brief Default constructor: no particles, index 0.
  Event();
  //! @brief Destructor (the particles are owned by the particle factory).
  virtual ~Event();
  //! @brief Remove the particles and set the event index and number to 0.
  virtual void clear();
  //! @brief Remove the particles, increment the event index and set the event number to 0 (next event).
  virtual void reset();
  //! @brief Add a particle. @param particle particle (owned by the particle factory)
  void addParticle(Particle * particle);
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
  void setMult(unsigned int index, double value)        { _multiplicity[index] = value; }
  //! @brief Set the energy of a selection. @param index selection @param value value
  void setEnergy(unsigned int index, double value)      { _energy[index] = value; }
  //! @brief Set the net charge of a selection. @param index selection @param value value
  void setCharge(unsigned int index, double value)      { _charge[index] = value; }
  //! @brief Set the net strangeness of a selection. @param index selection @param value value
  void setStrangeness(unsigned int index, double value) { _strangeness[index] = value; }
  //! @brief Set the net baryon number of a selection. @param index selection @param value value
  void setBaryon(unsigned int index, double value)      { _baryon[index] = value; }
  //! @brief Set the pT sum of a selection. @param index selection @param value value
  void setPtSum(unsigned int index, double value)       { _ptSum[index] = value; }
  //! @brief Set the spherocity of a selection. @param index selection @param value value
  void setSpherocity(unsigned int index, double value)  { _spherocity[index] = value; }

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

  ClassDef(Event,0)
};

} //namespace CAP

#endif /* Event */
