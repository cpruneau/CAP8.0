#ifndef CAP__Graph
#define CAP__Graph
#include <vector>
#include "Drawable2D.hpp"
#include "Axis.hpp"
#include "Margins.hpp"
#include "FillAttributes.hpp"
#include "LineAttributes.hpp"
#include "MarkerAttributes.hpp"
#include "TextAttributes.hpp"
#include "Rect2D.hpp"

namespace CAP
{
  namespace MATH
  {
  class Function;
  class DataPoints;
  }

  namespace GRAPHICS
  {

  //!
  //! @brief One object shown in a Graph: a function of one variable or a set of data points, with its drawing style.
  //!
  //! The objects are not owned: they must outlive the graphs that show them (in CapApp they are owned by the
  //! application or by the files it opened).
  //!
  class GraphItem
  {
  public:

  enum Kind { kFunction, kData };
  //! How the systematic uncertainties of data points are drawn: boxes around the points, brackets above and below
  //! the points, or a band joining the points.
  enum SysStyle { kSysBoxes, kSysBrackets, kSysBand };

  //! @brief Function f(x) drawn as a curve over [xMinimum, xMaximum]. @param function function of rank 1
  //! @param xMinimum lower end of the curve @param xMaximum upper end @param label legend label (the function's
  //! name if empty)
  GraphItem(const MATH::Function * function, float xMinimum, float xMaximum, const String & label="");
  //! @brief Data points drawn as markers with error bars. @param data data points @param label legend label (the
  //! name of the data if empty)
  GraphItem(const MATH::DataPoints * data, const String & label="");

  Kind kind() const { return _kind; }
  const MATH::Function   * function() const { return _function; }
  const MATH::DataPoints * data()     const { return _data; }
  //! @brief Show other data points (e.g. a new version of the same object); ignored for a function. @param data data
  void setData(const MATH::DataPoints * data) { if (_kind == kData && data) _data = data; }
  //! @brief The object shown (function or data), to compare items. @return pointer
  const void * object() const { return _kind == kFunction ? (const void*)_function : (const void*)_data; }

  const String & label() const { return _label; }
  void setLabel(const String & label) { _label = label; }

  //! Curve range of a function (ignored for data).
  float xMinimum() const { return _xMinimum; }
  float xMaximum() const { return _xMaximum; }
  void  setXRange(float xMinimum, float xMaximum) { _xMinimum = xMinimum; _xMaximum = xMaximum; }
  //! Number of points at which a function is evaluated to draw its curve.
  int   nSamples() const { return _nSamples; }
  void  setNSamples(int n) { _nSamples = n < 2 ? 2 : n; }

  LineAttributes   & lineAttributes()   { return _line; }
  MarkerAttributes & markerAttributes() { return _marker; }
  const LineAttributes   & lineAttributes()   const { return _line; }
  const MarkerAttributes & markerAttributes() const { return _marker; }

  //! What is drawn: line through the points (functions: the curve), markers, error bars of the statistical
  //! uncertainties (and of the x uncertainties), systematic uncertainties (drawn as set by sysStyle()).
  bool showLine()      const { return _showLine; }
  bool showMarkers()   const { return _showMarkers; }
  bool showErrors()    const { return _showErrors; }
  bool showSysErrors() const { return _showSysErrors; }
  void setShowLine(bool v)      { _showLine = v; }
  void setShowMarkers(bool v)   { _showMarkers = v; }
  void setShowErrors(bool v)    { _showErrors = v; }
  void setShowSysErrors(bool v) { _showSysErrors = v; }

