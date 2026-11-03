/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__EposEventReader
#define CAP__EposEventReader
#include "RootChainManager.hpp"
#include "EventProcessor.hpp"
#include "Event.hpp"
#include "EventFilter.hpp"
#include "EventEfficiency.hpp"
#include "EventSmearer.hpp"
#include "Particle.hpp"
#include "ParticleFilter.hpp"
#include "ParticleEfficiency.hpp"
#include "ParticleSmearer.hpp"
#include <map>

namespace CAP
{

//!
//! @brief Task reading EPOS events from ROOT trees (event number, multiplicity, impact parameter, PDG codes and
//! momenta) and copying their particles into the current CAP event.
//!
//! Keys: \<name\>:DataImportPath (default $CAP_DATA_IMPORT_PATH), DataImportFile (a file name or a pattern, e.g.
//! "epos_*.root", relative to DataImportPath unless absolute; the files are read as one chain), TreeName (name of the
//! tree). The tree holds, per event, the branches Events (int), Mult (int), Impact (float), and the arrays of Mult
//! values PID (int, PDG code), Px, Py, Pz, E (float, GeV). The energy of a particle is computed from its momentum and
//! the mass of its type in the particle database; particles whose PDG code is not in the database are skipped and
//! counted (reported by finalize()). A particle is added to the event if it passes the first particle filter of the
//! task, or always if the task has no particle filter.
//!
class EposEventReader
:
public EventProcessor,
public RootChainManager
{
public:

  //! @brief Default constructor.
  EposEventReader();
  //! @brief Destructor.
  virtual ~EposEventReader() {}
  //! @brief Add the default configuration of the task (input files and tree name).
  virtual void setDefaultConfiguration();
  //! @brief Open the input files and map the branches of the tree.
  virtual void initialize();
  //! @brief Read the next event and add its particles to the current event.
  //! @throws EndOfDataException at the end of the input
  virtual void execute();
  //! @brief Report the PDG codes skipped (not in the particle database), and finalize the task.
  virtual void finalize();

protected:

  //! @brief Particle type of a PDG code, or null if it is not in the particle database (counted as skipped).
  //! @param pdg PDG code @return type, or null
  ParticleType * particleType(int pdg);

protected:

  //! Maximum number of particles per event.
  static const int arraySize = 40000;

   // Declaration of leaf types
   //! Event number.
   Int_t           events;
   //! Number of particles.
   Int_t           nParticles;
   //! Impact parameter.
   Float_t         impact;
   //! PDG codes.
   Int_t           pid[arraySize];   //[Mult]
   //! Momentum components and energy of the particles.
   Float_t         px[arraySize];   //[Mult]
   Float_t         py[arraySize];   //[Mult]
   Float_t         pz[arraySize];   //[Mult]
   Float_t         e[arraySize];   //[Mult]

   // List of branches
   //! @name Tree branches
   //! @{
   TBranch        *b_Events;   //!
   TBranch        *b_Mult;   //!
   TBranch        *b_Impact;   //!
   TBranch        *b_PID;   //!
   TBranch        *b_Px;   //!
   TBranch        *b_Py;   //!
   TBranch        *b_Pz;   //!
   TBranch        *b_E;   //!
 
  //! @}
  //! Input file(s) (DataImportPath/DataImportFile).
  String          _inputFiles;
  //! Particle types of the PDG codes met (null: not in the particle database).
  std::map<int,ParticleType*> _typeOfCode;
  //! Number of particles skipped, per PDG code not in the particle database.
  std::map<int,long> _skippedCodes;

};

}


#endif /* CAP__EposEventReader */
