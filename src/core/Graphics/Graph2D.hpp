#ifndef CAP__Graph2D
#define CAP__Graph2D
#include "DataPoints.hpp"
#include "Drawable2D.hpp"
#include "FillAttributes.hpp"
#include "LineAttributes.hpp"
#include "TextAttributes.hpp"

namespace CAP
{
  namespace GRAPHICS
  {

  //!
  //!Class that encapsulates all types of 2D graphs
  //!including y vs. x (w/ or w/o errors), histogram
  //!Pie charts and so on are not included in this class (for now)
  //!
  class Graph2D : public Drawable2D
  {
  public:

  enum AxisStyle { kOneSide, kBothSides}

  Graph2D();
  Graph2D(MATH::DataPoints* dataSet);
  Graph2D(std:vector<MATH::DataPoints*> & dataSets);
  Graph2D(const Graph2D & src);
  Graph2D  & operator=(const Graph2D & rhs);
  virtual ~Graph2D() { }

  void setDrawBoundary(bool drawBoundary) { _drawBoundary = drawBoundary; }
  void setAxisStyle(AxisStyle _axesStyle) { _axesStyle    = axesStyle;    }
  void setMargins(const Margins & margins) { _margins     = margins;      }
  void setWidth(float width) { _width = width; }
  void setHeight(float height) { _height = height; }
  void setWidthHeight(float width, float height) { _width = width; _height = height; }
  void setFillAttributes(const FillAttributes & fillAttributes) { _fillAttributes = fillAttributes; }
  void setLineAttributes(const LineAttributes & lineAttributes) { _lineAttributes = lineAttributes; }
  void setTextAttributes(const TextAttributes & textAttributes) { _textAttributes = textAttributes; }

  void addDataSet(MATH::DataPoints* dataSet); //  { _dataSets; }

  AxisStyle axisStyle() const { return _axesStyle; }
  const Margins & margins() const { return _margins; }
  Margins & margins() { return _margins; }
  float width()  const { return _width; }
  float height() const { return _height;}

  Axis & xAxis()  { return *_xAxis; }
  Axis & xAxis2() { return *_xAxis2; }
  Axis & yAxis()  { return *_yAxis; }
  Axis & yAxis2() { return *_yAxis2; }

  Polygon & boundaryBox  { return *_boundaryBox; }
  const Polygon & boundaryBox const { return *_boundaryBox; }

  std:vector<MATH::DataPoints*> & dataSets { return _dataSets; }
  const std:vector<MATH::DataPoints*> & dataSets  const  { return _dataSets; }

  FillAttributes & fillAttributes() { return _fillAttributes; }
  LineAttributes & lineAttributes() { return _lineAttributes; }
  TextAttributes & textAttributes() { return _textAttributes; }
  const FillAttributes & fillAttributes() const { return _fillAttributes; }
  const LineAttributes & lineAttributes() const { return _lineAttributes; }
  const TextAttributes & textAttributes() const { return _textAttributes; }

  protected:
  AxisStyle _axesStyle(0),
  Margins _margins;
  float   _width;
  float   _height;
  Axis *  _xAxis;
  Axis *  _xAxis2;
  Axis *  _yAxis;
  Axis *  _yAxis2;
  Polygon * _boundaryBox; //! This is the box surrounding the actual graph(s)
  std:vector<MATH::DataPoints*> _dataSets;

  FillAttributes _fillAttributes;
  LineAttributes _lineAttributes;
  TextAttributes _textAttributes;

  ClassDef(Graph2D,0)
  };




  } //  namespace GRAPHICS
} // namespace CAP

#endif // !CAP__Graph2D
