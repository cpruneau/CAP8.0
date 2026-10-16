#ifndef CAP__MarkerVector
#define CAP__MarkerVector
#include "Drawable2D.hpp"
#include "MarkerAttributes.hpp"
#include "AxisRange.hpp"
#include "Data2D.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  //!
  //! One marker per point of a Data2D object.
  //! Data values are mapped to the canvas through a frame: the data ranges xRange() x yRange()
  //! are mapped onto the frame box (normalized canvas units). By default both are [0,1],
  //! so data values are used directly as canvas coordinates. The data object is not owned.
  //!
  class MarkerVector : public Drawable2D
  {
  public:

  MarkerVector();
  MarkerVector(Data2D * data);
  MarkerVector(const MarkerVector & src);
  MarkerVector  & operator=(const MarkerVector & rhs);
  virtual ~MarkerVector() { }
  void setData(Data2D * data) { _data = data; }
  Data2D * data() { return _data; }
  const Data2D * data() const { return _data; }
  void setMarkerAttributes(const MarkerAttributes & markerAttributes) { _markerAttributes = markerAttributes; }
  MarkerAttributes & markerAttributes() { return _markerAttributes; }
  const MarkerAttributes & markerAttributes() const { return _markerAttributes; }

  //! Data ranges mapped onto the frame.
  void setRanges(const AxisRange & xRange, const AxisRange & yRange) { _xRange = xRange; _yRange = yRange; }
  //! Frame box in normalized canvas units (lower-left and upper-right corners).
  void setFrame(float xLow, float yLow, float xHigh, float yHigh);
  const AxisRange & xRange() const { return _xRange; }
  const AxisRange & yRange() const { return _yRange; }

  //! Canvas position of the data value (x,y).
  Point2D toCanvas(float x, float y) const;

  protected:

  MarkerAttributes _markerAttributes;
  Data2D * _data;
  AxisRange _xRange;
  AxisRange _yRange;
  float _frameXLow;
  float _frameYLow;
  float _frameXHigh;
  float _frameYHigh;

  ClassDef(MarkerVector,0)
  };

  } // namespace GRAPHICS
}   // namespace CAP

#endif // !CAP__MarkerVector
