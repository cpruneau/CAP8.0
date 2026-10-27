/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__PythiaEventGenerator
#define CAP__PythiaEventGenerator
#include "Pythia.h"
#include <map>
#include "EventProcessor.hpp"
#include "Event.hpp"
#include "EventFilter.hpp"
#include "EventEfficiency.hpp"
#include "EventSmearer.hpp"
#include "Particle.hpp"
#include "ParticleFilter.hpp"
#include "ParticleEfficiency.hpp"
#include "ParticleSmearer.hpp"

namespace CAP
{

//!
//! @brief Event generator task running PYTHIA 8: each execute() generates one event and copies its particles into the
//! current CAP event.
//!
//! PYTHIA settings are given by keys \<name\>:Beams:idA, Beams:idB, Beams:frameType, Beams:eCM, Beams:eA, Beams:eB,
//! SetSeed, SeedValue, UseQCDCR, UseRopes, UseShoving (presets), Option0..Option29 (any PYTHIA setting string, applied
//! after the presets so that it overrides them), Print:Banner,
//! Print:Statistics and xmlInputPath (PYTHIA xml directory; default $PYTHIA8DATA, then
//! $CAP_PYTHIA8_PATH/share/Pythia8/xmldoc, then that of the Pythia8 CAP was built with); which particles are saved is set by SaveFinalOnly, SavePhotons,
//! SaveNeutrinos, SaveQuarks, SaveGaugeBosons.
//!
class PythiaEventGenerator
:
public EventProcessor
{
public:

  //! @brief Default constructor.
  PythiaEventGenerator();
  //! @brief Copy constructor. @param task object to copy
  PythiaEventGenerator(const PythiaEventGenerator & task);
  //! @brief Destructor (deletes the generator if finalize() was not called).
  virtual ~PythiaEventGenerator() { delete pythia; }
  //! @brief Copy assignment. @param task object to copy @return this object
  PythiaEventGenerator & operator=(const PythiaEventGenerator & task);
  //! @brief Add the default configuration of the task.
  virtual void setDefaultConfiguration();
  //! @brief Initialize the task.
  virtual void initialize();
  //! @brief Execute the task (process the current event).
  virtual void execute();
  //! @brief Finalize the task.
  virtual void finalize();

protected:

  //! PYTHIA generator.
  Pythia8::Pythia * pythia;
  //! @brief Type of a PDG code in the particle database, nullptr if the database does not have it (cached per code;
  //! the codes missing are counted and reported by finalize()). @param pdg PDG code
  ParticleType * particleType(int pdg);
  //! @brief Report the PDG codes skipped because the particle database does not have them.
  void reportSkippedCodes();
  //! Types of the PDG codes met so far (nullptr: not in the particle database).
  std::map<int,ParticleType*> typeOfCode;
  //! Number of particles skipped, per PDG code missing from the particle database.
  std::map<int,long> skippedCodes;
  //! Whether only final-state particles are saved.
  bool saveFinalOnly;
  //! Whether photons are saved.
  bool savePhotons;
  //! Whether neutrinos are saved.
  bool saveNeutrinos;
  //! Whether quarks are saved.
  bool saveQuarks;
  //! Whether gauge bosons are saved.
  bool saveGaugeBosons;

};

}

#endif /* CAP__PythiaEventGenerator */
