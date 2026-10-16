#ifndef CAP__JetHelpers
#define CAP__JetHelpers
#include "TVector3.h"

namespace CAP
{

double calculateJt(TVector3 a, TVector3 b);

//! @brief Momentum of a particle perpendicular to a jet axis (jt) and angle to that axis.
//! @param jet_px jet px @param jet_py jet py @param jet_pz jet pz @param jet_p jet momentum
//! @param part_px particle px @param part_py particle py @param part_pz particle pz @param part_p particle momentum
//! @param part_jt perpendicular momentum (set) @param part_theta angle (set)
void calculateJtTheta(double jet_px,
                      double jet_py,
                      double jet_pz,
                      double jet_p,
                      double part_px,
                      double part_py,
                      double part_pz,
                      double part_p,
                      double & part_jt,
                      double & part_theta);

} // namespace CAP
#endif // JetSingleHistos_HPP
