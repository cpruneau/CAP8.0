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
#include <fstream>
#include <map>
#include <memory>
#include <vector>
#include "EventProcessor.hpp"

namespace CAP
{

//!
//! @brief Task reading PHSD (Parton-Hadron-String Dynamics) events from its ASCII output files and copying their
//! particles into the current CAP event.
//!
//! Keys: \<name\>:DataImportPath (default $CAP_DATA_IMPORT_PATH), a file or a directory, and DataImportFile, a
//! pattern of file names (default "*.dat") searched in the directory and in its sub-directories down to
//! DataImportDepth levels (default 2: the directory and its sub-directories). The files are read in the order of
//! their paths. Each event of a file is
//! @code
//!   nParticles runIndex eventIndex impactParameter impactSamplingMethod pdgTarget pdgProjectile sqrtS
//!   nParticipants psi2 epsilon2 psi3 epsilon3 psi4 epsilon4 psi5 epsilon5
//!   pdgCode charge px py pz e phsdCode parentCode        (nParticles lines)
//! @endcode
//! with momenta and energies in GeV. The particles keep the energy of the file (PHSD particles may be off their mass
//! shell) and are placed at the origin. Particles whose PDG code is not in the particle database are skipped and
//! counted (reported by finalize()). A particle is added to the event if it passes the first particle filter of the
//! task, or always if the task has no particle filter. The event loop ends (EndOfDataException) when all the files
//! have been read.
//!
class PHSDEventReader
:
public EventProcessor
{
public:
  //! @brief Default constructor.
  PHSDEventReader();
  //! @brief Copy constructor: the configuration and the list of files, not the open file. @param task object to copy
  PHSDEventReader(const PHSDEventReader & task);
  //! @brief Destructor.
  virtual ~PHSDEventReader() {}
  //! @brief Copy assignment: the configuration and the list of files, not the open file. @param task object to copy
  //! @return this object
  PHSDEventReader & operator=(const PHSDEventReader & task);
  //! @brief Add the default configuration of the task.
  virtual void setDefaultConfiguration() override;
  //! @brief Initialize the task: find the input files. @throws ConfigurationException if there is no input file
  virtual void initialize() override;
  //! @brief Read the next event and add its particles to the current event.
  //! @throws EndOfDataException at the end of the input; FileException if an event is incomplete
  virtual void execute() override;
  //! @brief Report the PDG codes skipped, and finalize the task.
  virtual void finalize() override;

  //! @brief Header of a PHSD event.
  struct Header
  {
    long   nParticles = 0;               //!< number of particles
    long   runIndex = 0;                 //!< run index
    long   eventIndex = 0;               //!< event index
    double impactParameter = 0.0;        //!< impact parameter (fm)
    int    impactSamplingMethod = 0;     //!< impact parameter sampling method
    int    pdgTarget = 0;                //!< PDG code of the target
    int    pdgProjectile = 0;            //!< PDG code of the projectile
    double sqrtS = 0.0;                  //!< energy in the centre of mass (GeV)
    int    nParticipants = 0;            //!< number of participants
    double psi[4] = {0,0,0,0};           //!< event-plane angles of orders 2 to 5
    double epsilon[4] = {0,0,0,0};       //!< eccentricities of orders 2 to 5
  };
  //! @brief Header of the last event read. @return header
  const Header & header() const { return _header; }

protected:

  //! @brief Particle type of a PDG code, or null if it is not in the particle database (counted as skipped).
  //! @param pdg PDG code @return type, or null
  ParticleType * particleType(int pdg);
  //! @brief Open the next input file. @return false if there is none left
  bool openNextFile();

  //! Input files.
  std::vector<String> _inputFileNames;
  //! Index of the next input file to open.
  unsigned int _nextFileIndex;
  //! Current input file (null: none open).
  std::unique_ptr<std::ifstream> _inputFile;
  //! Name of the current input file.
  String _inputFileName;
  //! Header of the last event read.
  Header _header;
  //! Particle types of the PDG codes met (null: not in the particle database).
  std::map<int,ParticleType*> _typeOfCode;
  //! Number of particles skipped, per PDG code not in the particle database.
  std::map<int,long> _skippedCodes;

};

}

#endif /* CAP__PHSDEventReader */