  //! @name Systematic uncertainties
  //! @{
  SysStyle sysStyle() const { return _sysStyle; }
  void setSysStyle(SysStyle style) { _sysStyle = style; }
  //! @brief Fill and outline of the boxes or band. Unless sysUsesMarkerColor() is false, their colors are those of
  //! the markers (the alpha of the fill is kept).
  FillAttributes & sysFillAttributes() { return _sysFill; }
  LineAttributes & sysLineAttributes() { return _sysLine; }
  const FillAttributes & sysFillAttributes() const { return _sysFill; }
  const LineAttributes & sysLineAttributes() const { return _sysLine; }
  bool sysUsesMarkerColor() const { return _sysUsesMarkerColor; }
  void setSysUsesMarkerColor(bool v) { _sysUsesMarkerColor = v; }
  //! @brief Half width of the boxes and brackets of points without x uncertainty, as a fraction of the frame width.
  float sysWidth() const { return _sysWidth; }
  void  setSysWidth(float width) { _sysWidth = width < 0 ? 0 : width; }
  //! @}

  protected:

  Kind _kind;
  const MATH::Function   * _function;
  const MATH::DataPoints * _data;
  String _label;
  float  _xMinimum;
  float  _xMaximum;
  int    _nSamples;
  LineAttributes   _line;
  MarkerAttributes _marker;
  bool   _showLine;
  bool   _showMarkers;
  bool   _showErrors;
  bool   _showSysErrors;
  SysStyle _sysStyle;
  FillAttributes _sysFill;
  LineAttributes _sysLine;
  bool   _sysUsesMarkerColor;
  float  _sysWidth;
  };

  //!
  //! @brief A plot: a frame with an x and a y axis, showing functions and data points (GraphItem).
  //!
  //! A graph occupies a cell of its canvas (setCell(), in normalized canvas coordinates); the frame is the cell
  //! minus the margins (fractions of the cell), which hold the axis labels and the title. Each axis is linear or
  //! logarithmic (setLogX(), setLogY()): unlike ROOT, the scale belongs to the graph, not to the canvas. Ranges are
  //! automatic (covering all the items, rounded to nice values) unless set with setXRange() / setYRange().
  //!
  //! Items are added with add() or replace() (clear the graph, then add). The graph does not own the objects shown.
  //! dimension() is k2D; k3D (surfaces, 3D points) is reserved for the 3D views to come.
  //!
  //! The title, the axis titles and the legend have automatic positions, which can be changed by the program or
  //! with the mouse: setTitleOffset(), Axis::setTitleOffset() (fractions of the cell), setLegendPosition()
  //! (fractions of the frame). Text sizes are automatic (following the size of the cell) unless set in points with
  //! setTitlePointSize(), setLegendPointSize(), Axis::setLabelPointSize() and Axis::setTitlePointSize().
  //!
  //! Each drawing records where the parts of the graph went (titleRect(), legendRect(), Axis::titleRect(),
  //! Axis::bodyRect(), the positions of the data points); pick() uses them to tell what is at a canvas position.
  //!
  class Graph : public Drawable2D
  {
  public:

  enum Dimension { k2D, k3D };

  //! @brief Ranges and scales actually used to draw the graph (automatic or user ranges).
  struct View
  {
  float xMinimum, xMaximum, yMinimum, yMaximum;
  bool  logX, logY;
  };

  //! @brief Parts of a graph, as found by pick().
  enum Part { kNothing, kCell, kFrame, kTitle, kLegend, kAxis, kAxisTitle, kDataPoint };

  //! @brief What is at a canvas position: the part, and for an axis its index (0 x, 1 y; 2 z for the 3D graphs),
  //! for a data point the index of the item and of the point.
  struct Pick
  {
  Part part  = kNothing;
  int  axis  = -1;
  int  item  = -1;
  int  index = -1;
  };

  //! @brief Position at which a data point was drawn (canvas units).
  struct PointPosition
  {
  int     item;
  int     index;
  Point2D position;
  };

  Graph();
  Graph(const Graph & src);
  Graph & operator=(const Graph & rhs);
  virtual ~Graph() { }

