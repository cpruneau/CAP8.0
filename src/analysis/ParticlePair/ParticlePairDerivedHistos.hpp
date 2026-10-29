/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__ParticlePairDerivedHistos
#define CAP__ParticlePairDerivedHistos
#include "ParticlePairHistos.hpp"

namespace CAP
{

//!
//! @brief Former class of the derived pair histograms, now part of ParticlePairHistos (base and derived histograms of a
//! pair in one group). Kept as a name until the balance functions move to the new design.
//! @deprecated Use ParticlePairHistos.
//!
using ParticlePairDerivedHistos = ParticlePairHistos;

} // namespace CAP

#endif /* CAP__ParticlePairDerivedHistos  */
