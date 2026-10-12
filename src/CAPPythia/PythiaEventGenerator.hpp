/* **********************************************************************
 * Copyright (C) 2019-2024, Claude Pruneau, Victor Gonzalez   
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__PythiaEventGenerator
#define CAP__PythiaEventGenerator
#include "Pythia.h"
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
//! SetSeed, SeedValue, UseQCDCR, UseRopes, UseShoving, Print:Banner, ...; which particles are saved is set by
//! saveFinalOnly, savePhotons, saveNeutrinos, saveQuarks, saveGaugeBosons.
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
  //! @brief Destructor.
  virtual ~PythiaEventGenerator() {}
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

  ClassDef(PythiaEventGenerator,0)
};

}

#endif /* CAP__PythiaEventGenerator */
