/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file HerwigEventGenerator.hpp
//! @brief HerwigEventGenerator: a task running Herwig 7 in the CAP job, each of its events copied into the CAP event.
#ifndef CAP__HerwigEventGenerator
#define CAP__HerwigEventGenerator
#include <map>
#include "EventProcessor.hpp"

namespace CAP
{

class HerwigRun;
class ParticleType;

//!
//! @brief Event generator task running Herwig 7 in the job: each execute() generates one Herwig event (a ThePEG event)
//! and copies its final-state particles into the current CAP event.
//!
//! Herwig is configured as on the command line: a run file (.run), made by "Herwig read" from an input file (.in),
//! holds the event generator; "Herwig run" then generates its events. The task does both:
//!  - InputFile: an input file (.in). If it is given, the task reads it first ("Herwig read", in the directory of the
//!    file), unless the run file it saves (its saverun command) is there already and newer than the input file; the
//!    run file is then the one of the input file.
//!  - RunFile: the run file (.run), when no input file is given (relative paths: to the current directory); an input
//!    file (.in) given here is taken as InputFile.
//!  - Repository: the repository read by "Herwig read" (default HerwigDefaults.rpo, from Herwig's share directory).
//!  - SetupFile: a file of repository commands applied to the generator before the run ("Herwig run -x"), and
//!    Option0..Option19: more such commands (e.g. "set /Herwig/Generators/EventGenerator:EventHandler:
//!    LuminosityFunction:Energy 13600.0"), written with the setup file into \<name\>_setup.in in the current
//!    directory. Changing the beam energy or switching off decays (e.g. "set /Herwig/Particles/pi0:Stable Stable")
//!    thus needs no new run file.
//!  - SetSeed, SeedValue: the seed of the random numbers (Herwig run -s); without, that of the run file.
//!  - Tag: a tag appended to the names of the files Herwig writes (its .log and .out files, in the current directory).
//! Processes integrated by Matchbox need their grids first ("Herwig build" and "Herwig integrate" on the command line).
//!
//! Each event: the final-state particles (ThePEG::Event::getFinalState()), photons and neutrinos included unless
//! SavePhotons or SaveNeutrinos is false, whose PDG codes are in the particle database (the others are skipped and
//! reported by finalize()), with their momentum (GeV) and production vertex (mm, as for PYTHIA); a particle is added if
//! it passes the first particle filter of the task (or always if the task has no particle filter). Herwig's events
//! are unweighted by default; weighted events are counted and reported (CAP events have no weight).
//!
//! finalize() reports the cross section (Print:Statistics) and finalizes the generator (Herwig writes its .out file).
//! Herwig holds global state: one HerwigEventGenerator per job.
//!
//! Build: CMake option CAP_WITH_HERWIG, Herwig found as CAP_HERWIG_DIR (or ../Herwig/install, $CAP_HERWIG_PATH,
//! Herwig-config). On macOS, Herwig and ThePEG must be built with -fno-typed-cxx-new-delete and linked to libc++
//! (~/Developer/Herwig/build_herwig.sh does); this module is compiled the same way.
//!
class HerwigEventGenerator
:
public EventProcessor
{
public:

  //! @brief Default constructor.
  HerwigEventGenerator();
  //! @brief Copy constructor (the generator is not copied: the copy makes its own in initialize()).
  //! @param task object to copy
  HerwigEventGenerator(const HerwigEventGenerator & task);
  //! @brief Destructor (releases the generator if finalize() was not called).
  virtual ~HerwigEventGenerator();
  //! @brief Copy assignment. @param task object to copy @return this object
  HerwigEventGenerator & operator=(const HerwigEventGenerator & task);
  //! @brief Add the default configuration of the task.
  virtual void setDefaultConfiguration();
  //! @brief Read the input file if needed, and prepare the event generator.
  //! @throws Exception if Herwig cannot read the input or the run file
  virtual void initialize();
  //! @brief Generate one event and add its final-state particles to the current event.
  virtual void execute();
  //! @brief Report the cross section and the particles skipped, and finalize the generator.
  virtual void finalize();

protected:

  //! @brief Type of a PDG code in the particle database, nullptr if the database does not have it (cached; the codes
  //! missing are counted and reported by finalize()). @param pdg PDG code @return type
  ParticleType * particleType(int pdg);

  HerwigRun * herwig;                         //!< the Herwig event generator (ThePEG), made by initialize()
  std::map<int,ParticleType*> typeOfCode;     //!< types of the PDG codes met (nullptr: not in the database)
  std::map<int,long> skippedCodes;            //!< particles skipped, per PDG code missing from the database
  bool savePhotons;                           //!< whether photons are saved
  bool saveNeutrinos;                         //!< whether neutrinos are saved
  long nWeighted;                             //!< events whose weight is not 1
  long nFailed;                               //!< events Herwig could not generate
};

}

#endif /* CAP__HerwigEventGenerator */