  //! @name Placement
  //! @{
  //! @brief Cell occupied by the graph (normalized canvas coordinates, lower-left and upper-right corners).
  void setCell(float xLow, float yLow, float xHigh, float yHigh);
  float cellXLow()  const { return _cellXLow;  }
  float cellYLow()  const { return _cellYLow;  }
  float cellXHigh() const { return _cellXHigh; }
  float cellYHigh() const { return _cellYHigh; }
  //! @brief Margins between the cell and the frame, as fractions of the cell size.
  Margins & margins() { return _margins; }
  const Margins & margins() const { return _margins; }
  void setMargins(const Margins & margins) { _margins = margins; }
  //! @brief Frame (plotting area) in normalized canvas coordinates.
  float frameXLow()  const;
  float frameYLow()  const;
  float frameXHigh() const;
  float frameYHigh() const;
  //! @brief True if the canvas point p is inside the cell of the graph (e.g. to find the target of a drop).
  bool cellContains(const Point2D & p) const;
  //! @brief True if the canvas point p is inside the frame.
  bool frameContains(const Point2D & p) const;
  //! @}

  //! @name Items
  //! @{
  //! @brief Add an item (an object already shown is not added twice). @return the item in the graph
  GraphItem & add(const GraphItem & item);
  //! @brief Remove all the items, then add item. @return the item in the graph
  GraphItem & replace(const GraphItem & item);
  //! @brief Remove the item showing object. @return true if it was found
  bool remove(const void * object);
  //! @brief Remove all the items.
  void clearItems() { _items.clear(); }
  std::vector<GraphItem> & items() { return _items; }
  const std::vector<GraphItem> & items() const { return _items; }
  //! @}

  //! @name Scales and ranges
  //! @{
  void setLogX(bool v) { _logX = v; }
  void setLogY(bool v) { _logY = v; }
  bool isLogX() const { return _logX; }
  bool isLogY() const { return _logY; }
  //! @brief Fixed x range (the y range stays automatic unless set too).
  void setXRange(float minimum, float maximum) { _xMinimum = minimum; _xMaximum = maximum; _autoX = false; }
  void setYRange(float minimum, float maximum) { _yMinimum = minimum; _yMaximum = maximum; _autoY = false; }
  //! @brief Back to automatic ranges.
  void setAutoRange() { _autoX = true; _autoY = true; }
  void setAutoX() { _autoX = true; }
  void setAutoY() { _autoY = true; }
  bool isAutoX() const { return _autoX; }
  bool isAutoY() const { return _autoY; }
  //! @brief Ranges and scales used for drawing: automatic ranges cover all the items (rounded to nice values; on a
  //! log axis, the positive values only). @return view
  View view() const;
  //! @brief Number of major tick intervals that falls on nice values (1, 2, 5 x 10^k) for a linear axis whose
  //! range was rounded by view(). @param minimum lower end @param maximum upper end @return intervals (>= 1)
  static int niceTickCount(double minimum, double maximum);
  //! @brief Canvas position of the data point (x,y) for the given view.
  Point2D toCanvas(const View & view, double x, double y) const;
  //! @brief Data coordinates of a canvas point for the given view (inverse of toCanvas()).
  void fromCanvas(const View & view, const Point2D & p, double & x, double & y) const;
  //! @}

  //! @name Appearance
  //! @{
  Dimension dimension() const { return _dimension; }
  void setDimension(Dimension dimension) { _dimension = dimension; }
  //! Axis templates: title, ticks, line and text attributes (ranges and scales are set from the graph).
  Axis & xAxis() { return _xAxis; }
  Axis & yAxis() { return _yAxis; }
  const Axis & xAxis() const { return _xAxis; }
  const Axis & yAxis() const { return _yAxis; }
  //! @brief Axis k: 0 x, 1 y (2 z, for the 3D graphs to come). @param k index in [0,nAxes())
  int nAxes() const { return 2; }
  Axis & axis(int k) { return k == 0 ? _xAxis : _yAxis; }
  const Axis & axis(int k) const { return k == 0 ? _xAxis : _yAxis; }
  FillAttributes & frameFill() { return _frameFill; }
  LineAttributes & frameLine() { return _frameLine; }
  const FillAttributes & frameFill() const { return _frameFill; }
  const LineAttributes & frameLine() const { return _frameLine; }
  TextAttributes & titleAttributes() { return _titleAttributes; }
  const TextAttributes & titleAttributes() const { return _titleAttributes; }
  bool showLegend() const { return _showLegend; }
  void setShowLegend(bool v) { _showLegend = v; }
  //! @}

