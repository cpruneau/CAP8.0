/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,  Nov 10, 2026
 * Based on TGlauber.C written originally by B. Alver, M. Baker, 
 * C. Loizides, P. Steinberg, [arXiv:0805.4411](https://arxiv.org/abs/0805.4411) [nucl-ex]
 *
 * *********************************************************************/
#ifndef CAP__GlauberPlotter
#define CAP__GlauberPlotter

namespace CAP
{

//! @brief Draw the participants and spectators of the current Glauber event (legacy function).
//! @param xs nucleon-nucleon cross section @param participantColor color of the participants
//! @param participantStyle fill style of the participants @param spectatorColor color of the spectators
//! @param spectatorStyle fill style of the spectators
void draw(double xs,
          int participantColor = 4,
          int participantStyle = 1000,
          int spectatorColor   = 2,
          int spectatorStyle   = 1001);

}

#endif // CAP__GlauberPoint



