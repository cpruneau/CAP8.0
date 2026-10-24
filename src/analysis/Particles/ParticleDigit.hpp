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
#ifndef CAP__ParticleDigit
#define CAP__ParticleDigit
#include "Aliases.hpp"
#include "Factory.hpp"
#include "ClassDef.hpp"

using namespace std;
namespace CAP
{
  //!
  //! @brief Particle reduced to its kinematic bins and values, used by the analyzers to fill histograms quickly.
  //!
  //! Digits are created by a shared factory (factory()) to avoid allocations in the event loop.
  //!
  class ParticleDigit
  {
  public:

  //! @brief Default constructor: all bins and values 0.
  ParticleDigit();
  //! @brief Constructor. @param iY rapidity bin @param iEta pseudorapidity bin @param iPhi azimuth bin @param iPt pT bin
  //! @param pt transverse momentum @param phi azimuth @param eta pseudorapidity @param y rapidity @param e energy
  ParticleDigit(unsigned int iY,
                unsigned int iEta,
                unsigned int iPhi,
                unsigned int iPt,
                float       pt,
                float       phi,
                float       eta,
                float       y,
                float       e);
  //! @brief Destructor.
  virtual ~ParticleDigit(){}
  //! @brief Copy constructor. @param other digit to copy
  ParticleDigit(const ParticleDigit& other);
  //! @brief Copy assignment. @param other digit to copy @return this digit
  ParticleDigit & operator=(const ParticleDigit & other);
  //! @brief Print the digit. @param output stream
  void print(ostream & output);
  //! @brief Set all bins and values to 0.
  void reset();

  //! Rapidity bin.
  unsigned int _iY;
  //! Pseudorapidity bin.
  unsigned int _iEta;
  //! Azimuth bin.
  unsigned int _iPhi;
  //! Transverse momentum bin.
  unsigned int _iPt;
  //! Transverse momentum.
  float _pt;
  //! Azimuth.
  float _phi;
  //! Pseudorapidity.
  float _eta;
  //! Rapidity.
  float _y;
  //! Energy.
  float _e;

  //! Initial size of the factory.
  static int _factorySize;
  //! Factory of digits (created by factory()).
  static Factory<ParticleDigit> * _factory;
  //! @brief Factory of digits, created on first use. @return factory
  static Factory<ParticleDigit> * factory();


  ClassDef(ParticleDigit,0)
  };

}

#endif /* CAP__ParticleDigit */
