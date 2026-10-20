#include "Parabolic.hpp"
#include "MathExceptions.hpp"

ClassImp(CAP::MATH::Parabolic);

namespace CAP
{
  namespace MATH
  {

  Parabolic::Parabolic(double c0, double c1, double c2)
  :
  Polynomial(FixedDegree(),2)
  {
  setParameterValue(0,c0);
  setParameterValue(1,c1);
  setParameterValue(2,c2);
  }

  double Parabolic::extremumX() const
  {
  if (parameterValue(2)==0.0) throw MathException("No extremum: quadratic coefficient is zero",__FUNCTION__);
  return -parameterValue(1)/(2.0*parameterValue(2));
  }

  } // namespace MATH
} // namespace CAP
