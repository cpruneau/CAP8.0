#include "AxisRange.hpp"
#include "Exceptions.hpp"
#include "PrintHelpers.hpp"

ClassImp(CAP::GRAPHICS::AxisRange);

namespace CAP
{
  namespace GRAPHICS
  {

  AxisRange::AxisRange()
  :
  _minimum(0),
  _maximum(1)
  { }

  AxisRange::AxisRange(float minimum, float maximum)
  :
  _minimum(0),
  _maximum(1)
  {
  set(minimum,maximum);
  }

  AxisRange::AxisRange(const AxisRange & src)
  :
  _minimum(src._minimum),
  _maximum(src._maximum)
  { }

  AxisRange  & AxisRange::operator=(const AxisRange & rhs)
  {
  _minimum = rhs._minimum;
  _maximum = rhs._maximum;
  return *this;
  }


  void AxisRange::set(float minimum, float maximum)
  {
  if (maximum<=minimum) throw RangeException(minimum, maximum, "Invalid range: maximum<=minimum", __FUNCTION__);
  _minimum = minimum;
  _maximum = maximum;
  }

  } // namespace GRAPHICS
}   // namespace CAP
