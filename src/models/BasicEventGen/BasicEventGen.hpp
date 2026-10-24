/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__BasicEventGen
#define CAP__BasicEventGen
#include "EventProcessor.hpp"
#include "ParticleDb.hpp"

namespace CAP
{

//!
//! @brief Simple event generator: for each selected particle type, a random number of particles (0 to 99) with
//! uniform azimuth, rapidity uniform in [RapidityMin, RapidityMax] and exponential pT (slope PtSlope).
//!
//! Keys: \<name\>:MultMin, MultMax, PdgSelected, RapidityMin, RapidityMax, PtSlope, Random:SetSeed, Random:Seed.
//! @note The multiplicity range MultMin, MultMax is read but not used (the number of particles is drawn uniformly
//! below 100).
//!
class BasicEventGen
:
public EventProcessor
{
public:

  //! @brief Default constructor.
  BasicEventGen();
  //! @brief Copy constructor. @param source object to copy
  BasicEventGen(const BasicEventGen & source);
  //! @brief Destructor.
  virtual ~BasicEventGen() {}
  //! @brief Copy assignment. @param rhs object to copy @return this object
  BasicEventGen & operator=(const BasicEventGen & rhs);
  //! @brief Add the default configuration of the task.
  virtual void setDefaultConfiguration();
  //! @brief Initialize the task.
  virtual void initialize();
  //! @brief Execute the task (process the current event).
  virtual void execute();

protected:

  //! PDG code of the generated particles.
  int    pdgSelected;
  //! Minimum multiplicity (not used).
  double multMin;
  //! Maximum multiplicity (not used).
  double multMax;
  //! Lower edge of the rapidity range.
  double rapidityMin;
  //! Upper edge of the rapidity range.
  double rapidityMax;
  //! Slope (mean) of the exponential pT distribution.
  double ptSlope;
  //! Whether the random seed is set.
  bool   setSeed;
  //! Random seed.
  int    seedValue;
  //! Masses of the generated types.
  std::vector<double> mass;
  //! Squared masses of the generated types.
  std::vector<double> massSq;
  //! Generated types.
  std::vector<ParticleType*> types;

};

}

#endif /* CAP__BasicEventGen */
