/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#include "ParticlePairFilter.hpp"
#include "String.hpp"
#include "PrintHelpers.hpp"

ClassImp(CAP::ParticlePairFilter);

namespace CAP
{

ParticlePairFilter::ParticlePairFilter()
:
Filter<Particle>()
{
  // no ops
}


bool ParticlePairFilter::accept(const Particle & particle1, const Particle & particle2) const
{
//  ParticleType & type1 = particle1.type();
//  ParticleType & type2 = particle2.type();
  if (particle1.isDecayed() )          return false;
  if (particle2.isDecayed() )          return false;
  return true;
}
} // namespace CAP
