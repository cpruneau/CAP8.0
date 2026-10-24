#include "MarkerVector.hpp"
#include "MarkerRenderer.hpp"


namespace CAP
{
  namespace GRAPHICS
  {

  MarkerVector::MarkerVector()
  :
  Drawable2D(),
  _markerAttributes(*MarkerAttributes::defaultMarkerAttributes()),
  _data(nullptr),
  _xRange(0.0,1.0),
  _yRange(0.0,1.0),
  _frameXLow(0.0),
  _frameYLow(0.0),
  _frameXHigh(1.0),
  _frameYHigh(1.0)
  {
  setRenderer(MarkerRenderer::defaultRenderer());
  }

  MarkerVector::MarkerVector(const MATH::DataPoints * data)
  :
  MarkerVector()
  {
  _data = data;
  }

  MarkerVector::MarkerVector(const MarkerVector & src)
  :
  Drawable2D(src),
  _markerAttributes(src._markerAttributes),
  _data(src._data),
  _xRange(src._xRange),
  _yRange(src._yRange),
  _frameXLow(src._frameXLow),
  _frameYLow(src._frameYLow),
  _frameXHigh(src._frameXHigh),
  _frameYHigh(src._frameYHigh)
  {   }

  MarkerVector  & MarkerVector::operator=(const MarkerVector & rhs)
  {
  if (this != &rhs)
    {
    Drawable2D::operator=(rhs);
    _markerAttributes = rhs._markerAttributes;
    _data       = rhs._data;
    _xRange     = rhs._xRange;
    _yRange     = rhs._yRange;
    _frameXLow  = rhs._frameXLow;
    _frameYLow  = rhs._frameYLow;
    _frameXHigh = rhs._frameXHigh;
    _frameYHigh = rhs._frameYHigh;
    }
  return *this;
  }

  void MarkerVector::setFrame(float xLow, float yLow, float xHigh, float yHigh)
  {
  _frameXLow  = xLow;
  _frameYLow  = yLow;
  _frameXHigh = xHigh;
  _frameYHigh = yHigh;
  }

  Point2D MarkerVector::toCanvas(float x, float y) const
  {
  float u = _frameXLow + (x - _xRange.minimum())/_xRange.range()*(_frameXHigh - _frameXLow);
  float v = _frameYLow + (y - _yRange.minimum())/_yRange.range()*(_frameYHigh - _frameYLow);
  return Point2D(u,v);
  }

  } // namespace GRAPHICS
}   // namespace CAP
