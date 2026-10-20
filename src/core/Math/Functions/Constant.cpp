#include "Constant.hpp"

ClassImp(CAP::MATH::Constant);

namespace CAP
{
  namespace MATH
  {

  Constant::Constant(double c0)
  :
  Polynomial(FixedDegree(),0)
  {
  setParameterValue(0,c0);
  }

  } // namespace MATH
} // namespace CAP
