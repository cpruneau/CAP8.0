#ifndef CAP__MarkerVector
#define CAP__MarkerVector
#include "Drawable2D.hpp"
#include "MarkerAttributes.hpp"
#include "AxisRange.hpp"
#include "DataPoints.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  //!
  //! One marker per point of a data set (CAP::MATH::DataPoints).
  //! Data values are mapped to the canvas through a frame: the data ranges xRange() x yRange()
  //! are mapped onto the frame box (normalized canvas units). By default both are [0,1],
  //! so data values are used directly as canvas coordinates. The data object is not owned.
  //!
  class MarkerVector : public Drawable2D
  {
  public:

  MarkerVector();
  MarkerVector(const MATH::DataPoints * data);
  MarkerVector(const MarkerVector & src);
  MarkerVector  & operator=(const MarkerVector & rhs);
  virtual ~MarkerVector() { }
  void setData(const MATH::DataPoints * data) { _data = data; }
  const MATH::DataPoints * data() const { return _data; }
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
  const MATH::DataPoints * _data;
  AxisRange _xRange;
  AxisRange _yRange;
  float _frameXLow;
  float _frameYLow;
  float _frameXHigh;
  float _frameYHigh;

  };

  } // namespace GRAPHICS
}   // namespace CAP

#endif // !CAP__MarkerVector
