/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ParticleSingleDerivedHistos
#define CAP__ParticleSingleDerivedHistos
#include "ParticleSingleHistos.hpp"

namespace CAP
{

//!
//! @brief Former class of the derived single-particle histograms, now part of ParticleSingleHistos (base and derived
//! histograms in one folder). Kept as a name for the pair analyses until they move to the new design.
//! @deprecated Use ParticleSingleHistos.
//!
using ParticleSingleDerivedHistos = ParticleSingleHistos;

} // namespace CAP

#endif /* CAP__ParticleSingleDerivedHistos  */
