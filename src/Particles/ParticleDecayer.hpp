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
#ifndef CAP__ParticleDecayer
#define CAP__ParticleDecayer
#include <iostream>
#include <iomanip>
#include "TRandom.h"
#include "Particle.hpp"
#include "VectorLorentz.hpp"

using namespace std;

namespace CAP
{

//!
//! @brief Kinematics of particle decays: momenta and production points of the decay products.
//!
//! The parent decays after a time drawn from an exponential distribution of mean gamma*tau (tau: lifetime of its type,
//! gamma: Lorentz factor); the products are created at the decay point. Two-body decays are isotropic in the parent
//! rest frame; three-body decays draw the product energies in the rest frame; four-body decays are uniform in phase
//! space (sequential two-body decays with GENBOD weights). Momenta are then boosted to the frame of
//! the parent. If the parent mass is below the sum of the product masses, it is set to 1.02 times that sum.
//!
class ParticleDecayer
{
public:
  //! @brief Default constructor (uses gRandom).
  ParticleDecayer();
  //! @brief Destructor.
  virtual ~ParticleDecayer() {}
  //! @brief Set the random number generator. @param _random generator (not owned)
  void setRandomGenerator(TRandom * _random) { random = _random; }
  //! @brief Random number generator. @return generator
  TRandom * getRandomGenerator() { return random; }

  //! @brief Two-body decay. @param parentType type of the parent @param parentMomentum its momentum
  //! @param parentPosition its production point @param childType1 type of product 1 @param p1 its momentum (set)
  //! @param r1 its production point (set) @param childType2 type of product 2 @param p2 its momentum (set)
  //! @param r2 its production point (set)
  void decay2(ParticleType  & parentType,
              VectorLorentz & parentMomentum,
              VectorLorentz & parentPosition,
              ParticleType  & childType1,
              VectorLorentz & p1,
              VectorLorentz & r1,
              ParticleType  & childType2,
              VectorLorentz & p2,
              VectorLorentz & r2);

  //! @brief Three-body decay (see decay2() for the parameters; p3, r3 for product 3).
  void decay3(ParticleType  & parentType,
              VectorLorentz & parentMomentum,
              VectorLorentz & parentPosition,
              ParticleType  & childType1,
              VectorLorentz & p1,
              VectorLorentz & r1,
              ParticleType  & childType2,
              VectorLorentz & p2,
              VectorLorentz & r2,
              ParticleType  & childType3,
              VectorLorentz & p3,
              VectorLorentz & r3);

  //! @brief Four-body decay, uniform in phase space (sequential two-body decays with GENBOD weights; see decay2() for
  //! the parameters, p3, r3, p4, r4 for products 3 and 4).
  void decay4(ParticleType  & parentType,
              VectorLorentz & parentMomentum,
              VectorLorentz & parentPosition,
              ParticleType  & childType1,
              VectorLorentz & p1,
              VectorLorentz & r1,
              ParticleType  & childType2,
              VectorLorentz & p2,
              VectorLorentz & r2,
              ParticleType  & childType3,
              VectorLorentz & p3,
              VectorLorentz & r3,
              ParticleType  & childType4,
              VectorLorentz & p4,
              VectorLorentz & r4
              );

  //! @brief Two-body decay of a particle. @param parent parent @param child1 product 1 (type set) @param child2 product 2 (type set)
  void  decay2(Particle & parent,
               Particle & child1,
               Particle & child2);
  //! @brief Three-body decay of a particle. @param parent parent @param child1 product 1 @param child2 product 2 @param child3 product 3
  void  decay3(Particle & parent,
               Particle & child1,
               Particle & child2,
               Particle & child3);
  //! @brief Four-body decay of a particle.
  //! @param parent parent @param child1 product 1 @param child2 product 2 @param child3 product 3 @param child4 product 4
  void  decay4(Particle & parent,
               Particle & child1,
               Particle & child2,
               Particle & child3,
               Particle & child4);

protected:

  //! @brief Momentum of the products of a two-body decay in the rest frame of the mother.
  //! @param m mass of the mother @param ma mass of product a @param mb mass of product b @return momentum (0 if m<ma+mb)
  static double twoBodyMomentum(double m, double ma, double mb);
  //! @brief Isotropic two-body decay in the rest frame of the mother. @param m mass of the mother
  //! @param ma mass of product a @param mb mass of product b @param pa momentum of a (set) @param pb momentum of b (set)
  void twoBodyDecay(double m, double ma, double mb, VectorLorentz & pa, VectorLorentz & pb);

  //! 2 pi.
  static double   twoPi;

  //! Random number generator (not owned).
  TRandom * random;



  ClassDef(ParticleDecayer,0)
};

}

#endif  // ParticleDecayer