  //! @name Positions and sizes of the texts
  //! @{
  //! @brief Displacement of the title from its automatic position (centered above the frame), in fractions of the
  //! cell. @param dx horizontal displacement @param dy vertical displacement
  void  setTitleOffset(float dx, float dy) { _titleOffsetX = dx; _titleOffsetY = dy; }
  float titleOffsetX() const { return _titleOffsetX; }
  float titleOffsetY() const { return _titleOffsetY; }
  //! @brief Size of the title in device units (points); 0: automatic.
  void  setTitlePointSize(float size) { _titlePointSize = size < 0 ? 0 : size; }
  float titlePointSize() const { return _titlePointSize; }
  //! @brief Upper left corner of the legend, in fractions of the frame ((0,0) lower left, (1,1) upper right).
  //! The default, automatic, position is the upper right corner of the frame.
  void  setLegendPosition(float x, float y) { _legendX = x; _legendY = y; _autoLegend = false; }
  void  setAutoLegendPosition() { _autoLegend = true; }
  bool  isAutoLegendPosition() const { return _autoLegend; }
  float legendX() const { return _legendX; }
  float legendY() const { return _legendY; }
  //! @brief Size of the legend text in device units (points); 0: automatic.
  void  setLegendPointSize(float size) { _legendPointSize = size < 0 ? 0 : size; }
  float legendPointSize() const { return _legendPointSize; }
  //! @brief Background and border of the legend (default: none).
  FillAttributes & legendFill() { return _legendFill; }
  LineAttributes & legendLine() { return _legendLine; }
  const FillAttributes & legendFill() const { return _legendFill; }
  const LineAttributes & legendLine() const { return _legendLine; }
  //! @}

  //! @name Layout of the last drawing
  //! @{
  const Rect2D & titleRect()  const { return _titleRect; }
  const Rect2D & legendRect() const { return _legendRect; }
  const std::vector<PointPosition> & pointPositions() const { return _pointPositions; }
  //! Set by the renderer.
  void setDrawnLayout(const Rect2D & title, const Rect2D & legend, const std::vector<PointPosition> & points) const
  { _titleRect = title; _legendRect = legend; _pointPositions = points; }
  //! @brief What is at the canvas position p, according to the last drawing: legend, title, axis titles, data
  //! points (within toleranceX, toleranceY of p, canvas units), axes, frame, cell. @return pick
  Pick pick(const Point2D & p, float toleranceX, float toleranceY) const;
  //! @}

  virtual void reset();

  protected:

  float _cellXLow, _cellYLow, _cellXHigh, _cellYHigh;
  Margins _margins;
  std::vector<GraphItem> _items; //! not streamed (the objects shown are not owned)
  bool  _logX, _logY;
  bool  _autoX, _autoY;
  float _xMinimum, _xMaximum, _yMinimum, _yMaximum;
  Dimension _dimension;
  Axis _xAxis;
  Axis _yAxis;
  FillAttributes _frameFill;
  LineAttributes _frameLine;
  TextAttributes _titleAttributes;
  bool _showLegend;
  float _titleOffsetX, _titleOffsetY;
  float _titlePointSize;
  bool  _autoLegend;
  float _legendX, _legendY;
  float _legendPointSize;
  FillAttributes _legendFill;
  LineAttributes _legendLine;
  mutable Rect2D _titleRect;                          //! not streamed (set when drawn)
  mutable Rect2D _legendRect;                         //! not streamed (set when drawn)
  mutable std::vector<PointPosition> _pointPositions; //! not streamed (set when drawn)

  ClassDef(Graph,0)
  };

  //! @brief Round [low,high] outward to multiples of a 1, 2, 5 x 10^k step (about 5 intervals); step is set.
  void niceLinear(double & low, double & high, double & step);

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Graph
