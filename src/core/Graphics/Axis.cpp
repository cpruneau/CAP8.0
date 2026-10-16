#include <cmath>
#include "Axis.hpp"
#include "AxisRenderer.hpp"

ClassImp(CAP::GRAPHICS::Axis);

namespace CAP
{
  namespace GRAPHICS
  {

  Axis::Axis()
  :
  Drawable2D(),
  _log(false),
  _reversed(false),
  _showMajorTicks(true),
  _showMinorTicks(true),
  _showLabels(true),
  _nMajorTicks(5),
  _nMinorTicks(5),
  _tickSide(1),
  _majorTickSize(0.02),
  _minorTickSize(0.01),
  _labelOffset(0.03),
  _labelAngle(0.0),
  _nominalRange(),
  _userRange(),
  _currentRange(),
  _lineAttributes(*LineAttributes::defaultLineAttributes()),
  _textAttributes(*TextAttributes::defaultTextAttributes())
  {
  setName("x");
  setTitle("x");
  setEndPoints(Point2D(0.1,0.1), Point2D(0.9,0.1));
  setRenderer(AxisRenderer::defaultRenderer());
  }

  Axis::Axis(const Point2D & start, const Point2D & end, float minimum, float maximum)
  :
  Axis()
  {
  setEndPoints(start, end);
  setRange(minimum, maximum);
  }

  Axis::Axis(const Axis & src)
  :
  Drawable2D(src),
  _log(src._log),
  _reversed(src._reversed),
  _showMajorTicks(src._showMajorTicks),
  _showMinorTicks(src._showMinorTicks),
  _showLabels(src._showLabels),
  _nMajorTicks(src._nMajorTicks),
  _nMinorTicks(src._nMinorTicks),
  _tickSide(src._tickSide),
  _majorTickSize(src._majorTickSize),
  _minorTickSize(src._minorTickSize),
  _labelOffset(src._labelOffset),
  _labelAngle(src._labelAngle),
  _nominalRange(src._nominalRange),
  _userRange(src._userRange),
  _currentRange(src._currentRange),
  _lineAttributes(src._lineAttributes),
  _textAttributes(src._textAttributes)
  { }

  Axis  & Axis::operator=(const Axis & rhs)
  {
  if (this != &rhs)
    {
    Drawable2D::operator=(rhs);
    _log            = rhs._log;
    _reversed       = rhs._reversed;
    _showMajorTicks = rhs._showMajorTicks;
    _showMinorTicks = rhs._showMinorTicks;
    _showLabels     = rhs._showLabels;
    _nMajorTicks    = rhs._nMajorTicks;
    _nMinorTicks    = rhs._nMinorTicks;
    _tickSide       = rhs._tickSide;
    _majorTickSize  = rhs._majorTickSize;
    _minorTickSize  = rhs._minorTickSize;
    _labelOffset    = rhs._labelOffset;
    _labelAngle     = rhs._labelAngle;
    _nominalRange   = rhs._nominalRange;
    _userRange      = rhs._userRange;
    _currentRange   = rhs._currentRange;
    _lineAttributes = rhs._lineAttributes;
    _textAttributes = rhs._textAttributes;
    }
  return *this;
  }

  //!
  //! Restore the default flags and tick settings, set the user and current ranges back
  //! to the nominal range, and reset the line and text attributes to their defaults.
  //!
  void Axis::reset()
  {
  Drawable2D::reset();
  _log            = false;
  _reversed       = false;
  _showMajorTicks = true;
  _showMinorTicks = true;
  _showLabels     = true;
  _nMajorTicks    = 5;
  _nMinorTicks    = 5;
  _userRange      = _nominalRange;
  _currentRange   = _nominalRange;
  _lineAttributes.reset();
  _textAttributes.reset();
  }

  void Axis::setEndPoints(const Point2D & start, const Point2D & end)
  {
  _points.clear();
  _points.push_back(start);
  _points.push_back(end);
  }

  void Axis::setNominalRange(float minimum, float maximum)
  {
  _nominalRange = AxisRange(minimum,maximum);
  }

  void Axis::setUserRange(float minimum, float maximum)
  {
  _userRange = AxisRange(minimum,maximum);
  }

  void Axis::setCurrentRange(float minimum, float maximum)
  {
  _currentRange = AxisRange(minimum,maximum);
  }

  void Axis::setRange(float minimum, float maximum)
  {
  _nominalRange = AxisRange(minimum,maximum);
  _userRange    = _nominalRange;
  _currentRange = _nominalRange;
  }

  bool Axis::usesLogScale() const
  {
  return _log && _currentRange.minimum() > 0;
  }

  float Axis::fraction(float value) const
  {
  float f;
  if (usesLogScale())
    {
    if (value <= 0) return _reversed ? 1.0 : 0.0;
    double lmin = std::log10(_currentRange.minimum());
    double lmax = std::log10(_currentRange.maximum());
    f = (std::log10(value) - lmin)/(lmax - lmin);
    }
  else
    f = (value - _currentRange.minimum())/_currentRange.range();
  return _reversed ? 1.0 - f : f;
  }

  Point2D Axis::toCanvas(float value) const
  {
  Point2D a = _points.size() > 0 ? _points[0] : Point2D(0.0,0.0);
  Point2D b = _points.size() > 1 ? _points[1] : Point2D(1.0,0.0);
  float f = fraction(value);
  return Point2D(a.x() + f*(b.x()-a.x()), a.y() + f*(b.y()-a.y()));
  }

  } // namespace GRAPHICS
}   // namespace CAP
