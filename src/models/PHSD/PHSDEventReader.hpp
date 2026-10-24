/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__PHSDEventReader
#define CAP__PHSDEventReader
#include "EventProcessor.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Task reading PHSD (Parton-Hadron-String Dynamics) events from ASCII output files and copying their particles
//! into the current CAP event.
//! @warning The member "double std::sqrtS;" does not compile; PHSD is built only when enabled.
//!
class PHSDEventReader
:
public EventProcessor
{
public:
  //! @brief Default constructor.
  PHSDEventReader();
  //! @brief Copy constructor. @param task object to copy
  PHSDEventReader(const PHSDEventReader & task);
  //! @brief Destructor.
  virtual ~PHSDEventReader() {}
  //! @brief Copy assignment. @param task object to copy @return this object
  PHSDEventReader & operator=(const PHSDEventReader & task);
  //! @brief Add the default configuration of the task.
  virtual void setDefaultConfiguration();
  //! @brief Initialize the task.
  virtual void initialize();
  //! @brief Execute the task (process the current event).
  virtual void execute();

protected:

  //! Number of particles of the current event.
  int    nParticles;
  //! Run index.
  int    runIndex;
  //! Event index.
  int    eventIndex;
  //! Impact parameter.
  double impact;
  //! Impact parameter sampling method.
  int    impactSamplingMethod;
  //! PDG code of the target.
  int    pdgCodeTarget;
  //! PDG code of the projectile.
  int    pdgCodeProjectile;
  double std::sqrtS;
  //! Number of participants.
  int    nPart;
  //! Event-plane angles and eccentricities of orders 2 to 5.
  double psi2, epsilon2, psi3, epsilon3, psi4, epsilon4, psi5, epsilon5;
  
  //! Position of the current particle.
  double x, y, z, t;
  double cosPhi;
  double sinPhi;
  //! Random rotation angle of the event.
  double eventPhi;

  //! PDG code of the current particle.
  int    pdgCode;
  //! Charge of the current particle.
  int    charge;
  //! Momentum of the current particle (px, py, pz, e).
  double px;
  double py;
  double pz;
  double e;
  //! PHSD code of the current particle.
  int    phsdCode;
  //! Code of the parent of the current particle.
  int    parentCode;

  //! Whether an input file is open.
  bool  fileOpened;
  //! Current input file.
  std::ifstream  *inputFile;
  //! Index of the current input file.
  unsigned int    currentFileIndex;
  //! Input directory.
  String inputPath;
  //! Input files.
  std::vector<String> inputFileNames;
  //! Current input file name.
  String inputFileName;


  ClassDef(PHSDEventReader,0)
};

}


#endif /* CAP__PHSDEventReader */
