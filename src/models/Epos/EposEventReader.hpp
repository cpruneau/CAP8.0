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

namespace CAP
{

//!
//! @brief Task reading EPOS events from ROOT trees (event number, multiplicity, impact parameter, PDG codes and
//! momenta) and copying their particles into the current CAP event.
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
  //! @brief Add the default configuration of the task.
  void setDefaultConfiguration();
  //! @brief Configure the task. @param configuration configuration
  void configure(const Configuration & configuration);
  //! @brief Initialize the task.
  void initialize();
  //! @brief Execute the task (process the current event).
  void execute();

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
  ClassDef(EposEventReader,0)
};

}


#endif /* CAP__EposEventReader */
