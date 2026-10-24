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
#ifndef CAP__ParticleDecayerTask
#define CAP__ParticleDecayerTask
#include "EventProcessor.hpp"
#include "Event.hpp"
#include "EventFilter.hpp"
#include "EventEfficiency.hpp"
#include "EventSmearer.hpp"
#include "Particle.hpp"
#include "ParticleFilter.hpp"
#include "ParticleEfficiency.hpp"
#include "ParticleSmearer.hpp"
#include "ParticleDecayer.hpp"
#include "ClassDef.hpp"

namespace CAP
{

//!
//! @brief Task decaying the unstable particles of the events.
//!
//! Task handles the decay of all decayable particles in the incoming event stream. At execution time, i.e., for each event encountered,
//! each particle of the input stream is examined to determine whether it should be decayed. Particle deemed decayable are passed to the
//! decay handler (decayer) and decayed.  Currently, the decayer handles two and three particle decays. Decayed particles are kept in the
//! event stream but tagged as decayed. Child particles are inserted in the stream and tagged as "live" (undecayed).  Child particles that
//! are themselves decayable are decayed iteratively. 
//!
class ParticleDecayerTask
:
public EventProcessor,
public ParticleDecayer
{
public:

  //! @brief Default constructor.
  ParticleDecayerTask();
  //! @brief Destructor.
  virtual ~ParticleDecayerTask() {}
  //! @brief Decay the live, unstable particles of the current event (recursively, see decayParticle()).
  virtual void execute();
  //!
  //! @brief Decay a particle: draw a decay mode of its type (ParticleType::generateDecayMode()), create the products
  //! with the particle factory, compute their momenta and positions (ParticleDecayer::decay2(), decay3(), decay4()), add them to
  //! the event, decay the unstable ones, and mark the parent as decayed.
  //! @param particle particle to decay @param event event receiving the products
  //! @throws ParticleDecayException for a decay mode with fewer than 2 or more than 4 products
  //!
  virtual void decayParticle(Particle & particle, Event & event);

  ClassDef(ParticleDecayerTask,0)
};

} // namespace CAP

#endif /* CAP__ParticleDecayerTask */
