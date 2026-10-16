#ifndef CAP__Marker
#define CAP__Marker
#include "Drawable2D.hpp"
#include "MarkerAttributes.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  class Marker : public Drawable2D
  {
  public:

  Marker();
  Marker(const Marker & src);
  Marker  & operator=(const Marker & rhs);
  virtual ~Marker() { }

  void setMarkerAttributes(const MarkerAttributes & markerAttributes) { _markerAttributes = markerAttributes; }
  MarkerAttributes & markerAttributes() { return _markerAttributes; }
  const MarkerAttributes & markerAttributes() const { return _markerAttributes; }

  protected:

  MarkerAttributes _markerAttributes;

  ClassDef(Marker,0)
  };


  } // namespace GRAPHICS
}   // namespace CAP

#endif // !CAP__Marker
