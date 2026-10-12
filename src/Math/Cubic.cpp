#include "Cubic.hpp"
#include "MathExceptions.hpp"

ClassImp(CAP::MATH::Cubic);

namespace CAP
{
  namespace MATH
  {

  Cubic::Cubic(double c0, double c1, double c2, double c3)
  :
  Polynomial(FixedDegree(),3)
  {
  setCoefficients(c0,c1,c2,c3);
  }

  double Cubic::inflectionX() const
  {
  if (parameterValue(3)==0.0) throw MathException("No inflection point: cubic coefficient is zero",__FUNCTION__);
  return -parameterValue(2)/(3.0*parameterValue(3));
  }

  } // namespace MATH
} // namespace CAP
