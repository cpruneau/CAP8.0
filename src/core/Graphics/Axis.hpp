#ifndef CAP__Axis
#define CAP__Axis
#include "AxisRange.hpp"
#include "Drawable2D.hpp"
#include "LineAttributes.hpp"
#include "TextAttributes.hpp"
#include "Rect2D.hpp"

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
  //! The labels use textAttributes(), the title titleAttributes() (color, font and size). The title can be moved
  //! from its automatic position (centered beyond the labels) with setTitleOffset(), in canvas units.
  //!
  //! When the axis belongs to a Graph, the graph sets the sizes, the ticks and the label offset from the size of its
  //! cell; the axis then only provides the style: labelPointSize() and titlePointSize() (in device units, e.g.
  //! points; 0 for the automatic size), labelGap() (distance of the labels from the axis, relative to the automatic
  //! distance) and the title offset, in fractions of the cell of the graph.
  //!
  //! After each drawing, titleRect() and bodyRect() (axis line, ticks and labels) tell where the axis was drawn.
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
  void setTitleAttributes(const TextAttributes & attributes) { _titleAttributes = attributes; }
  //! @brief Displacement of the title from its automatic position (canvas units; fractions of the cell for the axis
  //! of a Graph).
  void setTitleOffset(float dx, float dy) { _titleOffsetX = dx; _titleOffsetY = dy; }
  //! @brief Size of the labels and of the title in device units (0: automatic; used by Graph).
  void setLabelPointSize(float size) { _labelPointSize = size < 0 ? 0 : size; }
  void setTitlePointSize(float size) { _titlePointSize = size < 0 ? 0 : size; }
  //! @brief Distance of the labels from the axis, relative to the automatic distance (1; used by Graph).
  void setLabelGap(float gap) { _labelGap = gap < 0 ? 0 : gap; }
  //! @brief Leave out the label of the tick at the start or at the end of the axis, for one drawing (e.g. where
  //! two axes of a 3D view meet). Not copied: copies of the axis show all their labels.
  //! @param start hide the label at the start @param end hide the label at the end
  void hideEndLabels(bool start, bool end) { _hideStartLabel = start; _hideEndLabel = end; }
  bool hidesStartLabel() const { return _hideStartLabel; }   //!< Whether the label at the start is left out
  bool hidesEndLabel() const { return _hideEndLabel; }   //!< Whether the label at the end is left out

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
  TextAttributes & titleAttributes() { return _titleAttributes; }
  const TextAttributes & titleAttributes() const { return _titleAttributes; }
  float titleOffsetX()   const { return _titleOffsetX; }
  float titleOffsetY()   const { return _titleOffsetY; }
  float labelPointSize() const { return _labelPointSize; }
  float titlePointSize() const { return _titlePointSize; }
  float labelGap()       const { return _labelGap; }

  //! @brief Where the title and the body (line, ticks, labels) were drawn last (canvas units; empty if not drawn).
  const Rect2D & titleRect() const { return _titleRect; }
  const Rect2D & bodyRect()  const { return _bodyRect; }
  //! Set by the renderer.
  void setDrawnRects(const Rect2D & title, const Rect2D & body) const { _titleRect = title; _bodyRect = body; }

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
  bool  _hideStartLabel = false;   //!<! label at the start left out, for one drawing (not streamed, not copied)
  bool  _hideEndLabel = false;     //!<! label at the end left out, for one drawing (not streamed, not copied)
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
  TextAttributes _titleAttributes;
  float _titleOffsetX;
  float _titleOffsetY;
  float _labelPointSize;
  float _titlePointSize;
  float _labelGap;
  mutable Rect2D _titleRect; //! not streamed (set when drawn)
  mutable Rect2D _bodyRect;  //! not streamed (set when drawn)

  };
  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Axis
