#include "Marker.hpp"
#include "MarkerRenderer.hpp"

ClassImp(CAP::GRAPHICS::Marker);

namespace CAP
{
  namespace GRAPHICS
  {
  Marker::Marker()
  :
  Drawable2D(),
  _markerAttributes(*MarkerAttributes::defaultMarkerAttributes())
  {
  setRenderer(MarkerRenderer::defaultRenderer());
  }

  Marker::Marker(const Marker & src)
  :
  Drawable2D(src),
  _markerAttributes(src._markerAttributes)
  { }

  Marker  & Marker::operator=(const Marker & rhs)
  {
  if (this != &rhs)
    {
    Drawable2D::operator=(rhs);
    _markerAttributes = rhs._markerAttributes;
    }
  return *this;
  }

  } // namespace GRAPHICS
}   // namespace CAP
