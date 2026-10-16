#include "MarkerAttributes.hpp"

ClassImp(CAP::GRAPHICS::MarkerAttributes);

namespace CAP
{
  namespace GRAPHICS
  {
  MarkerAttributes::MarkerAttributes()
  :
  Attributes(),
  _size(1.0)
  { }

  MarkerAttributes::MarkerAttributes(const MarkerAttributes & src)
  :
  Attributes(src),
  _size(src._size)
  { }

  MarkerAttributes  & MarkerAttributes::operator=(const MarkerAttributes & rhs)
  {
  if (this != &rhs)
    {
    Attributes::operator=(rhs);
    _size = rhs._size;
    }
  return *this;
  }

  void MarkerAttributes::reset()
  {
  const MarkerAttributes & d = *defaultMarkerAttributes();
  _color = d._color;
  _style = d._style;
  _alpha = d._alpha;
  _size  = d._size;
  }

  MarkerAttributes * MarkerAttributes::_defaultMarkerAttributes = nullptr;

  MarkerAttributes * MarkerAttributes::defaultMarkerAttributes()
  {
  if (!_defaultMarkerAttributes) _defaultMarkerAttributes = new MarkerAttributes();
  return _defaultMarkerAttributes;
  }

  void MarkerAttributes::setDefaultMarkerAttributes(int color, float alpha, int style, float size)
  {
  if (!_defaultMarkerAttributes) _defaultMarkerAttributes = new MarkerAttributes();
  _defaultMarkerAttributes->set(color, alpha, style, size);
  }

  }  // namespace GRAPHICS
} // namespace CAP
