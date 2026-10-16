#ifndef CAP__Axis
#define CAP__Axis
#include "AxisRange.hpp"
#include "Drawable2D.hpp"
#include "LineAttributes.hpp"
#include "TextAttributes.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  //!
  //! Axis drawn from points()[0] (start) to points()[1] (end), in normalized canvas units.
  //! The current range is mapped onto that segment (linearly, or logarithmically if isLog()
  //! and the range is positive; reversed if isReversed()).
  //! Ticks are drawn on one side of the axis: tickSide() = +1 is the left of the start->end
  //! direction (up for a left-to-right x axis), -1 is the right (use -1 for a bottom-to-top
  //! y axis so its ticks point into the frame). Labels and the title go on the other side.
  //! Tick sizes and label offset are fractions of the canvas height.
  //!
  class Axis : public Drawable2D
  {
  public:

  Axis();
  Axis(const Point2D & start, const Point2D & end, float minimum, float maximum);
  Axis(const Axis & src);
  Axis  & operator=(const Axis & rhs);
  virtual ~Axis() { }

  void setEndPoints(const Point2D & start, const Point2D & end);
  void setLog(bool v) { _log = v; }
  void setReversed(bool v) { _reversed = v; }
  void setShowMajorTicks(bool v) { _showMajorTicks = v; }
  void setShowMinorTicks(bool v) { _showMinorTicks = v; }
  void setShowLabels(bool v)     { _showLabels = v; }
  void setNMajorTicks(int v) { _nMajorTicks = v; }
  void setNMinorTicks(int v) { _nMinorTicks = v; }
  void setTickSide(int v)    { _tickSide = (v<0) ? -1 : 1; }
  void setMajorTickSize(float v) { _majorTickSize = v; }
  void setMinorTickSize(float v) { _minorTickSize = v; }
  void setLabelOffset(float v)   { _labelOffset = v; }
  void setLabelAngle(float v)    { _labelAngle = v; }
  void setNominalRange(const AxisRange & v) { _nominalRange = v; }
  void setUserRange(const AxisRange & v)    { _userRange = v; }
  void setCurrentRange(const AxisRange & v) { _currentRange = v; }
  void setNominalRange(float minimum, float maximum);
  void setUserRange(float minimum, float maximum);
  void setCurrentRange(float minimum, float maximum);
  //! Set nominal, user and current ranges at once.
  void setRange(float minimum, float maximum);
  void setLineAttributes(const LineAttributes & attributes) { _lineAttributes = attributes; }
  void setTextAttributes(const TextAttributes & attributes) { _textAttributes = attributes; }

  bool  isLog() const          { return _log; }
  bool  isReversed() const     { return _reversed; }
  bool  showMajorTicks() const { return _showMajorTicks; }
  bool  showMinorTicks() const { return _showMinorTicks; }
  bool  showLabels() const     { return _showLabels; }
  int   nMajorTicks() const    { return _nMajorTicks; }
  int   nMinorTicks() const    { return _nMinorTicks; }
  int   tickSide() const       { return _tickSide; }
  float majorTickSize() const  { return _majorTickSize; }
  float minorTickSize() const  { return _minorTickSize; }
  float labelOffset() const    { return _labelOffset; }
  float labelAngle() const     { return _labelAngle; }
  AxisRange & nominalRange()   { return _nominalRange; }
  AxisRange & userRange()      { return _userRange; }
  AxisRange & currentRange()   { return _currentRange; }
  const AxisRange & nominalRange()  const { return _nominalRange; }
  const AxisRange & userRange()     const { return _userRange; }
  const AxisRange & currentRange()  const { return _currentRange; }
  float nominalRangeMinimum() const { return _nominalRange.minimum(); }
  float nominalRangeMaximum() const { return _nominalRange.maximum(); }
  float userRangeMinimum()    const { return _userRange.minimum(); }
  float userRangeMaximum()    const { return _userRange.maximum(); }
  float currentRangeMinimum() const { return _currentRange.minimum(); }
  float currentRangeMaximum() const { return _currentRange.maximum(); }

  LineAttributes & lineAttributes() { return _lineAttributes; }
  TextAttributes & textAttributes() { return _textAttributes; }
  const LineAttributes & lineAttributes() const { return _lineAttributes; }
  const TextAttributes & textAttributes() const { return _textAttributes; }

  //! True if the axis is drawn with a log scale (isLog() and current minimum > 0).
  bool  usesLogScale() const;
  //! Fraction (0 at start, 1 at end) of the axis length where value sits.
  float fraction(float value) const;
  //! Canvas position of value along the axis.
  Point2D toCanvas(float value) const;

  virtual void reset();

  protected:

  bool  _log;
  bool  _reversed;
  bool  _showMajorTicks;
  bool  _showMinorTicks;
  bool  _showLabels;
  int   _nMajorTicks;
  int   _nMinorTicks;
  int   _tickSide;
  float _majorTickSize;
  float _minorTickSize;
  float _labelOffset;
  float _labelAngle;
  AxisRange _nominalRange;
  AxisRange _userRange;
  AxisRange _currentRange;
  LineAttributes _lineAttributes;
  TextAttributes _textAttributes;

  ClassDef(Axis,0)
  };
  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Axis
