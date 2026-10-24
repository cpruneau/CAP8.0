#include "LineAttributes.hpp"


namespace CAP
{
  namespace GRAPHICS
  {
  LineAttributes::LineAttributes()
  :
  Attributes(),
  _width(1.0)
  { }

  LineAttributes::LineAttributes(const LineAttributes & src)
  :
  Attributes(src),
  _width(src._width)
  { }

  LineAttributes  & LineAttributes::operator=(const LineAttributes & rhs)
  {
  if (this != &rhs)
    {
    Attributes::operator=(rhs);
    _width = rhs._width;
    }
  return *this;
  }


  void LineAttributes::reset()
  {
  const LineAttributes & d = *defaultLineAttributes();
  _color = d._color;
  _style = d._style;
  _alpha = d._alpha;
  _width = d._width;
  }

  LineAttributes * LineAttributes::_defaultLineAttributes = nullptr;

  LineAttributes * LineAttributes::defaultLineAttributes()
  {
  if (!_defaultLineAttributes) _defaultLineAttributes = new LineAttributes();
  return _defaultLineAttributes;
  }

  void LineAttributes::setDefaultLineAttributes(int color, float alpha, int style, float width)
  {
  if (!_defaultLineAttributes) _defaultLineAttributes = new LineAttributes();
  _defaultLineAttributes->set(color, alpha, style, width);
  }


  } // namespace GRAPHICS
} // namespace CAP
