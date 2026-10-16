#ifndef CAP__GlauberPlotter
#define CAP__GlauberPlotter
#include "TObject.h"

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



