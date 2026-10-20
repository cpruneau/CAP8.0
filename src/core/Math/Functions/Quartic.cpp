#include "Quartic.hpp"

ClassImp(CAP::MATH::Quartic);

namespace CAP
{
  namespace MATH
  {

  Quartic::Quartic(double c0, double c1, double c2, double c3, double c4)
  :
  Polynomial(FixedDegree(),4)
  {
  setCoefficients(c0,c1,c2,c3,c4);
  }

  } // namespace MATH
} // namespace CAP
