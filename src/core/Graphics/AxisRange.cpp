#include "AxisRange.hpp"
#include "Exceptions.hpp"
#include "PrintHelpers.hpp"


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
  if (maximum<=minimum)
    {
    String message = "Invalid axis range [";
    message += minimum; message += ","; message += maximum; message += "]: maximum<=minimum";
    throw Exception(message,__FUNCTION__,"RangeException");
    }
  _minimum = minimum;
  _maximum = maximum;
  }

  } // namespace GRAPHICS
}   // namespace CAP
