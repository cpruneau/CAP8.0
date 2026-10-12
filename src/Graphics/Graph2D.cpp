#include "Graph2D.hpp"
#include "Exceptions.hpp"

ClassImp(CAP::GRAPHICS::Graph2D);

namespace CAP
{
  namespace GRAPHICS
  {

  Graph2D::Graph2D()
  :
  Drawable2D(),
  _axesStyle(kOneSide),
  _margins(),
  _width(5.0),
  _height(5.0),
  _xAxis(new Axis()),
  _xAxis2(nullptr),
  _yAxis(new Axis()),
  _yAxis2(nullptr),
  _boundaryBox(new Polygon(4)),
  _dataSets(),
  _fillAttributes(*FillAttributes::defaultFillAttributes()),
  _lineAttributes(*LineAttributes::defaultLineAttributes()),
  _textAttributes(*TextAttributes::defaultTextAttributes())
  {  }

  Graph2D::Graph2D(Data2D* dataSet)
  :
  Drawable2D(),
  _axesStyle(kOneSide),
  _margins(),
  _width(5.0),
  _height(5.0),
  _xAxis(new Axis()),
  _xAxis2(nullptr),
  _yAxis(new Axis()),
  _yAxis2(nullptr),
  _boundaryBox(new Polygon(4)),
  _dataSets(),
  _fillAttributes(*FillAttributes::defaultFillAttributes()),
  _lineAttributes(*LineAttributes::defaultLineAttributes()),
  _textAttributes(*TextAttributes::defaultTextAttributes())
  {
  if (!dataSet) throw Exception("Null pointer provided in lieu of data set",__FUNCTION__);
  _dataSets.push_back(dataSet);
  }

  Graph2D::Graph2D(std:vector<Data2D*> & dataSets)
  :
  Drawable2D(),
  _axesStyle(kOneSide),
  _margins(),
  _width(5.0),
  _height(5.0),
  _xAxis(new Axis()),
  _xAxis2(nullptr),
  _yAxis(new Axis()),
  _yAxis2(nullptr),
  _boundaryBox(new Polygon(4)),
  _dataSets(dataSets),
  _fillAttributes(*FillAttributes::defaultFillAttributes()),
  _lineAttributes(*LineAttributes::defaultLineAttributes()),
  _textAttributes(*TextAttributes::defaultTextAttributes())
  { }


  Graph2D::Graph2D(const Graph2D & src)
  :
  Drawable2D(src),
  _axesStyle(src._axesStyle),
  _margins(src._margins),
  _width(src._width),
  _height(src._height),
  _xAxis(new Axis(*src._xAxis)),
  _xAxis2(nullptr),
  _yAxis(new Axis(*src._yAxis)),
  _yAxis2(nullptr),
  _boundaryBox(new Polygon(*src._boundaryBox))
  _dataSets(src._dataSets),
  _fillAttributes(src._fillAttributes),
  _lineAttributes(src._lineAttributes),
  _textAttributes(src._textAttributes)
  {
  if (src._xAxis2) _xAxis2 = new Axis(*src._xAxis2);
  if (src._yAxis2) _yAxis2 = new Axis(*src._yAxis2);
  }


  Graph2D  & Graph2D::operator=(const Graph2D & rhs)
  {
  if (this != &rhs)
    {
    _axesStyle = rhs._axesStyle;
    _margins = rhs._margins;
    _width = rhs._width;
    _height = rhs._height;
    (*_xAxis) = *src._xAxis;
    if (rhs._xAxis2) (*_xAxis2) = *rhs._xAxis2;
    (*_yAxis)  = *src._yAxis;
    if (rhs._yAxis2) (*_yAxis2) = *rhs._yAxis2;
    (*_boundaryBox) = *rhs._boundaryBox;
    _dataSets = src._dataSets;
    _fillAttributes = rhs._fillAttributes;
    _lineAttributes = rhs._lineAttributes;
    _textAttributes = rhs._textAttributes;
    }
  return *this;
  }

  } //  namespace GRAPHICS
} // namespace CAP
