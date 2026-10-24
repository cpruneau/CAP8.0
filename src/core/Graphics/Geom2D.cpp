#include "Geom2D.hpp"


namespace CAP
{
  namespace GRAPHICS
  {
  Geom2D::Geom2D()
  :
  Drawable2D(),
  _width(0.1),
  _height(0.1),
  _angle(0.0),
  _text(""),
  _fillAttributes(*FillAttributes::defaultFillAttributes()),
  _lineAttributes(*LineAttributes::defaultLineAttributes()),
  _textAttributes(*TextAttributes::defaultTextAttributes())
  {
  _textAttributes.setJustification(TextAttributes::kCenter); // text is centered in shapes
  }

  Geom2D::Geom2D(const Geom2D & src)
  :
  Drawable2D(src),
  _width(src._width),
  _height(src._height),
  _angle(src._angle),
  _text(src._text),
  _fillAttributes(src._fillAttributes),
  _lineAttributes(src._lineAttributes),
  _textAttributes(src._textAttributes)
  { }

  Geom2D  & Geom2D::operator=(const Geom2D & rhs)
  {
  if (this != &rhs)
    {
    Drawable2D::operator=(rhs);
    _width          = rhs._width;
    _height         = rhs._height;
    _angle          = rhs._angle;
    _text           = rhs._text;
    _fillAttributes = rhs._fillAttributes;
    _lineAttributes = rhs._lineAttributes;
    _textAttributes = rhs._textAttributes;
    }
  return *this;
  }

  void Geom2D::reset()
  {
  Drawable2D::reset();
  _fillAttributes.reset();
  _lineAttributes.reset();
  _textAttributes.reset();
  }


  } // namespace GRAPHICS
} // namespace CAP
