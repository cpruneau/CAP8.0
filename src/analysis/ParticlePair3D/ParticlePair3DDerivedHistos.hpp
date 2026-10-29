/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * *********************************************************************/
#ifndef CAP__ParticlePair3DDerivedHistos
#define CAP__ParticlePair3DDerivedHistos
#include "ParticlePair3DHistos.hpp"

namespace CAP
{

//!
//! @brief Former class of the derived 3D pair histograms, now part of ParticlePair3DHistos (base and derived histograms
//! in one folder). Kept as a name for ParticlePair3DBfHistos until the balance functions move to the new design.
//! @deprecated Use ParticlePair3DHistos.
//!
using ParticlePair3DDerivedHistos = ParticlePair3DHistos;

} // namespace CAP

#endif /* CAP__ParticlePair3DDerivedHistos */
