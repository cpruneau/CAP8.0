#include "Linear.hpp"
#include "MathExceptions.hpp"

ClassImp(CAP::MATH::Linear);

namespace CAP
{
  namespace MATH
  {

  Linear::Linear(double c0, double c1)
  :
  Polynomial(FixedDegree(),1)
  {
  setParameterValue(0,c0);
  setParameterValue(1,c1);
  }

  double Linear::root() const
  {
  if (parameterValue(1)==0.0) throw MathException("No root: slope is zero",__FUNCTION__);
  return -parameterValue(0)/parameterValue(1);
  }

  } // namespace MATH
} // namespace CAP
